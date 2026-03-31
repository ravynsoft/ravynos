//
//  BonjourTop.cpp
//  TestTB
//
//  Created by Terrin Eager on 9/26/12.
//
//

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ncurses.h>

#include "BonjourTop.h"
#include "DNSFrame.h"
#include "CaptureFile.h"
#include "bjsocket.h"
#include "bjstring.h"

#include <sys/sysctl.h>
#include <mach/mach.h>

#define SERVICE_IPV4    0
#define SERVICE_IPV6    1
#define APP_IPV4        2
#define APP_IPV6        3

#define TRACE_PLATFORM_UNKNOWN              0
#define TRACE_PLATFORM_OSX                  1
#define TRACE_PLATFORM_iOS                  2
#define TRACE_PLATFORM_APPLE_TV             3
#define TRACE_PLATFORM_NON_APPLE_PLATFORM   4
#define DISCOVERYD_TRACE_PLATFORM_OSX                   (1 | 0x80)
#define DISCOVERYD_TRACE_PLATFORM_iOS                   (2 | 0x80)
#define DISCOVERYD_TRACE_PLATFORM_APPLE_TV              (3 | 0x80)
#define DISCOVERYD_TRACE_PLATFORM_NON_APPLE_PLATFORM    (4 | 0x80)

int CDeviceNode::nCreateCount = 0;
static integer_t Usage(void);

char CVSFileNameExample[] ="BonjourTop";
char DeviceNameExample[] ="BonjourTopDevice.csv";

char Service2App[][50] = {
    // Service_Name,             Application_Name,    Browse_OS_Type, Register_OS_Type
    "_device-info._tcp.local.",     "Device-Info",  "?" , "?",
    "_rfb._tcp.local.",             "Finder",       "X" , "?",
    "_afpovertcp._tcp.local.",      "Finder",       "?" , "X",
    "_adisk._tcp.local.",           "Finder",       "?" , "X",
    "_odisk._tcp.local.",           "Finder",       "?" , "X",
    "_smb._tcp.local.",             "Finder",       "X" , "X",
    "_smb2._tcp.local.",            "Finder",       "X" , "X",
    "_workstation._tcp.local.",     "Finder",       "X" , "X",
    "_kerberos.",                   "Finder",       "X" , "X",
    "_nfs._tcp.local.",             "Finder",       "X" , "X",
    "_ftp._tcp.local.",             "Finder",       "X" , "X",

    "_appletv._tcp.local.",         "AppleTV",      "?" , "t",
    "_appletv-v2._tcp.local.",      "AppleTV",      "?" , "?",
    "_appletv-pair._tcp.local.",    "AppleTV",      "?" , "?",

    "A",                            "LinkLocal",    "?" , "?",
    "AAAA",                         "LinkLocal",    "?" , "?",
    "*.ip6.arpa.",                  "LinkLocal",     "?" , "?",
    "*.arpa.",                      "LinkLocal",    "?" , "?",

    "_airplay._tcp.local.",         "AirPlay",      "?" , "t",
    "_airplayTXT",                  "AirPlay","?" , "t",
    "_raop._tcp.local.",            "AirPlay","?" , "?",

    "_ubd._tcp.local.",             "Ubiquity",     "?" , "?",
    "_ubiquity._tcp.local.",        "Ubiquity",     "?" , "?",
    "_ubiquityV1._tcp.local.",        "Ubiquity",     "?" , "?",
    "_ubiquityV2._tcp.local.",        "Ubiquity",     "?" , "?",

    " _ipps._tcp.local.",           "Printing",     "?" , "?",
    "_ipp._tcp.local.",             "Printing",     "?" , "?",
    "_ipps._tcp.local.",            "Printing",     "?" , "?",
    "_ipp-tls._tcp.local.",         "Printing",     "?" , "?",
    "_printer._tcp.local.",         "Printing",     "?" , "?",
    "_scanner._tcp.local.",         "Printing",     "?" , "?",
    "_pdl-datastream._tcp.local.",  "Printing",     "?" , "?",
    "_fax-ipp._tcp.local.",         "Printing",     "?" , "?",

    "_apple-mobdev._tcp.local.",    "iTunes-WiFiSync","?" , "i",
    "_daap._tcp.local.",            "iTunes",       "?" , "?",

    "_sftp-ssh._tcp.local.",        "Terminal",     "?" , "X",
    "_ssh._tcp.local.",             "Terminal",     "?" , "X",

    "_sleep-proxy._udp.local.",     "Sleep Proxy",  "?" , "?",
    "_keepalive._dns-sd._udp.local.","Sleep Proxy", "X" , "?",
    "_services._dns-sd._udp.local.", "Services",    "?" , "?",
    "ANY *.ip6.arpa.",              "Sleep Proxy",  "?" , "?",
    "ANY *.arpa.",                  "Sleep Proxy",  "?" , "?",

    "AirPort_presence._tcp.local.", "AirPort",      "?" , "?",
    "_airport._tcp.local.",         "AirPort",      "?" , "?",
    "_presence._tcp.local.",        "iChat",        "X" , "X",
    "_home-sharing._tcp.local.",    "HomeSharing",  "?" , "X",

    "_ptp._tcp.local.",             "iPhoto",       "?" , "X",
    "_ica-networking2._tcp.local.", "iPhoto",       "X" , "X",
    "_mobileiphoto._udp.local.",    "iPhoto",       "?" , "?",
    "_mobileiphoto2._udp.local.",   "iPhoto",       "?" , "?",
    "_dpap._tcp.local.",            "iPhoto",       "?" , "X",
    "_airdrop._tcp.local.",         "AirDrop",      "?" , "?",
    "_http._tcp.local.",            "Safari",       "X" , "X",
    "_net-assistant._udp.local.","Apple Remote Desktop","X" , "X",
    "_servermgr._tcp.local.",       "OSX Server",   "X" , "X",
    ""
};

char DeviceInfo2DeviceOS[][50] = {
    // deviceModel,   deviceType
    "MacBookAir",       "X",
    "MacBookPro",       "X",
    "Macmini",          "X",
    "iMac",             "X",
    "MacPro",           "X",
    "MacBook",          "X",
    "AAPLJ15",          "X",
    "AAPLJ41",          "X",
    "AAPLJ43",          "X",
    "AAPLJ45",          "X",
    "AAPLJ90",          "X",
    "AAPLJ17",          "X",
    "PowerMac",         "X",


    "J33AP",            "t",
    "J33iAP",           "t",
    "J71AP",            "i",
    "J72AP",            "i",
    "J75AP",            "i",
    "J85DEV",           "i",
    "K66AP",            "t",
    "N41AP",            "i",
    "N42AP",            "i",
    "N48AP",            "i",
    "N51AP",            "i",
    "N53AP",            "i",
    "N78AP",            "i",
    "P101AP",           "i",
    "P102AP",           "i",
    "P103AP",           "i",
    "P105AP",           "i",
    "P106AP",           "i",
    "P107AP",           "i",
    "AirPort",          "b",
    "TimeCapsule",          "b",
    ""
};

char Name2DeviceOS[][50] = {
    // Name contains,   osType
    "iPhone",           "i",
    "phone",            "i",
    "Phone",            "i",
    "iPod",             "i",
    "iPad",             "i",
    "ipad",             "i",
    "Apple-TV",         "t",
    "AppleTV",          "t",
    "MacBook",          "X",
    "macbook",          "X",
    "iMac",             "X",
    "macmini",          "X",
    ""
};

BJ_UINT64 Hash(const char* pStr);
BJ_UINT64 Hash2(char* pStr);


CSocketStats::CSocketStats()
{
    Init();

}
void CSocketStats::Init()
{
    m_nFrameCount = 0;

    m_nQuestionOnlyFrames = 0;
    m_nAnswerOnlyFrames = 0;
    m_nQandAFrames = 0;

    m_SampleDay = 0;
}

void CSocketStats::Clear()
{
    Init();
}

CBonjourTop::CBonjourTop()
{

    m_bCursers = true;
    m_pTcpDumpFileName = NULL;
    m_pExportFileName = CVSFileNameExample;
    m_DeviceFileName = DeviceNameExample;

    m_nFrameCount = 0;
    m_nTotalBytes = 0;

    m_StartTime = m_EndTime = time(NULL);

    m_SnapshotSeconds = 0;

    m_MinAnswerCountForTruncatedFrames = 0;
    m_AvgAnswerCountForTruncatedFrames = 0;
    m_MaxAnswerCountForTruncatedFrames = 0;

    window_size_changed = false;
    m_bImportExportDeviceMap = false;

    // loadup application mapping
    for(int i=0; Service2App[i][0] != 0;)
    {
        BJString a(Service2App[i++]);

        m_Service2AppMap.FindwithAddRecord(&a)->value = Service2App[i++];
        m_Service2osBrowseMap.FindwithAddRecord(&a)->value = Service2App[i++];
        m_Service2osRegisterMap.FindwithAddRecord(&a)->value = Service2App[i++];

    }


    m_CurrentDisplay = CBonjourTop::BJ_DISPLAY_APP;
  //  m_CurrentDisplay = CBonjourTop::BJ_DISPLAY_SERVICE;

    Usage();

}

static int CbClearDataDevice(const void* pNode, const void*)
{
    CDeviceNode* pDeviceRecord = (CDeviceNode*)pNode;

    pDeviceRecord->ClearData();
    return 0;
}

void CBonjourTop::Reset()
{
    m_nFrameCount = 0;
    m_nTotalBytes = 0;

    m_MinAnswerCountForTruncatedFrames = 0;
    m_AvgAnswerCountForTruncatedFrames = 0;
    m_MaxAnswerCountForTruncatedFrames = 0;

    m_StartTime = m_EndTime = time(NULL);

    m_ServicePtrCache.ClearAll();
    m_ApplPtrCache.ClearAll();

    m_ServicePtrCacheIPv6.ClearAll();
    m_ApplPtrCacheIPv6.ClearAll();

    // Clear all data in the map
    m_AppBreakdownIPv4OSX.clear();
    m_AppBreakdownIPv4iOS.clear();
    m_AppBreakdownIPv6OSX.clear();
    m_AppBreakdownIPv6iOS.clear();

    m_ServiceBreakdownIPv4OSX.clear();
    m_ServiceBreakdownIPv4iOS.clear();
    m_ServiceBreakdownIPv6OSX.clear();
    m_ServiceBreakdownIPv6iOS.clear();

    // Clear Socket Status
    for (int i = 0; i < NUM_SOCKET_STATUS; i++)
    {
        m_SocketStatus[i].Clear();
    }

    for (int i = 0; i < HOURS_IN_DAY; i++)
    {
        for (int j = 0; j < MINUTES_IN_HOUR; j++)
        {
            m_MinSnapshot[i][j].Clear();
        }
    }

    CDeviceNode* pDeviceNode = m_DeviceMap.GetRoot();
    if (pDeviceNode)
    {
        pDeviceNode->CallBack(&CbClearDataDevice,NULL);
    }

}

