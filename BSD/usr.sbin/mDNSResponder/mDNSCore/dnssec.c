/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2011-2019 Apple Inc. All rights reserved.
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

#include "mDNSEmbeddedAPI.h"
#include "DNSSECSupport.h"
#include "DNSCommon.h"
#include "dnssec.h"
#include "CryptoAlg.h"
#include "nsec.h"
#include "nsec3.h"

// Define DNSSEC_DISABLED to remove all the DNSSEC functionality
// and use the stub functions implemented later in this file.

#ifndef DNSSEC_DISABLED

//#define DNSSEC_DEBUG

#ifdef DNSSEC_DEBUG
#define debugdnssec LogMsg
#else
#define debugdnssec debug_noop
#endif
//
// Implementation Notes
//
// The entry point to DNSSEC Verification is VerifySignature. This function is called from the "core" when
// the answer delivered to the application needs DNSSEC validation. If a question needs DNSSEC
// validation, "ValidationRequired" would be set. As we need to issue more queries to validate the
// original question, we create another question as part of the verification process (question is part of
// DNSSECVerifier). This question sets "ValidatingResponse" to distinguish itself from the original
// question. Without this, it will be a duplicate and never sent out. The "core" almost treats both the
// types identically (like adding EDNS0 option with DO bit etc.) except for a few differences. When RRSIGs
// are added to the cache, "ValidatingResponse" question gets called back as long as the typeCovered matches
// the question's qtype. See the comment in DNSSECRecordAnswersQuestion for the details. The other big
// difference is that "ValidationRequired" question kicks off the verification process by calling into
// "VerifySignature" whereas ValidationResponse don't do that as it gets callback for its questions.
//
// VerifySignature does not retain the original question that started the verification process. It just
// remembers the name and the type. It takes a snapshot of the cache at that instance which will be
// verified using DNSSEC. If the cache changes subsequently e.g., network change etc., it will be detected
// when the validation is completed. If there is a change, it will be revalidated.
//
// The verification flow looks like this:
//
// VerifySignature -> StartDNSSECVerification - GetAllRRSetsForVerification -> FinishDNSSECVerification -> VerifySignature
//
// Verification is a recursive process. It stops when we find a trust anchor or if we have recursed too deep.
//
// If the original question resulted in NODATA/NXDOMAIN error, there should have been NSECs as part of the response.
// These nsecs are cached along with the negative cache record. These are validated using ValidateWithNSECS called
// from Verifysignature.
//
// The flow in this case looks like this:
//
// VerifySignature -> ValidateWithNSECS -> {NoDataProof, NameErrorProof} -> VerifyNSECS -> StartDNSSECVerification
//
// Once the DNSSEC verification is started, it is similar to the previous flow described above. When the verification
// is done, DNSSECPositiveValidationCB or DNSSECNegativeValidationCB will be called which will then deliver the
// validation results to the original question that started the validation.
//
// Insecure proofs are done when the verification ends up bogus. The flow would look like this
//
// VerifySignature -> StartDNSSECVerification - GetAllRRSetsForVerification -> FinishDNSSECVerification -> DNSSECValidationCB
// {DNSSECPositiveValidationCB, DNSSECNegativeValidationCB} -> ProveInsecure -> VerifySignaure ->
//
// ProveInsecure finds the break in trust in a top-down fashion.
//
// Forward declaration
mDNSlocal void VerifySigCallback(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord);
mDNSlocal mStatus TrustedKey(mDNS *const m, DNSSECVerifier *dv);
mDNSlocal mDNSBool TrustedKeyPresent(mDNS *const m, DNSSECVerifier *dv);
mDNSlocal mStatus ValidateDS(DNSSECVerifier *dv);
mDNSlocal void DNSSECNegativeValidationCB(mDNS *const m, DNSSECVerifier *dv, CacheGroup *cg, ResourceRecord *answer, DNSSECStatus status);
mDNSlocal RRVerifier* CopyRRVerifier(RRVerifier *from);
mDNSlocal void FreeDNSSECAuthChainInfo(AuthChain *ac);

// Currently we use this to convert a RRVerifier to resource record so that we can
// use the standard DNS utility functions
LargeCacheRecord largerec;

// Verification is a recursive process. We arbitrarily limit to 10 just to be cautious which should be
// removed in the future.
#define MAX_RECURSE_COUNT   10

// TTL (in seconds) when the DNSSEC status is Bogus
#define RR_BOGUS_TTL        60

// RFC 4034 Appendix B: Get the keyid of a DNS KEY. It is not transmitted
// explicitly on the wire.
//
// Note: This just helps narrow down the list of keys to look at. It is possible
// for two DNS keys to have the same ID i.e., key ID is not a unqiue tag
//
// 1st argument - the RDATA part of the DNSKEY RR
// 2nd argument - the RDLENGTH
//
mDNSlocal mDNSu32 keytag(mDNSu8 *key, mDNSu32 keysize)
{
    unsigned long ac;
    unsigned int i;

    // DST_ALG_RSAMD5 will be rejected automatically as the keytag
    // is calculated wrongly

    for (ac = 0, i = 0; i < keysize; ++i)
        ac += (i & 1) ? key[i] : key[i] << 8;
    ac += (ac >> 16) & 0xFFFF;
    return ac & 0xFFFF;
}

mDNSexport int DNSMemCmp(const mDNSu8 *const m1, const mDNSu8 *const m2, int len)
{
    int res;

    res = mDNSPlatformMemCmp(m1, m2, len);
    if (res != 0)
        return (res < 0 ? -1 : 1);
    return 0;
}

// RFC 4034:
//
// Section 6.1:
//
// For the purposes of DNS security, owner names are ordered by treating
// individual labels as unsigned left-justified octet strings.  The
// absence of a octet sorts before a zero value octet, and uppercase
// US-ASCII letters are treated as if they were lowercase US-ASCII
// letters.
//
// To compute the canonical ordering of a set of DNS names, start by
// sorting the names according to their most significant (rightmost)
// labels.  For names in which the most significant label is identical,
// continue sorting according to their next most significant label, and
// so forth.
//
// Returns 0 if the names are same
// Returns -1 if d1 < d2
// Returns  1 if d1 > d2
//
// subdomain is set if there is at least one label match (starting from the end)
// and d1 has more labels than d2 e.g., a.b.com is a subdomain of b.com
//
mDNSexport int DNSSECCanonicalOrder(const domainname *const d1, const domainname *const d2, int *subdomain)
{
    int count, c1, c2;
    int i, skip1, skip2;

    c1 = CountLabels(d1);
    skip1 = c1 - 1;
    c2 = CountLabels(d2);
    skip2 = c2 - 1;

    if (subdomain) *subdomain = 0;

    // Compare as many labels as possible starting from the rightmost
    count = c1 < c2 ? c1 : c2;
    for (i = count; i > 0; i--)
    {
        mDNSu8 *a, *b;
        int j, len, lena, lenb;

        a = (mDNSu8 *)SkipLeadingLabels(d1, skip1);
        b = (mDNSu8 *)SkipLeadingLabels(d2, skip2);
        lena = *a;
        lenb = *b;
        // Compare label by label. Note that "z" > "yak" because z > y, but z < za
        // (lena - lenb check below) because 'za' has two characters. Hence compare the
        // letters first and then compare the length of the label at the end.
        len = lena < lenb ? lena : lenb;
        a++; b++;
        for (j = 0; j < len; j++)
        {
            mDNSu8 ac = *a++;
            mDNSu8 bc = *b++;
            if (mDNSIsUpperCase(ac)) ac += 'a' - 'A';
            if (mDNSIsUpperCase(bc)) bc += 'a' - 'A';
            if (ac != bc)
            {
                verbosedebugf("DNSSECCanonicalOrder: returning ac %c, bc %c", ac, bc);
                return ((ac < bc) ? -1 : 1);
            }
        }
        if ((lena - lenb) != 0)
        {
            verbosedebugf("DNSSECCanonicalOrder: returning lena %d lenb %d", lena, lenb);
            return ((lena < lenb) ? -1 : 1);
        }

        // Continue with the next label
        skip1--;
        skip2--;
    }
    // We have compared label by label. Both of them are same if we are here.
    //
    // Two possibilities.
    //
    // 1) Both names have same number of labels. In that case, return zero.
    // 2) The number of labels is not same. As zero label sorts before, names
    //    with more number of labels is greater.

    // a.b.com is a subdomain of b.com
    if ((c1 > c2) && subdomain)
        *subdomain = 1;

    verbosedebugf("DNSSECCanonicalOrder: returning c1 %d c2 %d\n", c1, c2);
    if (c1 != c2)
        return ((c1 < c2) ? -1 : 1);
    else
        return 0;
}

// Initialize the question enough so that it can be answered from the cache using SameNameCacheRecordAnswersQuestion or
// ResourceRecordAnswersQuestion.
mDNSexport void InitializeQuestion(mDNS *const m, DNSQuestion *question, mDNSInterfaceID InterfaceID, const domainname *qname,
                                   mDNSu16 qtype, mDNSQuestionCallback *callback, void *context)
{
    debugf("InitializeQuestion: Called for %##s (%s)", qname->c, DNSTypeName(qtype));

    if (question->ThisQInterval != -1) mDNS_StopQuery(m, question);

    mDNS_SetupQuestion(question, InterfaceID, qname, qtype, callback, context);
    question->qnamehash  = DomainNameHashValue(qname);
    question->ValidatingResponse = mDNStrue;

    // Need to hold the lock, as GetServerForQuestion (its callers) references m->timenow.
    mDNS_Lock(m);
    // We need to set the DNS server appropriately to match the question against the cache record.
    // Though not all callers of this function need it, we always do it to keep it simple.
    SetValidDNSServers(m, question);
    question->qDNSServer = GetServerForQuestion(m, question);
    mDNS_Unlock(m);

    // Make it look like unicast
    question->TargetQID = onesID;
    question->TimeoutQuestion = 1;
    question->ReturnIntermed = 1;
    // SetupQuestion sets LongLived if qtype == PTR
    question->LongLived = 0;
}

mDNSexport DNSSECVerifier *AllocateDNSSECVerifier(mDNS *const m, const domainname *name, mDNSu16 rrtype, mDNSInterfaceID InterfaceID,
    mDNSu8 ValidationRequired, DNSSECVerifierCallback dvcallback, mDNSQuestionCallback qcallback)
{
    DNSSECVerifier *dv;

    dv = (DNSSECVerifier *) mDNSPlatformMemAllocateClear(sizeof(*dv));
    if (!dv) { LogMsg("AllocateDNSSECVerifier: ERROR!! memory alloc failed"); return mDNSNULL; }

    LogDNSSEC("AllocateDNSSECVerifier called %p", dv);

    // Remember the question's name and type so that when we are done processing all
    // the verifications, we can trace the original question back
    AssignDomainName(&dv->origName, name);
    dv->origType = rrtype;
    dv->InterfaceID = InterfaceID;
    dv->DVCallback = dvcallback;
    dv->q.ThisQInterval = -1;
    ResetAuthChain(dv);
    // These two are used for Insecure proof if we end up doing it.
    // -Value of ValidationRequired so that we know whether this is a secure or insecure validation
    // -InsecureProofDone tells us whether the proof has been done or not
    dv->ValidationRequired = ValidationRequired;
    dv->InsecureProofDone = 0;
    dv->NumPackets = 0;
    mDNS_Lock(m);
    dv->StartTime = m->timenow;
    mDNS_Unlock(m);
    // The verifier's question has to be initialized as some of the callers assume it
    InitializeQuestion(m, &dv->q, InterfaceID, name, rrtype, qcallback, dv);
    return dv;
}

mDNSlocal AuthChain *AuthChainCopy(AuthChain *ae)
{
    RRVerifier *rvfrom, **rvto;
    AuthChain **prev = mDNSNULL;
    AuthChain *retac = mDNSNULL;
    AuthChain *ac;


    while (ae)
    {
        ac = (AuthChain *) mDNSPlatformMemAllocateClear(sizeof(*ac));
        if (!ac)
        {
            LogMsg("AuthChainCopy: AuthChain alloc failure");
            if (retac)
                FreeDNSSECAuthChainInfo(retac);
            return mDNSNULL;
        }

        ac->next  = mDNSNULL;

        if (!retac)
            retac = ac;

        rvfrom = ae->rrset;
        rvto = &ac->rrset;
        while (rvfrom && rvto)
        {
            *rvto = CopyRRVerifier(rvfrom);
            rvfrom = rvfrom->next;
            rvto = &((*rvto)->next);
        }

        rvfrom = ae->rrsig;
        rvto = &ac->rrsig;
        while (rvfrom && rvto)
        {
            *rvto = CopyRRVerifier(rvfrom);
            rvfrom = rvfrom->next;
            rvto = &((*rvto)->next);
        }

        rvfrom = ae->key;
        rvto = &ac->key;
        while (rvfrom && rvto)
        {
            *rvto = CopyRRVerifier(rvfrom);
            rvfrom = rvfrom->next;
            rvto = &((*rvto)->next);
        }

        if (prev)
        {
            *prev = ac;
        }
        prev = &(ac->next);
        ae = ae->next;
    }
    return retac;
}

mDNSlocal void FreeDNSSECAuthChainInfo(AuthChain *ac)
{
    RRVerifier *rrset;
    RRVerifier *next;
    AuthChain *acnext;

    LogDNSSEC("FreeDNSSECAuthChainInfo: called");

    while (ac)
    {
        acnext = ac->next;
        rrset = ac->rrset;
        while (rrset)
        {
            next = rrset->next;
            mDNSPlatformMemFree(rrset);
            rrset = next;
        }
        ac->rrset = mDNSNULL;

        rrset = ac->rrsig;
        while (rrset)
        {
            next = rrset->next;
            mDNSPlatformMemFree(rrset);
            rrset = next;
        }
        ac->rrsig = mDNSNULL;

        rrset = ac->key;
        while (rrset)
        {
            next = rrset->next;
            mDNSPlatformMemFree(rrset);
            rrset = next;
        }
        ac->key = mDNSNULL;

        mDNSPlatformMemFree(ac);
        ac = acnext;
    }
}

mDNSlocal void FreeDNSSECAuthChain(DNSSECVerifier *dv)
{
    if (dv->ac)
    {
        FreeDNSSECAuthChainInfo(dv->ac);
        // if someone reuses the "dv", it will be initialized properly
        ResetAuthChain(dv);
    }
    if (dv->saveac)
    {
        FreeDNSSECAuthChainInfo(dv->saveac);
        dv->saveac = mDNSNULL;
    }
}

mDNSlocal void FreeAuthChain(mDNS *const m, void *context)
{
    AuthChain *ac = (AuthChain *)context;
    (void) m; // unused

    FreeDNSSECAuthChainInfo(ac);
}

mDNSlocal void FreeDNSSECVerifierRRSets(DNSSECVerifier *dv)
{
    RRVerifier *rrset;
    RRVerifier *next;

    //debugdnssec("FreeDNSSECVerifierRRSets called %p", dv);
    rrset = dv->rrset;
    while (rrset)
    {
        next = rrset->next;
        mDNSPlatformMemFree(rrset);
        rrset = next;
    }
    dv->rrset = mDNSNULL;

    rrset = dv->rrsig;
    while (rrset)
    {
        next = rrset->next;
        mDNSPlatformMemFree(rrset);
        rrset = next;
    }
    dv->rrsig = mDNSNULL;

    rrset = dv->key;
    while (rrset)
    {
        next = rrset->next;
        mDNSPlatformMemFree(rrset);
        rrset = next;
    }
    dv->key = mDNSNULL;

    rrset = dv->rrsigKey;
    while (rrset)
    {
        next = rrset->next;
        mDNSPlatformMemFree(rrset);
        rrset = next;
    }
    dv->rrsigKey = mDNSNULL;

    rrset = dv->ds;
    while (rrset)
    {
        next = rrset->next;
        mDNSPlatformMemFree(rrset);
        rrset = next;
    }
    dv->ds = mDNSNULL;
    rrset = dv->pendingNSEC;
    while (rrset)
    {
        next = rrset->next;
        mDNSPlatformMemFree(rrset);
        rrset = next;
    }
    dv->pendingNSEC = mDNSNULL;
}

mDNSexport void FreeDNSSECVerifier(mDNS *const m, DNSSECVerifier *dv)
{
    LogDNSSEC("FreeDNSSECVerifier called %p", dv);
    if (dv->q.ThisQInterval != -1)
        mDNS_StopQuery(m, &dv->q);
    FreeDNSSECVerifierRRSets(dv);
    if (dv->ctx)
        AlgDestroy(dv->ctx);
    if (dv->ac || dv->saveac)
        FreeDNSSECAuthChain(dv);
    if (dv->parent)
    {
        LogDNSSEC("FreeDNSSECVerifier freeing parent %p", dv->parent);
        FreeDNSSECVerifier(m, dv->parent);
    }
    mDNSPlatformMemFree(dv);
}

mDNSlocal RRVerifier* CopyRRVerifier(RRVerifier *from)
{
    RRVerifier *r;

    r = (RRVerifier *) mDNSPlatformMemAllocate(sizeof(*r) + from->rdlength);
    if (!r)
    {
        LogMsg("CopyRRVerifier: memory failure");
        return mDNSNULL;
    }
    mDNSPlatformMemCopy(r, from, sizeof(RRVerifier));
    r->next = mDNSNULL;
    r->rdata = (mDNSu8*) ((mDNSu8 *)r + sizeof(RRVerifier));
    mDNSPlatformMemCopy(r->rdata, from->rdata, r->rdlength);
    return r;
}

mDNSexport RRVerifier* AllocateRRVerifier(const ResourceRecord *const rr, mStatus *status)
{
    RRVerifier *r;

    r = (RRVerifier *) mDNSPlatformMemAllocateClear(sizeof(*r) + rr->rdlength);
    if (!r)
    {
        LogMsg("AllocateRRVerifier: memory failure");
        *status = mStatus_NoMemoryErr;
        return mDNSNULL;
    }
    r->next = mDNSNULL;
    r->rrtype = rr->rrtype;
    r->rrclass = rr->rrclass;
    r->rroriginalttl = rr->rroriginalttl;
    r->rdlength = rr->rdlength;
    r->namehash = rr->namehash;
    r->rdatahash = rr->rdatahash;
    AssignDomainName(&r->name, rr->name);
    r->rdata = (mDNSu8*) ((mDNSu8 *)r + sizeof(RRVerifier));

    // When we parsed the DNS response in GeLargeResourceRecord, for some records, we parse them into
    // host order so that the rest of the code does not have to bother with converting from network order
    // to host order. For signature verification, we need them back in network order. For DNSSEC records
    // like DNSKEY and DS, we just copy over the data both in GetLargeResourceRecord and putRData.

    if (!putRData(mDNSNULL, r->rdata, r->rdata + rr->rdlength, rr))
    {
        LogMsg("AllocateRRVerifier: putRData failed");
        *status = mStatus_BadParamErr;
        return mDNSNULL;
    }
    *status = mStatus_NoError;
    return r;
}

mDNSexport mStatus AddRRSetToVerifier(DNSSECVerifier *dv, const ResourceRecord *const rr, RRVerifier *rv, RRVerifierSet set)
{
    RRVerifier *r;
    RRVerifier **v;
    mStatus status;

    if (!rv)
    {
        r = AllocateRRVerifier(rr, &status);
        if (!r) return status;
    }
    else
        r = rv;

    switch (set)
    {
    case RRVS_rr:
        v = &dv->rrset;
        break;
    case RRVS_rrsig:
        v = &dv->rrsig;
        break;
    case RRVS_key:
        v = &dv->key;
        break;
    case RRVS_rrsig_key:
        v = &dv->rrsigKey;
        break;
    case RRVS_ds:
        v = &dv->ds;
        break;
    default:
        LogMsg("AddRRSetToVerifier: ERROR!! default case %d", set);
        return mStatus_BadParamErr;
    }
    while (*v)
        v = &(*v)->next;
    *v = r;
    return mStatus_NoError;
}

