//
//  BonjourTop.h
//  TestTB
//
//  Created by Terrin Eager on 9/26/12.
//
//

#ifndef __TestTB__BonjourTop__
#define __TestTB__BonjourTop__

#include <iostream>
#include <stdio.h>
#include <map>
#include <utility>

#include "bjtypes.h"
#include "bjsocket.h"
#include "LLRBTree.h"
#include "DNSFrame.h"
#include "bjStringtoStringMap.h"
#include "bjstring.h"

#include "CollectBy.h"

using namespace std;

typedef struct device_count {
    BJ_UINT64 iOS = 0;
    BJ_UINT64 OSX = 0;
    BJ_UINT64 unknownOS = 0;
    BJ_UINT64 OSXWithEDNSField = 0;
    BJ_UINT64 iOSWithEDNSField = 0;
} device_count;

class CSocketStats
{
public:
    CSocketStats();
    void Init();
    void Clear();
    BJ_UINT64 m_nFrameCount;

    BJ_UINT64 m_nQuestionOnlyFrames;
    BJ_UINT64 m_nAnswerOnlyFrames;
    BJ_UINT64 m_nQandAFrames;

    BJ_INT32 m_SampleDay;

};

class FrameCount
{
public:
    FrameCount() { count = 0;lastFrameNumber = -1;};
    void Increment(BJ_UINT64 frameNumber)
    {
        if (frameNumber != lastFrameNumber)
            count++;
        lastFrameNumber = frameNumber;

    };
    BJ_UINT64 GetValue() const {return count;};
    void Reset() { count = 0;lastFrameNumber = -1;};
    FrameCount &operator+=(const FrameCount &src) { count += src.count; return *this;};

private:
    BJ_UINT64       count;
    BJ_UINT64       lastFrameNumber;
};

////////////////
class CDeviceMap;
class CDeviceNode;

class CIPDeviceNode : public CRBNode<BJIPAddr>
{
public:
    CIPDeviceNode(BJIPAddr* pSrc) {m_Key = *pSrc; /* temp deviceOS = '?';lastQUFrame = 0;*/pDeviceNode=NULL;};
    CIPDeviceNode(){ /*deviceOS = '?';lastQUFrame = 0; */pDeviceNode=NULL;};
    ~CIPDeviceNode(){};
    inline virtual BJ_COMPARE Compare(BJIPAddr* pKey) {return m_Key.Compare(pKey);};
    inline virtual void CopyNode(CRBNode* pSrc)
    {
        m_Key.Set(((BJIPAddr*)pSrc)->Getin6_addr());
        pDeviceNode = ((CIPDeviceNode*)pSrc)->pDeviceNode;
    };
    void Init() {};
    void Clear() {};


    CDeviceNode* pDeviceNode;

};

class CIPAddrMap: public CLLRBTree<BJIPAddr,CIPDeviceNode>
{
public:
    void GetDeviceOSTypes(CIPDeviceNode* node, CIPAddrMap* pGobalMap, BJ_UINT64& iOS,BJ_UINT64& OSX,BJ_UINT64& unknowOS);
};

