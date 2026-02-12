//
//  bjIPAddr.cpp
//  TestTB
//
//  Created by Terrin Eager on 1/19/13.
//
//

#include <netinet/in.h>
#include <arpa/inet.h>

#include "bjIPAddr.h"
#include "bjstring.h"


//   static
sockaddr_storage BJIPAddr::emptySockAddrStorage;



BJIPAddr::BJIPAddr()
{
    memset(&emptySockAddrStorage,0,sizeof(emptySockAddrStorage));
    Empty();
}

BJIPAddr::BJIPAddr(const BJIPAddr& src)
{
    memcpy(&sockAddrStorage,&src.sockAddrStorage,sizeof(sockAddrStorage));
    IPv4SubNet = src.IPv4SubNet;
}
void BJIPAddr::Empty()
{
    memset(&sockAddrStorage,0,sizeof(sockAddrStorage));
    IPv4SubNet = 0;

}

bool BJIPAddr::IsBonjourMulticast()
{
    bool bResult = false;

    struct in_addr  BonjourMulicastAddrIPv4= {0xFB0000E0};

    struct in6_addr BonjourMulicastAddrIPv6 = {{{ 0xFF,0x02,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0xFB }}};


    if (sockAddrStorage.ss_family == AF_INET)
    {
        struct sockaddr_in* pAddrIn = (sockaddr_in*) &sockAddrStorage;
        return (pAddrIn->sin_addr.s_addr == BonjourMulicastAddrIPv4.s_addr);
    }

    if (sockAddrStorage.ss_family == AF_INET6)
    {
        struct sockaddr_in6* pAddrIn = (sockaddr_in6*) &sockAddrStorage;
        return (memcmp(&pAddrIn->sin6_addr,&BonjourMulicastAddrIPv6,sizeof(in6_addr)) == 0);
    }


    return bResult;

}

bool BJIPAddr::IsSameSubNet(BJIPAddr* pCheckAddr)
{

    if (IPv4SubNet == 0)
        return true;

    if (!pCheckAddr->IsIPv4())
        return true;

    in_addr_t Mask = 0xFFFFFFFF;

    Mask = Mask << (32-IPv4SubNet);

    endian_swap(Mask);

    struct sockaddr_in* pMyAddrIn = (sockaddr_in*) &sockAddrStorage;
    in_addr_t myNetworkAddress = pMyAddrIn->sin_addr.s_addr & Mask;

    struct sockaddr_in* pCheckAddrIn = (sockaddr_in*) pCheckAddr->GetRawValue();
    in_addr_t CheckNetworkAddress = pCheckAddrIn->sin_addr.s_addr & Mask;


    return (myNetworkAddress == CheckNetworkAddress);
}


bool BJIPAddr::IsIPv4()
{
    return (sockAddrStorage.ss_family == AF_INET);
}

bool BJIPAddr::IsIPv6()
{
    return (sockAddrStorage.ss_family == AF_INET6);
}

bool BJIPAddr::IsIPv6LinkLocal()
{
     struct sockaddr_in6* pAddrIn = (sockaddr_in6*) &sockAddrStorage;
    return (pAddrIn->sin6_addr.__u6_addr.__u6_addr8[0] == 0xfe &&
            pAddrIn->sin6_addr.__u6_addr.__u6_addr8[1] == 0x80);
}
bool BJIPAddr::IsEmpty()
{
    return (memcmp(&sockAddrStorage,&emptySockAddrStorage,sizeof(sockAddrStorage)) == 0);
}

bool BJIPAddr::IsEmptySubnet()
{
    return (IPv4SubNet == 0);
}

void BJIPAddr::Setv6(const char* pIPaddr)
{
    memset(&sockAddrStorage,0,sizeof(sockAddrStorage));
    struct sockaddr_in6* pAddrIn = (sockaddr_in6*) &sockAddrStorage;

    if (inet_pton(AF_INET6, pIPaddr, &pAddrIn->sin6_addr) && memcmp(&sockAddrStorage,&emptySockAddrStorage,sizeof(sockAddrStorage)) == 0)
            pAddrIn->sin6_family = AF_INET6;
}

