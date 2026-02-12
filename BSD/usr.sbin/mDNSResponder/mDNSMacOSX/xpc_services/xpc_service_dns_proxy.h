//
//  xpc_service_dns_server.h
//  mDNSResponder
//
//  Copyright (c) 2019 Apple Inc. All rights reserved.
//

#ifndef XPC_SERVICE_DNS_PROXY_H
#define XPC_SERVICE_DNS_PROXY_H

#include "xpc_client_dns_proxy.h"
#include "dnsproxy.h"

mDNSexport void log_dnsproxy_info(mDNS *const m);
mDNSexport void log_dnsproxy_info_to_fd(int fd, mDNS *const m);
mDNSexport void init_dnsproxy_service(void);

#endif /* XPC_SERVICE_DNS_PROXY_H */
