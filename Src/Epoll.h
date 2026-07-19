#ifndef EPOLL_H
#define EPOLL_H

#include <unistd.h>
#include <sys/epoll.h>

class Epoll
{
private:
    int fd;

public:

    Epoll()
        : fd(-1)
    {
    }

    ~Epoll()
    {
        if (fd >= 0)
        {
            close(fd);
        }
    }

    Epoll(const Epoll&) = delete;
    Epoll& operator=(const Epoll&) = delete;

    Epoll(Epoll&& other)
        : fd(other.fd)
    {
        other.fd = -1;
    }

    Epoll& operator=(Epoll&& other)
    {
        if (this != &other)
        {
            if (fd >= 0)
                close(fd);

            fd = other.fd;
            other.fd = -1;
        }

        return *this;
    }

    bool Create()
    {
        fd = epoll_create1(0);
        return fd >= 0;
    }

    bool Add(int socket_fd,
             epoll_event& event)
    {
        return epoll_ctl(fd,
                     EPOLL_CTL_ADD,
                     socket_fd,
                     &event) >= 0;
    }

    bool Remove(int socket_fd)
    {
        return epoll_ctl(fd,
                     EPOLL_CTL_DEL,
                     socket_fd,
                     nullptr) >= 0;
    }

    int Wait(epoll_event events[],
             int max_events,
             int timeout)
    {
        return epoll_wait(fd,
                      events,
                      max_events,
                      timeout);
    } 

    int GetFD() const
    {
        return fd;
    }

    bool IsValid() const
    {
        return fd >= 0;
    }
};
#endif
