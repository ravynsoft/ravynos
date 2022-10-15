/*
    CFMachPort_Lifetime.h
    
    Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
    
    Utilities for orchestrating the exact time/circumstances we decrement 
    mach_port rights for CF types that make use of: deallocate, invalidate
    and dispatch_sources.
*/

#if !defined(__COREFOUNDATION_CFMACHPORT_LIFETIME__)
#define __COREFOUNDATION_CFMACHPORT_LIFETIME__ 1

CF_EXTERN_C_BEGIN

#include <mach/mach_port.h>
#include <os/lock.h>

typedef CF_ENUM(uint8_t, _CFMPLifetimeClient) {
    _CFMPLifetimeClientCFMachPort = 0,
    _CFMPLifetimeClientCFMessagePort = 1,
    // NOTE: We current support MAX_CLIENTS more clients (currently 2)
};

/// Records that that a given mach_port has been deallocated.
CF_PRIVATE void _cfmp_record_deallocation(_CFMPLifetimeClient const client, mach_port_t const port, Boolean const doSend, Boolean const doReceive);

/// Records that we have called dispatch_source_cancel.
CF_PRIVATE void _cfmp_record_intent_to_invalidate(_CFMPLifetimeClient const client, mach_port_t const port);

/// Records when the cancel-handler for a given dispatch_port is called.
CF_PRIVATE void _cfmp_source_invalidated(_CFMPLifetimeClient const client, mach_port_t const port);

/// Records that we have received a deadname notification for the specified port.
CF_PRIVATE void _cfmp_source_record_deadness(_CFMPLifetimeClient const client, mach_port_t const port);

/// For Foundation Only
/// Records when an nsmach port is going to be interested in a bridges CFMachPort
CF_EXPORT void _cfmp_record_nsmachport_is_interested(_CFMPLifetimeClient const client, mach_port_t const port);

/// For Foundation Only
/// Records when an nsmachport deallocates its context
CF_EXPORT void _cfmp_record_nsmachport_deallocation(_CFMPLifetimeClient const client, mach_port_t const port, Boolean const doSend, Boolean const doReceive);

CF_EXTERN_C_END
#endif // __COREFOUNDATION_CFMACHPORT_LIFETIME__
