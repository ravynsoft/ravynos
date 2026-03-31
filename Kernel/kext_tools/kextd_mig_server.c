/*
 * Copyright (c) 2000-2008 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
#include <CoreFoundation/CoreFoundation.h>

#include <mach/mach.h>
#include <mach/mach_port.h>
#include <servers/bootstrap.h>
#include <sysexits.h>

#include <IOKit/kext/OSKext.h>
#include "kext_tools_util.h"
#include "kextd_globals.h"
#include "kextd_mig_server.h"

/* mig-generated externals and functions */
extern struct mig_subsystem _kextmanager_subsystem;
extern boolean_t kextmanager_server(mach_msg_header_t *, mach_msg_header_t *);

extern struct mig_subsystem svc_kextd_kernel_request_subsystem;
extern boolean_t kextd_kernel_request_server(
        mach_msg_header_t *InHeadP,
        mach_msg_header_t *OutHeadP);

uid_t gClientUID = -1;

/*******************************************************************************
*******************************************************************************/
boolean_t kextd_demux(
    mach_msg_header_t * request,
    mach_msg_header_t * reply)
{
    boolean_t processed = FALSE;

    mach_msg_format_0_trailer_t * trailer;

   /* Feed the request into the ("MiG" generated) server. We have two
    * subsystems, one for user-space clients, one for the kernel.
    */
    if (!processed) {
        if (request->msgh_id >= _kextmanager_subsystem.start &&
            request->msgh_id < _kextmanager_subsystem.end) {

            /*
             * Get the caller's credentials (eUID/eGID) from the message trailer.
             */
            trailer = (mach_msg_security_trailer_t *)((vm_offset_t)request +
                round_msg(request->msgh_size));

            if ((trailer->msgh_trailer_type == MACH_MSG_TRAILER_FORMAT_0) &&
               (trailer->msgh_trailer_size >= MACH_MSG_TRAILER_FORMAT_0_SIZE)) {

                gClientUID = trailer->msgh_sender.val[0];

                OSKextLog(/* kext */ NULL,
                    kOSKextLogDebugLevel | kOSKextLogIPCFlag,
                    "MIG message received: caller has eUID = %d, eGID = %d.",
                    trailer->msgh_sender.val[0],
                    trailer->msgh_sender.val[1]);

            } else {
                OSKextLog(/* kext */ NULL,
                    kOSKextLogWarningLevel | kOSKextLogIPCFlag,
                    "Caller's credentials not available.");
                gClientUID = -1;

            }

           /* Process user task requests.
            */
            processed = kextmanager_server(request, reply);

        } else if (request->msgh_id >= svc_kextd_kernel_request_subsystem.start &&
            request->msgh_id < svc_kextd_kernel_request_subsystem.end) {

           /* Process kernel requests.
            */
            processed = kextd_kernel_request_server(request, reply);
        }
    }

    if (!processed) {
        if (request->msgh_id >= MACH_NOTIFY_FIRST &&
            request->msgh_id < MACH_NOTIFY_LAST) {

            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogIPCFlag,
                "Failed to process MIG message.");
        } else {
            OSKextLog(/* kext */ NULL,
                kOSKextLogErrorLevel | kOSKextLogIPCFlag,
                "Unknown MIG message received.");
        }
    }

    gClientUID = (uid_t)-1;

    return processed;
}

/*******************************************************************************
*******************************************************************************/
void kextd_mach_port_callback(
    CFMachPortRef port,
    void *msg,
    CFIndex size,
    void *info)
{
    mig_reply_error_t * bufRequest = msg;
    mig_reply_error_t * bufReply = (mig_reply_error_t *)malloc(_kextmanager_subsystem.maxsize);
    mach_msg_return_t   mr;
    int                 options;

    if (!bufReply) {
        OSKextLog(/* kext */ NULL,
                  kOSKextLogErrorLevel | kOSKextLogIPCFlag,
                  "No memory for mach_msg reply!");
        return;
    }

    memset(bufReply, 0, _kextmanager_subsystem.maxsize);
    bufReply->RetCode = MIG_BAD_ID;

    /* we have a request message */
    (void) kextd_demux(&bufRequest->Head, &bufReply->Head);

    if (!(bufReply->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) &&
        (bufReply->RetCode != KERN_SUCCESS)) {

        if (bufReply->RetCode == MIG_NO_REPLY) {
            /*
             * This return code is a little tricky -- it appears that the
             * demux routine found an error of some sort, but since that
             * error would not normally get returned either to the local
             * user or the remote one, we pretend it's ok.
             */
            free(bufReply);
            return;
        }

        /*
         * destroy any out-of-line data in the request buffer but don't destroy
         * the reply port right (since we need that to send an error message).
         */
        bufRequest->Head.msgh_remote_port = MACH_PORT_NULL;
        mach_msg_destroy(&bufRequest->Head);
    }

    if (bufReply->Head.msgh_remote_port == MACH_PORT_NULL) {
        /* no reply port, so destroy the reply */
        if (bufReply->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) {
            mach_msg_destroy(&bufReply->Head);
        }
        free(bufReply);
        return;
    }

    /*
     * send reply.
     *
     * We don't want to block indefinitely because the client
     * isn't receiving messages from the reply port.
     * If we have a send-once right for the reply port, then
     * this isn't a concern because the send won't block.
     * If we have a send right, we need to use MACH_SEND_TIMEOUT.
     * To avoid falling off the kernel's fast RPC path unnecessarily,
     * we only supply MACH_SEND_TIMEOUT when absolutely necessary.
     */

    options = MACH_SEND_MSG;
    if (MACH_MSGH_BITS_REMOTE(bufReply->Head.msgh_bits) == MACH_MSG_TYPE_MOVE_SEND_ONCE) {
        options |= MACH_SEND_TIMEOUT;
    }
    mr = mach_msg(&bufReply->Head,        /* msg */
              options,            /* option */
              bufReply->Head.msgh_size,    /* send_size */
              0,            /* rcv_size */
              MACH_PORT_NULL,        /* rcv_name */
              MACH_MSG_TIMEOUT_NONE,    /* timeout */
              MACH_PORT_NULL);        /* notify */


    /* Has a message error occurred? */
    switch (mr) {
        case MACH_SEND_INVALID_DEST:
        case MACH_SEND_TIMED_OUT:
            /* the reply can't be delivered, so destroy it */
            mach_msg_destroy(&bufReply->Head);
            break;

        default :
            /* Includes success case.  */
            break;
    }

    free(bufReply);
}
