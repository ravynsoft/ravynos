//
//  bjMACAddr.h
//  TestTB
//
//  Created by Terrin Eager on 3/23/13.
//
//

#ifndef __TestTB__bjMACAddr__
#define __TestTB__bjMACAddr__

#include <iostream>
#include <sys/socket.h>
#include "bjtypes.h"

class BJMACAddr
{
public:
    BJMACAddr() { memset(addr,0,sizeof(addr));};
    BJMACAddr(const BJMACAddr& Src) { memcpy(addr,Src.addr,sizeof(addr)); };

    void operator=(const BJMACAddr& Src) { memcpy(addr,Src.addr,sizeof(addr)); };

    void Set(unsigned char* newAddr) {memcpy(addr,newAddr,sizeof(addr));};
    void SetString(char* newAddrString)
    {
        int newAddr[6] = {0,0,0,0,0,0};
        sscanf(newAddrString, "%02X:%02X:%02X:%02X:%02X:%02X", &newAddr[0],&newAddr[1],&newAddr[2],&newAddr[3],&newAddr[4],&newAddr[5]);
        for (int i=0; i< 6; i++)
            addr[i] = newAddr[i];
    };
    unsigned char* Get() {return addr;};

    void CopyVendor(BJMACAddr& src) { memset(addr,0,sizeof(addr)); memcpy(addr,src.addr,4);}; // 3 is standar vendor But 4 is better with apple products

    char* GetString() {sprintf(buffer,"%02X:%02X:%02X:%02X:%02X:%02X", addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]); return buffer;};
    char* GetStringVendor() {sprintf(buffer,"%02X:%02X:%02X", addr[0],addr[1],addr[2]); return buffer;};
    BJ_COMPARE Compare(BJMACAddr* compareAddr)
    {
        int result = memcmp(addr, compareAddr->addr, sizeof(addr));
        if (result > 0)
            return BJ_GT;
        if (result < 0)
            return BJ_LT;
        return BJ_EQUAL;
    };
    bool IsEmpty() { return (addr[0] | addr[1] | addr[2] | addr[3] | addr[4] | addr[5]) == 0;};

private:
    unsigned char addr[6];
    char buffer[25];
};

#endif /* defined(__TestTB__bjMACAddr__) */
