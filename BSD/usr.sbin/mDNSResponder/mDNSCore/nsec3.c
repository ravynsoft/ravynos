/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2011-2018 Apple Inc. All rights reserved.
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
// nsec3.c: This file contains support functions to validate NSEC3 records for
// NODATA and NXDOMAIN error.
// ***************************************************************************

#include "mDNSEmbeddedAPI.h"
#include "DNSCommon.h"
#include "CryptoAlg.h"
#include "nsec3.h"
#include "nsec.h"

// Base32 encoding takes 5 bytes of the input and encodes as 8 bytes of output.
// For example, SHA-1 hash of 20 bytes will be encoded as 20/5 * 8 = 32 base32
// bytes. For a max domain name size of 255 bytes of base32 encoding : (255/8)*5
// is the max hash length possible.
#define NSEC3_MAX_HASH_LEN  155
// In NSEC3, the names are hashed and stored in the first label and hence cannot exceed label
// size.
#define NSEC3_MAX_B32_LEN   MAX_DOMAIN_LABEL

// Define DNSSEC_DISABLED to remove all the DNSSEC functionality
// and use the stub functions implemented later in this file.

#ifndef DNSSEC_DISABLED

typedef enum 
{
    NSEC3ClosestEncloser,
    NSEC3Covers,
    NSEC3CEProof
} NSEC3FindValues;

//#define NSEC3_DEBUG 1

#if NSEC3_DEBUG
mDNSlocal void PrintHash(mDNSu8 *digest, int digestlen, char *buffer, int buflen)
{
    int length = 0;
    for (int j = 0; j < digestlen; j++)
    {
        length += mDNS_snprintf(buffer+length, buflen-length-1, "%x", digest[j]);
    }
}
#endif

mDNSlocal mDNSBool NSEC3OptOut(CacheRecord *cr)
{
    const RDataBody2 *const rdb = (RDataBody2 *)cr->resrec.rdata->u.data;
    rdataNSEC3 *nsec3 = (rdataNSEC3 *)rdb->data;
    return (nsec3->flags & NSEC3_FLAGS_OPTOUT);
}

mDNSlocal int NSEC3SameName(const mDNSu8 *name, int namelen, const mDNSu8 *nsecName, int nsecLen)
{
    int i;

    // Note: With NSEC3, the lengths should always be same. 
    if (namelen != nsecLen)
    {
        LogMsg("NSEC3SameName: ERROR!! namelen %d, nsecLen %d", namelen, nsecLen);
        return ((namelen < nsecLen) ? -1 : 1);
    }

    for (i = 0; i < namelen; i++)
    {
        mDNSu8 ac = *name++;
        mDNSu8 bc = *nsecName++;
        if (mDNSIsUpperCase(ac)) ac += 'a' - 'A';
        if (mDNSIsUpperCase(bc)) bc += 'a' - 'A';
        if (ac != bc)
        {
            verbosedebugf("NSEC3SameName: returning ac %c, bc %c", ac, bc);
            return ((ac < bc) ? -1 : 1);
        }
    }
    return 0;
}

