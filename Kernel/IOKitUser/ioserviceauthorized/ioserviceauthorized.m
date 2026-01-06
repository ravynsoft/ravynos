/*
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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

#include "open.h"

#include <libproc.h>
#include <xpc/private.h>
#include <IOKit/IOKitLib.h>

static void __MessageCallback( xpc_connection_t connection, xpc_object_t message, uint64_t authorizationID )
{
    xpc_type_t type;

    type = xpc_get_type( message );

    if ( type == XPC_TYPE_DICTIONARY )
    {
        int64_t oflag;
        uint64_t serviceID;
        io_service_t service;
        int fd;
        IOReturn status;
        xpc_object_t reply;

        oflag = xpc_dictionary_get_int64( message, "oflag" );
        serviceID = xpc_dictionary_get_uint64( message, "service" );

        service = IOServiceGetMatchingService( kIOMasterPortDefault, IORegistryEntryIDMatching( serviceID ) );

        if ( service )
        {
            fd = _Open( service, oflag, authorizationID );

            if ( fd == -1 )
            {
                status = unix_err( errno );
            }
            else
            {
                status = kIOReturnSuccess;
            }

            IOObjectRelease( service );
        }
        else
        {
            status = unix_err( EINVAL );
        }

        reply = xpc_dictionary_create_reply( message );

        if ( reply )
        {
            xpc_dictionary_set_uint64( reply, "status", status );

            if ( status == kIOReturnSuccess )
            {
               xpc_dictionary_set_fd( reply, "fd", fd );
            }

            xpc_connection_send_message( connection, reply );
        }

        if ( status == kIOReturnSuccess )
        {
            close( fd );
        }
    }
}

static void __ConnectionCallback( xpc_connection_t connection )
{
    pid_t processID;

    processID = xpc_connection_get_pid( connection );

    if ( processID )
    {
        struct proc_uniqidentifierinfo authorizationID = { };

        proc_pidinfo( processID, PROC_PIDUNIQIDENTIFIERINFO, 0, &authorizationID, sizeof( authorizationID ) );

        if ( authorizationID.p_uniqueid )
        {
            xpc_connection_set_event_handler( connection, ^( xpc_object_t message )
            {
                __MessageCallback( connection, message, authorizationID.p_uniqueid );
            } );

            xpc_connection_resume( connection );

            return;
        }
    }

    xpc_connection_cancel( connection );
}

int main( int argc __unused, const char * argv[ ] __unused )
{
    xpc_main( __ConnectionCallback );

    return 0;
}
