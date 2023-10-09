#pragma once

#include <cstdlib>
#include <sys/socket.h>
#include "common/protocols/protocol_list.hpp"



class ProtocolBase
{
    public:
        virtual ~ProtocolBase() {}

        // Generates a socket of the type that should be used for this protocol
        // returns:
        // -1           : socket failed
        // >0           : new socket file descriptor
        virtual int            generateSocket() const = 0;
        
        // you must specify a data_buffer at least as long as the specified size (in bytes)
        // receiveMethod will fill the data_buffer with the received data on socket
        // returns:
        // -1   : an error has occured, disconnect the client
        //  0   : the client disconnected
        // >0   : received data
        virtual int             receiveMethod(const int socket, void *data_buffer, const size_t size) = 0;

        // you must specify a data array at least as long as the specified size (in bytes)
        // sendMethod will send the content pointed by data to the specified socket
        // returns:
        // -1   : an error has occured, the data was not sent
        //  0   : send has been able to send the whole provided data, remove clientWantsWrite from SocketHandler
        // >0   : send did not sent the data entierly, the size of the sent data is returned, dont remove clientWantsWrite from SocketHandler
        virtual int             sendMethod(const int socket, const void *data, const size_t size) = 0;

        // will try to accept a new incomming connection from the endpoint_socket
        // addr_buffer needs to be at least as long as the value pointed by addr_size
        // addr_buffer and addr_size will be filled by the sockaddr struct receive
        // TODO: addr size must be managed along with socket family types (i.e. sockaddr_in vs sockaddr_in6)
        //       THIS METHOD SHOULD ABSTRACT ALL SOCKADDR TYPES
        // returns:
        // -1           : an error has occured, unable to accept the client
        // >0           : accepted client socket file descriptor
        virtual int             acceptMethod(const int endpoint_socket, void *addr_buffer, socklen_t *addr_size) = 0;
};

template<Protocol P>
class ProtocolMethod
{};

