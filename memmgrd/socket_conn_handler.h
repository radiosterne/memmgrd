/*
 *@project memmgrd
 *@file socket_conn_handler .h
 *@brief Definition of socket_conn_handler class
 *@author Andrew Kurnoskin
 *@date 04/12/2012
 */

#ifndef SOCKET_CONN_HANDLER_H
#define SOCKET_CONN_HANDLER_H


#include "common_definitions.h"
#include "queue.h"
#include "mem_list_mgr.h"
#include "keccak.h"
#include <pthread.h>
#include <sys/socket.h>
#include <vector>

/*
 *@brief A handler of every incoming connection
 *
 *@thread Authomatically starts loop() thread after calling to create_thread(void* conn_handler)
 */
class socket_conn_handler : public conn_handler_interface
{
public:

    /*
     *@brief. Initializes fields with corresponding values. BE ULTRA WARY: Does no checking of any kind!
     */
    socket_conn_handler(pthread_mutex_t* qsm, pthread_cond_t* qsc, mem_list_mgr* mgr, logger* log, keccak* hash, int socket);

    /*
     *@brief Main handler function. Listens on the socket, providing overall socket functionality, checks queue for succesful allocations
     */
    void loop();
    /*
     *@brief A callback function for mem_list_mgr
     *@param query : a query element to put in the queue. Represents one succesfully allocated memory block.
     */
    void allocation_success(allocation_query_element query);

    /*
     *@brief A signal for loop() to stop it's activity
     */
    bool quit_signal;

    /*
     *@brief List-organizing routine. Managed from outside of this class.
     */
    socket_conn_handler* next;

    /*
     *@brief Creates and starts new thread, the one from conn_handler
     *@param conn_handler : a pointer to socket_conn_handler object, which loop() needs to be started
     */
    static void create_thread(void* conn_handler);

    /*
     *@brief Waits for main object's thread to exit
     */
    void join();
private:

    //-------------------------------------------------------------------------
    //Memory-related routines

    //Memory manager for handling allocation and freeing queries
    mem_list_mgr* mgr;

    //Array of nodes' pointers
    std::vector<array_interface*> nodes_array;

    //-------------------------------------------------------------------------
    //Query of allocated elements waiting to be processed
    queue<allocation_query_element> allocation_query;

    //A socket file descriptor
    const int socket;


    //-------------------------------------------------------------------------
    //Main password authentication method
    keccak* hash;

    //Indicates authentication state
    bool is_authenticated;


    //-------------------------------------------------------------------------
    //Linux send() wrapper with logging and failure-checking functionality
    bool safe_send(unsigned int* buffer);


    //-------------------------------------------------------------------------
    //Inner threading stuff
    static void* start_thread(void* conn_handler);
    pthread_t thread;


    //-------------------------------------------------------------------------
    //Logging interface
    logger * const log;
    void log_format(std::string message);


    //-------------------------------------------------------------------------
    //Server's synchronization routines
    pthread_mutex_t* quit_signal_mutex;
    pthread_cond_t* quit_signal_cond;
};

#endif //SOCKET_CONN_HANDLER_H
