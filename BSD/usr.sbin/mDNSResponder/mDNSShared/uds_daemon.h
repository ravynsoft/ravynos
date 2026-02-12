/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2018 Apple Inc. All rights reserved.
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

#ifndef UDS_DAEMON_H
#define UDS_DAEMON_H

#include "mDNSEmbeddedAPI.h"
#include "dnssd_ipc.h"
#include "ClientRequests.h"

/* Client request: */

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Types and Data Structures
#endif

typedef enum
{
	t_uninitialized,
	t_morecoming,
	t_complete,
	t_error,
	t_terminated
} transfer_state;

typedef struct request_state request_state;

typedef void (*req_termination_fn)(request_state *request);

typedef struct registered_record_entry
{
	struct registered_record_entry *next;
	mDNSu32 key;
	client_context_t regrec_client_context;
	request_state *request;
	mDNSBool external_advertise;
	mDNSInterfaceID origInterfaceID;
	AuthRecord *rr;             // Pointer to variable-sized AuthRecord (Why a pointer? Why not just embed it here?)
} registered_record_entry;

// A single registered service: ServiceRecordSet + bookkeeping
// Note that we duplicate some fields from parent service_info object
// to facilitate cleanup, when instances and parent may be deallocated at different times.
typedef struct service_instance
{
	struct service_instance *next;
	request_state *request;
	AuthRecord *subtypes;
	mDNSBool renameonmemfree;       // Set on config change when we deregister original name
	mDNSBool clientnotified;        // Has client been notified of successful registration yet?
	mDNSBool default_local;         // is this the "local." from an empty-string registration?
	mDNSBool external_advertise;    // is this is being advertised externally?
	domainname domain;
	ServiceRecordSet srs;           // note -- variable-sized object -- must be last field in struct
} service_instance;

// for multi-domain default browsing
typedef struct browser_t
{
	struct browser_t *next;
	domainname domain;
	DNSQuestion q;
} browser_t;

#ifdef _WIN32
typedef unsigned int pid_t;
typedef unsigned int socklen_t;
#endif

#if (!defined(MAXCOMLEN))
#define MAXCOMLEN 16
#endif

struct request_state
{
	request_state *next;
	request_state *primary;         // If this operation is on a shared socket, pointer to primary
	// request_state for the original DNSServiceCreateConnection() operation
	dnssd_sock_t sd;
	pid_t process_id;               // Client's PID value
	char  pid_name[MAXCOMLEN];      // Client's process name
	mDNSu8 uuid[UUID_SIZE];
	mDNSBool validUUID;
	dnssd_sock_t errsd;
	mDNSu32 uid;
    mDNSu32 request_id;
	void * platform_data;

	// Note: On a shared connection these fields in the primary structure, including hdr, are re-used
	// for each new request. This is because, until we've read the ipc_msg_hdr to find out what the
	// operation is, we don't know if we're going to need to allocate a new request_state or not.
	transfer_state ts;
	mDNSu32 hdr_bytes;              // bytes of header already read
	ipc_msg_hdr hdr;
	mDNSu32 data_bytes;             // bytes of message data already read
	char          *msgbuf;          // pointer to data storage to pass to free()
	const char    *msgptr;          // pointer to data to be read from (may be modified)
	char          *msgend;          // pointer to byte after last byte of message

	// reply, termination, error, and client context info
	int no_reply;                   // don't send asynchronous replies to client
	mDNSs32 time_blocked;           // record time of a blocked client
	int unresponsiveness_reports;
	struct reply_state *replies;    // corresponding (active) reply list
	req_termination_fn terminate;
	DNSServiceFlags flags;
	mDNSu32 interfaceIndex;

