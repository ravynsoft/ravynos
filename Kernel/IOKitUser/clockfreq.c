/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
cc clockfreq.c -o /tmp/clockfreq -Wall -Wno-four-char-constants -framework IOKit
*/

#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <stdio.h>


#include <IOKit/IOKitLib.h>


mach_port_t	masterPort;


void printInt32( CFDataRef data, CFStringRef key )
{
    UInt32	value;

    printf("\"%s\" ", CFStringGetCStringPtr(key, kCFStringEncodingMacRoman));

    if( data
     && (CFDataGetLength(data) >= sizeof( value ))) {
        value = *((UInt32 *)CFDataGetBytePtr(data));
        printf("= %ld = %08lx\n", value, value );
    } else
	printf("not found\n");
}

void printDictInt32( CFDictionaryRef dict, CFStringRef key )
{
    printInt32( CFDictionaryGetValue(dict, key), key );
}

void printEntryInt32( io_registry_entry_t entry, CFStringRef key )
{
    CFDataRef data;

    data = IORegistryEntryCreateCFProperty( entry, key,
            kCFAllocatorDefault, kNilOptions );
    printInt32( data, key );
    if( data)
        CFRelease(data);
}

void getClockFrequency( void )
{
    kern_return_t	kr;
    io_registry_entry_t	root;
    io_registry_entry_t	cpus;
    io_registry_entry_t	cpu;
    io_iterator_t	iter;
    io_name_t		name;
    CFDataRef		data;
    CFDictionaryRef	properties;

    assert( (
    root = IORegistryEntryFromPath( masterPort,
		kIODeviceTreePlane ":/" )
    ));
    assert( KERN_SUCCESS == (
    kr = IORegistryEntryCreateCFProperties(root, &properties,
                                        kCFAllocatorDefault, kNilOptions )
    ));
    data = CFDictionaryGetValue(properties, CFSTR("compatible"));

    printf("machine ");
    if( data)
        printf(CFDataGetBytePtr(data));
    printf("\n---------------------\n");
    printDictInt32(properties, CFSTR("clock-frequency"));

    CFRelease(properties);

    // go looking for a cpu

    if( (cpus = IORegistryEntryFromPath( masterPort,
		kIODeviceTreePlane ":/cpus" ))) {
        assert( KERN_SUCCESS == (
	kr = IORegistryEntryGetChildIterator( cpus, kIODeviceTreePlane, &iter )
        ));
        IOObjectRelease( cpus );
    } else {
        assert( KERN_SUCCESS == (
	kr = IORegistryEntryGetChildIterator( root, kIODeviceTreePlane, &iter )
        ));
    }

    while( (cpu = IOIteratorNext( iter ))) {
	if( (data = IORegistryEntryCreateCFProperty( cpu, CFSTR("device_type"),
		kCFAllocatorDefault, kNilOptions ))
	 && (0 == strcmp("cpu", CFDataGetBytePtr(data)))) {

            printf("\nprocessor ");
            if( KERN_SUCCESS == IORegistryEntryGetName( cpu, name))
		printf(name);
            printf("\n---------------------\n");


            assert( KERN_SUCCESS == (
            kr = IORegistryEntryCreateCFProperties(cpu, &properties,
                                                kCFAllocatorDefault, kNilOptions )
            ));

	    printDictInt32(properties, CFSTR("clock-frequency"));
	    printDictInt32(properties, CFSTR("bus-frequency"));
	    printDictInt32(properties, CFSTR("timebase-frequency"));
	    printDictInt32(properties, CFSTR("d-cache-size"));
	    printDictInt32(properties, CFSTR("i-cache-size"));
	    printDictInt32(properties, CFSTR("cpu-version"));

            CFRelease(properties);
	}
	IOObjectRelease(cpu);
    }

    IOObjectRelease( iter );
    IOObjectRelease( root );
}

int
main(int argc, char **argv)
{
	kern_return_t		kr;

	/*
	 * Get master device port
	 */
	assert( KERN_SUCCESS == (
	kr = IOMasterPort(   bootstrap_port,
			     &masterPort)
	));

	getClockFrequency();

        return(0);
}