void CBonjourTop::SetIPAddr(const char* pStr)
{
    m_IPv4Addr.Set(pStr);

}

void CBonjourTop::UpdateRecord(CStringTree&  Cache,CDNSRecord* pDNSRecord,BJString& RecordName,BJString& ServiceName,BJ_UINT32 nBytes,bool bGoodbye)
{
    BJ_UINT64 nHashValue = 0;
    char deviceOS = '?';

    nHashValue = Hash(RecordName.GetBuffer());
    CStringNode* pRecord = Cache.Find(&nHashValue);
    if (pRecord == NULL)
    {
        pRecord = (CStringNode*) Cache.FindwithAddRecord(&nHashValue);
        strlcpy(pRecord->m_Value, RecordName.GetBuffer(), sizeof(pRecord->m_Value));
    }

    if (pRecord == NULL)
        return;
    CDeviceNode dummyDevice;
    CDeviceNode *device;
    CIPDeviceNode *pipNode = m_IPtoNameMap.Find(&m_Frame.m_SourceIPAddress);

    device = (pipNode)? pipNode->pDeviceNode : &dummyDevice;
    pRecord->m_nBytes += 10 + nBytes;
    deviceOS = device->GetDeviceOS();
    device->frameTotal.Increment(m_nFrameCount);

    if (pRecord->m_nLastFrameIndex != m_nFrameCount)
    {
        pRecord->m_nLastFrameIndex = m_nFrameCount;

        pRecord->m_nFrames++;
        if (deviceOS == 't' || deviceOS == 'i')
        {
            pRecord->m_nFramesiOS++;
        }
        else if (deviceOS == 'X')
        {
            pRecord->m_nFramesOSX++;
        }
    }

    // Update Total Device Count
    if (pRecord->m_DeviceTotalTree.Find(&m_Frame.m_SourceIPAddress) == NULL)
    {
        pRecord->m_nDeviceTotalCount++;
        pRecord->m_DeviceTotalTree.FindwithAddRecord(&m_Frame.m_SourceIPAddress);
    }

    if (m_Frame.IsQueryFrame())
    {
        GetOSTypeFromQuery(pDNSRecord, ServiceName);
        device->questionFrame.Increment(m_nFrameCount);
        if (pRecord->m_nLastQueryFrameIndex != m_nFrameCount)
        {
            pRecord->m_nLastQueryFrameIndex = m_nFrameCount;

            pRecord->m_nQuestionFrames++;

            if (deviceOS == 't' || deviceOS == 'i')
            {
                pRecord->m_nQuestionFramesiOS++;
            }
            else if (deviceOS == 'X')
            {
                pRecord->m_nQuestionFramesOSX++;
            }

            if (pRecord->m_DeviceAskingTree.Find(&m_Frame.m_SourceIPAddress) == NULL)
            {
                pRecord->m_nDeviceAskingCount++;
                pRecord->m_DeviceAskingTree.FindwithAddRecord(&m_Frame.m_SourceIPAddress);
            }
        }
    }
    else
    {
        GetOSTypeFromRegistration(pDNSRecord,ServiceName);

        device->answerFrame.Increment(m_nFrameCount);
        if (pRecord->m_nLastRespondsFrameIndex != m_nFrameCount)
        {
            pRecord->m_nLastRespondsFrameIndex = m_nFrameCount;

            pRecord->m_nAnswerFrames++;
            if (deviceOS == 't' || deviceOS == 'i')
            {
                pRecord->m_nAnswerFramesiOS++;
            }
            else if (deviceOS == 'X')
            {
                pRecord->m_nAnswerFramesOSX++;
            }

            if (bGoodbye)
            {
                pRecord->m_nGoodbyeFrames++;
            }

            if (pRecord->m_DeviceAnsweringTree.Find(&m_Frame.m_SourceIPAddress) == NULL)
            {
                pRecord->m_nDeviceAnsweringCount++;
                pRecord->m_DeviceAnsweringTree.FindwithAddRecord(&m_Frame.m_SourceIPAddress);
            }
        }
    }

    if (m_Frame.IsWakeFrame())
    {
        if (pRecord->m_nLastWakeFrameIndex != m_nFrameCount)
        {
            pRecord->m_nLastWakeFrameIndex = m_nFrameCount;
            if (pRecord->m_lastQUFrameTime +1000000ll < m_Frame.GetTime() || pRecord->m_lastQUFrameTime == 0) // last qu frame has been over 1 sec
            {
                pRecord->m_nWakeFrames++;
                pRecord->m_lastQUFrameTime = m_Frame.GetTime();
                device->QUFrame.Increment(m_nFrameCount);
            }
            pRecord->m_lastQUFrameTime = m_Frame.GetTime();
        }
    }


}

void CBonjourTop::UpdateShortRecordHelper(BJ_UINT32 cacheType, BJ_UINT32 tracePlatform, BJ_UINT32 traceVersion, char deviceOS, CDNSRecord* pDNSRecord,BJString& RecordName,BJString& ServiceName,BJ_UINT32 nBytes,bool bGoodbye)
{
    bool isOSX = false;
    BJString versionNumber = "mDNSResponder-";
    const int version_max_length = 11; // largest number is 0xffffffff = 4294967295
    char versionChar[version_max_length];

    CStringShortTree *cache;
    map<BJString, CStringShortTree*>* myMap;

    if ((tracePlatform | 0x80) == tracePlatform)
    {
        versionNumber = "Discoveryd-";
    }

    snprintf(versionChar, sizeof(versionChar), "%u", 0); // Set versionChar to "0" by default
    if (tracePlatform == TRACE_PLATFORM_UNKNOWN) // Pre iOS 7 or Pre OSX 10.9
    {
        if (deviceOS == 'i' || deviceOS == 't') // Pre iOS 7
        {
            isOSX = false;
        }
        else if (deviceOS == 'X') // Pre OSX 10.9
        {
            isOSX = true;
        }
    }
    else if ((tracePlatform == TRACE_PLATFORM_OSX) || (tracePlatform == DISCOVERYD_TRACE_PLATFORM_OSX)) // >= OSX 10.9
    {
        isOSX = true;
        snprintf(versionChar, sizeof(versionChar), "%u", traceVersion);
    }
    else if ((tracePlatform == TRACE_PLATFORM_iOS) || (tracePlatform == DISCOVERYD_TRACE_PLATFORM_iOS)) // >= iOS 7.x
    {
        isOSX = false;
        snprintf(versionChar, sizeof(versionChar), "%u", traceVersion);
    }
    else if ((tracePlatform == TRACE_PLATFORM_APPLE_TV) || (tracePlatform == DISCOVERYD_TRACE_PLATFORM_APPLE_TV))
    {
        snprintf(versionChar, sizeof(versionChar), "%u", traceVersion);
    }

    versionNumber += (const char*)versionChar;

    switch (cacheType) {
        case SERVICE_IPV4:
            if (isOSX)
            {
                myMap = &m_ServiceBreakdownIPv4OSX;
            }
            else
            {
                myMap = &m_ServiceBreakdownIPv4iOS;
            }
            break;
        case SERVICE_IPV6:
            if (isOSX)
            {
                myMap = &m_ServiceBreakdownIPv6OSX;
            }
            else
            {
                myMap = &m_ServiceBreakdownIPv6iOS;
            }
            break;
        case APP_IPV4:
            if (isOSX)
            {
                myMap = &m_AppBreakdownIPv4OSX;
            }
            else
            {
                myMap = &m_AppBreakdownIPv4iOS;
            }
            break;
        case APP_IPV6:
            if (isOSX)
            {
                myMap = &m_AppBreakdownIPv6OSX;
            }
            else
            {
                myMap = &m_AppBreakdownIPv6iOS;
            }
            break;

        default:
            return;
            break;
    }

    if (myMap->find(versionNumber) == myMap->end()) // Version number not found. Create new record
    {
        myMap->insert(std::pair<BJString, CStringShortTree*>(versionNumber, new CStringShortTree()));
    }
    cache = (*myMap)[versionNumber];
    UpdateShortRecord(cache, pDNSRecord, RecordName, ServiceName, nBytes, bGoodbye);
}

