#include <stdatomic.h>

void memory_barrier (void) { atomic_thread_fence (memory_order_acq_rel); }
int atomic_add (atomic_int *i) { return atomic_fetch_add_explicit (i, 1, memory_order_relaxed); }
int mutex_trylock (atomic_flag *m) { return atomic_flag_test_and_set_explicit (m, memory_order_acquire); }
void mutex_unlock (atomic_flag *m) { atomic_flag_clear_explicit (m, memory_order_release); }

int main(void) { return 0;}
