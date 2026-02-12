//
//  DNSFrame.cpp
//  TestTB
//
//  Created by Terrin Eager on 9/26/12.
//
//

#include "DNSFrame.h"

#define DNS_LABEL_MAX_LENGTH    63
#define DNS_NAME_MAX_LENGTH     255

void CDNSRecord::GetDnsRecordName(BJString& ReturnString,int nLabelToSkip,int nMaxLabel)
{
    GetDnsRecordNameFromBuffer(m_pStartofRec, ReturnString, nLabelToSkip, nMaxLabel);
}

void CDNSRecord::GetDnsRecordNameFromBuffer(BJ_UINT8* pBuffer,BJString& ReturnString,int nLabelToSkip,int nMaxLabel)
{
    BJ_UINT8* pNameBuffer = NULL;
    int nOffset = 0;
   // char* pTemp = pReturnBuffer;
    int nCharCount  = 0;
    int nSkippedLabels = 0;
    int nLabelProcessed = 0;
    ReturnString.Set(NULL,255);

    if (ReturnString.GetBuffer() == NULL)
        return;

    pNameBuffer = pBuffer;
    if (pNameBuffer == NULL)
    {
        return;
    }

    while (ReturnString.GetBufferLength() < 1024)
    {
        nCharCount = *pNameBuffer++;
        if (nCharCount == 0)
            break;

        if ((nCharCount&(DNS_NAME_OFFSET_MASK)) == DNS_NAME_OFFSET_MASK)
        {
            nOffset = *pNameBuffer++;
            nOffset |= (nCharCount&(~DNS_NAME_OFFSET_MASK)) << 8;
            pNameBuffer = m_pDNSFrame->GetBuffer() + nOffset;
            continue;
        }

        if (nCharCount > DNS_LABEL_MAX_LENGTH)
        {
            printf("label too long %d\n",nCharCount);
            break;
        }

        if (ReturnString.GetLength() + nCharCount + 1 > DNS_NAME_MAX_LENGTH) // + 1 is for the '.' added later on
        {
            printf("Name exceeded limit allowed for DNS: %d\n", ReturnString.GetLength() + nCharCount + 1);
            break;
        }

        if (nLabelToSkip > nSkippedLabels)
        {
            nSkippedLabels++;
            pNameBuffer += nCharCount;
            continue;
        }
        ReturnString.Append((char*)pNameBuffer, nCharCount);
        pNameBuffer+= nCharCount;
        nLabelProcessed++;

        if (nLabelProcessed >= nMaxLabel)
            return;

        ReturnString += ".";
    }
}




CDNSFrame::CDNSFrame()
{

    for(int nIndex=0; nIndex < MAX_DNS_RECORDS_PER_FRAME; nIndex++)
        m_dnsItems[nIndex].m_pDNSFrame = this;

}

CDNSRecord* CDNSFrame::GetDnsRecord(int nIndex)
{
    if (nIndex > m_nMaxItems)
        return NULL;
    return &m_dnsItems[nIndex];
}