// Does the NSEC3 in "ncr" covers the "name" ?
// hashName is hash of the "name" and b32Name is the base32 encoded equivalent.
mDNSlocal mDNSBool NSEC3CoversName(mDNS *const m, CacheRecord *ncr, const mDNSu8 *hashName, int hashLen, const mDNSu8 *b32Name,
	int b32len)
{
    mDNSu8 *nxtName;
    int nxtLength;
    int ret, ret1, ret2;
    const mDNSu8 b32nxtname[NSEC3_MAX_B32_LEN+1];
    int b32nxtlen;

    NSEC3Parse(&ncr->resrec, mDNSNULL, &nxtLength, &nxtName, mDNSNULL, mDNSNULL);

    if (nxtLength != hashLen || ncr->resrec.name->c[0] != b32len)
        return mDNSfalse;

    // Compare the owner names and the "nxt" names.
    //
    // Owner name is base32 encoded and hence use the base32 encoded name b32name.
    // nxt name is binary and hence use the binary value in hashName. 
    ret1 = NSEC3SameName(&ncr->resrec.name->c[1], ncr->resrec.name->c[0], b32Name, b32len);
    ret2 = DNSMemCmp(nxtName, hashName, hashLen);

#if NSEC3_DEBUG
    {
        char nxtbuf1[50];
        char nxtbuf2[50];

        PrintHash(nxtName, nxtLength, nxtbuf1, sizeof(nxtbuf1));
        PrintHash((mDNSu8 *)hashName, hashLen, nxtbuf2, sizeof(nxtbuf2));
        LogMsg("NSEC3CoversName: Owner name %s, name %s", &ncr->resrec.name->c[1], b32Name);
        LogMsg("NSEC3CoversName: Nxt hash name %s, name %s", nxtbuf1, nxtbuf2);
    }
#endif

    // "name" is greater than the owner name and smaller than nxtName. This also implies
	// that nxtName > owner name implying that it is normal NSEC3.
    if (ret1 < 0 && ret2 > 0)
    {
        LogDNSSEC("NSEC3CoversName: NSEC3 %s covers %s (Normal)", CRDisplayString(m, ncr), b32Name);
        return mDNStrue;
    }
    // Need to compare the owner name and "nxt" to see if this is the last
    // NSEC3 in the zone. Only the owner name is in base32 and hence we need to
    // convert the nxtName to base32.
    b32nxtlen = baseEncode((char *)b32nxtname, sizeof(b32nxtname), nxtName, nxtLength, ENC_BASE32);
    if (!b32nxtlen)
    {
        LogDNSSEC("NSEC3CoversName: baseEncode of nxtName of %s failed", CRDisplayString(m, ncr));
        return mDNSfalse;
    }
    if (b32len != b32nxtlen)
    {
        LogDNSSEC("NSEC3CoversName: baseEncode of nxtName for %s resulted in wrong length b32nxtlen %d, b32len %d",
            CRDisplayString(m, ncr), b32len, b32nxtlen);
        return mDNSfalse;
    }
    LogDNSSEC("NSEC3CoversName: Owner name %s, b32nxtname %s, ret1 %d, ret2 %d", &ncr->resrec.name->c[1], b32nxtname, ret1, ret2);

    // If it is the last NSEC3 in the zone nxt < "name" and NSEC3SameName returns -1.
    //
    // - ret1 < 0 means "name > owner"
    // - ret2 > 0 means "name < nxt"
    // 
    // Note: We also handle the case of only NSEC3 in the zone where NSEC3SameName returns zero.
    ret = NSEC3SameName(b32nxtname, b32nxtlen, &ncr->resrec.name->c[1], ncr->resrec.name->c[0]);
    if (ret <= 0 &&
        (ret1 < 0 || ret2 > 0))
    {
        LogDNSSEC("NSEC3CoversName: NSEC3 %s covers %s (Last), ret1 %d, ret2 %d", CRDisplayString(m, ncr), b32Name, ret1, ret2);
        return mDNStrue;
    }

    return mDNSfalse;
}

mDNSlocal const mDNSu8 *NSEC3HashName(const domainname *name, rdataNSEC3 *nsec3, const mDNSu8 hash[NSEC3_MAX_HASH_LEN], int *dlen)
{
    AlgContext *ctx;
    unsigned int i;
    unsigned int iterations;
    domainname lname;
    mDNSu8 *p = (mDNSu8 *)&nsec3->salt;
    const mDNSu8 *digest;
    int digestlen;
    mDNSBool first = mDNStrue;

    if (DNSNameToLowerCase((domainname *)name, &lname) != mStatus_NoError)
    {
        LogMsg("NSEC3HashName: ERROR!! DNSNameToLowerCase failed");
        return mDNSNULL;
    }

    digest = lname.c;
    digestlen = DomainNameLength(&lname);

    // Note that it is "i <=". The first iteration is for digesting the name and salt.
    // The iteration count does not include that.
    iterations = swap16(nsec3->iterations);
    for (i = 0; i <= iterations; i++)
    {
        ctx = AlgCreate(DIGEST_ALG, nsec3->alg);
        if (!ctx)
        {
            LogMsg("NSEC3HashName: ERROR!! Cannot allocate context");
            return mDNSNULL;
        }

        AlgAdd(ctx, digest, digestlen);
        if (nsec3->saltLength)
            AlgAdd(ctx, p, nsec3->saltLength);
        if (first)
        {
            first = mDNSfalse;
            digest = hash;
            digestlen = AlgLength(ctx);
        }
        AlgFinal(ctx, (void *)digest, digestlen);
        AlgDestroy(ctx);
    }
    *dlen = digestlen;
    return digest;
}

