/*
 * Copyright (c) 2012-2019 Apple Inc. All rights reserved.
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

// ***************************************************************************
// DNSSECSupport.c: Platform specific support for DNSSEC like fetching root
// trust anchor and dnssec probes etc.
// ***************************************************************************

#include "mDNSEmbeddedAPI.h"
#include "DNSCommon.h"                  // For mDNS_Lock, mDNS_Random
#include "dnssec.h"
#include "DNSSECSupport.h"

#include <CommonCrypto/CommonDigest.h>  // For Hash algorithms SHA1 etc.

// Following are needed for fetching the root trust anchor dynamically
#include <CoreFoundation/CoreFoundation.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <notify.h>

// 30 days
#define ROOT_TA_UPDATE_INTERVAL  (30 * 24 * 3600)   // seconds

// After 100 days, the test anchors are not valid. Just an arbitrary number
// to configure validUntil. 
#define TEST_TA_EXPIRE_INTERVAL  (100 * 24 * 4600)

// When we can't fetch the root TA due to network errors etc., we start off a timer
// to fire at 60 seconds and then keep doubling it till we fetch it
#define InitialTAFetchInterval 60
#define DNSSECProbePercentage 1


#if !TARGET_OS_IPHONE
DNSQuestion DNSSECProbeQuestion;
#endif

mDNSlocal int RegisterNotification(mDNS *const m, unsigned int interval);

mDNSlocal void LinkTrustAnchor(mDNS *const m, TrustAnchor *ta)
{
    int length = 0;
    int i;
    mDNSu8 *p;
    TrustAnchor **t = &m->TrustAnchors;
    char buffer[256];

    while (*t)
        t = &((*t)->next);
    *t = ta;

    buffer[0] = 0;
    p = ta->rds.digest;
    for (i = 0; i < ta->digestLen; i++)
    {
        length += mDNS_snprintf(buffer+length, sizeof(buffer)-1-length, "%x", p[i]);
    }
    LogInfo("LinkTrustAnchor: Zone %##s, keytag %d, alg %d, digestType %d, digestLen %d, digest %s", ta->zone.c, ta->rds.keyTag,
        ta->rds.alg, ta->rds.digestType, ta->digestLen, buffer);
}

mDNSlocal void DelTrustAnchor(mDNS *const m, const domainname *zone)
{
    TrustAnchor **ta = &m->TrustAnchors;
    TrustAnchor *tmp;

    while (*ta && !SameDomainName(&(*ta)->zone, zone))
        ta = &(*ta)->next;

    // First time, we won't find the TrustAnchor in the list as it has
    // not been added.
    if (!(*ta))
        return;

    tmp = *ta;
    *ta = (*ta)->next;                  // Cut this record from the list
    tmp->next = mDNSNULL;
    if (tmp->rds.digest)
        mDNSPlatformMemFree(tmp->rds.digest);
    mDNSPlatformMemFree(tmp);
}

mDNSlocal void AddTrustAnchor(mDNS *const m, const domainname *zone, mDNSu16 keytag, mDNSu8 alg, mDNSu8 digestType, int diglen,
    mDNSu8 *digest)
{
    TrustAnchor *ta, *tmp;
    mDNSu32 t = (mDNSu32) time(NULL); 

    // Check for duplicates
    tmp = m->TrustAnchors;
    while (tmp)
    {
        if (SameDomainName(zone, &tmp->zone) && tmp->rds.keyTag == keytag && tmp->rds.alg == alg && tmp->rds.digestType == digestType &&
            !memcmp(tmp->rds.digest, digest, diglen))
        {
            LogMsg("AddTrustAnchors: Found a duplicate");
            return;
        }
        tmp = tmp->next;
    }

    ta = (TrustAnchor *) mDNSPlatformMemAllocateClear(sizeof(*ta));
    if (!ta)
    {
        LogMsg("AddTrustAnchor: malloc failure ta");
        return;
    }
    ta->rds.keyTag = keytag;
    ta->rds.alg = alg;
    ta->rds.digestType = digestType;
    ta->rds.digest = digest;
    ta->digestLen = diglen;
    ta->validFrom = t;
    ta->validUntil = t + TEST_TA_EXPIRE_INTERVAL;
    AssignDomainName(&ta->zone, zone);
    ta->next = mDNSNULL;

    LinkTrustAnchor(m, ta);
}

#define HexVal(X) ( ((X) >= '0' && (X) <= '9') ? ((X) - '0'     ) :   \
                    ((X) >= 'A' && (X) <= 'F') ? ((X) - 'A' + 10) :   \
                    ((X) >= 'a' && (X) <= 'f') ? ((X) - 'a' + 10) : -1)

mDNSlocal mDNSu8 *ConvertDigest(char *digest, int digestType, int *diglen)
{
    int i, j;
    mDNSu8 *dig;

    switch (digestType)
    {
    case SHA1_DIGEST_TYPE:
        *diglen = CC_SHA1_DIGEST_LENGTH;
        break;
    case SHA256_DIGEST_TYPE:
        *diglen = CC_SHA256_DIGEST_LENGTH;
        break;
    default:
        LogMsg("ConvertDigest: digest type %d not supported", digestType);
        return mDNSNULL;
    }
    dig = (mDNSu8 *) mDNSPlatformMemAllocate(*diglen);
    if (!dig)
    {
        LogMsg("ConvertDigest: malloc failure");
        return mDNSNULL;
    }

    for (j=0,i=0; i<*diglen*2; i+=2)
    {
        int l, h;
        l = HexVal(digest[i]);
        h = HexVal(digest[i+1]);
        if (l<0 || h<0) { LogMsg("ConvertDigest: Cannot convert digest"); mDNSPlatformMemFree(dig); return NULL;}
        dig[j++] = (mDNSu8)((l << 4) | h);
    }
    return dig;
}

// All the children are in a linked list
//
// <TrustAnchor> has two children: <Zone> and <KeyDigest>
// <KeyDigest> has four children <KeyTag> <Algorithm> <DigestType> <Digest>
//
// Returns false if failed to parse the element i.e., malformed xml document.
// Validity of the actual values itself is done outside the function.
mDNSlocal mDNSBool ParseElementChildren(xmlDocPtr tadoc, xmlNode *node, TrustAnchor *ta)
{
    xmlNode *cur_node;
    xmlChar *val1, *val2, *val;
    char *invalid = NULL;

    val = val1 = val2 = NULL;

    for (cur_node = node; cur_node; cur_node = cur_node->next)
    {
        invalid = NULL;
        val1 = val2 = NULL;
        
        val = xmlNodeListGetString(tadoc, cur_node->xmlChildrenNode, 1);
        if (!val)
        {
           LogInfo("ParseElementChildren: NULL value for %s", cur_node->name);
           continue; 
        }
        if (!xmlStrcmp(cur_node->name, (const xmlChar *)"Zone"))
        {
            // MaeDomainNameFromDNSNameString does not work for "."
            if (!xmlStrcmp(val, (const xmlChar *)"."))
            {
                ta->zone.c[0] = 0;
            }
            else if (!MakeDomainNameFromDNSNameString(&ta->zone, (char *)val))
            {
                LogMsg("ParseElementChildren: Cannot parse Zone %s", val);
                goto error;
            }
            else
            {
                LogInfo("ParseElementChildren: Element %s, value %##s", cur_node->name, ta->zone.c);
            }
        }
        else if (!xmlStrcmp(cur_node->name, (const xmlChar *)"KeyTag"))
        {
            ta->rds.keyTag = strtol((const char *)val, &invalid, 10);
            if (*invalid != '\0')
            {
                LogMsg("ParseElementChildren: KeyTag invalid character %d", *invalid);
                goto error;
            }
            else
            {
                LogInfo("ParseElementChildren: Element %s, value %d", cur_node->name, ta->rds.keyTag);
            }
        }
        else if (!xmlStrcmp(cur_node->name, (const xmlChar *)"Algorithm"))
        {
            ta->rds.alg = strtol((const char *)val, &invalid, 10);
            if (*invalid != '\0')
            {
                LogMsg("ParseElementChildren: Algorithm invalid character %c", *invalid);
                goto error;
            }
            else
            {
                LogInfo("ParseElementChildren: Element %s, value %d", cur_node->name, ta->rds.alg);
            }
        }
        else if (!xmlStrcmp(cur_node->name, (const xmlChar *)"DigestType"))
        {
            ta->rds.digestType = strtol((const char *)val, &invalid, 10);
            if (*invalid != '\0')
            {
                LogMsg("ParseElementChildren: Algorithm invalid character %c", *invalid);
                goto error;
            }
            else
            {
                LogInfo("ParseElementChildren: Element %s, value %d", cur_node->name, ta->rds.digestType);
            }
        }
        else if (!xmlStrcmp(cur_node->name, (const xmlChar *)"Digest"))
        {
            int diglen;
            mDNSu8 *dig = ConvertDigest((char *)val, ta->rds.digestType, &diglen);
            if (dig)
            { 
                LogInfo("ParseElementChildren: Element %s, digest %s", cur_node->name, val);
                ta->digestLen = diglen;
                ta->rds.digest = dig;
            }
            else
            {
                LogMsg("ParseElementChildren: Element %s, NULL digest", cur_node->name);
                goto error;
            }
        }
        else if (!xmlStrcmp(cur_node->name, (const xmlChar *)"KeyDigest"))
        {
            struct tm tm;
            val1 = xmlGetProp(cur_node, (const xmlChar *)"validFrom");
            if (val1)
            {
                char *s = strptime((const char *)val1, "%Y-%m-%dT%H:%M:%S", &tm);
                if (!s)
                {
                    LogMsg("ParseElementChildren: Parsing ValidFrom failed %s", val1);
                    goto error;
                }
                else
                {
                    ta->validFrom = (mDNSu32)timegm(&tm);
                }
            }
            val2 = xmlGetProp(cur_node, (const xmlChar *)"validUntil");
            if (val2)
            {
                char *s = strptime((const char *)val2, "%Y-%m-%dT%H:%M:%S", &tm);
                if (!s)
                {
                    LogMsg("ParseElementChildren: Parsing ValidFrom failed %s", val2);
                    goto error;
                }
                else
                {
                    ta->validUntil = (mDNSu32)timegm(&tm);
                }
            }
            else
            {
                // If there is no validUntil, set it to the next probing interval
                mDNSu32 t = (mDNSu32) time(NULL); 
                ta->validUntil = t + ROOT_TA_UPDATE_INTERVAL;
            }
            LogInfo("ParseElementChildren: ValidFrom time %u, validUntil %u", (unsigned)ta->validFrom, (unsigned)ta->validUntil);
        }
        if (val1)
            xmlFree(val1);
        if (val2)
            xmlFree(val2);
        if (val)
            xmlFree(val);
    }
    return mDNStrue;
error:
    if (val1)
        xmlFree(val1);
    if (val2)
        xmlFree(val2);
    if (val)
        xmlFree(val);
    return mDNSfalse;
}

mDNSlocal mDNSBool ValidateTrustAnchor(TrustAnchor *ta)
{
    time_t currTime = time(NULL);

    // Currently only support trust anchor for root.
    if (!SameDomainName(&ta->zone, (const domainname *)"\000"))
    {
        LogInfo("ParseElementChildren: Zone %##s not root", ta->zone.c);
        return mDNSfalse;
    }

    switch (ta->rds.digestType)
    {
    case SHA1_DIGEST_TYPE:
        if (ta->digestLen != CC_SHA1_DIGEST_LENGTH) 
        {
            LogMsg("ValidateTrustAnchor: Invalid digest len %d for SHA1", ta->digestLen);
            return mDNSfalse;
        }
        break;
    case SHA256_DIGEST_TYPE:
        if (ta->digestLen != CC_SHA256_DIGEST_LENGTH) 
        {
            LogMsg("ValidateTrustAnchor: Invalid digest len %d for SHA256", ta->digestLen);
            return mDNSfalse;
        }
        break;
    default:
        LogMsg("ValidateTrustAnchor: digest type %d not supported", ta->rds.digestType);
        return mDNSfalse;
    }
    if (!ta->rds.digest)
    {
        LogMsg("ValidateTrustAnchor: digest NULL for %d", ta->rds.digestType);
        return mDNSfalse;
    }
    switch (ta->rds.alg)
    {
    case CRYPTO_RSA_SHA512:
    case CRYPTO_RSA_SHA256:
    case CRYPTO_RSA_NSEC3_SHA1:
    case CRYPTO_RSA_SHA1:
        break;
    default:
        LogMsg("ValidateTrustAnchor: Algorithm %d not supported", ta->rds.alg);
        return mDNSfalse;
    }
    
    if (DNS_SERIAL_LT(currTime, ta->validFrom))
    {
        LogMsg("ValidateTrustAnchor: Invalid ValidFrom time %u, currtime %u", (unsigned)ta->validFrom, (unsigned)currTime);
        return mDNSfalse;
    }
    if (DNS_SERIAL_LT(ta->validUntil, currTime))
    {
        LogMsg("ValidateTrustAnchor: Invalid ValidUntil time %u, currtime %u", (unsigned)ta->validUntil, (unsigned)currTime);
        return mDNSfalse;
    }
    return mDNStrue;
}

mDNSlocal mDNSBool ParseElement(xmlDocPtr tadoc, xmlNode * a_node, TrustAnchor *ta)
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            // There could be multiple KeyDigests per TrustAnchor. We keep parsing till we
            // reach the last one or we encounter an error in parsing the document.
            if (!xmlStrcmp(cur_node->name, (const xmlChar *)"KeyDigest"))
            {
                if (ta->rds.digest)
                    mDNSPlatformMemFree(ta->rds.digest);
                ta->rds.digestType = 0;
                ta->digestLen = 0;
            }
            if (!ParseElementChildren(tadoc, cur_node->children, ta))
                return mDNSfalse;
            if (!ParseElement(tadoc, cur_node->children, ta))
                return mDNSfalse;
        }
    }
    return mDNStrue;
}

mDNSlocal void TAComplete(mDNS *const m, void *context)
{
    TrustAnchor *ta = (TrustAnchor *)context;

    DelTrustAnchor(m, &ta->zone);
    LinkTrustAnchor(m, ta);
}

mDNSlocal void FetchRootTA(mDNS *const m)
{
    CFStringRef urlString = CFSTR("https://data.iana.org/root-anchors/root-anchors.xml");
    CFDataRef xmlData;
    CFStringRef fileRef = NULL;
    const char *xmlFileName = NULL;
    char buf[512];
    CFURLRef url = NULL;
    static unsigned int RootTAFetchInterval = InitialTAFetchInterval;

    (void) m;

    TrustAnchor *ta = (TrustAnchor *) mDNSPlatformMemAllocateClear(sizeof(*ta));
    if (!ta)
    {
        LogMsg("FetchRootTA: TrustAnchor alloc failed");
        return;
    }

    url = CFURLCreateWithString(NULL, urlString, NULL);
    if (!url)
    {
        LogMsg("FetchRootTA: CFURLCreateWithString error");
        mDNSPlatformMemFree(ta);
        return;
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    // If we can't fetch the XML file e.g., network problems, trigger a timer. All other failures
    // should hardly happen in practice for which schedule the normal interval to refetch the TA.
    Boolean success = CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault, url, &xmlData, NULL, NULL, NULL);
#pragma clang diagnostic pop
    if (!success)
    {
        LogInfo("FetchRootTA: CFURLCreateDataAndPropertiesFromResource error");
        CFRelease(url);
        mDNSPlatformMemFree(ta);
        RegisterNotification(m, RootTAFetchInterval);
        RootTAFetchInterval *= 2 + 1;
        return;
    }

    // get the name of the last component from the url, libxml will use it if
    // it has to report an error
    fileRef = CFURLCopyLastPathComponent(url);
    if (fileRef)
    {
        xmlFileName = CFStringGetCStringPtr(fileRef, kCFStringEncodingUTF8);
        if (!xmlFileName)
        {
            if (!CFStringGetCString(fileRef, buf, sizeof(buf), kCFStringEncodingUTF8) )
                strlcpy(buf, "nofile.xml", sizeof(buf));
            xmlFileName = (const char *)buf;
        }
    }

    // Parse the XML and get the CFXMLTree.
    xmlDocPtr tadoc = xmlReadMemory((const char*)CFDataGetBytePtr(xmlData),
        (int)CFDataGetLength(xmlData), xmlFileName, NULL, 0);        

    if (fileRef)
        CFRelease(fileRef);
    CFRelease(url);
    CFRelease(xmlData);

    if (!tadoc)
    {
        LogMsg("FetchRootTA: xmlReadMemory failed");
        goto done;
    }

    xmlNodePtr root = xmlDocGetRootElement(tadoc);
    if (!root)
    {
        LogMsg("FetchRootTA: Cannot get root element");
        goto done;
    }

    if (ParseElement(tadoc, root, ta) && ValidateTrustAnchor(ta))
    {
        // Do the actual addition of TA on the main queue.
        mDNSPlatformDispatchAsync(m, ta, TAComplete);
    }
    else
    {
        if (ta->rds.digest)
            mDNSPlatformMemFree(ta->rds.digest);
        mDNSPlatformMemFree(ta);
    }
done:
    if (tadoc)
        xmlFreeDoc(tadoc);
    RegisterNotification(m, ROOT_TA_UPDATE_INTERVAL);
    RootTAFetchInterval = InitialTAFetchInterval;
    return;
}


#if APPLE_OSX_mDNSResponder && !TARGET_OS_IPHONE
mDNSlocal void DNSSECProbeCallback(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
{
    if (!AddRecord)
        return;

    mDNS_Lock(m);
    if ((m->timenow - question->StopTime) >= 0)
    {
        mDNS_Unlock(m);
        LogDNSSEC("DNSSECProbeCallback: Question %##s (%s) timed out", question->qname.c, DNSTypeName(question->qtype));
        mDNS_StopQuery(m, question);
        return;
    }
    mDNS_Unlock(m);

    // Wait till we get the DNSSEC results. If we get a negative response e.g., no DNS servers, the
    // question will be restarted by the core and we should have the DNSSEC results eventually.
    if (AddRecord != QC_dnssec)
    {
        LogDNSSEC("DNSSECProbeCallback: Question %##s (%s)", question->qname.c, DNSTypeName(question->qtype), RRDisplayString(m, answer));
        return;
    }

    LogDNSSEC("DNSSECProbeCallback: Question %##s (%s), DNSSEC status %s", question->qname.c, DNSTypeName(question->qtype),
            DNSSECStatusName(question->ValidationStatus));

    mDNS_StopQuery(m, question);
}

// Send a DNSSEC probe just for the sake of collecting DNSSEC statistics.
mDNSexport void DNSSECProbe(mDNS *const m)
{
    mDNSu32 rand;

    if (DNSSECProbeQuestion.ThisQInterval != -1)
        return;
    
    rand = mDNSRandom(FutureTime) % 100;
    // Probe 1% of the time
    if (rand >= DNSSECProbePercentage)
        return;
    
    mDNS_DropLockBeforeCallback();
    InitializeQuestion(m, &DNSSECProbeQuestion, mDNSInterface_Any, (const domainname *)"\003com", kDNSType_DS, DNSSECProbeCallback, mDNSNULL);
    DNSSECProbeQuestion.ValidatingResponse = 0;
    DNSSECProbeQuestion.ValidationRequired = DNSSEC_VALIDATION_SECURE;

    BumpDNSSECStats(m, kStatsActionIncrement, kStatsTypeProbe, 1);
    mDNS_StartQuery(m, &DNSSECProbeQuestion);
    mDNS_ReclaimLockAfterCallback(); 
}
#endif // APPLE_OSX_mDNSResponder && !TARGET_OS_IPHONE

// For now we fetch the root trust anchor and update the local copy
mDNSexport void UpdateTrustAnchors(mDNS *const m)
{
    // Register for a notification to fire immediately which in turn will update
    // the trust anchor
    if (RegisterNotification(m, 1))
    {
        LogMsg("UpdateTrustAnchors: ERROR!! failed to register for notification");
    }
}

mDNSlocal int RegisterNotification(mDNS *const m, unsigned int interval)
{
    int len = strlen("com.apple.system.notify.service.timer:+") + 21; // 21 bytes to accomodate the interval
    char buffer[len];
    unsigned int blen;
    int status;

    // Starting "interval" second from now (+ below indicates relative) register for a notification
    blen = mDNS_snprintf(buffer, sizeof(buffer), "com.apple.system.notify.service.timer:+%us", interval);
    if (blen >= sizeof(buffer))
    {
        LogMsg("RegisterNotification: Buffer too small blen %d, buffer size %d", blen, sizeof(buffer));
        return -1;
    } 
    LogInfo("RegisterNotification: buffer %s", buffer);
    if (m->notifyToken)
    {
        notify_cancel(m->notifyToken);
        m->notifyToken = 0;
    }
    status = notify_register_dispatch(buffer, &m->notifyToken,
                dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0),
                ^(int t) { (void) t; FetchRootTA(m); });

    if (status != NOTIFY_STATUS_OK)
    {
        LogMsg("RegisterNotification: notify_register_dispatch failed");
        return -1;
    }
    return 0;
}

mDNSexport mStatus DNSSECPlatformInit(mDNS *const m)
{
    int diglen;

    m->TrustAnchors = mDNSNULL;
    m->notifyToken  = 0;

    // Add a couple of trust anchors for testing purposes.
    mDNSlocal const domainname *testZone  = (const domainname*)"\007example";

    char *digest = "F122E47B5B7D2B6A5CC0A21EADA11D96BB9CC927";
    mDNSu8 *dig = ConvertDigest(digest, 1, &diglen);
    if (dig) AddTrustAnchor(m, testZone, 23044, 5, 1, diglen, dig);

    char *digest1 = "D795AE5E1AFB200C6139474199B70EAD3F3484553FD97BE5A43704B8A4791F21";
    dig = ConvertDigest(digest1, 2, &diglen);
    if (dig) AddTrustAnchor(m, testZone, 23044, 5, 2, diglen, dig);

    // Add the TA for root zone manually here. We will dynamically fetch the root TA and
    // update it shortly. If that fails e.g., disconnected from the network, we still
    // have something to work with.
    char *digest2 = "49AAC11D7B6F6446702E54A1607371607A1A41855200FD2CE1CDDE32F24E8FB5";
    dig = ConvertDigest(digest2, 2, &diglen);
    if (dig) AddTrustAnchor(m, (const domainname *)"\000", 19036, 8, 2, diglen, dig);

#if !TARGET_OS_IPHONE
    DNSSECProbeQuestion.ThisQInterval = -1;
#endif
    return mStatus_NoError;
}
