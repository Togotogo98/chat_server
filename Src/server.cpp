/*********************************************************************************
 * Project : Multi-Client TCP Chat Server
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
 *********************************************************************************/

/* ================================================================================
 *     HEADERS
 * ================================================================================
 */
#include <iostream>
using namespace std;
#include <cstring>

/*provides linux system calls*/
#include <unistd.h>

/*for working with IP addresses*/
#include <arpa/inet.h>

/*core socket api*/
#include <sys/socket.h>

#include <sys/epoll.h>

#include <vector>
#include <algorithm>
#include <fcntl.h>
#include <cerrno>

#include "Socket.h"
#include "Epoll.h"
/* ================================================================================
 *     MACROS & GLOBAL VARIABLES
 * ================================================================================
 */

#define SUCCESS     0
#define FAILURE     1
#define PORT        8080
#define MAX_EVENTS  64
#define BUFFER_SIZE 1024

/* vector is a dynamic array. It resizes itself when 
 * new clients join. Here, connected_clients are array
 * of client socket file descriptors.
 * */
std::vector<int> connected_clients;

/* ================================================================================
 *     FUNCTION DEFINATIONS
 * ================================================================================
 */
void SetNonBlocking(int socket_fd)
{
   int flags = 0;
   
   flags = fcntl(socket_fd, F_GETFL, 0);
   if (flags < 0)
   {
      cout << "ERR : Failed to get socket flags." << endl;
      return;
   }
   
   flags |= O_NONBLOCK;

   if (fcntl(socket_fd, F_SETFL, flags) < 0)
   {
      cout << "ERR : Failed to set socket as non-blocking." << endl;
   }
}

void DisconnectClient( Epoll& epoll,
                       int client_socket )
{
   /* Remove socket from epoll monitoring.*/
   if ( !epoll.Remove(client_socket)  )
   {
      cout << "ERR : Failed to remove socket from epoll." << endl;
   }
   
   /* Remove client from the connected client list by using find().
    * It returns an iterator that points to the position of where
    * the element is in the vector.
    * erase() removes that value from that position from vector.
    * */
   auto iterator = std::find(connected_clients.begin(),
                             connected_clients.end(),
		             client_socket);
   /* if the element is not found, find() returns the position
    * of one after the last element.
    * */
   if( iterator != connected_clients.end() )
   {
      connected_clients.erase(iterator);
   }
   
   /* closing socket */
   close(client_socket);
   cout << "Client [" << client_socket
        << "] Disconnected." << endl;
   cout << "Connected clients: "
        << connected_clients.size()
        << endl;
   return;   
}

void BroadcastMessage(int sender_socket,
                      const char *msg,
                      size_t msg_len)
{
   ssize_t bytes_sent   = 0;
   
   for ( const auto& rcvr_client : connected_clients )
   {
      if ( rcvr_client != sender_socket )
      {
         bytes_sent = send( rcvr_client,
                            msg,
                            msg_len,
                            0 );
         if (bytes_sent < 0)
         {
            cout << "ERR : Failed to send message" 
                 << " to client [" << rcvr_client << "]."
                 << endl;
            continue;
         }
      }
   }
   return;
}

/* ================================================================================
 *                                M A I N
 * ================================================================================
 */