// This function can be called with NSEC3ClosestEncloser, NSEC3Covers and NSEC3CEProof
//
// Passing in NSEC3ClosestEncloser means "find an exact match for the origName".
// Passing in NSEC3Covers means "find an NSEC3 that covers the origName".
//
// i.e., in both cases the nsec3 records are iterated to find the best match and returned.
// With NSEC3ClosestEncloser, as we are just looking for a name match, extra checks for
// the types being present or absent will not be checked.
//
// If NSEC3CEProof is passed, the name is tried as such first by iterating through all NSEC3s
// finding a ClosestEncloser or CloserEncloser and then one label skipped from the left and
// retried again till both the closest and closer encloser is found.
//
// ncr is the negative cache record that has the NSEC3 chain
// origName is the name for which we are trying to find the ClosestEncloser etc.
// closestEncloser and closerEncloser are the return values of the function
// ce is the closest encloser that will be returned if we find one
mDNSlocal mDNSBool NSEC3Find(mDNS *const m, NSEC3FindValues val, CacheRecord *ncr, domainname *origName, CacheRecord **closestEncloser,
	CacheRecord **closerEncloser, const domainname **ce, mDNSu16 qtype)
{
    int i;
    int labelCount = CountLabels(origName);
    CacheRecord *cr;
    rdataNSEC3 *nsec3;

    (void) qtype; // unused
    // Pick the first NSEC for the iterations, salt etc.
    for (cr = ncr->nsec; cr; cr = cr->next)
    {
        if (cr->resrec.rrtype == kDNSType_NSEC3)
        {
            const RDataBody2 *const rdb = (RDataBody2 *)cr->resrec.rdata->u.data;
            nsec3 = (rdataNSEC3 *)rdb->data;
            break;
        }
    }
    if (!cr)
    {
        LogMsg("NSEC3Find: cr NULL");
        return mDNSfalse;
    }

    // Note: The steps defined in this function are for "NSEC3CEProof". As part of NSEC3CEProof,
    // we need to find both the closestEncloser and closerEncloser which can also be found
    // by passing NSEC3ClosestEncloser and NSEC3Covers respectively.
    //
    // Section 8.3 of RFC 5155.
    // 1.  Set SNAME=QNAME.  Clear the flag.
    //
    // closerEncloser is the "flag". "name" below is SNAME.

    if (closestEncloser)
    {
        *ce = mDNSNULL;
        *closestEncloser = mDNSNULL;
    }
    if (closerEncloser)
        *closerEncloser = mDNSNULL;

    // If we are looking for a closestEncloser or a covering NSEC3, we don't have
    // to truncate the name. For the give name, try to find the closest or closer
    // encloser.
    if (val != NSEC3CEProof)
    {
        labelCount = 0;
    }

    for (i = 0; i < labelCount + 1; i++)
    { 
        int hlen;
        const mDNSu8 hashName[NSEC3_MAX_HASH_LEN];
        const domainname *name;
        const mDNSu8 b32Name[NSEC3_MAX_B32_LEN+1];
        int b32len;

        name = SkipLeadingLabels(origName, i);
        if (!NSEC3HashName(name, nsec3, hashName, &hlen))
        {
            LogMsg("NSEC3Find: NSEC3HashName failed for %##s", name->c);
            continue;
        }

        b32len = baseEncode((char *)b32Name, sizeof(b32Name), (mDNSu8 *)hashName, hlen, ENC_BASE32);
        if (!b32len)
        {
            LogMsg("NSEC3Find: baseEncode of name %##s failed", name->c);
            continue;
        }


        for (cr = ncr->nsec; cr; cr = cr->next)
        {
            const domainname *nsecZone;
            int result, subdomain;

            if (cr->resrec.rrtype != kDNSType_NSEC3)
                continue;

            nsecZone = SkipLeadingLabels(cr->resrec.name, 1);
            if (!nsecZone)
            {
                LogMsg("NSEC3Find: SkipLeadingLabel failed for %s, current name %##s",
                    CRDisplayString(m, cr), name->c);
                continue;
            }

            // NSEC3 owner names are formed by hashing the owner name and then appending
            // the zone name to it. If we skip the first label, the rest should be
            // the zone name. See whether it is the subdomain of the name we are looking
            // for. 
            result = DNSSECCanonicalOrder(origName, nsecZone, &subdomain);
            
            // The check can't be a strict subdomain check. When NSEC3ClosestEncloser is
            // passed in, there can be an exact match. If it is a subdomain or an exact
            // match, we should continue with the proof.
            if (!(subdomain || !result))
            {
                LogMsg("NSEC3Find: NSEC3 %s not a subdomain of %##s, result %d", CRDisplayString(m, cr),
                    origName->c, result);
                continue;
            }

            // 2.1) If there is no NSEC3 RR in the response that matches SNAME
            // (i.e., an NSEC3 RR whose owner name is the same as the hash of
            // SNAME, prepended as a single label to the zone name), clear
            // the flag.
            //
            // Note: We don't try to determine the actual zone name. We know that
            // the labels following the hash (nsecZone) is the ancestor and we don't
            // know where the zone cut is. Hence, we verify just the hash to be
            // the same.

            if (val == NSEC3ClosestEncloser || val == NSEC3CEProof)
            {
                if (!NSEC3SameName(&cr->resrec.name->c[1], cr->resrec.name->c[0], (const mDNSu8 *)b32Name, b32len))
                {
                    int bmaplen;
                    mDNSu8 *bmap;

                    // For NSEC3ClosestEncloser, we are finding an exact match and
                    // "type" specific checks should be done by the caller.
                    if (val != NSEC3ClosestEncloser)
                    {
                        // DNAME bit must not be set and NS bit may be set only if SOA bit is set
                        NSEC3Parse(&cr->resrec, mDNSNULL, mDNSNULL, mDNSNULL, &bmaplen, &bmap);
                        if (BitmapTypeCheck(bmap, bmaplen, kDNSType_DNAME))
                        {
                            LogDNSSEC("NSEC3Find: DNAME bit set in %s, ignoring", CRDisplayString(m, cr));
                            return mDNSfalse;
                        }
                        // This is the closest encloser and should come from the right zone.
                        if (BitmapTypeCheck(bmap, bmaplen, kDNSType_NS) &&
                            !BitmapTypeCheck(bmap, bmaplen, kDNSType_SOA))
                        {
                            LogDNSSEC("NSEC3Find: NS bit set without SOA bit in %s, ignoring", CRDisplayString(m, cr));
                            return mDNSfalse;
                        }
                    }

                    LogDNSSEC("NSEC3Find: ClosestEncloser %s found for name %##s", CRDisplayString(m, cr), name->c);
                    if (closestEncloser)
                    {
                        *ce = name;
                        *closestEncloser = cr;
                    }
                    if (val == NSEC3ClosestEncloser)
                        return mDNStrue;
                    else
                        break;
                }
            }

            if ((val == NSEC3Covers || val == NSEC3CEProof) && (!closerEncloser || !(*closerEncloser)))
            {
                if (NSEC3CoversName(m, cr, hashName, hlen, b32Name, b32len))
                {
                    // 2.2) If there is an NSEC3 RR in the response that covers SNAME, set the flag.
                    if (closerEncloser)
                        *closerEncloser = cr;
                    if (val == NSEC3Covers)
                        return mDNStrue;
                    else
                        break;
                }
            }
        }
        // 2.3) If there is a matching NSEC3 RR in the response and the flag
        // was set, then the proof is complete, and SNAME is the closest
        // encloser.
        if (val == NSEC3CEProof && closestEncloser && *closestEncloser)
        {
            if (closerEncloser && *closerEncloser)
            {
                LogDNSSEC("NSEC3Find: Found closest and closer encloser");
                return mDNStrue;
            }
            else
            {
                // 2.4) If there is a matching NSEC3 RR in the response, but the flag
                // is not set, then the response is bogus.
                //
                // Note: We don't have to wait till we finish trying all the names. If the matchName
                // happens, we found the closest encloser which means we should have found the closer
                // encloser before.

                LogDNSSEC("NSEC3Find: Found closest, but not closer encloser");
                return mDNSfalse;
            }
        }
        // 3.  Truncate SNAME by one label from the left, go to step 2.
    }
    LogDNSSEC("NSEC3Find: Cannot find name %##s (%s)", origName->c, DNSTypeName(qtype));
    return mDNSfalse;
}

