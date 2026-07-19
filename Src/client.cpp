/******************************************************************************
 * Project : Multi-Client TCP Chat Server (TCP Chat Client)
 *
 * Description:
 * This program connects to a TCP chat server, sends user messages, and
 * receives messages from other connected clients.
 *
 ******************************************************************************/
#include <iostream>
using namespace std;

/*C style string handling*/
#include <cstring>

/*provides linux system calls*/
#include <unistd.h>

/*for working with IP addresses*/
#include <arpa/inet.h>

/*core socket api*/
#include <sys/socket.h>
#include <thread>
#include <string>

#define SUCCESS   0
#define FAILURE   1

void ReceiveMessages(int sock_fd)
{
   ssize_t bytes_recvd = 0;
   char buffer[1024];

   while (true)
   {
      memset(buffer, 0, sizeof(buffer));

      /* Receive Messages from Server */
      bytes_recvd = recv(sock_fd,
                         buffer,
                         sizeof(buffer) - 1,
                         0);

      if (bytes_recvd < 0)
      {
         cout << "ERR : Failed to receive reply." << endl;
         break;
      }

      if (bytes_recvd == 0)
      {
         cout << "--Server closed connection.--" << endl;
         break;
      }
      buffer[bytes_recvd] = '\0';

      cout << buffer << endl;
   }
   return;
}

int main()
{
   int iRetVal         = 0;
   int sock_fd         = 0;
   ssize_t bytes_sent  = 0;
   std::string message;
   std::string username;
   std::string full_message;

   /* server_addr struct stores the IP and port details of the server */
   struct sockaddr_in server_addr;
   
   memset(&server_addr, 0, sizeof(server_addr));

   /* creating a socket for the client */
   sock_fd = socket(AF_INET, SOCK_STREAM, 0);
   if (sock_fd < 0)
   {
      cout << "ERR : Socket creation failed." << endl;
      return FAILURE;
   }
   
   /* Populate server IP and Port details */
   server_addr.sin_family = AF_INET;
   server_addr.sin_port   = htons(8080);
   
   /* inet_pton() converts human-readable IP string to binary format. 
    * It stands for Internet Presentation to Network. 
    * AF_INET : Connecting to IPv4 address.
    * <ip address> : IP address in human readable format.
    * server_addr.sin_addr : stores the ip in binary 
    * 
    * Return Values : 1. 1  -> Success.
    *                 2. 0  -> Invalid IP string
    *                 3. -1 -> System Error
    * */
   iRetVal = inet_pton( AF_INET,
                        "127.0.0.1",
                        &server_addr.sin_addr );
   if (iRetVal <= 0)
   {
      cout << "ERR : Invalid server IP address." << endl;
      close(sock_fd);
      return FAILURE;
   }

   /* Connect to the server */
   iRetVal = connect( sock_fd,
                     (struct sockaddr *)&server_addr,
                     sizeof(server_addr) );

   if (iRetVal < 0)
   {
      cout << "ERR : Connection failed." << endl;
      close(sock_fd);
      return FAILURE;
   }

   cout << "--Connected to Server--" << endl;
   cout << "Enter your name:" << endl;
   getline(cin,username);
   
   std::thread receiver(ReceiveMessages, sock_fd);


   while(true)
   {
      getline(cin, message);

      full_message = username + " : " + message;
      /* Send msg to server */
      bytes_sent = send(sock_fd,
                        full_message.c_str(),
                        full_message.length(),
                        0);

      if (bytes_sent < 0)
      {
         cout << "ERR : Failed to send message." << endl;
         close(sock_fd);
         return FAILURE;
      }
   }
   receiver.join();
   
   /* Closing socket before exiting */
   close(sock_fd);
   return SUCCESS;
}
