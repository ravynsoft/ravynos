//
//  pdcrypto_sha1_impl.cpp
//  pdcrypto
//
//  Created by rafirafi on 3/17/16.
//  Copyright (c) 2016 rafirafi. All rights reserved.
//
//  adapted from botan/src/lib/hash/sha1/sha160.cpp
//  Botan  http://botan.randombit.net
//  License :  https://github.com/randombit/botan/blob/master/doc/license.txt

#include <sys/types.h>
#include <corecrypto/ccdigest.h>
#if KERNEL
#include <libkern/crypto/sha1.h>
#include <sys/systm.h>
#else
#include <stdlib.h>
#endif

/**
* Load a big-endian word
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th T of in, as a big-endian value
*/
/*
template<typename T>
inline T load_be(const uint8_t in[], size_t off)
*/
static inline uint32_t load_be(const uint8_t in[], size_t off)
{
    in += off * sizeof(uint32_t);
    uint32_t out = 0;
    for(size_t i = 0; i != sizeof(uint32_t); ++i)
        out = (out << 8) | in[i];
    return out;
}

/**
* Load a variable number of big-endian words
* @param out the output array of words
* @param in the input array of bytes
* @param count how many words are in in
*/
/*
template<typename T>
inline void load_be(T out[],
                    const uint8_t in[],
                    size_t count)
*/
static inline void load_be_n(uint32_t out[],
                             const uint8_t in[],
                             size_t count)
{
    if(count > 0)
    {

        for(size_t i = 0; i != count; ++i)
            out[i] = load_be(in, i);
    }
}

/**
* Bit rotation left
* @param input the input word
* @param rot the number of bits to rotate
* @return input rotated left by rot bits
*/
/*
template<typename T> inline T rotate_left(T input, size_t rot)
*/
static inline uint32_t rotate_left(uint32_t input, size_t rot)
{
    if(rot == 0)
        return input;
    return static_cast<uint32_t>((input << rot) | (input >> (8*sizeof(uint32_t)-rot)));;
}

/*
* SHA-160 F1 Function
*/
static inline void F1(uint32_t A, uint32_t& B, uint32_t C, uint32_t D, uint32_t& E, uint32_t msg)
{
    E += (D ^ (B & (C ^ D))) + msg + 0x5A827999 + rotate_left(A, 5);
    B  = rotate_left(B, 30);
}

/*
* SHA-160 F2 Function
*/
static inline void F2(uint32_t A, uint32_t& B, uint32_t C, uint32_t D, uint32_t& E, uint32_t msg)
{
    E += (B ^ C ^ D) + msg + 0x6ED9EBA1 + rotate_left(A, 5);
    B  = rotate_left(B, 30);
}

/*
* SHA-160 F3 Function
*/
static inline void F3(uint32_t A, uint32_t& B, uint32_t C, uint32_t D, uint32_t& E, uint32_t msg)
{
    E += ((B & C) | ((B | C) & D)) + msg + 0x8F1BBCDC + rotate_left(A, 5);
    B  = rotate_left(B, 30);
}

/*
* SHA-160 F4 Function
*/
static inline void F4(uint32_t A, uint32_t& B, uint32_t C, uint32_t D, uint32_t& E, uint32_t msg)
{
    E += (B ^ C ^ D) + msg + 0xCA62C1D6 + rotate_left(A, 5);
    B  = rotate_left(B, 30);
}

