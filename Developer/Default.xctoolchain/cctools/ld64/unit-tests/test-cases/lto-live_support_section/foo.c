
int my_global = 0;

__attribute__ ((section ("__DATA,__asan_globals,regular")))
struct { void *ptr; } global_metadata = { .ptr = &my_global };

__attribute__ ((used, section ("__DATA,__asan_liveness,regular,live_support")))
struct { void *a, *b; } liveness_binder = { .a = &global_metadata, .b = &my_global };

int unused_global = 0;

__attribute__ ((section ("__DATA,__asan_globals,regular")))
struct { void *ptr; } unused_global_metadata = { .ptr = &unused_global };

__attribute__ ((used, section ("__DATA,__asan_liveness,regular,live_support")))
struct { void *a, *b; } unused_liveness_binder = { .a = &unused_global_metadata, .b = &unused_global };


int main(int argc, char *argv[])
{
  return my_global;
}