int main ()
{
   int client_fd        = 0;
   char buffer[BUFFER_SIZE];
   socklen_t client_len = 0;
   ssize_t bytes_recvd  = 0;
   Socket server_socket;
   Epoll epoll;
   int ready_sock_nmbr  = 0;
   int current_sock_fd  = 0;
   struct epoll_event event;
   struct epoll_event ready_events[MAX_EVENTS];
   

   /* sockaddr_in is a struct that contains fields for IP, port and address
    * family - all used to initialise the socket.
    *
    * sockaddr_in  : struct for IPv4
    * sockaddr_in6 : struct for IPv6
    * sockaddr     : generic struct - bind expects this
    * */
   sockaddr_in server_addr{};
   sockaddr_in client_addr{};
   
   /* Create a server socket. */
   if ( !server_socket.Create() )
   {
      cout << "ERR : Socket creation failed." << endl;
      return FAILURE;
   }

   /* Create an epoll instance. */
   if (!epoll.Create())
   {
      cout << "ERR : Failed to create epoll instance." << endl;
      return FAILURE;
   }

   /* Populate the server_addr struct with socket IP and port details.
    * htons() : It twiddles host's little-endian no. to big-endian for network.
    * INADDR_ANY : Accept connections on any network interface.*/
   server_addr.sin_family        = AF_INET;
   server_addr.sin_port          = htons(PORT);
   server_addr.sin_addr.s_addr   = INADDR_ANY;

   /* Bind the socket */
   if ( !server_socket.Bind(server_addr) )
   {
      cout << "ERR : Bind failed." << endl;
      return FAILURE;
   }

   /* Listen to socket */
   if ( !server_socket.Listen(5) )
   {
      cout << "ERR : Listen failed." << endl;
      return FAILURE;
   }

   /* Set server to Non-Blocking */
   server_socket.SetNonBlocking();

   /* EPOLLIN - event to indicate an fd is ready to read */
   event.events = EPOLLIN;
   event.data.fd = server_socket.GetFD();

   /* Register the Server Socket */
   if (!epoll.Add(server_socket.GetFD(), event))
   {
      cout << "ERR : Failed to add server socket to epoll." << endl;
      return FAILURE;
   }

   cout << "--Server Initialized--" << endl;

   
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
      /* Wait until there is ready events among sockets in epoll */
      ready_sock_nmbr = epoll.Wait(ready_events,
                                   MAX_EVENTS,
                                   -1); 
      if ( ready_sock_nmbr < 0 )
      {
         cout << "ERR : epoll_wait loop failure. " << endl;
	 return FAILURE;
      }

      /* Iterate through the array of ready sockets. */
      for ( int nmbr = 0; nmbr < ready_sock_nmbr; ++nmbr )
      {
         current_sock_fd = ready_events[nmbr].data.fd;

	 /*Server Socket will handle connection requests*/
	 if ( current_sock_fd == server_socket.GetFD() )
         {
            while (true)
            {
               client_len = sizeof(client_addr);

              /* accept() takes one pending connection from the listen queue 
	       * and creates a new socket dedicated to that client.
               * client_addr struct : contains client's IP and port
               * Note : accept pauses the server, waits for a client
               *        to connect before continuing execution.
               * */
               client_fd = accept(server_socket.GetFD(),
                                  (struct sockaddr *)&client_addr,
                                  &client_len);

               if (client_fd < 0)
               {
                  if ( errno == EAGAIN || errno == EWOULDBLOCK )
		  {   break; }

                  cout << "ERR : Accept failed." << endl;
                  break;
               }

              /* Set client socket as Non-Blocking. A Non-blocking socket returns
               * immediatly if there is no data present (eg: Client doesn't type
               * a message for a long time). A blocking socket keeps waiting. */
               SetNonBlocking(client_fd);

	       /* Register client to epoll. */
	       event.events = EPOLLIN;
               event.data.fd = client_fd;

               if (!epoll.Add(client_fd, event))
               {
                  cout << "ERR : Failed to add client socket to epoll." << endl;
		  close(client_fd);
		  continue;
               }

               connected_clients.push_back(client_fd);

               cout << "Connected clients: "
                    << connected_clients.size()
                    << endl;
            }//while end
         }
	 else
         {
            memset(buffer, 0, sizeof(buffer));

            /* recv() receives data from the client socket into buffer.
             *
             * Note : The return type of recv() is ssize_t because
             *        it returns a size in number of bytes or -1 for
             *        error. POSIX uses the signed size type or
             *        ssize_t to represent the negative values.
             * */

	    bytes_recvd = recv( current_sock_fd,
                                buffer,
                                sizeof(buffer) - 1,
                                0 );

            if ( bytes_recvd == 0 || 
                 (bytes_recvd < 0 && errno != EAGAIN))
            {
	       DisconnectClient(epoll, current_sock_fd);
	       continue;
            }

            buffer[bytes_recvd] = '\0';
            
	    /* Print msg on server.*/
	    cout << buffer << endl;

            BroadcastMessage(current_sock_fd,
                             buffer,
                             bytes_recvd); 
	 }
      }
   } 
   return SUCCESS;
}