// Validate the RRSIG. "type" tells which RRSIG that we are supposed to validate. We fetch RRSIG for
// the rrset (type is RRVS_rrsig) and RRSIG for the key (type is RRVS_rrsig_key).
mDNSexport void ValidateRRSIG(DNSSECVerifier *dv, RRVerifierSet type, const ResourceRecord *const rr)
{
    RRVerifier *rv;
    mDNSu32 currentTime;
    rdataRRSig *rrsigRData = (rdataRRSig *)((mDNSu8 *)rr->rdata + sizeofRDataHeader);

    if (type == RRVS_rrsig)
    {
        rv = dv->rrset;
    }
    else if (type == RRVS_rrsig_key)
    {
        rv = dv->key;
    }
    else
    {
        LogMsg("ValidateRRSIG: ERROR!! type not valid %d", type);
        return;
    }

    // RFC 4035:
    // For each authoritative RRset in a signed zone, there MUST be at least
    // one RRSIG record that meets the following requirements:
    //
    // RRSet is defined by same name, class and type
    //
    // 1. The RRSIG RR and the RRset MUST have the same owner name and the same class.
    if (!SameDomainName(&rv->name, rr->name) || (rr->rrclass != rv->rrclass))
    {
        debugdnssec("ValidateRRSIG: name mismatch or class mismatch");
        return;
    }

    // 2. The RRSIG RR's Type Covered field MUST equal the RRset's type.
    if ((swap16(rrsigRData->typeCovered)) != rv->rrtype)
    {
        debugdnssec("ValidateRRSIG: typeCovered mismatch rrsig %d, rr type %d", swap16(rrsigRData->typeCovered), rv->rrtype);
        return;
    }

    // 3. The number of labels in the RRset owner name MUST be greater than or equal
    //    to the value in the RRSIG RR's Labels field.
    if (rrsigRData->labels > CountLabels(&rv->name))
    {
        debugdnssec("ValidateRRSIG: labels count problem rrsig %d, rr %d", rrsigRData->labels, CountLabels(&rv->name));
        return;
    }

    // 4. The RRSIG RR's Signer's Name field MUST be the name of the zone that contains
    //    the RRset. For a stub resolver, this can't be done in a secure way. Hence we
    //    do it this way (discussed in dnsext mailing list)
    switch (rv->rrtype)
    {
    case kDNSType_NS:
    case kDNSType_SOA:
    case kDNSType_DNSKEY:
        //Signed by the owner
        if (!SameDomainName(&rv->name, (domainname *)&rrsigRData->signerName))
        {
            debugdnssec("ValidateRRSIG: Signer Name does not match the record name for %s", DNSTypeName(rv->rrtype));
            return;
        }
        break;
    case kDNSType_DS:
        // Should be signed by the parent
        if (SameDomainName(&rv->name, (domainname *)&rrsigRData->signerName))
        {
            debugdnssec("ValidateRRSIG: Signer Name matches the record name for %s", DNSTypeName(rv->rrtype));
            return;
        }
    // FALLTHROUGH
    default:
    {
        int c1 = CountLabels(&rv->name);
        int c2 = CountLabels((domainname *)&rrsigRData->signerName);
        if (c1 < c2)
        {
            debugdnssec("ValidateRRSIG: Signer Name not a subdomain label count %d < %d ", c1, c2);
            return;
        }
        domainname *d = (domainname *)SkipLeadingLabels(&rv->name, c1 - c2);
        if (!SameDomainName(d, (domainname *)&rrsigRData->signerName))
        {
            debugdnssec("ValidateRRSIG: Signer Name not a subdomain");
            return;
        }
        break;
    }
    }

    // 5. The validator's notion of the current time MUST be less than or equal to the
    //    time listed in the RRSIG RR's Expiration field.
    //
    // 6. The validator's notion of the current time MUST be greater than or equal to the
    //    time listed in the RRSIG RR's Inception field.
    currentTime = mDNSPlatformUTC();

    if (DNS_SERIAL_LT(swap32(rrsigRData->sigExpireTime), currentTime))
    {
        LogDNSSEC("ValidateRRSIG: Expired: currentTime %d, ExpireTime %d", (int)currentTime,
                  swap32((int)rrsigRData->sigExpireTime));
        return;
    }
    if (DNS_SERIAL_LT(currentTime, swap32(rrsigRData->sigInceptTime)))
    {
        LogDNSSEC("ValidateRRSIG: Future: currentTime %d, InceptTime %d", (int)currentTime,
                  swap32((int)rrsigRData->sigInceptTime));
        return;
    }

    if (AddRRSetToVerifier(dv, rr, mDNSNULL, type) != mStatus_NoError)
    {
        LogMsg("ValidateRRSIG: ERROR!! cannot allocate RRSet");
        return;
    }
}

mDNSlocal mStatus CheckRRSIGForRRSet(mDNS *const m, DNSSECVerifier *dv, CacheRecord **negcr)
{
    CacheGroup *cg;
    CacheRecord *cr;
    RRVerifier *rv;
    mDNSBool expectRRSIG = mDNSfalse;

    *negcr = mDNSNULL;
    if (!dv->rrset)
    {
        LogMsg("CheckRRSIGForRRSet: ERROR!! rrset NULL for origName %##s (%s)", dv->origName.c,
               DNSTypeName(dv->origType));
        return mStatus_BadParamErr;
    }

    rv = dv->rrset;
    cg = CacheGroupForName(m, rv->namehash, &rv->name);
    if (!cg)
    {
        debugdnssec("CheckRRSIGForRRSet: cg null");
        return mStatus_NoSuchRecord;
    }

    for (cr=cg->members; cr; cr=cr->next)
    {
        debugdnssec("CheckRRSIGForRRSet: checking the validity of rrsig");
        if (cr->resrec.rrtype != kDNSType_RRSIG)
        {
            // Check to see if we should expect RRSIGs for the type that we are looking for.
            // We would expect RRSIGs, if we had previously issued the question with the
            // EDNS0/DOK bit set.
            if (cr->resrec.rrtype == dv->rrset->rrtype)
            {
                expectRRSIG = cr->CRDNSSECQuestion;
                LogDNSSEC("CheckRRSIGForRRSet: %s RRSIG for %s", (expectRRSIG ? "Expecting" : "Not Expecting"), CRDisplayString(m, cr));
            }
            continue;
        }
        if (cr->resrec.RecordType == kDNSRecordTypePacketNegative)
        {
            if (!(*negcr))
            {
                LogDNSSEC("CheckRRSIGForRRSet: Negative cache record %s encountered for %##s (%s)", CRDisplayString(m, cr),
                          rv->name.c, DNSTypeName(rv->rrtype));
                *negcr = cr;
            }
            else
            {
                LogMsg("CheckRRSIGForRRSet: ERROR!! Negative cache record %s already set for %##s (%s)", CRDisplayString(m, cr),
                       rv->name.c, DNSTypeName(rv->rrtype));
            }
            continue;
        }
        ValidateRRSIG(dv, RRVS_rrsig, &cr->resrec);
    }
    if (*negcr && dv->rrsig)
    {
        // Encountered both RRSIG and negative CR
        LogMsg("CheckRRSIGForRRSet: ERROR!! Encountered negative cache record %s and RRSIG for %##s (%s)",
               CRDisplayString(m, *negcr), rv->name.c, DNSTypeName(rv->rrtype));
        return mStatus_BadParamErr;
    }
    // If we can't find RRSIGs, but we find a negative response then we need to validate that
    // which the caller will do it. Otherwise, if we should be expecting RRSIGs to be in the
    // cache already, then return error.
    if (dv->rrsig || *negcr)
        return mStatus_NoError;
    else if (expectRRSIG)
        return mStatus_BadParamErr;
    else
        return mStatus_NoSuchRecord;
}

mDNSlocal void CheckOneKeyForRRSIG(DNSSECVerifier *dv, const ResourceRecord *const rr)
{
    rdataRRSig *rrsig;

    if (!dv->rrsig)
    {
        LogMsg("CheckOneKeyForRRSIG: ERROR!! rrsig NULL");
        return;
    }
    rrsig = (rdataRRSig *)dv->rrsig->rdata;
    if (!SameDomainName((domainname *)&rrsig->signerName, rr->name))
    {
        debugdnssec("CheckOneKeyForRRSIG: name mismatch");
        return;
    }

    // We store all the keys including the ZSK and KSK and use them appropriately
    // later
    if (AddRRSetToVerifier(dv, rr, mDNSNULL, RRVS_key) != mStatus_NoError)
    {
        LogMsg("CheckOneKeyForRRSIG: ERROR!! cannot allocate RRSet");
        return;
    }
}

mDNSlocal mStatus CheckKeyForRRSIG(mDNS *const m, DNSSECVerifier *dv, CacheRecord **negcr)
{
    mDNSu32 namehash;
    CacheGroup *cg;
    CacheRecord *cr;
    rdataRRSig *rrsig;
    domainname *name;

    *negcr = mDNSNULL;
    if (!dv->rrsig)
    {
        LogMsg("CheckKeyForRRSIG: ERROR!! rrsig NULL");
        return mStatus_BadParamErr;
    }

    // Signer name should be the same on all rrsig ??
    rrsig = (rdataRRSig *)dv->rrsig->rdata;
    name = (domainname *)&rrsig->signerName;

    namehash = DomainNameHashValue(name);
    cg = CacheGroupForName(m, namehash, name);
    if (!cg)
    {
        debugdnssec("CheckKeyForRRSIG: cg null for %##s", name->c);
        return mStatus_NoSuchRecord;
    }

    for (cr=cg->members; cr; cr=cr->next)
    {
        if (cr->resrec.rrtype != kDNSType_DNSKEY) continue;
        if (cr->resrec.RecordType == kDNSRecordTypePacketNegative)
        {
            if (!(*negcr))
            {
                LogDNSSEC("CheckKeyForRRSIG: Negative cache record %s encountered for %##s (DNSKEY)", CRDisplayString(m, cr),
                          name->c);
                *negcr = cr;
            }
            else
            {
                LogMsg("CheckKeyForRRSIG: ERROR!! Negative cache record %s already set for %##s (DNSKEY)", CRDisplayString(m, cr),
                       name->c);
            }
            continue;
        }
        debugdnssec("CheckKeyForRRSIG: checking the validity of key record");
        CheckOneKeyForRRSIG(dv, &cr->resrec);
    }
    if (*negcr && dv->key)
    {
        // Encountered both RRSIG and negative CR
        LogMsg("CheckKeyForRRSIG: ERROR!! Encountered negative cache record %s and DNSKEY for %##s",
               CRDisplayString(m, *negcr), name->c);
        return mStatus_BadParamErr;
    }
    if (dv->key || *negcr)
        return mStatus_NoError;
    else
        return mStatus_NoSuchRecord;
}

mDNSlocal void CheckOneRRSIGForKey(DNSSECVerifier *dv, const ResourceRecord *const rr)
{
    rdataRRSig *rrsig;
    if (!dv->rrsig)
    {
        LogMsg("CheckOneRRSIGForKey: ERROR!! rrsig NULL");
        return;
    }
    rrsig = (rdataRRSig *)dv->rrsig->rdata;
    if (!SameDomainName((domainname *)&rrsig->signerName, rr->name))
    {
        debugdnssec("CheckOneRRSIGForKey: name mismatch");
        return;
    }
    ValidateRRSIG(dv, RRVS_rrsig_key, rr);
}

mDNSlocal mStatus CheckRRSIGForKey(mDNS *const m, DNSSECVerifier *dv, CacheRecord **negcr)
{
    mDNSu32 namehash;
    CacheGroup *cg;
    CacheRecord *cr;
    rdataRRSig *rrsig;
    domainname *name;
    mDNSBool expectRRSIG = mDNSfalse;

    *negcr = mDNSNULL;
    if (!dv->rrsig)
    {
        LogMsg("CheckRRSIGForKey: ERROR!! rrsig NULL");
        return mStatus_BadParamErr;
    }
    if (!dv->key)
    {
        LogMsg("CheckRRSIGForKey:  ERROR!! key NULL");
        return mStatus_BadParamErr;
    }
    rrsig = (rdataRRSig *)dv->rrsig->rdata;
    name = (domainname *)&rrsig->signerName;

    namehash = DomainNameHashValue(name);
    cg = CacheGroupForName(m, namehash, name);
    if (!cg)
    {
        debugdnssec("CheckRRSIGForKey: cg null %##s", name->c);
        return mStatus_NoSuchRecord;
    }
    for (cr=cg->members; cr; cr=cr->next)
    {
        if (cr->resrec.rrtype != kDNSType_RRSIG)
        {
            // Check to see if we should expect RRSIGs for the DNSKEY record that we are
            // looking for. We would expect RRSIGs, if we had previously issued the question
            // with the EDNS0/DOK bit set.
            if (cr->resrec.rrtype == kDNSType_DNSKEY)
            {
                expectRRSIG = cr->CRDNSSECQuestion;
                LogDNSSEC("CheckRRSIGForKey: %s RRSIG for %s", (expectRRSIG ? "Expecting" : "Not Expecting"), CRDisplayString(m, cr));
            }
            continue;
        }
        if (cr->resrec.RecordType == kDNSRecordTypePacketNegative)
        {
            if (!(*negcr))
            {
                LogDNSSEC("CheckRRSIGForKey: Negative cache record %s encountered for %##s (RRSIG)", CRDisplayString(m, cr),
                          name->c);
                *negcr = cr;
            }
            else
            {
                LogMsg("CheckRRSIGForKey: ERROR!! Negative cache record %s already set for %##s (RRSIG)", CRDisplayString(m, cr),
                       name->c);
            }
            continue;
        }
        debugdnssec("CheckRRSIGForKey: checking the validity of rrsig");
        CheckOneRRSIGForKey(dv, &cr->resrec);
    }
    if (*negcr && dv->rrsigKey)
    {
        // Encountered both RRSIG and negative CR
        LogMsg("CheckRRSIGForKey: ERROR!! Encountered negative cache record %s and DNSKEY for %##s",
               CRDisplayString(m, *negcr), name->c);
        return mStatus_BadParamErr;
    }
    // If we can't find RRSIGs, but we find a negative response then we need to validate that
    // which the caller will do it. Finally, make sure that we are not expecting RRSIGS.
    if (dv->rrsigKey || *negcr)
        return mStatus_NoError;
    else if (expectRRSIG)
        return mStatus_BadParamErr;
    else
        return mStatus_NoSuchRecord;
}

mDNSlocal void CheckOneDSForKey(DNSSECVerifier *dv, const ResourceRecord *const rr)
{
    mDNSu16 tag;
    rdataDS *DS;
    RRVerifier *keyv;
    rdataDNSKey *key;
    rdataRRSig *rrsig;

    if (!dv->rrsig)
    {
        LogMsg("CheckOneDSForKey: ERROR!! rrsig NULL");
        return;
    }
    rrsig = (rdataRRSig *)dv->rrsig->rdata;
    DS = (rdataDS *)((mDNSu8 *)rr->rdata + sizeofRDataHeader);

    if (!SameDomainName((domainname *)&rrsig->signerName, rr->name))
    {
        debugdnssec("CheckOneDSForKey: name mismatch");
        return;
    }
    for (keyv = dv->key; keyv; keyv = keyv->next)
    {
        key = (rdataDNSKey *)keyv->rdata;
        tag = (mDNSu16)keytag((mDNSu8 *)key, keyv->rdlength);
        if (tag != swap16(DS->keyTag))
        {
            debugdnssec("CheckOneDSForKey: keyTag mismatch keyTag %d, DStag %d", tag, swap16(DS->keyTag));
            continue;
        }
        if (key->alg != DS->alg)
        {
            debugdnssec("CheckOneDSForKey: alg mismatch key alg%d, DS alg %d", key->alg, swap16(DS->alg));
            continue;
        }
        if (AddRRSetToVerifier(dv, rr, mDNSNULL, RRVS_ds) != mStatus_NoError)
        {
            debugdnssec("CheckOneDSForKey: cannot allocate RRSet");
        }
    }
}

mDNSlocal mStatus CheckDSForKey(mDNS *const m, DNSSECVerifier *dv, CacheRecord **negcr)
{
    mDNSu32 namehash;
    CacheGroup *cg;
    CacheRecord *cr;
    rdataRRSig *rrsig;
    domainname *name;

    *negcr = mDNSNULL;
    if (!dv->rrsig)
    {
        LogMsg("CheckDSForKey: ERROR!! rrsig NULL");
        return mStatus_BadParamErr;
    }
    if (!dv->key)
    {
        LogMsg("CheckDSForKey: ERROR!! key NULL");
        return mStatus_BadParamErr;
    }
    rrsig = (rdataRRSig *)dv->rrsig->rdata;
    name = (domainname *)&rrsig->signerName;
    namehash = DomainNameHashValue(name);
    cg = CacheGroupForName(m, namehash, name);
    if (!cg)
    {
        debugdnssec("CheckDSForKey: cg null for %s", name->c);
        return mStatus_NoSuchRecord;
    }
    for (cr=cg->members; cr; cr=cr->next)
    {
        if (cr->resrec.rrtype != kDNSType_DS) continue;
        if (cr->resrec.RecordType == kDNSRecordTypePacketNegative)
        {
            if (!(*negcr))
            {
                LogDNSSEC("CheckDSForKey: Negative cache record %s encountered for %##s (DS)", CRDisplayString(m, cr),
                          name->c);
                *negcr = cr;
            }
            else
            {
                LogMsg("CheckDSForKey: ERROR!! Negative cache record %s already set for %##s (DS)", CRDisplayString(m, cr),
                       name->c);
            }
            continue;
        }
        CheckOneDSForKey(dv, &cr->resrec);
    }
    if (*negcr && dv->ds)
    {
        // Encountered both RRSIG and negative CR
        LogMsg("CheckDSForKey: ERROR!! Encountered negative cache record %s and DS for %##s",
               CRDisplayString(m, *negcr), name->c);
        return mStatus_BadParamErr;
    }
    if (dv->ds || *negcr)
        return mStatus_NoError;
    else
        return mStatus_NoSuchRecord;
}