	union
	{
		registered_record_entry *reg_recs;  // list of registrations for a connection-oriented request
		struct
		{
			mDNSInterfaceID interface_id;
			mDNSBool default_domain;
			mDNSBool ForceMCast;
			domainname regtype;
			browser_t *browsers;
		} browser;
		struct
		{
			mDNSInterfaceID InterfaceID;
			mDNSu16 txtlen;
			void *txtdata;
			mDNSIPPort port;
			domainlabel name;
			char type_as_string[MAX_ESCAPED_DOMAIN_NAME];
			domainname type;
			mDNSBool default_domain;
			domainname host;
			mDNSBool autoname;              // Set if this name is tied to the Computer Name
			mDNSBool autorename;            // Set if this client wants us to automatically rename on conflict
			mDNSBool allowremotequery;      // Respond to unicast queries from outside the local link?
			int num_subtypes;
			service_instance *instances;
		} servicereg;
		struct
		{
			mDNSIPPort ReqExt;              // External port we originally requested, for logging purposes
			NATTraversalInfo NATinfo;
		} pm;
		struct
		{
			DNSServiceFlags flags;
			DNSQuestion q_all;
			DNSQuestion q_default;
			DNSQuestion q_autoall;
		} enumeration;
		struct
		{
			DNSQuestion qtxt;
			DNSQuestion qsrv;
			const ResourceRecord *txt;
			const ResourceRecord *srv;
			mDNSs32 ReportTime;
			mDNSBool external_advertise;
		} resolve;
        GetAddrInfoClientRequest addrinfo;
        QueryRecordClientRequest queryrecord;
	} u;
};

// struct physically sits between ipc message header and call-specific fields in the message buffer
typedef struct
{
	DNSServiceFlags flags;          // Note: This field is in NETWORK byte order
	mDNSu32 ifi;                    // Note: This field is in NETWORK byte order
	DNSServiceErrorType error;      // Note: This field is in NETWORK byte order
} reply_hdr;

typedef struct reply_state
{
	struct reply_state *next;       // If there are multiple unsent replies
	mDNSu32 totallen;
	mDNSu32 nwriten;
	ipc_msg_hdr mhdr[1];
	reply_hdr rhdr[1];
} reply_state;

/* Client interface: */

#define SRS_PORT(S) mDNSVal16((S)->RR_SRV.resrec.rdata->u.srv.port)

#define LogTimerToFD(FILE_DESCRIPTOR, MSG, T) LogToFD((FILE_DESCRIPTOR), MSG " %08X %11d  %08X %11d", (T), (T), (T)-now, (T)-now)

extern int udsserver_init(dnssd_sock_t skts[], mDNSu32 count);
extern mDNSs32 udsserver_idle(mDNSs32 nextevent);
extern void udsserver_info_dump_to_fd(int fd);
extern void udsserver_handle_configchange(mDNS *const m);
extern int udsserver_exit(void);    // should be called prior to app exit
extern void LogMcastStateInfo(mDNSBool mflag, mDNSBool start, mDNSBool mstatelog);
#define LogMcastQ       (mDNS_McastLoggingEnabled == 0) ? ((void)0) : LogMcastQuestion
#define LogMcastS       (mDNS_McastLoggingEnabled == 0) ? ((void)0) : LogMcastService
#define LogMcast        (mDNS_McastLoggingEnabled == 0) ? ((void)0) : LogMsg
#define LogMcastNoIdent (mDNS_McastLoggingEnabled == 0) ? ((void)0) : LogMsgNoIdent

/* Routines that uds_daemon expects to link against: */

typedef void (*udsEventCallback)(int fd, void *context);
extern mStatus udsSupportAddFDToEventLoop(dnssd_sock_t fd, udsEventCallback callback, void *context, void **platform_data);
extern int     udsSupportReadFD(dnssd_sock_t fd, char* buf, int len, int flags, void *platform_data);
extern mStatus udsSupportRemoveFDFromEventLoop(dnssd_sock_t fd, void *platform_data); // Note: This also CLOSES the file descriptor as well

extern void RecordUpdatedNiceLabel(mDNSs32 delay);

// Globals and functions defined in uds_daemon.c and also shared with the old "daemon.c" on OS X

extern mDNS mDNSStorage;
extern DNameListElem *AutoRegistrationDomains;
extern DNameListElem *AutoBrowseDomains;

extern int CountExistingRegistrations(domainname *srv, mDNSIPPort port);
extern void FreeExtraRR(mDNS *const m, AuthRecord *const rr, mStatus result);
extern int CountPeerRegistrations(ServiceRecordSet *const srs);

extern const char mDNSResponderVersionString_SCCS[];
#define mDNSResponderVersionString (mDNSResponderVersionString_SCCS+5)

#if DEBUG
extern void SetDebugBoundPath(void);
extern int IsDebugSocketInUse(void);
#endif

#endif /* UDS_DAEMON_H */