////////////////////
class CDeviceNode : public CRBNode<BJString>
{
public:
    CDeviceNode(BJString* pSrc) {m_Key = *pSrc; deviceOS = '?'; bOSXWithEDNSField = false; biOSWithEDNSField = false; bDuplicate = false; bIPName = false; bHasFrames = false; nCreateCount++;};
    CDeviceNode(){deviceOS = '?'; bDuplicate = false; bIPName = false;  bHasFrames = false; nCreateCount++;};
    ~CDeviceNode(){nCreateCount--;};
    inline virtual BJ_COMPARE Compare(BJString* pKey) {return m_Key.Compare(*pKey);};
    inline virtual void CopyNode(CRBNode* pSource)
    {
        m_Key = pSource->m_Key;
        deviceOS = dynamic_cast<CDeviceNode*>(pSource)->deviceOS;
        model = dynamic_cast<CDeviceNode*>(pSource)->model;
        settingService = dynamic_cast<CDeviceNode*>(pSource)->settingService;
        macAddress = dynamic_cast<CDeviceNode*>(pSource)->macAddress;
        ipAddressv4 = dynamic_cast<CDeviceNode*>(pSource)->ipAddressv4;
        ipAddressv6 = dynamic_cast<CDeviceNode*>(pSource)->ipAddressv6;
        bOSXWithEDNSField = dynamic_cast<CDeviceNode*>(pSource)->bOSXWithEDNSField;
        biOSWithEDNSField = dynamic_cast<CDeviceNode*>(pSource)->biOSWithEDNSField;
        bHasFrames = dynamic_cast<CDeviceNode*>(pSource)->bHasFrames;
    };
    inline virtual void MergeData(CDeviceNode* src)
    {
        deviceOS = src->deviceOS;
        model = src->model;
        settingService = src->settingService;
        macAddress = src->macAddress;
        bOSXWithEDNSField = src->bOSXWithEDNSField;
        biOSWithEDNSField = src->biOSWithEDNSField;

        frameTotal += src->frameTotal;
        questionFrame += src->questionFrame;
        QUFrame += src->QUFrame;
        answerFrame += src->answerFrame;
        bHasFrames |=  src->bHasFrames;
    };
    void ClearData()
    {
        frameTotal.Reset();
        questionFrame.Reset();
        QUFrame.Reset();
        answerFrame.Reset();
        bHasFrames = false;
    }
    void Init() {deviceOS = '?';};

    void Clear() {};
    char GetDeviceOS() {return deviceOS;};
    void SetDeviceOS(char t,const char* pSettingService)
    {
        BJString EDNS0 = "EDNS0 Trace";
        if (pSettingService == NULL)
            printf("SetDeviceOS: pSettingService is NULL\n");

        if (settingService != EDNS0 || EDNS0 == pSettingService)
        {
            settingService = pSettingService;;
            deviceOS = t;
        }
   //     if ( t != deviceOS && deviceOS != '?' && (deviceOS != 'b' ))
    //         printf("SetDeviceOS: %s deviceOS not equal %c by %s != %c by %s\n", m_Key.GetBuffer(),deviceOS,settingService.GetBuffer(),t,pSettingService);
    };
    void SetModel(char* pModel) {model = pModel;};
    void Export(FILE* file)
    {
        //  fprintf(hFile,"Name,IPAddress,MACAddress,OSType,Model,Method\n");
        if (m_rbRight)
            dynamic_cast<CDeviceNode*>(m_rbRight)->Export(file);

        if (!bDuplicate || frameTotal.GetValue() > 0)
        {
            fprintf(file,"\"%s\",\"%s\",\"%s\",\"%s\",%c,\"%s\",\"%s\",%llu,%llu,%llu,%llu\n",
                m_Key.GetBuffer(),
                ipAddressv4.GetString(),
                ipAddressv6.GetString(),
                macAddress.GetString(),
                deviceOS,
                model.GetBuffer()?model.GetBuffer():" ",
                (bDuplicate)?"dup":settingService.GetBuffer()?settingService.GetBuffer():" ",
                frameTotal.GetValue(),
                questionFrame.GetValue(),
                QUFrame.GetValue(),
                answerFrame.GetValue());
        }

        if (m_rbLeft)
            dynamic_cast<CDeviceNode*>(m_rbLeft)->Export(file);
    };

    BJMACAddr macAddress;
    BJIPAddr ipAddressv4;
    BJIPAddr ipAddressv6;
    BJString model;
    BJString settingService;
    bool     biOSWithEDNSField;
    bool     bOSXWithEDNSField;
    bool     bDuplicate;
    bool     bIPName;
    bool     bHasFrames;

    FrameCount frameTotal;
    FrameCount questionFrame;
    FrameCount QUFrame;
    FrameCount answerFrame;
    static int nCreateCount;

private:
    char deviceOS;
};

class CDeviceMap: public CLLRBTree<BJString,CDeviceNode>
{
public:
    void GetDeviceOSTypes(CDeviceNode *node, CDeviceMap *pGobalMap, device_count& dev_cnt);
};

