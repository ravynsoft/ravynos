void memory_barrier (void) { __sync_synchronize (); }
int atomic_add (int *i) { return __sync_fetch_and_add (i, 1); }
int mutex_trylock (int *m) { return __sync_lock_test_and_set (m, 1); }
void mutex_unlock (int *m) { __sync_lock_release (m); }

int main(void) { return 0;}
