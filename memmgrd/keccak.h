/*
 *@project memmgrd
 *@file keccak.h
 *@brief Definition of keccak class : a password hash
 *@author Andrew Kurnoskin
 *@date 14/12/2012
 */


#ifndef KECCAK_H
#define KECCAK_H

#include <string>
#include <stdexcept>


/*
 *@brief Provides functionality for password validation using SHA-3 algorithm
 *
 *@note For algorithm specifications, please visit http://keccak.noekeon.org/specs_summary.html
 */
class keccak
{

public:
    /*
     *@briefs Constructs new object
     *@param str : a string to use as initial hash value
     *
     *@throws std::invalid_argument, if str.length() != 128 or input string contains any invalid characters
     */
    keccak(std::string str) throw (std::invalid_argument);

    /*
     *@brief A destructor. Frees used memory.
     */
    ~keccak();

    /*
     *@brief Computes hash of given string and compares it with initial value
     *@param str : a string message to hash
     *
     *@retval true, if str hash is equal to initial hash, false otherwise
     */
    bool validate(std::string str);
private:
    std::string* input_string;

    //An array with round constants
    static unsigned long long round_constants[24];

    //An array with rotation steps on each of every round on rho and pi steps
    static unsigned long long rotations[25];

    //Actual hash
    char* hash(std::string input);

    //Keccak round
    void round(unsigned long long* state,unsigned int n);

    //Keccak sponge function
    void sponge(unsigned long long* state);

    //Bitwise shift
    unsigned long long rol(unsigned long long in, unsigned long q);

};

#endif //KECCAK_H
