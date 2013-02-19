/*
 *@project memmgrtest
 *@file main.cpp
 *@brief Entry point for an test application. Takes up one memory block.
 *@author Andrew Kurnoskin
 *@date 06/01/2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>

//Definition of the socket path
#define SOCK_PATH "/home/radiosterne/tmp/memmgrd_socket"

//Message codes (copied from main project)
enum socket_message_codes
{
    //Normally daemon-recievable codes
    socket_connection_closed,
    socket_byte_sent,
    socket_byte_requested,
    socket_memory_requested,
    socket_memory_termination,
    socket_authenticate,
    socket_quit,
    socket_memory_map_requested,
    socket_passwd_byte,
    socket_passwd_end,

    //Normally daemon-sendable codes
    socket_memory_allocated,
    socket_memory_map_sent,
    socket_not_authenticated,
    socket_authenticated,
    socket_node_descriptor,
    socket_error
};

// Just opens a connection, asks user for a size of memory block and
// sends request to occupy it.
// BE WARY: DOES NO CHECKING, even of socket state, use at your own risk!
int main(void)
{
    int s, len;
    struct sockaddr_un remote;

    s = socket(AF_UNIX, SOCK_STREAM, 0);

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    connect(s, (struct sockaddr *)&remote, len);
    std::cout << "Connected to socket\n";
    unsigned int* buffer = new unsigned int[4];
    buffer[0] = socket_memory_requested;
    buffer[1] = 0;
    std::cout << "Input size of space needed:";
    std::cin >> buffer[2];
    send(s, buffer, 16, 0);
    read(s, buffer, 16);
    
    std::cout << "Input something to quit...";
    std::cin >> buffer[2];

	buffer[0] = socket_connection_closed;
    send(s, buffer, 16, 0);
    read(s, buffer, 16);
    close(s);
    std::cout << "socket closed\n";
    delete[] buffer;
    return 0;
}
