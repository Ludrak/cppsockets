#pragma once

#include <string>
#include <queue>

// later rename to BufferedIO
class BufferedIO
{
    public:
        /* RECEIVING */
        // appends the receive buffer with the provided buffer
        void        appendRecvBuffer(const uint8_t* data_strip, const size_t size);
        // clears the first n_bytes of the receive buffer, if n_bytes is <0 it clears the whole buffer
        void        clearRecvBuffer(const size_t n_bytes = -1);
        // returns the whole receive buffer
        std::string getRecvBuffer() const;
        // returns the size of the receive buffer
        size_t      getRecvSize() const;

        /* SENDING */
        // appends a new buffer to be sent 
        void        appendSendBuffer(const uint8_t* data_strip, const size_t size);
        // trims the last buffer to be sent by n_bytes, in case the buffer could'nt be sent entierly
        // if n_bytes is <0 it clears the whole buffer
        void        clearSendBuffer(const size_t n_bytes = -1);
        // pops the last buffer to be sent, this does not clear out the contained buffer
        std::string getSendBuffer() const;
        // returns the size of the send buffer
        size_t      getSendSize() const;
    
    protected:
        std::string                 _recv_buffer;
        std::queue<std::string>     _send_queue;
};