// It returns mDNStrue if we have all the rrsets for verification and mDNSfalse otherwise.
mDNSlocal mDNSBool GetAllRRSetsForVerification(mDNS *const m, DNSSECVerifier *dv)
{
    mStatus err;
    CacheRecord *negcr;
    rdataRRSig *rrsig;

    if (!dv->rrset)
    {
        LogMsg("GetAllRRSetsForVerification: ERROR!! rrset NULL");
        dv->DVCallback(m, dv, DNSSEC_Bogus);
        return mDNSfalse;
    }

    if (dv->next == RRVS_done) return mDNStrue;

    debugdnssec("GetAllRRSetsForVerification: next %d", dv->next);
    switch (dv->next)
    {
    case RRVS_rrsig:
        // If we can't find the RRSIG for the rrset, re-issue the query.
        //
        // NOTE: It is possible that the cache might answer partially e.g., RRSIGs match qtype but the
        // whole set is not there. In that case the validation will fail. Ideally we should flush the
        // cache and reissue the query (TBD).
        err = CheckRRSIGForRRSet(m, dv, &negcr);
        if (err != mStatus_NoSuchRecord && err != mStatus_NoError)
        {
            dv->DVCallback(m, dv, DNSSEC_Bogus);
            return mDNSfalse;
        }
        // Need to initialize the question as if we end up in ValidateWithNSECS below, the nsec proofs
        // looks in "dv->q" for the proof. Note that we have to use currQtype as the response could be
        // a CNAME and dv->rrset->rrtype would be set to CNAME and not the original question type that
        // resulted in CNAME.
        InitializeQuestion(m, &dv->q, dv->InterfaceID, &dv->rrset->name, dv->currQtype, VerifySigCallback, dv);
        // We may not have the NSECS if the previous query was a non-DNSSEC query
        if (negcr && negcr->nsec)
        {
            ValidateWithNSECS(m, dv, negcr);
            return mDNSfalse;
        }

        dv->next = RRVS_key;
        if (!dv->rrsig)
        {
            // We already found the rrset to verify. Ideally we should just issue the query for the RRSIG. Unfortunately,
            // that does not work well as the response may not contain the RRSIG whose typeCovered matches the
            // rrset->rrtype (recursive server returns what is in its cache). Hence, we send the original query with the
            // DO bit set again to get the RRSIG. Normally this would happen if there was question which did not require
            // DNSSEC validation (ValidationRequied = 0) populated the cache and later when the ValidationRequired question
            // comes along, we need to get the RRSIGs. If we started off with ValidationRequired question we would have
            // already set the DO bit and not able to get RRSIGs e.g., bad CPE device, we would reissue the query here
            // again once more.
            //
            // Also, if it is a wildcard expanded answer, we need to issue the query with the original type for it to
            // elicit the right NSEC records. Just querying for RRSIG alone is not sufficient.
            //
            // Note: For this to work, the core needs to deliver RRSIGs when they are added to the cache even if the
            // "qtype" is not RRSIG.
            debugdnssec("GetAllRRSetsForVerification: Fetching RRSIGS for RRSET");
            dv->NumPackets++;
            mDNS_StartQuery(m, &dv->q);
            return mDNSfalse;
        }
        // if we found the RRSIG, then fall through to find the DNSKEY
    case RRVS_key:
        err = CheckKeyForRRSIG(m, dv, &negcr);
        if (err != mStatus_NoSuchRecord && err != mStatus_NoError)
        {
            dv->DVCallback(m, dv, DNSSEC_Bogus);
            return mDNSfalse;
        }
        // Need to initialize the question as if we end up in ValidateWithNSECS below, the nsec proofs
        // looks in "dv->q" for the proof.
        rrsig = (rdataRRSig *)dv->rrsig->rdata;
        InitializeQuestion(m, &dv->q, dv->InterfaceID, (domainname *)&rrsig->signerName, kDNSType_DNSKEY, VerifySigCallback, dv);
        // We may not have the NSECS if the previous query was a non-DNSSEC query
        if (negcr && negcr->nsec)
        {
            ValidateWithNSECS(m, dv, negcr);
            return mDNSfalse;
        }

        dv->next = RRVS_rrsig_key;
        if (!dv->key)
        {
            debugdnssec("GetAllRRSetsForVerification: Fetching DNSKEY for RRSET");
            dv->NumPackets++;
            mDNS_StartQuery(m, &dv->q);
            return mDNSfalse;
        }
    // if we found the DNSKEY, then fall through to find the RRSIG for the DNSKEY
    case RRVS_rrsig_key:
        err = CheckRRSIGForKey(m, dv, &negcr);
        // if we are falling through, then it is okay if we don't find the record
        if (err != mStatus_NoSuchRecord && err != mStatus_NoError)
        {
            dv->DVCallback(m, dv, DNSSEC_Bogus);
            return mDNSfalse;
        }
        // Need to initialize the question as if we end up in ValidateWithNSECS below, the nsec proofs
        // looks in "dv->q" for the proof.
        rrsig = (rdataRRSig *)dv->rrsig->rdata;
        InitializeQuestion(m, &dv->q, dv->InterfaceID, (domainname *)&rrsig->signerName, kDNSType_DNSKEY, VerifySigCallback, dv);
        // We may not have the NSECS if the previous query was a non-DNSSEC query
        if (negcr && negcr->nsec)
        {
            ValidateWithNSECS(m, dv, negcr);
            return mDNSfalse;
        }
        dv->next = RRVS_ds;
        debugdnssec("GetAllRRSetsForVerification: RRVS_rrsig_key %p", dv->rrsigKey);
        if (!dv->rrsigKey)
        {
            debugdnssec("GetAllRRSetsForVerification: Fetching RRSIGS for DNSKEY");
            dv->NumPackets++;
            mDNS_StartQuery(m, &dv->q);
            return mDNSfalse;
        }
    // if we found RRSIG for the DNSKEY, then fall through to find the DS
    case RRVS_ds:
    {
        domainname *qname;
        rrsig = (rdataRRSig *)dv->rrsig->rdata;
        qname = (domainname *)&rrsig->signerName;

        err = CheckDSForKey(m, dv, &negcr);
        if (err != mStatus_NoSuchRecord && err != mStatus_NoError)
        {
            dv->DVCallback(m, dv, DNSSEC_Bogus);
            return mDNSfalse;
        }
        // Need to initialize the question as if we end up in ValidateWithNSECS below, the nsec proofs
        // looks in "dv->q" for the proof.
        InitializeQuestion(m, &dv->q, dv->InterfaceID, qname, kDNSType_DS, VerifySigCallback, dv);
        // We may not have the NSECS if the previous query was a non-DNSSEC query
        if (negcr && negcr->nsec)
        {
            ValidateWithNSECS(m, dv, negcr);
            return mDNSfalse;
        }
        dv->next = RRVS_done;
        // If we have a trust anchor, then don't bother looking up the DS record
        if (!dv->ds && !TrustedKeyPresent(m, dv))
        {
            // There is no DS for the root. Hence, if we don't have the trust
            // anchor for root, just fail.
            if (SameDomainName(qname, (const domainname *)"\000"))
            {
                LogDNSSEC("GetAllRRSetsForVerification: Reached root");
                dv->DVCallback(m, dv, DNSSEC_Bogus);
                return mDNSfalse;
            }
            debugdnssec("GetAllRRSetsForVerification: Fetching DS");
            dv->NumPackets++;
            mDNS_StartQuery(m, &dv->q);
            return mDNSfalse;
        }
        else
        {
            debugdnssec("GetAllRRSetsForVerification: Skipped fetching the DS");
            return mDNStrue;
        }
    }
    default:
        LogMsg("GetAllRRSetsForVerification: ERROR!! unknown next %d", dv->next);
        dv->DVCallback(m, dv, DNSSEC_Bogus);
        return mDNSfalse;
    }
}

#ifdef DNSSEC_DEBUG
mDNSlocal void PrintFixedSignInfo(rdataRRSig *rrsig, domainname *signerName, int sigNameLen, mDNSu8 *fixedPart, int fixedPartLen)
{
    int j;
    char buf[RRSIG_FIXED_SIZE *3 + 1]; // 3 bytes count for %2x + 1 and the one byte for null at the end
    char sig[sigNameLen * 3 + 1];
    char fp[fixedPartLen * 3 + 1];
    int length;

    length = 0;
    for (j = 0; j < RRSIG_FIXED_SIZE; j++)
        length += mDNS_snprintf(buf+length, sizeof(buf) - length - 1, "%2x ", ((mDNSu8 *)rrsig)[j]);
    LogMsg("RRSIG(%d) %s", RRSIG_FIXED_SIZE, buf);


    length = 0;
    for (j = 0; j < sigNameLen; j++)
        length += mDNS_snprintf(sig+length, sizeof(sig) - length - 1, "%2x ", signerName->c[j]);
    LogMsg("SIGNAME(%d) %s", sigNameLen, sig);

    length = 0;
    for (j = 0; j < fixedPartLen; j++)
        length += mDNS_snprintf(fp+length, sizeof(fp) - length - 1, "%2x ", fixedPart[j]);
    LogMsg("fixedPart(%d) %s", fixedPartLen, fp);
}

mDNSlocal void PrintVarSignInfo(mDNSu16 rdlen, mDNSu8 *rdata)
{
    unsigned int j;
    mDNSu8 *r;
    unsigned int blen = swap16(rdlen);
    char buf[blen * 3 + 1]; // 3 bytes count for %2x + 1 and the one byte for null at the end
    int length;

    length = 0;

    r = (mDNSu8 *)&rdlen;
    for (j = 0; j < sizeof(mDNSu16); j++)
        length += mDNS_snprintf(buf+length, sizeof(buf) - length - 1, "%2x ", r[j]);
    LogMsg("RDLENGTH(%d) %s", sizeof(mDNSu16), buf);

    length = 0;
    for (j = 0; j < blen; j++)
        length += mDNS_snprintf(buf+length, sizeof(buf) - length - 1, "%2x ", rdata[j]);
    LogMsg("RDATA(%d) %s", blen, buf);
}
#else
mDNSlocal void PrintVarSignInfo(mDNSu16 rdlen, mDNSu8 *rdata)
{
    (void)rdlen;
    (void)rdata;
}
mDNSlocal void PrintFixedSignInfo(rdataRRSig *rrsig, domainname *signerName, int sigNameLen, mDNSu8 *fixedPart, int fixedPartLen)
{
    (void)rrsig;
    (void)signerName;
    (void)sigNameLen;
    (void)fixedPart;
    (void)fixedPartLen;
}
#endif

// Used for RDATA comparison
typedef struct
{
    mDNSu16 rdlength;
    mDNSu16 rrtype;
    mDNSu8 *rdata;
} rdataComp;

mDNSlocal int rdata_compare(mDNSu8 *const rdata1, mDNSu8 *const rdata2, int rdlen1, int rdlen2)
{
    int len;
    int ret;

    len = (rdlen1 < rdlen2) ? rdlen1 : rdlen2;

    ret = DNSMemCmp(rdata1, rdata2, len);
    if (ret != 0) return ret;

    // RDATA is same at this stage. Consider them equal if they are of same length. Otherwise
    // decide based on their lengths.
    return ((rdlen1 == rdlen2) ? 0 : (rdlen1 < rdlen2) ? -1 : 1);
}

mDNSlocal int name_compare(mDNSu8 *const rdata1, mDNSu8 *const rdata2, int rdlen1, int rdlen2)
{
    domainname *n1 = (domainname *)rdata1;
    domainname *n2 = (domainname *)rdata2;
    mDNSu8 *a = n1->c;
    mDNSu8 *b = n2->c;
    int count, c1, c2;
    int i, j, len;

    c1 = CountLabels(n1);
    c2 = CountLabels(n2);

    count = c1 < c2 ? c1 : c2;

    // We can't use SameDomainName as we need to know exactly which is greater/smaller
    // for sorting purposes. Hence, we need to compare label by label
    for (i = 0; i < count; i++)
    {
        // Are the lengths same ?
        if (*a != *b)
        {
            debugdnssec("compare_name: returning c1 %d, c2 %d", *a, *b);
            return ((*a < *b) ? -1 : 1);
        }
        len = *a;
        rdlen1 -= (len + 1);
        rdlen2 -= (len + 1);
        if (rdlen1 < 0 || rdlen2 < 0)
        {
            LogMsg("name_compare: ERROR!! not enough data rdlen1 %d, rdlen2 %d", rdlen1, rdlen2);
            return -1;
        }
        a++; b++;
        for (j = 0; j < len; j++)
        {
            mDNSu8 ac = *a++;
            mDNSu8 bc = *b++;
            if (mDNSIsUpperCase(ac)) ac += 'a' - 'A';
            if (mDNSIsUpperCase(bc)) bc += 'a' - 'A';
            if (ac != bc)
            {
                debugdnssec("compare_name: returning ac %c, bc %c", ac, bc);
                return ((ac < bc) ? -1 : 1);
            }
        }
    }

    return 0;
}

mDNSlocal int srv_compare(rdataComp *const r1, rdataComp *const r2)
{
    int res;
    int length1, length2;

    length1 = r1->rdlength;
    length2 = r2->rdlength;
    // We should have at least priority, weight, port plus 1 byte
    if (length1 < 7 || length2 < 7)
    {
        LogMsg("srv_compare: ERROR!! Length smaller than 7 bytes");
        return -1;
    }
    // Compare priority, weight and port
    res = DNSMemCmp(r1->rdata, r2->rdata, 6);
    if (res != 0) return res;
    length1 -= 6;
    length2 -= 6;
    return (name_compare(r1->rdata + 6, r2->rdata + 6, length1, length2));
}

mDNSlocal int tsig_compare(rdataComp *const r1, rdataComp *const r2)
{
    int offset1, offset2;
    int length1, length2;
    int res, dlen;

    offset1 = offset2 = 0;
    length1 = r1->rdlength;
    length2 = r2->rdlength;

    // we should have at least one byte to start with
    if (length1 < 1 || length2 < 1)
    {
        LogMsg("sig_compare: Length smaller than 18 bytes");
        return -1;
    }

    res = name_compare(r1->rdata, r2->rdata, length1, length2);
    if (res != 0) return res;

    dlen = DomainNameLength((domainname *)r1->rdata);
    offset1 += dlen;
    offset2 += dlen;
    length1 -= dlen;
    length2 -= dlen;

    if (length1 <= 1 || length2 <= 1)
    {
        LogMsg("tsig_compare: data too small to compare length1 %d, length2 %d", length1, length2);
        return -1;
    }

    return (rdata_compare(r1->rdata + offset1, r2->rdata + offset2, length1, length2));
}

// Compares types that conform to : <length><Value>
mDNSlocal int lenval_compare(mDNSu8 *d1, mDNSu8 *d2, int *len1, int *len2, int rem1, int rem2)
{
    int len;
    int res;

    if (rem1 <= 1 || rem2 <= 1)
    {
        LogMsg("lenval_compare: data too small to compare length1 %d, length2 %d", rem1, rem2);
        return -1;
    }
    *len1 = (int)d1[0];
    *len2 = (int)d2[0];
    len = (*len1 < *len2 ? *len1 : *len2);
    res = DNSMemCmp(d1, d2, len + 1);
    return res;
}

// RFC 2915: Order (2) Preference(2) and variable length: Flags Service Regexp Replacement
mDNSlocal int naptr_compare(rdataComp *const r1, rdataComp *const r2)
{
    mDNSu8 *d1 = r1->rdata;
    mDNSu8 *d2 = r2->rdata;
    int len1, len2, res;
    int length1, length2;

    length1 = r1->rdlength;
    length2 = r2->rdlength;

    // Order, Preference plus at least 1 byte
    if (length1 < 5 || length2 < 5)
    {
        LogMsg("naptr_compare: Length smaller than 18 bytes");
        return -1;
    }
    // Compare order and preference
    res = DNSMemCmp(d1, d2, 4);
    if (res != 0) return res;

    d1 += 4;
    d2 += 4;
    length1 -= 4;
    length2 -= 4;

    // Compare Flags (including the length byte)
    res = lenval_compare(d1, d2, &len1, &len2, length1, length2);
    if (res != 0) return res;
    d1 += (len1 + 1);
    d2 += (len2 + 1);
    length1 -= (len1 + 1);
    length2 -= (len2 + 1);

    // Compare Service (including the length byte)
    res = lenval_compare(d1, d2, &len1, &len2, length1, length2);
    if (res != 0) return res;
    d1 += (len1 + 1);
    d2 += (len2 + 1);
    length1 -= (len1 + 1);
    length2 -= (len2 + 1);

    // Compare regexp (including the length byte)
    res = lenval_compare(d1, d2, &len1, &len2, length1, length2);
    if (res != 0) return res;
    d1 += (len1 + 1);
    d2 += (len2 + 1);
    length1 -= (len1 + 1);
    length2 -= (len2 + 1);

    // Compare Replacement
    return name_compare(d1, d2, length1, length2);
}

// RFC 1035: MINFO: Two domain names
// RFC 1183: RP: Two domain names
mDNSlocal int dom2_compare(mDNSu8 *d1, mDNSu8 *d2, int length1, int length2)
{
    int res, dlen;

    // We need at least one byte to start with
    if (length1 < 1 || length2 < 1)
    {
        LogMsg("dom2_compare:1: data too small length1 %d, length2 %d", length1, length2);
        return -1;
    }
    res = name_compare(d1, d2, length1, length2);
    if (res != 0) return res;
    dlen = DomainNameLength((domainname *)d1);

    length1 -= dlen;
    length2 -= dlen;
    // We need at least one byte to start with
    if (length1 < 1 || length2 < 1)
    {
        LogMsg("dom2_compare:2: data too small length1 %d, length2 %d", length1, length2);
        return -1;
    }

    d1 += dlen;
    d2 += dlen;

    return name_compare(d1, d2, length1, length2);
}

// MX : preference (2 bytes), domainname
mDNSlocal int mx_compare(rdataComp *const r1, rdataComp *const r2)
{
    int res;
    int length1, length2;

    length1 = r1->rdlength;
    length2 = r2->rdlength;

    // We need at least two bytes + 1 extra byte for the domainname to start with
    if (length1 < 3 || length2 < 3)
    {
        LogMsg("mx_compare: data too small length1 %d, length2 %d", length1, length2);
        return -1;
    }

    res = DNSMemCmp(r1->rdata, r2->rdata, 2);
    if (res != 0) return res;
    length1 -= 2;
    length2 -= 2;
    return name_compare(r1->rdata + 2, r2->rdata + 2, length1, length2);
}

// RFC 2163 (PX) : preference (2 bytes), map822. mapx400 (domainnames)
mDNSlocal int px_compare(rdataComp *const r1, rdataComp *const r2)
{
    int res;

    // We need at least two bytes + 1 extra byte for the domainname to start with
    if (r1->rdlength < 3 || r2->rdlength < 3)
    {
        LogMsg("px_compare: data too small length1 %d, length2 %d", r1->rdlength, r2->rdlength);
        return -1;
    }

    res = DNSMemCmp(r1->rdata, r2->rdata, 2);
    if (res != 0) return res;

    return dom2_compare(r1->rdata + 2, r2->rdata + 2, r1->rdlength - 2, r2->rdlength - 2);
}

mDNSlocal int soa_compare(rdataComp *r1, rdataComp *r2)
{
    int res, dlen;
    int offset1, offset2;
    int length1, length2;

    length1 = r1->rdlength;
    length2 = r2->rdlength;
    offset1 = offset2 = 0;

    // We need at least 20 bytes plus 1 byte for each domainname
    if (length1 < 22 || length2 < 22)
    {
        LogMsg("soa_compare:1: data too small length1 %d, length2 %d", length1, length2);
        return -1;
    }

    // There are two domainnames followed by 20 bytes of serial, refresh, retry, expire and min
    // Compare the names and then the rest of the bytes

    res = name_compare(r1->rdata, r2->rdata, length1, length2);
    if (res != 0) return res;

    dlen = DomainNameLength((domainname *)r1->rdata);

    length1 -= dlen;
    length2 -= dlen;
    if (length1 < 1 || length2 < 1)
    {
        LogMsg("soa_compare:2: data too small length1 %d, length2 %d", length1, length2);
        return -1;
    }
    offset1 += dlen;
    offset2 += dlen;

    res = name_compare(r1->rdata + offset1, r2->rdata + offset2, length1, length2);
    if (res != 0) return res;

    dlen = DomainNameLength((domainname *)r1->rdata);
    length1 -= dlen;
    length2 -= dlen;
    if (length1 < 20 || length2 < 20)
    {
        LogMsg("soa_compare:3: data too small length1 %d, length2 %d", length1, length2);
        return -1;
    }
    offset1 += dlen;
    offset2 += dlen;

    return (rdata_compare(r1->rdata + offset1, r2->rdata + offset2, length1, length2));
}

