/*
 *@project memmgrd
 *@file config_reader.h
 *@brief Definition of config_reader class
 *@author Andrew Kurnoskin
 *@date 14/12/2012
 */

#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <string>

/*
 *@brief Fetсhes data from config file
 */
class config_reader
{
public:
    /*
     *@brief A constructor. Opens config file and sets private members to their respective values
     */
    config_reader();


    /*
     *@brief Returns memory list size, read from config
     */
    unsigned int mem_list_size();

    /*
     *@brief Returns path to socket, read from config
     */
    std::string path_to_socket();

    /*
     *@brief Returns path to log, read from config
     */
    std::string path_to_log();

    /*
     *@brief Returns password, read from config
     */
    std::string passwd();
private:
    unsigned int memory_size;
    std::string socket_path;
    std::string log_path;
    std::string password;
};


#endif //CONFIG_READER_H