void CBonjourTop::UpdateShortRecord(CStringShortTree*  Cache,CDNSRecord* pDNSRecord,BJString& RecordName,BJString& ServiceName,BJ_UINT32 nBytes,bool bGoodbye)
{
    if (Cache == NULL)
    {
        return;
    }

    BJ_UINT64 nHashValue = 0;

    nHashValue = Hash(RecordName.GetBuffer());
    CStringShortNode* pRecord = Cache->Find(&nHashValue);
    if (pRecord == NULL)
    {
        pRecord = (CStringShortNode*) Cache->FindwithAddRecord(&nHashValue);
        if (pRecord)
            strlcpy(pRecord->m_Value, RecordName.GetBuffer(), sizeof(pRecord->m_Value));
    }

    if (pRecord == NULL)
    {
        return;
    }

    CDeviceNode dummyDevice;
    CDeviceNode *device;
    CIPDeviceNode *pipNode = m_IPtoNameMap.Find(&m_Frame.m_SourceIPAddress);

    device = (pipNode)? pipNode->pDeviceNode : &dummyDevice;
    pRecord->m_nBytes += 10 + nBytes;
    device->frameTotal.Increment(m_nFrameCount);

    if (pRecord->m_nLastFrameIndex != m_nFrameCount)
    {
        pRecord->m_nLastFrameIndex = m_nFrameCount;
        pRecord->m_nFrames++;
    }

    // Update Total Device Count
    if (pRecord->m_DeviceTotalTree.Find(&m_Frame.m_SourceIPAddress) == NULL)
    {
        pRecord->m_nDeviceTotalCount++;
        pRecord->m_DeviceTotalTree.FindwithAddRecord(&m_Frame.m_SourceIPAddress);
    }

    if (m_Frame.IsQueryFrame())
    {
        GetOSTypeFromQuery(pDNSRecord, ServiceName);
        device->questionFrame.Increment(m_nFrameCount);
        if (pRecord->m_nLastQueryFrameIndex != m_nFrameCount)
        {
            pRecord->m_nLastQueryFrameIndex = m_nFrameCount;

            pRecord->m_nQuestionFrames++;

            if (pRecord->m_DeviceAskingTree.Find(&m_Frame.m_SourceIPAddress) == NULL)
            {
                pRecord->m_nDeviceAskingCount++;
                pRecord->m_DeviceAskingTree.FindwithAddRecord(&m_Frame.m_SourceIPAddress);
            }

        }
    }
    else
    {
        GetOSTypeFromRegistration(pDNSRecord,ServiceName);

        device->answerFrame.Increment(m_nFrameCount);
        if (pRecord->m_nLastRespondsFrameIndex != m_nFrameCount)
        {
            pRecord->m_nLastRespondsFrameIndex = m_nFrameCount;

            pRecord->m_nAnswerFrames++;

            if (bGoodbye)
            {
                pRecord->m_nGoodbyeFrames++;
            }

            if (pRecord->m_DeviceAnsweringTree.Find(&m_Frame.m_SourceIPAddress) == NULL)
            {
                pRecord->m_nDeviceAnsweringCount++;
                pRecord->m_DeviceAnsweringTree.FindwithAddRecord(&m_Frame.m_SourceIPAddress);
            }
        }
    }

    if (m_Frame.IsWakeFrame())
    {
        if (pRecord->m_nLastWakeFrameIndex != m_nFrameCount)
        {
            pRecord->m_nLastWakeFrameIndex = m_nFrameCount;
            if (pRecord->m_lastQUFrameTime +1000000ll < m_Frame.GetTime() || pRecord->m_lastQUFrameTime == 0) // last qu frame has been over 1 sec
            {
                pRecord->m_nWakeFrames++;
                pRecord->m_lastQUFrameTime = m_Frame.GetTime();
                device->QUFrame.Increment(m_nFrameCount);
            }
            pRecord->m_lastQUFrameTime = m_Frame.GetTime();
        }
    }


}

void CBonjourTop::GetOSTypeFromQuery(CDNSRecord *pDNSRecord,BJString& ServiceName)
{
    if (pDNSRecord->m_RecType == DNS_TYPE_PTR)
    {
        StringMapNode* pStringNode = m_Service2osBrowseMap.Find(&ServiceName);
        if (pStringNode && *pStringNode->value.GetBuffer() != '?')
        {
            CIPDeviceNode *ipNode = m_IPtoNameMap.FindwithAddRecord(&m_Frame.m_SourceIPAddress);
            if (ipNode->pDeviceNode)
            {
                StringMapNode* pStringNode_temp = m_Service2osBrowseMap.Find(&ServiceName);
                ipNode->pDeviceNode->SetDeviceOS(*pStringNode_temp->value.GetBuffer(),ServiceName.GetBuffer());
            }
        }
    }
}

