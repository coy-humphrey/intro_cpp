// Author: Coy Humphrey (cmhumphr)

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include "cix_protocol.h"
#include "logstream.h"
#include "signal_action.h"
#include "sockets.h"

logstream log (cout);

unordered_map<string,cix_command> command_map {
   {"exit", CIX_EXIT},
   {"help", CIX_HELP},
   {"ls"  , CIX_LS  },
   {"put" , CIX_PUT },
   {"rm"  , CIX_RM  },
   {"get" , CIX_GET },
};

void cix_help() {
   static vector<string> help = {
      "exit         - Exit the program.  Equivalent to EOF.",
      "get filename - Copy remote file to local host.",
      "help         - Print help summary.",
      "ls           - List names of files on remote server.",
      "put filename - Copy local file to remote host.",
      "rm filename  - Remove file from remote server.",
   };
   for (const auto& line: help) cout << line << endl;
}

void cix_ls (client_socket& server) {
   cix_header header;
   memset (&header, 0, sizeof (header));
   header.cix_command = CIX_LS;
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.cix_command != CIX_LSOUT) {
      log << "sent CIX_LS, server did not return CIX_LSOUT" << endl;
      log << "server returned " << header << endl;
   }else {
      header.cix_nbytes = ntohl (header.cix_nbytes);
      char buffer[header.cix_nbytes + 1];
      recv_packet (server, buffer, header.cix_nbytes);
      log << "received " << header.cix_nbytes << " bytes" << endl;
      buffer[header.cix_nbytes] = '\0';
      cout << buffer;
   }
}

void cix_put (client_socket& server, vector<string> args)
{
   if (args.size() < 2)
   {
      log << "put: filename required" << endl;
      return;
   }
   if (args.size() > 2)
   {
      log << "put: too many arguments" << endl;
      return;
   }
   // Parse args[1] to determine if filename is ok
   if (args[1].size() > 58)
   {
      log << "put: filename is too long" << endl;
      return;
   }
   if (args[1].find_first_of ("/") != string::npos)
   {
      log << "put: filename cannot contain /" << endl;
      return;
   }
   
   struct stat st;
   int rc = stat (args[1].c_str(), &st);
   if (rc < 0)
   {
      log << "put: stat failed: " << strerror (errno) << endl;
      return;
   }
   if (S_ISDIR (st.st_mode))
   {
      log << "put: cannot put a directory" << endl;
      return;
   }
   
   cix_header header;
   memset (&header, 0, sizeof (header));
   header.cix_command = CIX_PUT;
   strncpy (header.cix_filename, args[1].c_str(), CIX_FILENAME_SIZE);
   header.cix_nbytes = htonl (st.st_size);
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof (header));
   ifstream ifs;
   ifs.open (args[1], ifstream::in | ifstream::binary);
   if (!ifs.is_open())
   {
      log << "put: failed to open file" << endl;
      return;
   }
   int bufsize = 1024;
   char buf[bufsize];
   for (int i = 0; i < st.st_size;)
   {
      int to_read = st.st_size - i;
      to_read = to_read > bufsize ? bufsize : to_read;
      ifs.read (buf, to_read);
      send_packet (server, &buf, to_read);
      i += to_read;
   }
   log << "sent " << st.st_size << " bytes" << endl;
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.cix_command != CIX_ACK) {
      log << "sent CIX_PUT, server did not return CIX_ACK" << endl;
   }
}

void cix_rm (client_socket& server, vector<string> args)
{
   if (args.size() < 2)
   {
      log << "rm: filename required" << endl;
      return;
   }
   if (args.size() > 2)
   {
      log << "rm: too many arguments" << endl;
      return;
   }
   // Parse args[1] to determine if filename is ok
   if (args[1].size() >= CIX_FILENAME_SIZE)
   {
      log << "rm: filename is too long" << endl;
      return;
   }
   if (args[1].find_first_of ("/") != string::npos)
   {
      log << "rm: filename cannot contain /" << endl;
      return;
   }
   // Fill header
   cix_header header;
   memset (&header, 0, sizeof (header));
   header.cix_command = CIX_RM;
   strncpy (header.cix_filename, args[1].c_str(), CIX_FILENAME_SIZE);
   // Send header
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof (header));
   
   // Recv response
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.cix_command != CIX_ACK) {
      log << "sent CIX_RM, server did not return CIX_ACK" << endl;
   }
}

