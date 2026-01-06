#include <stdio.h>

#if DSO_DEF
	void* __dso_handle = NULL;
#elif DSO_TENT
	void* __dso_handle;
#else
	extern void* __dso_handle;
#endif

int main()
{
	printf("dso_handle=%p\n", __dso_handle);
	return 0;
}