mDNSlocal mDNSBool NSEC3ClosestEncloserProof(mDNS *const m, CacheRecord *ncr, domainname *name, CacheRecord **closestEncloser, CacheRecord **closerEncloser,
	const domainname **ce, mDNSu16 qtype)
{
    if (!NSEC3Find(m, NSEC3CEProof, ncr, name, closestEncloser, closerEncloser, ce, qtype))
    {
        LogDNSSEC("NSEC3ClosestEncloserProof: ERROR!! Cannot do closest encloser proof");
        return mDNSfalse;
    }

    // Note: It is possible that closestEncloser and closerEncloser are the same.
    if (!closestEncloser || !closerEncloser || !ce)
    {
        LogMsg("NSEC3ClosestEncloserProof: ClosestEncloser %p or CloserEncloser %p ce %p, something is NULL", closestEncloser, closerEncloser, ce);
        return mDNSfalse;
    }

    // If the name exists, we should not have gotten the name error
    if (SameDomainName((*ce), name))
    {
        LogMsg("NSEC3ClosestEncloserProof: ClosestEncloser %s same as origName %##s", CRDisplayString(m, *closestEncloser),
            (*ce)->c);
        return mDNSfalse;
    }
    return mDNStrue;
}

mDNSlocal mDNSBool VerifyNSEC3(mDNS *const m, DNSSECVerifier *dv, CacheRecord *ncr, CacheRecord *closestEncloser,
    CacheRecord *closerEncloser, CacheRecord *wildcard, DNSSECVerifierCallback callback)
{
    mStatus status;
    RRVerifier *r;

    // We have three NSEC3s. If any of two are same, we should just prove one of them.
    // This is just not an optimization; DNSSECNegativeValidationCB does not handle
    // identical NSEC3s very well.

    if (closestEncloser == closerEncloser)
        closestEncloser = mDNSNULL;
    if (closerEncloser == wildcard)
        closerEncloser = mDNSNULL;
    if (closestEncloser == wildcard)
        closestEncloser = mDNSNULL;

    dv->pendingNSEC = mDNSNULL;
    if (closestEncloser)
    {
        r = AllocateRRVerifier(&closestEncloser->resrec, &status);
        if (!r)
            return mDNSfalse;
        r->next = dv->pendingNSEC;
        dv->pendingNSEC = r;
    }
    if (closerEncloser)
    {
        r = AllocateRRVerifier(&closerEncloser->resrec, &status);
        if (!r)
            return mDNSfalse;
        r->next = dv->pendingNSEC;
        dv->pendingNSEC = r;
    }
    if (wildcard)
    {
        r = AllocateRRVerifier(&wildcard->resrec, &status);
        if (!r)
            return mDNSfalse;
        r->next = dv->pendingNSEC;
        dv->pendingNSEC = r;
    }
    if (!dv->pendingNSEC)
    {
        LogMsg("VerifyNSEC3: ERROR!! pending NSEC null");
        return mDNSfalse;
    }
    r = dv->pendingNSEC;
    dv->pendingNSEC = r->next;
    r->next = mDNSNULL;

    LogDNSSEC("VerifyNSEC3: Verifying %##s (%s)", r->name.c, DNSTypeName(r->rrtype));
    if (!dv->pendingNSEC)
        VerifyNSEC(m, mDNSNULL, r, dv, ncr, mDNSNULL);
    else
        VerifyNSEC(m, mDNSNULL, r, dv, ncr, callback);
    return mDNStrue;
}