bool CDNSFrame::ParseDNSFrame(BJ_UINT8* pBuffer,BJ_INT32 nLength, BJ_UINT64 frameTime)
{
    if (pBuffer == NULL)
        return false;

    int nIndex = 0;

    m_Servicev4Address.Empty();
    m_Servicev6Address.Empty();

    m_pStartBuffer = pBuffer;
    m_nFrameLen = (BJ_UINT32) nLength;

    m_pCurrentBuffer = m_pStartBuffer;
    m_pEndBuffer = m_pStartBuffer + m_nFrameLen;
    m_Time = frameTime;

    m_nId = PF_GET_UINT16(m_pStartBuffer,0);
    m_nFlags = PF_GET_UINT16(m_pStartBuffer,2);
    m_nQuestionCount = PF_GET_UINT16(m_pStartBuffer,4);
    m_nAnswersCount = PF_GET_UINT16(m_pStartBuffer,6);
    m_NSCOUNT = PF_GET_UINT16(m_pStartBuffer,8);
    m_ARCOUNT = PF_GET_UINT16(m_pStartBuffer,10);

    m_nMaxItems = 0;



    // printf("FrameNum= %d,nQuestionCount= %d nAnswersCount= %d NSCOUNT= %d ARCOUNT= %d\n",nFrameCount++,m_nQuestionCount, m_nAnswersCount,m_NSCOUNT,  m_ARCOUNT);

    m_pCurrentBuffer = m_pStartBuffer + 12;


    for (nIndex =0; nIndex < m_nQuestionCount;nIndex++)
    {
        //      printf("FramePosition= %ld  ",m_pCurrentBuffer);
        ParseDnsRecord(CDNSRecord::Question);

    }
    for (nIndex =0; nIndex < m_nAnswersCount;nIndex++)
    {
        //      printf("FramePosition= %ld  ",m_pCurrentBuffer);
        ParseDnsRecord(CDNSRecord::Answer);
    }
    for (nIndex =0; nIndex < m_NSCOUNT;nIndex++)
    {
        //      printf("FramePosition= %ld  ",m_pCurrentBuffer);
        ParseDnsRecord(CDNSRecord::Answer);
    }
    for (nIndex =0; nIndex < m_ARCOUNT;nIndex++)
    {
        //      printf("FramePosition= %ld  ",m_pCurrentBuffer);
        ParseDnsRecord(CDNSRecord::Answer);
        CDNSRecord* pRecord =  &m_dnsItems[m_nMaxItems-1];
        if (pRecord->m_RecType == DNS_TYPE_AAAA && m_Servicev6Address.IsEmpty())
        {
            m_Servicev6Address.Setv6Raw(pRecord->GetStartofRdata());
        }
        if (pRecord->m_RecType == DNS_TYPE_A && m_Servicev4Address.IsEmpty())
        {
            m_Servicev4Address.Setv4Raw(pRecord->GetStartofRdata());
        }
    }
    //
    ///   for (dnsItemsIndex =0; dnsItemsIndex < m_nQuestionCount+m_nAnswersCount;dnsItemsIndex++)
    ///   {
    ///       printf("Name = %s\n", GetDnsRecordName(&Frame,dnsItemsIndex,tempBuffer,sizeof(tempBuffer)));
    //   }
    return true;
}

BJ_BOOL CDNSFrame::ParseDnsRecord(CDNSRecord::dnsItemType eItemType)
{
    unsigned char nCharCount = 0;
    BJ_UINT8* pTemp = m_pCurrentBuffer;
    CDNSRecord* pRecord =  &m_dnsItems[m_nMaxItems++];

    //temp
    BJ_UINT16 nRdataLen = 0;

    if (pTemp > m_pEndBuffer)
    {
        printf("Error in ParseDnsRecord pBuffer > pEndBuffer\n");
        pRecord->m_pStartofRec = NULL;
        pRecord->m_nNameLength = 0;
        return false;
    }


    pRecord->m_pStartofRec = pTemp;
    pRecord->m_nNameLength = 0;
    pRecord->m_nRdataLen = 0;


    // Skip over Name;
    while (pTemp < m_pEndBuffer)
    {
        nCharCount = *pTemp;
        pTemp++;

        if (nCharCount == 0)
            break;

        if ((nCharCount&(DNS_NAME_OFFSET_MASK)) == DNS_NAME_OFFSET_MASK)
        {  // offset string
            pTemp++;
            break;
        }

        if (nCharCount > DNS_LABEL_MAX_LENGTH)
        {
            printf("%d. label too long %d\n",m_nMaxItems-1,nCharCount);
        }

        if (pTemp + nCharCount < m_pEndBuffer)
            pTemp += nCharCount;
        else
            pTemp = m_pEndBuffer;
    }

    pRecord->m_nNameLength = (BJ_UINT32)(pTemp - pRecord->m_pStartofRec);

    if (eItemType == CDNSRecord::Question)
    {
        pRecord->m_RecType = PF_GET_UINT16(pTemp,0);
        pRecord->m_RecClass = PF_GET_UINT16(pTemp,2);
        pRecord->m_nTTL = PF_GET_UINT16(pTemp,4);

        //   printf("Namelen=%u, Type=%u, class=%u, TTL=%u, RDLength=%u\n", m_dnsItems[ndnsIndex].nNameLength,nType,nClass,nTTL,nRdataLen);

        pTemp += 4;
    }
    else
    {

        pRecord->m_RecType = PF_GET_UINT16(pTemp,0);
        pRecord->m_RecClass = PF_GET_UINT16(pTemp,2);

        pRecord->m_nTTL = PF_GET_UINT32(pTemp,4);
        pRecord->m_nRdataLen = PF_GET_UINT16(pTemp,8);
        if (nRdataLen > 1024*10)
        {
            printf("large Rdata ??");
        }
        //    printf("Namelen=%u, Type=%u, class=%u, TTL=%u, RDLength=%u\n", m_dnsItems[ndnsIndex].nNameLength,m_dnsItems[ndnsIndex].RecType,nClass,nTTL,m_dnsItems[ndnsIndex].nRdataLen);
        pTemp += 10 + pRecord->m_nRdataLen;
    }

    m_pCurrentBuffer = pTemp;

    return true;
}

