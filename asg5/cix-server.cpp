// Author: Coy Humphrey (cmhumphr)

#include <iostream>
#include <fstream>
using namespace std;

#include <libgen.h>
#include <sys/stat.h>

#include "cix_protocol.h"
#include "logstream.h"
#include "signal_action.h"
#include "sockets.h"

logstream log (cout);

void reply_ls (accepted_socket& client_sock, cix_header& header) {
   FILE* ls_pipe = popen ("ls -l", "r");
   if (ls_pipe == NULL) {
      log << "ls -l: popen failed: " << strerror (errno) << endl;
      header.cix_command = CIX_NAK;
      header.cix_nbytes = errno;
      send_packet (client_sock, &header, sizeof header);
   }
   string ls_output;
   char buffer[0x1000];
   for (;;) {
      char* rc = fgets (buffer, sizeof buffer, ls_pipe);
      if (rc == nullptr) break;
      ls_output.append (buffer);
   }
   header.cix_command = CIX_LSOUT;
   // Changed to htonl
   header.cix_nbytes = htonl (ls_output.size());
   memset (header.cix_filename, 0, CIX_FILENAME_SIZE);
   log << "sending header " << header << endl;
   send_packet (client_sock, &header, sizeof header);
   send_packet (client_sock, ls_output.c_str(), ls_output.size());
   log << "sent " << ls_output.size() << " bytes" << endl;
}

void reply_put (accepted_socket& client_sock, cix_header& header)
{
   int32_t nbytes = ntohl (header.cix_nbytes);
   ofstream out (header.cix_filename, ofstream::binary);
   if (!out.is_open())
   {
      header.cix_nbytes = htonl (errno);
      log << "reply_put: failed to open file" << endl;
      header.cix_command = CIX_NAK;
      log << "sending header " << header << endl;
      send_packet (client_sock, &header, sizeof (header));
      return;
   }
   // Read in all the data
   int bufsize = 1024;
   char buf[bufsize];
   for (int i = 0; i < nbytes;)
   {
      int to_read = nbytes - i;
      to_read = to_read > bufsize ? bufsize : to_read;
      recv_packet (client_sock, buf, to_read);
      out.write (buf, to_read);
      i += to_read;
   }
   log << "received " << nbytes << " bytes" << endl;
   header.cix_command = CIX_ACK;
   log << "sending header " << header << endl;
   send_packet (client_sock, &header, sizeof (header));
}

void reply_rm (accepted_socket& client_sock, cix_header& header)
{
   int rc = unlink (header.cix_filename);
   if (rc < 0)
   {
      header.cix_nbytes = htonl (errno);
      log << "reply_rm: unlink failed: " << strerror (errno) << endl;
      header.cix_command = CIX_NAK;
      send_packet (client_sock, &header, sizeof header);
      return;
   }
   header.cix_command = CIX_ACK;
   log << "sending header " << header << endl;
   send_packet (client_sock, &header, sizeof (header));
}

void reply_get (accepted_socket& client_sock, cix_header& header)
{
   struct stat st;
   int rc = stat (header.cix_filename, &st);
   if (rc < 0)
   {
      header.cix_nbytes = htonl (errno);
      log << "reply_get: stat failed" << strerror (errno) << endl;
      header.cix_command = CIX_NAK;
      log << "sending header " << header << endl;
      send_packet (client_sock, &header, sizeof (header));
      return;
   }
   if (S_ISDIR (st.st_mode))
   {
      log << "reply_get: cannot get a directory" << endl;
      header.cix_command = CIX_NAK;
      log << "sending header " << header << endl;
      send_packet (client_sock, &header, sizeof (header));
      return;
   }
   
   header.cix_command = CIX_FILE;
   header.cix_nbytes = htonl (st.st_size);
   log << "sending header " << header << endl;
   send_packet (client_sock, &header, sizeof (header));
   ifstream ifs;
   ifs.open (header.cix_filename, ifstream::in | ifstream::binary);
   if (!ifs.is_open())
   {
      log << "reply_get: failed to open file" << endl;
      header.cix_command = CIX_NAK;
      log << "sending header " << header << endl;
      send_packet (client_sock, &header, sizeof (header));
      return;
   }
   char buf[1024];
   for (int i = 0; i < st.st_size;)
   {
      int to_read = st.st_size - i;
      to_read = to_read > 1024 ? 1024 : to_read;
      ifs.read (buf, to_read);
      send_packet (client_sock, &buf, to_read);
      i += to_read;
   }
   log << "sent " << st.st_size << " bytes" << endl;
}


void signal_handler (int signal) {
   log << "signal_handler: caught " << strsignal (signal) << endl;
   switch (signal) {
      case SIGINT: case SIGTERM: throw cix_exit();
      default: break;
   }
}

int main (int argc, char**argv) {
   log.execname (basename (argv[0]));
   log << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   // signal_action (SIGINT, signal_handler);
   // signal_action (SIGTERM, signal_handler);
   int client_fd = stoi (args[0]);
   log << "starting client_fd " << client_fd << endl;
   try {
      accepted_socket client_sock (client_fd);
      log << "connected to " << to_string (client_sock) << endl;
      for (;;) {
         cix_header header;
         recv_packet (client_sock, &header, sizeof header);
         log << "received header " << header << endl;
         switch (header.cix_command) {
            case CIX_LS:
               reply_ls (client_sock, header);
               break;
            case CIX_PUT:
               reply_put (client_sock, header);
               break;
            case CIX_RM:
               reply_rm (client_sock, header);
               break;
            case CIX_GET:
               reply_get (client_sock, header);
               break;
            default:
               log << "invalid header from client" << endl;
               log << "cix_nbytes = " << header.cix_nbytes << endl;
               log << "cix_command = " << header.cix_command << endl;
               log << "cix_filename = " << header.cix_filename << endl;
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

