
#pragma once

#include <iostream>
#include <sys/select.h>
#include <vector>

#ifndef FD_COPY
#define FD_COPY(src, dst) std::memcpy(dst, src, sizeof(*dst));
#endif

/* SocketsHandler implementation for select() */
class SocketsHandler
{
    public:
        typedef std::vector<int>    fd_list_type;

        SocketsHandler();

        // Adds the socket to the list of checked sockets
        int		addSocket(const int socket);

        // Deletes the socket from the list of checked sockets
        int		delSocket(const int socket);

        // Set the socket flag for reading for socket depending on selected
        int		socketWantsRead(const int socket, bool selected);

        // Set the socket flag for writing for socket depending on selected
        int		socketWantsWrite(const int socket, bool selected);

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
        #define _SOCK_READ   0b0001
        #define _SOCK_WRITE  0b0010

        // macros to use the event struct
        #define EV_IS_SOCKET(ev, sock)    (ev.socket == (sock))
        #define EV_IS_READABLE(ev) ((ev.events & _SOCK_READ) != 0)
        #define EV_IS_WRITABLE(ev) ((ev.events & _SOCK_WRITE) != 0)
        #define EV_IS_ERROR(ev)    (false) // there is no way of checking errors on fd with select
        #define EV_IS_END(ev)      (ev.events == -1 && ev.socket == -1)

        // Needs to be called in a loop after processPoll() until EV_IS_END(result) is reached
        // if the returned event is not the end, it contains data about the socket to handle and its event.
        #define _SOCKET_END        socket_event()
        socket_event    nextSocketEvent();
    
    private:
        int _getnfds();

    
    private:
        // main fd sets
        fd_set          _read_fds;
        fd_set          _write_fds;

        // copy sets for polling
        fd_set          _selected_read_fds;
        fd_set          _selected_write_fds;

        // list of selected fds for processPoll
        fd_list_type    _selected_fds;

        // id of previous poll, needed to reset nextSocketEvent counter,
        // can overflow, it just need to change value between each poll.
        size_t          _poll_id;
};
