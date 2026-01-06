#include <mach/mach_types.h>

extern void extern_func();

int my_global = 3;
extern int extern_global;
extern int extern_unused_global;
extern int my_used_external_global;

kern_return_t mykext_start (kmod_info_t * ki, void * d) {
	++my_global;
	++extern_global;
	++my_used_external_global;
	extern_func();
    return KERN_SUCCESS;
}


kern_return_t mykext_stop (kmod_info_t * ki, void * d) {
	--my_global;
	--extern_global;
    return KERN_SUCCESS;
}

void my_dead_code() {
	++extern_unused_global;
}