void BJIPAddr::Set(const char* pIPaddr)
{
    memset(&sockAddrStorage,0,sizeof(sockAddrStorage));

    if (pIPaddr == NULL || strlen(pIPaddr) == 0)
        return;

    BJString sIPAddr;
    BJString sMask;

    const char* pSeperator = strstr(pIPaddr,"/");
    if (pSeperator)
    {
        sIPAddr.Set(pIPaddr, (BJ_UINT32)(pSeperator - pIPaddr));
        sMask.Set(pSeperator+1);
    }
    else
    {
        sIPAddr.Set(pIPaddr);
    }

    struct sockaddr_in* pAddrIn = (sockaddr_in*) &sockAddrStorage;
    pAddrIn->sin_family = AF_INET;
    pAddrIn->sin_addr.s_addr = inet_addr(sIPAddr.GetBuffer());

    IPv4SubNet = sMask.GetUINT32();

}
void BJIPAddr::Setv4Raw(BJ_UINT8* ipi4_addr)
{

    memset(&sockAddrStorage,0,sizeof(sockAddrStorage));
    struct sockaddr_in* pAddrIn = (sockaddr_in*) &sockAddrStorage;
    pAddrIn->sin_family = AF_INET;
    memcpy(&pAddrIn->sin_addr, ipi4_addr, sizeof(pAddrIn->sin_addr));

}
void BJIPAddr::Setv6Raw(BJ_UINT8* ipi6_addr)
{

    memset(&sockAddrStorage,0,sizeof(sockAddrStorage));
    struct sockaddr_in6* pAddrIn = (sockaddr_in6*) &sockAddrStorage;

    pAddrIn->sin6_family = AF_INET6;
    memcpy(&pAddrIn->sin6_addr, ipi6_addr, sizeof(pAddrIn->sin6_addr));
}

void BJIPAddr::Set(struct in6_addr* ipi6_addr)
{

    memset(&sockAddrStorage,0,sizeof(sockAddrStorage));
    struct sockaddr_in6* pAddrIn = (sockaddr_in6*) &sockAddrStorage;

    pAddrIn->sin6_family = AF_INET6;
    memcpy(&pAddrIn->sin6_addr, ipi6_addr, sizeof(pAddrIn->sin6_addr));
}

void BJIPAddr::Set(struct in_addr* ip_addr)
{

    memset(&sockAddrStorage,0,sizeof(sockAddrStorage));
    struct sockaddr_in* pAddrIn = (sockaddr_in*) &sockAddrStorage;
    pAddrIn->sin_family = AF_INET;
    pAddrIn->sin_addr = *ip_addr;
}

void BJIPAddr::Set(struct sockaddr_storage* pStorage)
{
    memcpy(&sockAddrStorage,pStorage,sizeof(sockAddrStorage));
}

sockaddr_storage* BJIPAddr::GetRawValue()
{
    return &sockAddrStorage;
}

struct in6_addr* BJIPAddr::Getin6_addr()
{
    struct sockaddr_in6* pAddrIn = (sockaddr_in6*) &sockAddrStorage;
    return &pAddrIn->sin6_addr;
}

BJ_UINT16 BJIPAddr::GetPortNumber()
{
    BJ_UINT16 port = 0;
    if (sockAddrStorage.ss_family == AF_INET)
    {
        struct sockaddr_in* pAddrIn = (struct sockaddr_in*)&sockAddrStorage;
        port = ntohs(pAddrIn->sin_port);
    }
    else if (sockAddrStorage.ss_family == AF_INET6)
    {
        struct sockaddr_in6* pAddrIn = (struct sockaddr_in6*)&sockAddrStorage;
        port = ntohs(pAddrIn->sin6_port);
    }
    return port;
}

