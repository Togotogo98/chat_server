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
