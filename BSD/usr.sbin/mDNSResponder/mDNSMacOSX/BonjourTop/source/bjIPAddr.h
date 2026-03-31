//
//  bjIPAddr.h
//  TestTB
//
//  Created by Terrin Eager on 1/19/13.
//
//

#ifndef __TestTB__bjIPAddr__
#define __TestTB__bjIPAddr__

#include <iostream>
#include <sys/socket.h>
#include "bjtypes.h"

class BJIPAddr
{
public:
    BJIPAddr();
    BJIPAddr(const BJIPAddr& src);

    void Empty();

    bool IsBonjourMulticast();
    bool IsSameSubNet(BJIPAddr* addr);

    bool IsIPv4();
    bool IsIPv6();
    bool IsIPv6LinkLocal();
    bool IsEmpty();
    bool IsEmptySubnet();

    void Set(const char* addr);
    void Setv6(const char* addr);
    void Set(struct in6_addr* ipi6_addr);
    void Set(struct in_addr* ip_addr);
    void Set(struct sockaddr_storage* sockStorage);
    void Setv4Raw(BJ_UINT8* ipi4_addr);
    void Setv6Raw(BJ_UINT8* ipi6_addr);

    sockaddr_storage* GetRawValue();
    struct in6_addr* Getin6_addr();

    void CreateLinkLocalIPv6(BJ_UINT8* mac);
    BJ_COMPARE Compare(BJIPAddr* addr);
    BJ_UINT16 GetPortNumber();
    char* GetString();
private:
    sockaddr_storage sockAddrStorage;
    BJ_INT32 IPv4SubNet;
    char stringbuffer[100];
    static sockaddr_storage emptySockAddrStorage;
};


#endif /* defined(__TestTB__bjIPAddr__) */