void CBonjourTop::GetOSTypeFromRegistration(CDNSRecord *pDNSRecord,BJString& ServiceName)
{
    CDeviceNode* deviceNode = NULL;
    BJString sDeviceName;

    if (pDNSRecord->m_RecType == DNS_TYPE_PTR)
    {
        BJString sInstanceName;
        pDNSRecord->GetRdata(sInstanceName,0,99);
        CDNSRecord* pSRVRecord = m_Frame.FindAdditionRecord(sInstanceName, DNS_TYPE_SRV);
        if (pSRVRecord)
        {
            pSRVRecord->GetRdata(sDeviceName,0,1);
        }
        else
        {
            sDeviceName = sInstanceName;
        }
        deviceNode = m_DeviceMap.Find(&sDeviceName);
    }

    // Name guess
     if (Name2OSType(sDeviceName,deviceNode))
         return;

    StringMapNode* pStringNode = m_Service2osRegisterMap.Find(&ServiceName);

    if (pStringNode == NULL || *pStringNode->value.GetBuffer() == '?')
        return;



    if (sDeviceName.GetLength() > 0)
    {
        // update global device table with os type
        if (deviceNode)
        {
            deviceNode->SetDeviceOS(*pStringNode->value.GetBuffer(),ServiceName.GetBuffer());
        }
    }
}
bool CBonjourTop::Name2OSType(BJString name,CDeviceNode* device)
{
    if (device == NULL)
        return false;
    // try to set device type from common names
    for (int i=0; Name2DeviceOS[i][0] != 0; i +=2)
    {
        if (name.Contains(Name2DeviceOS[i]))
        {
            device->SetDeviceOS(Name2DeviceOS[i+1][0], "Name Mapping");
            return true;
        }
    }
    return false;
}
void CBonjourTop::ProcessFrame(BJ_UINT8* pBuffer,BJ_INT32 nLength,BJ_UINT64 nFrameTime)
{

    m_Frame.ParseDNSFrame(pBuffer, nLength, nFrameTime);

    if (m_Collection.IsValid())
    {
        // setup static collectby
        CollectByPacketCount::nFrameIndex = m_nFrameCount;
        CollectBySameSubnetDiffSubnet::bSameSubnet = m_Frame.m_SourceIPAddress.IsIPv6()? true: m_IPv4Addr.IsSameSubNet(&m_Frame.m_SourceIPAddress);
        m_Collection.ProcessFrame(&m_Frame);
        return;
    }

    if (m_Frame.IsTruncatedFrame())
    {
        if (m_Frame.GetAnswerCount() > 0)
        {
            if (m_AvgAnswerCountForTruncatedFrames)
            {
                m_AvgAnswerCountForTruncatedFrames += m_Frame.GetAnswerCount();
                m_AvgAnswerCountForTruncatedFrames /=2;
            }
            else
                m_AvgAnswerCountForTruncatedFrames += m_Frame.GetAnswerCount();

            if (m_MinAnswerCountForTruncatedFrames > m_Frame.GetAnswerCount() || m_MinAnswerCountForTruncatedFrames == 0)
                m_MinAnswerCountForTruncatedFrames = m_Frame.GetAnswerCount();
            if (m_MaxAnswerCountForTruncatedFrames < m_Frame.GetAnswerCount())
                m_MaxAnswerCountForTruncatedFrames = m_Frame.GetAnswerCount();

        }
    }

    // find min snapshot bucket
    time_t now = time(NULL);
    struct tm* timeStruct = localtime(&now);
    if (m_MinSnapshot[timeStruct->tm_hour][timeStruct->tm_min].m_SampleDay != timeStruct->tm_mday)
    {
        //Reset Snapshot 24 hour wrap around
        m_MinSnapshot[timeStruct->tm_hour][timeStruct->tm_min].Init();
        m_MinSnapshot[timeStruct->tm_hour][timeStruct->tm_min].m_SampleDay = timeStruct->tm_mday;

    }
    m_MinSnapshot[timeStruct->tm_hour][timeStruct->tm_min].m_nFrameCount++;

    if (m_Frame.GetQuestionCount() == 0 && m_Frame.GetAnswerCount() > 0)
        m_SocketStatus[0].m_nAnswerOnlyFrames++;
    else if (m_Frame.GetQuestionCount() > 0 && m_Frame.GetAnswerCount() == 0)
        m_SocketStatus[0].m_nQuestionOnlyFrames++;
    else
        m_SocketStatus[0].m_nQandAFrames++;

    BJString InstanceName;
    BJString RecordName;
    BJString ApplRecordName;

    /// first get the name to address
    for (int dnsItemsIndex =m_Frame.GetQuestionCount(); dnsItemsIndex < m_Frame.GetMaxRecords();dnsItemsIndex++)
    {
        CDNSRecord* pDNSRecord = m_Frame.GetDnsRecord(dnsItemsIndex);
        if (pDNSRecord == NULL)
            continue;

        if (pDNSRecord->m_RecType == DNS_TYPE_A)
        {
            BJString sName;
            pDNSRecord->GetDnsRecordName(sName, 0, 1);
            BJIPAddr ip;
            ip.Setv4Raw(pDNSRecord->GetStartofRdata());

            CDeviceNode* device = m_DeviceMap.FindwithAddRecord(&sName);
            device->ipAddressv4 = ip;

            // create ip to name mapping
            CIPDeviceNode* pipNode = m_IPtoNameMap.FindwithAddRecord(&ip);

            if (pipNode->pDeviceNode && pipNode->pDeviceNode->bIPName &&  pipNode->pDeviceNode->m_Key != device->m_Key)
            {
                pipNode->pDeviceNode->bDuplicate = true;
                device->MergeData(pipNode->pDeviceNode);
                pipNode->pDeviceNode->ClearData();
                // remap IPv6
                if (!pipNode->pDeviceNode->ipAddressv6.IsEmpty())
                {
                    CIPDeviceNode* ipv6Node = m_IPtoNameMap.Find(&pipNode->pDeviceNode->ipAddressv6);
                    if (ipv6Node)
                        ipv6Node->pDeviceNode = device;
                }
            }

            pipNode->pDeviceNode = device;
            Name2OSType(sName,device);
        }

        if (pDNSRecord->m_RecType == DNS_TYPE_AAAA)
        {
            BJString sName;
            pDNSRecord->GetDnsRecordName(sName, 0, 1);
            BJIPAddr ip;
            ip.Setv6Raw(pDNSRecord->GetStartofRdata());

            if (ip.IsIPv6LinkLocal())
            {
                CDeviceNode* device = m_DeviceMap.FindwithAddRecord(&sName);
                device->ipAddressv6 = ip;

                // create ip to name mapping
                CIPDeviceNode* pipNode = m_IPtoNameMap.FindwithAddRecord(&ip);

                if (pipNode->pDeviceNode && pipNode->pDeviceNode->bIPName && pipNode->pDeviceNode->m_Key != device->m_Key)
                {
                    pipNode->pDeviceNode->bDuplicate = true;
                    device->MergeData(pipNode->pDeviceNode);
                    pipNode->pDeviceNode->ClearData();
                    // remap IPv4
                    if (!pipNode->pDeviceNode->ipAddressv4.IsEmpty())
                    {
                        CIPDeviceNode* ipv4Node = m_IPtoNameMap.Find(&pipNode->pDeviceNode->ipAddressv4);
                        if (ipv4Node)
                            ipv4Node->pDeviceNode = device;
                    }
                }

                pipNode->pDeviceNode = device;
                Name2OSType(sName,device);
            }
        }
        if (pDNSRecord->m_RecType == DNS_TYPE_SRV)
        {   // Save SVR to Target
            BJString sName;
            pDNSRecord->GetDnsRecordName(sName, 0, 1);
            StringMapNode *node = SVRtoDeviceName.FindwithAddRecord(&sName);
            pDNSRecord->GetRdata(node->value, 0, 1);
        }
    }

    CIPDeviceNode* pipNode = m_IPtoNameMap.FindwithAddRecord(&m_Frame.m_SourceIPAddress);
    CDeviceNode* device = pipNode->pDeviceNode;
    if (device == NULL)
    {
        // find the device by mac address
        CMACAddrDeviceNode *macDevice = m_MACtoDevice.FindwithAddRecord(&m_Frame.m_SourceMACAddress);
        device = macDevice->device;
        if (device == NULL)
        {
            // auto create a device record
            BJString name = m_Frame.m_SourceIPAddress.GetString();
            device = m_DeviceMap.FindwithAddRecord(&name);
            device->bIPName = true;
            macDevice->device = device;
        }

        if (m_Frame.m_SourceIPAddress.IsIPv4())
            device->ipAddressv4 = m_Frame.m_SourceIPAddress;
        else
            device->ipAddressv6 = m_Frame.m_SourceIPAddress;
        if (device->macAddress.IsEmpty())
            device->macAddress = m_Frame.m_SourceMACAddress;

        pipNode->pDeviceNode = device;
    }
    device->bHasFrames = true;
    // update mac address
    if (m_Frame.IsQueryFrame() ||  device->GetDeviceOS() == 'i' ) // iOS don't use BSP so we can use SourceIP
    {
        if (m_Frame.m_SourceIPAddress.IsIPv4())
            device->ipAddressv4 = m_Frame.m_SourceIPAddress;
        if (m_Frame.m_SourceIPAddress.IsIPv6())
            device->ipAddressv6 =m_Frame.m_SourceIPAddress;
        device->macAddress = m_Frame.m_SourceMACAddress;
    }

    BJ_UINT8 traceplatform = TRACE_PLATFORM_UNKNOWN;
    BJ_UINT32 traceversion = 0;
    BJMACAddr traceMac;
    if (device /*&& device->GetDeviceOS() == '?' */&& m_Frame.GetTracingInfo(traceplatform, traceversion, traceMac))
    {
   //     printf("Tracing Data found platform=%d traceversion=%d\n",traceplatform,traceversion);
        char platformMap[]= "?Xitw";
        device->SetDeviceOS((traceplatform < 5) ? platformMap[traceplatform] : '?', "EDNS0 Trace");
        if ((traceplatform == TRACE_PLATFORM_OSX) || (traceplatform == DISCOVERYD_TRACE_PLATFORM_OSX))
        {
             device->bOSXWithEDNSField = true;
        }
        else if ((traceplatform == TRACE_PLATFORM_iOS) || (traceplatform == DISCOVERYD_TRACE_PLATFORM_iOS))
        {
            device->biOSWithEDNSField = true;
        }
    }

    for (int dnsItemsIndex =0; dnsItemsIndex < m_Frame.GetQuestionCount()+m_Frame.GetAnswerCount();dnsItemsIndex++)
    {
        RecordName = "";
        ApplRecordName = "";
        InstanceName = "";
        //    printf("Name = %s\n", GetDnsRecordName(&Frame,dnsItemsIndex,tempBuffer,sizeof(tempBuffer),0));

        CDNSRecord* pDNSRecord = m_Frame.GetDnsRecord(dnsItemsIndex);
        if (pDNSRecord == NULL)
            continue;

        pDNSRecord->GetDnsRecordName(RecordName,0,99);
        InstanceName = RecordName;

        if (RecordName.Contains("_kerberos."))
        {
            RecordName = "_kerberos.";
        }
        else
            pDNSRecord->GetDnsRecordName(RecordName, (pDNSRecord->m_RecType == DNS_TYPE_PTR)?0:1,99);

        if (pDNSRecord->m_RecType == DNS_TYPE_PTR)
        {
            if (RecordName.Contains(".ip6.arpa."))
                RecordName = "*.ip6.arpa.";
            else if (RecordName.Contains(".arpa."))
                RecordName = "*.arpa.";
        }
        if (pDNSRecord->m_RecType == DNS_TYPE_A)
            RecordName = "A";
        if (pDNSRecord->m_RecType == DNS_TYPE_AAAA)
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

        BJ_UINT32 nBytes =  pDNSRecord->m_nNameLength + pDNSRecord->m_nRdataLen;

        m_nTotalBytes += 10 + nBytes;

        if (m_Frame.m_SourceIPAddress.IsIPv4())
        {
            UpdateRecord(m_ServicePtrCache,pDNSRecord,RecordName,RecordName, nBytes, (pDNSRecord->m_nTTL == 0));
            UpdateShortRecordHelper(SERVICE_IPV4, traceplatform, traceversion, device->GetDeviceOS(), pDNSRecord, RecordName, RecordName, nBytes, (pDNSRecord->m_nTTL == 0));
        }
        else
        {
            UpdateRecord(m_ServicePtrCacheIPv6,pDNSRecord,RecordName,RecordName, nBytes, (pDNSRecord->m_nTTL == 0));
            UpdateShortRecordHelper(SERVICE_IPV6, traceplatform, traceversion, device->GetDeviceOS(), pDNSRecord, RecordName, RecordName, nBytes, (pDNSRecord->m_nTTL == 0));
        }

        // Add application to cache
        if (RecordName.GetLength() != 0)
        {

            ApplRecordName = RecordName;
            StringMapNode* pNode;
            pNode = m_Service2AppMap.Find(&ApplRecordName);
            if (pNode && ApplRecordName.GetBuffer() != NULL)
            {
                ApplRecordName = pNode->value.GetBuffer();
                if ( ApplRecordName == "Device-Info")
                {
                    // find device record
                    BJString svrName;
                    pDNSRecord->GetDnsRecordName(svrName, 0, 1);
                    StringMapNode *nodeName = SVRtoDeviceName.Find(&svrName);

                    CDeviceNode* pDeviceNode = nodeName ? m_DeviceMap.Find(&nodeName->value) : NULL;

                    if (pDeviceNode)
                    {
                        BJString DeviceInfo;
                        DeviceInfo.Set((char*)pDNSRecord->GetStartofRdata(),MIN(pDNSRecord->m_nRdataLen,50));
                        char osType = '?';

                        for (int i=0; DeviceInfo2DeviceOS[i][0] != 0; i+=2)
                        {
                            if (DeviceInfo.Contains(DeviceInfo2DeviceOS[i]))
                            {
                                osType = *DeviceInfo2DeviceOS[i+1];
                                if (pDeviceNode->GetDeviceOS() != *DeviceInfo2DeviceOS[i])
                                {
                                    pDeviceNode->SetDeviceOS(osType,"_device-info._tcp.local.");
                                    pDeviceNode->SetModel(DeviceInfo2DeviceOS[i]);
                                }
                                break;
                            }
                        }


                        if (osType == 'X' ||  (pDeviceNode && pDeviceNode->GetDeviceOS() == 'X'))
                            ApplRecordName = "Finder";
                        if (osType == 't' ||  (pDeviceNode && pDeviceNode->GetDeviceOS() == 't'))
                            ApplRecordName = "AirPlay";

                        if (pDeviceNode && pDeviceNode->GetDeviceOS() == '?' && pDNSRecord->m_nRdataLen > 0)
                        {
                            BJString DeviceInfo_temp;
                            DeviceInfo_temp.Set((char*)pDNSRecord->GetStartofRdata(),MIN(pDNSRecord->m_nRdataLen,25));
                        }
                    }
                }
            }
            else
            {
                ApplRecordName = "Other";
            }

            if (m_Frame.m_SourceIPAddress.IsIPv4())
            {
                UpdateRecord(m_ApplPtrCache,pDNSRecord,ApplRecordName,RecordName, nBytes, (pDNSRecord->m_nTTL == 0));
                UpdateShortRecordHelper(APP_IPV4, traceplatform, traceversion, device->GetDeviceOS(), pDNSRecord, ApplRecordName, RecordName, nBytes, (pDNSRecord->m_nTTL == 0));

            }
            else
            {
                UpdateRecord(m_ApplPtrCacheIPv6,pDNSRecord,ApplRecordName,RecordName, nBytes, (pDNSRecord->m_nTTL == 0));
                UpdateShortRecordHelper(APP_IPV6, traceplatform, traceversion, device->GetDeviceOS(), pDNSRecord, ApplRecordName, RecordName, nBytes, (pDNSRecord->m_nTTL == 0));
            }

        }

    }
}


class CSortOptions
{

public:
    CStringTree m_SortedCache;
    int m_nSortCol;

};
static int CbPrintResults(const void* pNode, const void* pParam)
{
    CStringNode* pRecord = (CStringNode*)pNode;
    CSortOptions* pSortedOption = (CSortOptions*)pParam;

    CStringNode* pNewRecord;

    BJ_UINT64 SortKey =0;
    switch (pSortedOption->m_nSortCol) {
        case 0:
            SortKey = Hash2(pRecord->m_Value); // cheat the sort to the first 8 char
            break;
        case 1:
            SortKey = pRecord->m_nBytes;
            break;
        case 2:
            SortKey = pRecord->m_nFrames;
            break;
        case 3:
            SortKey = pRecord->m_nQuestionFrames;
            break;
        case 4:
            SortKey = pRecord->m_nAnswerFrames;
            break;
        default:
            break;
    }


    pNewRecord = pSortedOption->m_SortedCache.AddRecord(&SortKey);

    if (pNewRecord)
    {
        pNewRecord->CopyNode(pRecord);

    }

    return 0;
}

#if 0 // Not used 
static int CbPrintUnknownDevice(const void* pNode, const void*)
{
    CDeviceNode* pDeviceRecord = (CDeviceNode*)pNode;

    if (pDeviceRecord->GetDeviceOS() != '?')
        return 0;

  //  printf("%s %s\n",pDeviceRecord->m_Key.GetBuffer(), pDeviceRecord->macAddress.GetString());
    return 0;

}
#endif

