//
//  xpc_services.h
//  mDNSResponder
//
//  Copyright (c) 2019 Apple Inc. All rights reserved.
//


#ifndef XPC_SERVICES_H
#define XPC_SERVICES_H

#include "mDNSEmbeddedAPI.h"
#include <xpc/xpc.h>

mDNSexport void xpc_server_init(void);
mDNSexport mDNSBool IsEntitled(xpc_connection_t conn, const char *password);

#endif // XPC_SERVICES_H
