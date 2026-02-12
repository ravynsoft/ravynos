//
//  CollectBy.cpp
//  TestTB
//
//  Created by Terrin Eager on 3/17/13.
//
//

#include <iostream>
#include "bjtypes.h"
#include "DNSFrame.h"
#include "bjstring.h"
#include "LLRBTree.h"

#include "CollectBy.h"


//////////////////////
// Collection
void Collection::Init(BJ_COLLECTBY_TYPE collectByList[])
{
    CollectByAbstract* pLastCollectBy = nullptr;

    for (int i=0; i<20 && collectByList[i] != CBT_NOT_SET;i++)
    {
        m_CollectByList[i] = collectByList[i];
        if (i==0)
        {
            m_pHeaderCollectBy = Factory(m_CollectByList[i]);
            pLastCollectBy = m_pHeaderCollectBy;
            m_pFirstCollectBy = pLastCollectBy->Factory();
        }
        else
        {
            pLastCollectBy->pNext = Factory(m_CollectByList[i]);
            pLastCollectBy = pLastCollectBy->pNext;
        }
    }
}

void Collection::ProcessFrame(CDNSFrame* pFrame)
{
    m_pFirstCollectBy->Collect(pFrame,m_pHeaderCollectBy->pNext);
}

void Collection::ExportCollection(BJString sFileName)
{
    FILE* hFile = fopen(sFileName.GetBuffer(),"w");

    if (hFile == NULL)
    {
        printf("file open failed %s\n",sFileName.GetBuffer());
        return;
    }

    // Export Header Line
    CollectByAbstract *collectBy = m_pHeaderCollectBy;
    BJString sHeader;
    while (collectBy)
    {
        if (sHeader.GetBufferLength() != 0)
            sHeader += ",";
        sHeader += collectBy->GetTitle();
        collectBy = collectBy->pNext;
    }
    fprintf(hFile, "%s\n",sHeader.GetBuffer());

    m_pFirstCollectBy->Export(hFile,"");

    fclose(hFile);
}

CollectByAbstract* Collection::Factory(BJ_COLLECTBY_TYPE type)
{
    switch (type)
    {
        case CBT_NOT_SET:
            return NULL;
        case CBT_SERVICE:
            return new CollectByService();
        case CBT_REQUEST_RESPONDS:
            return new CollectByRequestResponds();
        case CBT_SAME_DIFF_SUBNET:
            return new CollectBySameSubnetDiffSubnet();
        case CBT_IP_ADDRESS_TYPE:
            return new CollectByIPAddressType();
        case CBT_PACKET:
            return new CollectByPacketCount();
        default:
            return NULL;
    }

}

/////////////
// CollectByService

void CServiceNode::Export(FILE* hFile,BJString sPrevColumns)
{
    if (pNext)
    {
        BJString sTemp = sPrevColumns;
        if (sPrevColumns.GetBufferLength())
            sTemp += ",";
        sTemp += m_Key;
        pNext->Export(hFile,sTemp);
    }
    if (m_rbLeft)
     dynamic_cast<CServiceNode*>(m_rbLeft)->Export(hFile,sPrevColumns);
    if (m_rbRight)
        dynamic_cast<CServiceNode*>(m_rbRight)->Export(hFile,sPrevColumns);
}

void CollectByService::Collect(CDNSFrame* pFrame,CollectByAbstract* nextCollectBy)
{
    for (int dnsItemsIndex =0; dnsItemsIndex < pFrame->GetQuestionCount()+pFrame->GetAnswerCount();dnsItemsIndex++)
    {
        BJString RecordName;
        CDNSRecord* pDNSRecord = pFrame->GetDnsRecord(dnsItemsIndex);
        if (pDNSRecord == NULL)
            continue;

        pDNSRecord->GetDnsRecordName(RecordName,0,99);

        if (RecordName.Contains("_kerberos."))
        {
            RecordName = "_kerberos.";
        }
        else
            pDNSRecord->GetDnsRecordName(RecordName, (pDNSRecord->m_RecType == 12)?0:1,99);

        if (pDNSRecord->m_RecType == 12)
        {
            if (RecordName.Contains(".ip6.arpa."))
                RecordName = "*.ip6.arpa.";
            else if (RecordName.Contains(".arpa."))
                RecordName = "*.arpa.";
        }
        if (pDNSRecord->m_RecType == 1)
            RecordName = "A";
        if (pDNSRecord->m_RecType == 28)
            RecordName = "AAAA";
        if (pDNSRecord->m_RecType == 255)
        {
            if (RecordName.Contains(".ip6.arpa."))
                RecordName = "ANY *.ip6.arpa.";
            else if (RecordName.Contains(".arpa."))
                RecordName = "ANY *.arpa.";
            else
                RecordName = "Any";
        }
        if (RecordName.Contains("_sub."))
        {
            pDNSRecord->GetDnsRecordName(RecordName,2,99); /// skip first label and _sub. label
        }


        CServiceNode *pNode= m_Cache.FindwithAddRecord(&RecordName);
        if ((pNode->pNext == NULL) && nextCollectBy)
            pNode->pNext = nextCollectBy->Factory();
        if (pNode->pNext)
            pNode->pNext->Collect(pFrame,nextCollectBy?nextCollectBy->pNext:NULL);

    }

}