//////////////
class CMACAddrNode: public CRBNode<BJMACAddr>
{
public:
    CMACAddrNode(BJMACAddr* pSrc) {m_Key.Set(pSrc->Get()); deviceOS = '?'; };
    CMACAddrNode(){deviceOS = '?';};
    ~CMACAddrNode(){};
    inline virtual BJ_COMPARE Compare(BJMACAddr* pKey) {return m_Key.Compare(pKey);};
    inline virtual void CopyNode(CRBNode* pSource)
    {
        m_Key.Set( pSource->m_Key.Get());
        deviceOS = dynamic_cast<CMACAddrNode*>(pSource)->deviceOS;
        model = dynamic_cast<CMACAddrNode*>(pSource)->model;
        method = dynamic_cast<CMACAddrNode*>(pSource)->method;
    };
    void Export(FILE* file)
    {
        if (m_rbRight)
            dynamic_cast<CMACAddrNode*>(m_rbRight)->Export(file);


        fprintf(file,"\"%s\",%c,\"%s\",\"%s\"\n",
                m_Key.GetString(),
                deviceOS,
                model.GetBuffer()?model.GetBuffer():" ",
                method.GetBuffer()?method.GetBuffer():" ");


        if (m_rbLeft)
            dynamic_cast<CMACAddrNode*>(m_rbLeft)->Export(file);
    };

    void Init() {deviceOS = '?';};
    void Clear(){};
    char deviceOS;
    BJString model;
    BJString method;

};

class CMACAddrTree: public CLLRBTree<BJMACAddr,CMACAddrNode>
{
public:

};
class CMACAddrDeviceNode: public CRBNode<BJMACAddr>
{
public:
    CMACAddrDeviceNode(BJMACAddr* pSrc) {m_Key.Set(pSrc->Get()); device = NULL; };
    CMACAddrDeviceNode(){device = NULL;};
    ~CMACAddrDeviceNode(){};
    inline virtual BJ_COMPARE Compare(BJMACAddr* pKey) {return m_Key.Compare(pKey);};
    inline virtual void CopyNode(CRBNode* pSource)
    {
        m_Key.Set( pSource->m_Key.Get());
        device = dynamic_cast<CMACAddrDeviceNode*>(pSource)->device;
    };
    void Init() {device = NULL;};
    void Clear(){};
    CDeviceNode *device;

};

class CMACDeviceMap: public CLLRBTree<BJMACAddr,CMACAddrDeviceNode>
{

};

/////////////

class CStringNode : public CRBNode<BJ_UINT64>
{
public:
    CStringNode(){Init();};
    CStringNode ( BJ_UINT64* Key) { Init(); m_Key = *Key;};
    inline virtual BJ_COMPARE Compare(BJ_UINT64* pKey)
	{

		if (m_Key < *pKey)
			return (BJ_GT);
		else if (m_Key > *pKey)
			return (BJ_LT);
		else
			return (BJ_EQUAL);
	}
    inline virtual void CopyNode(CRBNode* pSource)
	{
		CStringNode* pSrc = (CStringNode*) pSource;

		//  m_Key = pSrc->m_Key;
		m_nBytes = pSrc->m_nBytes;
		m_nFrames = pSrc->m_nFrames;
		m_nFramesiOS = pSrc->m_nFramesiOS;
		m_nFramesOSX = pSrc->m_nFramesOSX;
		m_nQuestionFrames = pSrc->m_nQuestionFrames;
		m_nQuestionFramesiOS = pSrc->m_nQuestionFramesiOS;
		m_nQuestionFramesOSX = pSrc->m_nQuestionFramesOSX;
		m_nAnswerFrames = pSrc->m_nAnswerFrames;
		m_nAnswerFramesiOS = pSrc->m_nAnswerFramesiOS;
		m_nAnswerFramesOSX = pSrc->m_nAnswerFramesOSX;
		strlcpy(m_Value,pSrc->m_Value,sizeof(m_Value));
		m_nDeviceAskingCount = pSrc->m_nDeviceAskingCount;
		m_nDeviceAskingiOSCount = pSrc->m_nDeviceAskingiOSCount;
		m_nDeviceAskingOSXCount = pSrc->m_nDeviceAskingOSXCount;
		m_nDeviceAnsweringCount = pSrc->m_nDeviceAnsweringCount;
		m_nDeviceAnsweringiOSCount = pSrc->m_nDeviceAnsweringiOSCount;
		m_nDeviceAnsweringOSXCount = pSrc->m_nDeviceAnsweringOSXCount;
		m_nDeviceTotalCount = pSrc->m_nDeviceTotalCount;
		m_nDeviceTotaliOSCount = pSrc->m_nDeviceTotaliOSCount;
		m_nDeviceTotalOSXCount = pSrc->m_nDeviceTotalOSXCount;
		m_nWakeFrames = pSrc->m_nWakeFrames;
		m_nLastWakeFrameIndex =  pSrc->m_nLastWakeFrameIndex;
		m_nGoodbyeFrames = pSrc->m_nGoodbyeFrames;
	}

