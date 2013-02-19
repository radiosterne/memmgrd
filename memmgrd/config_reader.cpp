/*
 *@project memmgrd
 *@file config_reader.cpp
 *@brief Implementation of config_reader class
 *@author Andrew Kurnoskin
 *@date 14/12/2012
 */

#include "config_reader.h"
#include <fstream>
#include <algorithm>
#include <cctype>


config_reader::config_reader()
{
    //Open file
    std::ifstream file;
    file.open("/etc/memmgrd/config");
    std::string buffer;


    //Set default values
    this->memory_size = 300;
    this->socket_path = "/tmp/memmgrd_socket";
    this->log_path = "/usr/local/memmgrd/log";
    this->password = "A353FB503019BC79B92C494980477759E20EB67E642C42643C146B1D512DC800533172000000000F23072CCB6B200000528649261E300000F21ACB84AD9F51C0";


    //Read file line-by-line
    while(!file.eof())
    {
        getline(file, buffer);

        //Trim spaces
        buffer.erase(std::remove_if(buffer.begin(), buffer.end(), (int(*)(int))std::isspace), buffer.end());

        //Skip empty strings and comments
        if(buffer.empty())
        {
            continue;
        }
        if(buffer[0] == '#')
        {
            continue;
        }

        //Strings should contain one equal sign only
        unsigned int f = buffer.find_first_of('=');
        unsigned int l = buffer.find_last_of('=');
        if(f == l)
        {
            //Divide the string into two substrings
            std::string first_half = buffer.substr(0, f);
            std::string last_half = buffer.substr(f + 1, buffer.length());

            //Check if the first half means something
            if(first_half == "socket")
            {
                this->socket_path = last_half;
            }

            if(first_half == "block_size")
            {
                this->memory_size = atoi(last_half.c_str());
            }

            if(first_half == "log")
            {
                this->log_path = last_half;
            }

            if(first_half == "passwd")
            {
                this->password = last_half;
            }
        }
    }

    file.close();
}

std::string config_reader::path_to_log()
{
    return this->log_path;
}

std::string config_reader::path_to_socket()
{
    return this->socket_path;
}

unsigned int config_reader::mem_list_size()
{
    return this->memory_size;
}

std::string config_reader::passwd()
{
    return this->password;
}