mDNSexport void NSEC3NameErrorProof(mDNS *const m, DNSSECVerifier *dv, CacheRecord *ncr)
{
    CacheRecord *closerEncloser;
    CacheRecord *closestEncloser;
    CacheRecord *wildcard;
    const domainname *ce = mDNSNULL;
    domainname wild;

    if (!NSEC3ClosestEncloserProof(m, ncr, &dv->q.qname, &closestEncloser, &closerEncloser, &ce, dv->q.qtype))
    {
        goto error;
    }
    LogDNSSEC("NSEC3NameErrorProof: ClosestEncloser %s, ce %##s", CRDisplayString(m, closestEncloser), ce->c);
    LogDNSSEC("NSEC3NameErrorProof: CloserEncloser %s", CRDisplayString(m, closerEncloser));

    // *.closestEncloser should be covered by some nsec3 which would then prove
    // that the wildcard does not exist
    wild.c[0] = 1;
    wild.c[1] = '*';
    wild.c[2] = 0;
    if (!AppendDomainName(&wild, ce))
    {
        LogMsg("NSEC3NameErrorProof: Can't append domainname to closest encloser name %##s", ce->c);
        goto error;
    }
    if (!NSEC3Find(m, NSEC3Covers, ncr, &wild, mDNSNULL, &wildcard, mDNSNULL, dv->q.qtype))
    {
        LogMsg("NSEC3NameErrorProof: Cannot find encloser for wildcard");
        goto error;
    }
    else
    {
        LogDNSSEC("NSEC3NameErrorProof: Wildcard %##s covered by %s", wild.c, CRDisplayString(m, wildcard));
        if (wildcard == closestEncloser)
        {
            LogDNSSEC("NSEC3NameErrorProof: ClosestEncloser matching Wildcard %s", CRDisplayString(m, wildcard));
        }
    }
    if (NSEC3OptOut(closerEncloser))
    {
        dv->flags |= NSEC3_OPT_OUT;
    }
    if (!VerifyNSEC3(m, dv, ncr, closestEncloser, closerEncloser, wildcard, NameErrorNSECCallback))
        goto error;
    else
        return;

error:
    dv->DVCallback(m, dv, DNSSEC_Bogus);
}

