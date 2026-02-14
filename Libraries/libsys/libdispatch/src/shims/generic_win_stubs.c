#include "internal.h"

/*
 * This file contains stubbed out functions we are using during
 * the initial Windows port.  When the port is complete, this file
 * should be empty (and thus removed).
 */

void
_dispatch_runloop_queue_dispose(dispatch_queue_t dq DISPATCH_UNUSED,
		bool *allow_free DISPATCH_UNUSED)
{
	WIN_PORT_ERROR();
}

void
_dispatch_runloop_queue_xref_dispose(dispatch_queue_t dq DISPATCH_UNUSED)
{
	WIN_PORT_ERROR();
}

/*
 * Stubbed out static data
 */
