/*
 *@project memmgrd
 *@file logger.cpp
 *@brief Implementation of logger class
 *@author Andrew Kurnoskin
 *@date 19/02/2013
 */

#include "logger.h"

#include <unistd.h>

logger::logger(std::string filename) throw (std::invalid_argument)
{
    this->log_queue = new queue<std::string>;
    this->log_file.open(filename.c_str());
    this->quit_signal = false;
    if(this->log_file.fail())
    {
        this->quit_signal = true;
        throw std::invalid_argument("Cannot open file specified in filename!");
    }
}

logger::~logger()
{
    delete this->log_queue;
    this->log_file.close();
}

void logger::add_entry(std::string message)
{
    this->log_queue->push(message);
}

void logger::loop()
{
    while(!this->quit_signal)
    {
        while(!this->log_queue->is_empty())
        {
            log_file << log_queue->front() << '\n';
            log_queue->pop();
        }
        log_file.flush();
        sleep(1);
    }
    while(!this->log_queue->is_empty())
    {
        log_file << log_queue->front() << '\n';
        log_queue->pop();
    }
}

void logger::create_thread(void *logr)
{
    logger* lgr = (logger*) logr;
    pthread_create(&lgr->thread,NULL, logger::start_thread,lgr);
}

void* logger::start_thread(void* logr)
{
    logger* lgr = (logger* ) logr;
    lgr->loop();
    pthread_exit(0);
}

void logger::join()
{
    void* a;
    pthread_join(this->thread, &a);
}
