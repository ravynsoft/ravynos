/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
#ifndef _DYLD_DEBUG_
#define _DYLD_DEBUG_

#include <mach/mach.h>
#ifndef DYLD_BUILD /* do not include this when building dyld itself */
#include <mach-o/dyld.h>
#endif /* !defined(DYLD_BUILD) */
/*
 * The dyld debugging API.
 */
enum dyld_debug_return {
    DYLD_SUCCESS,
    DYLD_INCONSISTENT_DATA,
    DYLD_INVALID_ARGUMENTS,
    DYLD_FAILURE
};

struct dyld_debug_module {
    struct mach_header *header;
    unsigned long vmaddr_slide;
    unsigned long module_index;
};

enum dyld_event_type {
    DYLD_IMAGE_ADDED,
    DYLD_MODULE_BOUND,
    DYLD_MODULE_REMOVED,
    DYLD_MODULE_REPLACED,
    DYLD_PAST_EVENTS_END,
    DYLD_IMAGE_REMOVED
};

struct dyld_event {
    enum dyld_event_type type;
    struct dyld_debug_module arg[2];
};

extern enum dyld_debug_return _dyld_debug_defining_module(
    mach_port_t target_task,
    unsigned long send_timeout,
    unsigned long rcv_timeout,
    boolean_t inconsistent_data_ok,
    char *name,
    struct dyld_debug_module *module);

extern enum dyld_debug_return _dyld_debug_is_module_bound(
    mach_port_t target_task,
    unsigned long send_timeout,
    unsigned long rcv_timeout,
    boolean_t inconsistent_data_ok,
    struct dyld_debug_module module,
    boolean_t *bound);

extern enum dyld_debug_return _dyld_debug_bind_module(
    mach_port_t target_task,
    unsigned long send_timeout,
    unsigned long rcv_timeout,
    boolean_t inconsistent_data_ok,
    struct dyld_debug_module module);

extern enum dyld_debug_return _dyld_debug_module_name(
    mach_port_t target_task,
    unsigned long send_timeout,
    unsigned long rcv_timeout,
    boolean_t inconsistent_data_ok,
    struct dyld_debug_module module,
    char **image_name,
    unsigned long *image_nameCnt,
    char **module_name,
    unsigned long *module_nameCnt);

extern enum dyld_debug_return _dyld_debug_subscribe_to_events(
    mach_port_t target_task,
    unsigned long send_timeout,
    unsigned long rcv_timeout,
    boolean_t inconsistent_data_ok,
    void (*dyld_event_routine)(struct dyld_event event));

/*
 * _dyld_debug_add_event_subscriber() uses the mig interface functions below
 * to dispatch the dyld event messages from the subscriber port specified.
 */
extern enum dyld_debug_return _dyld_debug_add_event_subscriber(
    mach_port_t target_task,
    unsigned long send_timeout,
    unsigned long rcv_timeout,
    boolean_t inconsistent_data_ok,
    mach_port_t subscriber);

/*
 * These structures should be produced by mig(1) from the mig generated files
 * but they are not.  These are really only needed so the correct size of the
 * request and reply messages can be allocated.
 */
struct _dyld_event_message_request {
#ifdef __MACH30__
    mach_msg_header_t head;
    NDR_record_t NDR;
    struct dyld_event event;
    mach_msg_trailer_t trailer;
#else
    msg_header_t head;
    msg_type_t eventType;
    struct dyld_event event;
#endif
};
struct _dyld_event_message_reply {
#ifdef __MACH30__
    mach_msg_header_t head;
    NDR_record_t NDR;
    struct dyld_event event;
#else
    msg_header_t head;
    msg_type_t RetCodeType;
    kern_return_t RetCode;
#endif
};
#ifndef	mig_internal
/*
 * _dyld_event_server() is the mig generated routine to dispatch dyld event
 * messages.
 */
extern boolean_t _dyld_event_server(
#ifdef __MACH30__
    mach_msg_header_t *request,
    mach_msg_header_t *reply);
#else
    struct _dyld_event_message_request *request,
    struct _dyld_event_message_reply *reply);
#endif
#endif /* mig_internal */

#ifndef SHLIB
/*
 * _dyld_event_server_callback() is the routine called by _dyld_event_server()
 * that must be written by users of _dyld_event_server().
 */
extern
#ifdef __MACH30__
kern_return_t
#else
void
#endif
_dyld_event_server_callback(
#ifdef __MACH30__
    mach_port_t subscriber,
#else
    port_t subscriber,
#endif
    struct dyld_event event);
#endif /* SHLIB */

/*
 * This is the state of the target task while we are sending a message to it.
 */
struct _dyld_debug_task_state {
    mach_port_t	   debug_port;
    mach_port_t    debug_thread;
    unsigned int   debug_thread_resume_count;
    unsigned int   task_resume_count;
    mach_port_t   *threads;
    unsigned int   thread_count;
};

/*
 * _dyld_debug_make_runnable() is called before sending messages to the
 * dynamic link editor.  Basically it assures that the debugging
 * thread is the only runnable thread in the task to receive the
 * message.  It also assures that the debugging thread is indeed
 * runnable if it was suspended.  The function will make sure each 
 * thread in the remote task is suspended and resumed the same number
 * of times, so in the end the suspend count of each individual thread
 * is the same.
 */
extern enum dyld_debug_return _dyld_debug_make_runnable(
    mach_port_t target_task,
    struct _dyld_debug_task_state *state);

/*
 * _dyld_debug_restore_runnable() is called after sending messages to the
 * dynamic link editor.  It undoes what _dyld_debug_make_runnable() did to the
 * task and put it back the way it was.
 */
extern enum dyld_debug_return _dyld_debug_restore_runnable(
    mach_port_t target_task,
    struct _dyld_debug_task_state *state);

/*
 * To provide more detailed information when the APIs of the dyld debug
 * interfaces fail (return DYLD_FAILURE) the following structure is filled in.
 * After it is filled in the function registered with
 * set_dyld_debug_error_func() is called with a pointer to that struct.
 *
 * The local_error field is a unique number for each possible error condition
 * in the source code in that makes up the dyld debug APIs.  The source file
 * and line number in the cctools libdyld directory where the dyld debug APIs
 * are implemented are set into the file_name and line_number fields.  The
 * field dyld_debug_return is filled in with that would be returned by the
 * API (usually DYLD_FAILURE).  The other fields will be zero or filled in by
 * the error code from the mach system call, or UNIX system call that failed.
 */
struct dyld_debug_error_data {
    enum dyld_debug_return dyld_debug_return;
    kern_return_t mach_error;
    int dyld_debug_errno;
    unsigned long local_error;
    char *file_name;
    unsigned long line_number;
};

extern void _dyld_debug_set_error_func(
    void (*func)(struct dyld_debug_error_data *e));

#ifndef DYLD_BUILD /* do not include this when building dyld itself */

extern enum dyld_debug_return _dyld_debug_task_from_core(
    NSObjectFileImage coreFileImage,
    mach_port_t *core_task);

#endif /* !defined(DYLD_BUILD) */

#endif /* _DYLD_DEBUG_ */
