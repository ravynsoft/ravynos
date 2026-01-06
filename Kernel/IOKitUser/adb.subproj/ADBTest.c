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
 cc -o ADBtest ADBTest.c -framework IOKit -Wall
 */
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include <mach/mach_port.h>
#include <mach/mach_interface.h>
#include <mach/mach_init.h>

//#include <IOKit/IOLib.h>

#include <IOKit/adb/IOADBLib.h>

#define DUT 0x01
#define DUTprime 0x0f

int
main(int argc, char **argv)
{
    mach_port_t		master_device_port;
    io_connect_t 	fb;
    kern_return_t	kr;
    IOReturn		err;
    unsigned char 	buffer[8];
    unsigned long	length;
    unsigned char 	bufferA[2];
    unsigned long	lengthA;
    unsigned char 	bufferB[2];
    unsigned long	lengthB;
    int		i;
    
    kr = IOMasterPort(bootstrap_port,&master_device_port);
    if ( kr == kIOReturnSuccess ) {
        fb = IOPMFindADBController(master_device_port);
        if ( fb != NULL ) {
            printf("ADB Controller contacted\n");					// claim tests
            err = IOPMClaimADBDevice (fb, 0 );	// bad address
            if ( err != (0xe0000000 + 706 ) ) {
                printf("claim bad address test failure (0), %08x, %d\n",err, (err & 0xfff));
            }
            err = IOPMClaimADBDevice (fb, 16 );	// bad address
            if ( err != (0xe0000000 + 706 ) ) {
                printf("claim bad address test failure (16), %08x, %d\n",err, (err & 0xfff));
            }
            err = IOPMClaimADBDevice (fb, 4 );	// doesn't exist			// **
            if ( err != (0xe0000000 + 704 ) ) {
                printf("claim non-existant device test failure (4), %08x, %d\n",err, (err & 0xfff));
            }
            err = IOPMClaimADBDevice (fb, 3 );	// already claimed by kernel	// **
            if ( err != (0xe0000000 + 709 ) ) {
                printf("claim owned device test failure (3), %08x, %d\n",err, (err & 0xfff));
            }
            err = IOPMClaimADBDevice (fb, DUT );	// should work
            if ( err != 0 ) {
                printf("claim test failure (%d), %08x, %d\n",DUT,err, (err & 0xfff));
            }
            err = IOPMClaimADBDevice (fb, DUT );	// already claimed by user
            if ( err != (0xe0000000 + 709 ) ) {
                printf("claim test failure (%d), %08x, %d\n",DUT,err, (err & 0xfff));
            }
            
            err = IOPMReleaseADBDevice (fb, 0 );	// bad address			// release tests
            if ( err != (0xe0000000 + 706 ) ) {
                printf("release bad address test failure (0), %08x, %d\n",err, (err & 0xfff));
            }
            err = IOPMReleaseADBDevice (fb, 16 );	// bad address
            if ( err != (0xe0000000 + 706 ) ) {
                printf("release bad address test failure (16), %08x, %d\n",err, (err & 0xfff));
            }
            err = IOPMReleaseADBDevice (fb, DUT );	// should work
            if ( err != 0 ) {
                printf("release test failure (%d), %08x, %d\n",DUT,err, (err & 0xfff));
            }
            err = IOPMReleaseADBDevice (fb, DUT );	// should not work
            if ( err != (0xe0000000 + 706 ) ) {
                printf("release unowned test failure (%d), %08x, %d\n",DUT,err, (err & 0xfff));
            }
          									  // read tests
            err = IOPMReadADBDevice (fb, 0, 0, buffer, &length );	// bad address
            if ( err != (0xe0000000 + 706 ) ) {
                printf("read bad address test failure (0), %08x, %d\n",err, (err & 0xfff));
            }
            err = IOPMReadADBDevice (fb, 16, 0, buffer, &length );	// bad address
            if ( err != (0xe0000000 + 706 ) ) {
                printf("read bad address test failure (16), %08x, %d\n",err, (err & 0xfff));
            }
            err = IOPMReadADBDevice (fb, 2, 4, buffer, &length );	// bad register
            if ( err != (0xe0000000 + 706 ) ) {
                printf("read bad register test failure (4), %08x, %d\n",err, (err & 0xfff));
            }
            err = IOPMReadADBDevice (fb, DUT, 3, buffer, &length );	// un owned
            if ( err != (0xe0000000 + 706 ) ) {
                printf("read unowned test failure (%d), %08x, %d\n",DUT,err, (err & 0xfff));
            }
            err = IOPMClaimADBDevice (fb, DUT );	// should work
            if ( err != 0 ) {
                printf("claim failure (%d), %08x, %d\n",DUT,err, (err & 0xfff));
            }
            length = 0;
            err = IOPMReadADBDevice (fb, DUT, 0, buffer, &length );			// register 0
            if ( err != 0 ) {
                printf("read test failure (%d,0), %08x, %d\n",DUT,err, (err & 0xfff));
            }
            else {
                printf("device %d register 0: length %d,",DUT,length);
                for ( i = 0; i < length; i++ ) {
                    printf(" %02x",buffer[i]);
                }
                printf("\n");
            }
            length = 0;
            err = IOPMReadADBDevice (fb, DUT, 1, buffer, &length );			// register 1
            if ( err != 0 ) {
                printf("read test failure (%d,1), %08x, %d\n",DUT,err, (err & 0xfff));
            }
            else {
                printf("device %d register 1: length %d,",DUT,length);
                for ( i = 0; i < length; i++ ) {
                    printf(" %02x",buffer[i]);
                }
                printf("\n");
            }
            length = 0;
            err = IOPMReadADBDevice (fb, DUT, 2, buffer, &length );			// register 2
            if ( err != 0 ) {
                printf("read test failure (%d,2), %08x, %d\n",DUT,err, (err & 0xfff));
            }
            else {
                printf("device %d register 2: length %d,",DUT,length);
                for ( i = 0; i < length; i++ ) {
                    printf(" %02x",buffer[i]);
                }
                printf("\n");
            }
            length = 0;
            err = IOPMReadADBDevice (fb, DUT, 3, buffer, &length );			// register 3
            if ( err != 0 ) {
                printf("read test failure (%d,3), %08x, %d\n",DUT,err, (err & 0xfff));
            }
            else {
                printf("device %d register 3: length %d,",DUT,length);
                for ( i = 0; i < length; i++ ) {
                    printf(" %02x",buffer[i]);
                }
                printf("\n");
            }
									// write tests
            err = IOPMWriteADBDevice (fb, 0, 0, buffer, length );	// bad address
            if ( err != (0xe0000000 + 706 ) ) {
                printf("write bad address test failure (0), %08x, %d\n",err, (err & 0xfff));
            }
            err = IOPMWriteADBDevice (fb, 16, 0, buffer, length );	// bad address
            if ( err != (0xe0000000 + 706 ) ) {
                printf("write bad address test failure (16), %08x, %d\n",err, (err & 0xfff));
            }
            err = IOPMWriteADBDevice (fb, 2, 4, buffer, length );	// bad register
            if ( err != (0xe0000000 + 706 ) ) {
                printf("write bad register test failure (4), %08x, %d\n",err, (err & 0xfff));
            }
            err = IOPMWriteADBDevice (fb, 3, 3, buffer, length );	// un owned
            if ( err != (0xe0000000 + 706 ) ) {
                printf("write unowned test failure (3), %08x, %d\n",err, (err & 0xfff));
            }
#ifdef q8q
// to do the following write test, the two lines marked ** above should be commented, and
// IOADBController::claimDevice must be altered to not check for device existance or kernel ownership
            bufferA[0] = DUTprime;
            bufferA[1] = 0xfe;
            lengthA = 2;
            err = IOPMWriteADBDevice (fb, DUT, 3, bufferA, lengthA );			// try to move the device
            if ( err != 0 ) {								// to address F
                printf("write test failure (%d,3), %08x, %d\n",DUT,err, (err & 0xfff));
            }
            err = IOPMClaimADBDevice (fb, DUTprime );	// should work
            if ( err != 0 ) {
                printf("claim failure (%d), %08x, %d\n",DUTprime,err, (err & 0xfff));
            }
            lengthB = 0;
            err = IOPMReadADBDevice (fb, DUTprime, 3, bufferB, &lengthB );			// verify it
            if ( err != 0 ) {
                printf("read failure (%d,3), %08x, %d\n",DUTprime,err, (err & 0xfff));
            }
            else {
                if ( (buffer[1] != bufferB[1]) || (length != lengthB) ) {
                    printf("write verify failure (%d,3), register 3: length %d, %02x %02x\n",DUT,lengthB,bufferB[0],bufferB[1]);
                }
            }
            bufferA[0] = DUT;
            err = IOPMWriteADBDevice (fb, DUTprime, 3, bufferA, lengthA );			// move it back
            if ( err != 0 ) {
                printf("write failure (%d,3), %08x, %d\n",DUTprime,err, (err & 0xfff));
            }
            lengthB = 0;
            err = IOPMReadADBDevice (fb, DUT, 3, bufferB, &lengthB );			// verify it
            if ( err != 0 ) {
                printf("read failure (%d,3), %08x, %d\n",DUT,err, (err & 0xfff));
            }
            else {
                if ( (buffer[1] != bufferB[1]) || (length != lengthB) ) {
                    printf("restore verify failure (%d,3), register 3: length %d, %02x %02x\n",DUT,lengthB,bufferB[0],bufferB[1]);
                }
            }
            err = IOPMReleaseADBDevice (fb, DUTprime );	// should work
            if ( err != 0 ) {
                printf("release failure (%d), %08x, %d\n",DUTprime,err, (err & 0xfff));
            }
#endif
            err = IOPMReleaseADBDevice (fb, DUT );	// should work
             if ( err != 0 ) {
                 printf("release failure (%d), %08x, %d\n",DUT,err, (err & 0xfff));
             }
            printf("done\n");
            return 1;
        }
    }
    printf("That didn't work for some reason\n");
    return 0;
}

