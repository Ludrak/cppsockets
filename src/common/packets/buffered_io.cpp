
#include "common/packets/buffered_io.hpp"

/* RECEIVING */
// appends the receive buffer with the provided buffer
void        BufferedIO::appendRecvBuffer(const uint8_t* data_strip, const size_t size)
{
    this->_recv_buffer += std::string(reinterpret_cast<const char*>(data_strip), size);
}

// clears the first n_bytes of the receive buffer, if n_bytes is <0 it clears the whole buffer
void        BufferedIO::clearRecvBuffer(const size_t n_bytes)
{
    if (this->_recv_buffer.empty())
        return ;
    
    if (n_bytes <= 0 || n_bytes >= this->_recv_buffer.size())
        this->_recv_buffer.clear();
    else 
        this->_recv_buffer.substr(0, n_bytes);
}

// returns the whole receive buffer
std::string BufferedIO::getRecvBuffer() const
{
    return (this->_recv_buffer);
}

// returns the size of the receive buffer
size_t      BufferedIO::getRecvSize() const
{
    return (this->_recv_buffer.size());
}


/* SENDING */
// appends a new buffer to be sent 
void        BufferedIO::appendSendBuffer(const uint8_t* data_strip, const size_t size)
{
    if (size == 0)
        return ;
    this->_send_queue.push(std::string(reinterpret_cast<const char*>(data_strip), size));
}

// trims the last buffer to be sent by n_bytes, in case the buffer could'nt be sent entierly
// if n_bytes is <0 it clears the whole buffer
void        BufferedIO::clearSendBuffer(const size_t n_bytes)
{
    if (n_bytes <= 0 || n_bytes >= this->_send_queue.front().size())
        this->_send_queue.pop();
    else
        this->_send_queue.front().substr(0, n_bytes);
}

// pops the last buffer to be sent, this does not clear out the contained buffer
std::string BufferedIO::getSendBuffer() const
{
    if (this->_send_queue.empty())
        return ("");
    else
        return (this->_send_queue.front());
}

// returns the size of the send buffer
size_t      BufferedIO::getSendSize() const
{
    if (this->_send_queue.empty())
        return (0);
    else
        return (this->_send_queue.front().size());
}
