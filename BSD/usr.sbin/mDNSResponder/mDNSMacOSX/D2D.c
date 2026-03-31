/*
 * Copyright (c) 2002-2019 Apple Inc. All rights reserved.
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

#include "D2D.h"
#include "mDNSEmbeddedAPI.h"        // Defines the interface provided to the client layer above
#include "DNSCommon.h"
#include "mDNSMacOSX.h"             // Defines the specific types needed to run mDNS on this platform
#include "dns_sd.h"                 // For mDNSInterface_LocalOnly etc.
#include "dns_sd_internal.h"
#include "uds_daemon.h"
#include "BLE.h"

D2DStatus D2DInitialize(CFRunLoopRef runLoop, D2DServiceCallback serviceCallback, void* userData) __attribute__((weak_import));
D2DStatus D2DRetain(D2DServiceInstance instanceHandle, D2DTransportType transportType) __attribute__((weak_import));
D2DStatus D2DStopAdvertisingPairOnTransport(const Byte *key, const size_t keySize, const Byte *value, const size_t valueSize, D2DTransportType transport) __attribute__((weak_import));
D2DStatus D2DRelease(D2DServiceInstance instanceHandle, D2DTransportType transportType) __attribute__((weak_import));
D2DStatus D2DStartAdvertisingPairOnTransport(const Byte *key, const size_t keySize, const Byte *value, const size_t valueSize, D2DTransportType transport) __attribute__((weak_import));
D2DStatus D2DStartBrowsingForKeyOnTransport(const Byte *key, const size_t keySize, D2DTransportType transport) __attribute__((weak_import));
D2DStatus D2DStopBrowsingForKeyOnTransport(const Byte *key, const size_t keySize, D2DTransportType transport) __attribute__((weak_import));
void D2DStartResolvingPairOnTransport(const Byte *key, const size_t keySize, const Byte *value, const size_t valueSize, D2DTransportType transport) __attribute__((weak_import));
void D2DStopResolvingPairOnTransport(const Byte *key, const size_t keySize, const Byte *value, const size_t valueSize, D2DTransportType transport) __attribute__((weak_import));
D2DStatus D2DTerminate(void) __attribute__((weak_import));

#pragma mark - D2D Support

mDNSexport void D2D_start_advertising_interface(NetworkInterfaceInfo *interface)
{
    // AWDL wants the address and reverse address PTR record communicated
    // via the D2D interface layer.
    if (interface->InterfaceID == AWDLInterfaceID)
    {
        // only log if we have a valid record to start advertising
        if (interface->RR_A.resrec.RecordType || interface->RR_PTR.resrec.RecordType)
            LogInfo("D2D_start_advertising_interface: %s", interface->ifname);

        if (interface->RR_A.resrec.RecordType)
            external_start_advertising_service(&interface->RR_A.resrec, 0);
        if (interface->RR_PTR.resrec.RecordType)
            external_start_advertising_service(&interface->RR_PTR.resrec, 0);
    }
}

mDNSexport void D2D_stop_advertising_interface(NetworkInterfaceInfo *interface)
{
    if (interface->InterfaceID == AWDLInterfaceID)
    {
        // only log if we have a valid record to stop advertising
        if (interface->RR_A.resrec.RecordType || interface->RR_PTR.resrec.RecordType)
            LogInfo("D2D_stop_advertising_interface: %s", interface->ifname);

        if (interface->RR_A.resrec.RecordType)
            external_stop_advertising_service(&interface->RR_A.resrec, 0);
        if (interface->RR_PTR.resrec.RecordType)
            external_stop_advertising_service(&interface->RR_PTR.resrec, 0);
    }
}

// If record would have been advertised to the D2D plugin layer, stop that advertisement.
mDNSexport void D2D_stop_advertising_record(AuthRecord *ar)
{
    DNSServiceFlags flags = deriveD2DFlagsFromAuthRecType(ar->ARType);
    if (callExternalHelpers(ar->resrec.InterfaceID, ar->resrec.name, flags))
    {
        external_stop_advertising_service(&ar->resrec, flags);
    }
}

// If record should be advertised to the D2D plugin layer, start that advertisement.
mDNSexport void D2D_start_advertising_record(AuthRecord *ar)
{
    DNSServiceFlags flags = deriveD2DFlagsFromAuthRecType(ar->ARType);
    if (callExternalHelpers(ar->resrec.InterfaceID, ar->resrec.name, flags))
    {
        external_start_advertising_service(&ar->resrec, flags);
    }
}

// Name compression items for fake packet version number 1
static const mDNSu8 compression_packet_v1 = 0x01;

static DNSMessage compression_base_msg = { { {{0}}, {{0}}, 2, 0, 0, 0 }, "\x04_tcp\x05local\x00\x00\x0C\x00\x01\x04_udp\xC0\x11\x00\x0C\x00\x01" };
static mDNSu8 *const compression_limit = (mDNSu8 *) &compression_base_msg + sizeof(DNSMessage);
static mDNSu8 *const compression_lhs = (mDNSu8 *const) compression_base_msg.data + 27;

mDNSlocal void FreeD2DARElemCallback(mDNS *const m, AuthRecord *const rr, mStatus result);

typedef struct D2DRecordListElem
{
    struct D2DRecordListElem *next;
    D2DServiceInstance       instanceHandle;
    D2DTransportType         transportType;
    AuthRecord               ar;    // must be last in the structure to accomodate extra space
                                    // allocated for large records.
} D2DRecordListElem;

static D2DRecordListElem *D2DRecords = NULL; // List of records returned with D2DServiceFound events

typedef struct D2DBrowseListElem
{
    struct D2DBrowseListElem *next;
    domainname name;
    mDNSu16 type;
    unsigned int refCount;
} D2DBrowseListElem;

D2DBrowseListElem* D2DBrowseList = NULL;

mDNSlocal mDNSu8 *putVal16(mDNSu8 *ptr, mDNSu16 val)
{
    ptr[0] = (mDNSu8)((val >> 8 ) & 0xFF);
    ptr[1] = (mDNSu8)((val      ) & 0xFF);
    return ptr + sizeof(mDNSu16);
}

mDNSlocal mDNSu8 *putVal32(mDNSu8 *ptr, mDNSu32 val)
{
    ptr[0] = (mDNSu8)((val >> 24) & 0xFF);
    ptr[1] = (mDNSu8)((val >> 16) & 0xFF);
    ptr[2] = (mDNSu8)((val >>  8) & 0xFF);
    ptr[3] = (mDNSu8)((val      ) & 0xFF);
    return ptr + sizeof(mDNSu32);
}

mDNSlocal void DomainnameToLower(const domainname * const in, domainname * const out)
{
    const mDNSu8 * const start = (const mDNSu8 * const)in;
    mDNSu8 *ptr = (mDNSu8*)start;
    while(*ptr)
    {
        mDNSu8 c = *ptr;
        out->c[ptr-start] = *ptr;
        ptr++;
        for (; c; c--,ptr++) out->c[ptr-start] = mDNSIsUpperCase(*ptr) ? (*ptr - 'A' + 'a') : *ptr;
    }
    out->c[ptr-start] = *ptr;
}

mDNSlocal mDNSu8 * DNSNameCompressionBuildLHS(const domainname* typeDomain, DNS_TypeValues qtype)
{
    mDNSu8 *ptr = putDomainNameAsLabels(&compression_base_msg, compression_lhs, compression_limit, typeDomain);
    if (!ptr) return ptr;
    *ptr = (qtype >> 8) & 0xff;
    ptr += 1;
    *ptr = qtype & 0xff;
    ptr += 1;
    *ptr = compression_packet_v1;
    return ptr + 1;
}

mDNSlocal mDNSu8 * DNSNameCompressionBuildRHS(mDNSu8 *start, const ResourceRecord *const resourceRecord)
{
    return putRData(&compression_base_msg, start, compression_limit, resourceRecord);
}

mDNSlocal void PrintHelper(const char *const tag, mDNSu8 *lhs, mDNSu16 lhs_len, mDNSu8 *rhs, mDNSu16 rhs_len)
{
    if (mDNS_LoggingEnabled)
    {
        LogDebug("%s: LHS: (%d bytes) %.*H", tag, lhs_len, lhs_len, lhs);
        if (rhs) LogDebug("%s: RHS: (%d bytes) %.*H", tag, rhs_len, rhs_len, rhs);
    }
}

mDNSlocal void FreeD2DARElemCallback(mDNS *const m, AuthRecord *const rr, mStatus result)
{
    (void)m;  // unused
    if (result == mStatus_MemFree)
    {
        D2DRecordListElem **ptr = &D2DRecords;
        D2DRecordListElem *tmp;
        while (*ptr && &(*ptr)->ar != rr) ptr = &(*ptr)->next;
        if (!*ptr) { LogMsg("FreeD2DARElemCallback: Could not find in D2DRecords: %s", ARDisplayString(m, rr)); return; }
        LogInfo("FreeD2DARElemCallback: Found in D2DRecords: %s", ARDisplayString(m, rr));
        tmp = *ptr;
        *ptr = (*ptr)->next;
        // Just because we stoppped browsing, doesn't mean we should tear down the PAN connection.
        mDNSPlatformMemFree(tmp);
    }
}

mDNSexport void external_connection_release(const domainname *instance)
{
    (void) instance;
    D2DRecordListElem *ptr = D2DRecords;

    for ( ; ptr ; ptr = ptr->next)
    {
        if ((ptr->ar.resrec.rrtype == kDNSServiceType_PTR) &&
             SameDomainName(&ptr->ar.rdatastorage.u.name, instance))
        {
            LogInfo("external_connection_release: Calling D2DRelease(instanceHandle = %p, transportType = %d", 
                ptr->instanceHandle,  ptr->transportType);
            if (D2DRelease) D2DRelease(ptr->instanceHandle, ptr->transportType);
        }
    }
}

mDNSlocal void xD2DClearCache(const domainname *regType, DNS_TypeValues qtype)
{
    D2DRecordListElem *ptr = D2DRecords;
    for ( ; ptr ; ptr = ptr->next)
    {
        if ((ptr->ar.resrec.rrtype == qtype) && SameDomainName(&ptr->ar.namestorage, regType))
        {
            LogInfo("xD2DClearCache: Clearing cache record and deregistering %s", ARDisplayString(&mDNSStorage, &ptr->ar));
            mDNS_Deregister(&mDNSStorage, &ptr->ar);
        }
    }
}

mDNSlocal D2DBrowseListElem ** D2DFindInBrowseList(const domainname *const name, mDNSu16 type)
{
    D2DBrowseListElem **ptr = &D2DBrowseList;

    for ( ; *ptr; ptr = &(*ptr)->next)
        if ((*ptr)->type == type && SameDomainName(&(*ptr)->name, name))
            break;

    return ptr;
}

mDNSlocal unsigned int D2DBrowseListRefCount(const domainname *const name, mDNSu16 type)
{
    D2DBrowseListElem **ptr = D2DFindInBrowseList(name, type);
    return *ptr ? (*ptr)->refCount : 0;
}

mDNSlocal void D2DBrowseListRetain(const domainname *const name, mDNSu16 type)
{
    D2DBrowseListElem **ptr = D2DFindInBrowseList(name, type);

    if (!*ptr)
    {
        *ptr = (D2DBrowseListElem *) mDNSPlatformMemAllocateClear(sizeof(**ptr));
        (*ptr)->type = type;
        AssignDomainName(&(*ptr)->name, name);
    }
    (*ptr)->refCount += 1;

    LogInfo("D2DBrowseListRetain: %##s %s refcount now %u", (*ptr)->name.c, DNSTypeName((*ptr)->type), (*ptr)->refCount);
}

// Returns true if found in list, false otherwise
mDNSlocal bool D2DBrowseListRelease(const domainname *const name, mDNSu16 type)
{
    D2DBrowseListElem **ptr = D2DFindInBrowseList(name, type);

    if (!*ptr) { LogMsg("D2DBrowseListRelease: Didn't find %##s %s in list", name->c, DNSTypeName(type)); return false; }

    (*ptr)->refCount -= 1;

    LogInfo("D2DBrowseListRelease: %##s %s refcount now %u", (*ptr)->name.c, DNSTypeName((*ptr)->type), (*ptr)->refCount);

    if (!(*ptr)->refCount)
    {
        D2DBrowseListElem *tmp = *ptr;
        *ptr = (*ptr)->next;
        mDNSPlatformMemFree(tmp);
    }
    return true;
}

mDNSlocal mStatus xD2DParse(const mDNSu8 * const lhs, const mDNSu16 lhs_len, const mDNSu8 * const rhs, const mDNSu16 rhs_len, D2DRecordListElem **D2DListp)
{
    mDNS *const m = &mDNSStorage;

	// Sanity check that key array (lhs) has one domain name, followed by the record type and single byte D2D
    // plugin protocol version number.
    // Note, we don't have a DNSMessage pointer at this point, so just pass in the lhs value as the lower bound
    // of the input bytes we are processing.  skipDomainName() does not try to follow name compression pointers,
    // so it is safe to pass it the key byte array since it will stop parsing the DNS name and return a pointer
    // to the byte after the first name compression pointer it encounters.
    const mDNSu8 *keyp = skipDomainName((const DNSMessage *const) lhs, lhs, lhs + lhs_len);

    // There should be 3 bytes remaining in a valid key,
    // two for the DNS record type, and one for the D2D protocol version number.
    if (keyp == NULL || (keyp + 3 != (lhs + lhs_len)))
    {
        LogInfo("xD2DParse: Could not parse DNS name in key");
        return mStatus_Incompatible;
    }
    keyp += 2;   // point to D2D compression packet format version byte
    if (*keyp != compression_packet_v1)
    {
        LogInfo("xD2DParse: Invalid D2D packet version: %d", *keyp);
        return mStatus_Incompatible;
    }

    if (mDNS_LoggingEnabled)
    {
        const int len = (int)(compression_lhs - (mDNSu8*)&compression_base_msg);
        LogInfo("xD2DParse: Static Bytes: (%d bytes) %.*H", len, len, &compression_base_msg);
    }

    mDNSu8 *ptr = compression_lhs; // pointer to the end of our fake packet

    // Check to make sure we're not going to go past the end of the DNSMessage data
    // 7 = 2 for CLASS (-1 for our version) + 4 for TTL + 2 for RDLENGTH
    if (ptr + lhs_len - 7 + rhs_len >= compression_limit) return mStatus_NoMemoryErr;

    // Copy the LHS onto our fake wire packet
    mDNSPlatformMemCopy(ptr, lhs, lhs_len);
    ptr += lhs_len - 1;

    // Check the 'fake packet' version number, to ensure that we know how to decompress this data
    if (*ptr != compression_packet_v1) return mStatus_Incompatible;

    // two bytes of CLASS
    ptr = putVal16(ptr, kDNSClass_IN | kDNSClass_UniqueRRSet);

    // four bytes of TTL
    ptr = putVal32(ptr, 120);

    // Copy the RHS length into the RDLENGTH of our fake wire packet
    ptr = putVal16(ptr, rhs_len);

    // Copy the RHS onto our fake wire packet
    mDNSPlatformMemCopy(ptr, rhs, rhs_len);
    ptr += rhs_len;

    if (mDNS_LoggingEnabled)
    {
        const int len = (int)(ptr - compression_lhs);
        LogInfo("xD2DParse: Our Bytes (%d bytes): %.*H", len, len, compression_lhs);
    }

    ptr = (mDNSu8 *) GetLargeResourceRecord(m, &compression_base_msg, compression_lhs, ptr, mDNSInterface_Any, kDNSRecordTypePacketAns, &m->rec);
    if (!ptr || m->rec.r.resrec.RecordType == kDNSRecordTypePacketNegative)
    {
        LogMsg("xD2DParse: failed to get large RR");
        m->rec.r.resrec.RecordType = 0;
        return mStatus_UnknownErr;
    }
    else
    {
        LogInfo("xD2DParse: got rr: %s", CRDisplayString(m, &m->rec.r));
    }

    *D2DListp = (D2DRecordListElem *) mDNSPlatformMemAllocateClear(sizeof(D2DRecordListElem) + (m->rec.r.resrec.rdlength <= sizeof(RDataBody) ? 0 : m->rec.r.resrec.rdlength - sizeof(RDataBody)));
    if (!*D2DListp) return mStatus_NoMemoryErr;

    AuthRecord *rr = &(*D2DListp)->ar;
    mDNS_SetupResourceRecord(rr, mDNSNULL, mDNSInterface_P2P, m->rec.r.resrec.rrtype, 7200, kDNSRecordTypeShared, AuthRecordP2P, FreeD2DARElemCallback, NULL);
    AssignDomainName(&rr->namestorage, &m->rec.namestorage);
    rr->resrec.rdlength = m->rec.r.resrec.rdlength;
    rr->resrec.rdata->MaxRDLength = m->rec.r.resrec.rdlength;
    mDNSPlatformMemCopy(rr->resrec.rdata->u.data, m->rec.r.resrec.rdata->u.data, m->rec.r.resrec.rdlength);
    rr->resrec.namehash = DomainNameHashValue(rr->resrec.name);
    SetNewRData(&rr->resrec, mDNSNULL, 0);  // Sets rr->rdatahash for us

    m->rec.r.resrec.RecordType = 0; // Mark m->rec as no longer in use

    return mStatus_NoError;
}

mDNSexport void xD2DAddToCache(D2DStatus result, D2DServiceInstance instanceHandle, D2DTransportType transportType, const Byte *key, size_t keySize, const Byte *value, size_t valueSize)
{
    mDNS *const m = &mDNSStorage;
	if (result == kD2DSuccess)
    {
        if ( key == NULL || value == NULL || keySize == 0 || valueSize == 0) { LogMsg("xD2DAddToCache: NULL Byte * passed in or length == 0"); return; }

        mStatus err;
        D2DRecordListElem *ptr = NULL;

        err = xD2DParse((const mDNSu8 * const)key, (const mDNSu16)keySize, (const mDNSu8 * const)value, (const mDNSu16)valueSize, &ptr);
        if (err)
        {
            LogMsg("xD2DAddToCache: xD2DParse returned error: %d", err);
            PrintHelper(__func__, (mDNSu8 *)key, (mDNSu16)keySize, (mDNSu8 *)value, (mDNSu16)valueSize);
            if (ptr)
                mDNSPlatformMemFree(ptr);
            return;
        }

#if ENABLE_BLE_TRIGGERED_BONJOUR
        // If the record was created based on a BLE beacon, update the interface index to indicate
        // this and thus match BLE specific queries.
        if (transportType == D2DBLETransport)
            ptr->ar.resrec.InterfaceID = mDNSInterface_BLE;
#endif // ENABLE_BLE_TRIGGERED_BONJOUR

        err = mDNS_Register(m, &ptr->ar);
        if (err)
        {
            LogMsg("xD2DAddToCache: mDNS_Register returned error %d for %s", err, ARDisplayString(m, &ptr->ar));
            mDNSPlatformMemFree(ptr);
            return;
        }

        LogInfo("xD2DAddToCache: mDNS_Register succeeded for %s", ARDisplayString(m, &ptr->ar));
        ptr->instanceHandle = instanceHandle;
        ptr->transportType = transportType;
        ptr->next = D2DRecords;
        D2DRecords = ptr;
    }
    else
        LogMsg("xD2DAddToCache: Unexpected result %d", result);
}

mDNSlocal D2DRecordListElem * xD2DFindInList(const Byte *const key, const size_t keySize, const Byte *const value, const size_t valueSize)
{
    D2DRecordListElem *ptr = D2DRecords;
    D2DRecordListElem *arptr = NULL;

    if ( key == NULL || value == NULL || keySize == 0 || valueSize == 0) { LogMsg("xD2DFindInList: NULL Byte * passed in or length == 0"); return NULL; }

    mStatus err = xD2DParse((const mDNSu8 *const)key, (const mDNSu16)keySize, (const mDNSu8 *const)value, (const mDNSu16)valueSize, &arptr);
    if (err)
    {
        LogMsg("xD2DFindInList: xD2DParse returned error: %d", err);
        PrintHelper(__func__, (mDNSu8 *)key, (mDNSu16)keySize, (mDNSu8 *)value, (mDNSu16)valueSize);
        if (arptr)
            mDNSPlatformMemFree(arptr);
        return NULL;
    }

    while (ptr)
    {
        if (IdenticalResourceRecord(&arptr->ar.resrec, &ptr->ar.resrec)) break;
        ptr = ptr->next;
    }

    if (!ptr) LogMsg("xD2DFindInList: Could not find in D2DRecords: %s", ARDisplayString(&mDNSStorage, &arptr->ar));
    mDNSPlatformMemFree(arptr);
    return ptr;
}

mDNSexport void xD2DRemoveFromCache(D2DStatus result, D2DServiceInstance instanceHandle, D2DTransportType transportType, const Byte *key, size_t keySize, const Byte *value, size_t valueSize)
{
    (void)transportType; // We don't care about this, yet.
    (void)instanceHandle; // We don't care about this, yet.

    if (result == kD2DSuccess)
    {
        D2DRecordListElem *ptr = xD2DFindInList(key, keySize, value, valueSize);
        if (ptr)
        {
            LogInfo("xD2DRemoveFromCache: Remove from cache: %s", ARDisplayString(&mDNSStorage, &ptr->ar));
            mDNS_Deregister(&mDNSStorage, &ptr->ar);
        }
    }
    else
        LogMsg("xD2DRemoveFromCache: Unexpected result %d", result);
}

mDNSlocal void xD2DServiceResolved(D2DStatus result, D2DServiceInstance instanceHandle, D2DTransportType transportType, const Byte *key, size_t keySize, const Byte *value, size_t valueSize)
{
    (void)key;
    (void)keySize;
    (void)value;
    (void)valueSize;

    if (result == kD2DSuccess)
    {
        LogInfo("xD2DServiceResolved: Starting up PAN connection for %p", instanceHandle);
        if (D2DRetain) D2DRetain(instanceHandle, transportType);
    }
    else LogMsg("xD2DServiceResolved: Unexpected result %d", result);
}

mDNSlocal void xD2DRetainHappened(D2DStatus result, D2DServiceInstance instanceHandle, D2DTransportType transportType, const Byte *key, size_t keySize, const Byte *value, size_t valueSize)
{
    (void)instanceHandle;
    (void)transportType;
    (void)key;
    (void)keySize;
    (void)value;
    (void)valueSize;

    if (result == kD2DSuccess) LogInfo("xD2DRetainHappened: Opening up PAN connection for %p", instanceHandle);
    else LogMsg("xD2DRetainHappened: Unexpected result %d", result);
}

mDNSlocal void xD2DReleaseHappened(D2DStatus result, D2DServiceInstance instanceHandle, D2DTransportType transportType, const Byte *key, size_t keySize, const Byte *value, size_t valueSize)
{
    (void)instanceHandle;
    (void)transportType;
    (void)key;
    (void)keySize;
    (void)value;
    (void)valueSize;

    if (result == kD2DSuccess) LogInfo("xD2DReleaseHappened: Closing PAN connection for %p", instanceHandle);
    else LogMsg("xD2DReleaseHappened: Unexpected result %d", result);
}

mDNSlocal void xD2DServiceCallback(D2DServiceEvent event, D2DStatus result, D2DServiceInstance instanceHandle, D2DTransportType transportType, const Byte *key, size_t keySize, const Byte *value, size_t valueSize, void *userData)
{
    const char *eventString = "unknown";

    KQueueLock();

    if (keySize   > 0xFFFF) LogMsg("xD2DServiceCallback: keySize too large: %u", keySize);
    if (valueSize > 0xFFFF) LogMsg("xD2DServiceCallback: valueSize too large: %u", valueSize);

    switch (event)
    {
    case D2DServiceFound:
        eventString = "D2DServiceFound";
        break;
    case D2DServiceLost:
        eventString = "D2DServiceLost";
        break;
    case D2DServiceResolved:
        eventString = "D2DServiceResolved";
        break;
    case D2DServiceRetained:
        eventString = "D2DServiceRetained";
        break;
    case D2DServiceReleased:
        eventString = "D2DServiceReleased";
        break;
    default:
        break;
    }

    LogInfo("xD2DServiceCallback: event=%s result=%d instanceHandle=%p transportType=%d LHS=%p (%u) RHS=%p (%u) userData=%p", eventString, result, instanceHandle, transportType, key, keySize, value, valueSize, userData);
    PrintHelper(__func__, (mDNSu8 *)key, (mDNSu16)keySize, (mDNSu8 *)value, (mDNSu16)valueSize);

    switch (event)
    {
    case D2DServiceFound:
        xD2DAddToCache(result, instanceHandle, transportType, key, keySize, value, valueSize);
        break;
    case D2DServiceLost:
        xD2DRemoveFromCache(result, instanceHandle, transportType, key, keySize, value, valueSize);
        break;
    case D2DServiceResolved:
        xD2DServiceResolved(result, instanceHandle, transportType, key, keySize, value, valueSize);
        break;
    case D2DServiceRetained:
        xD2DRetainHappened(result, instanceHandle, transportType, key, keySize, value, valueSize);
        break;
    case D2DServiceReleased:
        xD2DReleaseHappened(result, instanceHandle, transportType, key, keySize, value, valueSize);
        break;
    default:
        break;
    }

    // Need to tickle the main kqueue loop to potentially handle records we removed or added.
    KQueueUnlock("xD2DServiceCallback");
}

// Map interface index and flags to a specific D2D transport type or D2DTransportMax if all plugins 
// should be called.
// When D2DTransportMax is returned, if a specific transport should not be called, *excludedTransportType 
// will be set to the excluded transport value, otherwise, it will be set to D2DTransportMax.
// If the return value is not D2DTransportMax, excludedTransportType is undefined.

mDNSlocal D2DTransportType xD2DInterfaceToTransportType(mDNSInterfaceID InterfaceID, DNSServiceFlags flags, D2DTransportType * excludedTransportType)
{
    NetworkInterfaceInfoOSX *info;

    // Default exludes the D2DAWDLTransport when D2DTransportMax is returned.
    *excludedTransportType = D2DAWDLTransport;

    // Call all D2D plugins when both kDNSServiceFlagsIncludeP2P and kDNSServiceFlagsIncludeAWDL are set.
    if ((flags & kDNSServiceFlagsIncludeP2P) && (flags & kDNSServiceFlagsIncludeAWDL))
    {
        LogInfo("xD2DInterfaceToTransportType: returning D2DTransportMax (including AWDL) since both kDNSServiceFlagsIncludeP2P and kDNSServiceFlagsIncludeAWDL are set");
        *excludedTransportType = D2DTransportMax;
        return D2DTransportMax;
    } 
    // Call all D2D plugins (exlcluding AWDL) when only kDNSServiceFlagsIncludeP2P is set.
    else if (flags & kDNSServiceFlagsIncludeP2P)
    {
        LogInfo("xD2DInterfaceToTransportType: returning D2DTransportMax (excluding AWDL) since only kDNSServiceFlagsIncludeP2P is set");
        return D2DTransportMax;
    }
    // Call AWDL D2D plugin when only kDNSServiceFlagsIncludeAWDL is set.
    else if (flags & kDNSServiceFlagsIncludeAWDL)
    {
        LogInfo("xD2DInterfaceToTransportType: returning D2DAWDLTransport since only kDNSServiceFlagsIncludeAWDL is set");
        return D2DAWDLTransport;
    }

    if (InterfaceID == mDNSInterface_P2P)
    {
        LogInfo("xD2DInterfaceToTransportType: returning D2DTransportMax (excluding AWDL) for interface index mDNSInterface_P2P");
        return D2DTransportMax; 
    }

    // Compare to cached AWDL interface ID.
    if (AWDLInterfaceID && (InterfaceID == AWDLInterfaceID))
    {
        LogInfo("xD2DInterfaceToTransportType: returning D2DAWDLTransport for interface index %d", InterfaceID);
        return D2DAWDLTransport;
    }

    info = IfindexToInterfaceInfoOSX(InterfaceID);
    if (info == NULL)
    {
        LogInfo("xD2DInterfaceToTransportType: Invalid interface index %d", InterfaceID);
        return D2DTransportMax;
    }

    // Recognize AirDrop specific p2p* interface based on interface name.
    if (strncmp(info->ifinfo.ifname, "p2p", 3) == 0)
    {
        LogInfo("xD2DInterfaceToTransportType: returning D2DWifiPeerToPeerTransport for interface index %d", InterfaceID);
        return D2DWifiPeerToPeerTransport;
    }

    // Currently there is no way to identify Bluetooth interface by name,
    // since they use "en*" based name strings.

    LogInfo("xD2DInterfaceToTransportType: returning default D2DTransportMax for interface index %d", InterfaceID);
    return D2DTransportMax;
}

mDNSexport void external_start_browsing_for_service(mDNSInterfaceID InterfaceID, const domainname *const typeDomain, DNS_TypeValues qtype, DNSServiceFlags flags)
{
#if ENABLE_BLE_TRIGGERED_BONJOUR
    // BLE support currently not handled by a D2D plugin
    if (applyToBLE(InterfaceID, flags))
    {
        domainname lower;

        DomainnameToLower(typeDomain, &lower);
        // pass in the key and keySize
        mDNSu8 *end = DNSNameCompressionBuildLHS(&lower, qtype);
        start_BLE_browse(InterfaceID, &lower, qtype, flags, compression_lhs, end - compression_lhs);
    }
    else
#endif  // ENABLE_BLE_TRIGGERED_BONJOUR
        internal_start_browsing_for_service(InterfaceID, typeDomain, qtype, flags);
}

mDNSexport void internal_start_browsing_for_service(mDNSInterfaceID InterfaceID, const domainname *const typeDomain, DNS_TypeValues qtype, DNSServiceFlags flags)
{
    domainname lower;

    DomainnameToLower(typeDomain, &lower);

    if (!D2DBrowseListRefCount(&lower, qtype))
    {
        D2DTransportType transportType, excludedTransport;

        LogInfo("%s: Starting browse for: %##s %s", __func__, lower.c, DNSTypeName(qtype));
        mDNSu8 *end = DNSNameCompressionBuildLHS(&lower, qtype);
        PrintHelper(__func__, compression_lhs, end - compression_lhs, mDNSNULL, 0);

        transportType = xD2DInterfaceToTransportType(InterfaceID, flags, & excludedTransport);
        if (transportType == D2DTransportMax)
        {
            D2DTransportType i;
            for (i = 0; i < D2DTransportMax; i++)
            {
                if (i == excludedTransport) continue;
                if (D2DStartBrowsingForKeyOnTransport) D2DStartBrowsingForKeyOnTransport(compression_lhs, end - compression_lhs, i);
            }
        }
        else
        {
            if (D2DStartBrowsingForKeyOnTransport) D2DStartBrowsingForKeyOnTransport(compression_lhs, end - compression_lhs, transportType);
        }
    }
    D2DBrowseListRetain(&lower, qtype);
}

mDNSexport void external_stop_browsing_for_service(mDNSInterfaceID InterfaceID, const domainname *const typeDomain, DNS_TypeValues qtype, DNSServiceFlags flags)
{
#if ENABLE_BLE_TRIGGERED_BONJOUR
    // BLE support currently not handled by a D2D plugin
    if (applyToBLE(InterfaceID, flags))
    {
        domainname lower;

        // If this is the last instance of this browse, clear any cached records recieved for it.
        // We are not guaranteed to get a D2DServiceLost event for all key, value pairs cached over BLE.
        DomainnameToLower(typeDomain, &lower);
        if (stop_BLE_browse(InterfaceID, &lower, qtype, flags))
            xD2DClearCache(&lower, qtype);
    }
    else
#endif  // ENABLE_BLE_TRIGGERED_BONJOUR
        internal_stop_browsing_for_service(InterfaceID, typeDomain, qtype, flags);
}

mDNSexport void internal_stop_browsing_for_service(mDNSInterfaceID InterfaceID, const domainname *const typeDomain, DNS_TypeValues qtype, DNSServiceFlags flags)
{
    domainname lower;

    DomainnameToLower(typeDomain, &lower);

    // If found in list and this is the last reference to this browse, remove the key from the D2D plugins.
    if (D2DBrowseListRelease(&lower, qtype) && !D2DBrowseListRefCount(&lower, qtype))
    {
        D2DTransportType transportType, excludedTransport;

        LogInfo("%s: Stopping browse for: %##s %s", __func__, lower.c, DNSTypeName(qtype));
        mDNSu8 *end = DNSNameCompressionBuildLHS(&lower, qtype);
        PrintHelper(__func__, compression_lhs, end - compression_lhs, mDNSNULL, 0);

        transportType = xD2DInterfaceToTransportType(InterfaceID, flags, & excludedTransport);
        if (transportType == D2DTransportMax)
        {
            D2DTransportType i;
            for (i = 0; i < D2DTransportMax; i++)
            {
                if (i == excludedTransport) continue;
                if (D2DStopBrowsingForKeyOnTransport) D2DStopBrowsingForKeyOnTransport(compression_lhs, end - compression_lhs, i);
            }
        }
        else
        {
            if (D2DStopBrowsingForKeyOnTransport) D2DStopBrowsingForKeyOnTransport(compression_lhs, end - compression_lhs, transportType);
        }

        // The D2D driver may not generate the D2DServiceLost event for this key after
        // the D2DStopBrowsingForKey*() call above.  So, we flush the key from the D2D 
        // record cache now.
        xD2DClearCache(&lower, qtype);
    }
}

mDNSexport void external_start_advertising_service(const ResourceRecord *const resourceRecord, DNSServiceFlags flags)
{
#if ENABLE_BLE_TRIGGERED_BONJOUR
    if (applyToBLE(resourceRecord->InterfaceID, flags))
    {
        domainname lower;

        DomainnameToLower(resourceRecord->name, &lower);
        start_BLE_advertise(resourceRecord, &lower, resourceRecord->rrtype, flags);
    }
    else
#endif  // ENABLE_BLE_TRIGGERED_BONJOUR
        internal_start_advertising_service(resourceRecord, flags);
}

mDNSexport void internal_start_advertising_service(const ResourceRecord *const resourceRecord, DNSServiceFlags flags)
{
    domainname lower;
    mDNSu8 *rhs = NULL;
    mDNSu8 *end = NULL;
    D2DTransportType transportType, excludedTransport;
    DomainnameToLower(resourceRecord->name, &lower);

    LogInfo("%s: %s", __func__, RRDisplayString(&mDNSStorage, resourceRecord));

    // For SRV records, update packet filter if p2p interface already exists, otherwise,
    // if will be updated when we get the KEV_DL_IF_ATTACHED event for the interface.
    if (resourceRecord->rrtype == kDNSType_SRV)
        mDNSUpdatePacketFilter(NULL);

    rhs = DNSNameCompressionBuildLHS(&lower, resourceRecord->rrtype);
    end = DNSNameCompressionBuildRHS(rhs, resourceRecord);
    PrintHelper(__func__, compression_lhs, rhs - compression_lhs, rhs, end - rhs);

    transportType = xD2DInterfaceToTransportType(resourceRecord->InterfaceID, flags, & excludedTransport);
    if (transportType == D2DTransportMax)
    {
        D2DTransportType i;
        for (i = 0; i < D2DTransportMax; i++)
        {
            if (i == excludedTransport) continue;
            if (D2DStartAdvertisingPairOnTransport) D2DStartAdvertisingPairOnTransport(compression_lhs, rhs - compression_lhs, rhs, end - rhs, i);
        }
    }
    else
    {
        if (D2DStartAdvertisingPairOnTransport) D2DStartAdvertisingPairOnTransport(compression_lhs, rhs - compression_lhs, rhs, end - rhs, transportType);
    }
}

mDNSexport void external_stop_advertising_service(const ResourceRecord *const resourceRecord, DNSServiceFlags flags)
{
#if ENABLE_BLE_TRIGGERED_BONJOUR
    // BLE support currently not handled by a D2D plugin
    if (applyToBLE(resourceRecord->InterfaceID, flags))
    {
        domainname lower;

        DomainnameToLower(resourceRecord->name, &lower);
        stop_BLE_advertise(&lower, resourceRecord->rrtype, flags);
    }
    else
#endif  // ENABLE_BLE_TRIGGERED_BONJOUR
        internal_stop_advertising_service(resourceRecord, flags);
}

mDNSexport void internal_stop_advertising_service(const ResourceRecord *const resourceRecord, DNSServiceFlags flags)
{
    domainname lower;
    mDNSu8 *rhs = NULL;
    mDNSu8 *end = NULL;
    D2DTransportType transportType, excludedTransport;
    DomainnameToLower(resourceRecord->name, &lower);

    LogInfo("%s: %s", __func__, RRDisplayString(&mDNSStorage, resourceRecord));

    // For SRV records, update packet filter if p2p interface already exists, otherwise,
    // For SRV records, update packet filter to to remove this port from list
    if (resourceRecord->rrtype == kDNSType_SRV)
        mDNSUpdatePacketFilter(resourceRecord);

    rhs = DNSNameCompressionBuildLHS(&lower, resourceRecord->rrtype);
    end = DNSNameCompressionBuildRHS(rhs, resourceRecord);
    PrintHelper(__func__, compression_lhs, rhs - compression_lhs, rhs, end - rhs);

    transportType = xD2DInterfaceToTransportType(resourceRecord->InterfaceID, flags, & excludedTransport);
    if (transportType == D2DTransportMax)
    {
        D2DTransportType i;
        for (i = 0; i < D2DTransportMax; i++)
        {
            if (i == excludedTransport) continue;
            if (D2DStopAdvertisingPairOnTransport) D2DStopAdvertisingPairOnTransport(compression_lhs, rhs - compression_lhs, rhs, end - rhs, i);
        }
    }
    else
    {
        if (D2DStopAdvertisingPairOnTransport) D2DStopAdvertisingPairOnTransport(compression_lhs, rhs - compression_lhs, rhs, end - rhs, transportType);
    }
}

mDNSexport void external_start_resolving_service(mDNSInterfaceID InterfaceID, const domainname *const fqdn, DNSServiceFlags flags)
{
    domainname lower;
    mDNSu8 *rhs = NULL;
    mDNSu8 *end = NULL;
    mDNSBool AWDL_used = false;   // whether AWDL was used for this resolve
    D2DTransportType transportType, excludedTransport;
    DomainnameToLower(SkipLeadingLabels(fqdn, 1), &lower);

    LogInfo("external_start_resolving_service: %##s", fqdn->c);
    rhs = DNSNameCompressionBuildLHS(&lower, kDNSType_PTR);
    end = putDomainNameAsLabels(&compression_base_msg, rhs, compression_limit, fqdn);
    PrintHelper(__func__, compression_lhs, rhs - compression_lhs, rhs, end - rhs);

    transportType = xD2DInterfaceToTransportType(InterfaceID, flags, & excludedTransport);
    if (transportType == D2DTransportMax)
    {
        // Resolving over all the transports, except for excludedTransport if set.
        D2DTransportType i;
        for (i = 0; i < D2DTransportMax; i++)
        {
            if (i == excludedTransport) continue;
            if (D2DStartResolvingPairOnTransport) D2DStartResolvingPairOnTransport(compression_lhs, rhs - compression_lhs, rhs, end - rhs, i);

            if (i == D2DAWDLTransport)
                AWDL_used = true;
        }
    }
    else
    {
        // Resolving over one specific transport.
        if (D2DStartResolvingPairOnTransport) D2DStartResolvingPairOnTransport(compression_lhs, rhs - compression_lhs, rhs, end - rhs, transportType);

        if (transportType == D2DAWDLTransport)
            AWDL_used = true;
    }

    // AWDL wants the SRV and TXT record queries communicated over the D2D interface.
    // We only want these records going to AWDL, so use AWDLInterfaceID as the
    // interface and don't set any other flags.
    if (AWDL_used && AWDLInterfaceID)
    {
        LogInfo("external_start_resolving_service: browse for TXT and SRV over AWDL");
        external_start_browsing_for_service(AWDLInterfaceID, fqdn, kDNSType_TXT, 0);
        external_start_browsing_for_service(AWDLInterfaceID, fqdn, kDNSType_SRV, 0);
    }
}

mDNSexport void external_stop_resolving_service(mDNSInterfaceID InterfaceID, const domainname *const fqdn, DNSServiceFlags flags)
{
    domainname lower;
    mDNSu8 *rhs = NULL;
    mDNSu8 *end = NULL;
    mDNSBool AWDL_used = false;   // whether AWDL was used for this resolve
    D2DTransportType transportType, excludedTransport;
    DomainnameToLower(SkipLeadingLabels(fqdn, 1), &lower);

    LogInfo("external_stop_resolving_service: %##s", fqdn->c);
    rhs = DNSNameCompressionBuildLHS(&lower, kDNSType_PTR);
    end = putDomainNameAsLabels(&compression_base_msg, rhs, compression_limit, fqdn);
    PrintHelper(__func__, compression_lhs, rhs - compression_lhs, rhs, end - rhs);

    transportType = xD2DInterfaceToTransportType(InterfaceID, flags, & excludedTransport);
    if (transportType == D2DTransportMax)
    {
        D2DTransportType i;
        for (i = 0; i < D2DTransportMax; i++)
        {
            if (i == excludedTransport) continue;
            if (D2DStopResolvingPairOnTransport) D2DStopResolvingPairOnTransport(compression_lhs, rhs - compression_lhs, rhs, end - rhs, i);

            if (i == D2DAWDLTransport)
                AWDL_used = true;
        }
    }
    else
    {
        if (D2DStopResolvingPairOnTransport) D2DStopResolvingPairOnTransport(compression_lhs, rhs - compression_lhs, rhs, end - rhs, transportType);

        if (transportType == D2DAWDLTransport)
            AWDL_used = true;
    }

    // AWDL wants the SRV and TXT record queries communicated over the D2D interface.
    // We only want these records going to AWDL, so use AWDLInterfaceID as the
    // interface and don't set any other flags.
    if (AWDL_used && AWDLInterfaceID)
    {
        LogInfo("external_stop_resolving_service: stop browse for TXT and SRV on AWDL");
        external_stop_browsing_for_service(AWDLInterfaceID, fqdn, kDNSType_TXT, 0);
        external_stop_browsing_for_service(AWDLInterfaceID, fqdn, kDNSType_SRV, 0);
    }
}

mDNSexport mDNSBool callExternalHelpers(mDNSInterfaceID InterfaceID, const domainname *const domain, DNSServiceFlags flags)
{
    // Only call D2D layer routines if request applies to a D2D interface and the domain is "local".
    if (    (((InterfaceID == mDNSInterface_Any) && (flags & (kDNSServiceFlagsIncludeP2P | kDNSServiceFlagsIncludeAWDL | kDNSServiceFlagsAutoTrigger)))
            || mDNSPlatformInterfaceIsD2D(InterfaceID) || (InterfaceID == mDNSInterface_BLE))
        && IsLocalDomain(domain))
    {
        return mDNStrue;
    }
    else
        return mDNSfalse;
}

// Used to derive the original D2D specific flags specified by the client in the registration
// when we don't have access to the original flag (kDNSServiceFlags*) values.
mDNSexport mDNSu32 deriveD2DFlagsFromAuthRecType(AuthRecType authRecType)
{
    mDNSu32 flags = 0;
    if ((authRecType == AuthRecordAnyIncludeP2P) || (authRecType == AuthRecordAnyIncludeAWDLandP2P))
        flags |= kDNSServiceFlagsIncludeP2P;
    else if ((authRecType == AuthRecordAnyIncludeAWDL) || (authRecType == AuthRecordAnyIncludeAWDLandP2P))
        flags |= kDNSServiceFlagsIncludeAWDL;
    return flags;
}

void initializeD2DPlugins(mDNS *const m)
{
        // We only initialize if mDNSCore successfully initialized.
        if (D2DInitialize)
        {
            D2DStatus ds = D2DInitialize(CFRunLoopGetMain(), xD2DServiceCallback, m);
            if (ds != kD2DSuccess)
                LogMsg("D2DInitialiize failed: %d", ds);
            else
                LogMsg("D2DInitialize succeeded");
        }
}

void terminateD2DPlugins(void)
{
    if (D2DTerminate)
    {
        D2DStatus ds = D2DTerminate();
        if (ds != kD2DSuccess)
            LogMsg("D2DTerminate failed: %d", ds);
        else
            LogMsg("D2DTerminate succeeded");
    }
}

#ifdef UNIT_TEST
#pragma mark - Unit test support routines

// These unit test support routines are called from unittests/ framework
// and are not compiled for the mDNSResponder runtime code paths.

void D2D_unitTest(void)
{
}

#endif  //  UNIT_TEST
