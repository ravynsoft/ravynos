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
 * Display statistics for an Ethernet interface.
 *
 * To build:
 * cc enetstat.c -o enetstat -framework IOKit -Wall
 */

#include <assert.h>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOCFSerialize.h>
#include <IOKit/IOKitLib.h>

#include <IOKit/network/IONetworkLib.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

// NetworkInterface property table keys.
//
#define IF_NAME_KEY     "BSD Name"

//---------------------------------------------------------------------------
// Display generic network statistics.

static IOReturn displayGenericStats(io_connect_t con)
{
    IOReturn         kr;
    IONetworkStats   netStat;
    UInt32           size;
    IONDHandle       dataHandle;

    size = sizeof(netStat);

    kr = IONetworkGetDataHandle(con, kIONetworkStatsKey, &dataHandle);
    if (kr != kIOReturnSuccess)
    {
        printf("IONetworkGetDataHandle error %x\n", kr);
        return kr;
    }

    kr = IONetworkReadData(con,
                           dataHandle,
                           (UInt8 *) &netStat,
                           &size);

    if (kr == kIOReturnSuccess) {
        printf("TX stats\t\t\tRX stats\n");
        printf("----------------------------------------------------------\n");
        printf("Packets         : %-7ld\tPackets          : %-7ld\n", 
            netStat.outputPackets,      netStat.inputPackets);
        printf("Errors          : %-7ld\tErrors           : %-7ld\n", 
            netStat.outputErrors,       netStat.inputErrors);
        printf("Collisions      : %-7ld\n", 
            netStat.collisions);
    }
    else {
        printf("IONetworkDataRead error %x\n", kr);
    }
    
    return kr;
}

//---------------------------------------------------------------------------
// Display Ethernet statistics.

static IOReturn displayEthernetStats(io_connect_t con)
{
    IOReturn                kr;
    IOEthernetStats         etherStat;
    IODot3StatsEntry *      dot3StatsEntry = &etherStat.dot3StatsEntry;
    IODot3RxExtraEntry *    dot3RxExtraEntry = &etherStat.dot3RxExtraEntry;
    IODot3TxExtraEntry *    dot3TxExtraEntry = &etherStat.dot3TxExtraEntry;
//  IODot3CollEntry *       col = &etherStat.dot3CollEntry;
    UInt32                  size;
    IONDHandle              dataHandle;

    size = sizeof(etherStat);

    kr = IONetworkGetDataHandle(con, kIOEthernetStatsKey, &dataHandle);
    if (kr != kIOReturnSuccess)
    {
        printf("IONetworkGetDataHandle error %x\n", kr);
        return kr;
    }

    kr = IONetworkReadData(con,
                           dataHandle,
                           (UInt8 *) &etherStat,
                           &size);

    if (kr == kIOReturnSuccess) {
        printf("\n");

        printf("TX stats\t\t\tRX stats\n");
        printf("----------------------------------------------------------\n");

        printf("Interrupts      : %-7ld\tInterrupts       : %-7ld\n", 
            dot3TxExtraEntry->interrupts,
            dot3RxExtraEntry->interrupts);

        printf("Single coll.    : %-7ld\tAlignment errors : %-7ld\n", 
            dot3StatsEntry->singleCollisionFrames,
            dot3StatsEntry->alignmentErrors);

        printf("Multi coll.     : %-7ld\tFCS errors       : %-7ld\n", 
            dot3StatsEntry->multipleCollisionFrames,
            dot3StatsEntry->fcsErrors);

        printf("Late coll.      : %-7ld\tToo Longs        : %-7ld\n", 
            dot3StatsEntry->lateCollisions,
            dot3StatsEntry->frameTooLongs);

        printf("Excessive coll. : %-7ld\tToo shorts       : %-7ld\n", 
            dot3StatsEntry->excessiveCollisions,
            dot3RxExtraEntry->frameTooShorts);

        printf("MAC errors      : %-7ld\tMAC errors       : %-7ld\n", 
            dot3StatsEntry->internalMacTransmitErrors,
            dot3StatsEntry->internalMacReceiveErrors);

        printf("PHY errors      : %-7ld\tPHY errors       : %-7ld\n", 
            dot3TxExtraEntry->phyErrors,
            dot3RxExtraEntry->phyErrors);

        printf("Deferred        : %-7ld\tWatchdog timeouts: %-7ld\n", 
            dot3StatsEntry->deferredTransmissions,
            dot3RxExtraEntry->watchdogTimeouts);

        printf("Carrier errors  : %-7ld\tCollisions       : %-7ld\n", 
            dot3StatsEntry->carrierSenseErrors,
            dot3RxExtraEntry->collisionErrors);
            
        printf("Underruns       : %-7ld\tOverruns         : %-7ld\n", 
            dot3TxExtraEntry->underruns,
            dot3RxExtraEntry->overruns);

        printf("Resets          : %-7ld\tResets           : %-7ld\n", 
            dot3TxExtraEntry->resets,
            dot3RxExtraEntry->resets);

        printf("Resource errors : %-7ld\tResource errors  : %-7ld\n", 
            dot3TxExtraEntry->resourceErrors,
            dot3RxExtraEntry->resourceErrors);

        printf("Timeouts        : %-7ld\tTimeouts         : %-7ld\n", 
            dot3TxExtraEntry->timeouts,
            dot3RxExtraEntry->timeouts);

        printf("SQE errors      : %-7ld\tMissed frames    : %-7ld\n", 
            dot3StatsEntry->sqeTestErrors,
            dot3StatsEntry->sqeTestErrors);

        printf("Jabbers         : %-7ld\n", 
            dot3TxExtraEntry->jabbers);
    }
    else {
        printf("IONetworkDataRead error %x\n", kr);
    }
    
    return kr;
}

