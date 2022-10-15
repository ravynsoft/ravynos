/* 
    CFMachPort_Lifetime.h
    Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
    All of the functions in this file exist to orchestrate the exact time/circumstances we decrement the port references.
 */

#include "CFMachPort_Lifetime.h"
#include <mach/mach.h>
#include "CFInternal.h"

// Records information relevant for cleaning up after a given mach port. Here's
// a summary of its life cycle:
//   A) _cfmp_deallocation_record created and stored in _cfmp_records
//      This means a dispatch source has been created to track dead name for
//      the port.
//   B) There is no record for a given port in _cfmp_records
//      This means either: the dispatch source above has been cancelled, or that
//      there was a never a dispatch source in the first place.
//
// For pure CFMachPorts with no Foundation NSPort references, the flags doSend
// and doReceive record the kind of rights a given port should clear up when
// the cancel handler is called.
// 
// The reason for this is that the Deallocate of a CFMachPort can happen before
// the cancel handler has been called, and historically the deallocate was the
// where the rights would be decremented.
//
// When NSPort's are involved, we track a few other bits for exactly the same
// reasons. The reason these are tracked separately is out of an abundance of 
// caution - by all reads, the two mechanisms do not overlap, but tracking them
// separate is more debugable.
//
typedef struct {
    mach_port_t port; // which port (contributes to identity)
    uint8_t client; // which subsystem is tracking (contributes to identity)
                    // should be kept in sync with MAX_CLIENTS constants below
    
    uint8_t inSet:1; // helps detect invariant violations
    
    uint8_t deallocated:1;
    uint8_t invalidated:1;
    
    uint8_t doSend:1;
    uint8_t doReceive:1;
    
    // indicates that there is additional invalidation requested by NSMachPort
    uint8_t nsportIsInterested:1;
    uint8_t nsportDoSend:1;
    uint8_t nsportDoReceive:1;
} _cfmp_deallocation_record;

#define MAX_CLIENTS 256
#define MAX_CLIENTS_BITS 8

#pragma mark - Port Right Modification
CF_INLINE void _cfmp_mod_refs(mach_port_t const port, const Boolean doSend, const Boolean doReceive) {
    // NOTE: do receive right first per: https://howto.apple.com/wiki/pages/r853A7H2j/Mach_Ports_and_You.html
    if (doReceive) {
        mach_port_mod_refs(mach_task_self(), port, MACH_PORT_RIGHT_RECEIVE, -1);
    }
    if (doSend) {
        mach_port_deallocate(mach_task_self(), port);
    }    
}

#pragma mark - Logging
CF_BREAKPOINT_FUNCTION(void _CFMachPortDeallocationFailure(void));
static void _cfmp_log_failure(char const *const msg, _cfmp_deallocation_record *const pr, _CFMPLifetimeClient const client, mach_port_t const port) {
    if (pr) {
        _cfmp_deallocation_record const R = *pr;
        os_log_error(OS_LOG_DEFAULT, "*** %{public}s break on '_CFMachPortDeallocationFailure' to debug: {p:%{private}d c:%d is:%d <i:%d,d:%d> s:%d,r:%d nsi:%d,nss:%d,nsr:%d - ic:%d,ip:%d}", msg, R.port, R.client, R.inSet, R.invalidated, R.deallocated, R.doSend, R.doReceive, R.nsportIsInterested, R.nsportDoSend, R.nsportDoReceive, client, port);
    } else {
        os_log_error(OS_LOG_DEFAULT, "*** %{public}s break on '_CFMachPortDeallocationFailure' to debug: {null - ic:%d,ip:%d}", msg, client, port);
    }
    _CFMachPortDeallocationFailure();
}