    inline void Init() {
        m_nBytes = 0;
        m_nFrames = m_nFramesiOS = m_nFramesOSX = m_nQuestionFrames = m_nQuestionFramesiOS = m_nQuestionFramesOSX = m_nAnswerFrames = m_nAnswerFramesiOS = m_nAnswerFramesOSX = 0;
        m_nLastFrameIndex = 0;
        m_nLastQueryFrameIndex = 0;
        m_nLastRespondsFrameIndex = 0;
        m_nDeviceAskingCount = m_nDeviceAskingiOSCount = m_nDeviceAskingOSXCount = 0;
        m_nDeviceAnsweringCount = m_nDeviceAnsweringiOSCount = m_nDeviceAnsweringOSXCount = 0;
        m_nDeviceTotalCount = m_nDeviceTotaliOSCount = m_nDeviceTotalOSXCount = 0;
        m_nWakeFrames = 0;
        m_nGoodbyeFrames = 0;
        m_lastQUFrameTime = 0;
    };
    inline void Clear() {};

    void UpdateOSTypeCounts(CDeviceMap* pGobalMap,CIPAddrMap *pIp2NameMap);

    void Print(bool bCursers,bool bDescendingSort,BJ_UINT32 &nIndex,BJ_UINT32 nStartIndex,BJ_UINT32 nEndIndex);
    void Export(FILE* hFile);

    //  BJ_UINT64        m_Key;
    char            m_Value[255];
    BJ_UINT64       m_nBytes;
    BJ_UINT64       m_nFrames;
    BJ_UINT64       m_nFramesiOS;
    BJ_UINT64       m_nFramesOSX;
    BJ_UINT64       m_nQuestionFrames;
    BJ_UINT64       m_nQuestionFramesiOS;
    BJ_UINT64       m_nQuestionFramesOSX;
    BJ_UINT64       m_nAnswerFrames;
    BJ_UINT64       m_nAnswerFramesiOS;
    BJ_UINT64       m_nAnswerFramesOSX;
    BJ_UINT64       m_nLastFrameIndex;
    BJ_UINT64       m_nLastQueryFrameIndex;
    BJ_UINT64       m_nLastRespondsFrameIndex;
    BJ_UINT64       m_nLastWakeFrameIndex;
    CIPAddrMap      m_DeviceAskingTree;
    BJ_UINT64       m_nDeviceAskingCount;
    BJ_UINT64       m_nDeviceAskingiOSCount;
    BJ_UINT64       m_nDeviceAskingOSXCount;
    CIPAddrMap      m_DeviceAnsweringTree;
    BJ_UINT64       m_nDeviceAnsweringCount;
    BJ_UINT64       m_nDeviceAnsweringiOSCount;
    BJ_UINT64       m_nDeviceAnsweringOSXCount;
    CIPAddrMap      m_DeviceTotalTree;
    BJ_UINT64       m_nDeviceTotalCount;
    BJ_UINT64       m_nDeviceTotaliOSCount;
    BJ_UINT64       m_nDeviceTotalOSXCount;
    BJ_UINT64       m_nWakeFrames;
    BJ_UINT64       m_lastQUFrameTime;
    BJ_UINT64       m_nGoodbyeFrames;
};

