/*
 * sdbm - ndbm work-alike hashed database library
 * based on Per-Aake Larson's Dynamic Hashing algorithms. BIT 18 (1978).
 * author: oz@nexus.yorku.ca
 * status: public domain. keep it that way.
 *
 * hashing routine
 */

#include "config.h"
#include "EXTERN.h"
#include "sdbm.h"
/*
 * polynomial conversion ignoring overflows
 * [this seems to work remarkably well, in fact better
 * then the ndbm hash function. Replace at your own risk]
 * use: 65599   nice.
 *      65587   even better. 
 */
long
sdbm_hash(const char *str, int len)
{
        unsigned long n = 0;

#ifdef DUFF

#define HASHC   n = *str++ + 65599 * n

        if (len > 0) {
                int loop = (len + 8 - 1) >> 3;

                switch(len & (8 - 1)) {
                case 0: do {
                        HASHC; /* FALLTHROUGH */ case 7: HASHC; /* FALLTHROUGH */
                case 6: HASHC; /* FALLTHROUGH */ case 5: HASHC; /* FALLTHROUGH */
                case 4: HASHC; /* FALLTHROUGH */ case 3: HASHC; /* FALLTHROUGH */
                case 2: HASHC; /* FALLTHROUGH */ case 1: HASHC;
                        } while (--loop);
                }

        }
#else
        while (len--)
                n = *str++ + 65599 * n;
#endif
        return n;
}