static int CbBuildMacMap(const void* pNode, const void* pParam)
{
    CDeviceNode* pDeviceNode = (CDeviceNode*)pNode;
    CMACAddrTree* pMacMap = (CMACAddrTree*)pParam;


    BJMACAddr vendorMac;
    vendorMac.CopyVendor(pDeviceNode->macAddress);
    if (vendorMac.IsEmpty())
        return 0;



    if (pDeviceNode->GetDeviceOS() == '?')
    {
        // try to set device type from MAC address
        CMACAddrNode* pMacRecord = pMacMap->Find(&vendorMac);
        if (pMacRecord != NULL && pDeviceNode->GetDeviceOS() == '?')
        {
            pDeviceNode->SetDeviceOS(pMacRecord->deviceOS, "MAC Mapping");
//            printf("update device %s %c\n",vendorMac.GetStringVendor(),pMacRecord->deviceOS);
        }

        if (pDeviceNode->GetDeviceOS() == '?')
            return 0;
    }


    CMACAddrNode* pMacRecord = pMacMap->Find(&vendorMac);
    if (pMacRecord == NULL)
    {
        pMacRecord = pMacMap->FindwithAddRecord(&vendorMac);
        pMacRecord->deviceOS = pDeviceNode->GetDeviceOS();
        pMacRecord->model = pDeviceNode->model;
        pMacRecord->method = pDeviceNode->settingService;
    }
    else
    {
        // Check mapping
  ///      if (pMacRecord && pMacRecord->deviceOS != pDeviceNode->GetDeviceOS())
   //        printf("Mac Mapping Bad deviceOS %c != %c %s %s %s\n", pMacRecord->deviceOS, pDeviceNode->GetDeviceOS(),pMacRecord->method.GetBuffer(), pDeviceNode->settingService.GetBuffer(),vendorMac.GetStringVendor());
    //    if (pMacRecord && !(pMacRecord->model == pDeviceNode->model) && pMacRecord->model.GetLength() > 0 && pDeviceNode->model.GetLength() > 0)
    //        printf("Mac Mapping Bad model %s != %s\n", pMacRecord->model.GetBuffer(), pDeviceNode->model.GetBuffer());

    }
    return 0;

}

CStringNode* CBonjourTop::GetCurrentDisplayRoot(BJString &sTitle)
{
    CStringNode* pRecord = NULL;

    switch(m_CurrentDisplay)
    {
        case BJ_DISPLAY_APP:
            pRecord = m_ApplPtrCache.GetRoot();
            sTitle = "Application (IPv4)";
            break;
        case BJ_DISPLAY_APPv6:
            pRecord = m_ApplPtrCacheIPv6.GetRoot();
            sTitle = "Application (IPv6)";
            break;
        case BJ_DISPLAY_SERVICE:
            pRecord = m_ServicePtrCache.GetRoot();
            sTitle = "Services (IPv4)";
            break;
        case BJ_DISPLAY_SERVICEv6:
            pRecord = m_ServicePtrCacheIPv6.GetRoot();
            sTitle = "Services (IPv6)";
            break;
        case BJ_DISPLAY_24_MIN:
            printf("Error");
            break;
    }
    return pRecord;
}
void CBonjourTop::UpdateOSCounts()
{


    CDeviceNode* pDeviceNode = m_DeviceMap.GetRoot();
    if (pDeviceNode)
    {
        pDeviceNode->CallBack(&CbBuildMacMap,&m_MacMap);
        //        pDeviceNode->CallBack(&CbPrintUnknownDevice,NULL);
    }


    // Update Application Caches
    CStringNode* pCacheRoot = m_ApplPtrCache.GetRoot();

    if (pCacheRoot)
    {
        pCacheRoot->UpdateOSTypeCounts(&m_DeviceMap,&m_IPtoNameMap);
    }
    pCacheRoot = m_ApplPtrCacheIPv6.GetRoot();

    if (pCacheRoot)
    {
        pCacheRoot->UpdateOSTypeCounts(&m_DeviceMap,&m_IPtoNameMap);
    }

    // Update Service caches
    pCacheRoot = m_ServicePtrCache.GetRoot();

    if (pCacheRoot)
    {
        pCacheRoot->UpdateOSTypeCounts(&m_DeviceMap,&m_IPtoNameMap);
    }
    pCacheRoot = m_ServicePtrCacheIPv6.GetRoot();

    if (pCacheRoot)
    {
        pCacheRoot->UpdateOSTypeCounts(&m_DeviceMap,&m_IPtoNameMap);
    }


}
void CBonjourTop::PrintResults(int nSortCol, bool bSortAsc)
{

    BJString sTitle;
    GetCurrentDisplayRoot(sTitle);
    device_count devCount;
    BJString sTableTitle;

    UpdateOSCounts();

    /////
    BJ_UINT64 nRate = 0;
    BJ_UINT64 nElapsedTime = m_EndTime-m_StartTime;
    if (nElapsedTime > 0)
    {
            nRate = (m_nFrameCount *3600) /nElapsedTime;
    }
    if (m_bCursers)
    {
        resizeterm(0,0);
        clear();

        printw("While running the follow keys may be used:\n");
        printw("[p = sort by Packets (default)], [b = sort by Bytes], [n = sort by Name]\n");
        printw("[a = Display Application Names (default)], [s = Display Services Names], [t = Display 24 hour packet per min]\n");
        printw("[o = flip sort order], [e = export to BonjourTop.csv], [q = quit]\n\n");

        printw("Total Packets: %llu, Total Bytes: %llu, Elapse Time: %lld sec, Rate: %llu packet/hr\n",m_nFrameCount,m_nTotalBytes,nElapsedTime,nRate);
        printw("IPv4 multicast: %llu, IPv6 multicast: %llu, IPv4 Unicast: %lld, IPv6 Unicast: %llu\n",m_SocketStatus[0].m_nFrameCount,m_SocketStatus[1].m_nFrameCount,m_SocketStatus[2].m_nFrameCount,m_SocketStatus[3].m_nFrameCount);
        printw("IPv4 Wrong subnet: %llu, IPv6 Wrong subnet: %llu\n",m_SocketStatus[4].m_nFrameCount,m_SocketStatus[5].m_nFrameCount);
        printw("QuestionOnly Packets: %llu, AnswerOnly Packets: %llu, Q&A Packets %llu\n",m_SocketStatus[0].m_nQuestionOnlyFrames,m_SocketStatus[0].m_nAnswerOnlyFrames,m_SocketStatus[0].m_nQandAFrames);
        printw("AnswerCount for truncated frames(min,avg,max): %llu,%llu,%llu\n\n",m_MinAnswerCountForTruncatedFrames,m_AvgAnswerCountForTruncatedFrames,m_MaxAnswerCountForTruncatedFrames);

        bzero(&devCount, sizeof(devCount));
        m_DeviceMap.GetDeviceOSTypes(m_DeviceMap.GetRoot(),NULL, devCount);
        printw("Total Devices: %llu, iOS Devices: %llu(>= iOS7: %llu), OSX Devices %llu(>= OSX 10.9: %llu)\n",devCount.iOS+devCount.OSX+devCount.unknownOS,devCount.iOS, devCount.iOSWithEDNSField, devCount.OSX,devCount.OSXWithEDNSField);
    }
    else
    {
        printf("\nTotal Packets: %llu, Total Bytes: %llu, Elapse Time: %lld sec, Rate: %llu packet/hr\n",m_nFrameCount,m_nTotalBytes,nElapsedTime,nRate);
        printf("IPv4 multicast: %llu, IPv6 multicast: %llu, IPv4 Unicast: %lld, IPv6 Unicast: %llu\n",m_SocketStatus[0].m_nFrameCount,m_SocketStatus[1].m_nFrameCount,m_SocketStatus[2].m_nFrameCount,m_SocketStatus[3].m_nFrameCount);
        printf("IPv4 Wrong subnet: %llu, IPv6 Wrong subnet: %llu\n",m_SocketStatus[4].m_nFrameCount,m_SocketStatus[5].m_nFrameCount);
        printf("QuestionOnly Packets: %llu, AnswerOnly Packets: %llu, Q&A Packets %llu\n",m_SocketStatus[0].m_nQuestionOnlyFrames,m_SocketStatus[0].m_nAnswerOnlyFrames,m_SocketStatus[0].m_nQandAFrames);

        bzero(&devCount, sizeof(devCount));
        m_DeviceMap.GetDeviceOSTypes(m_DeviceMap.GetRoot(),NULL, devCount);

        printf("Total Devices: %llu, iOS Devices: %llu(>= iOS7: %llu), OSX Devices %llu(>= OSX 10.9: %llu), unknown Devices %llu\n",devCount.iOS+devCount.OSX+devCount.unknownOS,devCount.iOS, devCount.iOSWithEDNSField,devCount.OSX,devCount.OSXWithEDNSField,devCount.unknownOS);
        printf("AnswerCount for truncated frames(min,avg,max): %llu,%llu,%llu\n\n",m_MinAnswerCountForTruncatedFrames,m_AvgAnswerCountForTruncatedFrames,m_MaxAnswerCountForTruncatedFrames);
    }
    PrintDetailResults(nSortCol, bSortAsc);
    if (m_bCursers)
        refresh();
}
void CBonjourTop::PrintDetailResults(int nSortCol, bool bSortAsc)
{
    BJString sTitle;
    CStringNode* pCacheRoot = GetCurrentDisplayRoot(sTitle);

    if (m_bCursers)
    {
        if(m_CurrentDisplay == CBonjourTop::BJ_DISPLAY_24_MIN)
        {
            printw("    ");
            for(int i=0;i<24;i++)
                printw("   %02d ",i);
            printw("\n");
        }
        else
        {
            printw("\n%s\n",sTitle.GetBuffer());
            printw("     %- 30s %10s %10s%24s%24s%24s%24s%24s\n","","","Total","Question","Answer","Asking","Answering", "Total");
            printw("     %- 30s %10s%24s%24s%24s %23s%24s%24s %11s %10s\n","Name","Bytes","Packets(  iOS/  OSX)","Packets(  iOS/  OSX)","Packets(  iOS/  OSX)","Devices(  iOS/  OSX)","Devices(  iOS/  OSX)","Devices(  iOS/  OSX)", "QU Bit","Goodbye");
        }
    }
    else
    {
        if(m_CurrentDisplay == CBonjourTop::BJ_DISPLAY_24_MIN)
        {

        }
        else
        {
            printf("\n%s\n",sTitle.GetBuffer());
            printf("     %-30s %10s %10s%24s%24s%24s%24s%24s\n","","","Total","Question","Answer","Asking","Answering", "Total");
            printf("     %-30s %10s%24s%24s%24s %23s%24s%24s %11s %10s\n","Name","Bytes","Packets(  iOS/  OSX)","Packets(  iOS/  OSX)","Packets(  iOS/  OSX)","Devices(  iOS/  OSX)","Devices(  iOS/  OSX)","Devices(  iOS/  OSX)", "QU Bit","Goodbye");
        }
    }
    if (m_CurrentDisplay == CBonjourTop::BJ_DISPLAY_24_MIN)
    {

        for(int m=0;m<60;m++)
        {
            printw(" %02d ",m);
            for (int h=0;h<24;h++)
                printw("%5d ",m_MinSnapshot[h][m].m_nFrameCount);
            printw("\n");
        }
    }
    else
    {
    // sort list
        CSortOptions SortOptions;
        SortOptions.m_nSortCol = nSortCol;

        if (pCacheRoot)
            pCacheRoot->UpdateOSTypeCounts(&m_DeviceMap,&m_IPtoNameMap);

        if (pCacheRoot)
            pCacheRoot->CallBack(&CbPrintResults,&SortOptions);

        // print list

        CStringNode* pRecord = SortOptions.m_SortedCache.GetRoot();
        BJ_UINT32 nIndex = 1;

        if (pRecord)
            pRecord->Print(m_bCursers,bSortAsc, nIndex,0,40);
    }


}

