/*
 * SPDX: CC0
 * This code is in the public domain, or at your option, may be used
 * under the Creative Commons Zero (CC0) License.
 */


#include <stdatomic.h>

using namespace std;

extern "C"
long OSAtomicAdd64(long i, volatile _Atomic long *v) {
    return atomic_fetch_add(v, i);
}

extern "C"
int OSAtomicAdd32(long i, volatile _Atomic int *v) {
    return atomic_fetch_add(v, i);
}

