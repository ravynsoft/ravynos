/*
 * Copyright (c) 2016-2019 Apple Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#import "Metrics.h"

#if MDNSRESPONDER_SUPPORTS(APPLE, METRICS)
#import <CoreUtils/SoftLinking.h>
#import <WirelessDiagnostics/AWDDNSDomainStats.h>
#import <WirelessDiagnostics/AWDMDNSResponderDNSMessageSizeStats.h>
#import <WirelessDiagnostics/AWDMDNSResponderDNSStatistics.h>
#import <WirelessDiagnostics/AWDMDNSResponderServicesStats.h>
#import <WirelessDiagnostics/AWDMetricIds_MDNSResponder.h>
#import <WirelessDiagnostics/WirelessDiagnostics.h>

#import "DNSCommon.h"
#import "mDNSMacOSX.h"
#import "DebugServices.h"

//===========================================================================================================================
//  External Frameworks
//===========================================================================================================================

SOFT_LINK_FRAMEWORK(PrivateFrameworks, WirelessDiagnostics)

// AWDServerConnection class

SOFT_LINK_CLASS(WirelessDiagnostics, AWDServerConnection)

#define AWDServerConnectionSoft     getAWDServerConnectionClass()

// Classes for query stats

SOFT_LINK_CLASS(WirelessDiagnostics, AWDMDNSResponderDNSStatistics)
SOFT_LINK_CLASS(WirelessDiagnostics, AWDDNSDomainStats)

#define AWDMDNSResponderDNSStatisticsSoft       getAWDMDNSResponderDNSStatisticsClass()
#define AWDDNSDomainStatsSoft                   getAWDDNSDomainStatsClass()

// Classes for services stats

SOFT_LINK_CLASS(WirelessDiagnostics, AWDMetricManager)

#define AWDMetricManagerSoft        getAWDMetricManagerClass()

// Classes for DNS message size stats

SOFT_LINK_CLASS(WirelessDiagnostics, AWDMDNSResponderDNSMessageSizeStats)

#define AWDMDNSResponderDNSMessageSizeStatsSoft     getAWDMDNSResponderDNSMessageSizeStatsClass()

//===========================================================================================================================
//  Macros
//===========================================================================================================================

#define countof(X)                      (sizeof(X) / sizeof(X[0]))
#define countof_field(TYPE, FIELD)      countof(((TYPE *)0)->FIELD)
#define ForgetMem(X)                    do {if(*(X)) {free(*(X)); *(X) = NULL;}} while(0)

//===========================================================================================================================
//  Constants
//===========================================================================================================================

#define kQueryStatsMaxQuerySendCount        10
#define kQueryStatsSendCountBinCount        (kQueryStatsMaxQuerySendCount + 1)
#define kQueryStatsLatencyBinCount          55
#define kQueryStatsExpiredAnswerStateCount  (ExpiredAnswer_EnumCount)
#define kQueryStatsDNSOverTCPStateCount     (DNSOverTCP_EnumCount)

//===========================================================================================================================
//  Data structures
//===========================================================================================================================

// Data structures for query stats.

typedef struct QueryStats       QueryStats;
typedef struct DNSHistSet       DNSHistSet;
typedef mDNSBool                (*QueryNameTest_f)(const QueryStats *inStats, const domainname *inQueryName);

struct QueryStats
{
    QueryStats *        next;           // Pointer to next domain stats in list.
    const char *        domainStr;      // Domain (see below) as a C string.
    uint8_t *           domain;         // Domain for which these stats are collected.
    const char *        altDomainStr;   // Alt domain string to use in the AWD version of the stats instead of domainStr.
    DNSHistSet *        nonCellular;    // Query stats for queries sent over non-cellular interfaces.
    DNSHistSet *        cellular;       // Query stats for queries sent over cellular interfaces.
    QueryNameTest_f     test;           // Function that tests whether a given query's stats belong based on the query name.
    int                 labelCount;     // Number of labels in domain name. Used for domain name comparisons.
    mDNSBool            terminal;       // If true and test passes, then no other QueryStats on the list should be visited.
};

check_compile_time(sizeof(QueryStats) <= 64);

// DNSHist contains the per domain per network type histogram data that goes in a DNSDomainStats protobuf message. See
// <rdar://problem/23980546> MDNSResponder.proto update.
//
// answeredQuerySendCountBins
//
// An array of 11 histogram bins. The value at index i, for 0 <= i <= 9, is the number of times that an answered DNS query
// was sent i times. The value at index 10 is the number of times that an answered query was sent 10+ times.
//
// unansweredQuerySendCountBins
//
// An array of 11 histogram bins. The value at index i, for 0 <= i <= 9, is the number of times that an unanswered DNS query
// was sent i times. The value at index 10 is the number of times that an unanswered query was sent 10+ times.
//
// responseLatencyBins
//
// An array of 55 histogram bins. Each array value is the number of DNS queries that were answered in a paricular time
// interval. The 55 consecutive non-overlapping time intervals have the following non-inclusive upper bounds (all values are
// in milliseconds): 1, 2, 3, 4, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190,
// 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000, 1500, 2000, 2500, 3000, 3500, 4000,
// 4500, 5000, 6000, 7000, 8000, 9000, 10000, ∞.

typedef struct
{
    uint16_t    unansweredQuerySendCountBins[kQueryStatsSendCountBinCount];
    uint16_t    unansweredQueryDurationBins[kQueryStatsLatencyBinCount];
    uint16_t    answeredQuerySendCountBins[kQueryStatsSendCountBinCount];
    uint16_t    responseLatencyBins[kQueryStatsLatencyBinCount];
    uint16_t    negAnsweredQuerySendCountBins[kQueryStatsSendCountBinCount];
    uint16_t    negResponseLatencyBins[kQueryStatsLatencyBinCount];
    uint32_t    expiredAnswerStateBins[kQueryStatsExpiredAnswerStateCount];
    uint32_t    dnsOverTCPStateBins[kQueryStatsDNSOverTCPStateCount];

}   DNSHist;

check_compile_time(sizeof(DNSHist) <= 512);
check_compile_time(countof_field(DNSHist, unansweredQuerySendCountBins)  == (kQueryStatsMaxQuerySendCount + 1));
check_compile_time(countof_field(DNSHist, answeredQuerySendCountBins)    == (kQueryStatsMaxQuerySendCount + 1));
check_compile_time(countof_field(DNSHist, negAnsweredQuerySendCountBins) == (kQueryStatsMaxQuerySendCount + 1));
check_compile_time(countof_field(DNSHist, expiredAnswerStateBins)        == (kQueryStatsExpiredAnswerStateCount));
check_compile_time(countof_field(DNSHist, dnsOverTCPStateBins)           == (kQueryStatsDNSOverTCPStateCount));

// Important: Do not modify kResponseLatencyMsLimits because the code used to generate AWD reports expects the response
// latency histogram bins to observe these time interval upper bounds.

static const mDNSu32        kResponseLatencyMsLimits[] =
{
        1,     2,     3,     4,     5,
       10,    20,    30,    40,    50,    60,    70,    80,    90,
      100,   110,   120,   130,   140,   150,   160,   170,   180,   190,
      200,   250,   300,   350,   400,   450,   500,   550,   600,   650,   700,   750,   800,   850,   900,   950,
     1000,  1500,  2000,  2500,  3000,  3500,  4000,  4500,
     5000,  6000,  7000,  8000,  9000,
    10000
};

check_compile_time(countof(kResponseLatencyMsLimits) == 54);
check_compile_time(countof_field(DNSHist, unansweredQueryDurationBins) == (countof(kResponseLatencyMsLimits) + 1));
check_compile_time(countof_field(DNSHist, responseLatencyBins)         == (countof(kResponseLatencyMsLimits) + 1));
check_compile_time(countof_field(DNSHist, negResponseLatencyBins)      == (countof(kResponseLatencyMsLimits) + 1));

struct DNSHistSet
{
    DNSHist *       histA;      // Histogram data for queries for A resource records.
    DNSHist *       histAAAA;   // Histogram data for queries for AAAA resource records.
};

typedef struct
{
    const char *        domainStr;
    const char *        altDomainStr;
    QueryNameTest_f     test;
    mDNSBool            terminal;

}   QueryStatsArgs;

// Data structures for DNS message size stats.

#define kQuerySizeBinWidth      16
#define kQuerySizeBinMax        512
#define kQuerySizeBinCount      ((kQuerySizeBinMax / kQuerySizeBinWidth) + 1)

check_compile_time(kQuerySizeBinWidth > 0);
check_compile_time(kQuerySizeBinCount > 0);
check_compile_time((kQuerySizeBinMax % kQuerySizeBinWidth) == 0);

#define kResponseSizeBinWidth       16
#define kResponseSizeBinMax         512
#define kResponseSizeBinCount       ((kResponseSizeBinMax / kResponseSizeBinWidth) + 1)

check_compile_time(kResponseSizeBinWidth > 0);
check_compile_time(kResponseSizeBinCount > 0);
check_compile_time((kResponseSizeBinMax % kResponseSizeBinWidth) == 0);

typedef struct
{
    uint32_t    querySizeBins[kQuerySizeBinCount];
    uint32_t    responseSizeBins[kResponseSizeBinCount];

}   DNSMessageSizeStats;

check_compile_time(sizeof(DNSMessageSizeStats) <= 264);

//===========================================================================================================================
//  Local Prototypes
//===========================================================================================================================

// Query stats

mDNSlocal mStatus       QueryStatsCreate(const char *inDomainStr, const char *inAltDomainStr, QueryNameTest_f inTest, mDNSBool inTerminal, QueryStats **outStats);
mDNSlocal void          QueryStatsFree(QueryStats *inStats);
mDNSlocal void          QueryStatsFreeList(QueryStats *inList);
mDNSlocal mStatus       QueryStatsUpdate(QueryStats *inStats, int inType, const ResourceRecord *inRR, mDNSu32 inQuerySendCount, ExpiredAnswerMetric inExpiredAnswerState, DNSOverTCPMetric inDNSOverTCPState, mDNSu32 inLatencyMs, mDNSBool inForCell);
mDNSlocal const char *  QueryStatsGetDomainString(const QueryStats *inStats);
mDNSlocal mDNSBool      QueryStatsDomainTest(const QueryStats *inStats, const domainname *inQueryName);
mDNSlocal mDNSBool      QueryStatsHostnameTest(const QueryStats *inStats, const domainname *inQueryName);
mDNSlocal mDNSBool      QueryStatsContentiCloudTest(const QueryStats *inStats, const domainname *inQueryName);
mDNSlocal mDNSBool      QueryStatsCourierPushTest(const QueryStats *inStats, const domainname *inQueryName);

// DNS message size stats

mDNSlocal mStatus   DNSMessageSizeStatsCreate(DNSMessageSizeStats **outStats);
mDNSlocal void      DNSMessageSizeStatsFree(DNSMessageSizeStats *inStats);

mDNSlocal mStatus   CreateQueryStatsList(QueryStats **outList);
mDNSlocal mStatus   SubmitAWDMetric(UInt32 inMetricID);
mDNSlocal mStatus   SubmitAWDMetricQueryStats(void);
mDNSlocal mStatus   SubmitAWDMetricDNSMessageSizeStats(void);
mDNSlocal mStatus   CreateAWDDNSDomainStats(DNSHist *inHist, const char *inDomain, mDNSBool inForCell, AWDDNSDomainStats_RecordType inType, AWDDNSDomainStats **outStats);
mDNSlocal void      LogDNSHistSetToFD(int fd, const DNSHistSet *inSet, const char *inDomain, mDNSBool inForCell);
mDNSlocal void      LogDNSHistToFD(int fd, const DNSHist *inHist, const char *inDomain, mDNSBool inForCell, const char *inType);
mDNSlocal void      LogDNSHistSendCountsToFD(int fd, const uint16_t inSendCountBins[kQueryStatsSendCountBinCount]);
mDNSlocal void      LogDNSHistLatenciesToFD(int fd, const uint16_t inLatencyBins[kQueryStatsLatencyBinCount]);
mDNSlocal void      LogDNSMessageSizeStatsToFD(int fd, const uint32_t *inBins, size_t inBinCount, unsigned int inBinWidth);

//===========================================================================================================================
//  Histogram Bin Helpers
//===========================================================================================================================

#define INCREMENT_BIN_DEFINITION(BIN_SIZE) \
    mDNSlocal void IncrementBin ## BIN_SIZE (uint ## BIN_SIZE ## _t *inBin) \
    { \
        if (*inBin < UINT ## BIN_SIZE ## _MAX) ++(*inBin); \
    } \
    extern int _MetricsDummyVariable

INCREMENT_BIN_DEFINITION(16);
INCREMENT_BIN_DEFINITION(32);

//  Note: The return value is the size (in number of elements) of the smallest contiguous sub-array that contains the first
//  bin and all bins with non-zero values.

#define COPY_BINS_DEFINITION(BIN_SIZE) \
    mDNSlocal size_t CopyBins ## BIN_SIZE (uint32_t *inDstBins, uint ## BIN_SIZE ## _t *inSrcBins, size_t inBinCount) \
    { \
        if (inBinCount == 0) return (0); \
        size_t minCount = 1; \
        for (size_t i = 0; i < inBinCount; ++i) \
        { \
            inDstBins[i] = inSrcBins[i]; \
            if (inDstBins[i] > 0) minCount = i + 1; \
        } \
        return (minCount); \
    } \
    extern int _MetricsDummyVariable

COPY_BINS_DEFINITION(16);
COPY_BINS_DEFINITION(32);

//===========================================================================================================================
//  Globals
//===========================================================================================================================

static AWDServerConnection *        gAWDServerConnection    = nil;
static QueryStats *                 gQueryStatsList         = NULL;
static DNSMessageSizeStats *        gDNSMessageSizeStats    = NULL;

// Important: Do not add to this list without getting privacy approval. See <rdar://problem/24155761&26397203&34763471>.

static const QueryStatsArgs     kQueryStatsArgs[] =
{
    { ".",                      NULL,                               QueryStatsDomainTest,           mDNSfalse },
    { "",                       "alt:*-courier.push.apple.com.",    QueryStatsCourierPushTest,      mDNSfalse },
    { "apple.com.",             NULL,                               QueryStatsDomainTest,           mDNStrue  },
    { "gateway.icloud.com.",    "alt:gateway.icloud.com",           QueryStatsHostnameTest,         mDNSfalse },
    { "",                       "alt:*-content.icloud.com.",        QueryStatsContentiCloudTest,    mDNSfalse },
    { "icloud.com.",            NULL,                               QueryStatsDomainTest,           mDNStrue  },
    { "mzstatic.com.",          NULL,                               QueryStatsDomainTest,           mDNStrue  },
    { "google.com.",            NULL,                               QueryStatsDomainTest,           mDNStrue  },
    { "baidu.com.",             NULL,                               QueryStatsDomainTest,           mDNStrue  },
    { "yahoo.com.",             NULL,                               QueryStatsDomainTest,           mDNStrue  },
    { "qq.com.",                NULL,                               QueryStatsDomainTest,           mDNStrue  }
};

check_compile_time(countof(kQueryStatsArgs) == 11);

//===========================================================================================================================
//  MetricsInit
//===========================================================================================================================

mStatus MetricsInit(void)
{
    @autoreleasepool
    {
        gAWDServerConnection = [[AWDServerConnectionSoft alloc]
            initWithComponentId:     AWDComponentId_MDNSResponder
            andBlockOnConfiguration: NO];

        if (gAWDServerConnection)
        {
            [gAWDServerConnection
                registerQueriableMetricCallback: ^(UInt32 inMetricID)
                {
                    SubmitAWDMetric(inMetricID);
                }
                forIdentifier: (UInt32)AWDMetricId_MDNSResponder_DNSStatistics];

            [gAWDServerConnection
                registerQueriableMetricCallback: ^(UInt32 inMetricID)
                {
                    SubmitAWDMetric(inMetricID);
                }
                forIdentifier: (UInt32)AWDMetricId_MDNSResponder_ServicesStats];

            [gAWDServerConnection
                registerQueriableMetricCallback: ^(UInt32 inMetricID)
                {
                    SubmitAWDMetric(inMetricID);
                }
                forIdentifier: (UInt32)AWDMetricId_MDNSResponder_DNSMessageSizeStats];
        }
        else
        {
            LogMsg("MetricsInit: failed to create AWD server connection.");
        }
    }

    if( gAWDServerConnection )
    {
        CreateQueryStatsList(&gQueryStatsList);
        DNSMessageSizeStatsCreate(&gDNSMessageSizeStats);
    }

    return (mStatus_NoError);
}

//===========================================================================================================================
//  MetricsUpdateDNSQueryStats
//===========================================================================================================================

mDNSexport void MetricsUpdateDNSQueryStats(const domainname *inQueryName, mDNSu16 inType, const ResourceRecord *inRR, mDNSu32 inSendCount, ExpiredAnswerMetric inExpiredAnswerState, DNSOverTCPMetric inDNSOverTCPState, mDNSu32 inLatencyMs, mDNSBool inForCell)
{
    QueryStats *        stats;
    mDNSBool            match;

    require_quiet(gAWDServerConnection, exit);
    require_quiet((inType == kDNSType_A) || (inType == kDNSType_AAAA), exit);

    for (stats = gQueryStatsList; stats; stats = stats->next)
    {
        match = stats->test(stats, inQueryName);
        if (match)
        {
            QueryStatsUpdate(stats, inType, inRR, inSendCount, inExpiredAnswerState, inDNSOverTCPState, inLatencyMs, inForCell);
            if (stats->terminal) break;
        }
    }

exit:
    return;
}

//===========================================================================================================================
//  MetricsUpdateDNSQuerySize
//===========================================================================================================================

mDNSlocal void UpdateMessageSizeCounts(uint32_t *inBins, size_t inBinCount, unsigned int inBinWidth, uint32_t inSize);

mDNSexport void MetricsUpdateDNSQuerySize(mDNSu32 inSize)
{
    if (!gDNSMessageSizeStats) return;
    UpdateMessageSizeCounts(gDNSMessageSizeStats->querySizeBins, kQuerySizeBinCount, kQuerySizeBinWidth, inSize);
}

mDNSlocal void UpdateMessageSizeCounts(uint32_t *inBins, size_t inBinCount, unsigned int inBinWidth, uint32_t inSize)
{
    size_t      i;

    if (inSize == 0) return;
    i = (inSize - 1) / inBinWidth;
    if (i >= inBinCount) i = inBinCount - 1;
    IncrementBin32(&inBins[i]);
}

//===========================================================================================================================
//  MetricsUpdateDNSResponseSize
//===========================================================================================================================

mDNSexport void MetricsUpdateDNSResponseSize(mDNSu32 inSize)
{
    if (!gDNSMessageSizeStats) return;
    UpdateMessageSizeCounts(gDNSMessageSizeStats->responseSizeBins, kResponseSizeBinCount, kResponseSizeBinWidth, inSize);
}

//===========================================================================================================================
//  LogMetrics
//===========================================================================================================================

mDNSexport void LogMetricsToFD(int fd)
{
    QueryStats *        stats;

    LogToFD(fd, "gAWDServerConnection %p", gAWDServerConnection);
    LogToFD(fd, "---- DNS query stats by domain -----");

    for (stats = gQueryStatsList; stats; stats = stats->next)
    {
        if (!stats->nonCellular && !stats->cellular)
        {
            LogToFD(fd, "No data for %s", QueryStatsGetDomainString(stats));
            continue;
        }
        if (stats->nonCellular) LogDNSHistSetToFD(fd, stats->nonCellular, QueryStatsGetDomainString(stats), mDNSfalse);
        if (stats->cellular)    LogDNSHistSetToFD(fd, stats->cellular,    QueryStatsGetDomainString(stats), mDNStrue);
    }

    LogToFD(fd, "---- Num of Services Registered -----");
    LogToFD(fd, "Current_number_of_services_registered :[%d], Max_number_of_services_registered :[%d]",
              curr_num_regservices, max_num_regservices);

    if (gDNSMessageSizeStats)
    {
        LogToFD(fd, "---- DNS query size stats ---");
        LogDNSMessageSizeStatsToFD(fd, gDNSMessageSizeStats->querySizeBins, kQuerySizeBinCount, kQuerySizeBinWidth);

        LogToFD(fd, "-- DNS response size stats --");
        LogDNSMessageSizeStatsToFD(fd, gDNSMessageSizeStats->responseSizeBins, kResponseSizeBinCount, kResponseSizeBinWidth);
    }
    else
    {
        LogToFD(fd, "No DNS message size stats.");
    }
}

//===========================================================================================================================
//  QueryStatsCreate
//===========================================================================================================================

mDNSlocal mStatus StringToDomainName(const char *inString, uint8_t **outDomainName);

mDNSlocal mStatus QueryStatsCreate(const char *inDomainStr, const char *inAltDomainStr, QueryNameTest_f inTest, mDNSBool inTerminal, QueryStats **outStats)
{
    mStatus             err;
    QueryStats *        obj;

    obj = (QueryStats *)calloc(1, sizeof(*obj));
    require_action_quiet(obj, exit, err = mStatus_NoMemoryErr);

    obj->domainStr = inDomainStr;
    err = StringToDomainName(obj->domainStr, &obj->domain);
    require_noerr_quiet(err, exit);

    obj->altDomainStr   = inAltDomainStr;
    obj->test           = inTest;
    obj->labelCount     = CountLabels((const domainname *)obj->domain);
    obj->terminal       = inTerminal;

    *outStats = obj;
    obj = NULL;
    err = mStatus_NoError;

exit:
    if (obj) QueryStatsFree(obj);
    return (err);
}

mDNSlocal mStatus StringToDomainName(const char *inString, uint8_t **outDomainName)
{
    mStatus             err;
    uint8_t *           domainPtr = NULL;
    size_t              domainLen;
    const mDNSu8 *      ptr;
    domainname          domain;

    if (strcmp(inString, ".") == 0)
    {
        domain.c[0] = 0;
    }
    else
    {
        ptr = MakeDomainNameFromDNSNameString(&domain, inString);
        require_action_quiet(ptr, exit, err = mStatus_BadParamErr);
    }
    domainLen = DomainNameLength(&domain);

    domainPtr = (uint8_t *)malloc(domainLen);
    require_action_quiet(domainPtr, exit, err = mStatus_NoMemoryErr);

    memcpy(domainPtr, domain.c, domainLen);

    *outDomainName = domainPtr;
    domainPtr = NULL;
    err = mStatus_NoError;

exit:
    return(err);
}

//===========================================================================================================================
//  QueryStatsFree
//===========================================================================================================================

mDNSlocal void QueryStatsFree(QueryStats *inStats)
{
    ForgetMem(&inStats->domain);
    if (inStats->nonCellular)
    {
        ForgetMem(&inStats->nonCellular->histA);
        ForgetMem(&inStats->nonCellular->histAAAA);
        free(inStats->nonCellular);
        inStats->nonCellular = NULL;
    }
    if (inStats->cellular)
    {
        ForgetMem(&inStats->cellular->histA);
        ForgetMem(&inStats->cellular->histAAAA);
        free(inStats->cellular);
        inStats->cellular = NULL;
    }
    free(inStats);
}

//===========================================================================================================================
//  QueryStatsFreeList
//===========================================================================================================================

mDNSlocal void QueryStatsFreeList(QueryStats *inList)
{
    QueryStats *        stats;

    while ((stats = inList) != NULL)
    {
        inList = stats->next;
        QueryStatsFree(stats);
    }
}

//===========================================================================================================================
//  QueryStatsUpdate
//===========================================================================================================================

mDNSlocal mStatus QueryStatsUpdate(QueryStats *inStats, int inType, const ResourceRecord *inRR, mDNSu32 inQuerySendCount, ExpiredAnswerMetric inExpiredAnswerState, DNSOverTCPMetric inDNSOverTCPState, mDNSu32 inLatencyMs, mDNSBool inForCell)
{
    mStatus             err;
    DNSHistSet *        set;
    DNSHistSet **       pSet;
    DNSHist *           hist;
    DNSHist **          pHist;
    int                 i;

    require_action_quiet(inRR || (inQuerySendCount > 0), exit, err = mStatus_NoError);
    require_action_quiet((inType == kDNSType_A) || (inType == kDNSType_AAAA), exit, err = mStatus_NoError);

    pSet = inForCell ? &inStats->cellular : &inStats->nonCellular;
    if ((set = *pSet) == NULL)
    {
        set = (DNSHistSet *)calloc(1, sizeof(*set));
        require_action_quiet(set, exit, err = mStatus_NoMemoryErr);
        *pSet = set;
    }
    pHist = (inType == kDNSType_A) ? &set->histA : &set->histAAAA;
    if ((hist = *pHist) == NULL)
    {
        hist = (DNSHist *)calloc(1, sizeof(*hist));
        require_action_quiet(hist, exit, err = mStatus_NoMemoryErr);
        *pHist = hist;
    }

    if (inRR)
    {
        uint16_t *          sendCountBins;
        uint16_t *          latencyBins;
        const mDNSBool      isNegative = (inRR->RecordType == kDNSRecordTypePacketNegative);

        i = Min(inQuerySendCount, kQueryStatsMaxQuerySendCount);

        sendCountBins = isNegative ? hist->negAnsweredQuerySendCountBins : hist->answeredQuerySendCountBins;
        IncrementBin16(&sendCountBins[i]);

        if (inQuerySendCount > 0)
        {
            for (i = 0; (i < (int)countof(kResponseLatencyMsLimits)) && (inLatencyMs >= kResponseLatencyMsLimits[i]); ++i) {}
            latencyBins = isNegative ? hist->negResponseLatencyBins : hist->responseLatencyBins;
            IncrementBin16(&latencyBins[i]);
        }
    }
    else
    {
        i = Min(inQuerySendCount, kQueryStatsMaxQuerySendCount);
        IncrementBin16(&hist->unansweredQuerySendCountBins[i]);

        for (i = 0; (i < (int)countof(kResponseLatencyMsLimits)) && (inLatencyMs >= kResponseLatencyMsLimits[i]); ++i) {}
        IncrementBin16(&hist->unansweredQueryDurationBins[i]);
    }
    IncrementBin32(&hist->expiredAnswerStateBins[Min(inExpiredAnswerState, (kQueryStatsExpiredAnswerStateCount - 1))]);
    IncrementBin32(&hist->dnsOverTCPStateBins[Min(inDNSOverTCPState, (kQueryStatsDNSOverTCPStateCount - 1))]);
    err = mStatus_NoError;

exit:
    return (err);
}

//===========================================================================================================================
//  QueryStatsGetDomainString
//===========================================================================================================================

mDNSlocal const char * QueryStatsGetDomainString(const QueryStats *inStats)
{
    return (inStats->altDomainStr ? inStats->altDomainStr : inStats->domainStr);
}

//===========================================================================================================================
//  QueryStatsDomainTest
//===========================================================================================================================

mDNSlocal mDNSBool QueryStatsDomainTest(const QueryStats *inStats, const domainname *inQueryName)
{
    const domainname *      parentDomain;
    int                     labelCount;

    if (inStats->domain[0] == 0) return (mDNStrue);

    labelCount = CountLabels(inQueryName);
    if (labelCount < inStats->labelCount) return (mDNSfalse);

    parentDomain = SkipLeadingLabels(inQueryName, labelCount - inStats->labelCount);
    return (SameDomainName(parentDomain, (const domainname *)inStats->domain));
}

//===========================================================================================================================
//  QueryStatsHostnameTest
//===========================================================================================================================

mDNSlocal mDNSBool QueryStatsHostnameTest(const QueryStats *inStats, const domainname *inQueryName)
{
    return (SameDomainName(inQueryName, (const domainname *)inStats->domain));
}

//===========================================================================================================================
//  QueryStatsContentiCloudTest
//===========================================================================================================================

mDNSlocal const uint8_t *LocateLabelSuffix(const uint8_t *inLabel, const uint8_t *inSuffixPtr, size_t inSuffixLen);

#define kContentSuffixStr       "-content"

mDNSlocal mDNSBool QueryStatsContentiCloudTest(const QueryStats *inStats, const domainname *inQueryName)
{
    const mDNSu8 * const    firstLabel = inQueryName->c;
    const uint8_t *         suffix;
    const domainname *      parentDomain;
    int                     labelCount;

    (void) inStats; // Unused.

    labelCount = CountLabels(inQueryName);
    if (labelCount != 3) return (mDNSfalse);

    suffix = LocateLabelSuffix(firstLabel, (const uint8_t *)kContentSuffixStr, sizeof_string(kContentSuffixStr));
    if (suffix && (suffix > &firstLabel[1]))
    {
        parentDomain = SkipLeadingLabels(inQueryName, 1);
        if (SameDomainName(parentDomain, (const domainname *)"\x6" "icloud" "\x3" "com"))
        {
            return (mDNStrue);
        }
    }

    return (mDNSfalse);
}

mDNSlocal const uint8_t *LocateLabelSuffix(const uint8_t *inLabel, const uint8_t *inSuffixPtr, size_t inSuffixLen)
{
    const uint8_t *     ptr;
    const uint8_t *     lp;
    const uint8_t *     sp;
    size_t              len;
    const size_t        labelLen = inLabel[0];

    if (labelLen < inSuffixLen) return (NULL);

    ptr = &inLabel[1 + labelLen - inSuffixLen];
    lp  = ptr;
    sp  = inSuffixPtr;
    for (len = inSuffixLen; len > 0; --len)
    {
        if (tolower(*lp) != tolower(*sp)) return (NULL);
        ++lp;
        ++sp;
    }

    return (ptr);
}

//===========================================================================================================================
//  QueryStatsCourierPushTest
//===========================================================================================================================

#define kCourierSuffixStr       "-courier"

mDNSlocal mDNSBool QueryStatsCourierPushTest(const QueryStats *inStats, const domainname *inQueryName)
{
    const mDNSu8 * const    firstLabel = inQueryName->c;
    const uint8_t *         suffix;
    const uint8_t *         ptr;
    const domainname *      parentDomain;
    int                     labelCount;

    (void) inStats; // Unused.

    labelCount = CountLabels(inQueryName);
    if (labelCount != 4) return (mDNSfalse);

    suffix = LocateLabelSuffix(firstLabel, (const mDNSu8 *)kCourierSuffixStr, sizeof_string(kCourierSuffixStr));
    if (suffix && (suffix > &firstLabel[1]))
    {
        for (ptr = &firstLabel[1]; ptr < suffix; ++ptr)
        {
            if (!isdigit(*ptr)) break;
        }
        if (ptr == suffix)
        {
            parentDomain = SkipLeadingLabels(inQueryName, 1);
            if (SameDomainName(parentDomain, (const domainname *)"\x4" "push" "\x5" "apple" "\x3" "com"))
            {
                return (mDNStrue);
            }
        }
    }

    return (mDNSfalse);
}

//===========================================================================================================================
//  DNSMessageSizeStatsCreate
//===========================================================================================================================

mDNSlocal mStatus DNSMessageSizeStatsCreate(DNSMessageSizeStats **outStats)
{
    mStatus                     err;
    DNSMessageSizeStats *       stats;

    stats = (DNSMessageSizeStats *)calloc(1, sizeof(*stats));
    require_action_quiet(stats, exit, err = mStatus_NoMemoryErr);

    *outStats = stats;
    err = mStatus_NoError;

exit:
    return (err);
}

//===========================================================================================================================
//  DNSMessageSizeStatsFree
//===========================================================================================================================

mDNSlocal void DNSMessageSizeStatsFree(DNSMessageSizeStats *inStats)
{
    free(inStats);
}

//===========================================================================================================================
//  CreateQueryStatsList
//===========================================================================================================================

mDNSlocal mStatus CreateQueryStatsList(QueryStats **outList)
{
    mStatus                             err;
    QueryStats **                       p;
    QueryStats *                        stats;
    const QueryStatsArgs *              args;
    const QueryStatsArgs * const        end     = kQueryStatsArgs + countof(kQueryStatsArgs);
    QueryStats *                        list    = NULL;

    p = &list;
    for (args = kQueryStatsArgs; args < end; ++args)
    {
        err = QueryStatsCreate(args->domainStr, args->altDomainStr, args->test, args->terminal, &stats);
        require_noerr_quiet(err, exit);

        *p = stats;
        p = &stats->next;
    }

    *outList = list;
    list = NULL;
    err = mStatus_NoError;

exit:
    QueryStatsFreeList(list);
    return (err);
}

//===========================================================================================================================
//  SubmitAWDMetric
//===========================================================================================================================

mDNSlocal mStatus SubmitAWDMetric(UInt32 inMetricID)
{
    mStatus     err;

    switch (inMetricID)
    {
        case AWDMetricId_MDNSResponder_DNSStatistics:
            err = SubmitAWDMetricQueryStats();
            break;

        case AWDMetricId_MDNSResponder_ServicesStats:
            [AWDMetricManagerSoft postMetricWithId:AWDMetricId_MDNSResponder_ServicesStats unsignedIntegerValue:max_num_regservices];
            KQueueLock();
            // reset the no of max services since we want to collect the max no of services registered per AWD submission period
            max_num_regservices = curr_num_regservices;
            KQueueUnlock("SubmitAWDSimpleMetricServiceStats");
            err = mStatus_NoError;
            break;

        case AWDMetricId_MDNSResponder_DNSMessageSizeStats:
            err = SubmitAWDMetricDNSMessageSizeStats();
            break;

        default:
            err = mStatus_UnsupportedErr;
            break;
    }

    if (err) LogMsg("SubmitAWDMetric for metric ID 0x%08X failed with error %d", inMetricID, err);
    return (err);
}

//===========================================================================================================================
//  SubmitAWDMetricQueryStats
//===========================================================================================================================

mDNSlocal mStatus   AddQueryStats(AWDMDNSResponderDNSStatistics *inMetric, const QueryStats *inStats);
mDNSlocal mStatus   AddDNSHistSet(AWDMDNSResponderDNSStatistics *inMetric, DNSHistSet *inSet, const char *inDomain, mDNSBool inForCell);

mDNSlocal mStatus SubmitAWDMetricQueryStats(void)
{
    mStatus                             err;
    BOOL                                success;
    QueryStats *                        stats;
    QueryStats *                        statsList;
    QueryStats *                        newStatsList;
    AWDMetricContainer *                container   = nil;
    AWDMDNSResponderDNSStatistics *     metric      = nil;

    newStatsList = NULL;
    CreateQueryStatsList(&newStatsList);

    KQueueLock();
    statsList       = gQueryStatsList;
    gQueryStatsList = newStatsList;
    KQueueUnlock("SubmitAWDMetricQueryStats");

    container = [gAWDServerConnection newMetricContainerWithIdentifier:AWDMetricId_MDNSResponder_DNSStatistics];
    require_action_quiet(container, exit, err = mStatus_UnknownErr);

    metric = [[AWDMDNSResponderDNSStatisticsSoft alloc] init];
    require_action_quiet(metric, exit, err = mStatus_UnknownErr);

    while ((stats = statsList) != NULL)
    {
        err = AddQueryStats(metric, stats);
        require_noerr_quiet(err, exit);

        statsList = stats->next;
        QueryStatsFree(stats);
    }

    container.metric = metric;
    success = [gAWDServerConnection submitMetric:container];
    LogMsg("SubmitAWDMetricQueryStats: metric submission %s.", success ? "succeeded" : "failed");
    err = success ? mStatus_NoError : mStatus_UnknownErr;

exit:
    QueryStatsFreeList(statsList);
    return (err);
}

mDNSlocal mStatus AddQueryStats(AWDMDNSResponderDNSStatistics *inMetric, const QueryStats *inStats)
{
    mStatus     err;

    if (inStats->nonCellular)
    {
        err = AddDNSHistSet(inMetric, inStats->nonCellular, QueryStatsGetDomainString(inStats), mDNSfalse);
        require_noerr_quiet(err, exit);
    }
    if (inStats->cellular)
    {
        err = AddDNSHistSet(inMetric, inStats->cellular, QueryStatsGetDomainString(inStats), mDNStrue);
        require_noerr_quiet(err, exit);
    }
    err = mStatus_NoError;

exit:
    return (err);
}

mDNSlocal mStatus AddDNSHistSet(AWDMDNSResponderDNSStatistics *inMetric, DNSHistSet *inSet, const char *inDomain, mDNSBool inForCell)
{
    mStatus                 err;
    AWDDNSDomainStats *     awdStats;

    if (inSet->histA)
    {
        err = CreateAWDDNSDomainStats(inSet->histA, inDomain, inForCell, AWDDNSDomainStats_RecordType_A, &awdStats);
        require_noerr_quiet(err, exit);

        [inMetric addStats:awdStats];
    }
    if (inSet->histAAAA)
    {
        err = CreateAWDDNSDomainStats(inSet->histAAAA, inDomain, inForCell, AWDDNSDomainStats_RecordType_AAAA, &awdStats);
        require_noerr_quiet(err, exit);

        [inMetric addStats:awdStats];
    }
    err = mStatus_NoError;

exit:
    return (err);
}

//===========================================================================================================================
//  SubmitAWDMetricDNSMessageSizeStats
//===========================================================================================================================

mDNSlocal mStatus SubmitAWDMetricDNSMessageSizeStats(void)
{
    mStatus                                     err;
    DNSMessageSizeStats *                       stats;
    DNSMessageSizeStats *                       newStats;
    AWDMetricContainer *                        container;
    AWDMDNSResponderDNSMessageSizeStats *       metric = nil;
    BOOL                                        success;

    newStats = NULL;
    DNSMessageSizeStatsCreate(&newStats);

    KQueueLock();
    stats                   = gDNSMessageSizeStats;
    gDNSMessageSizeStats    = newStats;
    KQueueUnlock("SubmitAWDMetricDNSMessageSizeStats");

    container = [gAWDServerConnection newMetricContainerWithIdentifier:AWDMetricId_MDNSResponder_DNSMessageSizeStats];
    require_action_quiet(container, exit, err = mStatus_UnknownErr);

    metric = [[AWDMDNSResponderDNSMessageSizeStatsSoft alloc] init];
    require_action_quiet(metric, exit, err = mStatus_UnknownErr);

    if (stats)
    {
        size_t          binCount;
        uint32_t        bins[Max(kQuerySizeBinCount, kResponseSizeBinCount)];

        // Set query size counts.

        binCount = CopyBins32(bins, stats->querySizeBins, kQuerySizeBinCount);
        [metric setQuerySizeCounts:bins count:(NSUInteger)binCount];

        // Set response size counts.

        binCount = CopyBins32(bins, stats->responseSizeBins, kResponseSizeBinCount);
        [metric setResponseSizeCounts:bins count:(NSUInteger)binCount];
    }

    container.metric = metric;
    success = [gAWDServerConnection submitMetric:container];
    LogMsg("SubmitAWDMetricDNSMessageSizeStats: metric submission %s.", success ? "succeeded" : "failed");
    err = success ? mStatus_NoError : mStatus_UnknownErr;

exit:
    if (stats) DNSMessageSizeStatsFree(stats);
    return (err);
}

//===========================================================================================================================
//  CreateAWDDNSDomainStats
//===========================================================================================================================

mDNSlocal mStatus CreateAWDDNSDomainStats(DNSHist *inHist, const char *inDomain, mDNSBool inForCell, AWDDNSDomainStats_RecordType inType, AWDDNSDomainStats **outStats)
{
    mStatus                 err;
    AWDDNSDomainStats *     awdStats    = nil;
    NSString *              domain      = nil;
    size_t                  binCount;
    uint32_t                sendCountBins[kQueryStatsSendCountBinCount];
    uint32_t                latencyBins[kQueryStatsLatencyBinCount];
    uint32_t                expiredAnswerBins[kQueryStatsExpiredAnswerStateCount];
    uint32_t                dnsOverTCPBins[kQueryStatsDNSOverTCPStateCount];

    awdStats = [[AWDDNSDomainStatsSoft alloc] init];
    require_action_quiet(awdStats, exit, err = mStatus_UnknownErr);

    domain = [[NSString alloc] initWithUTF8String:inDomain];
    require_action_quiet(domain, exit, err = mStatus_UnknownErr);

    awdStats.domain      = domain;
    awdStats.networkType = inForCell ? AWDDNSDomainStats_NetworkType_Cellular : AWDDNSDomainStats_NetworkType_NonCellular;
    awdStats.recordType  = inType;

    // Positively answered query send counts

    binCount = CopyBins16(sendCountBins, inHist->answeredQuerySendCountBins, kQueryStatsSendCountBinCount);
    [awdStats setAnsweredQuerySendCounts:sendCountBins count:(NSUInteger)binCount];

    // binCount > 1 means that at least one of the non-zero send count bins had a non-zero count, i.e., at least one query
    // was sent out on the wire. In that case, include the associated latency bins as well.

    if (binCount > 1)
    {
        binCount = CopyBins16(latencyBins, inHist->responseLatencyBins, kQueryStatsLatencyBinCount);
        [awdStats setResponseLatencyMs:latencyBins count:(NSUInteger)binCount];
    }

    // Negatively answered query send counts

    binCount = CopyBins16(sendCountBins, inHist->negAnsweredQuerySendCountBins, kQueryStatsSendCountBinCount);
    [awdStats setNegAnsweredQuerySendCounts:sendCountBins count:(NSUInteger)binCount];

    if (binCount > 1)
    {
        binCount = CopyBins16(latencyBins, inHist->negResponseLatencyBins, kQueryStatsLatencyBinCount);
        [awdStats setNegResponseLatencyMs:latencyBins count:(NSUInteger)binCount];
    }

    // Unanswered query send counts

    binCount = CopyBins16(sendCountBins, inHist->unansweredQuerySendCountBins, kQueryStatsSendCountBinCount);
    [awdStats setUnansweredQuerySendCounts:sendCountBins count:(NSUInteger)binCount];

    if (binCount > 1)
    {
        binCount = CopyBins16(latencyBins, inHist->unansweredQueryDurationBins, kQueryStatsLatencyBinCount);
        [awdStats setUnansweredQueryDurationMs:latencyBins count:(NSUInteger)binCount];
    }
    
    // Expired answers states
    
    binCount = CopyBins32(expiredAnswerBins, inHist->expiredAnswerStateBins, kQueryStatsExpiredAnswerStateCount);
    [awdStats setExpiredAnswerStates:expiredAnswerBins count:(NSUInteger)binCount];

    // DNS Over TCP states

    binCount = CopyBins32(dnsOverTCPBins, inHist->dnsOverTCPStateBins, kQueryStatsDNSOverTCPStateCount);
    [awdStats setDnsOverTCPStates:dnsOverTCPBins count:(NSUInteger)binCount];

   *outStats = awdStats;
    err = mStatus_NoError;

exit:
    return (err);
}

//===========================================================================================================================
//  LogDNSHistSetToFD
//===========================================================================================================================

mDNSlocal void LogDNSHistSetToFD(int fd, const DNSHistSet *inSet, const char *inDomain, mDNSBool inForCell)
{
    if (inSet->histA)       LogDNSHistToFD(fd, inSet->histA,    inDomain, inForCell, "A");
    if (inSet->histAAAA)    LogDNSHistToFD(fd, inSet->histAAAA, inDomain, inForCell, "AAAA");
}

//===========================================================================================================================
//  LogDNSHistToFD
//===========================================================================================================================

#define Percent(N, D)       (((N) * 100) / (D)), ((((N) * 10000) / (D)) % 100)
#define PercentFmt          "%3u.%02u"
#define LogStatToFD(FILE_DESCRIPTOR, LABEL, COUNT, ACCUMULATOR, TOTAL) \
    LogToFD((FILE_DESCRIPTOR), "%s %5u " PercentFmt " " PercentFmt, (LABEL), (COUNT), Percent(COUNT, TOTAL), Percent(ACCUMULATOR, TOTAL))

mDNSlocal void LogDNSHistToFD(int fd, const DNSHist *inHist, const char *inDomain, mDNSBool inForCell, const char *inType)
{
    unsigned int        totalAnswered;
    unsigned int        totalNegAnswered;
    unsigned int        totalUnanswered;
    int                 i;

    totalAnswered = 0;
    for (i = 0; i < kQueryStatsSendCountBinCount; ++i)
    {
        totalAnswered += inHist->answeredQuerySendCountBins[i];
    }

    totalNegAnswered = 0;
    for (i = 0; i < kQueryStatsSendCountBinCount; ++i)
    {
        totalNegAnswered += inHist->negAnsweredQuerySendCountBins[i];
    }

    totalUnanswered = 0;
    for (i = 0; i < kQueryStatsSendCountBinCount; ++i)
    {
        totalUnanswered += inHist->unansweredQuerySendCountBins[i];
    }

    LogToFD(fd, "Domain: %s (%s, %s)", inDomain, inForCell ? "C" : "NC", inType);
    LogToFD(fd, "Answered questions            %10u", totalAnswered);
    LogToFD(fd, "Negatively answered questions %10u", totalNegAnswered);
    LogToFD(fd, "Unanswered questions          %10u", totalUnanswered);
    LogToFD(fd, "Expired - no cached answer    %10u", inHist->expiredAnswerStateBins[ExpiredAnswer_Allowed]);
    LogToFD(fd, "Expired - answered from cache %10u", inHist->expiredAnswerStateBins[ExpiredAnswer_AnsweredWithCache]);
    LogToFD(fd, "Expired - answered expired    %10u", inHist->expiredAnswerStateBins[ExpiredAnswer_AnsweredWithExpired]);
    LogToFD(fd, "Expired - cache changed       %10u", inHist->expiredAnswerStateBins[ExpiredAnswer_ExpiredAnswerChanged]);
    LogToFD(fd, "DNSoTCP - truncated           %10u", inHist->dnsOverTCPStateBins[DNSOverTCP_Truncated]);
    LogToFD(fd, "DNSoTCP - suspicious          %10u", inHist->dnsOverTCPStateBins[DNSOverTCP_Suspicious]);
    LogToFD(fd, "DNSoTCP - suspicious defense  %10u", inHist->dnsOverTCPStateBins[DNSOverTCP_SuspiciousDefense]);
    LogToFD(fd, "-- Query send counts ---------");
    LogDNSHistSendCountsToFD(fd, inHist->answeredQuerySendCountBins);
    LogToFD(fd, "-- Query send counts (NAQs) --");
    LogDNSHistSendCountsToFD(fd, inHist->negAnsweredQuerySendCountBins);

    if (totalAnswered > inHist->answeredQuerySendCountBins[0])
    {
        LogToFD(fd, "--- Response times -----------");
        LogDNSHistLatenciesToFD(fd, inHist->responseLatencyBins);
    }

    if (totalNegAnswered > inHist->negAnsweredQuerySendCountBins[0])
    {
        LogToFD(fd, "--- Response times (NAQs) ----");
        LogDNSHistLatenciesToFD(fd, inHist->negResponseLatencyBins);
    }

    if (totalUnanswered > 0)
    {
        LogToFD(fd, "--- Unanswered query times ---");
        LogDNSHistLatenciesToFD(fd, inHist->unansweredQueryDurationBins);
    }
}

//===========================================================================================================================
//  LogDNSHistSendCountsToFD
//===========================================================================================================================

mDNSlocal void LogDNSHistSendCountsToFD(int fd, const uint16_t inSendCountBins[kQueryStatsSendCountBinCount])
{
    uint32_t        total;
    char            label[16];
    int             i;

    total = 0;
    for (i = 0; i < kQueryStatsSendCountBinCount; ++i)
    {
        total += inSendCountBins[i];
    }

    if (total > 0)
    {
        uint32_t        accumulator = 0;

        for (i = 0; i < kQueryStatsSendCountBinCount; ++i)
        {
            accumulator += inSendCountBins[i];
            if (i < (kQueryStatsSendCountBinCount - 1))
            {
                snprintf(label, sizeof(label), "%2d ", i);
            }
            else
            {
                snprintf(label, sizeof(label), "%2d+", i);
            }
            LogStatToFD(fd, label, inSendCountBins[i], accumulator, total);
            if (accumulator == total) break;
        }
    }
    else
    {
        LogToFD(fd, "No data.");
    }
}

//===========================================================================================================================
//  LogDNSHistLatenciesToFD
//===========================================================================================================================

mDNSlocal void LogDNSHistLatenciesToFD(int fd,
                                         const uint16_t inLatencyBins[kQueryStatsLatencyBinCount])
{
    uint32_t        total;
    int             i;
    char            label[16];

    total = 0;
    for (i = 0; i < kQueryStatsLatencyBinCount; ++i)
    {
        total += inLatencyBins[i];
    }

    if (total > 0)
    {
        uint32_t        accumulator = 0;

        for (i = 0; i < kQueryStatsLatencyBinCount; ++i)
        {
            accumulator += inLatencyBins[i];
            if (i < (int)countof(kResponseLatencyMsLimits))
            {
                snprintf(label, sizeof(label), "< %5u ms", kResponseLatencyMsLimits[i]);
            }
            else
            {
                snprintf(label, sizeof(label), "<     ∞ ms");
            }
            LogStatToFD(fd, label, inLatencyBins[i], accumulator, total);
            if (accumulator == total) break;
        }
    }
    else
    {
        LogToFD(fd, "No data.");
    }
}

//===========================================================================================================================
//  LogDNSMessageSizeStatsToFD
//===========================================================================================================================

mDNSlocal void LogDNSMessageSizeStatsToFD(int fd, const uint32_t *inBins, size_t inBinCount, unsigned int inBinWidth)
{
    size_t          i;
    uint32_t        total;

    total = 0;
    for (i = 0; i < inBinCount; ++i)
    {
        total += inBins[i];
    }

    if (total > 0)
    {
        uint32_t            accumulator;
        unsigned int        lower, upper;
        char                label[16];

        accumulator = 0;
        upper       = 0;
        for (i = 0; i < inBinCount; ++i)
        {
            accumulator += inBins[i];
            lower = upper + 1;
            if (i < (inBinCount - 1))
            {
                upper += inBinWidth;
                snprintf(label, sizeof(label), "%3u - %-3u", lower, upper);
            }
            else
            {
                snprintf(label, sizeof(label), "%3u+     ", lower);
            }
            LogStatToFD(fd, label, inBins[i], accumulator, total);
            if (accumulator == total) break;
        }
    }
    else
    {
        LogToFD(fd, "No data.");
    }
}

#endif  // MDNSRESPONDER_SUPPORTS(APPLE, METRICS)