void CBonjourTop::LiveCapture()
{
    /// Live Capture
    const BJ_UINT16 BonjourPort = 5353;
    BJSocket Sockv4;
    BJSocket Sockv6;
    BJSelect SockSelect;

    Sockv4.CreateListenerIPv4(interfaceName);
    Sockv6.CreateListenerIPv6(interfaceName);

    SockSelect.Add(Sockv4);
    SockSelect.Add(Sockv6);


    m_StartTime = time(NULL);

    bool bSortAsc = false;
    int nSortCol = 1;

    while (1)
    {
        SockSelect.Add(Sockv4);
        SockSelect.Add(Sockv6);

        int result = SockSelect.Wait(1);
        if (result < 0)
        {
            // if SockSelect.Wait failed due to an interrupt, then we want to continue processing the packets
            if (errno == EINTR)
            {
                continue;
            }
            printf("Error in Select\n");
            break;
        }

        if (SockSelect.IsReady(Sockv4))
        {

            int recvsize = Sockv4.Read();

            if ((recvsize != 0) &&
                (Sockv4.GetSrcAddr()->GetPortNumber() == BonjourPort))
            {
                m_nFrameCount++;
                m_SocketStatus[Sockv4.IsMulticastPacket()? 0:2].m_nFrameCount++;

                if (!m_IPv4Addr.IsSameSubNet(Sockv4.GetSrcAddr()))
                {
                    m_SocketStatus[4].m_nFrameCount++;
                }
                m_Frame.m_SourceIPAddress = *Sockv4.GetSrcAddr();
                ProcessFrame(Sockv4.GetBuffer(),recvsize,Sockv4.m_CurrentFrame.GetTime());
            }
        }

        if (SockSelect.IsReady(Sockv6))
        {
            int recvsize = Sockv6.Read();
            if ((recvsize != 0) &&
                (Sockv6.GetSrcAddr()->GetPortNumber() == BonjourPort))
            {
                m_nFrameCount++;
                m_SocketStatus[Sockv6.IsMulticastPacket()? 1:3].m_nFrameCount++;
                m_Frame.m_SourceIPAddress = *Sockv6.GetSrcAddr();

                ProcessFrame(Sockv6.GetBuffer(),recvsize,Sockv6.m_CurrentFrame.GetTime());
            }
        }

        if (m_bCursers)
        {
            int ch = getch();
            switch (ch)
            {
                case 'o':
                    bSortAsc = !bSortAsc;
                    result = 0; // force an update
                    break;
                case 'n':
                    nSortCol = 0;
                    result = 0; // force an update
                    break;
                case 'b':
                    nSortCol = 1;
                    result = 0; // force an update
                    break;
                case 'p':
                    if (nSortCol == 2)
                        nSortCol = 3;
                    else if (nSortCol == 3)
                        nSortCol = 4;
                    else
                        nSortCol = 2;
                    result = 0; // force an update
                    break;
                case 'a':
                case 'A':
                    if (m_CurrentDisplay == CBonjourTop::BJ_DISPLAY_APP)
                        m_CurrentDisplay = CBonjourTop::BJ_DISPLAY_APPv6;
                    else
                        m_CurrentDisplay = CBonjourTop::BJ_DISPLAY_APP;

                    result = 0;
                    break;
                case 's':
                case 'S':
                    if (m_CurrentDisplay == CBonjourTop::BJ_DISPLAY_SERVICE)
                        m_CurrentDisplay = CBonjourTop::BJ_DISPLAY_SERVICEv6;
                    else
                        m_CurrentDisplay = CBonjourTop::BJ_DISPLAY_SERVICE;
                    result = 0;
                    break;
                case 't':
                case 'T':
                   m_CurrentDisplay = CBonjourTop::BJ_DISPLAY_24_MIN;
                    result = 0;
                    break;
                case 'q':
                    return;
                case 'e':
                    ExportResults();
                    printf("\a");
                    break;
                case KEY_RESIZE:
                    endwin();
                    initscr();
                    result = 0; // force an update
                    break;
                case KEY_DOWN:
                    result = 0; // force an update
                    break;
            }
            if (window_size_changed)
            {
                endwin();
                initscr();
                window_size_changed = false;
            }
        }
        if (m_EndTime != time(NULL) || result == 0)
        {
            m_EndTime = time(NULL);
            PrintResults(nSortCol,bSortAsc);
            if (m_SnapshotSeconds && (time(NULL) - m_StartTime) > m_SnapshotSeconds)
            {
                ExportResults();
                if (m_bImportExportDeviceMap)
                {
                    WriteDeviceFile();
                    WriteVendorFile();
                }
                Reset();
            }
        }
    }
}



void CBonjourTop::CaptureFile()
{
    CCaptureFile CaptureFile;
    BJIPAddr* pIPSrcAddr;
    BJIPAddr* pIPDestAddr;

    CIPAddrMap LocalSubnetIPv6;


    CaptureFile.Open(m_pTcpDumpFileName);

    m_StartTime = 0;
    int nFrameIndex  = 0;

    while (CaptureFile.NextFrame())
    {
        nFrameIndex++;

        BJ_UINT8* pBonjourBuffer = (BJ_UINT8*)CaptureFile.m_CurrentFrame.GetBonjourStart();
        if (!pBonjourBuffer)
            continue;

        m_nFrameCount++;
        m_nTotalBytes += CaptureFile.GetWiredLength();

        pIPSrcAddr = CaptureFile.m_CurrentFrame.GetSrcIPAddr();
        pIPDestAddr = CaptureFile.m_CurrentFrame.GetDestIPAddr();
        m_Frame.m_SourceIPAddress = *CaptureFile.m_CurrentFrame.GetSrcIPAddr();;
        m_Frame.m_SourceMACAddress = *CaptureFile.m_CurrentFrame.GetSrcMACAddr();

        if (pIPSrcAddr->IsIPv4())
        {
            // check fragment flag
            BJ_UINT8* pIP = CaptureFile.m_CurrentFrame.GetIPStart();
            BJ_UINT16 flags = * ((BJ_UINT16*)(pIP+6));
            if (flags)
                continue;

            if (!m_IPv4Addr.IsEmptySubnet())
            {
                if (m_IPv4Addr.IsSameSubNet(pIPSrcAddr))
                {
                    BJ_UINT8* pSourceMac = CaptureFile.m_CurrentFrame.GetEthernetStart()+6;
                    BJIPAddr IPv6Addr;
                    IPv6Addr.CreateLinkLocalIPv6(pSourceMac);
                    LocalSubnetIPv6.FindwithAddRecord(&IPv6Addr);

                }
                else
                {
                    m_SocketStatus[4].m_nFrameCount++;

                    if (!m_Collection.IsValid())
                        continue;
                }
            }
            m_SocketStatus[(pIPDestAddr->IsBonjourMulticast())?0:2].m_nFrameCount++;
        }
        if (pIPSrcAddr->IsIPv6())
        {
            if (!LocalSubnetIPv6.Find(pIPSrcAddr) && !m_IPv4Addr.IsEmptySubnet())
            {
                m_SocketStatus[5].m_nFrameCount++;
                 if (!m_Collection.IsValid())
                     continue;
            }
            m_SocketStatus[(pIPDestAddr->IsBonjourMulticast())?1:3].m_nFrameCount++;
        }

        ProcessFrame(pBonjourBuffer,CaptureFile.GetBufferLen((pBonjourBuffer)),CaptureFile.m_CurrentFrame.GetTime());

    }
    m_EndTime = CaptureFile.GetDeltaTime();

    PrintResults(2,false);
    if ( m_CurrentDisplay ==  BJ_DISPLAY_APP)
        m_CurrentDisplay =  BJ_DISPLAY_APPv6;
    else
        m_CurrentDisplay =  BJ_DISPLAY_SERVICEv6;

    PrintDetailResults(2,false);

}

void CBonjourTop::ExportPtrCache(FILE* hFile, BJString sTitle,CStringNode* pRoot)
{
    fprintf(hFile,"%s\n",sTitle.GetBuffer());
    fprintf(hFile,"Name,Bytes,Total Packets,Total Packets iOS,Total Packets OSX,Question Packets,Question Packets iOS,Question Packets OSX,Answer Packets,Answer Packets iOS,Answer Packets OSX,Asking Devices, Asking Devices iOS,Asking Devices OSX,Answering Devices,Answering Devices iOS,Answering Devices OSX,Total Devices,Total Devices iOS, Total Devices OSX,QU Bit,Goodbye\n");

    if (pRoot)
        pRoot->Export(hFile);
}

