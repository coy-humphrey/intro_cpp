// $Id: commands.cpp,v 1.10 2014-04-09 17:04:58-07 - - $
// Author: Coy Humphrey (cmhumphr)

#include <iomanip>

#include "commands.h"
#include "debug.h"

commands::commands(): map ({
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   }
}){}

function commands::at (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (function)
   commandmap::const_iterator result = map.find (cmd);
   if (result == map.end()) {
      throw yshell_exn (cmd + ": no such function");
   }
   return result->second;
}

void pwd_recursive (inode_state& state, inode *cwd, string &result)
{
   if (cwd != state.get_root())
   {
      inode *parent = (*cwd).get_parent();
      pwd_recursive (state, parent, result);
      result += "/" + (*cwd).get_name();
   }
}


void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   for (size_t ind = 1; ind < words.size(); ++ind)
   {
      inode *file = state.inode_from_path (words[ind]);
      if (file == nullptr)
      {
         throw yshell_exn ("cat: " + words[ind] + 
                           ": No such file or directory");
      }
      if ((*file).get_type() != FILE_INODE)
         throw yshell_exn ("cat: " + words[ind] + ": Not a file");
         
      cout << (*file).readfile() << endl;
   }
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   if (words.size() > 2)
      throw yshell_exn ("cd: Too many arguments");
   
   if (words.size() == 1)
   {
      state.set_cwd (state.get_root());
      return;
   }
   inode *ncwd = state.inode_from_path (words[1]);
   if (ncwd == nullptr)
      throw yshell_exn ("cd: " + words[1] + ": No such directory");
   if ((*ncwd).get_type() != DIR_INODE)
      throw yshell_exn ("cd: " + words[1] + ": Not a directory");
   state.set_cwd (ncwd);
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   bool want_space = false;
    // Ignore function name
   for (size_t index = 1; index < words.size(); ++index) {
      if (want_space) cout << " ";
      else want_space = true;
      cout << words[index];
   }
   cout << endl;
}

void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   if (words.size() == 1)
   {
      exit_status::set (0);
      throw ysh_exit_exn();
   }
   
   int status {};
   size_t idx {};
   try {status = stoi (words[1], &idx);}
   catch (invalid_argument& ia)
      {status = 127;}
   catch (out_of_range& oor)
      {status = 127;}
   
   if (idx != words[1].size())
      status = 127;
      
   exit_status::set (status);
   throw ysh_exit_exn();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   if (words.size() == 1)
   {
      inode *cwd = state.get_cwd();
      directory dir = (*cwd).get_dirents();
      string result = "";
      pwd_recursive (state, cwd, result);
      if (cwd == state.get_root()) result = "/";
      cout << result << ":" << endl;
      for (auto it = dir.cbegin(); it != dir.cend(); ++it)
      {
         int size = it->second->size();
         int nr   = it->second->get_inode_nr();
         string name = it->first;
         if (name != "." && name != "..")
            if (it->second->get_type() == DIR_INODE)
               name += "/";
         cout << setw (6) << nr << "  " << setw (6) << size << 
                  "  " << setw (6) << name << endl;
      }
   }
   
   else
   {
      for (size_t ind = 1; ind < words.size(); ++ind)
      {
         inode *curr = state.inode_from_path (words[ind]);
         if (curr == nullptr)
            throw yshell_exn ("ls: " + words[ind] + 
                              ": No such file or directory");
         if (curr->get_type() == FILE_INODE)
         {
            int size = curr->size();
            int nr   = curr->get_inode_nr();
            string name = curr->get_name();
            cout << setw (6) << nr << "  " << setw (6) << size << 
                     "  " << setw (6) << name << endl;
         }
         else
         {
            directory dir = curr->get_dirents();
            cout << words[ind] << ":" << endl;
            for (auto it = dir.cbegin(); it != dir.cend(); ++it)
            {
               int size = it->second->size();
               int nr   = it->second->get_inode_nr();
               string name = it->first;
               if (name != "." && name != "..")
                  if (it->second->get_type() == DIR_INODE)
                     name += "/";
               cout << setw (6) << nr << "  " << setw (6) << size << 
                        "  " << setw (6) << name << endl;
            }
         }
         
      }
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   inode *cd {nullptr};
   
   if (words.size() == 1)
   {
      cd = state.get_cwd();
      ls_recursive (state, cd);
   }
   else
   {                   
      for (size_t ind = 1; ind < words.size(); ++ind)
      {
         cd = state.inode_from_path (words[ind]);
         if (cd == nullptr)
            throw yshell_exn ("lsr: " + words[ind] + 
                              ": No such file or directory");
         if (cd->get_type() != DIR_INODE)
         {
            wordvec ls_words {"", words[ind]};
            fn_ls (state, ls_words);
         }
         else
            ls_recursive (state, cd);
      }
   }
   
}

