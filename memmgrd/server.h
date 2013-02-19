/*
 *@project memmgrd
 *@file server.h
 *@brief Definition and implementation of non-blocking thread-safe queue
 *@author Andrew Kurnoskin
 *@date 06/01/2013
 */

#ifndef SERVER_H
#define SERVER_H

#include "common_definitions.h"
#include "mem_list_mgr.h"
#include "socket_conn_handler.h"
#include "config_reader.h"
#include "keccak.h"

/*
 *@brief Main program logic class
 */
class server
{
public:
    /*
     *@brief Provides singleton functionality: returns  pointer to an instance
     *
     *@retval Returns a pointer to an instance
     */
    static server* get_instance();

    /*
     *@brief Kills an instance
     */
    static void kill_instance();
    /*
     *@brief Main server working routine
     */
    void loop();
private:
    server();
    ~server();
    logger* log;
    socket_conn_handler* first_node;
    mem_list_mgr* mgr;
    keccak* hash;
    pthread_t quit_listen_thread;
    static void* quit_thread(void* dmn);
    void quit_listen();
    bool quit_signal;
    pthread_mutex_t* quit_signal_mutex;
    pthread_cond_t* quit_signal_cond;
    config_reader config;
    static server* instance;
};


#endif //SERVER_H
