//
//  Frame.cpp
//  TestTB
//
//  Created by Terrin Eager on 1/19/13.
//
//

#include "Frame.h"

#define EthernetHeaderStart 14

void Frame::Set(BJ_UINT8* data,BJ_UINT32 len,BJ_UINT64 t)
{
    frameData = data;
    length = len;
    frameTime = t;
}

BJ_UINT8* Frame::GetEthernetStart()
{
    //todo Support other media types
    return frameData;
}
BJ_UINT8* Frame::GetIPStart()
{
    BJ_UINT8* ether = GetEthernetStart();

    return ether + 14;

}
BJ_UINT8* Frame::GetUDPStart()
{
    BJ_UINT8* ip = GetIPStart();

    BJ_UINT16 nSize = *((__uint16_t*) (ip));
    BJ_UINT16 nVerison = (nSize&0xf0) >> 4;
    if (nVerison == 0x4)
    {
        m_bCurrentFrameIPversion = 4;

        nSize &= 0x0f;
        nSize *= 4;


        BJ_UINT8 nProtocol = *(ip+9);

        if (nProtocol != 17) // Not UDP
            return NULL;
    }
    else if (nVerison == 0x6)
    {
        m_bCurrentFrameIPversion = 6;
        BJ_UINT8 nProtocol = *(ip+6);

        if (nProtocol != 17) // Not UDP
            return NULL;
        nSize = 40;

    }

    return ip+nSize;
}

BJ_UINT8* Frame::GetBonjourStart()
{
    BJ_UINT8* udp = GetUDPStart();


    if (udp == NULL)
        return NULL;

    BJ_UINT16 nSourcePort = *((__uint16_t*)(udp));
    BJ_UINT16 nDestPort = *((__uint16_t*)(udp+2));
    BJ_UINT16 nBonjourPort  = htons(5353);

    if (nSourcePort == nBonjourPort && nDestPort == nBonjourPort)
        return (udp+8);
    else
        return NULL;

}



BJIPAddr* Frame::GetSrcIPAddr()
{
    BJ_UINT8* ip = GetIPStart();

    BJ_UINT16 nSize = (__uint16_t) (*ip);
    BJ_UINT16 nVerison = (nSize&0xf0) >> 4;
    if (nVerison == 0x4)
    {
        m_bCurrentFrameIPversion = 4;

        struct in_addr* ipi_addr;

        ipi_addr = (in_addr*)(ip+12);

        sourceIPAddr.Set(ipi_addr);

    }
    else if (nVerison == 0x6)
    {
        m_bCurrentFrameIPversion = 6;
        BJ_UINT8* ipi_addr;

        ipi_addr = (ip+8);

        sourceIPAddr.Setv6Raw(ipi_addr);

    }

    return &sourceIPAddr;
}

BJIPAddr* Frame::GetDestIPAddr()
{
    BJ_UINT8* ip = GetIPStart();

    BJ_UINT16 nSize = *((__uint16_t*) (ip));
    BJ_UINT16 nVerison = (nSize&0xf0) >> 4;
    if (nVerison == 0x4)
    {
        m_bCurrentFrameIPversion = 4;

        struct in_addr* ipi_addr;

        ipi_addr = (in_addr*)(ip+16);

        destIPAddr.Set(ipi_addr);

    }
    else if (nVerison == 0x6)
    {
        m_bCurrentFrameIPversion = 6;
        struct in6_addr* ipi_addr;

        ipi_addr = (in6_addr*)(ip+24);

        destIPAddr.Set(ipi_addr);

    }
    return &destIPAddr;
}

BJMACAddr* Frame::GetSrcMACAddr()
{
    sourceMACAddr.Set(GetEthernetStart()+6);

    return &sourceMACAddr;
}

BJMACAddr* Frame::GetDestMACAddr()
{
    destMACAddr.Set(GetEthernetStart());

    return &destMACAddr;
}

void Frame::SetDatalinkType(BJ_DATALINKTYPE datalinkType)
{
    m_datalinkType = datalinkType;
}

BJ_UINT32 Frame::GetLinklayerHeaderLength()
{
    switch (m_datalinkType)
    {
        case (BJ_DLT_EN10MB):
            return EthernetHeaderStart;
        case (BJ_DLT_IEEE802_11):
            return Get80211HeaderLength();
        default:
            // Default to Ethernet
            return EthernetHeaderStart;
    }
}

BJ_UINT32 Frame::Get80211HeaderLength()
{
    // XXX: 802.11 header is tricky since it has no "length" field.
    // We should look at "FrameControl" and derive the length manually for each frame.
    BJ_UINT16 * frameControl = (BJ_UINT16*)GetEthernetStart();

    // [SubType] [Type - Ver]

    bool isFrameData = (0x0C & *frameControl) == 0x08;
    bool isQosData   = ((0xF0 & *frameControl) == 0x80) && isFrameData;

    if (isQosData)
    {
        //Standard (24) + QoS (2) + LLC (3) + SNAP (5)
        return 24 + 2 + 3 + 5;
    }
    else
    {
        //Standard (24) + LLC (3) + SNAP (5)
        return 24 + 3 + 5;
    }
}
