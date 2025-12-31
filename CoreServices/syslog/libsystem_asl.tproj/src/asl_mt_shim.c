/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
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

#include <asl_msg.h>
#include <os/lock_private.h>
#include <xpc/private.h>

#if TARGET_OS_OSX

#define MT_SHIM_SERVICE_NAME "com.apple.analyticsd.messagetracer"

static os_unfair_lock _mt_shim_lock = OS_UNFAIR_LOCK_INIT;
static xpc_pipe_t _mt_shim_pipe;

void
_asl_mt_shim_fork_child(void)
{
    if (_mt_shim_pipe) {
        xpc_pipe_invalidate(_mt_shim_pipe);
        xpc_release(_mt_shim_pipe);
        _mt_shim_pipe = NULL;
    }
    _mt_shim_lock = OS_UNFAIR_LOCK_INIT;
}

static xpc_pipe_t
_asl_mt_shim_pipe_copy(OS_OBJECT_CONSUMED xpc_pipe_t prev)
{
    xpc_pipe_t pipe;

    os_unfair_lock_lock_with_options(&_mt_shim_lock,
            OS_UNFAIR_LOCK_DATA_SYNCHRONIZATION);
    pipe = _mt_shim_pipe;
    if (prev) {
        if (pipe == prev) {
            xpc_release(pipe);
            pipe = NULL;
        }
        xpc_release(prev);
    }
    if (!pipe) {
        uint64_t flags = XPC_PIPE_PRIVILEGED | XPC_PIPE_USE_SYNC_IPC_OVERRIDE;
        _mt_shim_pipe = pipe = xpc_pipe_create(MT_SHIM_SERVICE_NAME, flags);
    }
    if (pipe) xpc_retain(pipe);
    os_unfair_lock_unlock(&_mt_shim_lock);
    return pipe;
}

static xpc_object_t
_asl_mt_shim_send_with_reply(xpc_object_t msg)
{
    xpc_object_t reply = NULL;
    xpc_pipe_t pipe = _asl_mt_shim_pipe_copy(NULL);

    if (pipe && xpc_pipe_routine(pipe, msg, &reply) == EPIPE) {
        pipe = _asl_mt_shim_pipe_copy(pipe);
        if (pipe) (void)xpc_pipe_routine(pipe, msg, &reply);
    }
    if (pipe) xpc_release(pipe);
    return reply;
}

void
_asl_mt_shim_send_message(asl_msg_t *msg)
{
    /* don't send messages that were already shimmed by the caller */
    const char *val = NULL;
    if ((asl_msg_lookup(msg, "com.apple.message.__source__", &val, NULL) == 0) && (val != NULL)) {
        if (!strcmp(val, "SPI")) return;
    }

    xpc_object_t xmsg = xpc_dictionary_create(NULL, NULL, 0);
    _asl_log_args_to_xpc(NULL, (asl_object_t)msg, xmsg);
    xpc_object_t reply = _asl_mt_shim_send_with_reply(xmsg);
    if (reply) xpc_release(reply);
    xpc_release(xmsg);
}

#endif /* TARGET_OS_OSX */