// Section 8.5, 8.6 of RFC 5155 first paragraph
mDNSlocal mDNSBool NSEC3NoDataError(mDNS *const m, CacheRecord *ncr, domainname *name, mDNSu16 qtype, CacheRecord **closestEncloser)
{
    const domainname *ce = mDNSNULL;

    *closestEncloser = mDNSNULL;
    // Note: This also covers ENT in which case the bitmap is empty
    if (NSEC3Find(m, NSEC3ClosestEncloser, ncr, name, closestEncloser, mDNSNULL, &ce, qtype))
    {
        int bmaplen;
        mDNSu8 *bmap;
        mDNSBool ns, soa;

        NSEC3Parse(&(*closestEncloser)->resrec, mDNSNULL, mDNSNULL, mDNSNULL, &bmaplen, &bmap);
        if (BitmapTypeCheck(bmap, bmaplen, qtype) || BitmapTypeCheck(bmap, bmaplen, kDNSType_CNAME))
        {
            LogMsg("NSEC3NoDataError: qtype %s exists in %s", DNSTypeName(qtype), CRDisplayString(m, *closestEncloser));
            return mDNSfalse;
        }
        ns = BitmapTypeCheck(bmap, bmaplen, kDNSType_NS);
        soa = BitmapTypeCheck(bmap, bmaplen, kDNSType_SOA);
        if (qtype != kDNSType_DS)
        {
            // For non-DS type questions, we don't want to use the parent side records to
            // answer it
            if (ns && !soa)
            {
                LogDNSSEC("NSEC3NoDataError: Parent side NSEC %s, can't use for child qname %##s (%s)",
                    CRDisplayString(m, *closestEncloser), name->c, DNSTypeName(qtype));
                return mDNSfalse;
            }
        }
        else
        {
            if (soa)
            {
                LogDNSSEC("NSEC3NoDataError: Child side NSEC %s, can't use for parent qname %##s (%s)",
                    CRDisplayString(m, *closestEncloser), name->c, DNSTypeName(qtype));
                return mDNSfalse;
            }
        }
        LogDNSSEC("NSEC3NoDataError: Name -%##s- exists, but qtype %s does not exist in %s", name->c, DNSTypeName(qtype), CRDisplayString(m, *closestEncloser));
        return mDNStrue;
    }
    return mDNSfalse;
}

