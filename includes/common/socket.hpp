#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <string>
#include <errno.h>
#include "common/protocols/protocol.hpp"


class Socket
{
    public:
        // generates a socket using the ProtocolMethod
        Socket(const ProtocolBase* method);

        // wraps an already existing socket
        Socket(const int socket, const sa_family_t family);

        int     getSocket() const;
        
        int     getFamily() const;

        void    close();
    
    
    private:
        int         _socket;
        sa_family_t _family;
};
