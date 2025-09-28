#include <atomic.h>
/* This requires Solaris Studio 12.2 or newer: */
#include <mbarrier.h>
void memory_barrier (void) { __machine_rw_barrier (); }
int atomic_add (volatile unsigned *i) { return atomic_add_int_nv (i, 1); }
void *atomic_ptr_cmpxchg (volatile void **target, void *cmp, void *newval) { return atomic_cas_ptr (target, cmp, newval); }

int main(void) { return 0; }