void CBonjourTop::ExportShortCacheHelper(FILE* hFile, BJString sTitle, CStringShortNode* pRoot)
{
    fprintf(hFile,"%s\n",sTitle.GetBuffer());
    fprintf(hFile,"Name,Bytes,Total Packets,Question Packets,Answer Packets,Asking Devices,Answering Devices,Total Devices,QU Bit,Goodbye\n");

    if (pRoot)
    {
        pRoot->Export(hFile);
    }

}

void CBonjourTop::ExportShortCache(FILE* hFile, BJString sTitle, map<BJString, CStringShortTree*>* myMap)
{
    CStringShortTree* cache;
    BJString versionNumber;

    fprintf(hFile,"%s\n",sTitle.GetBuffer());

    for (map<BJString, CStringShortTree*>::iterator it = myMap->begin(); it != myMap->end(); ++it)
    {
        versionNumber = (*it).first;
        cache = (*it).second;

        ExportShortCacheHelper(hFile, versionNumber, cache->GetRoot());
    }
}

void CBonjourTop::ExportResults()
{

    BJString sTempFileName;
    device_count devCount;
    sTempFileName = m_pExportFileName;

    if (m_SnapshotSeconds)
    {
        BJString sTimeStamp;
        sTimeStamp.Format(time(NULL), BJString::BJSS_TIME);
        sTempFileName += "_";
        sTempFileName += sTimeStamp;
    }
    sTempFileName += ".csv";

    if (m_Collection.IsValid())
    {
        m_Collection.ExportCollection(sTempFileName);
        return;
    }

    FILE* hFile = fopen(sTempFileName.GetBuffer(),"w");

    if (hFile == NULL)
    {
        printf("file open failed %s\n",m_pExportFileName);
        return;
    }

    fprintf(hFile,"Total Number of Frames, %llu\n",m_nFrameCount);
    fprintf(hFile,"Total Number of Bytes, %llu\n",m_nTotalBytes);
    fprintf(hFile,"Total Number of Sec, %llu\n",m_EndTime-m_StartTime);

    bzero(&devCount, sizeof(devCount));
    m_DeviceMap.GetDeviceOSTypes(m_DeviceMap.GetRoot(),NULL, devCount);
    fprintf(hFile,"Total Number of Devices, %llu\n\n",devCount.iOS+devCount.OSX+devCount.unknownOS);
    fprintf(hFile,"Total Number of iOS Devices, %llu\n",devCount.iOS);
    fprintf(hFile,"Total Number of iOS Devices (>= iOS7), %llu\n", devCount.iOSWithEDNSField);
    fprintf(hFile,"Total Number of OSX Devices, %llu\n",devCount.OSX);
    fprintf(hFile,"Total Number of OSX Devices (>= OSX 10.9), %llu\n",devCount.OSXWithEDNSField);

    fprintf(hFile,"IPv4 multicast, %llu\n",m_SocketStatus[0].m_nFrameCount);
    fprintf(hFile,"IPv6 multicast, %llu\n",m_SocketStatus[1].m_nFrameCount);
    fprintf(hFile,"IPv4 Unicast, %llu\n",m_SocketStatus[2].m_nFrameCount);
    fprintf(hFile,"IPv6 Unicast, %llu\n",m_SocketStatus[3].m_nFrameCount);
    fprintf(hFile,"IPv4 Wrong subnet, %llu\n",m_SocketStatus[4].m_nFrameCount);
    fprintf(hFile,"IPv6 Wrong subnet, %llu\n\n",m_SocketStatus[5].m_nFrameCount);

    fprintf(hFile,"QuestionOnly Packets, %llu\n", m_SocketStatus[0].m_nQuestionOnlyFrames);
    fprintf(hFile,"AnswerOnly Packets, %llu\n", m_SocketStatus[0].m_nAnswerOnlyFrames);
    fprintf(hFile,"Q&A Packets, %llu\n\n", m_SocketStatus[0].m_nQandAFrames);

    fprintf(hFile,"AnswerCount for truncated frames min, %llu\n", m_MinAnswerCountForTruncatedFrames);
    fprintf(hFile,"AnswerCount for truncated frames avg, %llu\n", m_AvgAnswerCountForTruncatedFrames);
    fprintf(hFile,"AnswerCount for truncated frames max, %llu\n\n", m_MaxAnswerCountForTruncatedFrames);

    // Export Cache
    UpdateOSCounts();
    ExportPtrCache(hFile,"Application IPv4 Cache",m_ApplPtrCache.GetRoot());
    ExportShortCache(hFile, "OSX", &m_AppBreakdownIPv4OSX);
    ExportShortCache(hFile, "iOS", &m_AppBreakdownIPv4iOS);

    ExportPtrCache(hFile,"Application IPv6 Cache",m_ApplPtrCacheIPv6.GetRoot());
    ExportShortCache(hFile, "OSX", &m_AppBreakdownIPv6OSX);
    ExportShortCache(hFile, "iOS", &m_AppBreakdownIPv6iOS);

    ExportPtrCache(hFile,"Service IPv4 Cache",m_ServicePtrCache.GetRoot());
    ExportShortCache(hFile, "OSX", &m_ServiceBreakdownIPv4OSX);
    ExportShortCache(hFile, "iOS", &m_ServiceBreakdownIPv4iOS);

    ExportPtrCache(hFile,"Service IPv6 Cache",m_ServicePtrCacheIPv6.GetRoot());
    ExportShortCache(hFile, "OSX", &m_ServiceBreakdownIPv6OSX);
    ExportShortCache(hFile, "iOS", &m_ServiceBreakdownIPv6iOS);

    /// min snapshot table

    fprintf(hFile,"Min Snapshot table\n");

    for (int h=0;h<24;h++)
    {
        for(int m=0;m<60;m++)
        {
            if (m_MinSnapshot[h][m].m_nFrameCount)
            {
                fprintf(hFile,"%02d:%02d,%llu\n",h,m,m_MinSnapshot[h][m].m_nFrameCount);
            }
        }

    }

    fclose(hFile);

}

void CBonjourTop::WriteDeviceFile()
{
    BJString sTempFileName;
    BJString sTimeStamp;

    sTempFileName = m_DeviceFileName;
    sTimeStamp.Format(time(NULL), BJString::BJSS_TIME);
    sTempFileName += "_";
    sTempFileName += sTimeStamp;
    sTempFileName += ".csv";

    FILE* hFile = fopen(sTempFileName.GetBuffer(),"w");

    if (hFile == NULL)
    {
        printf("file open failed %s\n",sTempFileName.GetBuffer());
        return;
    }

    fprintf(hFile,"\"Name\",\"IPv4Address\",\"IPv6Address\",\"MACAddress\",O,\"Model\",\"Method\",\"total frames\",\"question frames\",\"QU frames\",\"answer frames\"\n");

    CDeviceNode *pDeviceNode = m_DeviceMap.GetRoot();

    if (pDeviceNode)
        pDeviceNode->Export(hFile);

    fclose(hFile);

    printf("devicemap count %llu %d\n",m_DeviceMap.GetCount(),CDeviceNode::nCreateCount);

}

void CBonjourTop::WriteVendorFile()
{
    BJString sTempFileName = "BonjourTopVendor";
    BJString sTimeStamp;

    sTimeStamp.Format(time(NULL), BJString::BJSS_TIME);
    sTempFileName += "_";
    sTempFileName += sTimeStamp;
    sTempFileName += ".csv";

    FILE* hFile = fopen(sTempFileName.GetBuffer(),"w");

    if (hFile == NULL)
    {
        printf("file open failed %s\n",sTempFileName.GetBuffer());
        return;
    }
    fprintf(hFile,"\"MACAddress\",O,\"Model\",\"Method\"\n");

    CMACAddrNode *node = m_MacMap.GetRoot();

    if (node)
        node->Export(hFile);

    fclose(hFile);
}

void CBonjourTop::WindowSizeChanged()
{
    window_size_changed = true;
}

BJ_UINT64 Hash(const char* pStr)
{
    // to fix
    BJ_UINT64 hash = 0;
    int c;

    while ((c = *pStr++))
        hash += c;

    return hash;


}

BJ_UINT64 Hash2(char* pStr)
{
    // to fix
    BJ_UINT64 hash = 0;
    int c;
    int i = 0;

    while ((c = *pStr++) && i++ < 8)
    {
        hash = hash << 8;
        hash |= c;
    }

    return hash;


}

static integer_t Usage(void)
{
    task_t targetTask = mach_task_self();
    struct task_basic_info ti;
    mach_msg_type_number_t count = TASK_BASIC_INFO_64_COUNT;

    kern_return_t kr = task_info(targetTask, TASK_BASIC_INFO_64,
                                 (task_info_t) &ti, &count);
    if (kr != KERN_SUCCESS)
    {
        printf("Kernel returned error during memory usage query");
        return 0;
    }

    // On Mac OS X, the resident_size is in bytes, not pages!
    // (This differs from the GNU Mach kernel)
   // return ti.resident_size;
    return ti.user_time.seconds;
}

///////////////


/* CStringNode */

void CStringNode::UpdateOSTypeCounts(CDeviceMap* pGlobalDeviceMap,CIPAddrMap *pIp2NameMap)
{
    if (m_rbLeft)
        ((CStringNode*)m_rbLeft)->UpdateOSTypeCounts(pGlobalDeviceMap,pIp2NameMap);
    if (m_rbRight)
        ((CStringNode*)m_rbRight)->UpdateOSTypeCounts(pGlobalDeviceMap,pIp2NameMap);

    BJ_UINT64  nDeviceUnknown = 0;
    m_nDeviceAskingiOSCount = 0;
    m_nDeviceAskingOSXCount = 0;
    m_nDeviceAnsweringiOSCount = 0;
    m_nDeviceAnsweringOSXCount = 0;
    m_nDeviceTotaliOSCount = 0;
    m_nDeviceTotalOSXCount = 0;
    m_DeviceAskingTree.GetDeviceOSTypes(m_DeviceAskingTree.GetRoot(),pIp2NameMap,m_nDeviceAskingiOSCount,m_nDeviceAskingOSXCount,nDeviceUnknown);
    nDeviceUnknown = 0;
    m_DeviceAnsweringTree.GetDeviceOSTypes(m_DeviceAnsweringTree.GetRoot(),pIp2NameMap,m_nDeviceAnsweringiOSCount,m_nDeviceAnsweringOSXCount,nDeviceUnknown);
    nDeviceUnknown = 0;
    m_DeviceTotalTree.GetDeviceOSTypes(m_DeviceTotalTree.GetRoot(), pIp2NameMap, m_nDeviceTotaliOSCount, m_nDeviceTotalOSXCount, nDeviceUnknown);
}

