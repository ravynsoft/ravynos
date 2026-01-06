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
 * Display the link status properties of a network interface.
 *
 * To build:
 * cc monitorlink.c -o monitorlink -framework IOKit -Wall
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

#include <mach/mach.h>

//---------------------------------------------------------------------------
// Property table keys.

#define IF_NAME_KEY              "BSD Name"
#define kIOMediumDictionary      "IOMediumDictionary"
#define kIODefaultMedium         "IODefaultMedium"
#define kIOCurrentMedium         "IOCurrentMedium"
#define kIOActiveMedium          "IOActiveMedium"
#define kIOLinkSpeed             "IOLinkSpeed"
#define kIOLinkStatus            "IOLinkStatus"
#define kIOLinkData              "IOLinkData"

//---------------------------------------------------------------------------
// Utilities functions to print CFNumber and CFString.

static void CFNumberShow(CFNumberRef object)
{
    long long number = 0;

    if (CFNumberGetValue(object, kCFNumberLongLongType, &number))
    {
        printf("%qd", number);
    }
}

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

    return match;   // true if matched
}

//---------------------------------------------------------------------------
// Searches the IOKit registry and return an interface object with the
// given BSD interface name, i.e. en0.

static io_object_t
getInterfaceWithBSDName(mach_port_t masterPort, CFStringRef ifname)
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
        printf("IORegistryCreateIterator() error %x\n", kr);
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
// Display the link related properties for an IONetworkController object.

static void
printLinkProperties(io_object_t controller)
{
    kern_return_t           kr;
    CFMutableDictionaryRef  properties = 0;
    CFStringRef             string;
    CFNumberRef             number;

    do {
        kr = IORegistryEntryCreateCFProperties(controller,
                                               &properties,
                                               kCFAllocatorDefault,
                                               kNilOptions);
        if (kr != kIOReturnSuccess) {
            printf("Error: cannot get properties %x\n", kr);
            break;
        }
        
        // Print kIOActiveMedium string.
        //
        string = (CFStringRef) CFDictionaryGetValue(properties, 
                                                    CFSTR(kIOActiveMedium));
        printf("Active  medium : ");
        if (string) {
            CFStringShow(string);
            printf("\n");
        }
        else {
            printf("None\n");
        }

        // Print kIOCurrentMedium string.
        //
        string = (CFStringRef) CFDictionaryGetValue(properties, 
                                                    CFSTR(kIOCurrentMedium));
        printf("Current medium : ");
        if (string) {
            CFStringShow(string);
            printf("\n");
        }
        else {
            printf("None\n");
        }

        // Print kIOLinkSpeed number.
        //
        number = (CFNumberRef) CFDictionaryGetValue(properties, 
                                                    CFSTR(kIOLinkSpeed));
        if (number) {
            printf("Link speed bps : ");
            CFNumberShow(number);
            printf("\n");
        }

        // Print kIOLinkSpeed number.
        //
        number = (CFNumberRef) CFDictionaryGetValue(properties, 
                                                    CFSTR(kIOLinkStatus));
        if (number) {
            long status;

            if (CFNumberGetValue(number, kCFNumberLongType, &status))
            {
                printf("Link status    : ");
            
                if (status & kIONetworkLinkValid)
                {
                    printf("%s\n", (status & kIONetworkLinkActive) ? "Active" : 
                        "Inactive");
                }
                else
                    printf("Not reported\n");
            }
        }
    }
    while (0);
    
    if (properties)
        CFRelease(properties);
}

//---------------------------------------------------------------------------
// Get the parent object (a network controller) of an interface object.

static io_object_t
getControllerForInterface(io_object_t netif)
{
    io_iterator_t    ite;
    kern_return_t    kr;
    io_object_t      controller = 0;
    
    // We have the interface, we need its parent, the network controller.

    kr = IORegistryEntryGetParentIterator(netif, kIOServicePlane, &ite);
    if (kr == kIOReturnSuccess) {
        controller = IOIteratorNext(ite);   // the first entry
        IORegistryDisposeEnumerator(ite);
    }
    
    return controller;  // caller must release this object
}

//---------------------------------------------------------------------------
// Returns when a mach message is received on the port specified.

static void
waitForNotification(mach_port_t port)
{
    kern_return_t   kr;

    struct {
        IONetworkNotifyMsg      hdr;
        mach_msg_trailer_t      trailer;
    } msg;

    // Now wait for a notification.
    //
    kr = mach_msg(&msg.hdr.h, MACH_RCV_MSG,
                  0, sizeof(msg), port, 0, MACH_PORT_NULL);

    if (kr != KERN_SUCCESS)
        printf("Error: mach_msg %x\n", kr);
//  else
//      printf("\n\n[message id=%x]\n", msg.hdr.h.msgh_id);
}

//---------------------------------------------------------------------------
// Display usage and quit.

static void usage(void)
{
    /*
     * Print usage and exit.
     */
    printf("usage: monitorlink [-i ifname] [-w]\n");
    printf("option flags:\n");
    printf("   -i ifname : specify interface. Default is en0.\n");
    printf("   -w        : wait for link event notification.\n");
    printf("\n");
    exit(1);
}

//---------------------------------------------------------------------------
// Main function.

int
main(int argc, char ** argv)
{
    mach_port_t     masterPort;
    mach_port_t     notifyPort;
    kern_return_t   kr;
    io_object_t     netif;
    io_object_t     controller;
    io_connect_t    con;
    int             ch;
    const char *    name = "en0";   // default interface name
    CFStringRef     ifname;
    int             wait = 0;

    while ((ch = getopt(argc, argv, ":i:w")) != -1)
    {
        switch ((char) ch)
        {
            case 'i':   /* specify interface */
                if (!optarg)
                    usage();
                name = optarg;
                break;

            case 'w':   /* specify interface */
                wait = 1;
                break;

            default:
                usage();
        }
    }

    if (name == 0)
        usage();

    // Get master device port
    //
    kr = IOMasterPort(bootstrap_port, &masterPort);
    if (kr != KERN_SUCCESS)
        printf("IOMasterPort() failed: %x\n", kr);


    // Create a mach port to receive media/link change notifications.
    //
    kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE,
                            &notifyPort);
    if (kr != KERN_SUCCESS) {
        printf("Error: mach_port_allocate %x\n", kr);
        exit(1);
    }

    ifname = CFStringCreateWithCString(kCFAllocatorDefault,
                                       name,
                                       kCFStringEncodingMacRoman);

    netif = getInterfaceWithBSDName(masterPort, ifname);

    if (netif)
    {
        kr = IONetworkOpen(netif, &con);
        if (kr != kIOReturnSuccess)
        {
            printf("Error: IONetworkOpen error %x\n", kr);
            exit(1);
        }

        kr = IOConnectSetNotificationPort(con,
             kIONUCNotificationTypeLinkChange, notifyPort, 0);
        if (kr != kIOReturnSuccess) {
            printf("Error: IOConnectSetNotificationPort %x\n", kr);
            exit(1);
        }

        controller = getControllerForInterface(netif);
        
        if (controller)
        {
            do {
                printLinkProperties(controller);
                printf("\n");

                if (wait)
                    waitForNotification(notifyPort);
            }
            while (wait);

            IOObjectRelease(controller);
        }

        IONetworkClose(con);
        IOObjectRelease(netif);
    }

    exit(0);
    return 0;
}