void cix_get (client_socket& server, vector<string> args)
{
   if (args.size() < 2)
   {
      log << "get: filename required" << endl;
      return;
   }
   if (args.size() > 2)
   {
      log << "get: too many arguments" << endl;
      return;
   }
   // Parse args[1] to determine if filename is ok
   if (args[1].size() > 58)
   {
      log << "get: filename is too long" << endl;
      return;
   }
   if (args[1].find_first_of ("/") != string::npos)
   {
      log << "get: filename cannot contain /" << endl;
      return;
   }
   cix_header header;
   memset (&header, 0, sizeof (header));
   header.cix_command = CIX_GET;
   strncpy (header.cix_filename, args[1].c_str(), CIX_FILENAME_SIZE);
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof (header));
   
   recv_packet (server, &header, sizeof (header));
   log << "received header " << header << endl;
   
   if (header.cix_command != CIX_FILE) {
      log << "sent CIX_GET, server did not return CIX_FILE" << endl;
      return;
   }
   
   int32_t nbytes = ntohl (header.cix_nbytes);
   ofstream out (header.cix_filename, ofstream::binary);
   if (!out.is_open())
   {
      header.cix_nbytes = htonl (errno);
      log << "get: failed to open file" << endl;
   }
   // Read in all the data
   int bufsize = 1024;
   char buf[bufsize];
   for (int i = 0; i < nbytes;)
   {
      int to_read = nbytes - i;
      to_read = to_read > bufsize ? bufsize : to_read;
      recv_packet (server, buf, to_read);
      out.write (buf, to_read);
      i += to_read;
   }
   log << "received " << nbytes << " bytes" << endl;
}


void usage() {
   cerr << "Usage: " << log.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

void signal_handler (int signal) {
   log << "signal_handler: caught " << strsignal (signal) << endl;
   switch (signal) {
      case SIGINT: case SIGTERM: throw cix_exit();
      default: break;
   }
}

vector<string> split (const string& line, const string& delimiters) {
   vector<string> words;
   int end = 0;
   for (;;) {
      size_t start = line.find_first_not_of (delimiters, end);
      if (start == string::npos) break;
      end = line.find_first_of (delimiters, start);
      words.push_back (line.substr (start, end - start));
   }
   return words;
}

int main (int argc, char** argv) {
   log.execname (basename (argv[0]));
   log << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   // signal_action (SIGINT, signal_handler);
   // signal_action (SIGTERM, signal_handler);
   if (args.size() > 2) usage();
   string host = get_cix_server_host (args, 0);
   int ind = args.size() - 1;
   in_port_t port = get_cix_server_port (args, ind);
   log << to_string (hostinfo()) << endl;
   try {
      log << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      log << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         log << "command " << line << endl;
         // Splitting command
         vector<string> command_args = split (line, " \t");
         cix_command cmd;
         if (command_args.size() < 1) continue;
         else
         { 
            const auto& itor = command_map.find (command_args[0]);
            cmd = itor == command_map.end() ? CIX_ERROR : itor->second;
         }
         switch (cmd) {
            case CIX_EXIT:
               throw cix_exit();
               break;
            case CIX_HELP:
               cix_help();
               break;
            case CIX_LS:
               cix_ls (server);
               break;
            case CIX_PUT:
               cix_put (server, command_args);
               break;
            case CIX_RM:
               cix_rm (server, command_args);
               break;
            case CIX_GET:
               cix_get (server, command_args);
               break;
            default:
               log << line << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      log << error.what() << endl;
   }catch (cix_exit& error) {
      log << "caught cix_exit" << endl;
   }
   log << "finishing" << endl;
   return 0;
}

