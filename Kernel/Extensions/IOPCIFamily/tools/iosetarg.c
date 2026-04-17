/*
cc tools/iosetarg.c -o /tmp/iosetarg -framework IOKit -framework CoreFoundation -g -Wall
*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <IOKit/IOKitLib.h>
#include <mach/mach_error.h>

#ifndef kIODebugArgumentsKey
#define kIODebugArgumentsKey "IODebugArguments"
#endif

int main(int argc, char * argv[])
{
	kern_return_t          kr;
    io_service_t           service;
    CFStringRef            str;
    CFMutableArrayRef	   array;
    CFMutableDictionaryRef matching;
    uint32_t	           idx;
    uint64_t               id;

	if (argc < 3) exit(1);

    id = strtoll(argv[1], NULL, 0);

    matching = id ? IORegistryEntryIDMatching(id) : IOServiceMatching(argv[1]);

	array = CFArrayCreateMutable(kCFAllocatorDefault, argc - 2, &kCFTypeArrayCallBacks);

	for (idx = 2; idx < argc; idx++)
	{
		str = CFStringCreateWithCString(kCFAllocatorDefault, argv[idx], CFStringGetSystemEncoding());
		CFArrayAppendValue(array, str);
	}

    service = IOServiceGetMatchingService(kIOMasterPortDefault, matching);
    assert(service);
	kr = IORegistryEntrySetCFProperty(service, CFSTR(kIODebugArgumentsKey), array);

	printf("result: 0x%x, %s\n", kr, mach_error_string(kr));

	exit(0);
}