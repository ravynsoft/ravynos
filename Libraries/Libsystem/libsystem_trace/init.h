/**
 * init.h
 * Libsystem trace internal declarations and lazy symbol bindings
 */

#ifndef __LIBSYSTEM_TRACE_INIT_H
#define __LIBSYSTEM_TRACE_INIT_H

#include <sys/cdefs.h>
#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFData.h>

__BEGIN_DECLS

void _libtrace_init(void);

__END_DECLS

#endif /* __LIBSYSTEM_TRACE_INIT_H */