mDNSexport void NSEC3NoDataProof(mDNS *const m, DNSSECVerifier *dv, CacheRecord *ncr)
{
    CacheRecord *closerEncloser = mDNSNULL;
    CacheRecord *closestEncloser = mDNSNULL;
    CacheRecord *wildcard = mDNSNULL;
    const domainname *ce = mDNSNULL;
    domainname wild;

    // Section 8.5, 8.6 of RFC 5155
    if (NSEC3NoDataError(m, ncr, &dv->q.qname, dv->q.qtype, &closestEncloser))
    {
        goto verify;
    }
    // Section 8.6, 8.7: if we can't find the NSEC3 RR, verify the closest encloser proof
    // for QNAME and the "next closer" should have the opt out
    if (!NSEC3ClosestEncloserProof(m, ncr, &dv->q.qname, &closestEncloser, &closerEncloser, &ce, dv->q.qtype))
    {
        goto error;
    }

    // Section 8.7: find a matching NSEC3 for *.closestEncloser
    wild.c[0] = 1;
    wild.c[1] = '*';
    wild.c[2] = 0;
    if (!AppendDomainName(&wild, ce))
    {
        LogMsg("NSEC3NameErrorProof: Can't append domainname to closest encloser name %##s", ce->c);
        goto error;
    }
    if (!NSEC3Find(m, NSEC3ClosestEncloser, ncr, &wild, &wildcard, mDNSNULL, &ce, dv->q.qtype))
    {
        // Not a wild card case. Section 8.6 second para applies.
        LogDNSSEC("NSEC3NoDataProof: Cannot find encloser for wildcard, perhaps not a wildcard case");
        if (!NSEC3OptOut(closerEncloser))
        {
            LogDNSSEC("NSEC3DataProof: opt out not set for %##s (%s), bogus", dv->q.qname.c, DNSTypeName(dv->q.qtype));
            goto error;
        }
        LogDNSSEC("NSEC3DataProof: opt out set, proof complete for %##s (%s)", dv->q.qname.c, DNSTypeName(dv->q.qtype));
        dv->flags |= NSEC3_OPT_OUT;
    }
    else
    {
        int bmaplen;
        mDNSu8 *bmap;
        NSEC3Parse(&wildcard->resrec, mDNSNULL, mDNSNULL, mDNSNULL, &bmaplen, &bmap);
        if (BitmapTypeCheck(bmap, bmaplen, dv->q.qtype) || BitmapTypeCheck(bmap, bmaplen, kDNSType_CNAME))
        {
            LogDNSSEC("NSEC3NoDataProof: qtype %s exists in %s", DNSTypeName(dv->q.qtype), CRDisplayString(m, wildcard));
            goto error;
        }
        if (dv->q.qtype == kDNSType_DS && BitmapTypeCheck(bmap, bmaplen, kDNSType_SOA))
        {
            LogDNSSEC("NSEC3NoDataProof: Child side wildcard NSEC3 %s, can't use for parent qname %##s (%s)",
                CRDisplayString(m, wildcard), dv->q.qname.c, DNSTypeName(dv->q.qtype));
            goto error;
        }
        else if (dv->q.qtype != kDNSType_DS && !BitmapTypeCheck(bmap, bmaplen, kDNSType_SOA) &&
            BitmapTypeCheck(bmap, bmaplen, kDNSType_NS))
        {
            // Don't use the parent side record for this
            LogDNSSEC("NSEC3NoDataProof: Parent side wildcard NSEC3 %s, can't use for child qname %##s (%s)",
                CRDisplayString(m, wildcard), dv->q.qname.c, DNSTypeName(dv->q.qtype));
            goto error;
        }
        LogDNSSEC("NSEC3NoDataProof: Wildcard %##s matched by %s", wild.c, CRDisplayString(m, wildcard));
    }
verify:

    if (!VerifyNSEC3(m, dv, ncr, closestEncloser, closerEncloser, wildcard, NoDataNSECCallback))
        goto error;
    else
        return;
error:
    dv->DVCallback(m, dv, DNSSEC_Bogus);
}

mDNSexport mDNSBool NSEC3WildcardAnswerProof(mDNS *const m, CacheRecord *ncr, DNSSECVerifier *dv)
{
    int skip;
    const domainname *nc;
    CacheRecord *closerEncloser;

    (void) m;

    // Find the next closer name and prove that it is covered by the NSEC3
    skip = CountLabels(&dv->origName) - CountLabels(dv->wildcardName) - 1;
    if (skip)
        nc = SkipLeadingLabels(&dv->origName, skip);
    else
        nc = &dv->origName;

    LogDNSSEC("NSEC3WildcardAnswerProof: wildcard name %##s", nc->c);

    if (!NSEC3Find(m, NSEC3Covers, ncr, (domainname *)nc, mDNSNULL, &closerEncloser, mDNSNULL, dv->q.qtype))
    {
        LogMsg("NSEC3WildcardAnswerProof: Cannot find closer encloser");
        return mDNSfalse;
    }
    if (!closerEncloser)
    {
        LogMsg("NSEC3WildcardAnswerProof: closerEncloser NULL");
        return mDNSfalse;
    }
    if (NSEC3OptOut(closerEncloser))
    {
        dv->flags |= NSEC3_OPT_OUT;
    }
    // NSEC3 Verification is done by the caller
    return mDNStrue;
}

