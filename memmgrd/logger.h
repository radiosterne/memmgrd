/*
 *@project memmgrd
 *@file logger.h
 *@brief Definition of logger class
 *@author Andrew Kurnoskin
 *@date 19/02/2013
 */

#ifndef LOGGER_H
#define LOGGER_H

#include "queue.h"
#include <pthread.h>
#include <string>
#include <fstream>

/*
 *@brief A logger facility for simple multi-threaded logging
 *
 *@thread Authomatically starts loop() thread after calling to create_thread(void* logr)
 */
class logger
{
public:
    /*
     *@brief Creates logger associated with file filename
     *@param filename : a file for logging
     *
     *@throws std:invalid argument if file was not opened
     */
    logger(std::string filename) throw (std::invalid_argument);

    /*
     *@brief Frees all allocated memory
     */
    ~logger();

    /*
     *@brief Adds message to logging queue
     *@param message : a string to log
     */
    void add_entry(std::string message);

    /*
     *@brief A signal for loop() to stop it's activity
     */
    bool quit_signal;

    /*
     *@brief Waits for main object's thread to exit
     */
    void join();

    /*
     *@brief Creates and starts new thread
     *@param logr: a pointer to logger object, which loop() needs to be started
     */
    static void create_thread(void* logr);
private:  
    //Main logging function
    void loop();

    //-------------------------------------------------------------------------
    //Thread starting func
    static void* start_thread(void* logr);
    //Thread container
    pthread_t thread;

    //-------------------------------------------------------------------------
    //Queue of messages to log. Used across many threads,
    //and hence allocated on stack
    queue<std::string>* log_queue;

    //-------------------------------------------------------------------------
    //Log file  stream
    std::ofstream log_file;
};

#endif // LOGGER_H
