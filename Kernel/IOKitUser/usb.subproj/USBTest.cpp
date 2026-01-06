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
 cc -fno-rtti -fno-exceptions -fcheck-new -fvtable-thunks -I.. -I../../BUILD/obj/EXPORT_HDRS/libkern -I../../BUILD/obj/EXPORT_HDRS/osfmk USBTest.cpp ../../BUILD/obj/RELEASE_PPC/IOKit/User/libIOKit.A.dylib -o USBTest
*/

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include <IOKit/IOLib.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOContainers.h>
#include <IOKit/IOSerialize.h>
#include <IOKit/usb/IOUSBLib.h>

mach_port_t	master_device_port;

void printUSBInterface(IOUSBInterface *intf)
{
    int i;
    int nEP = intf->descriptor->numEndpoints;
    printf("%d endpoints\n", nEP);

    for(i=0; i<nEP; i++) {
        IOUSBEndpoint *ep = intf->endpoints[i];
        printf("ep #%d, dir %d, type %d, maxPack %d\n",
            ep->number, ep->direction, ep->transferType,
            ep->maxPacketSize);
    }
}

void processDevice(IOUSBIteratorRef enumer)
{
    kern_return_t		kr2;
    IOUSBDeviceDescriptor	desc;
    IOUSBInterfaceDescriptor	intf;

    kr2 = IOUSBGetDeviceDescriptor(enumer, &desc, sizeof(desc));
    if (kr2) {
        printf("IOUSBGetDeviceDescriptor failed, kr=%d\n", kr2);
            return;
    }
    printf("Device Descriptor\n------------\n");
    printf("length = %d\n", desc.length);
    printf("descType = %d\n", desc.descType);
    printf("usbRel = %d\n", desc.usbRel);
    printf("deviceClass = %d\n", desc.deviceClass);
    printf("deviceSubClass = %d\n", desc.deviceSubClass);
    printf("protocol = %d\n", desc.protocol);
    printf("maxPacketSize = %d\n", desc.maxPacketSize);
    printf("vendor = %d\n", desc.vendor);
    printf("product = %d\n", desc.product);
    printf("devRel = %d\n", desc.devRel);
    printf("manuIdx = %d\n", desc.manuIdx);
    printf("prodIdx = %d\n", desc.prodIdx);
    printf("serialIdx = %d\n", desc.serialIdx);
    printf("numConf = %d\n", desc.numConf);



    kr2 = IOUSBGetInterfaceDescriptor(enumer, &intf, sizeof(intf));
    if (kr2) {
        UInt8 conf = 123;
        IOUSBDeviceRef dev;
        printf("IOUSBGetInterfaceDescriptor failed, kr=%x\n", kr2);

        kr2 = IOUSBNewDeviceRef( enumer, &dev);

        if(kIOReturnSuccess != kr2) {
            printf("IOUSBNewDeviceRef: err = %x\n", kr2);
        }
        else {
            kr2 = IOUSBGetConfiguration(dev, &conf);
            if(kIOReturnSuccess == kr2)
                printf("IOUSBGetConfiguration: conf = %d\n", conf);
            else
                printf("IOUSBGetConfiguration: err = %x\n", kr2);
            if(0 == conf || kIOReturnSuccess != kr2) {
                conf = 1;
                kr2 = IOUSBSetConfiguration(dev, conf);
                if(kIOReturnSuccess == kr2)
                    printf("IOUSBSetConfiguration: conf = %d\n", conf);
                else
                    printf("IOUSBSetConfiguration: err = %x\n", kr2);
                kr2 = IOUSBGetConfiguration(dev, &conf);
                if(kIOReturnSuccess == kr2)
                    printf("IOUSBGetConfiguration: conf = %d\n", conf);
                else
                    printf("IOUSBGetConfiguration: err = %x\n", kr2);
            }
            // Get full configuration
            char hack[1024];
            IOUSBConfigurationDescriptorPtr conf;
            kr2 = IOUSBGetConfigDescriptor(dev, 0, &conf);
            if(kIOReturnSuccess == kr2) {
                printf("IOUSBGetConfigDescriptor(%d) %x %x %x %x %x %x %x\n",
                 USBToHostWord(conf->totalLength), conf->length,
 			conf->descriptorType, conf->numInterfaces,
			conf->configValue,
                        conf->configStrIndex, conf->attributes, conf->maxPower);
            }
            else
                printf("IOUSBGetConfigDescriptor: err = %x\n", kr2);

            // Try to get first string descriptor
            UInt16 size = sizeof(hack);
            kr2 = IOUSBDeviceRequest(dev,
                USBMakeBMRequestType(kUSBIn, kUSBStandard, kUSBDevice),
                kUSBRqGetDescriptor, (kUSBStringDesc << 8) | 1, 0x409,
                hack, &size);
            if(kIOReturnSuccess == kr2) {
                int i;
                printf("String 1(%d/%d):", size, hack[0]);
                for(i=2; i<hack[0]; i+=2)
                    printf("%c", hack[i]);
                printf("\n");
            }
            else
                printf("IOUSBDeviceRequest: err = %x\n", kr2);

            IOUSBInterface *ioint;
            IOUSBFindInterfaceRequest freq;

            freq.theClass = 0;		// requested class,    0 = don't care
            freq.subClass = 0;		// requested subclass; 0 = don't care
            freq.protocol = 0;		// requested protocol; 0 = don't care
            freq.busPowered = 0;	// 1 = not bus powered, 2 = bus powered,
                                        // 0 = don't care
            freq.selfPowered = 0;	// 1 = not self powered, 2 = self powered,
                                        // 0 = don't care
            freq.remoteWakeup = 0;	// 1 = doesn't support remote wakeup; 2 = does
                                        // 0 = don't care
            freq.reserved = 0;
            freq.maxPower = 0;		// max power in 2ma increments; 0 = don't care
            freq.alternateIndex = 0;	// alternate #; 0xff = find first only
            freq.configIndex = 0;	// 0 = start at beginning
            freq.interfaceIndex = 0;	// 0 = start at beginning

            while(ioint = IOUSBFindNextInterface(dev, &freq)) {
                printUSBInterface(ioint);

                IOUSBDisposeInterface(ioint);
            }
            // test vendor specific read/write for ISVF
            if(desc.vendor == 0xc706) {
                printf("Found ImageStudioVF!\n");
                size = 3;
                kr2 = IOUSBDeviceRequest(dev,
                    USBMakeBMRequestType(kUSBIn, kUSBVendor, kUSBDevice),
                    4, 0, 0,
                    hack, &size);
                if(kIOReturnSuccess == kr2) {
                    int i;
                    printf("Read %d:", size);
                    for(i=0; i<size; i++)
                        printf("%d ", hack[i]);
                    printf("\n");
                }
                else {
                    printf("IOUSBDeviceRequest: err = %x\n", kr2);
                    return;
                }
                size = 3;
                hack[0]++;
                hack[1] += 2;
                hack[2] += 3;
                kr2 = IOUSBDeviceRequest(dev,
                    USBMakeBMRequestType(kUSBOut, kUSBVendor, kUSBDevice),
                    4, 0, 0,
                    hack, &size);
                if(kIOReturnSuccess != kr2) {
                    printf("IOUSBDeviceRequest: err = %x\n", kr2);
                    return;
                }
                size = 3;
                kr2 = IOUSBDeviceRequest(dev,
                    USBMakeBMRequestType(kUSBIn, kUSBVendor, kUSBDevice),
                    4, 0, 0,
                    hack, &size);
                if(kIOReturnSuccess == kr2) {
                    int i;
                    printf("Now Read %d:", size);
                    for(i=0; i<size; i++)
                        printf("%d ", hack[i]);
                    printf("\n");
                }
                else {
                    printf("IOUSBDeviceRequest: err = %x\n", kr2);
                    return;
                }

            }
            IOUSBDisposeRef(dev);
        }
    }
    else {
        printf("Interface Descriptor\n------------\n");
        printf("length = %d\n", intf.length);
        printf("descType = %d\n", intf.descriptorType);
        printf("interfaceNumber = %d\n", intf.interfaceNumber);
        printf("alternateSetting = %d\n", intf.alternateSetting);
        printf("numEndpoints = %d\n", intf.numEndpoints);
        printf("interfaceClass = %d\n", intf.interfaceClass);
        printf("interfaceSubClass = %d\n", intf.interfaceSubClass);
        printf("interfaceProtocol = %d\n", intf.interfaceProtocol);
        printf("interfaceStrIndex = %d\n", intf.interfaceStrIndex);

        // See if we have a HID interface, if so open and read the interupt pipe a few times.
        if(intf.interfaceClass == kUSBHIDClass) {
            char hack[1024];
            UInt16 size;
            IOUSBDeviceRef dev;
            IOUSBPipeRef pipe;
            int i;

            printf("HID Interface\n");
            kr2 = IOUSBNewDeviceRef( enumer, &dev);
            if(kIOReturnSuccess != kr2)
                return;
            /*
             * If a boot device, SetProtocol to Boot protocol
             * SetIdle to read complete after 24mSec
             * Open interface's first pipe
             * Read 8 bytes from pipe
             * Close pipe
             */
            size = 0;
            if(1 == intf.interfaceSubClass) {	// 1 = Boot Interface subclass
                kr2 = IOUSBDeviceRequest(dev,
                    USBMakeBMRequestType(kUSBOut, kUSBClass, kUSBInterface),
                    kHIDRqSetProtocol, kHIDBootProtocolValue,
		    intf.interfaceNumber,
                    NULL, &size);
            }
            if(kIOReturnSuccess != kr2) {
                printf("HIDRqSetProtocol: err = %x\n", kr2);
                return;
            }
#if 0
            size = 0;
            kr2 = IOUSBDeviceRequest(dev,
                USBMakeBMRequestType(kUSBOut, kUSBClass, kUSBInterface),
                kHIDRqSetIdle, (24/4)<<8, intf.interfaceNumber,
                NULL, &size);
            if(kIOReturnSuccess != kr2) {
                printf("HIDRqSetIdle: err = %x\n", kr2);
                continue;
            }
#endif
            size = 4;
            kr2 = IOUSBDeviceRequest(dev,
                USBMakeBMRequestType(kUSBIn, kUSBClass, kUSBInterface),
                kHIDRqGetReport, kHIDRtInputReport<<8,
                intf.interfaceNumber,
                hack, &size);
            if(kIOReturnSuccess != kr2) {
                printf("HIDRqGetReport: err = %x\n", kr2);
                return;
            }
            printf("Read %d:%d %d %d %d\n", size,
                hack[0], hack[1], hack[2], hack[3]);

            IOUSBInterface *ioint;
            IOUSBFindInterfaceRequest freq;
            IOUSBEndpoint *ep;

            freq.theClass = intf.interfaceClass;
            freq.subClass = intf.interfaceSubClass;
            freq.protocol = intf.interfaceProtocol;
            freq.busPowered = 0;
            freq.selfPowered = 0;
            freq.remoteWakeup = 0;
            freq.reserved = 0;
            freq.maxPower = 0;
            freq.alternateIndex = 0;
            freq.configIndex = 0;
            freq.interfaceIndex = 0;

            ioint = IOUSBFindNextInterface(dev, &freq);
            if(!ioint) {
                printf("No HID Interfaces found!!\n");
                return;
            }
            printUSBInterface(ioint);
            ep = 0;
            for(i=0; i < ioint->descriptor->numEndpoints; i++) {
                if(ioint->endpoints[i]->direction == kUSBIn &&
                    ioint->endpoints[i]->transferType == kUSBInterrupt) {
                    ep = ioint->endpoints[i];
                    break;
                }
            }
            if(ep) {
                kr2 = IOUSBOpenPipe(dev, ep, &pipe);
                if(kIOReturnSuccess != kr2) {
                    printf("IOUSBOpenPipe: err = %x\n", kr2);
                    IOUSBDisposeRef(dev);
                    return;
                }
                for(int i=0; i<4; i++) {
                    UInt32 len = 4;
                    kr2 = IOUSBReadPipe(pipe, hack, &len);
                    if(kIOReturnSuccess != kr2) {
                        printf("IOUSBReadPipe: err = %x\n", kr2);
                        break;
                    }
                    else {
                        printf("Read %d:%d %d %d %d\n", len,
                            hack[0], hack[1], hack[2], hack[3]);
                    }
                }
                IOUSBClosePipe(pipe);
            }
            IOUSBDisposeInterface(ioint);
            IOUSBDisposeRef(dev);
        }
    }
}
int pokeDevices(IOUSBMatch * descMatch)
{
    kern_return_t	kr;
    IOUSBIteratorRef	enumer;
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

    kr = IOUSBCreateIterator(master_device_port, port, descMatch, &enumer);
    if(kIOReturnSuccess != kr) {
        printf("IOUSBCreateIterator = %d\n", kr);
        return kr;
    }

    // First process any devices already connected
    while(kIOReturnSuccess == IOUSBIteratorNext(enumer)) {
        processDevice(enumer);
    }

    // Now wait for a new device (or devices) to appear and
    // process them.
    kr = mach_msg(&msg.msgHdr, MACH_RCV_MSG,
                0, sizeof(msg), port, 0, MACH_PORT_NULL);
    kr = OSGetNotificationFromMessage( &msg.msgHdr, 0, &notifyType, &ref,
                    0, 0 );

    if(ref == (unsigned long int)enumer) {
        while(kIOReturnSuccess == IOUSBIteratorNext(enumer)) {
            processDevice(enumer);
        }
    }
    else
        printf("Unexpected notification type %d, ref %d\n",
		notifyType, ref);
    kr = IOUSBDisposeIterator( enumer);
    return kr;
}

int
main(int argc, char **argv)
{
    kern_return_t	kr;

    IOUSBMatch match = {
    kIOUSBAnyClass,
    kIOUSBAnySubClass,
    kIOUSBAnyProtocol,
    kIOUSBAnyVendor,
    kIOUSBAnyProduct
    };

    if(argc > 1)
        match.usbClass = atoi(argv[1]);
    if(argc > 2)
        match.usbSubClass = atoi(argv[2]);
    if(argc > 3)
        match.usbProtocol = atoi(argv[3]);
    if(argc > 4)
        match.usbVendor = atoi(argv[4]);
    if(argc > 5)
        match.usbProduct = atoi(argv[5]);

    /*
     * Get master device port
     */
    kr = IOMasterPort(bootstrap_port,
                              &master_device_port);

    if (kr != KERN_SUCCESS)
        printf("IOMasterPort failed: %lx\n", kr);

    printf("Looking for devices matching class=%d subclass=%d ",
        match.usbClass, match.usbSubClass);
    printf("protocol=%d vendor=%d product=%d\n", match.usbProtocol,
        match.usbVendor, match.usbProduct);
    pokeDevices(&match);
}