// RFC 4034 Section 6.0 states that:
//
// A canonical RR form and ordering within an RRset are required in order to
// construct and verify RRSIG RRs.
//
// This function is called to order within an RRset. We can't just do a memcmp as
// as stated in 6.3. This function is responsible for the third bullet in 6.2, where
// the RDATA has to be converted to lower case if it has domain names.
mDNSlocal int RDATACompare(const void *rdata1, const void *rdata2)
{
    rdataComp *r1 = (rdataComp *)rdata1;
    rdataComp *r2 = (rdataComp *)rdata2;

    if (r1->rrtype != r2->rrtype)
    {
        LogMsg("RDATACompare: ERROR!! comparing rdata of wrong types type1: %d, type2: %d", r1->rrtype, r2->rrtype);
        return -1;
    }
    switch (r1->rrtype)
    {
    case kDNSType_A:                // 1. Address Record
    case kDNSType_NULL:             // 10 NULL RR
    case kDNSType_WKS:              // 11 Well-known-service
    case kDNSType_HINFO:            // 13 Host information
    case kDNSType_TXT:              // 16 Arbitrary text string
    case kDNSType_X25:              // 19 X_25 calling address
    case kDNSType_ISDN:             // 20 ISDN calling address
    case kDNSType_NSAP:             // 22 NSAP address
    case kDNSType_KEY:              // 25 Security key
    case kDNSType_GPOS:             // 27 Geographical position (withdrawn)
    case kDNSType_AAAA:             // 28 IPv6 Address
    case kDNSType_LOC:              // 29 Location Information
    case kDNSType_EID:              // 31 Endpoint identifier
    case kDNSType_NIMLOC:           // 32 Nimrod Locator
    case kDNSType_ATMA:             // 34 ATM Address
    case kDNSType_CERT:             // 37 Certification record
    case kDNSType_A6:               // 38 IPv6 Address (deprecated)
    case kDNSType_SINK:             // 40 Kitchen sink (experimental)
    case kDNSType_OPT:              // 41 EDNS0 option (meta-RR)
    case kDNSType_APL:              // 42 Address Prefix List
    case kDNSType_DS:               // 43 Delegation Signer
    case kDNSType_SSHFP:            // 44 SSH Key Fingerprint
    case kDNSType_IPSECKEY:         // 45 IPSECKEY
    case kDNSType_RRSIG:            // 46 RRSIG
    case kDNSType_NSEC:             // 47 Denial of Existence
    case kDNSType_DNSKEY:           // 48 DNSKEY
    case kDNSType_DHCID:            // 49 DHCP Client Identifier
    case kDNSType_NSEC3:            // 50 Hashed Authenticated Denial of Existence
    case kDNSType_NSEC3PARAM:       // 51 Hashed Authenticated Denial of Existence
    case kDNSType_HIP:              // 55 Host Identity Protocol
    case kDNSType_SPF:              // 99 Sender Policy Framework for E-Mail
    default:
        return rdata_compare(r1->rdata, r2->rdata, r1->rdlength, r2->rdlength);
    case kDNSType_NS:               //  2 Name Server
    case kDNSType_MD:               //  3 Mail Destination
    case kDNSType_MF:               //  4 Mail Forwarder
    case kDNSType_CNAME:            //  5 Canonical Name
    case kDNSType_MB:               //  7 Mailbox
    case kDNSType_MG:               //  8 Mail Group
    case kDNSType_MR:               //  9 Mail Rename
    case kDNSType_PTR:              // 12 Domain name pointer
    case kDNSType_NSAP_PTR:         // 23 Reverse NSAP lookup (deprecated)
    case kDNSType_DNAME:            // 39 Non-terminal DNAME (for IPv6)
        return name_compare(r1->rdata, r2->rdata, r1->rdlength, r2->rdlength);
    case kDNSType_SRV:              // 33 Service record
        return srv_compare(r1, r2);
    case kDNSType_SOA:              //  6 Start of Authority
        return soa_compare(r1, r2);

    case kDNSType_RP:               // 17 Responsible person
    case kDNSType_MINFO:            // 14 Mailbox information
        return dom2_compare(r1->rdata, r2->rdata, r1->rdlength, r2->rdlength);
    case kDNSType_MX:               // 15 Mail Exchanger
    case kDNSType_AFSDB:            // 18 AFS cell database
    case kDNSType_RT:               // 21 Router
    case kDNSType_KX:               // 36 Key Exchange
        return mx_compare(r1, r2);
    case kDNSType_PX:               // 26 X.400 mail mapping
        return px_compare(r1, r2);
    case kDNSType_NAPTR:            // 35 Naming Authority PoinTeR
        return naptr_compare(r1, r2);
    case kDNSType_TKEY:             // 249 Transaction key
    case kDNSType_TSIG:             // 250 Transaction signature
        // TSIG and TKEY have a domainname followed by data
        return tsig_compare(r1, r2);
    // TBD: We are comparing them as opaque types, perhaps not right
    case kDNSType_SIG:              // 24 Security signature
    case kDNSType_NXT:              // 30 Next domain (security)
        LogMsg("RDATACompare: WARNING!! explicit support has not been added, using default");
        return rdata_compare(r1->rdata, r2->rdata, r1->rdlength, r2->rdlength);
    }
}



// RFC 4034 section 6.2 requirement for verifying signature.
//
// 3. if the type of the RR is NS, MD, MF, CNAME, SOA, MB, MG, MR, PTR,
// HINFO, MINFO, MX, HINFO, RP, AFSDB, RT, SIG, PX, NXT, NAPTR, KX,
// SRV, DNAME, A6, RRSIG, or NSEC, all uppercase US-ASCII letters in
// the DNS names contained within the RDATA are replaced by the
// corresponding lowercase US-ASCII letters;
//
// NSEC and HINFO is not needed as per dnssec-bis update. RRSIG is done elsewhere
// as part of signature verification
mDNSlocal void ConvertRDATAToCanonical(mDNSu16 rrtype, mDNSu16 rdlength, mDNSu8 *rdata)
{
    domainname name;
    int len;
    mDNSu8 *origRdata = rdata;

    // Ensure that we have at least one byte of data to examine and modify.

    if (!rdlength) { LogMsg("ConvertRDATAToCanonical: rdlength zero for rrtype %s", DNSTypeName(rrtype)); return; }

    switch (rrtype)
    {
    // Not adding suppot for A6 as it is deprecated
    case kDNSType_A6:               // 38 IPv6 Address (deprecated)
    default:
        debugdnssec("ConvertRDATAToCanonical: returning from default %s", DNSTypeName(rrtype));
        return;
    case kDNSType_NS:               //  2 Name Server
    case kDNSType_MD:               //  3 Mail Destination
    case kDNSType_MF:               //  4 Mail Forwarder
    case kDNSType_CNAME:            //  5 Canonical Name
    case kDNSType_MB:               //  7 Mailbox
    case kDNSType_MG:               //  8 Mail Group
    case kDNSType_MR:               //  9 Mail Rename
    case kDNSType_PTR:              // 12 Domain name pointer
    case kDNSType_DNAME:            // 39 Non-terminal DNAME (for IPv6)
    case kDNSType_NXT:              // 30 Next domain (security)

    // TSIG and TKEY are not mentioned in RFC 4034, but we just leave it here
    case kDNSType_TSIG:             // 250 Transaction signature
    case kDNSType_TKEY:             // 249 Transaction key

        if (DNSNameToLowerCase((domainname *)rdata, &name) != mStatus_NoError)
        {
            LogMsg("ConvertRDATAToCanonical: ERROR!! DNSNameToLowerCase failed");
            return;
        }
        AssignDomainName((domainname *)rdata, &name);
        return;
    case kDNSType_MX:               // 15 Mail Exchanger
    case kDNSType_AFSDB:            // 18 AFS cell database
    case kDNSType_RT:               // 21 Router
    case kDNSType_KX:               // 36 Key Exchange

        // format: preference - 2 bytes, followed by name
        // Ensure that we have at least 3 bytes (preference + 1 byte for the domain name)
        if (rdlength <= 3)
        {
            LogMsg("ConvertRDATAToCanonical:MX: rdlength %d for rrtype %s too small", rdlength, DNSTypeName(rrtype));
            return;
        }
        if (DNSNameToLowerCase((domainname *)(rdata + 2), &name) != mStatus_NoError)
        {
            LogMsg("ConvertRDATAToCanonical: MX: ERROR!! DNSNameToLowerCase failed");
            return;
        }
        AssignDomainName((domainname *)(rdata + 2), &name);
        return;
    case kDNSType_SRV:              // 33 Service record
        // format : priority, weight and port - 6 bytes, followed by name
        if (rdlength <= 7)
        {
            LogMsg("ConvertRDATAToCanonical:SRV: rdlength %d for rrtype %s too small", rdlength, DNSTypeName(rrtype));
            return;
        }
        if (DNSNameToLowerCase((domainname *)(rdata + 6), &name) != mStatus_NoError)
        {
            LogMsg("ConvertRDATAToCanonical: SRV: ERROR!! DNSNameToLowerCase failed");
            return;
        }
        AssignDomainName((domainname *)(rdata + 6), &name);
        return;
    case kDNSType_PX:               // 26 X.400 mail mapping
        if (rdlength <= 3)
        {
            LogMsg("ConvertRDATAToCanonical:PX: rdlength %d for rrtype %s too small", rdlength, DNSTypeName(rrtype));
            return;
        }
        // Preference followed by two domain names
        rdata += 2;
    /* FALLTHROUGH */
    case kDNSType_RP:               // 17 Responsible person
    case kDNSType_SOA:              //  6 Start of Authority
    case kDNSType_MINFO:            // 14 Mailbox information
        if (DNSNameToLowerCase((domainname *)rdata, &name) != mStatus_NoError)
        {
            LogMsg("ConvertRDATAToCanonical: SOA1: ERROR!! DNSNameToLowerCase failed");
            return;
        }

        AssignDomainName((domainname *)rdata, &name);
        len = DomainNameLength((domainname *)rdata);
        if (rdlength <= len + 1)
        {
            LogMsg("ConvertRDATAToCanonical:RP: rdlength %d for rrtype %s too small", rdlength, DNSTypeName(rrtype));
            return;
        }
        rdata += len;

        if (DNSNameToLowerCase((domainname *)rdata, &name) != mStatus_NoError)
        {
            LogMsg("ConvertRDATAToCanonical: SOA2: ERROR!! DNSNameToLowerCase failed");
            return;
        }
        AssignDomainName((domainname *)rdata, &name);
        return;
    case kDNSType_NAPTR:            // 35 Naming Authority Pointer
        // order and preference
        rdata += 4;
        // Flags (including the length byte)
        rdata += (((int) rdata[0]) + 1);
        // Service (including the length byte)
        rdata += (((int) rdata[0]) + 1);
        // regexp (including the length byte)
        rdata += (((int) rdata[0]) + 1);

        // Replacement field is a domainname. If we have at least one more byte, then we are okay.
        if ((origRdata + rdlength) < rdata + 1)
        {
            LogMsg("ConvertRDATAToCanonical:NAPTR: origRdata %p, rdlength %d, rdata %p for rrtype %s too small", origRdata, rdlength, rdata, DNSTypeName(rrtype));
            return;
        }
        if (DNSNameToLowerCase((domainname *)rdata, &name) != mStatus_NoError)
        {
            LogMsg("ConvertRDATAToCanonical: NAPTR2: ERROR!! DNSNameToLowerCase failed");
            return;
        }
        AssignDomainName((domainname *)rdata, &name);
    case kDNSType_SIG:              // 24 Security signature
        // format: <18 bytes> <domainname> <data>
        if (rdlength <= 19)
        {
            LogMsg("ConvertRDATAToCanonical:SIG: rdlength %d for rrtype %s too small", rdlength, DNSTypeName(rrtype));
            return;
        }
        // Preference followed by two domain names
        rdata += 18;
        if (DNSNameToLowerCase((domainname *)rdata, &name) != mStatus_NoError)
        {
            LogMsg("ConvertRDATAToCanonical: SIG: ERROR!! DNSNameToLowerCase failed");
            return;
        }
        AssignDomainName((domainname *)rdata, &name);
        return;
    }
}

mDNSlocal mDNSBool ValidateSignatureWithKey(DNSSECVerifier *dv, RRVerifier *rrset, RRVerifier *keyv, RRVerifier *sig)
{
    domainname name;
    domainname signerName;
    int labels;
    mDNSu8 fixedPart[MAX_DOMAIN_NAME + 8];  // domainname + type + class + ttl
    int fixedPartLen;
    RRVerifier *tmp;
    int nrrsets;
    rdataComp *ptr, *start, *p;
    rdataRRSig *rrsig;
    rdataDNSKey *key;
    int i;
    int sigNameLen;
    mDNSu16 temp;
    mStatus algRet;


    key = (rdataDNSKey *)keyv->rdata;
    rrsig = (rdataRRSig *)sig->rdata;

    LogDNSSEC("ValidateSignatureWithKey: Validating signature with key with tag %d", (mDNSu16)keytag((mDNSu8 *)key, keyv->rdlength));

    if (DNSNameToLowerCase((domainname *)&rrsig->signerName, &signerName) != mStatus_NoError)
    {
        LogMsg("ValidateSignatureWithKey: ERROR!! cannot convert signer name to lower case");
        return mDNSfalse;
    }

    if (DNSNameToLowerCase((domainname *)&rrset->name, &name) != mStatus_NoError)
    {
        LogMsg("ValidateSignatureWithKey: ERROR!! cannot convert rrset name to lower case");
        return mDNSfalse;
    }

    sigNameLen = DomainNameLength(&signerName);
    labels = CountLabels(&name);
    // RFC 4034: RRSIG validation
    //
    // signature = sign(RRSIG_RDATA | RR(1) | RR(2)... )
    //
    // where RRSIG_RDATA excludes the signature and signer name in canonical form

    if (dv->ctx) AlgDestroy(dv->ctx);
    dv->ctx = AlgCreate(CRYPTO_ALG, rrsig->alg);
    if (!dv->ctx)
    {
        LogDNSSEC("ValidateSignatureWithKey: ERROR!! No algorithm support for %d", rrsig->alg);
        return mDNSfalse;
    }
    AlgAdd(dv->ctx, (const mDNSu8 *)rrsig, RRSIG_FIXED_SIZE);
    AlgAdd(dv->ctx, signerName.c, sigNameLen);

    if (labels - rrsig->labels > 0)
    {
        domainname *d;
        LogDNSSEC("ValidateSignatureWithKey: ====splitting labels %d, rrsig->labels %d====", labels,rrsig->labels);
        d = (domainname *)SkipLeadingLabels(&name, labels - rrsig->labels);
        fixedPart[0] = 1;
        fixedPart[1] = '*';
        AssignDomainName((domainname *)(fixedPart + 2), d);
        fixedPartLen = DomainNameLength(d) + 2;
        // See RFC 4034 section 3.1.3. If you are looking up *.example.com,
        // the labels count in the RRSIG is 2, but this is not considered as
        // a wildcard answer
        if (name.c[0] != 1 || name.c[1] != '*')
        {
            LogDNSSEC("ValidateSignatureWithKey: Wildcard exapnded answer for %##s (%s)", dv->origName.c, DNSTypeName(dv->origType));
            dv->flags |= WILDCARD_PROVES_ANSWER_EXPANDED;
            dv->wildcardName = (domainname *)SkipLeadingLabels(&dv->origName, labels - rrsig->labels);
            if (!dv->wildcardName) return mDNSfalse;
        }
    }
    else
    {
        debugdnssec("ValidateSignatureWithKey: assigning domainname");
        AssignDomainName((domainname *)fixedPart, &name);
        fixedPartLen = DomainNameLength(&name);
    }
    temp = swap16(rrset->rrtype);
    mDNSPlatformMemCopy(fixedPart + fixedPartLen, (mDNSu8 *)&temp, sizeof(rrset->rrtype));
    fixedPartLen += sizeof(rrset->rrtype);
    temp = swap16(rrset->rrclass);
    mDNSPlatformMemCopy(fixedPart + fixedPartLen, (mDNSu8 *)&temp, sizeof(rrset->rrclass));
    fixedPartLen += sizeof(rrset->rrclass);
    mDNSPlatformMemCopy(fixedPart + fixedPartLen, (mDNSu8 *)&rrsig->origTTL, sizeof(rrsig->origTTL));
    fixedPartLen += sizeof(rrsig->origTTL);


    for (tmp = rrset, nrrsets = 0; tmp; tmp = tmp->next)
        nrrsets++;

    tmp = rrset;
    start = ptr = (rdataComp *) mDNSPlatformMemAllocateClear(nrrsets * sizeof(rdataComp));
    debugdnssec("ValidateSignatureWithKey: start %p, nrrsets %d", start, nrrsets);
    if (ptr)
    {
        while (tmp)
        {
            ptr->rdlength = tmp->rdlength;
            ptr->rrtype = tmp->rrtype;
            if (ptr->rdlength)
            {
                ptr->rdata = (mDNSu8 *) mDNSPlatformMemAllocate(ptr->rdlength);
                if (ptr->rdata)
                {
                    mDNSPlatformMemCopy(ptr->rdata, tmp->rdata, tmp->rdlength);
                }
                else
                {
                    for (i = 0; i < nrrsets; i++)
                        if (start[i].rdata) mDNSPlatformMemFree(start[i].rdata);
                    mDNSPlatformMemFree(start);
                    LogMsg("ValidateSignatureWithKey:1: ERROR!! RDATA memory alloation failure");
                    return mDNSfalse;
                }
            }
            ptr++;
            tmp = tmp->next;
        }
    }
    else
    {
        LogMsg("ValidateSignatureWithKey:2: ERROR!! RDATA memory alloation failure");
        return mDNSfalse;
    }

    PrintFixedSignInfo(rrsig, &signerName, sigNameLen, fixedPart, fixedPartLen);

    mDNSPlatformQsort(start, nrrsets, sizeof(rdataComp), RDATACompare);
    for (p = start, i = 0; i < nrrsets; p++, i++)
    {
        int rdlen;

        // The array is sorted and hence checking adjacent entries for duplicate is sufficient
        if (i > 0)
        {
            rdataComp *q = p - 1;
            if (!RDATACompare((void *)p, (void *)q)) continue;
        }

        // Add the fixed part
        AlgAdd(dv->ctx, (const mDNSu8 *)fixedPart, fixedPartLen);

        // Add the rdlength
        rdlen = swap16(p->rdlength);
        AlgAdd(dv->ctx, (const mDNSu8 *)&rdlen, sizeof(mDNSu16));

        ConvertRDATAToCanonical(p->rrtype, p->rdlength, p->rdata);

        PrintVarSignInfo(rdlen, p->rdata);
        AlgAdd(dv->ctx, (const mDNSu8 *)p->rdata, p->rdlength);
    }
    // free the memory as we don't need it anymore
    for (i = 0; i < nrrsets; i++)
        if (start[i].rdata) mDNSPlatformMemFree(start[i].rdata);
    mDNSPlatformMemFree(start);

    algRet = AlgVerify(dv->ctx, (mDNSu8 *)&key->data, keyv->rdlength - DNSKEY_FIXED_SIZE, (mDNSu8 *)(sig->rdata + sigNameLen + RRSIG_FIXED_SIZE), sig->rdlength - RRSIG_FIXED_SIZE - sigNameLen);
    AlgDestroy(dv->ctx);
    dv->ctx = mDNSNULL;
    if (algRet != mStatus_NoError)
    {
        LogDNSSEC("ValidateSignatureWithKey: AlgVerify failed for %##s (%s)", dv->origName.c, DNSTypeName(dv->origType));
        // Reset the state if we set any above.
        if (dv->flags & WILDCARD_PROVES_ANSWER_EXPANDED)
        {
            dv->flags &= ~WILDCARD_PROVES_ANSWER_EXPANDED;
            dv->wildcardName = mDNSNULL;
        }
        return mDNSfalse;
    }
    return mDNStrue;
}

