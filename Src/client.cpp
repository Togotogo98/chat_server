/******************************************************************************
 * Project : Multi-Client TCP Chat Server (Phase 1)
 *
 * Description:
 *   Basic TCP client that:
 *     1. Creates a socket
 *     2. Connect to server
 *     3. Send data to server
 *     4. Receive a reply
 *     5. Close socket 
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

#define SUCCESS   0
#define FAILURE   1


int main()
{
   int iRetVal         = 0;
   int sock_fd         = 0;
   ssize_t bytes_sent  = 0;
   ssize_t bytes_recvd = 0;
   
   const char *msg = "Hello Server!";
   char buffer[1024];

   /* server_addr struct stores the IP and port details of the server */
   struct sockaddr_in server_addr;
   
   memset(&server_addr, 0, sizeof(server_addr));
   memset(buffer, 0, sizeof(buffer));
   
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
   
   /* Send msg to server */
   bytes_sent = send(sock_fd,
                     msg,
                     strlen(msg),
                     0);

   if (bytes_sent < 0)
   {
      cout << "ERR : Failed to send message." << endl;
      close(sock_fd);
      return FAILURE;
   }

   cout << "Message sent to server." << endl;
   
   /* Receive Acknowledgement from Server */
   bytes_recvd = recv(sock_fd,
                   buffer,
                   sizeof(buffer) - 1,
                   0);

   if (bytes_recvd < 0)
   {
      cout << "ERR : Failed to receive reply." << endl;
      close(sock_fd);
      return FAILURE;
   }

   if (bytes_recvd == 0)
   {
      cout << "--Server closed connection.--" << endl;
      close(sock_fd);
      return SUCCESS;
   }

   buffer[bytes_recvd] = '\0';

   cout << "Server : " << buffer << endl;
   
   /* Closing socket before exiting */
   close(sock_fd);
   return SUCCESS;
}