void CStringNode::Print(bool bCursers,bool bDescendingSort,BJ_UINT32 &nIndex, BJ_UINT32 nStartIndex,BJ_UINT32 nEndIndex)
{
    if (bDescendingSort)
    {
        if (m_rbLeft)
            ((CStringNode*)m_rbLeft)->Print(bCursers,bDescendingSort,nIndex,nStartIndex,nEndIndex);

        if (nIndex >= nStartIndex && nIndex <= nEndIndex)
        {
            if (bCursers)
            {
                printw("%3d. %-30s %10llu %10llu(%5llu/%5llu) %10llu(%5llu/%5llu) %10llu(%5llu/%5llu) %10llu(%5llu/%5llu) %10llu(%5llu/%5llu) %10llu(%5llu/%5llu)  %10llu %10llu\n",nIndex,(char*)&(m_Value),m_nBytes,m_nFrames, m_nFramesiOS, m_nFramesOSX, m_nQuestionFrames, m_nQuestionFramesiOS, m_nQuestionFramesOSX, m_nAnswerFrames,m_nAnswerFramesiOS, m_nAnswerFramesOSX, m_nDeviceAskingCount,m_nDeviceAskingiOSCount,m_nDeviceAskingOSXCount, m_nDeviceAnsweringCount,m_nDeviceAnsweringiOSCount,m_nDeviceAnsweringOSXCount, m_nDeviceTotalCount, m_nDeviceTotaliOSCount, m_nDeviceTotalOSXCount, m_nWakeFrames,m_nGoodbyeFrames);

            }
            else
            {
                printf("%3d. %-30s %10llu %10llu(%5llu/%5llu) %10llu(%5llu/%5llu) %10llu(%5llu/%5llu) %10llu(%5llu/%5llu) %10llu(%5llu/%5llu) %10llu(%5llu/%5llu)  %10llu %10llu\n",nIndex,(char*)&(m_Value),m_nBytes,m_nFrames, m_nFramesiOS, m_nFramesOSX, m_nQuestionFrames, m_nQuestionFramesiOS, m_nQuestionFramesOSX, m_nAnswerFrames,m_nAnswerFramesiOS, m_nAnswerFramesOSX, m_nDeviceAskingCount,m_nDeviceAskingiOSCount,m_nDeviceAskingOSXCount, m_nDeviceAnsweringCount,m_nDeviceAnsweringiOSCount,m_nDeviceAnsweringOSXCount, m_nDeviceTotalCount, m_nDeviceTotaliOSCount, m_nDeviceTotalOSXCount, m_nWakeFrames,m_nGoodbyeFrames);

            }
        }
        nIndex++;
        if (m_rbRight)
            ((CStringNode*)m_rbRight)->Print(bCursers,bDescendingSort,nIndex,nStartIndex,nEndIndex);
    }
    else
    {
        if (m_rbRight)
            ((CStringNode*)m_rbRight)->Print(bCursers,bDescendingSort,nIndex,nStartIndex,nEndIndex);

        if (nIndex >= nStartIndex && nIndex <= nEndIndex)
        {
            if (bCursers)
            {
                printw("%3d. %-30s %10llu %10llu(%5llu/%5llu) %10llu(%5llu/%5llu) %10llu(%5llu/%5llu) %10llu(%5llu/%5llu) %10llu(%5llu/%5llu) %10llu(%5llu/%5llu)  %10llu %10llu\n",nIndex,(char*)&(m_Value),m_nBytes,m_nFrames, m_nFramesiOS, m_nFramesOSX, m_nQuestionFrames, m_nQuestionFramesiOS, m_nQuestionFramesOSX, m_nAnswerFrames,m_nAnswerFramesiOS, m_nAnswerFramesOSX, m_nDeviceAskingCount,m_nDeviceAskingiOSCount,m_nDeviceAskingOSXCount, m_nDeviceAnsweringCount,m_nDeviceAnsweringiOSCount,m_nDeviceAnsweringOSXCount, m_nDeviceTotalCount, m_nDeviceTotaliOSCount, m_nDeviceTotalOSXCount, m_nWakeFrames,m_nGoodbyeFrames);
            }
            else
            {
                printf("%3d. %-30s %10llu %10llu(%5llu/%5llu) %10llu(%5llu/%5llu) %10llu(%5llu/%5llu) %10llu(%5llu/%5llu) %10llu(%5llu/%5llu) %10llu(%5llu/%5llu)  %10llu %10llu\n",nIndex,(char*)&(m_Value),m_nBytes,m_nFrames, m_nFramesiOS, m_nFramesOSX, m_nQuestionFrames, m_nQuestionFramesiOS, m_nQuestionFramesOSX, m_nAnswerFrames,m_nAnswerFramesiOS, m_nAnswerFramesOSX, m_nDeviceAskingCount,m_nDeviceAskingiOSCount,m_nDeviceAskingOSXCount, m_nDeviceAnsweringCount,m_nDeviceAnsweringiOSCount,m_nDeviceAnsweringOSXCount, m_nDeviceTotalCount, m_nDeviceTotaliOSCount, m_nDeviceTotalOSXCount, m_nWakeFrames,m_nGoodbyeFrames);
            }
        }
        nIndex++;
        if (m_rbLeft)
            ((CStringNode*)m_rbLeft)->Print(bCursers,bDescendingSort,nIndex,nStartIndex,nEndIndex);
    }

}
void CStringNode::Export(FILE* hFile)
{
    fprintf(hFile, "%s,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu\n",
            (char*)&(m_Value),m_nBytes,
            m_nFrames, m_nFramesiOS, m_nFramesOSX,
            m_nQuestionFrames, m_nQuestionFramesiOS, m_nQuestionFramesOSX,
            m_nAnswerFrames, m_nAnswerFramesiOS, m_nAnswerFramesOSX,
            m_nDeviceAskingCount, m_nDeviceAskingiOSCount,m_nDeviceAskingOSXCount,
            m_nDeviceAnsweringCount,m_nDeviceAnsweringiOSCount,m_nDeviceAnsweringOSXCount,
            m_nDeviceTotalCount, m_nDeviceTotaliOSCount, m_nDeviceTotalOSXCount,
            m_nWakeFrames,m_nGoodbyeFrames);

    if (m_rbLeft)
        ((CStringNode*)m_rbLeft)->Export(hFile);
    if (m_rbRight)
        ((CStringNode*)m_rbRight)->Export(hFile);

}

/* CStringShortNode */

void CStringShortNode::Export(FILE *hFile)
{
    fprintf(hFile, "%s,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu\n",
            (char*)&(m_Value),m_nBytes,
            m_nFrames, m_nQuestionFrames, m_nAnswerFrames,
            m_nDeviceAskingCount, m_nDeviceAnsweringCount, m_nDeviceTotalCount,
            m_nWakeFrames,m_nGoodbyeFrames);


    if (m_rbLeft)
    {
        ((CStringShortNode*)m_rbLeft)->Export(hFile);
    }
    if (m_rbRight)
    {
        ((CStringShortNode*)m_rbRight)->Export(hFile);
    }
}

/* CDeviceMap */

void CDeviceMap::GetDeviceOSTypes(CDeviceNode *node, CDeviceMap *pGlobalDeviceMap, device_count& dev_cnt)
{
    if (node == NULL)
        return;

    GetDeviceOSTypes(dynamic_cast<CDeviceNode*>(node->m_rbLeft),pGlobalDeviceMap, dev_cnt);
    GetDeviceOSTypes(dynamic_cast<CDeviceNode*>(node->m_rbRight),pGlobalDeviceMap, dev_cnt);

    if (node->bDuplicate || !node->bHasFrames)
        return;

    char deviceType = '?';
    if (pGlobalDeviceMap)
    {
        CDeviceNode* globalDevice = pGlobalDeviceMap->Find(&node->m_Key);
        if (globalDevice)
        {
            deviceType = globalDevice->GetDeviceOS();

            if (globalDevice->bOSXWithEDNSField && deviceType == 'X')
            {
                dev_cnt.OSXWithEDNSField++;
            }
            else if (globalDevice->biOSWithEDNSField && (deviceType == 't' || deviceType == 'i'))
            {
                dev_cnt.iOSWithEDNSField++;
            }
        }
    }
    else
    {
        deviceType = node->GetDeviceOS();
        if (node->bOSXWithEDNSField && deviceType == 'X')
        {
            dev_cnt.OSXWithEDNSField++;
        }
        else if (node->biOSWithEDNSField && (deviceType == 't' || deviceType == 'i'))
        {
            dev_cnt.iOSWithEDNSField++;
        }
    }
    switch (deviceType)
    {
        case 'i':
        case 't':
            dev_cnt.iOS++;
            break;
        case 'X':
            dev_cnt.OSX++;
            break;
        default:
            dev_cnt.unknownOS++;
            break;
    }
}

void CIPAddrMap::GetDeviceOSTypes(CIPDeviceNode* node, CIPAddrMap* pGobalMap, BJ_UINT64& iOS,BJ_UINT64& OSX,BJ_UINT64& unknowOS)
{
    if (node == NULL)
        return;

    GetDeviceOSTypes(dynamic_cast<CIPDeviceNode*>(node->m_rbLeft),pGobalMap, iOS, OSX, unknowOS);
    GetDeviceOSTypes(dynamic_cast<CIPDeviceNode*>(node->m_rbRight),pGobalMap,iOS, OSX, unknowOS);

    char deviceType = '?';
    if (pGobalMap)
    {
        CIPDeviceNode *ipDevice = pGobalMap->Find(&node->m_Key);

        if (ipDevice && ipDevice->pDeviceNode )
            deviceType = ipDevice->pDeviceNode->GetDeviceOS();

    }

    switch (deviceType)
    {
        case 'i':
        case 't':
            iOS++;
            break;
        case 'X':
            OSX++;
            break;
        default:
            unknowOS++;
    }

}


///////////

