#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <string>
#include <errno.h>
#include "common/side.hpp"

// predefinition for Socket
template<Side S>
class IProtocolMethod;

// levels for SOL_SOCKET
// DEBUG=SO_DEBUG,             // enables recording of debugging information
// REUSEADDR=SO_REUSEADDR,     // enables local address reuse
// REUSEPORT=SO_REUSEPORT,     // enables duplicate address and port bindings
// KEEPALIVE=SO_KEEPALIVE,     // enables keep connections alive
// DONTROUTE=SO_DONTROUTE,     // enables routing bypass for outgoing messages
// LINGER=SO_LINGER,           // linger on close if data present
// BROADCAST=SO_BROADCAST,     // enables permission to transmit broadcast messages
// OOBINLINE=SO_OOBINLINE,     // enables reception of out-of-band data in band
// SNDBUF=SO_SNDBUF,           // set buffer size for output
// RCVBUF=SO_RCVBUF,           // set buffer size for input
// SNDLOWAT=SO_SNDLOWAT,       // set minimum count for output
// RCVLOWAT=SO_RCVLOWAT,       // set minimum count for input
// SNDTIMEO=SO_SNDTIMEO,       // set timeout value for output
// RCVTIMEO=SO_RCVTIMEO,       // set timeout value for input
// TYPE=SO_TYPE,               // get the type of the socket (get only)
// ERROR=SO_ERROR,             // get and clear error on the socket (get only)
// NOSIGPIPE=SO_NOSIGPIPE,     // do not generate SIGPIPE, instead return EPIPE
// NREAD=SO_NREAD,             // number of bytes to be read (get only)
// NWRITE=SO_NWRITE,           // number of bytes written not yet sent by the protocol (get only)
// LINGER_SEC=SO_LINGER_SEC    // linger on close if data present with timeout in seconds


template<Side S>
class Socket
{
    public:
        // generates a socket using the ProtocolMethod
        Socket(const IProtocolMethod<S>* method)
        : _socket(method->generateSocket())
        {}

        // wraps an already existing socket
        Socket(const int socket, const sa_family_t family)
        : _socket(socket), _family(family)
        {}

        int     getSocket() const
            { return (this->_socket); }
        
        int     getFamily() const
            { return (this->_family); }

        void    close()
        { 
            ::close(this->_socket);
            this->_socket = -1;
        }

        void    setOption(int level, int option_name, void* option_value)
        {
            ::setsockopt(this->_socket, level, option_name, option_value);
        }
    
    
    private:
        int         _socket;
        sa_family_t _family;
};
