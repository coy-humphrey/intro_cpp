// Author: Coy Humphrey (cmhumphr)

#include <cstdlib>
#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

typedef xpair<string,string> str_str_pair;
typedef listmap<string,string> str_str_map;

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            traceflags::setflags (optarg);
            break;
         default:
            complain() << "-" << (char) optopt << ": invalid option"
                       << endl;
            break;
      }
   }
}

string trim (const string &str) {
   size_t first = str.find_first_not_of (" \t");
   if (first == string::npos) return "";
   size_t last = str.find_last_not_of (" \t");
   return str.substr (first, last - first + 1);
}

void scan_file (istream &in, string fname, str_str_map &map)
{
   for (int lnum = 1;;++lnum)
   {
      string line;
      getline (in, line);
      if (in.eof()) break;
      cout << fname << ": " << lnum << ": " << line << endl;
      line = trim (line);
      if (line.size() == 0 || line[0] == '#') continue;
      size_t pos = line.find_first_of ("=");
      if (pos == string::npos)
      {
         //cout << "key only: " << line << endl;
         // Print key and value pair
         auto itor = map.find (line);
         if (itor == map.end())
         {
            complain() << line << ": Key not found" << endl;
            continue;
         }
         cout << (*itor).first << " = " << (*itor).second << endl;
      }
      else
      {
         string key = trim (line.substr (0, pos));
         string value = trim (line.substr (pos + 1));
         //cout << "key is: " << key << endl;
         //cout << "val is: " << value << endl;
         if (key.empty() && value.empty())
         {
            //cout << "only =" << endl;
            // Print values of map
            for (auto itor = map.begin(); itor != map.end(); ++itor)
            {
               cout << (*itor).first << " = " << (*itor).second << endl;
            }
         }
         else if (value.empty())
         {
            //cout << "key =" << endl;
            // Delete key from map
            auto itor = map.find (key);
            itor.erase();
         }
         else if (key.empty())
         {
            //cout << "= value" << endl;
            // Print all pairs with value
            for (auto itor = map.begin(); itor != map.end(); ++itor)
            {
               if ((*itor).second == value)
               {
                  cout << (*itor).first << " = " 
                     << (*itor).second << endl;
               }
            }
         }
         else
         {
            cout << key << " = " << value << endl;
            // Set key and value pair
            map.insert (xpair<string,string> (key, value));
         }
      }
   }
}

int main (int argc, char** argv) {
   sys_info::set_execname (argv[0]);
   scan_options (argc, argv);
   str_str_map map;
   for (int argi = optind; argi < argc; ++argi) {
      if ((string ("-")).compare (argv[argi]) == 0)
      {
         scan_file (cin, "-", map);
         continue;
      }
      ifstream in;
      in.open (argv[argi]);
      if (!in.is_open())
      {
         complain() << argv[argi] 
            << ": No such file or directory" << endl;
         continue;
      }
      scan_file (in, argv[argi], map);
   }
   if (optind == argc)
   {
      scan_file (cin, "-", map);
   }
   
   delete sys_info::execname;
   return sys_info::get_exit_status();
}