#pragma mark - _cfmp_deallocation_record CFSet Callbacks
// Various CFSet callbacks for _cfmp_deallocation_record
static CFTypeRef _cfmp_deallocation_record_retain(CFAllocatorRef allocator, CFTypeRef const cf) {
    _cfmp_deallocation_record *const pr = (_cfmp_deallocation_record *)cf;
    if (pr->inSet) {
        HALT_MSG("refcnt overflow");
    }
    pr->inSet = 1;
    return pr;
}
static Boolean _cfmp_equal(void const *const value1, void const *const value2) {
    Boolean equal = false;
    if (value1 == value2) {
        equal = true;
    } else if (value1 && value2){
        _cfmp_deallocation_record const R1 = *(_cfmp_deallocation_record *)value1;
        _cfmp_deallocation_record const R2 = *(_cfmp_deallocation_record *)value2;
        equal = R1.port == R2.port && R1.client == R2.client;
    }
    return equal;
}
static CFHashCode _cfmp_hash(void const *const value) {
    CFHashCode hash = 0;
    if (value) {
        _cfmp_deallocation_record const R = *(_cfmp_deallocation_record *)value;
        hash = _CFHashInt(R.port << MAX_CLIENTS_BITS | R.client); 
    }
    return hash;
}
static void _cfmp_deallocation_record_release(CFAllocatorRef const allocator, void const *const value) {
    _cfmp_deallocation_record *pr = (_cfmp_deallocation_record *)value;
    if (!pr->inSet) {
        _cfmp_log_failure("Freeing a record not in the set", pr, pr->client, pr->port);
    }
    free(pr);
}
static CFStringRef _cfmp_copy_description(const void *value) {
    CFStringRef s = CFSTR("{null}");
    if (value) {
        _cfmp_deallocation_record const R = *(_cfmp_deallocation_record *)value;
        s = CFStringCreateWithFormat(NULL, NULL, CFSTR("{p:%d c:%d is:%d <i:%d,d:%d> s:%d,r:%d nsi:%d,nss:%d,nsr:%d}"), R.port, R.client, R.inSet, R.invalidated, R.deallocated, R.doSend, R.doReceive, R.nsportIsInterested, R.nsportDoSend, R.nsportDoReceive);
    }
    return s;
}

#pragma mark - Deallocation Records

// Pending deallocations/invalidations are recorded in the global set returned by _cfmp_records, whose access should be protected by  _cfmp_records_lock
static os_unfair_lock _cfmp_records_lock = OS_UNFAIR_LOCK_INIT;
static CFMutableSetRef _cfmp_records(void)  {
    static CFMutableSetRef oRecords;
    static dispatch_once_t oGuard;
    dispatch_once(&oGuard, ^{
        CFSetCallBacks const cb = {
            .version = 0,
            .retain = _cfmp_deallocation_record_retain,
            .release = _cfmp_deallocation_record_release,
            .copyDescription = _cfmp_copy_description,
            .equal = _cfmp_equal,
            .hash = _cfmp_hash
        };
        oRecords = CFSetCreateMutable(NULL, 16, &cb);
    });
    return oRecords;
};

CF_INLINE _cfmp_deallocation_record *const _cfmp_find_record_for_port(CFSetRef const records, _CFMPLifetimeClient const client, mach_port_t const port) {
    _cfmp_deallocation_record const lookup = {.port = port, .client = client};
    _cfmp_deallocation_record *const pr = (_cfmp_deallocation_record *)CFSetGetValue(records, &lookup);
    return pr;
}

#pragma mark - Lifetime Management
CF_PRIVATE void _cfmp_cleanup(_cfmp_deallocation_record const R) {
    _cfmp_mod_refs(R.port, R.doSend, R.doReceive);
    if (R.nsportIsInterested) {
        _cfmp_mod_refs(R.port, R.nsportDoSend, R.nsportDoReceive);
    }
}

/// Records that a given mach_port has been deallocated.
void _cfmp_record_deallocation(_CFMPLifetimeClient const client, mach_port_t const port, Boolean const doSend, Boolean const doReceive) {
    if (port == MACH_PORT_NULL) { return; }

    // now that we know we're not a no-op, look for an existing deallocation record
    CFMutableSetRef records = _cfmp_records();

    Boolean cleanupNow = false;
    _cfmp_deallocation_record R = {0};
    
    os_unfair_lock_lock(&_cfmp_records_lock);
    _cfmp_deallocation_record *const pr = _cfmp_find_record_for_port(records, client, port);
    if (pr) {
        if (pr->invalidated) {
            // it's already been invalidated, so can tidy up now
            R = *(_cfmp_deallocation_record *)pr;
            CFSetRemoveValue(records, pr);
            cleanupNow = true;
        } else {
            // we're expecting invalidation, record that we want clean up doSend/Receive for later
            pr->deallocated = true;
            pr->doSend = doSend;
            pr->doReceive = doReceive;
        }
    } else  {
        R.port = port;
        R.doSend = doSend;
        R.doReceive = doReceive;
        cleanupNow = true;
    }
    os_unfair_lock_unlock(&_cfmp_records_lock);
    
    if (cleanupNow) {
        _cfmp_cleanup(R);
    }
}