mDNSexport CacheRecord *NSEC3RecordIsDelegation(mDNS *const m, domainname *name, mDNSu16 qtype)
{
    CacheGroup *cg;
    CacheRecord *cr;
    CacheRecord *ncr;
    mDNSu32 namehash;

    namehash = DomainNameHashValue(name);

    cg = CacheGroupForName(m, namehash, name);
    if (!cg)
    {
        LogDNSSEC("NSEC3RecordForName: cg NULL for %##s", name);
        return mDNSNULL;
    }
    for (ncr = cg->members; ncr; ncr = ncr->next)
    {
        if (ncr->resrec.RecordType != kDNSRecordTypePacketNegative ||
            ncr->resrec.rrtype != qtype)
        {
            continue;
        }
        for (cr = ncr->nsec; cr; cr = cr->next)
        {
            int hlen, b32len;
            const mDNSu8 hashName[NSEC3_MAX_HASH_LEN];
            const mDNSu8 b32Name[NSEC3_MAX_B32_LEN+1];
            const RDataBody2 *const rdb = (RDataBody2 *)cr->resrec.rdata->u.data;
            rdataNSEC3 *nsec3;

            if (cr->resrec.rrtype != kDNSType_NSEC3)
                continue;

            nsec3 = (rdataNSEC3 *)rdb->data;

            if (!NSEC3HashName(name, nsec3, hashName, &hlen))
            {
                LogMsg("NSEC3RecordIsDelegation: NSEC3HashName failed for %##s", name->c);
                return mDNSNULL;
            }

            b32len = baseEncode((char *)b32Name, sizeof(b32Name), (mDNSu8 *)hashName, hlen, ENC_BASE32);
            if (!b32len)
            {
                LogMsg("NSEC3RecordIsDelegation: baseEncode of name %##s failed", name->c);
                return mDNSNULL;
            }
            // Section 2.3 of RFC 4035 states that:
            //
            // Each owner name in the zone that has authoritative data or a delegation point NS RRset MUST
            // have an NSEC resource record. 
            //
            // This applies to NSEC3 record. So, if we have an NSEC3 record matching the question name with the
            // NS bit set, then this is a delegation.
            //
            if (!NSEC3SameName(&cr->resrec.name->c[1], cr->resrec.name->c[0], (const mDNSu8 *)b32Name, b32len))
            {
                int bmaplen;
                mDNSu8 *bmap;
                
                LogDNSSEC("NSEC3RecordIsDelegation: CacheRecord %s matches name %##s, b32name %s", CRDisplayString(m, cr), name->c, b32Name);
                NSEC3Parse(&cr->resrec, mDNSNULL, mDNSNULL, mDNSNULL, &bmaplen, &bmap);

                // See the Insecure Delegation Proof section in dnssec-bis: DS bit and SOA bit
                // should be absent
                if (BitmapTypeCheck(bmap, bmaplen, kDNSType_SOA) ||
                    BitmapTypeCheck(bmap, bmaplen, kDNSType_DS))
                {
                    LogDNSSEC("NSEC3RecordIsDelegation: CacheRecord %s has DS or SOA bit set, ignoring", CRDisplayString(m, cr));
                    return mDNSNULL;
                }
                if (BitmapTypeCheck(bmap, bmaplen, kDNSType_NS))
                    return cr;
                else
                    return mDNSNULL;
            }
            // If opt-out is not set, then it does not cover any delegations
            if (!(nsec3->flags & NSEC3_FLAGS_OPTOUT))
                continue;
            // Opt-out allows insecure delegations to exist without the NSEC3 RR at the
            // hashed owner name (see RFC 5155 section 6.0).
            if (NSEC3CoversName(m, cr, hashName, hlen, b32Name, b32len))
            {
                LogDNSSEC("NSEC3RecordIsDelegation: CacheRecord %s covers name %##s with optout", CRDisplayString(m, cr), name->c);
                return cr;
            }
        }
    }
    return mDNSNULL;
}

#else // !DNSSEC_DISABLED

#endif // !DNSSEC_DISABLED