void ls_recursive (inode_state& state, inode *cd)
{
   if (cd->get_type() != DIR_INODE)
      throw yshell_exn ("lsr: ls_recursive error");
      
   string result = "";
   pwd_recursive (state, cd, result);
   if (cd == state.get_root()) result = "/";
   
   wordvec ls_wordvec{"", result};
   fn_ls (state, ls_wordvec);
   
   directory dir = cd->get_dirents();
   for (auto it : dir)
   {
      if (it.first == "." || it.first == "..") continue;
      if (it.second->get_type() == FILE_INODE) continue;
      ls_recursive (state, it.second);
   }
}



void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   if (words.size() < 2)
      throw yshell_exn ("make: Not enough arguments");
   
   inode *existing = state.inode_from_path (words[1]);
   if (existing != nullptr)
   {
      if (existing->get_type() == DIR_INODE)
         throw yshell_exn ("make: " + words[1] + 
                           ": Cannot write to directory");
      wordvec filedata;
      for (size_t i = 2; i < words.size(); ++i)
      {
         filedata.push_back (words[i]);
      }
      (*existing).writefile (filedata);
      return;
   }
   
   // Last entry in path will be file to make
   wordvec path = split (words[1], "/");
   string fname = path.at (path.size() - 1);
   // Cut out new file from path
   size_t fname_pos = words[1].rfind (fname);
   string path_str = words[1];
   path_str.erase (fname_pos);
   
   inode *parent {nullptr};
   
   if (path.size() == 1)
   {
      parent = state.get_cwd();
   }
   else
   {
      parent = state.inode_from_path (path_str);
   }
   
   inode& child = (*parent).mkfile (fname);
   if (words.size() > 2)
   {
      // Write the remaining words to the file
      wordvec filedata;
      for (size_t ind = 2; ind < words.size(); ++ind)
      {
         filedata.push_back (words[ind]);
      }
      (child).writefile (filedata);
   }
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   if (words.size() != 2)
   {
      throw yshell_exn ("mkdir: Invalid arguments");
   }
   
   inode *existing = state.inode_from_path(words[1]);
   if (existing != nullptr)
   {
      throw yshell_exn ("mkdir: " + words[1] + 
                        ": Directory already exists");
   }
   
   // Last entry in path will be directory to make
   wordvec path = split (words[1], "/");
   string fname = path.at (path.size() - 1);
   // Cut out new directory from path
   size_t fname_pos = words[1].rfind (fname);
   string path_str = words[1];
   path_str.erase (fname_pos);
   
   inode *parent {nullptr};
   
   if (path.size() == 1)
   {
      parent = state.get_cwd();
   }
   else
   {
      parent = state.inode_from_path (path_str);      
      if (parent == nullptr)
      {
         throw yshell_exn ("mkdir: " + words[1] + 
                           ": Parent directory does not exist");
      }
   }
   
   (*parent).mkdir (fname);
   
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   state.set_prompt("");
   for (size_t index = 1; index < words.size(); ++index)
   {
      state.set_prompt (state.get_prompt() + words[index] + " ");
   }
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string wd = "";
   pwd_recursive (state, state.get_cwd(), wd);
   if (state.get_cwd() == state.get_root()) wd += "/";
   cout << wd << endl;
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   if (words.size() != 2)
      throw yshell_exn ("rm: Invalid arguments");
   
      
   inode *existing = state.inode_from_path(words[1]);
   if (existing == nullptr)
      throw yshell_exn ("rm: " + words[1] + 
                        ": No such file or directory");
   if (existing->get_type() == DIR_INODE && existing->size() > 2)
      throw yshell_exn ("rm: " + words[1] + 
                        ": Can't remove non-empty directory");
   // Last entry in path will be file to remove
   wordvec path = split (words[1], "/");
   string fname = path.at (path.size() - 1);
   // Cut out file from path
   size_t fname_pos = words[1].rfind (fname);
   string path_str = words[1];
   path_str.erase (fname_pos);
   
   if (fname == "." || fname == ".." || fname == "/")
      throw yshell_exn ("rm: cannot remove . or .. or /");
   
   inode *parent {nullptr};
   
   if (path.size() == 1)
   {
      parent = state.get_cwd();
   }
   else
   {
      parent = state.inode_from_path (path_str);      
   }
   
   parent->remove (fname);
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   if (words.size() != 2)
      throw yshell_exn ("rmr: Invalid arguments");
      
   inode *existing = state.inode_from_path(words[1]);
   if (existing == nullptr)
      throw yshell_exn ("rmr: " + words[1] + 
                        ": No such file or directory");
   
   // Last entry in path will be file to remove
   wordvec path = split (words[1], "/");
   string fname = path.at (path.size() - 1);
   // Cut out file from path
   size_t fname_pos = words[1].rfind (fname);
   string path_str = words[1];
   path_str.erase (fname_pos);
   
   if (fname == "." || fname == ".." || fname == "/")
      throw yshell_exn ("rmr: cannot remove . or .. or /");
   
   inode *parent {nullptr};
   
   if (path.size() == 1)
   {
      parent = state.get_cwd();
   }
   else
   {
      parent = state.inode_from_path (path_str);      
   }
   
   parent->remove (fname);
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

