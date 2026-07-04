/******************************************************************************
 * Project : Multi-Client TCP Chat Server
 *
 * Description:
 *   TCP-based client-server application that accepts client connections,
 *   processes client requests, and facilitates communication between multiple
 *   clients.
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

/*for creating threads*/
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <fcntl.h>
#include <cerrno>

#define SUCCESS   0
#define FAILURE   1

/* vector is a dynamic array. It resizes itself when 
 * new clients join. Here, connected_clients are array
 * of client socket file descriptors.
 * */
std::vector<int> connected_clients;
std::mutex clients_mutex;

void DisconnectClient( int client_socket )
{
   /* Remove client from the connected client list by using find().
    * It returns an iterator that points to the position of where
    * the element is in the vector.
    * erase() removes that value from that position from vector.
    * */
   {
      std::lock_guard<std::mutex> lock(clients_mutex);
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
      cout << "Connected clients: "
              << connected_clients.size()
              << endl;
   }

   /* closing socket */
   close(client_socket);
   cout << "Client Disconnected..." << endl;
   return;   
}

void BroadcastMessage(int sender_socket,
                      const char *msg,
                      size_t msg_len)
{
   ssize_t bytes_sent   = 0;
   
   std::lock_guard<std::mutex> lock(clients_mutex);

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
            cout << "ERR : Failed to send message." << endl;
            return;
         }
      }
   }
   return;
}

void HandleClient( int ClientSocket )
{
   ssize_t bytes_recvd  = 0;
   
   /* Buffer is created to store the message from client */
   char buffer[1024];
   
   /* recv() receives data from the client socket into buffer.
    * 
    * Note : The return type of recv() is ssize_t because
    *        it returns a size in number of bytes or -1 for
    *        error. POSIX uses the signed size type or
    *        ssize_t to represent the negative values.
    * */
   while (true)
   {
      memset(buffer, 0, sizeof(buffer));

      bytes_recvd = recv( ClientSocket,
                          buffer,
                          sizeof(buffer) - 1,
                          0 );

      if ( bytes_recvd < 0 )
      {
	 if ( errno == EAGAIN || errno == EWOULDBLOCK )
         {
            continue;
         }
	 else
         {
            cout << "ERR : Failed to receive message." << endl;
            break;
         }
      }

      if ( bytes_recvd == 0 )
      {
         break;
      }
   
      buffer[bytes_recvd] = '\0';

      cout << buffer << endl;

      BroadcastMessage(ClientSocket,
                       buffer,
                       bytes_recvd);

   }
   
   DisconnectClient(ClientSocket);
   return;   
}


int main ()
{
   int iRetVal          = 0;
   int server_fd        = 0;
   int client_fd        = 0;
   int flags            = 0;
   socklen_t client_len = 0;
   
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
   while(true)
   {
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
      
      /* fcntl() returns the current file status flags of the socket.
       * These flags represent the default settings Linux assigned when
       * the socket was created. Copying/Storing them in 'flags' so the existing
       * settings are preserved before adding O_NONBLOCK.
       * F_GETFL: GET - read, FL - file status flags.
       * */    
      flags = fcntl(client_fd, F_GETFL, 0);

      /* Keep all the existing socket settings, and enable non-blocking mode. */
      flags |= O_NONBLOCK;

      /* Apply the updated flags to the socket.
       * F_SETFL: SET - set/apply updated flags.
       * */
      fcntl(client_fd, F_SETFL, flags);

      /* lock_guard locks clients_mutex and automatically unlocks it once the 
       * block ends. push_back adds the new client to the vector clients' 
       * list. */
      {
         std::lock_guard<std::mutex> lock(clients_mutex);
	 connected_clients.push_back(client_fd);
      
         cout << "Connected clients: "
              << connected_clients.size()
              << endl;
      }

      std::thread clientThread(HandleClient, client_fd);
      clientThread.detach();
   }
      
   /* Closing sockets before exiting program */
   close(server_fd);
   return SUCCESS;
}
