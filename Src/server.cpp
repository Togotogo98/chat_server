/******************************************************************************
 * Project : Multi-Client TCP Chat Server (Phase 1)
 *
 * Description:
 *   Basic TCP server that:
 *     1. Creates a socket
 *     2. Binds to port 8080
 *     3. Listens for one client
 *     4. Accepts a connection
 *     5. Receives a message
 *     6. Sends an acknowledgement
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


int main ()
{
   int iRetVal          = 0;
   int server_fd        = 0;
   int client_fd        = 0;
   socklen_t client_len = 0;
   ssize_t bytes_recvd  = 0;
   ssize_t bytes_sent   = 0;
   
   /* Buffer is created to store the message from client */
   char buffer[1024];
   /* Reply message for acknowledment */
   const char *pReply = "Message received.";
   
   /* sockaddr_in is a struct that contains fields for IP, port and address
    * family - all used to initialise the socket.
    *
    * sockaddr_in  : struct for IPv4
    * sockaddr_in6 : struct for IPv6
    * sockaddr     : generic struct - bind expects this
    * */
   struct sockaddr_in server_addr;
   struct sockaddr_in client_addr;
   
   /* Initialise all bytes to zero */
   memset(&server_addr, 0, sizeof(server_addr));
   memset(&client_addr, 0, sizeof(client_addr));
   memset(buffer, 0, sizeof(buffer));
   
   
   
   /* socket() creates a socket and returns a file descriptor - a number to
    * identify the socket.
    * AF_INET     : Address family IPv4 (INET6 is IPv6)
    * SOCK_STREAM : Communicate by TCP
    * 0 : Informs Linux to use the default TCP protocol
    * */
   server_fd = socket(AF_INET, SOCK_STREAM, 0);
   if (server_fd < 0)
   {
      cout << "ERR : Socket creation failed." << endl;
      return FAILURE;
   }

   /* Populate the server_addr struct with socket IP and port details */
   
   server_addr.sin_family        = AF_INET;
   
   /* htons() twiddles host's little-endian number to big-endian for network */
   server_addr.sin_port          = htons(8080);
   
   /* INADDR_ANY : Accept connections on any network interface.*/
   server_addr.sin_addr.s_addr   = INADDR_ANY;



   /* Bind the socket */
   iRetVal = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
   if ( iRetVal < 0 )
   {
      cout << "ERR : Bind failed." << endl;
      close(server_fd);
      return FAILURE;
   }
   cout << "--Socket Bound to Port 8080--" << endl;

   /* listen() puts the server socket into a passive waiting state and starts
    * maintaining a queue of incoming connection requests.
    * backlog = 5 : Allows up to 5 clients waiting in queue then reject new ones.
    * Note : listen does not block the program execution, it only queues the
    *        incoming requests.
    * */
   iRetVal = listen(server_fd, 5);
   if ( iRetVal < 0 )
   {
      cout << "ERR : Listen failed." << endl;
      close(server_fd);
      return FAILURE;
   }
   cout << "--Listening on port 8080--" << endl;

   
   /* Prepare to accept clients */
   client_len = sizeof(client_addr);

   /* accept() takes one pending connection from the listen queue and creates
    * a new socket dedicated to that client.
    * client_addr struct : contains client's IP and port
    * Note : accept pauses the server, waits for a client to connect before
    *        continuing execution.
    * */
   client_fd = accept(server_fd,
                     (struct sockaddr *)&client_addr,
                     &client_len);

   if (client_fd < 0)
   {
      cout << "ERR : Accept failed." << endl;
      close(server_fd);
      return FAILURE;
   }

   cout << "--Client Connected--" << endl;

   /* recv() receives data from the client socket into buffer.
    * 
    * Note : The return type of recv() is ssize_t because
    *        it returns a size in number of bytes or -1 for
    *        error. POSIX uses the signed size type or
    *        ssize_t to represent the negative values.
    * */

   bytes_recvd = recv( client_fd,
                       buffer,
                       sizeof(buffer) - 1,
                       0 );

   if ( bytes_recvd < 0 )
   {
      cout << "ERR : Failed to receive message." << endl;
      close(client_fd);
      close(server_fd);
      return FAILURE;
   }

   if ( bytes_recvd == 0 )
   {
      cout << "--Client disconnected.--" << endl;
      close(client_fd);
      close(server_fd);
      return SUCCESS;
   }

   buffer[bytes_recvd] = '\0';

   cout << "Client : " << buffer << endl;
   
   /* send() is to send data to client */
   bytes_sent = send( client_fd,
                      pReply,
                      strlen(pReply),
                      0 );
   if (bytes_sent < 0)
   {
      cout << "ERR : Failed to send message." << endl;
      close(client_fd);
      close(server_fd);
      return FAILURE;
   }
   
   cout << "--Reply sent to client.--" << endl;
   
   /* Closing sockets before exiting program */
   close(client_fd);
   close(server_fd);
   return SUCCESS;
}