BJ_BOOL CDNSFrame::IsQueryFrame()
{
    return !(m_nFlags&0x8000);
}

#define UNICAST_RESPONDS_REQUESTED 0x8000
BJ_BOOL CDNSFrame::IsWakeFrame()
{

    for (int i=0; i < m_nQuestionCount; i++)
    {
        if (m_dnsItems[i].m_RecType == DNS_TYPE_PTR &&  m_dnsItems[i].m_RecClass & UNICAST_RESPONDS_REQUESTED)
            return true;
    }

    return false;
}
#define DNS_HEADER_TRUNCATEED 0x0200
BJ_BOOL  CDNSFrame::IsTruncatedFrame()
{
       return (m_nFlags&DNS_HEADER_TRUNCATEED);
}


BJ_BOOL CDNSFrame::HasOnlyService(BJString sName, BJ_INT16 nRecType)
{
 /*   if (IsQueryFrame())
    {
        for (int i=0; i < m_nQuestionCount; i++)
        {
            CBJString sRecordName;
            m_dnsItems[i].GetDnsRecordName(sRecordName, 0);
            if (m_dnsItems[i].m_RecType != nRecType && nRecType != -1)
                return false;

            if (!sRecordName.Contains(sName.GetBuffer()))
                return false;
        }
    }
    else*/
    {
        for (int i=0; i < m_nQuestionCount+m_nAnswersCount; i++)
        {
            BJString sRecordName;
            m_dnsItems[i].GetDnsRecordName(sRecordName, 0,99);
            if (m_dnsItems[i].m_RecType != nRecType && nRecType != -1)
                return false;

            if (!sRecordName.Contains(sName.GetBuffer()))
                return false;
        }


    }


    return true;
}

CDNSRecord* CDNSFrame::FindAdditionRecord(BJString& sName,BJ_INT16 nRecType)
{
    for (int i = 0; i < m_nMaxItems; i++)
    {
        if (m_dnsItems[i].m_RecType != nRecType && nRecType != -1)
            continue;
        BJString sRecordName;
        m_dnsItems[i].GetDnsRecordName(sRecordName, 0,99);
        if (sRecordName == sName)
            return &m_dnsItems[i];
    }
    return NULL;
}

void CDNSFrame::SetAddress(BJIPAddr *pSourceIPAddress,BJMACAddr *pSourceMACAddress)
{
    m_SourceIPAddress = *pSourceIPAddress;
    m_SourceMACAddress = *pSourceMACAddress;


}


bool CDNSFrame::GetTracingInfo(BJ_UINT8 &platform, BJ_UINT32 &version, BJMACAddr &)
{
    // Find OPT record
    for (int i = m_nQuestionCount + m_nAnswersCount + m_NSCOUNT; i < m_nMaxItems; i++)
    {
        if (m_dnsItems[i].m_RecType == DNS_TYPE_OPT)
        {
            BJ_UINT8* rdata = m_dnsItems[i].GetStartofRdata();

            BJ_UINT8* rdataEnd = rdata +  m_dnsItems[i].m_nRdataLen;

            while (rdata < rdataEnd)
            {
                BJ_UINT16 type = PF_GET_UINT16(rdata,0);
                BJ_UINT16 len = PF_GET_UINT16(rdata,2);

                if (type == DNS_EDNS0_TRACE)
                {
                    platform = PF_GET_UINT8(rdata,4);
                    if (len == 3)   // EDNS field of length 3 <rdar://15101783>
                    {
                        version = static_cast<BJ_UINT32>(PF_GET_UINT16(rdata,5));
                    }
                    else if (len == 5) // EDNS field of length 5 <rdar://15235603>
                    {
                        version = static_cast<BJ_UINT32>(PF_GET_UINT32(rdata, 5));
                    }
                    else
                    {
                        return false;
                    }
                    return true;
                }


                rdata += sizeof(BJ_UINT16)*2 + len;
            }

        }
    }
    return false;

}





