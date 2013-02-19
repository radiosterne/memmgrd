/*
 *@project memmgrd
 *@file main.cpp
 *@brief Application entry point.
 *@author Andrew Kurnoskin
 *@date 06/01/2013
 */


#include "server.h"

/*
 *@brief Gets server instance, runs it loop.
 *
 *@summary Entry point for memory manager daemon.
 *         For configuration instructions, see config_reader.h.
 *         For socket messages protocol, see socket_conn_handler.h/socket_conn_handler.cpp
 */
int main()
{
    server* s = server::get_instance();
    s->loop();
    server::kill_instance();
    return 0;
}

