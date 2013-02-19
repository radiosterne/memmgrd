/*
 *@project memmgrd
 *@file mem_list_node.h
 *@brief Definition of mem_list_node for use in the mem_list_mgr class
 *@author Andrew Kurnoskin
 *@date 06/12/2012
 */

#ifndef MEM_LIST_NODE_H
#define MEM_LIST_NODE_H

#include "common_definitions.h"
#include <stdexcept>
#include <string>

/*
 *@brief A node of singly-linked list representing a block of memory
 *
 *@note All methods and fields are safely public, because only mem_list_mgr class uses them
 */
class mem_list_node : public array_interface
{
public:
    /*
     *@brief A simple constructor. Initializes fields with corresponding parameters
     */
    mem_list_node(char* begin, unsigned long size, bool is_empty);

    /*
     *@brief A memory-access operator. The only method actually available from outside the mem_list_mgr class
     *       (through interface)
     *@throws std::logic_error if the node is set as empty
     *@throws std:invalid_argument if location is out of the node range
     */
    char& operator[](unsigned long location) throw (std::logic_error,std::invalid_argument);

    /*
     *@brief A list pointer
     */
    mem_list_node* next;

    /*
     *@brief A pointer to the memory block. Is const due to overall logic
     */
    char *const begin;

    /*
     *@brief A size of the memory block
     */
    unsigned long size;

    /*
     *@brief Flag depicting if the node is in use
     */
    bool is_empty;
};

#endif // MEM_LIST_NODE_H
