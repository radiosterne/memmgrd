/*
 *@project memmgrd
 *@file mem_list_mgr.h
 *@brief Definition of memory list manager class
 *@author Andrew Kurnoskin
 *@date 13/12/2012
 */

#ifndef MEM_LIST_MGR_H
#define MEM_LIST_MGR_H

#include "common_definitions.h"
#include "mem_list_node.h"
#include "logger.h"
#include "queue.h"
#include <stdexcept>


/*
 *@brief Provides memory-managing routines
 *
 *@thread Authomatically starts loop() thread after calling to create_thread(void* mlmgr)
 */
class mem_list_mgr
{
public:

    /*
     *@brief Initializes new manager object
     *@param not_zero size: size of memory block to allocate
     *@param not_zero log:pointer to a logging facility
     *
     *@throws std::invalid_argument if one of arguments was set to zero
     */
    mem_list_mgr(unsigned long size, logger* log) throw (std::invalid_argument);

    /*
     *@brief Frees all the memory allocated by this object
     */
    ~mem_list_mgr();

    /*
     *@brief Sets node as an empty one
     *@param not_zero nd: node to set as empty
     *
     *@throws std::invalid argument if node was set to zero
     */
    void free(array_interface* nd) throw (std::invalid_argument);

    /*
     *@brief Puts element to the query queue
     *@param query: an element to enqueue
     */
    void enqueue(mem_query_element query);

    /*
     *@brief A signal to finish the loop()
     */
    bool quit_signal;

    /*
     *@brief Returns a list descripting current memory state
     *
     *@note Quite a slow procedure blocking all the other memory list operations
     */
    node_descriptor* mem_list_state();

    /*
     *@brief Magic doorbell for loop() to try to do smth
     */
    void ring();

    /*
     *@brief Creates new memory list manager' thread and starts it
     */
    static void create_thread(void* mlmgr);

    /*
     *@brief Executes join on this object loop() thread
     */
    void join();
private:

    //-------------------------------------------------------------------------
    //Actualy memory-related stuff
    //Used to allocate query form queue
    bool allocate(mem_query_element query);
    mem_list_node* first_node;
    char* mem_block;
    queue<mem_query_element> query_queue;

    //-------------------------------------------------------------------------
    //Multithreading procedures

    //Actual proc to be start in thread
    static void* start_thread(void* melmgr);
    //Main object loop
    void loop();


    //-------------------------------------------------------------------------
    //Multithreading data

    //Blocks access to memory list
    pthread_mutex_t* memory_list_mutex;
    //Used to signal to loop() on changed state
    pthread_cond_t* state_changed;
    //Stores loop() thread
    pthread_t thread;

    //-------------------------------------------------------------------------
    //Logger
    logger* log;

};

#endif // MEM_LIST_MGR_H
