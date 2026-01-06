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
 cc -fno-rtti -fno-exceptions -fcheck-new -fvtable-thunks -arch ppc -lIOKit FireWireTest.cpp -o FireWireTest

 cc -fno-rtti -fno-exceptions -fcheck-new -fvtable-thunks -I.. -I../../BUILD/obj/EXPORT_HDRS/libkern -I../../BUILD/obj/EXPORT_HDRS/osfmk FireWireTest.cpp ../../BUILD/obj/RELEASE_PPC/iokit/User/libIOKit.A.dylib -o FireWireTest

*/

extern "C" {
#include <mach/message.h>
}

#include <IOKit/IOLib.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOContainers.h>
#include <IOKit/IOSerialize.h>

extern "C" {
#include "../../BUILD/obj/RELEASE_PPC/iokit/User/DeviceUser.h"
    extern mach_port_t bootstrap_port;
}

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

// Temporary place for these
enum {
    kFireWireRead = 0,
    kFireWireWrite,
    kFireWireCompareSwap,
    kFireWireBusReset,
    kFireWireTest,
    kNumFireWireMethods
};

mach_port_t	master_device_port;
io_connect_t	connection;
bool dotest, domap, doread, dowrite, dolock, doforever, dobusreset;

UInt32 addrHi, addrLo;
int tranSize;

IOReturn doTest()
{
    IOReturn kr;
    int args[4];
    unsigned int size;
    size = 0;
    kr = io_connect_method_scalarI_scalarO( connection, kFireWireTest,
            args, 0, NULL, &size);
    if(kIOReturnSuccess != kr)
        printf("Test returned 0x%x, size = %d:\n", kr, size);
    return kr;
}

IOReturn doMapMem()
{
    IOReturn		kr;
    vm_size_t		shmemSize;
    vm_address_t	bufMem;

    kr = IOMapMemory( connection, addrLo, mach_task_self(),
                        &bufMem, &shmemSize, TRUE);
    if(kIOReturnSuccess == kr) {
        int i;
        printf("mapped %d bytes at 0x%x\n", shmemSize, bufMem);
        for(i=0; i<shmemSize/4; i++)
            *(int *)(bufMem+i*sizeof(int)) = i;
    }
    return kr;
}

void runTests(io_object_t device)
{
    IOReturn kr;
    unsigned int size;
    int i;
    int args[4];
    char buf[4096];
    kr = IOServiceOpen(device, mach_task_self(), 11, &connection);
    if(kIOReturnSuccess != kr) {
        printf("IOServiceOpen failed: 0x%x\n", kr);
	return;
    }

    if(domap) {
    // Now map some memory for it.
        kr = doMapMem();
        if(kIOReturnSuccess != kr)
            return;
    }
    if(doread) {
        args[0] = addrHi; // Addr Hi
        args[1] = addrLo; // Addr Lo
        size = tranSize;
        kr = io_connect_method_scalarI_structureO( connection, kFireWireRead,
                args, 2, buf, &size);
        printf("Read of 0x%x:0x%x returned 0x%x, size = %d: ", args[0], args[1], kr, size);
        for(i=0; i<size/4; i++)
            printf("0x%x ", *((UInt32 *)buf + i));
        printf("\n");
    }
    do {
        if(dobusreset) {
            size = 0;
            kr = io_connect_method_scalarI_scalarO(connection,
		kFireWireBusReset, args, 0, args, &size);
            if(kIOReturnSuccess != kr)
                printf("Bus reset returned 0x%x: ", kr);
        }

	if(dowrite) {
            size = tranSize;
            for(i=0; i<size/4; i++)
                *( (UInt32 *)buf+i) += 0x01010101;
            args[0] = addrHi; // Addr Hi
            args[1] = addrLo; // Addr Lo
            kr = io_connect_method_scalarI_structureI(connection, kFireWireWrite,
                    args, 2, buf, size);
            if(kIOReturnSuccess != kr)
                printf("Write returned 0x%x, size = %d:\n", kr, size);
	}
	if(dolock) {
            args[0] = addrHi; // Addr Hi
            args[1] = addrLo; // Addr Lo
            args[2] = ((UInt32 *)buf)[0];
            args[3] = ((UInt32 *)buf)[1];
            size = 0;
            kr = io_connect_method_scalarI_scalarO( connection, kFireWireCompareSwap,
                    args, 4, NULL, &size);
            if(kIOReturnSuccess != kr) {
                printf("Compare&Swap returned 0x%x\n", kr);
                break;
            }
        }
        if(doread) {
            args[0] = addrHi; // Addr Hi
            args[1] = addrLo; // Addr Lo
            size = tranSize;
            kr = io_connect_method_scalarI_structureO( connection, kFireWireRead,
                    args, 2, buf, &size);
            printf("Read of 0x%x:0x%x returned 0x%x, size = %d: ", args[0], args[1], kr, size);
            if(kIOReturnSuccess != kr)
		break;
            for(i=0; i<size/4; i++)
                printf("0x%x ", *((UInt32 *)buf + i));
            printf("\n");
        }
        if(dotest)
            kr = doTest();

    } while(doforever && kIOReturnSuccess == kr);

}

