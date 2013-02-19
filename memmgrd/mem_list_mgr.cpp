/*
 *@project memmgrd
 *@file mem_list_mgr.cpp
 *@brief Implementation of memory list manager class
 *@author Andrew Kurnoskin
 *@date 13/12/2012
 */

#include "mem_list_mgr.h"
#include <sstream>
#include <unistd.h>

mem_list_mgr::mem_list_mgr(unsigned long size, logger* log) throw (std::invalid_argument)
{
    //Arguments validation
    if(log == 0)
        throw std::invalid_argument("mem_list_mgr::mem_list_mgr() : log was set to zero!");
    if(size == 0)
        throw std::invalid_argument("mem_list_mgr::mem_list_mgr() : size was set to 0!");

    this->log = log;

    this->mem_block = new char[size];
    this->first_node = new mem_list_node(this->mem_block, size, 1);

    this->quit_signal = 0;

    this->memory_list_mutex = new pthread_mutex_t;
    this->state_changed = new pthread_cond_t;

    pthread_mutex_init(this->memory_list_mutex, 0);
    pthread_cond_init(this->state_changed, 0);

    this->log->add_entry("Mem_list_mgr constructor finished!");
}

mem_list_mgr::~mem_list_mgr()
{
    mem_list_node* current  = this->first_node;
    mem_list_node* temp = 0;

    //Deleting all the nodes
    while(current != 0)
    {
        temp = current;
        current = current->next;
        delete temp;
    }

    pthread_mutex_destroy(this->memory_list_mutex);
    pthread_cond_destroy(this->state_changed);

    delete this->memory_list_mutex;
    delete this->state_changed;
    delete[] this->mem_block;

    this->log->add_entry("Mem_list_mgr destructor finished!");
}

bool mem_list_mgr::allocate(mem_query_element query)
{
    bool scan_completed = false;
    unsigned long delta = 100000000;
    mem_list_node* winner = 0;
    mem_list_node* current = this->first_node;

    //Finding the most fitting free node
    while(!scan_completed)
    {
        if(current->is_empty && (current->size >= query.size) && ((current->size - query.size) < delta))
        {
            winner = current;
            delta = current->size - query.size;
        }
        if(current->next != 0)
            current = current->next;
        else
            scan_completed = true;
    }

    //Logging scan results
    std::ostringstream msg;
    msg << "mem_list_mgr::allocate() : scan completed! query size is " << query.size << ", winner size is";
    (winner == 0) ? msg << "undefined" : msg << winner->size;
    this->log->add_entry(msg.str());

    if(winner != 0)
    {
        if(winner->size - query.size != 0)
        {
            //Creating new list node
            mem_list_node* second = new mem_list_node(winner->begin + query.size, winner->size - query.size, 1);

            //Setting pointers...
            second->next = winner->next;
            winner->next = second;
        }
        winner->size = query.size;
        winner->is_empty = 0;

        allocation_query_element back_query;
        back_query.block_number = query.block_number;
        back_query.size = query.size;
        back_query.node = winner;
        query.initiator->allocation_success(back_query);

        this->log->add_entry(std::string("Memory succesfully allocated."));
        return true;
    }
    else
    {
        this->log->add_entry(std::string("Query returned to queue."));
        return false;
    }
}

void mem_list_mgr::free(array_interface* nd) throw (std::invalid_argument)
{
    if(nd == 0)
        throw std::invalid_argument("node was set to zero.");
    mem_list_node* node = (mem_list_node* ) nd;
    //Setting this node as an empty one
    node->is_empty = 1;
    this->log->add_entry(std::string("mem_list_mgr::allocate() : Memory freed."));

    //time for some allocation attempt
    this->ring();
}

void mem_list_mgr::enqueue(mem_query_element query)
{
    this->query_queue.push(query);
    this->log->add_entry(std::string("mem_list_mgr: Query added to queue."));

    this->ring();
}

void mem_list_mgr::ring()
{
    //Signal for loop() to try to do smth
    pthread_mutex_lock(this->memory_list_mutex);
    pthread_cond_signal(this->state_changed);
    pthread_mutex_unlock(this->memory_list_mutex);
}

void mem_list_mgr::loop()
{
    pthread_mutex_lock(this->memory_list_mutex);

    while(!this->quit_signal)
    {

        pthread_cond_wait(this->state_changed, this->memory_list_mutex);

        //Trying to unite free blocks in larger ones
        mem_list_node* current = this->first_node;

        while (current->next != 0)
        {
            if(current->is_empty && current->next->is_empty)
            {
                mem_list_node* temp = current->next;
                current->next = temp->next;
                current->size += temp->size;
                delete temp;
            }
            else
            {
                current = current->next;
            }
        }

        //Trying to unwind the queue
        bool ok = false;
        do
        {
            if(!this->query_queue.is_empty())
            {
                mem_query_element query = this->query_queue.front();
                ok = this->allocate(query);
                if(ok)
                    this->query_queue.pop();
            }
            else
            {
                ok = false;
            }
        }
        while(ok);
    }


    pthread_mutex_unlock(this->memory_list_mutex);
}

node_descriptor* mem_list_mgr::mem_list_state()
{
    pthread_mutex_lock(this->memory_list_mutex);
    node_descriptor* retval = new node_descriptor;
    mem_list_node* current_mem = this->first_node;
    node_descriptor* current_descriptor = retval;
    while(current_mem != 0)
    {
        current_descriptor->is_empty = current_mem->is_empty;
        current_descriptor->size = current_mem->size;
        if(current_mem->next != 0)
        {
            current_descriptor->next = new node_descriptor;
            current_descriptor = current_descriptor->next;
        }
        else
        {
            current_descriptor->next = 0;
        }
        current_mem = current_mem->next;
    }
    pthread_mutex_unlock(this->memory_list_mutex);
    return retval;
}

void mem_list_mgr::create_thread(void *mlmgr)
{
    mem_list_mgr* mgr = (mem_list_mgr*) mlmgr;
    pthread_create(&mgr->thread,NULL, mem_list_mgr::start_thread, mgr);
}

void* mem_list_mgr::start_thread(void* mlmgr)
{
    mem_list_mgr* mgr = (mem_list_mgr*) mlmgr;
    mgr->loop();
    pthread_exit(0);
}

void mem_list_mgr::join()
{
    void* a;
    pthread_join(this->thread, &a);
}
