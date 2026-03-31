#include "unittest_common.h"
#include "dns_sd.h"
#include "mDNSEmbeddedAPI.h"
#include "mDNSMacOSX.h"

static mDNS_PlatformSupport PlatformStorage;
#define RR_CACHE_SIZE ((32*1024) / sizeof(CacheRecord))
static CacheEntity gRrcachestorage[RR_CACHE_SIZE];

// Primary interface info that is used when simulating the receive of the response packet
mDNSInterfaceID primary_interfaceID;
mDNSAddr primary_v4;
mDNSAddr primary_v6;
mDNSAddr primary_router;

// This function sets up the minimum environement to run a unit test. It
// initializes logging, interfaces, and timenow.
mDNSexport mStatus init_mdns_environment(mDNSBool enableLogging)
{
	mDNS *m = &mDNSStorage;

	init_logging_ut();
	mDNS_LoggingEnabled = enableLogging;
	mDNS_PacketLoggingEnabled = enableLogging;

	mStatus result = mDNS_InitStorage_ut(m, &PlatformStorage, gRrcachestorage, RR_CACHE_SIZE, mDNSfalse, mDNSNULL, mDNSNULL);
	if (result != mStatus_NoError)
		return result;

	primary_v4 = primary_v6 = primary_router = zeroAddr;
	SetInterfaces_ut(&primary_interfaceID, &primary_v4, &primary_v6, &primary_router);

	m->timenow = mDNS_TimeNow_NoLock(m);
	return mStatus_NoError;
}

// This function sets up the minimum environement to run a unit test. It
// initializes logging and timenow.  This is the call to use if your
// unit test does not use interfaces.
mDNSexport mStatus init_mdns_storage()
{
	mDNS *m = &mDNSStorage;

	init_logging_ut();
	mDNS_LoggingEnabled = 1;
	mDNS_PacketLoggingEnabled = 1;

	mStatus result = mDNS_InitStorage_ut(m, &PlatformStorage, gRrcachestorage, RR_CACHE_SIZE, mDNSfalse, mDNSNULL, mDNSNULL);
	if (result != mStatus_NoError)
		return result;

	return mStatus_NoError;
}

mDNSlocal void init_client_request(request_state* req, char *msgbuf, size_t msgSize, uint32_t op)
{
	// Simulate read_msg behavior since unit test does not open a socket
	memset(req, 0, sizeof(request_state));

	req->ts = t_complete;
	req->msgbuf = mDNSNULL;
	req->msgptr = msgbuf;
	req->msgend = msgbuf + msgSize;

	// The rest of the request values are set in order to simulate a request
	req->sd             = client_req_sd;
	req->uid            = client_req_uid;
	req->hdr_bytes      = client_req_hdr_bytes;
	req->hdr.version    = client_req_hdr_version;
	req->hdr.op         = op; // query_request
	req->hdr.datalen    = msgSize;
	req->data_bytes     = msgSize;
	req->process_id     = client_req_process_id;
	memcpy(req->pid_name, client_req_pid_name, strlen(client_req_pid_name));
}

// This function calls the mDNSResponder handle_client_request() API.  It initializes
// the request and query data structures.
mDNSexport mStatus start_client_request(request_state* req, char *msgbuf, size_t msgsz, uint32_t op, UDPSocket* socket)
{
	// Process the unit test's client request
	init_client_request(req, msgbuf, msgsz, op);

	mStatus result = handle_client_request_ut((void*)req);
	DNSQuestion* q = &req->u.queryrecord.op.q;
	q->LocalSocket = socket;
	return result;
}

// This function calls the mDNSResponder mDNSCoreReceive() API.
mDNSexport void receive_response(const request_state* req, DNSMessage *msg, size_t msgSize)
{
	mDNS *m = &mDNSStorage;
	mDNSAddr srcaddr;
	mDNSIPPort srcport, dstport;
	const mDNSu8 * end;
	DNSQuestion *q = (DNSQuestion *)&req->u.queryrecord.op.q;
	UInt8* data = (UInt8*)msg;

	// Used same values for DNS server as specified during init of unit test
	srcaddr.type				= mDNSAddrType_IPv4;
	srcaddr.ip.v4.NotAnInteger	= dns_server_ipv4.NotAnInteger;
	srcport.NotAnInteger		= client_resp_src_port;

	// Used random value for dstport
	dstport.NotAnInteger = swap16((mDNSu16)client_resp_dst_port);

	// Set DNS message (that was copied from a WireShark packet)
	end = (const mDNSu8 *)msg + msgSize;

	// Set socket info that mDNSCoreReceive uses to verify socket context
	q->LocalSocket->ss.port.NotAnInteger = swap16((mDNSu16)client_resp_dst_port);
	q->TargetQID.b[0] = data[0];
	q->TargetQID.b[1] = data[1];

	// Execute mDNSCoreReceive which copies two DNS records into the cache
	mDNSCoreReceive(m, msg, end, &srcaddr, srcport, &primary_v4, dstport, primary_interfaceID);
}

