#include "socket_conn_handler.h"

#include <sys/time.h>
#include <cstring>
#include <sstream>
#include <errno.h>
#include <unistd.h>


socket_conn_handler::socket_conn_handler(pthread_mutex_t* qsm, pthread_cond_t* qsc, mem_list_mgr* mgr, logger* log, keccak* h, int socket)
    : socket(socket),quit_signal_mutex(qsm), quit_signal_cond(qsc), mgr(mgr), log(log), hash(h),
      quit_signal(false),is_authenticated(false),next(0)
{
    this->nodes_array.resize(0);
}

void socket_conn_handler::loop()
{
    //Routine data for each select-read round
    unsigned int* buffer = new unsigned int[4];
    timeval delay;
    fd_set socket_set;
    int select_state;


    while(!this->quit_signal)
    {
        //Setting routines for select call
        delay.tv_sec = 0;
        delay.tv_usec = 300;
        FD_ZERO(&socket_set);
        FD_SET(this->socket, &socket_set);


        select_state = select(this->socket + 1, &socket_set, NULL, NULL, &delay);

        if(select_state != 0)
        {
            read(this->socket, buffer, 16);

            //This switch is a core logic of working with recieved query
            switch(buffer[0])
            {
            case socket_connection_closed:
                {
                    // Free all memory nodes
                    for(unsigned int i = 0; i < this->nodes_array.size(); i++)
                    {
                        this->mgr->free(this->nodes_array[i]);
                    }

                    //Marking this connection as closed
                    this->quit_signal = true;
                    break;
                }
            case socket_byte_sent:
                {
                    unsigned int& block = buffer[1];
                    unsigned int& position = buffer[2];
                    unsigned int& value = buffer[3];
                    //Trying to set byte info
                    try
                    {
                        (*(this->nodes_array[block]))[position] = value;
                    }
                    catch(const std::exception& exc)
                    {
                        //Logging exception
                        this->log_format(exc.what());

                        //Sending error report over socket
                        buffer[0] = socket_error;
                        this->safe_send(buffer);
                    }

                    break;
                }
            case socket_byte_requested:
                {
                    unsigned int& block = buffer[1];
                    unsigned int& position = buffer[2];
                    buffer[0] = socket_byte_sent;

                    //Trying to retrieve byte info

                    try
                    {
                        buffer[4] = (*(this->nodes_array[block]))[position];
                        this->safe_send(buffer);
                    }
                    catch(const std::exception& exc)
                    {
                        //Logging exception
                        this->log_format(exc.what());

                        //Sending error report over socket
                        buffer[0] = socket_error;
                        this->safe_send(buffer);
                    }
                    break;
                }
            case socket_memory_requested:
                {
                    //Preparing query
                    mem_query_element query;
                    query.block_number = buffer[1];
                    query.size = buffer[2];
                    query.initiator = this;

                    //Just sending query over to manager
                    this->mgr->enqueue(query);
                    break;
                }
            case socket_memory_termination:
                {
                    unsigned int& block = buffer[1];

                    //Checking whether it's a valid block number
                    if(block >= this->nodes_array.size())
                    {
                        //Logging exception
                        this->log_format("An attempt to free a block with invalid number was made.");

                        //Sending error report over socket
                        buffer[0] = socket_error;
                        this->safe_send(buffer);
                    }
                    else
                    {
                        this->mgr->free(this->nodes_array[block]);
                    }
                    break;
                }
            case socket_authenticate:
                {
                    //Recovering password string from socket
                    std::string passwd;
                    do
                    {
                        read(this->socket, buffer, 16);
                        passwd.push_back(buffer[1]);
                    }
                    while(buffer[0] != socket_passwd_end);

                    passwd.erase(passwd.size() - 1, 1);

                    //Validating using hash
                    if(this->hash->validate(passwd))
                    {
                        this->log_format("Succesfully authenticated");
                        this->is_authenticated = true;
                        buffer[0] = socket_authenticated;
                        this->safe_send(buffer);
                    }
                    else
                    {
                        this->log_format("Unsuccesful authentication attempt");
                        this->is_authenticated = false;
                        buffer[0] = socket_not_authenticated;
                        this->safe_send(buffer);
                    }

                    break;
                }
            case socket_quit:
                {
                    //Signaling to server that quit is requested
                    if(this->is_authenticated)
                    {
                        pthread_mutex_lock(this->quit_signal_mutex);
                        pthread_cond_signal(this->quit_signal_cond);
                        pthread_mutex_unlock(this->quit_signal_mutex);
                    }
                    else
                    {
                        this->log_format("Unauthenticated socket tried to initiate quit sequence!");
                        buffer[0] = socket_not_authenticated;
                        this->safe_send(buffer);
                    }
                    break;
                }
            case socket_memory_map_requested:
                {
                    if(this->is_authenticated)
                    {
                        buffer[0] = socket_memory_map_sent;
                        this->safe_send(buffer);

                        node_descriptor* start = this->mgr->mem_list_state();
                        while(start != 0)
                        {
                            node_descriptor* prev = start;
                            buffer[0] = socket_node_descriptor;
                            buffer[1] = start->size;
                            buffer[2] = start->is_empty;
                            start = start->next;
                            buffer[3] = (start == 0) ? 0 : 1;
                            this->safe_send(buffer);
                            delete prev;
                        }
                        break;
                    }
                    else
                    {
                        this->log_format("Unauthenticated socket tried to get memory map sequence!");
                        buffer[0] = socket_error;
                        this->safe_send(buffer);
                        break;
                    }
                }
            default:
                {
                    this->log_format("Unrecognized message code!");
                    buffer[0] = socket_error;
                    this->safe_send(buffer);
                    break;
                }
            }
        }


        //Unwinding allocation queue
        while(!this->allocation_query.is_empty())
        {
            allocation_query_element query = this->allocation_query.front();

            if(this->nodes_array.size() <= query.block_number)
            {
                this->nodes_array.resize(query.block_number + 1);
            }

            this->nodes_array[query.block_number] = query.node;

            buffer[0] = socket_memory_allocated;
            buffer[1] = query.block_number;
            buffer[2] = query.size;
            buffer[3]= 0;

            this->safe_send(buffer);
            this->allocation_query.pop();
        }
    }

    if(this->quit_signal)
    {
        buffer[0] = socket_connection_closed;
        this->safe_send(buffer);
    }


    close(this->socket);

    delete[] buffer;
}

void socket_conn_handler::allocation_success(allocation_query_element query)
{
    this->allocation_query.push(query);
}

void socket_conn_handler::join()
{
    void* a;
    pthread_join(this->thread, &a);
}

void socket_conn_handler::log_format(std::string message)
{
    std::ostringstream msg;
    msg << "socket_conn_handler::loop() at socket " << this->socket << " : " << message;
    this->log->add_entry(msg.str());
}

bool socket_conn_handler::safe_send(unsigned int* buffer)
{
    int err = send(this->socket, buffer, 16, 0);
    if(err == -1)
    {
        err = errno;
        std::ostringstream msg;
        msg << "Send failed, errno set to" << err;
        this->log_format(msg.str());

        return false;
    }
    return true;
}

void* socket_conn_handler::start_thread(void* conn_handler)
{
    socket_conn_handler* hndl = (socket_conn_handler*) conn_handler;
    hndl->loop();
    pthread_exit(0);
}

void socket_conn_handler::create_thread(void* conn_handler)
{
    socket_conn_handler* hndl = (socket_conn_handler*) conn_handler;
    pthread_create(&hndl->thread,NULL, socket_conn_handler::start_thread, hndl);
}
