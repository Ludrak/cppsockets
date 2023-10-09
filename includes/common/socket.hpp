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
        template<Protocol P>
        Socket(const ProtocolMethod<P>& method)
        {
            this->_socket = method.generateSocket();
        }

        Socket(const int socket, const sa_family_t family = AF_INET);

        Socket(const sa_family_t family = AF_INET);


        int     getSocket() const;
        
        int     getFamily() const;

        void    close();
    
    
    private:
        int         _socket;
        sa_family_t _family;
};