BJ_COMPARE BJIPAddr::Compare(BJIPAddr* pIPAddr)
{
    if (sockAddrStorage.ss_family > pIPAddr->sockAddrStorage.ss_family)
        return BJ_GT;
    if (sockAddrStorage.ss_family < pIPAddr->sockAddrStorage.ss_family)
        return BJ_LT;

    if (sockAddrStorage.ss_family == AF_INET)
    {
        struct sockaddr_in* pMyAddrIn = (sockaddr_in*) &sockAddrStorage;
        struct sockaddr_in* pAddrIn = (sockaddr_in*) &pIPAddr->sockAddrStorage;
        if (pMyAddrIn->sin_addr.s_addr > pAddrIn->sin_addr.s_addr)
            return BJ_GT;
        if (pMyAddrIn->sin_addr.s_addr < pAddrIn->sin_addr.s_addr)
            return BJ_LT;
        return BJ_EQUAL;

    }
    else
    {
        struct sockaddr_in6* pMyAddrIn = (sockaddr_in6*) &sockAddrStorage;
        struct sockaddr_in6* pAddrIn = (sockaddr_in6*) &pIPAddr->sockAddrStorage;

        int result = memcmp(&pMyAddrIn->sin6_addr, &pAddrIn->sin6_addr, sizeof(sockaddr_in6));

        if (result > 0)
            return BJ_GT;
        if (result < 0)
            return BJ_LT;
        return BJ_EQUAL;
    }


}

/*

 take the mac address: for example 52:74:f2:b1:a8:7f
 throw ff:fe in the middle: 52:74:f2:ff:fe:b1:a8:7f
 reformat to IPv6 notation 5274:f2ff:feb1:a87f
 convert the first octet from hexadecimal to binary: 52 -> 01010010
 invert the bit at position 6 (counting from 0): 01010010 -> 01010000
 convert octet back to hexadecimal: 01010000 -> 50
 replace first octet with newly calculated one: 5074:f2ff:feb1:a87f
 prepend the link-local prefix: fe80::5074:f2ff:feb1:a87f
 */

void BJIPAddr::CreateLinkLocalIPv6(BJ_UINT8* pmac)
{
    memset(&sockAddrStorage,0,sizeof(sockAddrStorage));
    struct sockaddr_in6* pAddrIn = (sockaddr_in6*) &sockAddrStorage;

    pAddrIn->sin6_family = AF_INET6;

    pAddrIn->sin6_addr.__u6_addr.__u6_addr8[0] = 0xfe;
    pAddrIn->sin6_addr.__u6_addr.__u6_addr8[1] = 0x80;

    pAddrIn->sin6_addr.__u6_addr.__u6_addr8[8] = *pmac;
    pAddrIn->sin6_addr.__u6_addr.__u6_addr8[8] ^= 1 << 1; // invert 6 bit
    pAddrIn->sin6_addr.__u6_addr.__u6_addr8[9] = *(pmac+1);
    pAddrIn->sin6_addr.__u6_addr.__u6_addr8[10] = *(pmac+2);

    pAddrIn->sin6_addr.__u6_addr.__u6_addr8[11] = 0xff;
    pAddrIn->sin6_addr.__u6_addr.__u6_addr8[12] = 0xfe;


    pAddrIn->sin6_addr.__u6_addr.__u6_addr8[13] = *(pmac+3);
    pAddrIn->sin6_addr.__u6_addr.__u6_addr8[14] = *(pmac+4);
    pAddrIn->sin6_addr.__u6_addr.__u6_addr8[15] = *(pmac+5);


}

char* BJIPAddr::GetString()
{
    memset(stringbuffer,0,sizeof(stringbuffer));
    if (IsIPv6())
    {
        struct sockaddr_in6* pAddrIn = (sockaddr_in6*) &sockAddrStorage;
        inet_ntop(AF_INET6, &pAddrIn->sin6_addr, stringbuffer, sizeof(stringbuffer));
    }
    else
    {
        struct sockaddr_in* pAddrIn = (sockaddr_in*) &sockAddrStorage;
        inet_ntop(AF_INET, &pAddrIn->sin_addr, stringbuffer, sizeof(stringbuffer));
    }
    return stringbuffer;
}