void _cfmp_record_intent_to_invalidate(_CFMPLifetimeClient const client, mach_port_t const port) {
    if (port == MACH_PORT_NULL) { return; }

    _cfmp_deallocation_record *pr = calloc(1, sizeof(_cfmp_deallocation_record));
    if (pr == NULL) {
        HALT_MSG("Unable to allocate mach_port deallocation record");
    }
    pr->port = port;
    pr->client = client;
    
    CFMutableSetRef const records = _cfmp_records();
    os_unfair_lock_lock(&_cfmp_records_lock);
    if (CFSetGetValue(records, pr) != NULL) {
        // since we calloc before we insert; we check to make sure records doesn't already have an entry for this port
        os_unfair_lock_unlock(&_cfmp_records_lock);
        free(pr);
    } else {
        CFSetAddValue(records, pr);
        os_unfair_lock_unlock(&_cfmp_records_lock);
    }
}

void _cfmp_source_invalidated(_CFMPLifetimeClient const client, mach_port_t port) {
    Boolean cleanupNow = false;
    _cfmp_deallocation_record R = {0};
    
    CFMutableSetRef const records = _cfmp_records();
    os_unfair_lock_lock(&_cfmp_records_lock);
    _cfmp_deallocation_record *pr = _cfmp_find_record_for_port(records, client, port);
    if (pr == NULL) {
        _cfmp_log_failure("not expecting invalidation", pr, client, port);
    } else {
        if (pr->deallocated) {
            cleanupNow = true;
            R = *(_cfmp_deallocation_record *)pr;
            CFSetRemoveValue(records, pr);
        } else {
            pr->invalidated = true;
        }
    }
    os_unfair_lock_unlock(&_cfmp_records_lock);
    
    if (cleanupNow) {
        _cfmp_cleanup(R);
    }
}

// records that we have received a deadname notification for the specified port
void _cfmp_source_record_deadness(_CFMPLifetimeClient const client, mach_port_t const port) {
    CFMutableSetRef const records = _cfmp_records();
    os_unfair_lock_lock(&_cfmp_records_lock);
    _cfmp_deallocation_record *const pr = _cfmp_find_record_for_port(records, client, port);
    if (pr == NULL) {
        _cfmp_log_failure("received deadname notification for untracked port", pr, client, port);
    } else {
        pr->doReceive = 0;
    }
    os_unfair_lock_unlock(&_cfmp_records_lock);
}

void _cfmp_record_nsmachport_is_interested(_CFMPLifetimeClient const client, mach_port_t const port) {
    if (port == MACH_PORT_NULL) { return; }
    
    // now that we know we're not a no-op, look for an existing deallocation record
    CFMutableSetRef records = _cfmp_records();
    
    os_unfair_lock_lock(&_cfmp_records_lock);
    _cfmp_deallocation_record *const pr = _cfmp_find_record_for_port(records, client, port);
    if (pr) {
        // we're expecting invalidation. record that nsport is interested.
        pr->nsportIsInterested = true;
    }
    os_unfair_lock_unlock(&_cfmp_records_lock);
}

void _cfmp_record_nsmachport_deallocation(_CFMPLifetimeClient const client, mach_port_t const port, Boolean const doSend, Boolean const doReceive) {
    if (port == MACH_PORT_NULL) { return; }
    if (doSend == false && doReceive == false) { return; }
    
    CFMutableSetRef records = _cfmp_records();
    
    Boolean cleanupNow = false;
    _cfmp_deallocation_record R = {0};
    
    os_unfair_lock_lock(&_cfmp_records_lock);
    _cfmp_deallocation_record *const pr = _cfmp_find_record_for_port(records, client, port);
    if (pr == NULL) {
        R.port = port;
        R.nsportDoSend = doSend;
        R.nsportDoReceive = doReceive;
        cleanupNow = true;
    } else {
        // we're expecting invalidation. record that we want to doSend/Receive for nsport later
        // but first make sure we were expecting an NSMachPort at all
        if (!pr->nsportIsInterested) {
            _cfmp_log_failure("setting nsport state - when its not interested", pr, client, port);
        } else if (pr->invalidated) {
            // it's already been invalidated, so can tidy up now
            R = *(_cfmp_deallocation_record *)pr;
            CFSetRemoveValue(records, pr);
            cleanupNow = true;
        } else {
            // we're expecting invalidation, record that we want clean up doSend/Receive for later
            pr->deallocated = true;
            pr->nsportDoSend = doSend;
            pr->nsportDoReceive = doReceive;
        }
    }
    os_unfair_lock_unlock(&_cfmp_records_lock);
    
    if (cleanupNow) {
        _cfmp_cleanup(R);
    }
}