extern "C"
{

/*
* SHA-160 Compression Function
*/
void pdcsha1_compress(ccdigest_state_t s, unsigned long nblocks, const void *data)
{
    //printf("%s\n", __func__);

    const uint8_t *input = (const uint8_t *)data;

    uint32_t *state = ccdigest_u32(s);

    uint32_t A = state[0], B = state[1], C = state[2], D = state[3], E = state[4];

    uint32_t m_W[80];

    for(size_t i = 0; i != nblocks; ++i)
    {
        load_be_n(m_W, input, 16);

        for(size_t j = 16; j != 80; j += 8)
        {
            m_W[j  ] = rotate_left((m_W[j-3] ^ m_W[j-8] ^ m_W[j-14] ^ m_W[j-16]), 1);
            m_W[j+1] = rotate_left((m_W[j-2] ^ m_W[j-7] ^ m_W[j-13] ^ m_W[j-15]), 1);
            m_W[j+2] = rotate_left((m_W[j-1] ^ m_W[j-6] ^ m_W[j-12] ^ m_W[j-14]), 1);
            m_W[j+3] = rotate_left((m_W[j  ] ^ m_W[j-5] ^ m_W[j-11] ^ m_W[j-13]), 1);
            m_W[j+4] = rotate_left((m_W[j+1] ^ m_W[j-4] ^ m_W[j-10] ^ m_W[j-12]), 1);
            m_W[j+5] = rotate_left((m_W[j+2] ^ m_W[j-3] ^ m_W[j- 9] ^ m_W[j-11]), 1);
            m_W[j+6] = rotate_left((m_W[j+3] ^ m_W[j-2] ^ m_W[j- 8] ^ m_W[j-10]), 1);
            m_W[j+7] = rotate_left((m_W[j+4] ^ m_W[j-1] ^ m_W[j- 7] ^ m_W[j- 9]), 1);
        }

        F1(A, B, C, D, E, m_W[ 0]);   F1(E, A, B, C, D, m_W[ 1]);
        F1(D, E, A, B, C, m_W[ 2]);   F1(C, D, E, A, B, m_W[ 3]);
        F1(B, C, D, E, A, m_W[ 4]);   F1(A, B, C, D, E, m_W[ 5]);
        F1(E, A, B, C, D, m_W[ 6]);   F1(D, E, A, B, C, m_W[ 7]);
        F1(C, D, E, A, B, m_W[ 8]);   F1(B, C, D, E, A, m_W[ 9]);
        F1(A, B, C, D, E, m_W[10]);   F1(E, A, B, C, D, m_W[11]);
        F1(D, E, A, B, C, m_W[12]);   F1(C, D, E, A, B, m_W[13]);
        F1(B, C, D, E, A, m_W[14]);   F1(A, B, C, D, E, m_W[15]);
        F1(E, A, B, C, D, m_W[16]);   F1(D, E, A, B, C, m_W[17]);
        F1(C, D, E, A, B, m_W[18]);   F1(B, C, D, E, A, m_W[19]);

        F2(A, B, C, D, E, m_W[20]);   F2(E, A, B, C, D, m_W[21]);
        F2(D, E, A, B, C, m_W[22]);   F2(C, D, E, A, B, m_W[23]);
        F2(B, C, D, E, A, m_W[24]);   F2(A, B, C, D, E, m_W[25]);
        F2(E, A, B, C, D, m_W[26]);   F2(D, E, A, B, C, m_W[27]);
        F2(C, D, E, A, B, m_W[28]);   F2(B, C, D, E, A, m_W[29]);
        F2(A, B, C, D, E, m_W[30]);   F2(E, A, B, C, D, m_W[31]);
        F2(D, E, A, B, C, m_W[32]);   F2(C, D, E, A, B, m_W[33]);
        F2(B, C, D, E, A, m_W[34]);   F2(A, B, C, D, E, m_W[35]);
        F2(E, A, B, C, D, m_W[36]);   F2(D, E, A, B, C, m_W[37]);
        F2(C, D, E, A, B, m_W[38]);   F2(B, C, D, E, A, m_W[39]);

        F3(A, B, C, D, E, m_W[40]);   F3(E, A, B, C, D, m_W[41]);
        F3(D, E, A, B, C, m_W[42]);   F3(C, D, E, A, B, m_W[43]);
        F3(B, C, D, E, A, m_W[44]);   F3(A, B, C, D, E, m_W[45]);
        F3(E, A, B, C, D, m_W[46]);   F3(D, E, A, B, C, m_W[47]);
        F3(C, D, E, A, B, m_W[48]);   F3(B, C, D, E, A, m_W[49]);
        F3(A, B, C, D, E, m_W[50]);   F3(E, A, B, C, D, m_W[51]);
        F3(D, E, A, B, C, m_W[52]);   F3(C, D, E, A, B, m_W[53]);
        F3(B, C, D, E, A, m_W[54]);   F3(A, B, C, D, E, m_W[55]);
        F3(E, A, B, C, D, m_W[56]);   F3(D, E, A, B, C, m_W[57]);
        F3(C, D, E, A, B, m_W[58]);   F3(B, C, D, E, A, m_W[59]);

        F4(A, B, C, D, E, m_W[60]);   F4(E, A, B, C, D, m_W[61]);
        F4(D, E, A, B, C, m_W[62]);   F4(C, D, E, A, B, m_W[63]);
        F4(B, C, D, E, A, m_W[64]);   F4(A, B, C, D, E, m_W[65]);
        F4(E, A, B, C, D, m_W[66]);   F4(D, E, A, B, C, m_W[67]);
        F4(C, D, E, A, B, m_W[68]);   F4(B, C, D, E, A, m_W[69]);
        F4(A, B, C, D, E, m_W[70]);   F4(E, A, B, C, D, m_W[71]);
        F4(D, E, A, B, C, m_W[72]);   F4(C, D, E, A, B, m_W[73]);
        F4(B, C, D, E, A, m_W[74]);   F4(A, B, C, D, E, m_W[75]);
        F4(E, A, B, C, D, m_W[76]);   F4(D, E, A, B, C, m_W[77]);
        F4(C, D, E, A, B, m_W[78]);   F4(B, C, D, E, A, m_W[79]);

        A = (state[0] += A);
        B = (state[1] += B);
        C = (state[2] += C);
        D = (state[3] += D);
        E = (state[4] += E);

        //input += hash_block_size();
        input += 64;
    }
}

}
