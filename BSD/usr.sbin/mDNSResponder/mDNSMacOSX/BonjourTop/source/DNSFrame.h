//
//  DNSFrame.h
//  TestTB
//
//  Created by Terrin Eager on 9/26/12.
//
//

#ifndef __TestTB__DNSFrame__
#define __TestTB__DNSFrame__

#include <iostream>
#include "bjtypes.h"
#include "bjstring.h"
#include "bjIPAddr.h"
#include "bjMACAddr.h"

#define MAX_DNS_RECORDS_PER_FRAME 500

#define DNS_TYPE_PTR    12
#define DNS_TYPE_SRV    33
#define DNS_TYPE_TXT    16
#define DNS_TYPE_A      1
#define DNS_TYPE_AAAA   28
#define DNS_TYPE_OPT    41

#define DNS_EDNS0_TRACE 65001

class CDNSFrame;




class CDNSRecord
{
public:
    typedef enum {Question,Answer,ns,ar} dnsItemType;

    void GetDnsRecordName(BJString& ReturnString,int nLabelToSkip,int nMaxLabel);
    void GetDnsRecordNameFromBuffer(BJ_UINT8* pBuffer,BJString& ReturnString,int nLabelToSkip,int nMaxLabel);

    BJ_UINT8* GetStartofRdata() {return m_pStartofRec+m_nNameLength+10;}; // 10 = type(2) +class(2) + ttl(4) + datalen(2)
    void GetRdata(BJString& ReturnString,int nLabelToSkip,int nMaxLabel)
    {
        if (m_RecType == DNS_TYPE_SRV)
            GetDnsRecordNameFromBuffer(GetStartofRdata()+6, ReturnString, nLabelToSkip, nMaxLabel); // 6 = Priority + Weight + Port
        else
            GetDnsRecordNameFromBuffer(GetStartofRdata(), ReturnString, nLabelToSkip, nMaxLabel);
    }
    dnsItemType m_dnsType;
    BJ_UINT8*   m_pStartofRec;
    BJ_UINT32   m_nNameLength;
    BJ_INT16    m_RecType;
    BJ_INT16    m_RecClass;
    BJ_UINT32   m_nTTL;
    BJ_UINT32   m_nRdataLen;

    CDNSFrame*  m_pDNSFrame;
};



class CDNSFrame
{
public:

    CDNSFrame();
    bool ParseDNSFrame(BJ_UINT8* pBuffer,BJ_INT32 nLength,BJ_UINT64 frameTime);


    CDNSRecord* GetDnsRecord(int nIndex);
    CDNSRecord* FindAdditionRecord(BJString& sName, BJ_INT16 nType);

    BJ_UINT16 GetQuestionCount() {return m_nQuestionCount;};
    BJ_UINT16 GetAnswerCount(){return m_nAnswersCount;};
    BJ_UINT16 GetMaxRecords(){return m_nMaxItems;};
    BJ_UINT8* GetBuffer() { return m_pStartBuffer;};

    BJ_BOOL ParseDnsRecord(CDNSRecord::dnsItemType eItemType);

    BJ_BOOL IsQueryFrame();
    BJ_BOOL IsWakeFrame();
    BJ_BOOL IsTruncatedFrame();

    BJ_BOOL HasOnlyService(BJString sName, BJ_INT16 nRecType);

    void SetAddress(BJIPAddr *SourceIPAddress,BJMACAddr *SourceMACAddress);

    bool GetTracingInfo(BJ_UINT8 &platform, BJ_UINT32 &version, BJMACAddr &mac);

    BJ_UINT64 GetTime() {return m_Time;};


    BJIPAddr m_Servicev4Address;
    BJIPAddr m_Servicev6Address;

    BJIPAddr   m_SourceIPAddress;
    BJMACAddr  m_SourceMACAddress;

protected:


    BJ_UINT8* m_pStartBuffer;
    BJ_UINT8* m_pEndBuffer;
    BJ_UINT8* m_pCurrentBuffer;
    BJ_INT32  m_nBufferLen;

    BJ_INT32 m_nFrameLen;

    // Header
    BJ_UINT16 m_nId;
    BJ_UINT16 m_nFlags;

    BJ_UINT16 m_nQuestionCount;
    BJ_UINT16 m_nAnswersCount;
    BJ_UINT16 m_NSCOUNT;
    BJ_UINT16 m_ARCOUNT;


    CDNSRecord m_dnsItems[MAX_DNS_RECORDS_PER_FRAME];

    int m_nMaxItems;

    BJ_UINT64 m_Time;

};





#endif /* defined(__TestTB__DNSFrame__) */