class CStringTree: public CLLRBTree<BJ_UINT64,CStringNode>
{
public:

};

///////////

class CStringShortNode: public CRBNode<BJ_UINT64>
{
public:
    CStringShortNode(BJ_UINT64* key) {Init(); m_Key = *key;};
	inline virtual BJ_COMPARE Compare(BJ_UINT64* pKey)
	{

		if (m_Key < *pKey)
			return (BJ_GT);
		else if (m_Key > *pKey)
			return (BJ_LT);
		else
			return (BJ_EQUAL);
	}

    inline virtual void CopyNode(CRBNode* pSource)
	{
		CStringShortNode* pSrc = (CStringShortNode*) pSource;

		//  m_Key = pSrc->m_Key;
		m_nBytes = pSrc->m_nBytes;
		m_nFrames = pSrc->m_nFrames;
		m_nQuestionFrames = pSrc->m_nQuestionFrames;
		m_nAnswerFrames = pSrc->m_nAnswerFrames;
		strlcpy(m_Value,pSrc->m_Value,sizeof(m_Value));
		m_nDeviceAskingCount = pSrc->m_nDeviceAskingCount;
		m_nDeviceAnsweringCount = pSrc->m_nDeviceAnsweringCount;
		m_nDeviceTotalCount = pSrc->m_nDeviceTotalCount;
		m_nWakeFrames = pSrc->m_nWakeFrames;
		m_nLastWakeFrameIndex =  pSrc->m_nLastWakeFrameIndex;
		m_nGoodbyeFrames = pSrc->m_nGoodbyeFrames;
	}

    inline void Init() {
        m_nBytes = 0;
        m_nFrames = m_nQuestionFrames = m_nAnswerFrames = 0;
        m_nLastFrameIndex = m_nLastQueryFrameIndex = m_nLastRespondsFrameIndex = m_nLastWakeFrameIndex = 0;
        m_nDeviceAskingCount = m_nDeviceAnsweringCount = m_nDeviceTotalCount = 0;
        m_nWakeFrames = m_lastQUFrameTime = m_nGoodbyeFrames = 0;
    };
    inline void Clear(){};

    void Export(FILE* hFile);

    //  BJ_UINT64        m_Key;
    char            m_Value[255];
    BJ_UINT64       m_nBytes;
    BJ_UINT64       m_nFrames;
    BJ_UINT64       m_nQuestionFrames;
    BJ_UINT64       m_nAnswerFrames;
    BJ_UINT64       m_nLastFrameIndex;
    BJ_UINT64       m_nLastQueryFrameIndex;
    BJ_UINT64       m_nLastRespondsFrameIndex;
    BJ_UINT64       m_nLastWakeFrameIndex;
    CIPAddrMap      m_DeviceAskingTree;
    BJ_UINT64       m_nDeviceAskingCount;
    CIPAddrMap      m_DeviceAnsweringTree;
    BJ_UINT64       m_nDeviceAnsweringCount;
    CIPAddrMap      m_DeviceTotalTree;
    BJ_UINT64       m_nDeviceTotalCount;
    BJ_UINT64       m_nWakeFrames;
    BJ_UINT64       m_lastQUFrameTime;
    BJ_UINT64       m_nGoodbyeFrames;
};

class CStringShortTree: public CLLRBTree<BJ_UINT64, CStringShortNode>
{
public:
};

///////////


class CBonjourTop
{
public:
    CBonjourTop();

    void SetIPAddr(const char*);
    void LiveCapture();
    void CaptureFile();

    void PrintResults(int nSortCol, bool bSortAsc);
    void UpdateOSCounts();
    void PrintDetailResults(int nSortCol, bool bSortAsc);
    void ExportResults();
    void Reset();

    void WriteDeviceFile();
    void WriteVendorFile();

    void ProcessFrame(BJ_UINT8* pBuffer,BJ_INT32 nLength, BJ_UINT64 frameTime);
    bool Name2OSType(BJString name,CDeviceNode* device);

    void UpdateRecord(CStringTree &Cache,CDNSRecord* pDNSRecord,BJString& RecordName,BJString& ServiceName,BJ_UINT32 nBytes,bool bGoodbye);

