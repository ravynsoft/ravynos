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
cc ionotify.c -o notify -Wall -Wno-four-char-constants -framework IOKit
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include <mach/mach_interface.h>
#include <IOKit/IOKitLib.h>

mach_port_t	masterPort;

void dumpIter( io_iterator_t iter, mach_port_t recvPort )
{
    kern_return_t	kr;
    io_object_t		obj;
    io_name_t		name;
    io_string_t		path;
    mach_port_t		iter3;
    
    while( (obj = IOIteratorNext( iter))) {
        assert( KERN_SUCCESS == (
        kr = IORegistryEntryGetName( obj, name )
        ));
	printf("name:%s(%d)\n", name, obj);
        kr = IORegistryEntryGetPath( obj, kIOServicePlane, path );
        if( KERN_SUCCESS == kr) {
            
	    // if the object is detached, getPath will fail
            printf("path:%s\n", path);

            // as will IOServiceAddInterestNotification
            if( KERN_SUCCESS != (
                    kr = IOServiceAddInterestNotificationMsg( obj, kIOGeneralInterest,
                            recvPort, (unsigned int) obj, &iter3)
            )) printf("IOServiceAddInterestNotification(%lx)\n", kr);
            // can compare two kernel objects with IOObjectIsEqualTo() if we keep the object,
            // otherwise, release
            IOObjectRelease( obj );
        }
    }
}


void notifyTest( char * arg )
{
    kern_return_t	kr;
    io_iterator_t	iter1, iter2, iter3;
    mach_port_t		port;
    io_service_t	obj;
    io_name_t		name;
    const char *	type;
    IOServiceInterestContent * content;
    vm_size_t		size;
    unsigned long int  	notifyType;
    unsigned long int  	ref;
    struct {
        mach_msg_header_t	msgHdr;
        OSNotificationHeader	notifyHeader;
        IOServiceInterestContent	content;
        mach_msg_trailer_t	trailer;
    } msg;

    assert( KERN_SUCCESS == (
    kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port)
    ));

    type = kIOMatchedNotification;// or kIOPublishNotification;
    assert( KERN_SUCCESS == (
    kr = IOServiceAddNotification( masterPort, type,
                                    IOServiceMatching( arg ),
//                                    IOBSDNameMatching( masterPort, 0, arg ),
                                    port, (unsigned int) type, &iter2 )
    ));
    printf("IOServiceAddNotification: %s: ", type );
    // dumping the iterator gives us the current list
    // and arms the notification for changes
    dumpIter( iter2, port );
    printf("\n");

    type = kIOTerminatedNotification;
    assert( KERN_SUCCESS == (
    kr = IOServiceAddNotification( masterPort, type,
                                    IOServiceMatching( arg ),
                                    port, (unsigned int) type, &iter1 )
    ));
    printf("IOServiceAddNotification: %s: ", type );
    dumpIter( iter1, port );
    printf("\n");

    type = kIOBusyInterest;
    obj = IORegistryEntryFromPath( masterPort, kIOServicePlane ":/");
    assert( obj );
    assert( KERN_SUCCESS == (
    kr = IOServiceAddInterestNotificationMsg( obj, type,
                                           port, (unsigned int) obj, &iter3)
    ));

    printf("ports: iter1=%d, iter2=%d, rcv=%d\n", iter1, iter2, port);

    do {
	printf("waiting...\n");
        assert( KERN_SUCCESS == (
        kr = mach_msg(&msg.msgHdr, MACH_RCV_MSG,
                    0, sizeof(msg), port, 0, MACH_PORT_NULL)
        ));

        assert( KERN_SUCCESS == (
	kr = OSGetNotificationFromMessage( &msg.msgHdr, 0, &notifyType, &ref,
			(void **) &content, &size )
        ));

	// we passed a string for the refcon
        printf("got notification, type=%d, local=%d, remote=%d\n", notifyType,
		msg.msgHdr.msgh_local_port, msg.msgHdr.msgh_remote_port );
        if( notifyType == kIOServiceMessageNotificationType) {
            obj = ref;
            if( KERN_SUCCESS == (
            kr = IORegistryEntryGetName( obj, name )
                                 )) printf(name); else printf("???");
            printf(": messageType %08lx, arg %08lx\n",
                   content->messageType, content->messageArgument[0]);
        } else {
            printf("%s: ", ref );
            // remote port is the notification (an iterator_t) that fired
            dumpIter( msg.msgHdr.msgh_remote_port, port );
        }

    } while( TRUE );

    IOObjectRelease( iter1 );
    IOObjectRelease( iter2 );
    IOObjectRelease( iter3 );
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

	if( argc > 1)
            notifyTest( argv[1] );
	else
	    printf("%s className\n", argv[0]);

	printf("Exit\n");
}