// Walk all the keys and for each key walk all the RRSIGS that signs the original rrset
mDNSlocal mStatus ValidateSignature(DNSSECVerifier *dv, RRVerifier **resultKey, RRVerifier **resultRRSIG)
{
    RRVerifier *rrset;
    RRVerifier *keyv;
    RRVerifier *rrsigv;
    RRVerifier *sig;
    rdataDNSKey *key;
    rdataRRSig *rrsig;
    mDNSu16 tag;

    rrset = dv->rrset;
    sig = dv->rrsig;

    for (keyv = dv->key; keyv; keyv = keyv->next)
    {
        key = (rdataDNSKey *)keyv->rdata;
        tag = (mDNSu16)keytag((mDNSu8 *)key, keyv->rdlength);
        for (rrsigv = sig; rrsigv; rrsigv = rrsigv->next)
        {
            rrsig = (rdataRRSig *)rrsigv->rdata;
            // 7. The RRSIG RR's Signer's Name, Algorithm, and Key Tag fields MUST match the owner
            //    name, algorithm, and key tag for some DNSKEY RR in the zone's apex DNSKEY RRset.
            if (!SameDomainName((domainname *)&rrsig->signerName, &keyv->name))
            {
                debugdnssec("ValidateSignature: name mismatch");
                continue;
            }
            if (key->alg != rrsig->alg)
            {
                debugdnssec("ValidateSignature: alg mismatch");
                continue;
            }
            if (tag != swap16(rrsig->keyTag))
            {
                debugdnssec("ValidateSignature: keyTag mismatch rrsig tag %d(0x%x), keyTag %d(0x%x)", swap16(rrsig->keyTag),
                            swap16(rrsig->keyTag), tag, tag);
                continue;
            }
            // 8. The matching DNSKEY RR MUST be present in the zone's apex DNSKEY RRset, and MUST
            //    have the Zone Flag bit (DNSKEY RDATA Flag bit 7) set.
            if (!((swap16(key->flags)) & DNSKEY_ZONE_SIGN_KEY))
            {
                debugdnssec("ValidateSignature: ZONE flag bit not set");
                continue;
            }
            debugdnssec("ValidateSignature:Found a key and RRSIG tag: %d", tag);
            if (ValidateSignatureWithKey(dv, rrset, keyv, rrsigv))
            {
                LogDNSSEC("ValidateSignature: Validated successfully with key tag %d", tag);
                *resultKey = keyv;
                *resultRRSIG = rrsigv;
                return mStatus_NoError;
            }
        }
    }
    *resultKey = mDNSNULL;
    *resultRRSIG = mDNSNULL;
    return mStatus_NoSuchRecord;
}

mDNSlocal mDNSBool ValidateSignatureWithKeyForAllRRSigs(DNSSECVerifier *dv, RRVerifier *rrset, RRVerifier *keyv, RRVerifier *sig)
{
    rdataRRSig *rrsig;
    mDNSu16 tag;

    while (sig)
    {
        rrsig = (rdataRRSig *)sig->rdata;
        tag = (mDNSu16)keytag(keyv->rdata, keyv->rdlength);
        if (tag == swap16(rrsig->keyTag))
        {
            if (ValidateSignatureWithKey(dv, rrset, keyv, sig))
            {
                LogDNSSEC("ValidateSignatureWithKeyForAllRRSigs: Validated");
                return mDNStrue;
            }
        }
        sig = sig->next;
    }
    return mDNSfalse;
}

mDNSlocal mStatus ValidateDS(DNSSECVerifier *dv)
{
    mDNSu8 *digest;
    int digestLen;
    domainname name;
    rdataRRSig *rrsig;
    rdataDS *ds;
    rdataDNSKey *key;
    RRVerifier *keyv;
    RRVerifier *dsv;
    mStatus algRet;

    rrsig = (rdataRRSig *)dv->rrsig->rdata;

    // Walk all the DS Records to see if we have a matching DNS KEY record that verifies
    // the hash. If we find one, verify that this key was used to sign the KEY rrsets in
    // this zone. Loop till we find one.
    for (dsv = dv->ds; dsv; dsv = dsv->next)
    {
        ds = (rdataDS *)dsv->rdata;
        if ((ds->digestType != SHA1_DIGEST_TYPE) && (ds->digestType != SHA256_DIGEST_TYPE))
        {
            LogDNSSEC("ValidateDS: Unsupported digest %d", ds->digestType);
            return mStatus_BadParamErr;
        }
        else debugdnssec("ValidateDS: digest type %d", ds->digestType);
        for (keyv = dv->key; keyv; keyv = keyv->next)
        {
            key = (rdataDNSKey *)keyv->rdata;
            mDNSu16 tag = (mDNSu16)keytag((mDNSu8 *)key, keyv->rdlength);
            if (tag != swap16(ds->keyTag))
            {
                debugdnssec("ValidateDS:Not a valid keytag %d", tag);
                continue;
            }

            if (DNSNameToLowerCase((domainname *)&rrsig->signerName, &name) != mStatus_NoError)
            {
                LogMsg("ValidateDS: ERROR!! cannot convert to lower case");
                continue;
            }

            if (dv->ctx) AlgDestroy(dv->ctx);
            dv->ctx = AlgCreate(DIGEST_ALG, ds->digestType);
            if (!dv->ctx)
            {
                LogMsg("ValidateDS: ERROR!! Cannot allocate context");
                continue;
            }
            digest = (mDNSu8 *)&ds->digest;
            digestLen = dsv->rdlength - DS_FIXED_SIZE;

            AlgAdd(dv->ctx, name.c, DomainNameLength(&name));
            AlgAdd(dv->ctx, (const mDNSu8 *)key, keyv->rdlength);

            algRet = AlgVerify(dv->ctx, mDNSNULL, 0, digest, digestLen);
            AlgDestroy(dv->ctx);
            dv->ctx = mDNSNULL;
            if (algRet == mStatus_NoError)
            {
                LogDNSSEC("ValidateDS: DS Validated Successfully, need to verify the key %d", tag);
                // We found the DNS KEY that is authenticated by the DS in our parent zone. Check to see if this key
                // was used to sign the DNS KEY RRSET. If so, then the keys in our DNS KEY RRSET are valid
                if (ValidateSignatureWithKeyForAllRRSigs(dv, dv->key, keyv, dv->rrsigKey))
                {
                    LogDNSSEC("ValidateDS: DS Validated Successfully %d", tag);
                    return mStatus_NoError;
                }
            }
        }
    }
    return mStatus_NoSuchRecord;
}

mDNSlocal mDNSBool UnlinkRRVerifier(DNSSECVerifier *dv, RRVerifier *elem, RRVerifierSet set)
{
    RRVerifier **v;

    switch (set)
    {
    case RRVS_rr:
        v = &dv->rrset;
        break;
    case RRVS_rrsig:
        v = &dv->rrsig;
        break;
    case RRVS_key:
        v = &dv->key;
        break;
    case RRVS_rrsig_key:
        v = &dv->rrsigKey;
        break;
    case RRVS_ds:
        v = &dv->ds;
        break;
    default:
        LogMsg("UnlinkRRVerifier: ERROR!! default case %d", set);
        return mDNSfalse;
    }
    while (*v && *v != elem)
        v = &(*v)->next;
    if (!(*v))
    {
        LogMsg("UnlinkRRVerifier: ERROR!! cannot find element in set %d", set);
        return mDNSfalse;
    }
    *v = elem->next;                  // Cut this record from the list
    elem->next = mDNSNULL;
    return mDNStrue;
}

// This can link a single AuthChain element or a list of AuthChain elements to
// DNSSECVerifier. The latter happens when we have multiple NSEC proofs and
// we gather up all the proofs in one place.
mDNSexport void AuthChainLink(DNSSECVerifier *dv, AuthChain *ae)
{
    AuthChain *head;

    LogDNSSEC("AuthChainLink: called");

    head = ae;
    // Get to the last element
    while (ae->next)
        ae = ae->next;
    *(dv->actail) = head;                // Append this record to tail of auth chain
    dv->actail = &(ae->next);          // Advance tail pointer
}

mDNSlocal mDNSBool AuthChainAdd(DNSSECVerifier *dv, RRVerifier *resultKey, RRVerifier *resultRRSig)
{
    AuthChain *ae;
    rdataDNSKey *key;
    mDNSu16 tag;

    if (!dv->rrset || !resultKey || !resultRRSig)
    {
        LogMsg("AuthChainAdd: ERROR!! input argument NULL");
        return mDNSfalse;
    }

    // Unlink resultKey and resultRRSig and store as part of AuthChain
    if (!UnlinkRRVerifier(dv, resultKey, RRVS_key))
    {
        LogMsg("AuthChainAdd: ERROR!! cannot unlink key");
        return mDNSfalse;
    }
    if (!UnlinkRRVerifier(dv, resultRRSig, RRVS_rrsig))
    {
        LogMsg("AuthChainAdd: ERROR!! cannot unlink rrsig");
        return mDNSfalse;
    }

    ae = (AuthChain *) mDNSPlatformMemAllocateClear(sizeof(*ae));
    if (!ae)
    {
        LogMsg("AuthChainAdd: AuthChain alloc failure");
        return mDNSfalse;
    }

    ae->next  = mDNSNULL;
    ae->rrset = dv->rrset;
    dv->rrset = mDNSNULL;

    ae->rrsig = resultRRSig;
    ae->key   = resultKey;

    key = (rdataDNSKey *)resultKey->rdata;
    tag = (mDNSu16)keytag((mDNSu8 *)key, resultKey->rdlength);
    LogDNSSEC("AuthChainAdd: inserting AuthChain element with rrset %##s (%s), DNSKEY tag %d", ae->rrset->name.c, DNSTypeName(ae->rrset->rrtype), tag);

    AuthChainLink(dv, ae);
    return mDNStrue;
}
	
// RFC 4035: Section 5.3.3
//
// If the resolver accepts the RRset as authentic, the validator MUST set the TTL of
// the RRSIG RR and each RR in the authenticated RRset to a value no greater than the
// minimum of:
//
//   o  the RRset's TTL as received in the response;
//
//   o  the RRSIG RR's TTL as received in the response;
//
//   o  the value in the RRSIG RR's Original TTL field; and
//
//   o  the difference of the RRSIG RR's Signature Expiration time and the
//      current time.
mDNSlocal void SetTTLRRSet(mDNS *const m, DNSSECVerifier *dv, DNSSECStatus status)
{
    DNSQuestion question;
    CacheRecord *rr;
    RRVerifier *rrsigv;
    rdataRRSig *rrsig;
    CacheGroup *cg;
    mDNSu32 rrTTL, rrsigTTL, rrsigOrigTTL, rrsigTimeTTL;
    domainname *qname;
    mDNSu16 qtype;
    CacheRecord *rrsigRR;
    mDNSs32 now;

    debugdnssec("SetTTLRRSet called");

    if (status == DNSSEC_Insecure || status == DNSSEC_Indeterminate)
    {
        LogDNSSEC("SetTTLRRSET: not setting ttl for status %s", DNSSECStatusName(status));
        return;
    }

    mDNS_Lock(m);
    now = m->timenow;
    mDNS_Unlock(m);

    mDNSPlatformMemZero(&question, sizeof(DNSQuestion));
    rrTTL = rrsigTTL = rrsigOrigTTL = rrsigTimeTTL = 0;

    // 1. Locate the rrset name and get its TTL (take the first one as a representative
    // of the rrset). Ideally, we should set the TTL on the first validation. Instead,
    // we do it whenever we validate which happens whenever a ValidationRequired question
    // finishes validation. 
    qname = &dv->origName;
    qtype = dv->origType;

    question.ThisQInterval = -1;
    InitializeQuestion(m, &question, dv->InterfaceID, qname, qtype, mDNSNULL, mDNSNULL);
    cg = CacheGroupForName(m, question.qnamehash, &question.qname);

    if (!cg)
    {
        LogMsg("SetTTLRRSet cg NULL for %##s (%s)", dv->origName.c, DNSTypeName(dv->origType));
        return;
    }

    for (rr = cg->members; rr; rr = rr->next)
        if (SameNameCacheRecordAnswersQuestion(rr, &question))
        {
            // originalttl is never touched. The actual TTL is derived based on when it was
            // received.
            rrTTL = rr->resrec.rroriginalttl - (now - rr->TimeRcvd)/mDNSPlatformOneSecond;
            break;
        }

    // Should we check to see if it matches the record in dv->ac->rrset ?
    if (!rr)
    {
        LogMsg("SetTTLRRSet: ERROR!! cannot locate main rrset for %##s (%s)", qname->c, DNSTypeName(qtype));
        return;
    }


    // 2. Get the RRSIG ttl. For NSEC records we need to get the NSEC record's TTL as
    // the negative cache record that we created may not be right.

    if (dv->ac && dv->ac->rrsig)
    {
        rrsigv = dv->ac->rrsig;
        rrsig = (rdataRRSig *)rrsigv->rdata;
    }
    else
    {
        rrsigv = mDNSNULL;
        rrsig = mDNSNULL;
    }

    rrsigRR = mDNSNULL;
    if (rr->resrec.RecordType == kDNSRecordTypePacketNegative && status == DNSSEC_Secure)
    {
        CacheRecord *ncr;
        rrTTL = 0;
        for (ncr = rr->nsec; ncr; ncr = ncr->next)
        {
            if (ncr->resrec.rrtype == kDNSType_NSEC || ncr->resrec.rrtype == kDNSType_NSEC3)
            {
                rrTTL = ncr->resrec.rroriginalttl - (now - ncr->TimeRcvd)/mDNSPlatformOneSecond;
                debugdnssec("SetTTLRRSet: NSEC TTL %u", rrTTL);
            }
            // Note: we can't use dv->origName here as the NSEC record's RRSIG may not match
            // the original name
            if (rrsigv && ncr->resrec.rrtype == kDNSType_RRSIG && SameDomainName(ncr->resrec.name, &rrsigv->name))
            {
                RDataBody2 *rdb = (RDataBody2 *)ncr->resrec.rdata->u.data;
                rdataRRSig *sig = (rdataRRSig *)rdb->data;
                if (rrsigv->rdlength != ncr->resrec.rdlength)
                {
                    debugdnssec("SetTTLRRSet length mismatch");
                    continue;
                }
                if (mDNSPlatformMemSame(sig, rrsig, rrsigv->rdlength))
                {
                    mDNSu32 remain = (now - ncr->TimeRcvd)/mDNSPlatformOneSecond;
                    rrsigTTL = ncr->resrec.rroriginalttl - remain;
                    rrsigOrigTTL = swap32(rrsig->origTTL) - remain;
                    rrsigTimeTTL = swap32(rrsig->sigExpireTime) - swap32(rrsig->sigInceptTime);
                }
            }
            if (rrTTL && (!rrsigv || rrsigTTL)) break;
        }
    }
    else if (rrsigv)
    {
        // Look for the matching RRSIG so that we can get its TTL
        for (rr = cg ? cg->members : mDNSNULL; rr; rr=rr->next)
            if (rr->resrec.rrtype == kDNSType_RRSIG && SameDomainName(rr->resrec.name, &rrsigv->name))
            {
                RDataBody2 *rdb = (RDataBody2 *)rr->resrec.rdata->u.data;
                rdataRRSig *sig = (rdataRRSig *)rdb->data;
                if (rrsigv->rdlength != rr->resrec.rdlength)
                {
                    debugdnssec("SetTTLRRSet length mismatch");
                    continue;
                }
                if (mDNSPlatformMemSame(sig, rrsig, rrsigv->rdlength))
                {
                    mDNSu32 remain = (now - rr->TimeRcvd)/mDNSPlatformOneSecond;
                    rrsigTTL = rr->resrec.rroriginalttl - remain;
                    rrsigOrigTTL = swap32(rrsig->origTTL) - remain;
                    rrsigTimeTTL = swap32(rrsig->sigExpireTime) - swap32(rrsig->sigInceptTime);
                    rrsigRR = rr;
                    break;
                }
            }
    }

    // It is possible that there are no RRSIGs and in that case it is not an error
    // to find the rrsigTTL.
    if (!rrTTL || (rrsigv && (!rrsigTTL || !rrsigOrigTTL || !rrsigTimeTTL)))
    {
        LogDNSSEC("SetTTLRRSet: ERROR!! Bad TTL rrtl %u, rrsigTTL %u, rrsigOrigTTL %u, rrsigTimeTTL %u for %##s (%s)",
               rrTTL, rrsigTTL, rrsigOrigTTL, rrsigTimeTTL, qname->c, DNSTypeName(qtype));
        return;
    }
    LogDNSSEC("SetTTLRRSet: TTL rrtl %u, rrsigTTL %u, rrsigOrigTTL %u, rrsigTimeTTL %u for %##s (%s)",
        rrTTL, rrsigTTL, rrsigOrigTTL, rrsigTimeTTL, qname->c, DNSTypeName(qtype));

    if (status == DNSSEC_Bogus)
    {
        rrTTL = RR_BOGUS_TTL;
        LogDNSSEC("SetTTLRRSet: setting to bogus TTL %d", rrTTL);
    }

    if (rrsigv)
    {
        if (rrsigTTL < rrTTL)
            rrTTL = rrsigTTL;
        if (rrsigOrigTTL < rrTTL)
            rrTTL = rrsigOrigTTL;
        if (rrsigTimeTTL < rrTTL)
            rrTTL = rrsigTimeTTL;
    }

    // Set the rrsig's TTL. For NSEC records, rrsigRR is NULL which means it expires when
    // the negative cache record expires.
    if (rrsigRR)
    {
        rrsigRR->resrec.rroriginalttl = rrTTL;
        rrsigRR->TimeRcvd = now;
        rrsigRR->UnansweredQueries = 0;
    }

    // Find the RRset and set its TTL
    for (rr = cg ? cg->members : mDNSNULL; rr; rr=rr->next)
    {
        if (SameNameCacheRecordAnswersQuestion(rr, &question))
        {
            LogDNSSEC("SetTTLRRSet: Setting the TTL %d for %s, question %##s (%s)", rrTTL, CRDisplayString(m, rr),
                      question.qname.c, DNSTypeName(rr->resrec.rrtype));
            rr->resrec.rroriginalttl = rrTTL;
            rr->TimeRcvd = now;
            rr->UnansweredQueries = 0;
            SetNextCacheCheckTimeForRecord(m, rr);
        }
    }
}

mDNSlocal void FinishDNSSECVerification(mDNS *const m, DNSSECVerifier *dv)
{
    RRVerifier *resultKey;
    RRVerifier *resultRRSig;

    LogDNSSEC("FinishDNSSECVerification: all rdata sets available for sig verification for %##s (%s)",
              dv->origName.c, DNSTypeName(dv->origType));

    // Stop outstanding query if one exists
    if (dv->q.ThisQInterval != -1)
        mDNS_StopQuery(m, &dv->q);
    if (ValidateSignature(dv, &resultKey, &resultRRSig) == mStatus_NoError)
    {
        rdataDNSKey *key;
        mDNSu16 tag;
        key = (rdataDNSKey *)resultKey->rdata;
        tag = (mDNSu16)keytag((mDNSu8 *)key, resultKey->rdlength);

        LogDNSSEC("FinishDNSSECVerification: RRSIG validated by DNSKEY tag %d, %##s (%s)", tag, dv->rrset->name.c,
                  DNSTypeName(dv->rrset->rrtype));

        if (TrustedKey(m, dv) == mStatus_NoError)
        {
            // Need to call this after we called TrustedKey, as AuthChainAdd
            // unlinks the resultKey and resultRRSig
            if (!AuthChainAdd(dv, resultKey, resultRRSig))
            {
                dv->DVCallback(m, dv, DNSSEC_Bogus);
                return;
            }
            // The callback will be called when NSEC verification is done.
            if ((dv->flags & WILDCARD_PROVES_ANSWER_EXPANDED))
            {
                WildcardAnswerProof(m, dv);
                return;
            }
            else
            {
                dv->DVCallback(m, dv, DNSSEC_Secure);
                return;
            }
        }
        if (!ValidateDS(dv))
        {
            // Need to call this after we called ValidateDS, as AuthChainAdd
            // unlinks the resultKey and resultRRSig
            if (!AuthChainAdd(dv, resultKey, resultRRSig))
            {
                dv->DVCallback(m, dv, DNSSEC_Bogus);
                return;
            }
            FreeDNSSECVerifierRRSets(dv);
            dv->recursed++;
            if (dv->recursed < MAX_RECURSE_COUNT)
            {
                LogDNSSEC("FinishDNSSECVerification: Recursion level %d for %##s (%s)", dv->recursed, dv->origName.c,
                          DNSTypeName(dv->origType));
                VerifySignature(m, dv, &dv->q);
                return;
            }
        }
        else
        {
            LogDNSSEC("FinishDNSSECVerification: ValidateDS failed %##s (%s)", dv->rrset->name.c, DNSTypeName(dv->rrset->rrtype));
            dv->DVCallback(m, dv, DNSSEC_Bogus);
            return;
        }
    }
    else
    {
        LogDNSSEC("FinishDNSSECVerification: Could not validate the rrset %##s (%s)", dv->origName.c, DNSTypeName(dv->origType));
        dv->DVCallback(m, dv, DNSSEC_Bogus);
        return;
    }
}

