/*
 *@project memmgrd
 *@file keccak.cpp
 *@brief Implementation of keccak class
 *@author Andrew Kurnoskin
 *@date 14/12/2012
 */

#include "keccak.h"
#include <cstdio>
#include <cstring>

keccak::keccak(std::string str) throw (std::invalid_argument)
{
    //Allocating space for proper input
    unsigned long size = str.size();
    if(size != 128)
        throw std::invalid_argument("keccak::keccak() : Hash initialization string has wrong size!");
    this->input_string = new std::string();

    //Parsing input string
    for(unsigned int i = 0; i < str.size(); i++)
    {
        if(!isxdigit(str[i]))
            throw std::invalid_argument("keccak::keccak() : Hash initialization string contains non-hexadecimal characters!");
        this->input_string->push_back(toupper(str[i]));
    }


}

keccak::~keccak()
{
    delete this->input_string;
}

bool keccak::validate(std::string str)
{
    char* n = hash(str);
    std::string a(n);
    bool retval = !a.compare(*this->input_string);
    delete n;
    return retval;
}


char* keccak::hash(std::string str)
{
    //Allocating space for proper input
    unsigned long size = str.size();
    size = ((unsigned long)(size / 9) + 1) * 9;
    unsigned long long* strng = new unsigned long long[size];

    //Parsing input string
    for(unsigned int i = 0; i < str.size(); i++)
    {
        strng[i] = (unsigned char)str[i];
    }
    for(unsigned int i = str.size(); i < size; i++)
    {
        strng[i] = 0;
    }
    strng[size - 1] = 80;

    //Allocating state array
    unsigned long long* state = new unsigned long long[25];
    for(unsigned int i = 0; i < 25; i++)
        state[i] = 0;


    //Absorbing
    for(unsigned int i = 0; i < size / 9; i++)
    {
        for(unsigned int j = 0; j < 9; j++)
            state[j] = strng[9*i + j];
        this->sponge(state);
    }
    //Squeezing
    char* c = new char[16];
    char* result = new char[128];
    for(unsigned int i = 0; i < 8; i++)
    {
        sprintf(c, "%llX", state[i]);
        memcpy(result + i*16, c, 16);
    }

    for(unsigned int i = 0; i < 128; i++)
    {
        if(result[i] == '\0')
            result[i]= '0';
    }

    delete[] strng;
    delete[] c;

    return result;
}

void keccak::sponge(unsigned long long* state)
{
    for(unsigned int i = 0; i < 24; i++)
        this->round(state, i);
}

void keccak::round(unsigned long long* state, unsigned int n)
{
    unsigned long long B[25];
    unsigned long long C[5];
    unsigned long long D[5];

    //Theta step
    for(unsigned int i = 0; i < 5; i++)
    {
        C[i] = state[5*i + 0] ^ state[5*i + 1] ^ state[5*i + 2] ^ state[5*i + 3] ^ state[5*i + 4];
    }

    for(unsigned int i = 0; i < 5; i++)
    {
        D[i] = C[(i == 0) ? 4 : (i - 1) % 5] ^ rol(C[(i + 1) % 5], 1);
    }
    
    for(unsigned int i = 0; i < 5; i++)
    {
        for(unsigned int y = 0; y < 5; y++)
        {
            state[i * 5 + y] ^= D[i];
        }
    }


    //Rho and pi steps
    for(unsigned int x = 0; x < 5; x++)
    {
        for(unsigned int y = 0; y < 5; y++)
        {
            B[y * 5 + (2 * x + 3*y)%5] = this->rol(state[x * 5 + y], rotations[x * 5 + y]);
        }
    }

    //Chi step
    for(unsigned int x = 0; x < 5; x++)
    {
        for(unsigned int y = 0; y < 5; y++)
        {
            state[x * 5 + y] = B[x * 5 + y] ^ ((~B[((x+1) % 5) * 5 + y]) & B[((x+2) % 5) * 5 + y]);
        }
    }

    //Iota step
    state[0] ^= keccak::round_constants[n];
}

unsigned long long keccak::rol(unsigned long long in, unsigned long q)
{
    unsigned long long answer = in;
    answer <<= q;
    answer = answer | q >> 64 - q;
    return answer;
}

unsigned long long keccak::round_constants[24] = {
    0x0000000000000001,
    0x0000000000008082,
    0x800000000000808A,
    0x8000000080008000,
    0x000000000000808B,
    0x0000000080000001,
    0x8000000080008081,
    0x8000000000008009,
    0x000000000000008A,
    0x0000000000000088,
    0x0000000080008009,
    0x000000008000000A,
    0x000000008000808B,
    0x800000000000008B,
    0x8000000000008089,
    0x8000000000008003,
    0x8000000000008002,
    0x8000000000000080,
    0x000000000000800A,
    0x800000008000000A,
    0x8000000080008081,
    0x8000000000008080,
    0x0000000080000001,
    0x8000000080008008
};

unsigned long long keccak::rotations[25] = {
    0,    36,     3,    41,    18,
    1,    44,    10,    45,     2,
    62,    6,    43,    15,    61,
    28,   55,    25,    21,    56,
    27,   20,    39,     8,    14
};