//---------------------------------------------------------------------------
// Display output queue statistics.

static IOReturn displayOutputQueueStats(io_connect_t con)
{
    IOReturn             kr;
    IOOutputQueueStats   queueStat;
    UInt32               size;
    IONDHandle           dataHandle;

    size = sizeof(queueStat);

    kr = IONetworkGetDataHandle(con, kIOOutputQueueStatsKey, &dataHandle);
    if (kr != kIOReturnSuccess)
    {
        printf("IONetworkGetDataHandle error %x\n", kr);
        return kr;
    }

    kr = IONetworkReadData(con,
                           dataHandle,
                           (UInt8 *) &queueStat,
                           &size);

    if (kr == kIOReturnSuccess) {
        printf("\n");

        printf("Output queue stats\n");
        printf("----------------------------------------------------------\n");
        printf("Capacity        : %-7ld\n", 
            queueStat.capacity);
        printf("Current Size    : %-7ld\n", 
            queueStat.size);
        printf("Peak Size       : %-7ld\n", 
            queueStat.peakSize);
        printf("Dropped packets : %-7ld\n", 
            queueStat.dropCount);
        printf("Output packets  : %-7ld\n", 
            queueStat.outputCount);
        printf("Stall count     : %-7ld\n", 
            queueStat.stallCount);
    }
    else {
        printf("IONetworkDataRead error %x\n", kr);
    }

#if 0   // Test IONetworkGetDataCapacity
    {
    UInt32 capacity;
    kr = IONetworkGetDataCapacity(con, dataHandle, &capacity);
    if (kr != kIOReturnSuccess) {
        printf("IONetworkGetDataCapacity error %x size %ld\n", kr, size);
    }
    else {
        printf("Queue data capacity: %ld\n", capacity);
    }
    }
#endif

#if 0   // Test IONetworkResetData
    kr = IONetworkResetData(con, dataHandle);
    if (kr != kIOReturnSuccess)
        printf("IONetworkResetData error %x\n", kr);
#endif

#if 0
    {
        char wrBuf[44] = {0x15, 0x14, 0x15, 0xff, 0x15, 0x14, 0x15, 0xff,
        0x15, 0x14, 0x15, 0xff, 0x15, 0x14, 0x15, 0xff,
        0x15, 0x14, 0x15, 0xff, 0x15, 0x14, 0x15, 0xff,
        0x15, 0x14, 0x15, 0xff, 0x15, 0x14, 0x15, 0xff};
        
        kr = IONetworkWriteData(con, dataHandle, wrBuf, sizeof(wrBuf));
        if (kr == kIOReturnSuccess) {
            printf("IONetworkWriteData OK\n");
        }
        else {
            printf("IONetworkWriteData error %x\n", kr);
        }
    }
#endif

    return kr;
}

//---------------------------------------------------------------------------
// Utilities functions to print CFNumber and CFString.

static void CFStringShow(CFStringRef object)
{
    const char * c = CFStringGetCStringPtr(object, 
                                           kCFStringEncodingMacRoman);

    if (c)
        printf(c);
    else
    {
        CFIndex bufferSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(object),
		kCFStringEncodingMacRoman) + sizeof('\0');
        char *  buffer     = (char *) malloc(bufferSize);

        if (buffer)
        {
            if ( CFStringGetCString(
                                /* string     */ object,
                                /* buffer     */ buffer,
                                /* bufferSize */ bufferSize,
                                /* encoding   */ kCFStringEncodingMacRoman) )
                printf(buffer);

            free(buffer);
        }
    }
}

