

// deadwood() is local to its linkage unit and is unsed,
// so reference to undef() is ok

extern void undef();

void dead_wood() __attribute__((visibility("hidden")));
void dead_wood() { undef(); }