    void UpdateShortRecordHelper(BJ_UINT32 cacheType, BJ_UINT32 tracePlatform, BJ_UINT32 traceVersion, char deviceOS, CDNSRecord* pDNSRecord,BJString& RecordName,BJString& ServiceName,BJ_UINT32 nBytes,bool bGoodbye);

    void UpdateShortRecord(CStringShortTree* Cache,CDNSRecord* pDNSRecord,BJString& RecordName,BJString& ServiceName,BJ_UINT32 nBytes,bool bGoodbye);

    void GetOSTypeFromQuery(CDNSRecord *pDNSRecord,BJString& ServiceName);
    void GetOSTypeFromRegistration(CDNSRecord *pDNSRecord,BJString& ServiceName);

    CStringNode* GetCurrentDisplayRoot(BJString &sTitle);
    void ExportPtrCache(FILE* hFile, BJString sTitle, CStringNode* pRoot);
    void ExportShortCache(FILE* hFile, BJString sTitle, map<BJString, CStringShortTree*>* myMap);
    void ExportShortCacheHelper(FILE* hFile, BJString sTitle, CStringShortNode* pRoot);

    void WindowSizeChanged();

    bool m_bCursers;
    const char* m_pTcpDumpFileName;
    const char* m_pExportFileName;
    bool window_size_changed;
    bool m_bImportExportDeviceMap;
    BJString m_DeviceFileName;

    CDNSFrame m_Frame;

#define NUM_SOCKET_STATUS   6
#define HOURS_IN_DAY        24
#define MINUTES_IN_HOUR     60

    CSocketStats m_SocketStatus[NUM_SOCKET_STATUS];

    CSocketStats m_MinSnapshot[HOURS_IN_DAY][MINUTES_IN_HOUR];

    BJ_UINT64 m_nFrameCount;
    BJ_UINT64 m_nTotalBytes;
    long m_StartTime;
    long m_EndTime;
    BJ_UINT64 m_MinAnswerCountForTruncatedFrames;
    BJ_UINT64 m_AvgAnswerCountForTruncatedFrames;
    BJ_UINT64 m_MaxAnswerCountForTruncatedFrames;

    BJIPAddr m_IPv4Addr;

    BJStringtoStringMap m_Service2AppMap;

    BJStringtoStringMap m_Service2osRegisterMap;
    BJStringtoStringMap m_Service2osBrowseMap;

    enum BJ_DISPLAY_MODE_ENUM {
        BJ_DISPLAY_APP,
        BJ_DISPLAY_APPv6,
        BJ_DISPLAY_SERVICE,
        BJ_DISPLAY_SERVICEv6,
        BJ_DISPLAY_24_MIN
    } m_CurrentDisplay ;

    BJ_INT32 m_SnapshotSeconds;

    CStringTree m_ServicePtrCache;
    CStringTree m_ApplPtrCache;

    CStringTree m_ServicePtrCacheIPv6;
    CStringTree m_ApplPtrCacheIPv6;

    map<BJString, CStringShortTree*> m_ServiceBreakdownIPv4OSX;
    map<BJString, CStringShortTree*> m_ServiceBreakdownIPv4iOS;
    map<BJString, CStringShortTree*> m_ServiceBreakdownIPv6OSX;
    map<BJString, CStringShortTree*> m_ServiceBreakdownIPv6iOS;

    map<BJString, CStringShortTree*> m_AppBreakdownIPv4OSX;
    map<BJString, CStringShortTree*> m_AppBreakdownIPv4iOS;
    map<BJString, CStringShortTree*> m_AppBreakdownIPv6OSX;
    map<BJString, CStringShortTree*> m_AppBreakdownIPv6iOS;

    CDeviceMap m_DeviceMap;

    CMACAddrTree m_MacMap;
    CIPAddrMap  m_IPtoNameMap;
    CMACDeviceMap m_MACtoDevice;
    BJStringtoStringMap SVRtoDeviceName;

    Collection m_Collection;

    BJString interfaceName;
    BJString filterApplicationName;


};

#endif /* defined(__TestTB__BonjourTop__) */
