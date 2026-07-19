#ifndef SOCKET_H
#define SOCKET_H

#include <unistd.h>

class Socket
{
private:
    int fd;

public:

    Socket()
        : fd(-1)
    {
    }

    ~Socket()
    {
        if (fd >= 0)
        {
            close(fd);
        }
    }

    Socket(const Socket&) = delete;

    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other)
        : fd(other.fd)
    {
        other.fd = -1;
    }

    Socket& operator=(Socket&& other)
    {
        if (this != &other)
        {
            if (fd >= 0)
            {
                close(fd);
            }

            fd = other.fd;
            other.fd = -1;
        }

        return *this;
    }

   /* socket() creates a socket and returns a file descriptor - a number to
    * identify the socket.
    * AF_INET     : Address family IPv4 (INET6 is IPv6)
    * SOCK_STREAM : Communicate by TCP
    * 0 : Informs Linux to use the default TCP protocol
    * */
    bool Create()
    {
        fd = socket(AF_INET, SOCK_STREAM, 0);
        return fd >= 0;
    }

    void SetNonBlocking()
    {
       /* fcntl() returns the current file status flags of the socket.
        * These flags represent the default settings Linux assigned when
        * the socket was created. Copying/Storing them in 'flags' so the existing
        * settings are preserved before adding O_NONBLOCK.
        * F_GETFL: GET - read, FL - file status flags.
        * */
        int flags = fcntl(fd, F_GETFL, 0);

        if (flags < 0)
           return;

	/* Keep all the existing socket settings, and enable non-blocking mode.*/
        flags |= O_NONBLOCK;

	/* Apply the updated flags to the socket.
         * F_SETFL: SET - set/apply updated flags.
         * */
        fcntl(fd, F_SETFL, flags);
    }

    bool Bind(const sockaddr_in& address)
    {
        return bind(fd,
                    (const sockaddr*)&address,
                    sizeof(address)) >= 0;
    }

   /* listen() puts the server socket into a passive waiting state and starts
    * maintaining a queue of incoming connection requests.
    * backlog = x : Allows up to x clients waiting in queue then reject new ones.
    * Note : listen does not block the program execution, it only queues the
    *        incoming requests.
    * */
    bool Listen(int backlog)
    {
        return listen(fd, backlog) >= 0;
    }

    int GetFD() const
    {
        return fd;
    }

    void SetFD(int socket_fd)
    {
        fd = socket_fd;
    }

    bool IsValid() const
    {
        return fd >= 0;
    }
};
#endif
