#include <stddef.h>

int  keep_global = 1;
asm(".desc _keep_global, 0x10");

__attribute__((visibility("hidden"))) int  keep_hidden = 1;
asm(".desc _keep_hidden, 0x10");

static int  keep_static = 1;
asm(".desc _keep_static, 0x10");


int  lose_global = 1;

__attribute__((visibility("hidden"))) int  lose_hidden = 1;

static int  lose_static = 1;



int get()
{
	return keep_global + keep_hidden + keep_static + lose_global + lose_hidden + lose_static;
}
