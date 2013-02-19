/*
 *@project memmgrd
 *@file mem_list_node.cpp
 *@brief Implementation of mem_list_node for use in the mem_list_mgr class
 *@author Andrew Kurnoskin
 *@date 06/12/2012
 */

#include "mem_list_node.h"


mem_list_node::mem_list_node(char* begin, unsigned long size, bool is_empty)
    : begin(begin), size(size), is_empty(is_empty), next(0)
{

}

char& mem_list_node::operator[](unsigned long location) throw (std::logic_error,std::invalid_argument)
{
    if(this->is_empty == 1)
        throw std::logic_error(std::string("Operator[] is called for an empty block!\0"));
    if(location >= this->size)
        throw std::invalid_argument(std::string("Operator[] is called with invalid argument!\0"));
    return *(this->begin + location);
}