mDNSexport void StartDNSSECVerification(mDNS *const m, void *context)
{
    mDNSBool done;
    DNSSECVerifier *dv = (DNSSECVerifier *)context;

    done = GetAllRRSetsForVerification(m, dv);
    if (done)
    {
        if (dv->next != RRVS_done)
            LogMsg("StartDNSSECVerification: ERROR!! dv->next is not done");
        else
            LogDNSSEC("StartDNSSECVerification: all rdata sets available for sig verification");
        FinishDNSSECVerification(m, dv);
        return;
    }
    else debugdnssec("StartDNSSECVerification: all rdata sets not available for sig verification next %d", dv->next);
}

mDNSexport char *DNSSECStatusName(DNSSECStatus status)
{
    switch (status)
    {
    case DNSSEC_Secure: return "Secure";
    case DNSSEC_Insecure: return "Insecure";
    case DNSSEC_Indeterminate: return "Indeterminate";
    case DNSSEC_Bogus: return "Bogus";
    default: return "Invalid";
    }
}

// We could not use GenerateNegativeResponse as it assumes m->CurrentQuestion to be set. Even if
// we change that, we needs to fix its callers and so on. It is much simpler to call the callback.
mDNSlocal void DeliverDNSSECStatus(mDNS *const m, DNSSECVerifier *dv, ResourceRecord *answer, DNSSECStatus status)
{

    // Can't use m->CurrentQuestion as it may already be in use
    if (m->ValidationQuestion)
        LogMsg("DeliverDNSSECStatus: ERROR!! m->ValidationQuestion already set: %##s (%s)",
               m->ValidationQuestion->qname.c, DNSTypeName(m->ValidationQuestion->qtype));

    BumpDNSSECStats(m, kStatsActionSet, kStatsTypeStatus, status);
    BumpDNSSECStats(m, kStatsActionSet, kStatsTypeExtraPackets, dv->NumPackets);
    mDNS_Lock(m);
    BumpDNSSECStats(m, kStatsActionSet, kStatsTypeLatency, m->timenow - dv->StartTime);
    mDNS_Unlock(m);

    m->ValidationQuestion = m->Questions;
    while (m->ValidationQuestion && m->ValidationQuestion != m->NewQuestions)
    {
        DNSQuestion *q = m->ValidationQuestion;

        if (q->ValidatingResponse || !q->ValidationRequired ||
           (q->ValidationState != DNSSECValInProgress) || !ResourceRecordAnswersQuestion(answer, q))
        {
            m->ValidationQuestion = q->next;
            continue;
        }

        q->ValidationState = DNSSECValDone;
        q->ValidationStatus = status;

        MakeNegativeCacheRecord(m, &largerec.r, &q->qname, q->qnamehash, q->qtype, q->qclass, 60, mDNSInterface_Any, mDNSNULL);
        if (q->qtype == answer->rrtype || status != DNSSEC_Secure)
        {
            LogDNSSEC("DeliverDNSSECStatus: Generating dnssec status %s for %##s (%s)", DNSSECStatusName(status),
                q->qname.c, DNSTypeName(q->qtype));
            if (q->QuestionCallback)
            {
                if (q->DNSSECAuthInfo)
                    FreeDNSSECAuthChainInfo((AuthChain *)q->DNSSECAuthInfo);
                q->DNSSECAuthInfo = AuthChainCopy(dv->ac);
                q->DAIFreeCallback = FreeAuthChain;
                q->QuestionCallback(m, q, &largerec.r.resrec, QC_dnssec);
            }
        }
        else if (FollowCNAME(q, answer, QC_add))
        {
            LogDNSSEC("DeliverDNSSECStatus: Following CNAME dnssec status %s for %##s (%s)", DNSSECStatusName(status),
                q->qname.c, DNSTypeName(q->qtype));
            mDNS_Lock(m); 
            AnswerQuestionByFollowingCNAME(m, q, answer);
            mDNS_Unlock(m);
        }

        if (m->ValidationQuestion == q)    // If m->ValidationQuestion was not auto-advanced, do it ourselves now
            m->ValidationQuestion = q->next;
    }
    m->ValidationQuestion = mDNSNULL;
}

// There is no work to be done if we could not validate DNSSEC (as the actual response for
// the query has already been delivered) except in the case of CNAMEs where we did not follow
// CNAMEs until we finished the DNSSEC processing.
mDNSlocal void DNSSECNoResponse(mDNS *const m, DNSSECVerifier *dv)
{
    CacheGroup *cg;
    CacheRecord *cr;
    mDNSu32 namehash;
    ResourceRecord *answer = mDNSNULL;

    LogDNSSEC("DNSSECNoResponse: called");

    if (dv->ValidationRequired != DNSSEC_VALIDATION_SECURE_OPTIONAL)
    {
        LogMsg("DNSSECNoResponse: ERROR!! ValidationRequired incorrect %d", dv->ValidationRequired);
        return;
    }

    BumpDNSSECStats(m, kStatsActionSet, kStatsTypeStatus, DNSSEC_NoResponse);

    namehash = DomainNameHashValue(&dv->origName);

    cg = CacheGroupForName(m, namehash, &dv->origName);
    if (!cg)
    {
        LogDNSSEC("DNSSECNoResponse: cg NULL for %##s (%s)", dv->origName.c, DNSTypeName(dv->origType));
        goto done;
    }

    InitializeQuestion(m, &dv->q, dv->InterfaceID, &dv->origName, dv->origType, mDNSNULL, mDNSNULL);

    // We don't have to reset ValidatingResponse (unlike in DeliverDNSSECStatus) as there are no
    // RRSIGs that can match the original question
    for (cr = cg->members; cr; cr = cr->next)
    {
        if (SameNameCacheRecordAnswersQuestion(cr, &dv->q))
        {
            answer = &cr->resrec;
            break;
        }
    }

    // It is not an error for things to disappear underneath
    if (!answer)
    {
        LogDNSSEC("DNSSECNoResponse: answer NULL for %##s, %s", dv->origName.c, DNSTypeName(dv->origType));
        goto done;
    }
    if (answer->rrtype == kDNSType_RRSIG)
    {
        LogDNSSEC("DNSSECNoResponse: RRSIG present for %##s, %s", dv->origName.c, DNSTypeName(dv->origType));
        goto done;
    }

    // Can't use m->CurrentQuestion as it may already be in use
    if (m->ValidationQuestion)
        LogMsg("DNSSECNoResponse: ERROR!! m->ValidationQuestion already set: %##s (%s)",
               m->ValidationQuestion->qname.c, DNSTypeName(m->ValidationQuestion->qtype));

    m->ValidationQuestion = m->Questions;
    while (m->ValidationQuestion && m->ValidationQuestion != m->NewQuestions)
    {
        DNSQuestion *q = m->ValidationQuestion;

        if (q->ValidatingResponse || !q->ValidationRequired ||
           (q->ValidationState != DNSSECValInProgress) || !ResourceRecordAnswersQuestion(answer, q))
        {
            m->ValidationQuestion = q->next;
            continue;
        }

        // If we could not validate e.g., zone was not signed or bad delegation etc., 
        // disable validation. Ideally, for long outstanding questions, we should try again when
        // we switch networks. But for now, keep it simple.
        // 
        // Note: If we followed a CNAME with no dnssec protection, it is even more important that
        // we disable validation as we don't want to deliver a "secure" dnssec response later e.g.,
        // it is possible that the CNAME is not secure but the address records are secure. In this
        // case, we don't want to deliver the secure response later as we followed a CNAME that was
        // not protected with DNSSEC.

        q->ValidationRequired = 0;
        q->ValidationState = DNSSECValNotRequired;
        
        if (FollowCNAME(q, answer, QC_add))
        {
            LogDNSSEC("DNSSECNoResponse: Following CNAME for %##s (%s)", q->qname.c, DNSTypeName(q->qtype));

            mDNS_Lock(m); 
            AnswerQuestionByFollowingCNAME(m, q, answer);
            mDNS_Unlock(m);
        }

        if (m->ValidationQuestion == q)    // If m->ValidationQuestion was not auto-advanced, do it ourselves now
            m->ValidationQuestion = q->next;
    }
    m->ValidationQuestion = mDNSNULL;

done:
    FreeDNSSECVerifier(m, dv);
}

mDNSlocal void DNSSECPositiveValidationCB(mDNS *const m, DNSSECVerifier *dv, CacheGroup *cg, ResourceRecord *answer, DNSSECStatus status)
{
    RRVerifier *rrset;
    RRVerifier *rv;
    CacheRecord *cr;
    mDNSu16 rrtype, rrclass;
    CacheRecord *const lrr = &largerec.r;

    LogDNSSEC("DNSSECPositiveValidationCB: called %s for %##s (%s)", DNSSECStatusName(status), dv->origName.c, DNSTypeName(dv->origType));

    //
    // 1. Check to see if the rrset that was validated is the same as in cache. If they are not same,
    //    this validation result is not valid. When the rrset changed while the validation was in
    //    progress, the act of delivering the changed rrset again should have kicked off another
    //    verification.
    //
    // 2. Walk the question list to find the matching question. The original question that started
    //    the DNSSEC verification may or may not be there. As long as there is a matching question
    //    and waiting for the response, deliver the response.
    //
    // 3. If we are answering with CNAME, it is time to follow the CNAME if the response is secure

    if (!dv->ac || status == DNSSEC_Insecure)
    {
        // For Insecure status, the auth chain contains information about the trust
        // chain starting from the known trust anchor. The rrsets are not related to
        // the origName like in Bogus or Secure.
        if (!answer)
            LogMsg("DNSSECPositiveValidationCB: ERROR: answer NULL");
    }
    else
    {
        if (!dv->ac->rrset)
        {
            LogMsg("DNSSECPositiveValidationCB: ERROR!! Validated RRSET NULL");
            goto done;
        }

        rrset = dv->ac->rrset;
        rrtype = rrset->rrtype;
        rrclass = rrset->rrclass;
    
        lrr->resrec.name = &largerec.namestorage;

        for (rv = dv->ac->rrset; rv; rv = rv->next)
            rv->found = 0;

        // Check to see if we can find all the elements in the rrset
        for (cr = cg ? cg->members : mDNSNULL; cr; cr = cr->next)
        {
            if (cr->resrec.rrtype == rrtype && cr->resrec.rrclass == rrclass)
            {
                for (rv = dv->ac->rrset; rv; rv = rv->next)
                {
                    if (rv->rdlength == cr->resrec.rdlength && rv->rdatahash == cr->resrec.rdatahash)
                    {
                        lrr->resrec.namehash = rv->namehash;
                        lrr->resrec.rrtype = rv->rrtype;
                        lrr->resrec.rrclass = rv->rrclass;
                        lrr->resrec.rdata = (RData*)&lrr->smallrdatastorage;
                        lrr->resrec.rdata->MaxRDLength = MaximumRDSize;

                        // Convert the "rdata" to a suitable form before we can call SameRDataBody which expects
                        // some of the resource records in host order and also domainnames fully expanded. We
                        // converted the resource records into network order for verification purpose and hence
                        // need to convert them back again before comparing them.
                        if (!SetRData(mDNSNULL, rv->rdata, rv->rdata + rv->rdlength, &largerec, rv->rdlength))
                        {
                            LogMsg("DNSSECPositiveValidationCB: SetRData failed for %##s (%s)", rv->name.c, DNSTypeName(rv->rrtype));
                        }
                        else if (SameRDataBody(&cr->resrec, &lrr->resrec.rdata->u, SameDomainName))
                        {
                            answer = &cr->resrec;
                            rv->found = 1;
                            break;
                        }
                    }
                }
                if (!rv)
                {
                    // The validated rrset does not have the element in the cache, re-validate
                    LogDNSSEC("DNSSECPositiveValidationCB: CacheRecord %s, not found in the validated set", CRDisplayString(m, cr));
                    goto done;
                }
            }
        }
        // Check to see if we have elements that were not in the cache
        for (rv = dv->ac->rrset; rv; rv = rv->next)
        {
            if (!rv->found)
            {
                // We had more elements in the validated set, re-validate
                LogDNSSEC("DNSSECPositiveValidationCB: Record %##s (%s) not found in the cache", rv->name.c, DNSTypeName(rv->rrtype));
                goto done;
            }
        }
    }

    // It is not an error for things to disappear underneath
    if (!answer)
    {
        LogDNSSEC("DNSSECPositiveValidationCB: answer NULL for %##s, %s", dv->origName.c, DNSTypeName(dv->origType));
        goto done;
    }

    DeliverDNSSECStatus(m, dv, answer, status);
    SetTTLRRSet(m, dv, status);

done:
    FreeDNSSECVerifier(m, dv);
}

mDNSlocal void DNSSECNegativeValidationCB(mDNS *const m, DNSSECVerifier *dv, CacheGroup *cg, ResourceRecord *answer, DNSSECStatus status)
{
    RRVerifier *rv;
    CacheRecord *cr;
    mDNSu16 rrtype, rrclass;
    AuthChain *ac;

    LogDNSSEC("DNSSECNegativeValidationCB: called %s for %##s (%s)", DNSSECStatusName(status), dv->origName.c, DNSTypeName(dv->origType));

    if (dv->parent)
    {
        // When NSEC/NSEC3s validation is completed, it calls the parent's DVCallback with the
        // parent DNSSECVerifier which is the original one that started the verification. It itself
        // should not have a parent. If the NSEC/NSEC3 validation results in another NSEC/NSEC3
        // validation, it should chain up via the dv->parent all the way to the top.
        LogMsg("DNSSECNegativeValidationCB: ERROR!! dv->parent is set for %##s (%s)", dv->origName.c, DNSTypeName(dv->origType));
        goto done;
    }

    // 1. Locate the negative cache record and check the cached NSEC/NSEC3 records to see if it matches the
    //    NSEC/NSEC3s that were valiated. If the cached NSEC/NSEC3s changed while the validation was in progress,
    //    we ignore the validation results.
    //
    // 2. Walk the question list to find the matching question. The original question that started
    //    the DNSSEC verification may or may not be there. As long as there is a matching question
    //    and waiting for the response, deliver the response.
    //
    if (!dv->ac || status == DNSSEC_Insecure)
    {
        // For Insecure status, the auth chain contains information about the trust
        // chain starting from the known trust anchor. The rrsets are not related to
        // the origName like in Bogus or Secure.
        if (!answer)
            LogMsg("DNSSECNegativeValidationCB: ERROR: answer NULL");
    }
    else
    {
        if (!dv->ac->rrset)
        {
            LogMsg("DNSSECNegativeValidationCB: ERROR!! Validated RRSET NULL");
            goto done;
        }

        rrtype = dv->origType;
        rrclass = dv->ac->rrset->rrclass;

        for (ac = dv->ac; ac; ac = ac->next)
        {
            for (rv = ac->rrset; rv; rv = rv->next)
            {
                if (rv->rrtype == kDNSType_NSEC || rv->rrtype == kDNSType_NSEC3)
                {
                    LogDNSSEC("DNSSECNegativeValidationCB: Record %p %##s (%s) marking zero", rv, rv->name.c, DNSTypeName(rv->rrtype));
                    rv->found = 0;
                }
            }
        }

        // Check to see if we can find all the elements in the rrset
        for (cr = cg->members; cr; cr = cr->next)
        {
            if (cr->resrec.RecordType == kDNSRecordTypePacketNegative && 
                cr->resrec.rrtype == rrtype && cr->resrec.rrclass == rrclass)
            {
                CacheRecord *ncr;
                for (ncr = cr->nsec; ncr; ncr = ncr->next)
                {
                    // We have RRSIGs for the NSECs cached there too
                    if (ncr->resrec.rrtype != kDNSType_NSEC && ncr->resrec.rrtype != kDNSType_NSEC3)
                        continue;
                    for (ac = dv->ac; ac; ac = ac->next)
                    {
                        for (rv = ac->rrset; rv; rv = rv->next)
                        {
                            if ((rv->rrtype == kDNSType_NSEC || rv->rrtype == kDNSType_NSEC3) && rv->rdlength == ncr->resrec.rdlength &&
                                rv->rdatahash == ncr->resrec.rdatahash)
                            {
                                if (SameDomainName(ncr->resrec.name, &rv->name) &&
                                    SameRDataBody(&ncr->resrec, (const RDataBody *)rv->rdata, SameDomainName))
                                {
                                    LogDNSSEC("DNSSECNegativeValidationCB: Record %p %##s (%s) marking one", rv, rv->name.c, DNSTypeName(rv->rrtype));
                                    answer = &cr->resrec;
                                    rv->found = 1;
                                    break;
                                }
                            }
                        }
                        if (rv)
                            break;
                    }
                }
                if (!rv)
                {
                    // The validated rrset does not have the element in the cache, re-validate
                    LogDNSSEC("DNSSECNegativeValidationCB: CacheRecord %s, not found in the validated set", CRDisplayString(m, cr));
                    goto done;
                }
            }
        }
        // Check to see if we have elements that were not in the cache
        for (ac = dv->ac; ac; ac = ac->next)
        {
            for (rv = ac->rrset; rv; rv = rv->next)
            {
                if (rv->rrtype == kDNSType_NSEC || rv->rrtype == kDNSType_NSEC3)
                {
                    if (!rv->found)
                    {
                        // We had more elements in the validated set, re-validate
                        LogDNSSEC("DNSSECNegativeValidationCB: Record %p %##s (%s) not found in the cache", rv, rv->name.c, DNSTypeName(rv->rrtype));
                        goto done;
                    }
                    rv->found = 0;
                }
            }
        }
    }

    // It is not an error for things to disappear underneath
    if (!answer)
    {
        LogDNSSEC("DNSSECNegativeValidationCB: answer NULL for %##s, %s", dv->origName.c, DNSTypeName(dv->origType));
        goto done;
    }

    DeliverDNSSECStatus(m, dv, answer, status); 
    SetTTLRRSet(m, dv, status);

done:
    FreeDNSSECVerifier(m, dv);
}

mDNSlocal void DNSSECValidationCB(mDNS *const m, DNSSECVerifier *dv, DNSSECStatus status)
{
    mDNSu32 namehash;
    CacheGroup *cg;
    CacheRecord *cr;

    LogDNSSEC("DNSSECValidationCB: called %s for %##s (%s)", DNSSECStatusName(status), dv->origName.c, DNSTypeName(dv->origType));

    // Currently, if we receive anything other than secure, we abort DNSSEC validation for
    // the optional case.
    if (dv->ValidationRequired == DNSSEC_VALIDATION_SECURE_OPTIONAL && status != DNSSEC_Secure)
    {
        DNSSECNoResponse(m, dv);
        return; 
    }

    if (dv->ValidationRequired == DNSSEC_VALIDATION_SECURE && !dv->InsecureProofDone && status == DNSSEC_Bogus)
    {
        dv->InsecureProofDone = 1;
        ProveInsecure(m, dv, mDNSNULL, mDNSNULL);
        return;
    }
    namehash = DomainNameHashValue(&dv->origName);

    cg = CacheGroupForName(m, namehash, &dv->origName);
    if (!cg)
    {
        LogDNSSEC("DNSSECValidationCB: cg NULL for %##s (%s)", dv->origName.c, DNSTypeName(dv->origType));
        FreeDNSSECVerifier(m, dv);
        return;
    }
    InitializeQuestion(m, &dv->q, dv->InterfaceID, &dv->origName, dv->origType, mDNSNULL, mDNSNULL);
    // Need to be reset ValidatingResponse as we are looking for the cache record that would answer
    // the original question
    dv->q.ValidatingResponse = mDNSfalse;
    for (cr = cg->members; cr; cr = cr->next)
    {
        if (SameNameCacheRecordAnswersQuestion(cr, &dv->q))
        {
            if (cr->resrec.RecordType == kDNSRecordTypePacketNegative)
                DNSSECNegativeValidationCB(m, dv, cg, &cr->resrec, status);
            else
                DNSSECPositiveValidationCB(m, dv, cg, &cr->resrec, status);
            return;
        }
    }
}