kern_return_t processDevice(io_object_t device, UInt64 guid)
{
    kern_return_t	kr;
    IOObject *		object;
    IODictionary *	properties;
    IOOffset *		dataDesc;
    UInt32		nodeID;
    UInt64		gotGUID;

    kr = IORegistryEntryGetProperties(device, &object);
    if (kr)
        return kr;
    if(!object)
        return kIOReturnBadArgument;

    properties = (IODictionary *)object;
    dataDesc = (IOOffset *)properties->getObject("FireWire Node ID");
    if(0 == dataDesc)
        return kIOReturnBadArgument;
    nodeID = dataDesc->unsigned32BitValue();
    dataDesc = (IOOffset *)properties->getObject("GUID");
    if(0 == dataDesc)
        return kIOReturnBadArgument;
    gotGUID = dataDesc->unsigned64BitValue();
    dataDesc = (IOOffset *)properties->getObject("Unit_Spec_ID");
    if(0 != dataDesc) {
        printf("Found device %x, Unit_Spec_ID: 0x%x, GUID: 0x%x:0x%x\n", nodeID, dataDesc->unsigned32BitValue(),
		(UInt32)(gotGUID>>32), (UInt32)(gotGUID & 0xffffffff));
    }
    else {
        printf("Found device %x with no units, GUID: 0x%x:0x%x\n", nodeID,
			(UInt32)(gotGUID>>32), (UInt32)(gotGUID & 0xffffffff));
    }
    object->release();
    if(guid == gotGUID) {
	printf("Found it!\n");
        runTests(device);
        exit(0);
    }
    return kIOReturnSuccess;
}

int pokeDevices(UInt64 guid)
{
    kern_return_t	kr;
    io_iterator_t	enumer;
    io_object_t		obj;
    mach_port_t		port;
    unsigned long int  	notifyType;
    unsigned long int  	ref;
    struct {
        mach_msg_header_t	msgHdr;
        OSNotificationHeader	notifyHeader;
        mach_msg_trailer_t	trailer;
    } msg;

    kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &port);
    if(KERN_SUCCESS != kr) {
        printf("Failed to create port!:%d\n", kr);
        return kr;
    }

    kr = IOServiceAddNotification( master_device_port, kIOMatchedNotification,
                                    IOServiceMatching( "IOFireWireDevice" ),
                                    port, 12345, &enumer );

    if(kIOReturnSuccess != kr) {
        printf("IOServiceAddNotification = %d\n", kr);
        return kr;
    }

    // First process any devices already connected
    while(obj = IOIteratorNext(enumer)) {
        processDevice(obj, guid);
    }

    // Now wait for a new device (or devices) to appear and
    // process them.
    kr = mach_msg(&msg.msgHdr, MACH_RCV_MSG,
                0, sizeof(msg), port, 0, MACH_PORT_NULL);
    kr = OSGetNotificationFromMessage( &msg.msgHdr, 0, &notifyType, &ref,
                    0, 0 );

    if(ref == 12345) {
        while(obj = IOIteratorNext(enumer)) {
            processDevice(obj, guid);
        }
    }
    else
        printf("Unexpected notification type %d, ref %d\n",
		notifyType, ref);
    IOObjectRelease( enumer);
    return kr;
}

int
main(int argc, char **argv)
{
    kern_return_t	kr;
    UInt64 guid = 0;

    IOString *grr;
    grr = new IOString;
    grr->release();

    /*
     * Get master device port
     */
    kr = IOMasterPort(bootstrap_port,
                              &master_device_port);

    if (kr != KERN_SUCCESS)
        printf("IOMasterPort failed: %lx\n", kr);
    tranSize = 32;
    addrHi = 0xffff;
    addrLo = 0xf0000400;
    if(argc > 1) {
        if(strrchr(argv[1], 'h')) {
            printf("%s guid [t/m/r/w/l/b/f] [size] [addrHi] [addrLo]\n", argv[0]);
            printf("t=test, m=map memory, r=read, w=write, l=lock, b=busreset, f=repeat forever\n");
            printf("size defaults to %d, addrHi to 0x%x, addrLo to 0x%x (config ROM)\n",
		tranSize, addrHi, addrLo);
            exit(0);
        }
        guid = strtoq(argv[1], NULL, 0);
    }
    if(argc > 2) {
        dotest = strrchr(argv[2], 't');
        domap = strrchr(argv[2], 'm');
        doread = strrchr(argv[2], 'r');
        dowrite = strrchr(argv[2], 'w');
        dolock = strrchr(argv[2], 'l');
        dobusreset = strrchr(argv[2], 'b');
        doforever = strrchr(argv[2], 'f');
    }
    if(argc > 3)
        tranSize = strtol(argv[3], NULL, 0);
    if(argc > 4)
        addrHi = strtol(argv[4], NULL, 0);
    if(argc > 5)
        addrLo = (UInt32)strtoq(argv[5], NULL, 0);
    pokeDevices(guid);
}

