#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <string>
#include <errno.h>


class TcpSocket
{
    public:
        TcpSocket(const int socket, const sa_family_t family = AF_INET);

        TcpSocket(const sa_family_t family = AF_INET);


        int     getSocket() const;
        
        int     getFamily() const;

        void    close();
    
    
    private:
        int         _socket;
        sa_family_t _family;
};
