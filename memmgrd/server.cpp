/*
 *@project memmgrd
 *@file server.cpp
 *@brief Implementation of daemon class
 *@author Andrew Kurnoskin
 *@date 06/12/2012
 *@TODO
 */

#include "server.h"

#include <string.h>
#include <sys/un.h>
#include <errno.h>
#include <fcntl.h>
#include <sstream>
#include <unistd.h>

server* server::instance;

server::server() :
    log(0), first_node(0), mgr(0), hash(0)
{
    unsigned int stat = daemon(0,0);

    //At this point, config object is constructed and the config is fetched from file,
    //so we can safely use its values

    this->quit_signal = false;

    try
    {
        this->log = new logger(config.path_to_log());
    }
    catch (std::invalid_argument exception)
    {
        //We can not even use log to signal about this exception
        //But server without logging is useless. Quitting.
        this->quit_signal = true;
    }

    try
    {

        this->mgr = new mem_list_mgr(this->config.mem_list_size(), this->log);

        this->hash = new keccak(this->config.passwd());

    }
    catch(std::invalid_argument exception)
    {
        //Got some exception initializing server
        //Log it and quit
        this->quit_signal = true;
        std::string message("server::server() : ");
        message.append(exception.what());
        this->log->add_entry(message);
    }

    this->quit_signal_mutex = new pthread_mutex_t;
    this->quit_signal_cond= new pthread_cond_t;
    pthread_mutex_init(quit_signal_mutex, 0);
    pthread_cond_init(quit_signal_cond, 0);
}

server::~server()
{
    pthread_cond_destroy(quit_signal_cond);
    pthread_mutex_destroy(quit_signal_mutex);
    if(this->mgr != 0)
        delete this->mgr;
    if(this->log != 0)
        delete this->log;
    if(this->hash != 0)
        delete this->hash;
    delete this->quit_signal_mutex;
    delete this->quit_signal_cond;

}

void* server::quit_thread(void* dmn)
{
    server* d = (server*) dmn;
    d->quit_listen();
    return 0;

}

void server::quit_listen()
{
    pthread_mutex_lock(this->quit_signal_mutex);
    pthread_cond_wait(this->quit_signal_cond, this->quit_signal_mutex);
    this->quit_signal = true;
    pthread_mutex_unlock(this->quit_signal_mutex);
}

void server::loop()
{
    this->log->add_entry("server::loop() : Starting...");

    this->log->add_entry("server::loop() : Starting logger.");
    //Starting logger thread
    logger::create_thread(this->log);

    //Creating and opening unix domain socket and setting it as non-blocking one

    int err = 0; //Error number
    struct sockaddr_un local, remote; //Socket addresses
    int s = socket(AF_UNIX, SOCK_STREAM, 0); //Main socket

    //Setting socket parameters
    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, this->config.path_to_socket().c_str());
    int len = strlen(local.sun_path) + sizeof(local.sun_family);

    //Clearing some place for descriptor
    err = unlink(local.sun_path);
    if(err != 0)
    {
        err = errno;
        if(err != 2 && local.sun_path != "")
        {
            std::ostringstream msg;
            msg << "server::loop() : unlink on socket name failed! Errno set to: ";
            msg << err;
            this->log->add_entry(msg.str());
            this->quit_signal = true;
        }
    }

    //Binding socket to a filename
    if(!this->quit_signal)
    {
        err = bind(s, (struct sockaddr *)&local, len);
        if(err != 0)
        {
            err = errno;
            std::ostringstream msg;
            msg << "server::loop() : binding socket to name failed! Errno set to: ";
            msg << err;
            this->log->add_entry(msg.str());
            this->quit_signal = true;
        }
    }

    //Starting to listen
    if(!this->quit_signal)
    {
        err = listen(s,5);
        if(err != 0)
        {
            err = errno;
            std::ostringstream msg;
            msg << "server::loop() : call to listen() failed! Errno set to: ";
            msg << err;
            this->log->add_entry(msg.str());
            this->quit_signal = true;
        }
    }

    //Changing socket to non-blocking IO mode
    if(!this->quit_signal)
    {
        err = fcntl(s, F_SETFL, O_NONBLOCK);
        if(err != 0)
        {
            err = errno;
            std::ostringstream msg;
            msg << "server::loop() : setting socket as non-blocking via fcntl failed! Errno set to: ";
            msg << err;
            this->log->add_entry(msg.str());
            this->quit_signal = true;
        }
    }


    //If something went wrong with the initialization or opening socket
    if(this->quit_signal)
    {
        this->log->add_entry("server::loop() : Initialization failure, initiaiting failure-quit.");
        this->log->quit_signal = 1;

        this->log->join();
        return;
    }

    this->log->add_entry("server::loop() : Socket succesfully created and put in the non-blocking mode...");

    //Starting quit_signal listener thread
    pthread_create(&this->quit_listen_thread,NULL, server::quit_thread, (void *)this);

    //Starting mem_list_mgr thread
    this->log->add_entry("server::loop() : Starting memory manager.");
    mem_list_mgr::create_thread(this->mgr);

    while(!this->quit_signal)
    {
        unsigned int sz = sizeof(remote);
        int new_socket = accept(s, (struct sockaddr *)&remote, &sz);
        //if(new_socket != -1)
        while(new_socket != -1)
        {
            this->log->add_entry("server::loop() : Accepted new connection.");
            try
            {
                socket_conn_handler* hndl = new socket_conn_handler(this->quit_signal_mutex, this->quit_signal_cond,
                                                                    this->mgr, this->log, this->hash, new_socket);
                socket_conn_handler::create_thread(hndl);

                //Inserting newly created connection handler to the list
                if(this->first_node == 0)
                    this->first_node == hndl;
                else
                {
                    socket_conn_handler* current = this->first_node;
                    while(current->next != 0)
                        current = current->next;
                    current->next = hndl;
                }

                this->log->add_entry("server::loop() : New connection thread started.");
            }
            catch(const std::exception& exc)
            {
                std::string message("server::loop() : ");
                message.append(exc.what());
                this->log->add_entry(message);
            }

            new_socket = accept(s, (struct sockaddr *)&remote, &sz);
        }

        //Searching for already finished threads
        socket_conn_handler* current = this->first_node;
        while(current != 0)
        {
            if(current->next != 0)
            {
                if(current->next->quit_signal == true)
                {
                    socket_conn_handler* to_trash = current->next;
                    to_trash->join();
                    current->next = current->next->next;
                    delete to_trash;
                }
            }
            if(this->first_node->quit_signal == true)
            {
                socket_conn_handler* to_trash = this->first_node;
                to_trash->join();
                this->first_node = this->first_node->next;
                delete to_trash;
            }
        }
        sleep(2);
    }

    this->log->add_entry("server::loop() : Beggining normal quit sequence");

    while(this->first_node !=  0)
    {
        this->first_node->quit_signal = true;
        socket_conn_handler* to_trash = this->first_node;
        to_trash->join();
        this->first_node = this->first_node->next;
        delete to_trash;
    }

    this->mgr->quit_signal = true;
    this->mgr->ring();
    this->log->quit_signal = 1;

    this->mgr->join();
    this->log->join();

    void* ptr;
    pthread_join(this->quit_listen_thread, &ptr);
}

server* server::get_instance()
{
    if(server::instance == 0)
        server::instance = new server();
    return server::instance;
}

void server::kill_instance()
{
    delete server::instance;
}