//---------------------------------------------------------------------------
// Returns non zero if the property table of the object has a "BSD Name"
// string entry, and the string matches the provided string argument.

static int matchBSDNameProperty(io_object_t obj, CFStringRef ifname)
{
    int                     match = 0;
    kern_return_t           kr;
    CFMutableDictionaryRef  properties;
    CFStringRef             string;

    kr = IORegistryEntryCreateCFProperties(obj,
                                           &properties,
                                           kCFAllocatorDefault,
                                           kNilOptions);

    if ((kr != KERN_SUCCESS) || !properties) {
        printf("IORegistryEntryCreateCFProperties error %x\n", kr);
        return false;
    }

    // Look up the interface "BSD Name" property and perform matching.
    //
    string = (CFStringRef) CFDictionaryGetValue(properties, 
                                                CFSTR(IF_NAME_KEY));
    if (string) {
        if (CFStringCompare(ifname, string, kNilOptions) == kCFCompareEqualTo)
        {
            printf("[ ");
            CFStringShow(string);
            printf(" ]\n");
            match = 1;
        }
    }

    CFRelease(properties);

    return match;
}

//---------------------------------------------------------------------------
// Searches the IOKit registry and return an interface object with the
// given BSD interface name, i.e. en0.

io_object_t getInterfaceWithBSDName(mach_port_t masterPort, CFStringRef ifname)
{
    kern_return_t   kr;
    io_iterator_t   ite;
    io_object_t     obj = 0;
    const char *    className = "IONetworkInterface";

    kr = IORegistryCreateIterator(masterPort,
                                  kIOServicePlane,
                                  true,             /* recursive */
                                  &ite);

    if (kr != kIOReturnSuccess) {
        printf("Error: IORegistryCreateIterator %x\n", kr);
        return 0;
    }

    while ((obj = IOIteratorNext(ite))) {
        if (IOObjectConformsTo(obj, (char *) className) &&
            (matchBSDNameProperty(obj, ifname)))
            break;

        IOObjectRelease(obj);
        obj = 0;
    }

    IORegistryDisposeEnumerator(ite);
    
    return obj;
}

//---------------------------------------------------------------------------
// Display usage and quit.

static void usage(void)
{
    /*
     * Print usage and exit.
     */
    printf("usage: enetstat [-i ifname]\n");
    printf("option flags:\n");
    printf("   -i ifname : specify interface. Default is en0.\n");
    printf("   -n        : Display generic network statistics.\n");
    printf("   -e        : Display Ethernet statistics.\n");
    printf("   -q        : Display output queue statistics.\n");
    printf("\n");
    exit(1);
}

//---------------------------------------------------------------------------
// Main function.

int
main(int argc, char ** argv)
{
    mach_port_t     masterPort;
    kern_return_t   kr;
    io_object_t     netif;
    io_connect_t    con;
    int             ch;
    const char *    name = "en0";
    CFStringRef     ifname;
    int             showGenericStats  = 0;
    int             showEthernetStats = 0;
    int             showQueueStats    = 0;

    while ((ch = getopt(argc, argv, ":i:nqe")) != -1)
    {
        switch ((char) ch)
        {
            case 'i':   /* specify interface */
                if (!optarg)
                    usage();
                name = optarg;
                break;

            case 'n':
                showGenericStats = 1;
                break;

            case 'q':
                showQueueStats = 1;
                break;

            case 'e':
                showEthernetStats = 1;
                break;

            default:
                usage();
        }
    }

    if (name == 0)
        usage();

    if ((showGenericStats || showEthernetStats || showQueueStats) == false)
        showGenericStats = 1;

    // Get master device port
    //
    kr = IOMasterPort(bootstrap_port, &masterPort);
    if (kr != KERN_SUCCESS)
        printf("IOMasterPort() failed: %x\n", kr);

    ifname = CFStringCreateWithCString(kCFAllocatorDefault,
                                       name,
                                       kCFStringEncodingMacRoman);

    netif = getInterfaceWithBSDName(masterPort, ifname);

    if (netif)
    {
        kr = IONetworkOpen(netif, &con);
        
        if (kr == kIOReturnSuccess)
        {
            if (showGenericStats)  displayGenericStats(con);
            if (showEthernetStats) displayEthernetStats(con);
            if (showQueueStats)    displayOutputQueueStats(con);
        }
        
        IONetworkClose(con);
        
        IOObjectRelease(netif);
    }

    exit(0);
    return 0;
}
