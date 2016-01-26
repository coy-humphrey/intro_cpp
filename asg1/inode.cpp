// $Id: inode.cpp,v 1.4 2014-04-09 17:04:58-07 - - $
// Author: Coy Humphrey (cmhumphr)

#include <cassert>
#include <iostream>

using namespace std;

#include "debug.h"
#include "inode.h"

int inode::next_inode_nr {1};

inode::inode(inode_t init_type):
   inode_nr (next_inode_nr++), type (init_type)
{
   switch (type) {
      case DIR_INODE:
           contents.dirents = new directory();
           break;
      case FILE_INODE:
           contents.data = new wordvec();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

//
// copy ctor -
//    Make a copy of a given inode.  This should not be used in
//    your program if you can avoid it, since it is expensive.
//    Here, we can leverage operator=.
//
inode::inode (const inode& that) 
{
   *this = that;
}

// destructor
//    Free up the memory allocated with new
inode::~inode ()
{
   switch (type)
   {
      case DIR_INODE:
         delete contents.dirents;
         break;
      case FILE_INODE:
         delete contents.data;
         break;
   }
}

//
// operator= -
//    Assignment operator.  Copy an inode.  Make a copy of a
//    given inode.  This should not be used in your program if
//    you can avoid it, since it is expensive.
//
inode& inode::operator= (const inode& that) 
{
   if (this != &that) {
      inode_nr = that.inode_nr;
      type = that.type;
      contents = that.contents;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
   return *this;
}


int inode::get_inode_nr() const 
{
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

int inode::size() const 
{
   int size {0};
   if (type == FILE_INODE)
   {
      for (auto i : *(contents.data))
      {
         // Add 1 for each word
         size++;
         // Add the length of each string
         size += i.length();
      }
      size--; // # of words - 1 is number of spaces to be printed
   }
   else
   {
      size += contents.dirents->size();
   }
   DEBUGF ('i', "size = " << size);
   return size;
}

inode_t inode::get_type() const
{
   return type;
}

const wordvec& inode::readfile() const 
{
   DEBUGF ('i', *contents.data);
   if (type != FILE_INODE) 
      throw yshell_exn ("readfile called on DIR_INODE");
   return *(contents.data);
}

void inode::writefile (const wordvec& words) 
{
   DEBUGF ('i', words);
   if (type != FILE_INODE) 
      throw yshell_exn ("writefile called on DIR_INODE");
   
   contents.data->assign (words.cbegin(), words.cend());
}

void inode::remove (const string& filename) 
{
   DEBUGF ('i', filename);
   assert (type == DIR_INODE);
   // Check to see if file exists
   if (contents.dirents->count (filename) == 0)
      throw yshell_exn ("remove: no such inode");
      
   if (filename == "." || filename == ".." || filename == "/")
      throw yshell_exn ("remove: can't remove . or .. or /");
      
   inode *curr = contents.dirents->at (filename);
   contents.dirents->erase (filename);
   inode_state::recursive_delete (curr);
}

inode& inode::mkdir (const string& dirname)
{
   inode *dir = new inode (DIR_INODE);
   dir->contents.dirents->insert (make_pair (".", dir));
   dir->contents.dirents->insert (make_pair ("..", this));
   contents.dirents->insert (make_pair (dirname, dir));
   dir->name = dirname;
   return *dir;
}

inode& inode::mkfile (const string& filename)
{
   inode *f = new inode (FILE_INODE);
   contents.dirents->insert (make_pair (filename, f));
   f->name = filename;
   return *f;
}

string inode::get_name()
{
   return name;
}

inode *inode::get_parent()
{
   if (type != DIR_INODE)
   {
      return nullptr;
   }
   directory *dir = contents.dirents;
   return dir->at ("..");
}

directory inode::get_dirents()
{
   return *(contents.dirents);
}

inode_state::inode_state() {
   root = new inode (DIR_INODE);
   root->contents.dirents->insert (make_pair (".", root));
   root->contents.dirents->insert (make_pair ("..", root));
   cwd = root;
   
   DEBUGF ('i', "root = " << (void*) root << ", cwd = " << (void*) cwd
          << ", prompt = " << prompt);
}

inode_state::~inode_state()
{
   recursive_delete (root);
   root = nullptr;
   cwd = nullptr;
}

void inode_state::recursive_delete(inode *node)
{
   if ((*node).get_type() == FILE_INODE)
   {
      delete node;
   }
   else
   {
      //directory dir = *(((*node).contents).dirents);
      directory *dir = node->contents.dirents;
      dir->erase (".");
      dir->erase ("..");
      
      for (auto i : *dir)
      {
         inode *curr = i.second;
         if ((*curr).get_type() == FILE_INODE)
         {
            delete curr;
         }
         else
         {
            recursive_delete (curr);
         }
      }
      delete node;
   }
}

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

// Pre: path.size >= 1
inode *inode_state::inode_from_path (const string &path)
{
   wordvec dirs = split (path, "/");
   inode *curr = cwd;
   if (path.at (0) == '/')
   {
      curr = root;
   }
   
   for (auto& i : dirs)
   {
      if ((*curr).get_type() != DIR_INODE)
         throw yshell_exn ("invalid path");
      directory *d = curr->contents.dirents;
      if (d->count (i) == 0)
         return nullptr;
      curr = d->at (i);
   }
   return curr;
}

string inode_state::get_prompt()
{
   return prompt;
}

void inode_state::set_prompt(const string &p)
{
   prompt = p;
}

inode *inode_state::get_cwd()
{
   return cwd;
}

inode *inode_state::get_root()
{
   return root;
}

void inode_state::set_cwd (inode *c)
{
   cwd = c;
}