mDNSexport void receive_suspicious_response_ut(const request_state* req, DNSMessage *msg, size_t msgSize, mDNSOpaque16 suspiciousqid, mDNSBool goodLastQID)
{
    mDNS *m = &mDNSStorage;
    mDNSAddr srcaddr;
    mDNSIPPort srcport, dstport;
    const mDNSu8 * end;
    DNSQuestion *q = (DNSQuestion *)&req->u.queryrecord.op.q;
    UInt8* data = (UInt8*)msg;

    // Used same values for DNS server as specified during init of unit test
    srcaddr.type                = mDNSAddrType_IPv4;
    srcaddr.ip.v4.NotAnInteger    = dns_server_ipv4.NotAnInteger;
    srcport.NotAnInteger        = client_resp_src_port;

    // Used random value for dstport
    dstport.NotAnInteger = swap16((mDNSu16)client_resp_dst_port);

    // Set DNS message (that was copied from a WireShark packet)
    end = (const mDNSu8 *)msg + msgSize;

    // Set socket info that mDNSCoreReceive uses to verify socket context
    q->LocalSocket->ss.port.NotAnInteger = swap16((mDNSu16)client_resp_dst_port);
    if (suspiciousqid.NotAnInteger)
    {
        q->TargetQID.NotAnInteger = swap16(suspiciousqid.NotAnInteger);
        if (goodLastQID)
        {
            q->LastTargetQID.b[0] = data[0];
            q->LastTargetQID.b[1] = data[1];
        }
        else q->LastTargetQID.NotAnInteger = 0;
    }
    else
    {
        q->TargetQID.b[0] = data[0];
        q->TargetQID.b[1] = data[1];
    }

    // Execute mDNSCoreReceive which copies two DNS records into the cache
    mDNSCoreReceive(m, msg, end, &srcaddr, srcport, &primary_v4, dstport, primary_interfaceID);
}

mDNSexport  size_t get_reply_len(char* name, uint16_t rdlen)
{
	size_t len = sizeof(DNSServiceFlags);
	len += sizeof(mDNSu32);     // interface index
	len += sizeof(DNSServiceErrorType);
	len += strlen(name) + 1;
	len += 3 * sizeof(mDNSu16); // type, class, rdlen
	len += rdlen;
	len += sizeof(mDNSu32);     // TTL
	return len;
}


void free_req(request_state* req)
{
	// Cleanup request's memory usage
	while (req->replies)
	{
		reply_state *reply = req->replies;
		req->replies = req->replies->next;
		mDNSPlatformMemFree(reply);
	}
	req->replies = NULL;
	mDNSPlatformMemFree(req);
}

// Unit test support functions follow
#define SA_LEN(addr) (((addr)->sa_family == AF_INET6) ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in))

mDNSexport void get_ip(const char *const name, struct sockaddr_storage *result)
{
	struct addrinfo* aiList;
	int err = getaddrinfo(name, NULL, NULL, &aiList);
	if (err) fprintf(stderr, "getaddrinfo error %d for %s", err, name);
	else memcpy(result, aiList->ai_addr, SA_LEN(aiList->ai_addr));
	if (aiList) freeaddrinfo(aiList);
}

// The AddDNSServer_ut function adds a dns server to mDNSResponder's list.
mDNSexport mStatus AddDNSServerScoped_ut(mDNSInterfaceID interfaceID, ScopeType scoped)
{
    mDNS *m = &mDNSStorage;
    m->timenow = 0;
    mDNS_Lock(m);
    domainname  d;
    mDNSAddr    addr;
    mDNSIPPort  port;
    mDNSs32     serviceID      = 0;
    mDNSu32     timeout        = dns_server_timeout;
    mDNSBool    cellIntf       = 0;
    mDNSBool    isExpensive    = 0;
    mDNSBool    isConstrained  = 0;
    mDNSBool    isCLAT46       = mDNSfalse;
    mDNSu32     resGroupID     = dns_server_resGroupID;
    mDNSBool    reqA           = mDNStrue;
    mDNSBool    reqAAAA        = mDNStrue;
    mDNSBool    reqDO          = mDNSfalse;
    d.c[0]                     = 0;
    addr.type                  = mDNSAddrType_IPv4;
    addr.ip.v4.NotAnInteger    = dns_server_ipv4.NotAnInteger;
    port.NotAnInteger          = client_resp_src_port;
    mDNS_AddDNSServer(m, &d, interfaceID, serviceID, &addr, port, scoped, timeout,
                      cellIntf, isExpensive, isConstrained, isCLAT46, resGroupID,
                      reqA, reqAAAA, reqDO);
    mDNS_Unlock(m);
    return mStatus_NoError;
}

mDNSexport mStatus AddDNSServer_ut(void)
{
    return AddDNSServerScoped_ut(primary_interfaceID, kScopeNone);
}

mDNSexport mStatus  force_uDNS_SetupDNSConfig_ut(mDNS *const m)
{
    m->p->LastConfigGeneration = 0;
    return uDNS_SetupDNSConfig(m);
}

mDNSexport mStatus verify_cache_addr_order_for_domain_ut(mDNS *const m, mDNSu8* octet, mDNSu32 count, const domainname *const name)
{
    mStatus result = mStatus_NoError;
    const CacheGroup *cg = CacheGroupForName(m, DomainNameHashValue(name), name);
    if (cg)
    {
        mDNSu32 i;
        CacheRecord **rp = (CacheRecord **)&cg->members;
        for (i = 0 ; *rp && i < count ; i++ )
        {
            if ((*rp)->resrec.rdata->u.ipv4.b[3] != octet[i])
            {
                LogInfo ("Octet %d compare failed %d != %d", i, (*rp)->resrec.rdata->u.ipv4.b[3], octet[i]);
                break;
            }
            rp = &(*rp)->next;
        }
        if (i != count) result = mStatus_Invalid;
    }
    else
    {
        LogInfo ("Cache group not found");
        result = mStatus_Invalid;
    }

    return result;
}