void CollectByService::Export(FILE* hFile,BJString sPrevColumns)
{

    // loop thur list
    CServiceNode *pNode = m_Cache.GetRoot();

    if (pNode)
        pNode->Export(hFile,sPrevColumns);
}

/////////////
//  CollectByRequestResponds
void CollectByRequestResponds::Collect(CDNSFrame* pFrame,CollectByAbstract* nextCollectBy)
{
    if (pFrame->IsQueryFrame())
    {
        if (pRequestNext == NULL)
            pRequestNext = nextCollectBy->Factory();
        pRequestNext->Collect(pFrame, nextCollectBy?nextCollectBy->pNext:NULL);
    }
    else
    {
        if (pRespondsNext == NULL)
            pRespondsNext = nextCollectBy->Factory();
        pRespondsNext->Collect(pFrame, nextCollectBy?nextCollectBy->pNext:NULL);
    }
}

void CollectByRequestResponds::Export(FILE* hFile,BJString sPrevColumns)
{
    if (pRequestNext)
    {
        BJString sTemp = sPrevColumns;
        if (sPrevColumns.GetBufferLength())
            sTemp += ",";
        sTemp += "Request";
        pRequestNext->Export(hFile,sTemp);
    }
    if (pRespondsNext)
    {
        BJString sTemp = sPrevColumns;
        if (sPrevColumns.GetBufferLength())
            sTemp += ",";
        sTemp += "Responds";
        pRespondsNext->Export(hFile,sTemp);
    }
}
/////////////
//  CollectByIPAddressType
void CollectByIPAddressType::Collect(CDNSFrame* pFrame,CollectByAbstract* nextCollectBy)
{
    if (pFrame->m_SourceIPAddress.IsIPv4())
    {
        if (pIPv4Next == NULL)
            pIPv4Next = nextCollectBy->Factory();
        pIPv4Next->Collect(pFrame, nextCollectBy?nextCollectBy->pNext:NULL);
    }
    if (pFrame->m_SourceIPAddress.IsIPv6())
    {
        if ((pIPv6Next == NULL) && nextCollectBy)
            pIPv6Next = nextCollectBy->Factory();
        if (pIPv6Next)
            pIPv6Next->Collect(pFrame, nextCollectBy?nextCollectBy->pNext:NULL);
    }
}
void CollectByIPAddressType::Export(FILE* hFile,BJString sPrevColumns)
{
    if (pIPv4Next)
    {
        BJString sTemp = sPrevColumns;
        if (sPrevColumns.GetBufferLength())
            sTemp += ",";
        sTemp += "IPv4";
        pIPv4Next->Export(hFile,sTemp);
    }
    if (pIPv6Next)
    {
        BJString sTemp = sPrevColumns;
        if (sPrevColumns.GetBufferLength())
            sTemp += ",";
        sTemp += "IPv6";
        pIPv6Next->Export(hFile,sTemp);
    }
}
/////////////
// CollectBySameSubnetDiffSubnet:

// static
bool CollectBySameSubnetDiffSubnet::bSameSubnet = true;

void CollectBySameSubnetDiffSubnet::Collect(CDNSFrame* pFrame,CollectByAbstract* nextCollectBy)
{
    if (bSameSubnet)
    {
        if (pSameSubnetNext == NULL)
            pSameSubnetNext = nextCollectBy->Factory();
        pSameSubnetNext->Collect(pFrame, nextCollectBy?nextCollectBy->pNext:NULL);
    }
    else
    {
        if (pDiffSubnetNext == NULL)
            pDiffSubnetNext = nextCollectBy->Factory();
        pDiffSubnetNext->Collect(pFrame, nextCollectBy?nextCollectBy->pNext:NULL);
    }

}
void CollectBySameSubnetDiffSubnet::Export(FILE* hFile,BJString sPrevColumns)
{
    if (pSameSubnetNext)
    {
        BJString sTemp = sPrevColumns;
        if (sPrevColumns.GetBufferLength())
            sTemp += ",";
        sTemp += "SameSubnet";
        pSameSubnetNext->Export(hFile,sTemp);
    }
    if (pDiffSubnetNext)
    {
        BJString sTemp = sPrevColumns;
        if (sPrevColumns.GetBufferLength())
            sTemp += ",";
        sTemp += "WrongSubnet";
        pDiffSubnetNext->Export(hFile,sTemp);
    }
}
/////////////
// CollectByPacketCount

// staticCollectByPacketCount
BJ_INT64 CollectByPacketCount::nFrameIndex = 0;

void CollectByPacketCount::Collect(CDNSFrame* ,CollectByAbstract* )
{
    if (nFrameIndex != nLastFrameIndex)
    {
        nFrameCount++;
        nLastFrameIndex = nFrameIndex;
    }
}
void CollectByPacketCount::Export(FILE* hFile,BJString sPrevColumns)
{

    fprintf(hFile,"%s,%llu\n",sPrevColumns.GetBuffer(),nFrameCount);
}


