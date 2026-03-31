//
//  CollectBy.h
//  TestTB
//
//  Created by Terrin Eager on 3/17/13.
//
//

#ifndef __TestTB__CollectBy__
#define __TestTB__CollectBy__

#include <iostream>
#include "bjtypes.h"
#include "DNSFrame.h"
#include "bjstring.h"
#include "LLRBTree.h"


// Service request/Respond v4/v6 sameSubnet/DifferentSubnet

enum BJ_COLLECTBY_TYPE
{
    CBT_NOT_SET,
    CBT_SERVICE,
    CBT_REQUEST_RESPONDS,
    CBT_SAME_DIFF_SUBNET,
    CBT_IP_ADDRESS_TYPE,
    CBT_PACKET
};

class CollectByAbstract;
class Collection
{
public:
    Collection() {m_pHeaderCollectBy = NULL;m_pFirstCollectBy = NULL;};

    void Init(BJ_COLLECTBY_TYPE collectByList[]);
    void ProcessFrame(CDNSFrame* pFrame);

    void ExportCollection(BJString sFileName);
    bool IsValid() { return (m_pFirstCollectBy != NULL);};
private:
    CollectByAbstract* Factory(BJ_COLLECTBY_TYPE type);

    BJ_COLLECTBY_TYPE m_CollectByList[20];
    CollectByAbstract* m_pHeaderCollectBy;
    CollectByAbstract* m_pFirstCollectBy;

};

class CollectByAbstract
{
public:
    CollectByAbstract()
    {
        pNext = NULL;
    }

    virtual void Collect(CDNSFrame* pFrame,CollectByAbstract* nextCollectBy)=0;
    virtual const char* GetTitle()=0;
    virtual CollectByAbstract* Factory()=0;
    virtual void Export(FILE* hFile,BJString sPrevColumns)=0;

    CollectByAbstract* pNext;
};
/////////////
// Service
class CServiceNode : public CRBNode<BJString>
{
public:
    CServiceNode() {pNext = NULL;};
    CServiceNode(BJString* pKey){ m_Key = *pKey;};
    ~CServiceNode(){};
    inline virtual BJ_COMPARE Compare(BJString* pKey) { return m_Key.Compare(*pKey);};
    inline virtual void CopyNode(CRBNode* pSource) {pNext = dynamic_cast<CServiceNode*>(pSource)->pNext;} ;
    inline virtual void Init(){};
    inline virtual void Clear() {};
    void Export(FILE* hFile,BJString sPrevColumns);
    CollectByAbstract* pNext;

};

class CServiceToCollectByMap : public CLLRBTree<BJString, CServiceNode>
{
public:


};

class CollectByService:public CollectByAbstract
{
public:
    virtual void Collect(CDNSFrame* pFrame,CollectByAbstract* nextCollectBy);
    virtual const char* GetTitle() {return "Service";};
    virtual CollectByAbstract* Factory(){ return new CollectByService();};
    virtual void Export(FILE* hFile,BJString sPrevColumns);
private:
    CServiceToCollectByMap m_Cache;
};

class CollectByRequestResponds:public CollectByAbstract
{
public:
    virtual void Collect(CDNSFrame* pFrame,CollectByAbstract* nextCollectBy);
    virtual const char* GetTitle() {return "Request/Responds";};
    virtual CollectByAbstract* Factory(){ return new CollectByRequestResponds();};
    virtual void Export(FILE* hFile,BJString sPrevColumns);

private:
    CollectByAbstract* pRequestNext;
    CollectByAbstract* pRespondsNext;
};

class CollectByIPAddressType:public CollectByAbstract
{
public:
    virtual void Collect(CDNSFrame* pFrame,CollectByAbstract* nextCollectBy);
    virtual const char* GetTitle() {return "V4/V6";};
    virtual CollectByAbstract* Factory(){ return new CollectByIPAddressType();};
    virtual void Export(FILE* hFile,BJString sPrevColumns);
private:
    CollectByAbstract* pIPv4Next;
    CollectByAbstract* pIPv6Next;
};

class CollectBySameSubnetDiffSubnet:public CollectByAbstract
{
public:
    virtual void Collect(CDNSFrame* pFrame,CollectByAbstract* nextCollectBy);
    virtual const char* GetTitle() {return "SameSubnet/DiffSubnet";};
    virtual CollectByAbstract* Factory(){ return new CollectBySameSubnetDiffSubnet();};
    virtual void Export(FILE* hFile,BJString sPrevColumns);

    static bool bSameSubnet;
private:
    CollectByAbstract* pSameSubnetNext;
    CollectByAbstract* pDiffSubnetNext;
};

class CollectByPacketCount:public CollectByAbstract
{

public:
    virtual void Collect(CDNSFrame* pFrame,CollectByAbstract* nextCollectBy);
    virtual const char* GetTitle() {return "Packets";};
    virtual CollectByAbstract* Factory(){ return new CollectByPacketCount();};
    virtual void Export(FILE* hFile,BJString sPrevColumns);

    BJ_INT64 nFrameCount;
    BJ_INT64 nLastFrameIndex;
    static BJ_INT64 nFrameIndex;
};


#endif /* defined(__TestTB__CollectBy__) */
