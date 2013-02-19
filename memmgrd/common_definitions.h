/*
 *@project memmgrd
 *@file common_definitions.h
 *@brief Interfaces and structs definitions for other classes to use
 *@author Andrew Kurnoskin
 *@date 06/12/2012
 *@TODO node_descriptor
 */

#ifndef COMMON_DEFINITIONS_H
#define COMMON_DEFINITIONS_H

//Forward definition for allocation_query_element
class mem_list_node;


/*
 *@brief Allocation confirmation struct
 *@member size : size of allocated block
 *@member block_number : internal number of allocated block inside this socket_connection_handler
 *@member node : a pointer to allocated block
 */
struct allocation_query_element
{
    unsigned int size;
    unsigned int block_number;
    mem_list_node* node;
};

/*
 *@brief Simple interface of connection handler. Used in mem_query_element and daemon.
 */
class conn_handler_interface
{
public:
    virtual void allocation_success(allocation_query_element query) = 0;
};

/*
 *@brief Memory query struct
 *@member size : size of allocated block
 *@member block_number : internal number of allocated block inside this socket_connection_handler
 *@member initiator : a pointer to appropriate connection handler. Provides appropriate callback.
 */
struct mem_query_element
{
    unsigned int size;
    unsigned int block_number;
    conn_handler_interface* initiator;
};

/*
 *@brief Memory state descriptor struct
 *@member size : size of allocated block
 *@member is_empty : self-explanatory
 *@member next : a pointer to next descriptor in list
 */
struct node_descriptor
{
    unsigned int size;
    bool is_empty;
    node_descriptor* next;

};

/*
 *@brief Array interface implemented by mem_list_node
 */
class array_interface
{
public:
    virtual char& operator[](unsigned long location) = 0;
};


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

#endif // COMMON_DEFINITIONS_H