mDNSexport void VerifySignature(mDNS *const m, DNSSECVerifier *dv, DNSQuestion *q)
{
    CacheGroup *const cg = CacheGroupForName(m, q->qnamehash, &q->qname);
    CacheRecord *rr;
    mDNSBool first = mDNSfalse;
    static mDNSBool TrustAnchorsUpdated = mDNSfalse;

    LogDNSSEC("VerifySignature called for %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
    if (!TrustAnchorsUpdated)
    {
        TrustAnchorsUpdated = mDNStrue;
        UpdateTrustAnchors(m);
    }
    if (!dv)
    {
        first = mDNStrue;
        if (!q->qDNSServer || q->qDNSServer->isCell)
        {
            LogDNSSEC("VerifySignature: Disabled");
            return;
        }
        // We assume that the verifier's question has been initialized here so that ValidateWithNSECS below
        // knows what it has prove the non-existence of.
        dv = AllocateDNSSECVerifier(m, &q->qname, q->qtype, q->InterfaceID, q->ValidationRequired, DNSSECValidationCB, VerifySigCallback);
        if (!dv)
        {
            LogMsg("VerifySignature: ERROR!! memory alloc failed");
            return;
        }
    }

    // If we find a CNAME response to the question, remember what qtype
    // caused the CNAME response. origType is not sufficient as we
    // recursively validate the response and origType is initialized above
    // the first time this function is called.
    dv->currQtype = q->qtype;

    // Walk the cache and get all the rrsets for verification.
    for (rr = cg ? cg->members : mDNSNULL; rr; rr=rr->next)
        if (SameNameCacheRecordAnswersQuestion(rr, q))
        {
            // We also get called for RRSIGs which matches qtype. We don't need that here as we are
            // building rrset for matching q->qname. Checking for RRSIG type is important as otherwise
            // we would miss the CNAME answering any qtype.
            if (rr->resrec.rrtype == kDNSType_RRSIG && rr->resrec.rrtype != q->qtype)
            {
                LogDNSSEC("VerifySignature: Question %##s (%s) answered with RRSIG record %s, not using it", q->qname.c, DNSTypeName(q->qtype), CRDisplayString(m, rr));
                continue;
            }

            // See DNSSECRecordAnswersQuestion: This should never happen. NSEC records are
            // answered directly only when the qtype is NSEC. Otherwise, NSEC records are
            // used only for denial of existence and hence should go through negative cache
            // entry.
            if (rr->resrec.rrtype == kDNSType_NSEC && q->qtype != kDNSType_NSEC)
            {
                LogMsg("VerifySignature: ERROR!! Question %##s (%s) answered using NSEC record %s", q->qname.c, DNSTypeName(q->qtype), CRDisplayString(m, rr));
                continue;
            }

            // We might get a NSEC response when we first send the query out from the "core" for ValidationRequired
            // questions. Later as part of validating the response, we might get a NSEC response.
            if (rr->resrec.RecordType == kDNSRecordTypePacketNegative && DNSSECQuestion(q))
            {
                // If we can't find the NSEC, we can't validate. This can happens if we are
                // behind a non-DNSSEC aware CPE/server.
                if (!rr->nsec)
                {
                    LogDNSSEC("VerifySignature: No nsecs found for %s", CRDisplayString(m, rr));
                    dv->DVCallback(m, dv, DNSSEC_Bogus);
                    return;
                }
                ValidateWithNSECS(m, dv, rr);
                return;
            }

            if (AddRRSetToVerifier(dv, &rr->resrec, mDNSNULL, RRVS_rr) != mStatus_NoError)
            {
                dv->DVCallback(m, dv, DNSSEC_Bogus);
                return;
            }
        }
    if (!dv->rrset)
    {
        LogMsg("VerifySignature: rrset mDNSNULL for %##s (%s)", dv->origName.c, DNSTypeName(dv->origType));
        dv->DVCallback(m, dv, DNSSEC_Bogus);
        return;
    }
    dv->next = RRVS_rrsig;
    // Delay this so that the mDNS "core" can deliver all the results before
    // we can deliver the dnssec result
    if (first)
    {
        mDNSPlatformDispatchAsync(m, dv, StartDNSSECVerification);
    }
    else
    {
        StartDNSSECVerification(m, dv);
    }
}

mDNSlocal mDNSBool TrustedKeyPresent(mDNS *const m, DNSSECVerifier *dv)
{
    rdataDS *ds;
    rdataDNSKey *key;
    TrustAnchor *ta;
    RRVerifier *keyv;

    // Walk all our trusted DS Records to see if we have a matching DNS KEY record that verifies
    // the hash. If we find one, verify that this key was used to sign the KEY rrsets in
    // this zone. Loop till we find one.
    for (ta = m->TrustAnchors; ta; ta = ta->next)
    {
        ds = (rdataDS *)&ta->rds;
        if ((ds->digestType != SHA1_DIGEST_TYPE) && (ds->digestType != SHA256_DIGEST_TYPE))
        {
            LogMsg("TrustedKeyPresent: Unsupported digest %d", ds->digestType);
            continue;
        }
        else
        {
            debugdnssec("TrustedKeyPresent: digest type %d", ds->digestType);
        }
        for (keyv = dv->key; keyv; keyv = keyv->next)
        {
            key = (rdataDNSKey *)keyv->rdata;
            mDNSu16 tag = (mDNSu16)keytag((mDNSu8 *)key, keyv->rdlength);
            if (tag != ds->keyTag)
            {
                debugdnssec("TrustedKeyPresent:Not a valid keytag %d", tag);
                continue;
            }
            if (!SameDomainName(&keyv->name, &ta->zone))
            {
                debugdnssec("TrustedKeyPresent: domainame mismatch key %##s, ta %##s", keyv->name.c, ta->zone.c);
                continue;
            }
            return mDNStrue;
        }
    }
    return mDNSfalse;
}

mDNSlocal mStatus TrustedKey(mDNS *const m, DNSSECVerifier *dv)
{
    mDNSu8 *digest;
    int digestLen;
    domainname name;
    rdataRRSig *rrsig;
    rdataDS *ds;
    rdataDNSKey *key;
    TrustAnchor *ta;
    RRVerifier *keyv;
    mStatus algRet;
    mDNSu32 currTime = mDNSPlatformUTC();

    rrsig = (rdataRRSig *)dv->rrsig->rdata;

    // Walk all our trusted DS Records to see if we have a matching DNS KEY record that verifies
    // the hash. If we find one, verify that this key was used to sign the KEY rrsets in
    // this zone. Loop till we find one.
    for (ta = m->TrustAnchors; ta; ta = ta->next)
    {
        ds = (rdataDS *)&ta->rds;
        if ((ds->digestType != SHA1_DIGEST_TYPE) && (ds->digestType != SHA256_DIGEST_TYPE))
        {
            LogMsg("TrustedKey: Unsupported digest %d", ds->digestType);
            continue;
        }
        else
        {
            debugdnssec("TrustedKey: Zone %##s, digest type %d, tag %d", ta->zone.c, ds->digestType, ds->keyTag);
        }
        for (keyv = dv->key; keyv; keyv = keyv->next)
        {
            key = (rdataDNSKey *)keyv->rdata;
            mDNSu16 tag = (mDNSu16)keytag((mDNSu8 *)key, keyv->rdlength);
            if (tag != ds->keyTag)
            {
                debugdnssec("TrustedKey:Not a valid keytag %d", tag);
                continue;
            }
            if (!SameDomainName(&keyv->name, &ta->zone))
            {
                debugdnssec("TrustedKey: domainame mismatch key %##s, ta %##s", keyv->name.c, ta->zone.c);
                continue;
            }
            if (DNS_SERIAL_LT(ta->validUntil, currTime))
            {
                LogDNSSEC("TrustedKey: Expired: currentTime %d, ExpireTime %d", (int)currTime, ta->validUntil);
                continue;
            }
            if (DNS_SERIAL_LT(currTime, ta->validFrom))
            {
                LogDNSSEC("TrustedKey: Future: currentTime %d, InceptTime %d", (int)currTime, ta->validFrom);
                continue;
            }

            if (DNSNameToLowerCase((domainname *)&rrsig->signerName, &name) != mStatus_NoError)
            {
                LogMsg("TrustedKey: ERROR!! cannot convert to lower case");
                continue;
            }

            if (dv->ctx) AlgDestroy(dv->ctx);
            dv->ctx = AlgCreate(DIGEST_ALG, ds->digestType);
            if (!dv->ctx)
            {
                LogMsg("TrustedKey: ERROR!! No digest support");
                continue;
            }
            digest = ds->digest;
            digestLen = ta->digestLen;

            AlgAdd(dv->ctx, name.c, DomainNameLength(&name));
            AlgAdd(dv->ctx, (const mDNSu8 *)key, keyv->rdlength);

            algRet = AlgVerify(dv->ctx, mDNSNULL, 0, digest, digestLen);
            AlgDestroy(dv->ctx);
            dv->ctx = mDNSNULL;
            if (algRet == mStatus_NoError)
            {
                LogDNSSEC("TrustedKey: DS Validated Successfully, need to verify the key %d", tag);
                // We found the DNS KEY that is authenticated by the DS in our parent zone. Check to see if this key
                // was used to sign the DNS KEY RRSET. If so, then the keys in our DNS KEY RRSET are valid
                if (ValidateSignatureWithKeyForAllRRSigs(dv, dv->key, keyv, dv->rrsigKey))
                {
                    LogDNSSEC("TrustedKey: DS Validated Successfully %d", tag);
                    return mStatus_NoError;
                }
            }
        }
    }
    return mStatus_NoSuchRecord;
}

mDNSlocal CacheRecord* NegativeCacheRecordForRR(mDNS *const m, const ResourceRecord *const rr)
{
    mDNSu32 namehash;
    CacheGroup *cg;
    CacheRecord *cr;

    namehash = DomainNameHashValue(rr->name);
    cg = CacheGroupForName(m, namehash, rr->name);
    if (!cg)
    {
        LogMsg("NegativeCacheRecordForRR: cg null %##s", rr->name->c);
        return mDNSNULL;
    }
    for (cr=cg->members; cr; cr=cr->next)
    {
        if (cr->resrec.RecordType == kDNSRecordTypePacketNegative && (&cr->resrec == rr))
            return cr;
    }
    return mDNSNULL;
}

mDNSlocal void VerifySigCallback(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
{
    DNSSECVerifier *dv = (DNSSECVerifier *)question->QuestionContext;
    mDNSu16 rrtype;
    CacheRecord *negcr;

    debugdnssec("VerifySigCallback: AddRecord %d, dv %p", AddRecord, dv);

    if (!AddRecord)
        return;

    // After the first ADD event, we should ideally stop the question. If we don't stop
    // the question, we might get more callbacks and that can cause problems. For example,
    // in the first callback, we could start a insecure proof and while that is in progress,
    // if we get more callbacks, we will try to start another insecure proof. As we already
    // started an insecure proof, we won't start another but terminate the verification
    // process where we free the current DNSSECVerifier while the first insecure proof is
    // still referencing it.
    //
    // But there are cases below which might return if we have not received the right answer
    // yet e.g., no RRSIGs. In that case if the question is stopped, we will never get any
    // callbacks again and also we leak "dv". Hence it is important that we either process
    // the result or wait for more results. Note that the question eventually times out
    // and cleans up the "dv" i.e., we don't wait forever.
    
    if (!answer)
    {
        LogDNSSEC("VerifySigCallback: Question %##s (%s) no dnssec response", question->qname.c, DNSTypeName(question->qtype));
        mDNS_StopQuery(m, question);
        dv->DVCallback(m, dv, DNSSEC_Bogus);
        return;
    }

    LogDNSSEC("VerifySigCallback(%p): Called with record %s for question %##s (%s)", dv, RRDisplayString(m, answer), question->qname.c,
        DNSTypeName(question->qtype));
    mDNS_Lock(m);
    if ((m->timenow - question->StopTime) >= 0)
    {
        mDNS_Unlock(m);
        LogDNSSEC("VerifySigCallback: Question %##s (%s) timed out", question->qname.c, DNSTypeName(question->qtype));
        mDNS_StopQuery(m, question);
        dv->DVCallback(m, dv, DNSSEC_Bogus);
        return;
    }
    mDNS_Unlock(m);

    if (answer->RecordType == kDNSRecordTypePacketNegative)
    {
        CacheRecord *cr;
        LogDNSSEC("VerifySigCallback: Received a negative answer with record %s, AddRecord %d",
                  RRDisplayString(m, answer), AddRecord);
        mDNS_StopQuery(m, question);
        cr = NegativeCacheRecordForRR(m, answer);
        if (cr && cr->nsec)
        {
            ValidateWithNSECS(m, dv, cr);
        }
        else
        {
            
            LogDNSSEC("VerifySigCallback: Missing record (%s) Negative Cache Record %p", RRDisplayString(m, answer), cr);
            dv->DVCallback(m, dv, DNSSEC_Bogus);
        }
        return;
    }

    if (!dv->rrset)
    {
        LogMsg("VerifySigCallback: ERROR!! rrset NULL");
        mDNS_StopQuery(m, question);
        dv->DVCallback(m, dv, DNSSEC_Bogus);
        return;
    }

    rrtype = answer->rrtype;
    // Check whether we got any answers for the question. If there are no answers, we
    // can't do the verification.
    //
    // We need to look at the whole rrset for verifying the signatures. This callback gets
    // called back for each record in the rrset sequentially and we won't know when to start the
    // verification. Hence, we look for all the records in the rrset ourselves using the
    // CheckXXX function below. The caller has to ensure that all the records in the rrset are
    // added to the cache before calling this callback which happens naturally because all
    // unicast records are marked for DelayDelivery and hence added to the cache before the
    // callback is done.
    //
    // We also need the RRSIGs for the rrset to do the validation. It is possible that the
    // cache contains RRSIG records but it may not be a valid record when we filter them
    // in CheckXXX function. For example, some application can query for RRSIG records which
    // might come back with a partial set of RRSIG records from the recursive server and
    // they may not be the right ones for the current validation. In this case, we still
    // need to send the query out to get the right RRSIGs but the "core" should not answer
    // this query with the same records that we checked and found them to be unusable.
    //
    // We handle this in two ways:
    //
    // 1) AnswerNewQuestion always sends the "ValidatingResponse" query out bypassing the cache.
    //
    // 2) DNSSECRecordAnswersQuestion does not answer a question with RRSIGs matching the
    //    same name as the query until the typeCovered also matches the query's type.
    //
    // NOTE: We use "next - 1" as next always points to what we are going to fetch next and not the one
    // we are fetching currently
    switch(dv->next - 1)
    {
    case RRVS_rr:
        // Verification always starts at RRVS_rrsig (which means dv->next points at RRVS_key) as verification does
        // not begin until we have the main rrset.
        LogDNSSEC("VerifySigCallback: ERROR!! rrset %##s dv->next is RRVS_rr", dv->rrset->name.c);
        return;
    case RRVS_rrsig:
        // We can get called back with rrtype matching qtype as new records are added to the cache
        // triggered by other questions. This could potentially mean that the rrset that is being
        // validated by this "dv" whose rrsets were initialized at the beginning of the verification
        // may not be the right one. If this case happens, we will detect this at the end of validation
        // and throw away the validation results. This should not be a common case.
        if (rrtype != kDNSType_RRSIG)
        {
            LogDNSSEC("VerifySigCallback: RRVS_rrsig called with %s", RRDisplayString(m, answer));
            return;
        }
        mDNS_StopQuery(m, question);
        if (CheckRRSIGForRRSet(m, dv, &negcr) != mStatus_NoError)
        {
            LogDNSSEC("VerifySigCallback: Unable to find RRSIG for %##s (%s), question %##s", dv->rrset->name.c,
                      DNSTypeName(dv->rrset->rrtype), question->qname.c);
            dv->DVCallback(m, dv, DNSSEC_Bogus);
            return;
        }
        break;
    case RRVS_key:
        // We are waiting for the DNSKEY record and hence dv->key should be NULL. If RRSIGs are being
        // returned first, ignore them for now.
        if (dv->key)
            LogDNSSEC("VerifySigCallback: ERROR!! RRVS_key dv->key non-NULL for %##s", question->qname.c);
        if (rrtype == kDNSType_RRSIG)
        {
            LogDNSSEC("VerifySigCallback: RRVS_key rrset type %s, %##s received before DNSKEY", DNSTypeName(rrtype), question->qname.c);
            return;
        }
        if (rrtype != question->qtype)
        {
            LogDNSSEC("VerifySigCallback: ERROR!! RRVS_key rrset type %s, %##s not matching qtype %d", DNSTypeName(rrtype), question->qname.c,
                question->qtype);
            return;
        }
        mDNS_StopQuery(m, question);
        if (CheckKeyForRRSIG(m, dv, &negcr) != mStatus_NoError)
        {
            LogDNSSEC("VerifySigCallback: Unable to find DNSKEY for %##s (%s), question %##s", dv->rrset->name.c,
                      DNSTypeName(dv->rrset->rrtype), question->qname.c);
            dv->DVCallback(m, dv, DNSSEC_Bogus);
            return;
        }
        break;
    case RRVS_rrsig_key:
        // If we are in RRVS_rrsig_key, it means that we already found the relevant DNSKEYs (dv->key should be non-NULL).
        // If DNSKEY record is being returned i.e., it means it is being added to the cache, then it can't be in our
        // list.
        if (!dv->key)
            LogDNSSEC("VerifySigCallback: ERROR!! RRVS_rrsig_key dv->key NULL for %##s", question->qname.c);
        if (rrtype == question->qtype)
        {
            LogDNSSEC("VerifySigCallback: RRVS_rrsig_key rrset type %s, %##s", DNSTypeName(rrtype), question->qname.c);
            CheckOneKeyForRRSIG(dv, answer);
            return;
        }
        if (rrtype != kDNSType_RRSIG)
        {
            LogDNSSEC("VerifySigCallback: RRVS_rrsig_key rrset type %s, %##s not matching qtype %d", DNSTypeName(rrtype), question->qname.c,
                question->qtype);
            return;
        }
        mDNS_StopQuery(m, question);
        if (CheckRRSIGForKey(m, dv, &negcr) != mStatus_NoError)
        {
            LogDNSSEC("VerifySigCallback: Unable to find RRSIG for %##s (%s), question %##s", dv->rrset->name.c,
                      DNSTypeName(dv->rrset->rrtype), question->qname.c);
            dv->DVCallback(m, dv, DNSSEC_Bogus);
            return;
        }
        break;
    case RRVS_ds:
        if (rrtype == question->qtype)
        {
            LogDNSSEC("VerifySigCallback: RRVS_ds rrset type %s, %##s", DNSTypeName(rrtype), question->qname.c);
        }
        else
        {
            LogDNSSEC("VerifySigCallback: RRVS_ds rrset type %s, %##s received before DS", DNSTypeName(rrtype), question->qname.c);
        }
        mDNS_StopQuery(m, question);
        // It is not an error if we don't find the DS record as we could have
        // a trusted key. Or this is not a secure delegation which will be handled
        // below.
        if (CheckDSForKey(m, dv, &negcr) != mStatus_NoError)
        {
            LogDNSSEC("VerifySigCallback: Unable find DS for %##s (%s), question %##s", dv->rrset->name.c,
                      DNSTypeName(dv->rrset->rrtype), question->qname.c);
        }
        // dv->next is already at RRVS_done, so if we "break" from here, we will end up
        // in FinishDNSSECVerification. We should not do that if we receive a negative
        // response. For all other cases above, GetAllRRSetsForVerification handles
        // negative cache record
        if (negcr)
        {
            if (!negcr->nsec)
            {
                LogDNSSEC("VerifySigCallback: No nsec records for %##s (DS)", dv->ds->name.c);
                dv->DVCallback(m, dv, DNSSEC_Bogus);
                return;
            }
            ValidateWithNSECS(m, dv, negcr);
            return;
        }
        break;
    default:
        LogDNSSEC("VerifySigCallback: ERROR!! default case rrset %##s question %##s", dv->rrset->name.c, question->qname.c);
        mDNS_StopQuery(m, question);
        dv->DVCallback(m, dv, DNSSEC_Bogus);
        return;
    }
    if (dv->next != RRVS_done)
    {
        mDNSBool done = GetAllRRSetsForVerification(m, dv);
        if (done)
        {
            if (dv->next != RRVS_done)
                LogMsg("VerifySigCallback ERROR!! dv->next is not done");
            else
                LogDNSSEC("VerifySigCallback: all rdata sets available for sig verification");
        }
        else
        {
            LogDNSSEC("VerifySigCallback: all rdata sets not available for sig verification");
            return;
        }
    }
    FinishDNSSECVerification(m, dv);
}

mDNSlocal TrustAnchor *FindTrustAnchor(mDNS *const m, const domainname *const name)
{
    TrustAnchor *ta;
    TrustAnchor *matchTA = mDNSNULL;
    TrustAnchor *rootTA = mDNSNULL;
    int currmatch = 0;
    int match;
    mDNSu32 currTime = mDNSPlatformUTC();

    for (ta = m->TrustAnchors; ta; ta = ta->next)
    {
        if (DNS_SERIAL_LT(ta->validUntil, currTime))
        {
            LogDNSSEC("FindTrustAnchor: Expired: currentTime %d, ExpireTime %d", (int)currTime, ta->validUntil);
            continue;
        }
        if (DNS_SERIAL_LT(currTime, ta->validFrom))
        {
            LogDNSSEC("FindTrustAnchor: Future: currentTime %d, InceptTime %d", (int)currTime, ta->validFrom);
            continue;
        }

        if (SameDomainName((const domainname *)"\000", &ta->zone))
            rootTA = ta;

        match = CountLabelsMatch(&ta->zone, name);
        if (match > currmatch)
        {
            currmatch = match;
            matchTA = ta;
        }
    }
    if (matchTA)
    {
        LogDNSSEC("FindTrustAnhcor: matched %##s", matchTA->zone.c);
        return matchTA;
    }
    else if (rootTA)
    {
        LogDNSSEC("FindTrustAnhcor: matched rootTA %##s", rootTA->zone.c);
        return rootTA;
    }
    else
    {
        LogDNSSEC("FindTrustAnhcor: No Trust Anchor");
        return mDNSNULL;
    }
}

mDNSlocal void DeliverInsecureProofResultAsync(mDNS *const m, void *context)
{
    InsecureContext *ic = (InsecureContext *)context;
    ic->dv->DVCallback(m, ic->dv, ic->status);
    if (ic->q.ThisQInterval != -1)
    {
        LogMsg("DeliverInsecureProofResultAsync: ERROR!! Question %##s (%s) not stopped already", ic->q.qname.c, DNSTypeName(ic->q.qtype));
        mDNS_StopQuery(m, &ic->q);
    }
    mDNSPlatformMemFree(ic);
}

mDNSlocal void DeliverInsecureProofResult(mDNS *const m, InsecureContext *ic, DNSSECStatus status)
{
    // If the status is Bogus, restore the original auth chain before the insecure
    // proof.
    if (status == DNSSEC_Bogus)
    {
        LogDNSSEC("DeliverInsecureProofResult: Restoring the auth chain");
        if (ic->dv->ac)
        {
            FreeDNSSECAuthChainInfo(ic->dv->ac);
        }
        ResetAuthChain(ic->dv);
        ic->dv->ac = ic->dv->saveac;
        if (ic->dv->ac)
        {
            AuthChain *tmp = ic->dv->ac;
            AuthChain **tail = &tmp->next;
            while (tmp->next)
            {
                tail = &tmp->next;
                tmp = tmp->next;
            }
            ic->dv->actail = tail;
        }
        ic->dv->saveac = mDNSNULL;
    }
    else if (ic->dv->saveac)
    {
        FreeDNSSECAuthChainInfo(ic->dv->saveac);
        ic->dv->saveac = mDNSNULL;
    }
    ic->status = status;
    // Stop the question before we schedule the block so that we don't receive additional
    // callbacks again. Once the block runs, it will free the "ic" and you can't
    // have another block queued up. This can happen if we receive a callback after we
    // queue the block below.
    if (ic->q.ThisQInterval != -1)
        mDNS_StopQuery(m, &ic->q);
    mDNSPlatformDispatchAsync(m, ic, DeliverInsecureProofResultAsync);
}

mDNSlocal mDNSBool AlgorithmSupported(rdataDS *ds)
{
    switch(ds->digestType)
    {
    case SHA1_DIGEST_TYPE:
    case SHA256_DIGEST_TYPE:
        break;
    default:
        LogDNSSEC("AlgorithmSupported: Unsupported digest %d", ds->digestType);
        return mDNSfalse;
    }

    switch(ds->alg)
    {
    case CRYPTO_RSA_NSEC3_SHA1:
    case CRYPTO_RSA_SHA1:
    case CRYPTO_RSA_SHA256:
    case CRYPTO_RSA_SHA512:
        return mDNStrue;
    default:
        LogDNSSEC("AlgorithmSupported: Unsupported algorithm %d", ds->alg);
        return mDNSfalse;
    }
}

// Note: This function is called when DNSSEC results are delivered (from DeliverDNSSECStatus) and we can't deliver DNSSEC result
// again within this function as "m->ValidationQuestion" is already in use. Hence we should dispatch off the delivery of insecure
// results asynchronously.
//
// Insecure proof callback can deliver either insecure or bogus, but never secure result.
mDNSlocal void ProveInsecureCallback(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
{
    InsecureContext *ic = (InsecureContext *)question->QuestionContext;
    DNSSECVerifier *pdv = ic->dv;
    AuthChain *ac;

    (void) answer;

    if (!AddRecord)
        return;

    mDNS_Lock(m);
    if ((m->timenow - question->StopTime) >= 0)
    {
        mDNS_Unlock(m);
        LogDNSSEC("ProveInsecureCallback: Question %##s (%s) timed out", question->qname.c, DNSTypeName(question->qtype));
        DeliverInsecureProofResult(m, ic, DNSSEC_Bogus);
        return;
    }
    mDNS_Unlock(m);

    // We only need to handle the actual DNSSEC results and the ones that are secure. Anything else results in
    // bogus.
    if (AddRecord != QC_dnssec)
    {
        LogDNSSEC("ProveInsecureCallback: Question %##s (%s), AddRecord %d, answer %s", question->qname.c,
            DNSTypeName(question->qtype), AddRecord, RRDisplayString(m, answer));
        return;
    }

    LogDNSSEC("ProveInsecureCallback: ic %p Question %##s (%s), DNSSEC status %s", ic, question->qname.c, DNSTypeName(question->qtype),
            DNSSECStatusName(question->ValidationStatus));

    // Insecure is delivered for NSEC3 OptOut 
    if (question->ValidationStatus != DNSSEC_Secure && question->ValidationStatus != DNSSEC_Insecure)
    {
        LogDNSSEC("ProveInsecureCallback: Question %##s (%s) returned DNSSEC status %s", question->qname.c,
            DNSTypeName(question->qtype), DNSSECStatusName(question->ValidationStatus));
        goto done;
    }
    ac = (AuthChain *)question->DNSSECAuthInfo;
    if (!ac)
    {
        LogDNSSEC("ProveInsecureCallback: ac NULL for question %##s, %s", question->qname.c, DNSTypeName(question->qtype));
        goto done;
    }
    if (!ac->rrset)
    {
        LogDNSSEC("ProveInsecureCallback: ac->rrset NULL for question %##s, %s", question->qname.c, DNSTypeName(question->qtype));
        goto done;
    }
    if (ac->rrset->rrtype != kDNSType_DS && ac->rrset->rrtype != kDNSType_NSEC && ac->rrset->rrtype != kDNSType_NSEC3)
    {
        LogDNSSEC("ProveInsecureCallback: ac->rrset->rrtype %##s (%s) not handled", ac->rrset->name.c,
            DNSTypeName(ac->rrset->rrtype));
        goto done;
    }
    AuthChainLink(pdv, ac);
    question->DNSSECAuthInfo = mDNSNULL;
    if (ac->rrset->rrtype == kDNSType_DS)
    {
        rdataDS *ds = (rdataDS *)ac->rrset->rdata;

        // If the delegation is secure, but the underlying zone is signed with an unsupported
        // algorithm, then we can't verify it. Deliver insecure in that case.
        if (!AlgorithmSupported(ds))
        {
            LogDNSSEC("ProveInsecureCallback: Unsupported algorithm %d or digest %d", ds->alg, ds->digestType);
            DeliverInsecureProofResult(m, ic, DNSSEC_Insecure);
            return;
        }

        // If the delegation is secure and the name that we queried for is same as the original
        // name that started the insecure proof, then something is not right. We started the
        // insecure proof e.g., the zone is not signed, but we are able to validate a DS for
        // the same name which implies that the zone is signed (whose algorithm we support) and
        // we should not have started the insecurity proof in the first place.
        if (SameDomainName(&question->qname, &pdv->origName))
        {
            LogDNSSEC("ProveInsecureCallback: Insecure proof reached original name %##s, error", question->qname.c);
            DeliverInsecureProofResult(m, ic, DNSSEC_Bogus);
            return;
        }

        LogDNSSEC("ProveInsecureCallback: Trying one more level down");
        ProveInsecure(m, pdv, ic, mDNSNULL);
    }
    else if (ac->rrset->rrtype == kDNSType_NSEC || ac->rrset->rrtype == kDNSType_NSEC3)
    {
        CacheRecord *cr;

        if (ac->rrset->rrtype == kDNSType_NSEC)
            cr = NSECRecordIsDelegation(m, &question->qname, question->qtype);
        else
            cr = NSEC3RecordIsDelegation(m, &question->qname, question->qtype);
		if (cr)
        {
            LogDNSSEC("ProveInsecureCallback: Non-existence proved and %s is a delegation for %##s (%s)", CRDisplayString(m, cr),
				question->qname.c, DNSTypeName(question->qtype));
            DeliverInsecureProofResult(m, ic, DNSSEC_Insecure);
			return;
        }
        // Could be a ENT. Go one more level down to see whether it is a secure delegation or not.
        if (!SameDomainName(&question->qname, &pdv->origName))
        {
            LogDNSSEC("ProveInsecureCallback: Not a delegation %##s (%s), go one more level down", question->qname.c, DNSTypeName(question->qtype));
            ProveInsecure(m, pdv, ic, mDNSNULL);
        }
        else
        {
             // Secure denial of existence and the name matches the original query. This means we should have
             // received an NSEC (if the type does not exist) or signed records (if the name and type exists)
             // and verified it successfully instead of starting the insecure proof. This could happen e.g.,
             // Wildcard expanded answer received without NSEC/NSEC3s etc. Also, is it possible that the
             // zone went from unsigned to signed in a short time ? For now, we return bogus.
             LogDNSSEC("ProveInsecureCallback: Not a delegation %##s (%s), but reached original name", question->qname.c,
                 DNSTypeName(question->qtype));
             DeliverInsecureProofResult(m, ic, DNSSEC_Bogus);
        }
    }
    return;
done:
    DeliverInsecureProofResult(m, ic, DNSSEC_Bogus);
}

// We return Insecure if we don't have a trust anchor or we have a trust anchor and
// can prove that the delegation is not secure (and hence can't establish the trust
// chain) or the delegation is possibly secure but we don't have the algorithm support
// to prove that.
mDNSexport void ProveInsecure(mDNS *const m, DNSSECVerifier *dv, InsecureContext *ic, domainname *trigger)
{
    TrustAnchor *ta;
    domainname *sname;

    if (ic == mDNSNULL)
    {
        ic = (InsecureContext *) mDNSPlatformMemAllocateClear(sizeof(*ic));
        if (!ic)
        {
            LogMsg("mDNSPlatformMemAllocateClear: ERROR!! memory alloc failed for ic");
            return;
        }
        
        // Save the AuthInfo while we are proving insecure. We don't want to mix up
        // the auth chain for Bogus and Insecure. If we prove it to be insecure, we
        // will add the chain corresponding to the insecure proof. Otherwise, we will
        // restore this chain.
        if (dv->ac)
        {
            if (!dv->saveac)
            {
                LogDNSSEC("ProveInsecure: saving authinfo");
            }
            else
            {
                LogDNSSEC("ProveInsecure: ERROR!! authinfo already set");
                FreeDNSSECAuthChainInfo(dv->saveac);
            }
            dv->saveac = dv->ac;
            ResetAuthChain(dv);
        }
        ic->dv = dv;
        ic->q.ThisQInterval = -1;

        if (trigger)
        {
            LogDNSSEC("ProveInsecure: Setting Trigger %##s", trigger->c);
            ic->triggerLabelCount = CountLabels(trigger);
        }
        else
        {
            LogDNSSEC("ProveInsecure: No Trigger");
            ic->triggerLabelCount = CountLabels(&dv->origName);
        }

        ta = FindTrustAnchor(m, &dv->origName);
        if (!ta)
        {
            LogDNSSEC("ProveInsecure: TrustAnchor NULL");
            DeliverInsecureProofResult(m, ic, DNSSEC_Insecure);
            return;
        }
        // We want to skip the labels that is already matched by the trust anchor so
        // that the first query starts just below the trust anchor
        ic->skip = CountLabels(&dv->origName) - CountLabels(&ta->zone);
        if (!ic->skip)
        {
            LogDNSSEC("ProveInsecure: origName %##s, skip is zero", dv->origName.c);
            DeliverInsecureProofResult(m, ic, DNSSEC_Bogus);
            return;
        }
    }
    // Look for the DS record starting just below the trust anchor.
    //
    // 1. If we find an NSEC record, then see if it is a delegation. If it is, then
    //    we are done. Otherwise, go down one more level.
    //
    // 2. If we find a DS record and no algorithm support, return "insecure". Otherwise, go
    //    down one more level.
    //
    sname = (domainname *)SkipLeadingLabels(&dv->origName, (ic->skip ? ic->skip - 1 : 0));
    if (!sname)
    {
        LogDNSSEC("ProveInsecure: sname NULL, origName %##s, skip %d", dv->origName.c, ic->skip);
        DeliverInsecureProofResult(m, ic, DNSSEC_Bogus);
        return;
    }

    // Insecurity proof is started during the normal bottom-up validation when we have a break in the trust
    // chain e.g., we get NSEC/NSEC3s when looking up a DS record. Insecurity proof is top-down looking
    // for a break in the trust chain. If we have already tried the validation (before the insecurity
    // proof started) for this "sname", then don't bother with the proof. This happens sometimes, when 
    // we can't prove whether a zone is insecurely delegated or not. For example, if we are looking up
    // host1.secure-nods.secure.example and when we encounter secure-nods, there is no DS record in the
    // parent. We start the insecurity proof remembering that "secure-nods.secure.example" is the trigger
    // point. As part of the proof we reach "secure-nods.secure.example". Even though secure.example
    // prove that the name "secure-nods.secure.example/DS" does not exist,  it can't prove that it is a
    // delegation. So, we continue one more level down to host1.secure-nods.secure.example and we
    // realize that we already tried the validation and hence abort here.

    if (CountLabels(sname) > ic->triggerLabelCount)
    {
        LogDNSSEC("ProveInsecure: Beyond the trigger current name %##s, origName %##s", sname->c, dv->origName.c);
        DeliverInsecureProofResult(m, ic, DNSSEC_Bogus);
        return;
    }

    LogDNSSEC("ProveInsecure: OrigName %##s (%s), Current %##s", dv->origName.c, DNSTypeName(dv->origType), sname->c);
    ic->skip--;
    InitializeQuestion(m, &ic->q, dv->InterfaceID, sname, kDNSType_DS, ProveInsecureCallback, ic);
    ic->q.ValidationRequired = DNSSEC_VALIDATION_INSECURE;
    ic->q.ValidatingResponse = 0;
    ic->q.DNSSECAuthInfo = mDNSNULL;
    mDNS_StartQuery(m, &ic->q);
}

mDNSexport void BumpDNSSECStats(mDNS *const m, DNSSECStatsAction action, DNSSECStatsType type, mDNSu32 value)
{
    switch (type)
    {
    case kStatsTypeMemoryUsage:
        if (action == kStatsActionIncrement)
        {
            m->DNSSECStats.TotalMemUsed += value;
        }
        else if (action == kStatsActionDecrement)
        {
            m->DNSSECStats.TotalMemUsed -= value;
        }
        break;
    case kStatsTypeLatency:
        if (action == kStatsActionSet)
        {
            if (value <= 4)
            {
                m->DNSSECStats.Latency0++;
            }
            else if (value <= 9)
            {
                m->DNSSECStats.Latency5++;
            }
            else if (value <= 19)
            {
                m->DNSSECStats.Latency10++;
            }
            else if (value <= 49)
            {
                m->DNSSECStats.Latency20++;
            }
            else if (value <= 99)
            {
                m->DNSSECStats.Latency50++;
            }
            else
            {
                m->DNSSECStats.Latency100++;
            }
        }
        break;
    case kStatsTypeExtraPackets:
        if (action == kStatsActionSet)
        {
            if (value <= 2)
            {
                m->DNSSECStats.ExtraPackets0++;
            }
            else if (value <= 6)
            {
                m->DNSSECStats.ExtraPackets3++;
            }
            else if (value <= 9)
            {
                m->DNSSECStats.ExtraPackets7++;
            }
            else
            {
                m->DNSSECStats.ExtraPackets10++;
            }
        }
        break;
    case kStatsTypeStatus:
        if (action == kStatsActionSet)
        {
            switch(value)
            {
            case DNSSEC_Secure:
                m->DNSSECStats.SecureStatus++;
                break;
            case DNSSEC_Insecure:
                m->DNSSECStats.InsecureStatus++;
                break;
            case DNSSEC_Indeterminate:
                m->DNSSECStats.IndeterminateStatus++;
                break;
            case DNSSEC_Bogus:
                m->DNSSECStats.BogusStatus++;
                break;
            case DNSSEC_NoResponse:
                m->DNSSECStats.NoResponseStatus++;
                break;
            default:
                LogMsg("BumpDNSSECStats: unknown status %d", value);
            }
        }
        break;
    case kStatsTypeMsgSize:
        if (action == kStatsActionSet)
        {
            if (value <= 1024)
            {
                m->DNSSECStats.MsgSize0++;
            }
            else if (value <= 2048)
            {
                m->DNSSECStats.MsgSize1++;
            }
            else
            {
                m->DNSSECStats.MsgSize2++;
            }
        }
        break;
    case kStatsTypeProbe:
        if (action == kStatsActionIncrement)
        {
            m->DNSSECStats.NumProbesSent += value;
        }
        break;
    default:
        LogMsg("BumpDNSSECStats: unknown type %d", type);
    }
    return;
}

#else // !DNSSEC_DISABLED

mDNSexport void VerifySignature(mDNS *const m, DNSSECVerifier *dv, DNSQuestion *q)
{
    (void)m;
    (void)dv;
    (void)q;
}

mDNSexport void BumpDNSSECStats(mDNS *const m, DNSSECStatsAction action, DNSSECStatsType type, mDNSu32 value)
{
    (void)m;
    (void)action;
    (void)type;
    (void)value;
}

mDNSexport void InitializeQuestion(mDNS *const m, DNSQuestion *question, mDNSInterfaceID InterfaceID, const domainname *qname, mDNSu16 qtype, mDNSQuestionCallback *callback, void *context)
{
    (void) m;
    (void) question;
    (void) InterfaceID;
    (void) qname;
    (void) qtype;
    (void) callback;
    (void) context;
}

mDNSexport char *DNSSECStatusName(DNSSECStatus status)
{
    (void) status;
    
    return mDNSNULL;
}

#endif // !DNSSEC_DISABLED
