
#pragma once

#include <sys/poll.h>
#include <vector>
#include <map>
#include <string>


/* SocketsHandler implementation for poll() */
class SocketsHandler
{
    public:
        typedef std::map<int, struct pollfd>    fd_map_type;
        typedef std::vector<struct pollfd>      fd_vector_type;

        SocketsHandler() = default;

        // Adds the socket to the list of checked sockets
        int addSocket(int socket);

        // Deletes the socket from the list of checked sockets  
        int delSocket(const int socket);

        // Set the socket flag for reading for socket depending on selected
        int socketWantsRead(const int socket, bool selected);

        // Set the socket flag for writing for socket depending on selected
        int socketWantsWrite(const int socket, bool selected);

        // process a poll on the given sockets list
        bool   processPoll(const int timeout_ms = -1);

        // event struct to interface socket handlers return values
        struct socket_event
        {
            int socket;
            int events;
            
            socket_event(int sock, int ev) : socket(sock), events(ev) {}

            socket_event() : socket(-1), events(-1) {}
        };

        // macros to use the event struct
        #define EV_IS_SOCKET(ev, sock)    (ev.socket == (sock))
        #define EV_IS_READABLE(ev) ((ev.events & POLLIN) != 0)
        #define EV_IS_WRITABLE(ev) ((ev.events & POLLOUT) != 0)
        #define EV_IS_ERROR(ev)    ((ev.events & (POLLERR | POLLNVAL)) != 0)
        #define EV_IS_END(ev)      (ev.events == -1 && ev.socket == -1)

        // Needs to be called in a loop after processPoll() until EV_IS_END(result) is reached
        // if the returned event is not the end, it contains data about the socket to handle and its event.
        #define _SOCKET_END        socket_event()
        socket_event    nextSocketEvent();

    
    private:
        fd_map_type     _poll_fds;
        fd_vector_type  _poll_fds_aligned;

        // id of previous poll, needed to reset nextSocketEvent counter,
        // can overflow, it just need to change value between each poll.
        size_t          _poll_id;
};
