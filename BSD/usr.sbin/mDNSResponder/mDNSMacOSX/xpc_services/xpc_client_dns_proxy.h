//
//  xpc_client_dns_proxy.h
//  mDNSResponder
//
//  Copyright (c) 2019 Apple Inc. All rights reserved.
//

#ifndef XPC_CLIENT_DNS_PROXY_H
#define XPC_CLIENT_DNS_PROXY_H

#define kDNSProxyService        "com.apple.mDNSResponder.dnsproxy"
#define kDNSProxyParameters     "DNSProxyParameters"

#define kDNSInIfindex0          "InputArrayInterfaceIndex[0]"
#define kDNSInIfindex1          "InputArrayInterfaceIndex[1]"
#define kDNSInIfindex2          "InputArrayInterfaceIndex[2]"
#define kDNSInIfindex3          "InputArrayInterfaceIndex[3]"
#define kDNSInIfindex4          "InputArrayInterfaceIndex[4]"

#define kDNSOutIfindex          "OutputInterfaceIndex"

#endif /* XPC_CLIENT_DNS_PROXY_H */
