/*
 * Copyright (c) 2013-2019 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _NET_NECP_H_
#define _NET_NECP_H_

#include <net/net_kev.h>

#include <netinet/in.h>
#include <netinet/in_stat.h>
#include <sys/socket.h>
#include <net/if.h>

/*
 * Name registered by the NECP
 */
#define NECP_CONTROL_NAME "com.apple.net.necp_control"

#define NECP_TLV_LENGTH_UINT32  1

struct necp_packet_header {
	u_int8_t            packet_type;
	u_int8_t                flags;
	u_int32_t           message_id;
};

/*
 * Control message commands
 */
#define NECP_PACKET_TYPE_POLICY_ADD                             1
#define NECP_PACKET_TYPE_POLICY_GET                             2
#define NECP_PACKET_TYPE_POLICY_DELETE                  3
#define NECP_PACKET_TYPE_POLICY_APPLY_ALL               4
#define NECP_PACKET_TYPE_POLICY_LIST_ALL                5
#define NECP_PACKET_TYPE_POLICY_DELETE_ALL              6
#define NECP_PACKET_TYPE_SET_SESSION_PRIORITY   7
#define NECP_PACKET_TYPE_LOCK_SESSION_TO_PROC   8
#define NECP_PACKET_TYPE_REGISTER_SERVICE               9
#define NECP_PACKET_TYPE_UNREGISTER_SERVICE             10
#define NECP_PACKET_TYPE_POLICY_DUMP_ALL                11

/*
 * Session actions
 */
#define NECP_SESSION_ACTION_POLICY_ADD                          1       // In: Policy TLVs				Out: necp_policy_id
#define NECP_SESSION_ACTION_POLICY_GET                          2       // In: necp_policy_id			Out: Policy TLVs
#define NECP_SESSION_ACTION_POLICY_DELETE                       3       // In: necp_policy_id			Out: None
#define NECP_SESSION_ACTION_POLICY_APPLY_ALL            4       // In: None						Out: None
#define NECP_SESSION_ACTION_POLICY_LIST_ALL                     5       // In: None						Out: TLVs of IDs
#define NECP_SESSION_ACTION_POLICY_DELETE_ALL           6       // In: None						Out: None
#define NECP_SESSION_ACTION_SET_SESSION_PRIORITY        7       // In: necp_session_priority	Out: None
#define NECP_SESSION_ACTION_LOCK_SESSION_TO_PROC        8       // In: None						Out: None
#define NECP_SESSION_ACTION_REGISTER_SERVICE            9       // In: uuid_t					Out: None
#define NECP_SESSION_ACTION_UNREGISTER_SERVICE          10      // In: uuid_t					Out: None
#define NECP_SESSION_ACTION_POLICY_DUMP_ALL                     11      // In: None						Out: uint32_t bytes length, then Policy TLVs

/*
 * Control message flags
 */
#define NECP_PACKET_FLAGS_RESPONSE                              0x01    // Used for acks, errors, and query responses

/*
 * Control message TLV types
 */
#define NECP_TLV_NIL                                                    0
#define NECP_TLV_ERROR                                                  1       // u_int32_t
#define NECP_TLV_POLICY_ORDER                                   2       // u_int32_t
#define NECP_TLV_POLICY_CONDITION                               3
#define NECP_TLV_POLICY_RESULT                                  4
#define NECP_TLV_POLICY_ID                                              5       // u_int32_t
#define NECP_TLV_SESSION_PRIORITY                               6       // u_int32_t
#define NECP_TLV_ATTRIBUTE_DOMAIN                               7       // char[]
#define NECP_TLV_ATTRIBUTE_ACCOUNT                              8       // char[]
#define NECP_TLV_SERVICE_UUID                                   9       // uuid_t
#define NECP_TLV_ROUTE_RULE                                             10

/*
 * Control message TLV sent only by the kernel to userspace
 */
#define NECP_TLV_POLICY_OWNER                                   100     // char []
#define NECP_TLV_POLICY_DUMP                                    101
#define NECP_TLV_POLICY_RESULT_STRING                   102     // char []
#define NECP_TLV_POLICY_SESSION_ORDER                   103     // u_int32_t

/*
 * Condition flags
 */
#define NECP_POLICY_CONDITION_FLAGS_NEGATIVE    0x01 // Negative

/*
 * Conditions
 * Used for setting policies as well as passing parameters to necp_match_policy.
 */
#define NECP_POLICY_CONDITION_DEFAULT                   0       // N/A, not valid with any other conditions
// Socket/Application conditions
#define NECP_POLICY_CONDITION_APPLICATION               1       // uuid_t, uses effective UUID when possible
#define NECP_POLICY_CONDITION_REAL_APPLICATION  2       // uuid_t, never uses effective UUID. Only valid with NECP_POLICY_CONDITION_APPLICATION
#define NECP_POLICY_CONDITION_DOMAIN                    3       // String, such as apple.com
#define NECP_POLICY_CONDITION_ACCOUNT                   4       // String
#define NECP_POLICY_CONDITION_ENTITLEMENT               5       // String
#define NECP_POLICY_CONDITION_PID                               6       // pid_t
#define NECP_POLICY_CONDITION_UID                               7       // uid_t
#define NECP_POLICY_CONDITION_ALL_INTERFACES    8       // N/A
#define NECP_POLICY_CONDITION_BOUND_INTERFACE   9       // String
#define NECP_POLICY_CONDITION_TRAFFIC_CLASS             10      // necp_policy_condition_tc_range
// Socket/IP conditions
#define NECP_POLICY_CONDITION_IP_PROTOCOL               11      // u_int16_t
#define NECP_POLICY_CONDITION_LOCAL_ADDR                12      // necp_policy_condition_addr
#define NECP_POLICY_CONDITION_REMOTE_ADDR               13      // necp_policy_condition_addr
#define NECP_POLICY_CONDITION_LOCAL_ADDR_RANGE  14      // necp_policy_condition_addr_range
#define NECP_POLICY_CONDITION_REMOTE_ADDR_RANGE 15      // necp_policy_condition_addr_range
#define NECP_POLICY_CONDITION_AGENT_TYPE                16      // struct necp_policy_condition_agent_type
#define NECP_POLICY_CONDITION_HAS_CLIENT                17      // N/A
#define NECP_POLICY_CONDITION_LOCAL_NETWORKS            18      // Matches all local networks
// Socket-only conditions
#define NECP_POLICY_CONDITION_FLOW_IP_PROTOCOL          19      // u_int16_t
#define NECP_POLICY_CONDITION_FLOW_LOCAL_ADDR           20      // necp_policy_condition_addr
#define NECP_POLICY_CONDITION_FLOW_REMOTE_ADDR          21      // necp_policy_condition_addr
#define NECP_POLICY_CONDITION_FLOW_LOCAL_ADDR_RANGE     22      // necp_policy_condition_addr_range
#define NECP_POLICY_CONDITION_FLOW_REMOTE_ADDR_RANGE    23      // necp_policy_condition_addr_range
// Socket/Application conditions, continued
#define NECP_POLICY_CONDITION_CLIENT_FLAGS              24      // u_int32_t, values from NECP_CLIENT_PARAMETER_FLAG_*
#define NECP_POLICY_CONDITION_FLOW_LOCAL_ADDR_EMPTY     25      // N/A
#define NECP_POLICY_CONDITION_FLOW_REMOTE_ADDR_EMPTY    26      // N/A
#define NECP_POLICY_CONDITION_PLATFORM_BINARY           27      // N/A
#define NECP_POLICY_CONDITION_SIGNING_IDENTIFIER        28      // String

/*
 * Results
 */
#define NECP_POLICY_RESULT_PASS                                 1       // N/A
#define NECP_POLICY_RESULT_SKIP                                 2       // u_int32_t, policy order to skip to. 0 to skip all session policies.
#define NECP_POLICY_RESULT_DROP                                 3       // N/A
#define NECP_POLICY_RESULT_SOCKET_DIVERT                4       // u_int32_t, flow divert control unit
#define NECP_POLICY_RESULT_SOCKET_FILTER                5       // u_int32_t, filter control unit
#define NECP_POLICY_RESULT_IP_TUNNEL                    6       // String, interface name
#define NECP_POLICY_RESULT_IP_FILTER                    7       // ?
#define NECP_POLICY_RESULT_TRIGGER                              8       // service uuid_t
#define NECP_POLICY_RESULT_TRIGGER_IF_NEEDED    9       // service uuid_t
#define NECP_POLICY_RESULT_TRIGGER_SCOPED               10      // service uuid_t
#define NECP_POLICY_RESULT_NO_TRIGGER_SCOPED    11      // service uuid_t
#define NECP_POLICY_RESULT_SOCKET_SCOPED                12      // String, interface name
#define NECP_POLICY_RESULT_ROUTE_RULES                  13      // N/A, must have route rules defined
#define NECP_POLICY_RESULT_USE_NETAGENT                 14      // netagent uuid_t
#define NECP_POLICY_RESULT_NETAGENT_SCOPED              15      // netagent uuid_t
#define NECP_POLICY_RESULT_SCOPED_DIRECT                16      // N/A, scopes to primary physical interface
#define NECP_POLICY_RESULT_ALLOW_UNENTITLED             17      // N/A

#define NECP_POLICY_RESULT_MAX                          NECP_POLICY_RESULT_ALLOW_UNENTITLED

/*
 * PASS Result Flags
 */
#define NECP_POLICY_PASS_NO_SKIP_IPSEC                  0x01

/*
 * Route Rules
 * Detailed parameters for NECP_POLICY_RESULT_ROUTE_RULES.
 */
#define NECP_ROUTE_RULE_NONE                                    0       // N/A
#define NECP_ROUTE_RULE_DENY_INTERFACE                  1       // String, or empty to match all
#define NECP_ROUTE_RULE_ALLOW_INTERFACE                 2       // String, or empty to match all
#define NECP_ROUTE_RULE_QOS_MARKING                             3       // String, or empty to match all
#define NECP_ROUTE_RULE_DENY_LQM_ABORT                  4       // String, or empty to match all

#define NECP_ROUTE_RULE_FLAG_CELLULAR                   0x01
#define NECP_ROUTE_RULE_FLAG_WIFI                       0x02
#define NECP_ROUTE_RULE_FLAG_WIRED                      0x04
#define NECP_ROUTE_RULE_FLAG_EXPENSIVE                  0x08
#define NECP_ROUTE_RULE_FLAG_CONSTRAINED                0x10

/*
 * Error types
 */
#define NECP_ERROR_INTERNAL                                             0
#define NECP_ERROR_UNKNOWN_PACKET_TYPE                  1
#define NECP_ERROR_INVALID_TLV                                  2
#define NECP_ERROR_POLICY_RESULT_INVALID                3
#define NECP_ERROR_POLICY_CONDITIONS_INVALID    4
#define NECP_ERROR_POLICY_ID_NOT_FOUND                  5
#define NECP_ERROR_INVALID_PROCESS                              6
#define NECP_ERROR_ROUTE_RULES_INVALID                  7

// Modifiers
#define NECP_MASK_USERSPACE_ONLY        0x80000000      // on filter_control_unit value

struct necp_policy_condition_tc_range {
	u_int32_t start_tc;
	u_int32_t end_tc;
} __attribute__((__packed__));

struct necp_policy_condition_addr {
	u_int8_t                prefix;
	union {
		struct sockaddr                 sa;
		struct sockaddr_in              sin;
		struct sockaddr_in6             sin6;
	} address;
} __attribute__((__packed__));

struct necp_policy_condition_addr_range {
	union {
		struct sockaddr                 sa;
		struct sockaddr_in              sin;
		struct sockaddr_in6             sin6;
	} start_address;
	union {
		struct sockaddr                 sa;
		struct sockaddr_in              sin;
		struct sockaddr_in6             sin6;
	} end_address;
} __attribute__((__packed__));

struct necp_policy_condition_agent_type {
	char agent_domain[32];
	char agent_type[32];
} __attribute__((__packed__));

#define NECP_SESSION_PRIORITY_UNKNOWN                   0
#define NECP_SESSION_PRIORITY_CONTROL                   1
#define NECP_SESSION_PRIORITY_PRIVILEGED_TUNNEL         2
#define NECP_SESSION_PRIORITY_HIGH                      3
#define NECP_SESSION_PRIORITY_HIGH_1                    4
#define NECP_SESSION_PRIORITY_HIGH_2                    5
#define NECP_SESSION_PRIORITY_HIGH_3                    6
#define NECP_SESSION_PRIORITY_HIGH_4                    7
#define NECP_SESSION_PRIORITY_HIGH_RESTRICTED           8
#define NECP_SESSION_PRIORITY_DEFAULT                   9
#define NECP_SESSION_PRIORITY_LOW                       10
#define NECP_SESSION_NUM_PRIORITIES                             NECP_SESSION_PRIORITY_LOW

typedef u_int32_t necp_policy_id;
typedef u_int32_t necp_policy_order;
typedef u_int32_t necp_session_priority;

typedef u_int32_t necp_kernel_policy_result;
typedef u_int32_t necp_kernel_policy_filter;

typedef union {
	u_int                                           tunnel_interface_index;
	u_int                                           scoped_interface_index;
	u_int32_t                                       flow_divert_control_unit;
	u_int32_t                                       filter_control_unit;
} necp_kernel_policy_routing_result_parameter;

#define NECP_SERVICE_FLAGS_REGISTERED                   0x01
#define NECP_MAX_NETAGENTS                                              8

#define NECP_AGENT_USE_FLAG_SCOPE                               0x01

#define NECP_TFO_COOKIE_LEN_MAX      16
struct necp_aggregate_result {
	necp_kernel_policy_result                       routing_result;
	necp_kernel_policy_routing_result_parameter     routing_result_parameter;
	necp_kernel_policy_filter                       filter_control_unit;
	necp_kernel_policy_result                       service_action;
	uuid_t                                                          service_uuid;
	u_int32_t                                                       service_flags;
	u_int32_t                                                       service_data;
	u_int                                                           routed_interface_index;
	u_int32_t                                                       policy_id;
	uuid_t                                                          netagents[NECP_MAX_NETAGENTS];
	u_int32_t                                                       netagent_use_flags[NECP_MAX_NETAGENTS];
	u_int8_t                                                        mss_recommended;
};

/*
 * Statistics.  It would be nice if the definitions in ntstat.h could be used,
 * but they get entangled with #defines for v4 etc in pfvar.h and it may be better practice
 * to have separate definitions here.
 */
struct necp_stat_counts {
	/*	Counters	*/
	u_int64_t       necp_stat_rxpackets             __attribute__((aligned(8)));
	u_int64_t       necp_stat_rxbytes               __attribute__((aligned(8)));
	u_int64_t       necp_stat_txpackets             __attribute__((aligned(8)));
	u_int64_t       necp_stat_txbytes               __attribute__((aligned(8)));

	u_int32_t       necp_stat_rxduplicatebytes;
	u_int32_t       necp_stat_rxoutoforderbytes;
	u_int32_t       necp_stat_txretransmit;

	u_int32_t       necp_stat_connectattempts;
	u_int32_t       necp_stat_connectsuccesses;

	u_int32_t       necp_stat_min_rtt;
	u_int32_t       necp_stat_avg_rtt;
	u_int32_t       necp_stat_var_rtt;

#define NECP_STAT_ROUTE_FLAGS   1
	u_int32_t       necp_stat_route_flags;
};

// Note, some metadata is implicit in the necp client itself:
// From the process itself : pid, upid, uuid, proc name.
// From the necp client parameters: local and remote addresses, euuid, traffic class, ifindex
//
// The following may well be supplied via future necp client parameters,
// but they are here so they don't get forgotten.
struct necp_basic_metadata {
	u_int32_t       rcvbufsize;
	u_int32_t       rcvbufused;
};

struct necp_connection_probe_status {
	unsigned int    probe_activated : 1;
	unsigned int    write_probe_failed : 1;
	unsigned int    read_probe_failed : 1;
	unsigned int    conn_probe_failed : 1;
};

struct necp_extra_tcp_metadata {
	struct necp_connection_probe_status probestatus;

	u_int32_t       sndbufsize;
	u_int32_t       sndbufused;
	u_int32_t       txunacked;
	u_int32_t       txwindow;
	u_int32_t       txcwindow;
	u_int32_t       flags;                  // use SOF_*
	u_int32_t       flags1;                 // use SOF1_*
	u_int32_t       traffic_mgt_flags;
	u_int32_t       cc_alg_index;
	u_int32_t       state;
};

struct necp_stats_hdr {
	u_int32_t                                       necp_stats_type __attribute__((aligned(8)));
	u_int32_t                                       necp_stats_ver;
	u_int64_t                                       __necp_stats_reserved; // Pad the field for future use
};

#define NECP_CLIENT_STATISTICS_TYPE_TCP                         1       // Identifies use of necp_tcp_stats
#define NECP_CLIENT_STATISTICS_TYPE_UDP                         2       // Identifies use of necp_udp_stats
#define NECP_CLIENT_STATISTICS_TYPE_QUIC                        3       // Identifies use of necp_quic_stats

#define NECP_CLIENT_STATISTICS_TYPE_TCP_VER_1           1       // Currently supported version for TCP
#define NECP_CLIENT_STATISTICS_TYPE_UDP_VER_1           1       // Currently supported version for UDP
#define NECP_CLIENT_STATISTICS_TYPE_QUIC_VER_1          1       // Currently supported version for QUIC

#define NECP_CLIENT_STATISTICS_TYPE_TCP_CURRENT_VER             NECP_CLIENT_STATISTICS_TYPE_TCP_VER_1
#define NECP_CLIENT_STATISTICS_TYPE_UDP_CURRENT_VER             NECP_CLIENT_STATISTICS_TYPE_UDP_VER_1
#define NECP_CLIENT_STATISTICS_TYPE_QUIC_CURRENT_VER            NECP_CLIENT_STATISTICS_TYPE_QUIC_VER_1

#define NECP_CLIENT_STATISTICS_EVENT_INIT                       0x00000000              // Register the flow
#define NECP_CLIENT_STATISTICS_EVENT_TIME_WAIT          0x00000001              // The flow is effectively finished but waiting on timer

struct necp_tcp_stats {
	struct necp_stats_hdr                   necp_tcp_hdr;
	struct necp_stat_counts                 necp_tcp_counts;
	struct necp_basic_metadata              necp_tcp_basic;
	struct necp_extra_tcp_metadata  necp_tcp_extra;
};

struct necp_udp_stats {
	struct necp_stats_hdr           necp_udp_hdr;
	struct necp_stat_counts         necp_udp_counts;
	struct necp_basic_metadata      necp_udp_basic;
};


/*
 * The following reflects the special case for QUIC.
 * It is a streaming protocol built on top of UDP.
 * Therefore QUIC stats are defined as basic UDP stats
 * with some extra meta data.
 * TODO: For now the extra metadata is an exact replica
 * of the metadata for TCP. However keeping that separate allows
 * the structures to diverge later as new stats are added.
 */
#define QUIC_STATELESS_RESET_TOKEN_SIZE               16
struct necp_extra_quic_metadata {
	u_int32_t       sndbufsize;
	u_int32_t       sndbufused;
	u_int32_t       txunacked;
	u_int32_t       txwindow;
	u_int32_t       txcwindow;
	u_int32_t       traffic_mgt_flags;
	u_int32_t       cc_alg_index;
	u_int32_t       state;
	u_int8_t        ssr_token[QUIC_STATELESS_RESET_TOKEN_SIZE];
};

#define necp_quic_hdr           necp_quic_udp_stats.necp_udp_hdr
#define necp_quic_counts        necp_quic_udp_stats.necp_udp_counts
#define necp_quic_basic         necp_quic_udp_stats.necp_udp_basic
struct necp_quic_stats {
	struct necp_udp_stats           necp_quic_udp_stats;
	struct necp_extra_quic_metadata necp_quic_extra;
};

typedef struct necp_all_stats {
	union {
		struct necp_tcp_stats   tcp_stats;
		struct necp_udp_stats   udp_stats;
		struct necp_quic_stats  quic_stats;
	} all_stats_u;
} necp_all_stats;

// Memory for statistics is requested via a necp_stats_bufreq
//
struct necp_stats_bufreq {
	u_int32_t                                       necp_stats_bufreq_id __attribute__((aligned(8)));
	u_int32_t                                       necp_stats_bufreq_type;         //  NECP_CLIENT_STATISTICS_TYPE_*
	u_int32_t                                       necp_stats_bufreq_ver;          //  NECP_CLIENT_STATISTICS_TYPE_*_VER
	u_int32_t                                       necp_stats_bufreq_size;
	union {
		void                                    *necp_stats_bufreq_addr;
		mach_vm_address_t               necp_stats_bufreq_uaddr;
	};
};

#define NECP_CLIENT_STATISTICS_BUFREQ_ID                                0xbf    // Distinguishes from statistics actions taking a necp_all_stats struct

// There is a limit to the number of statistics structures that may be allocated per process, subject to change
//
#define NECP_MAX_PER_PROCESS_CLIENT_STATISTICS_STRUCTS  512

#define NECP_TCP_ECN_HEURISTICS_SYN_RST 1
typedef struct necp_tcp_ecn_cache {
	u_int8_t                necp_tcp_ecn_heuristics_success:1;
	u_int8_t                necp_tcp_ecn_heuristics_loss:1;
	u_int8_t                necp_tcp_ecn_heuristics_drop_rst:1;
	u_int8_t                necp_tcp_ecn_heuristics_drop_rxmt:1;
	u_int8_t                necp_tcp_ecn_heuristics_aggressive:1;
	u_int8_t                necp_tcp_ecn_heuristics_syn_rst:1;
} necp_tcp_ecn_cache;

#define NECP_TCP_TFO_HEURISTICS_RST 1
typedef struct necp_tcp_tfo_cache {
	u_int8_t                necp_tcp_tfo_cookie[NECP_TFO_COOKIE_LEN_MAX];
	u_int8_t                necp_tcp_tfo_cookie_len;
	u_int8_t                necp_tcp_tfo_heuristics_success:1; // TFO succeeded with data in the SYN
	u_int8_t                necp_tcp_tfo_heuristics_loss:1; // TFO SYN-loss with data
	u_int8_t                necp_tcp_tfo_heuristics_middlebox:1; // TFO middlebox detected
	u_int8_t                necp_tcp_tfo_heuristics_success_req:1; // TFO succeeded with the TFO-option in the SYN
	u_int8_t                necp_tcp_tfo_heuristics_loss_req:1; // TFO SYN-loss with the TFO-option
	u_int8_t                necp_tcp_tfo_heuristics_rst_data:1; // Recevied RST upon SYN with data in the SYN
	u_int8_t                necp_tcp_tfo_heuristics_rst_req:1; // Received RST upon SYN with the TFO-option
} necp_tcp_tfo_cache;

#define NECP_CLIENT_CACHE_TYPE_ECN                 1       // Identifies use of necp_tcp_ecn_cache
#define NECP_CLIENT_CACHE_TYPE_TFO                 2       // Identifies use of necp_tcp_tfo_cache

#define NECP_CLIENT_CACHE_TYPE_ECN_VER_1           1       // Currently supported version for ECN
#define NECP_CLIENT_CACHE_TYPE_TFO_VER_1           1       // Currently supported version for TFO

typedef struct necp_cache_buffer {
	u_int8_t                necp_cache_buf_type;    //  NECP_CLIENT_CACHE_TYPE_*
	u_int8_t                necp_cache_buf_ver;     //  NECP_CLIENT_CACHE_TYPE_*_VER
	u_int32_t               necp_cache_buf_size;
	mach_vm_address_t       necp_cache_buf_addr;
} necp_cache_buffer;

/*
 * NECP Client definitions
 */
#define NECP_MAX_CLIENT_PARAMETERS_SIZE                                 1024
#define NECP_MAX_CLIENT_RESULT_SIZE                                             512

#define NECP_OPEN_FLAG_OBSERVER                                                 0x01 // Observers can query clients they don't own
#define NECP_OPEN_FLAG_BACKGROUND                                               0x02 // Mark this fd as backgrounded
#define NECP_OPEN_FLAG_PUSH_OBSERVER                                    0x04 // When used with the OBSERVER flag, allows updates to be pushed. Adding clients is not allowed in this mode.

#define NECP_FD_SUPPORTS_GUARD                                                  1

#define NECP_CLIENT_ACTION_ADD                                                  1 // Register a new client. Input: parameters in buffer; Output: client_id
#define NECP_CLIENT_ACTION_REMOVE                                               2 // Unregister a client. Input: client_id, optional struct ifnet_stats_per_flow
#define NECP_CLIENT_ACTION_COPY_PARAMETERS                              3 // Copy client parameters. Input: client_id; Output: parameters in buffer
#define NECP_CLIENT_ACTION_COPY_RESULT                                  4 // Copy client result. Input: client_id; Output: result in buffer
#define NECP_CLIENT_ACTION_COPY_LIST                                    5 // Copy all client IDs. Output: struct necp_client_list in buffer
#define NECP_CLIENT_ACTION_REQUEST_NEXUS_INSTANCE               6 // Request a nexus instance from a nexus provider, optional struct necp_stats_bufreq
#define NECP_CLIENT_ACTION_AGENT                                                7 // Interact with agent. Input: client_id, agent parameters
#define NECP_CLIENT_ACTION_COPY_AGENT                                   8 // Copy agent content. Input: agent UUID; Output: struct netagent
#define NECP_CLIENT_ACTION_COPY_INTERFACE                               9 // Copy interface details. Input: ifindex cast to UUID; Output: struct necp_interface_details
#define NECP_CLIENT_ACTION_SET_STATISTICS                               10 // Deprecated
#define NECP_CLIENT_ACTION_COPY_ROUTE_STATISTICS                11 // Get route statistics. Input: client_id; Output: struct necp_stat_counts
#define NECP_CLIENT_ACTION_AGENT_USE                                    12 // Return the use count and increment the use count. Input/Output: struct necp_agent_use_parameters
#define NECP_CLIENT_ACTION_MAP_SYSCTLS                                  13 // Get the read-only sysctls memory location. Output: mach_vm_address_t
#define NECP_CLIENT_ACTION_UPDATE_CACHE                                 14 // Update heuristics and cache
#define NECP_CLIENT_ACTION_COPY_CLIENT_UPDATE                   15 // Fetch an updated client for push-mode observer. Output: Client id, struct necp_client_observer_update in buffer
#define NECP_CLIENT_ACTION_COPY_UPDATED_RESULT                  16 // Copy client result only if changed. Input: client_id; Output: result in buffer
#define NECP_CLIENT_ACTION_ADD_FLOW                                             17 // Add a flow. Input: client_id; Output: struct necp_client_add_flow
#define NECP_CLIENT_ACTION_REMOVE_FLOW                                  18 // Remove a flow. Input: flow_id, optional struct ifnet_stats_per_flow
#define NECP_CLIENT_ACTION_CLAIM                                       19 // Claim a client that has been added for this unique PID. Input: client_id
#define NECP_CLIENT_ACTION_SIGN                                       20 // Sign a resolver answer. Input: struct necp_client_resolver_answer; Output: signed tag, expected to be 32 bytes

#define NECP_CLIENT_PARAMETER_APPLICATION                               NECP_POLICY_CONDITION_APPLICATION               // Requires entitlement
#define NECP_CLIENT_PARAMETER_REAL_APPLICATION                  NECP_POLICY_CONDITION_REAL_APPLICATION  // Requires entitlement
#define NECP_CLIENT_PARAMETER_DOMAIN                                    NECP_POLICY_CONDITION_DOMAIN
#define NECP_CLIENT_PARAMETER_ACCOUNT                                   NECP_POLICY_CONDITION_ACCOUNT
#define NECP_CLIENT_PARAMETER_PID                                               NECP_POLICY_CONDITION_PID                               // Requires entitlement
#define NECP_CLIENT_PARAMETER_UID                                               NECP_POLICY_CONDITION_UID                               // Requires entitlement
#define NECP_CLIENT_PARAMETER_BOUND_INTERFACE                   NECP_POLICY_CONDITION_BOUND_INTERFACE
#define NECP_CLIENT_PARAMETER_TRAFFIC_CLASS                             NECP_POLICY_CONDITION_TRAFFIC_CLASS
#define NECP_CLIENT_PARAMETER_IP_PROTOCOL                               NECP_POLICY_CONDITION_IP_PROTOCOL
#define NECP_CLIENT_PARAMETER_LOCAL_ADDRESS                             NECP_POLICY_CONDITION_LOCAL_ADDR
#define NECP_CLIENT_PARAMETER_REMOTE_ADDRESS                    NECP_POLICY_CONDITION_REMOTE_ADDR
#define NECP_CLIENT_PARAMETER_NEXUS_KEY                                 102

// "Prohibit" will never choose an interface with that property
#define NECP_CLIENT_PARAMETER_PROHIBIT_INTERFACE                100             // String, interface name
#define NECP_CLIENT_PARAMETER_PROHIBIT_IF_TYPE                  101             // u_int8_t, see ifru_functional_type in <net/if.h>
#define NECP_CLIENT_PARAMETER_PROHIBIT_AGENT                    102             // uuid_t, network agent UUID
#define NECP_CLIENT_PARAMETER_PROHIBIT_AGENT_TYPE               103             // struct necp_client_parameter_netagent_type

// "Require" will choose an interface with that property, or none if not found
#define NECP_CLIENT_PARAMETER_REQUIRE_IF_TYPE                   111             // u_int8_t, see ifru_functional_type in <net/if.h>
#define NECP_CLIENT_PARAMETER_REQUIRE_AGENT                             112             // uuid_t, network agent UUID
#define NECP_CLIENT_PARAMETER_REQUIRE_AGENT_TYPE                113             // struct necp_client_parameter_netagent_type

// "Prefer" will choose an interface with an agent, or best otherwise if not found
#define NECP_CLIENT_PARAMETER_PREFER_AGENT                              122             // uuid_t, network agent UUID
#define NECP_CLIENT_PARAMETER_PREFER_AGENT_TYPE                 123             // struct necp_client_parameter_netagent_type

// "Avoid" will choose an interface without an agent, or best otherwise if unavoidable
#define NECP_CLIENT_PARAMETER_AVOID_AGENT                               124             // uuid_t, network agent UUID
#define NECP_CLIENT_PARAMETER_AVOID_AGENT_TYPE                  125             // struct necp_client_parameter_netagent_type

// Use actions with NECP_CLIENT_ACTION_AGENT
#define NECP_CLIENT_PARAMETER_TRIGGER_AGENT                             130             // uuid_t, network agent UUID
#define NECP_CLIENT_PARAMETER_ASSERT_AGENT                              131             // uuid_t, network agent UUID
#define NECP_CLIENT_PARAMETER_UNASSERT_AGENT                    132             // uuid_t, network agent UUID

#define NECP_CLIENT_PARAMETER_PARENT_ID                                                 150 // uuid_t, client UUID

#define NECP_CLIENT_PARAMETER_LOCAL_ENDPOINT                    200             // struct necp_client_endpoint
#define NECP_CLIENT_PARAMETER_REMOTE_ENDPOINT                   201             // struct necp_client_endpoint
#define NECP_CLIENT_PARAMETER_BROWSE_DESCRIPTOR                  202             // struct necp_client_endpoint
#define NECP_CLIENT_PARAMETER_RESOLVER_TAG                      203                             // Tag as bytes, expected to be 32 bytes
#define NECP_CLIENT_PARAMETER_ADVERTISE_DESCRIPTOR                  204             // struct necp_client_endpoint

#define NECP_CLIENT_PARAMETER_DELEGATED_UPID                              210 // u_int64_t, requires entitlement

#define NECP_CLIENT_PARAMETER_ETHERTYPE                              220 // u_int16_t, ethertype
#define NECP_CLIENT_PARAMETER_TRANSPORT_PROTOCOL                        221 // u_int8_t, IPPROTO_

#define NECP_CLIENT_PARAMETER_LOCAL_ADDRESS_PREFERENCE                        230 // u_int8_t, NECP_CLIENT_PARAMETER_LOCAL_ADDRESS_PREFERENCE_

#define NECP_CLIENT_PARAMETER_LOCAL_ADDRESS_PREFERENCE_DEFAULT                    0
#define NECP_CLIENT_PARAMETER_LOCAL_ADDRESS_PREFERENCE_TEMPORARY                    1
#define NECP_CLIENT_PARAMETER_LOCAL_ADDRESS_PREFERENCE_STABLE                    2

#define NECP_CLIENT_PARAMETER_FLAGS                                             250             // u_int32_t, see NECP_CLIENT_PAREMETER_FLAG_* values

#define NECP_CLIENT_PARAMETER_FLAG_MULTIPATH                    0x0001  // Get multipath interface results
#define NECP_CLIENT_PARAMETER_FLAG_BROWSE                               0x0002  // Agent assertions on nexuses are requests to browse
#define NECP_CLIENT_PARAMETER_FLAG_PROHIBIT_EXPENSIVE   0x0004  // Prohibit expensive interfaces
#define NECP_CLIENT_PARAMETER_FLAG_LISTENER                             0x0008  // Client is interested in listening for inbound connections
#define NECP_CLIENT_PARAMETER_FLAG_DISCRETIONARY                0x0010  // Client's traffic is discretionary, and eligible for early defuncting
#define NECP_CLIENT_PARAMETER_FLAG_ECN_ENABLE                   0x0020  // Client is requesting to enable ECN
#define NECP_CLIENT_PARAMETER_FLAG_ECN_DISABLE                  0x0040  // Client is requesting to disable ECN
#define NECP_CLIENT_PARAMETER_FLAG_TFO_ENABLE                   0x0080  // Client is requesting to enable TFO
#define NECP_CLIENT_PARAMETER_FLAG_ONLY_PRIMARY_REQUIRES_TYPE   0x0100    // Interpret NECP_CLIENT_PARAMETER_REQUIRE_IF_TYPE only for primary interface, and allow exceptions for multipath or listeners
#define NECP_CLIENT_PARAMETER_FLAG_CUSTOM_ETHER                 0x0200  // Client expects to open a custom ethernet channel
#define NECP_CLIENT_PARAMETER_FLAG_CUSTOM_IP                    0x0400  // Client expects to open a custom IP protocol channel
#define NECP_CLIENT_PARAMETER_FLAG_INTERPOSE                    0x0800  // Client expects to open an interpose filter channel
#define NECP_CLIENT_PARAMETER_FLAG_PROHIBIT_CONSTRAINED       0x1000  // Prohibit constrained interfaces
#define NECP_CLIENT_PARAMETER_FLAG_FALLBACK_TRAFFIC           0x2000  // Fallback traffic
#define NECP_CLIENT_PARAMETER_FLAG_INBOUND                    0x4000  // Flow is inbound (passive)

#define NECP_CLIENT_RESULT_CLIENT_ID                                    1               // uuid_t
#define NECP_CLIENT_RESULT_POLICY_RESULT                                2               // u_int32_t
#define NECP_CLIENT_RESULT_POLICY_RESULT_PARAMETER              3               // u_int32_t
#define NECP_CLIENT_RESULT_FILTER_CONTROL_UNIT                  4               // u_int32_t
#define NECP_CLIENT_RESULT_INTERFACE_INDEX                              5               // u_int32_t
#define NECP_CLIENT_RESULT_NETAGENT                                             6               // struct necp_client_result_netagent
#define NECP_CLIENT_RESULT_FLAGS                                                7               // u_int32_t, see NECP_CLIENT_RESULT_FLAG_* values
#define NECP_CLIENT_RESULT_INTERFACE                                    8               // struct necp_client_result_interface
#define NECP_CLIENT_RESULT_INTERFACE_OPTION                             9               // struct necp_client_interface_option
#define NECP_CLIENT_RESULT_EFFECTIVE_MTU                                10              // u_int32_t
#define NECP_CLIENT_RESULT_FLOW                                                 11              // TLV array of a single flow's state
#define NECP_CLIENT_RESULT_PROTO_CTL_EVENT                              12
#define NECP_CLIENT_RESULT_TFO_COOKIE                                   13              // NECP_TFO_COOKIE_LEN_MAX
#define NECP_CLIENT_RESULT_TFO_FLAGS                                    14              // u_int8_t
#define NECP_CLIENT_RESULT_RECOMMENDED_MSS                              15              // u_int8_t
#define NECP_CLIENT_RESULT_FLOW_ID                                              16              // uuid_t
#define NECP_CLIENT_RESULT_INTERFACE_TIME_DELTA                 17              // u_int32_t, seconds since interface up/down
#define NECP_CLIENT_RESULT_REASON                                               18              // u_int32_t, see NECP_CLIENT_RESULT_REASON_* values

#define NECP_CLIENT_RESULT_NEXUS_INSTANCE                               100             // uuid_t
#define NECP_CLIENT_RESULT_NEXUS_PORT                                   101             // u_int16_t
#define NECP_CLIENT_RESULT_NEXUS_KEY                                    102             // uuid_t
#define NECP_CLIENT_RESULT_NEXUS_PORT_FLOW_INDEX                103             // u_int32_t
#define NECP_CLIENT_RESULT_NEXUS_FLOW_STATS                             104             // struct sk_stats_flow *

#define NECP_CLIENT_RESULT_LOCAL_ENDPOINT                               200             // struct necp_client_endpoint
#define NECP_CLIENT_RESULT_REMOTE_ENDPOINT                              201             // struct necp_client_endpoint
#define NECP_CLIENT_RESULT_DISCOVERED_ENDPOINT                  202             // struct necp_client_endpoint, result of browse
#define NECP_CLIENT_RESULT_RESOLVED_ENDPOINT                  203             // struct necp_client_endpoint, result of resolve
#define NECP_CLIENT_RESULT_LOCAL_ETHER_ADDR                                     204                     // struct ether_addr
#define NECP_CLIENT_RESULT_REMOTE_ETHER_ADDR                                    205                     // struct ether_addr
#define NECP_CLIENT_RESULT_EFFECTIVE_TRAFFIC_CLASS              210             // u_int32_t
#define NECP_CLIENT_RESULT_TRAFFIC_MGMT_BG                              211             // u_int32_t, 1: background, 0: not background
#define NECP_CLIENT_RESULT_GATEWAY                                      212             // struct necp_client_endpoint

#define NECP_CLIENT_RESULT_FLAG_IS_LOCAL                                0x0001  // Routes to this device
#define NECP_CLIENT_RESULT_FLAG_IS_DIRECT                               0x0002  // Routes to directly accessible peer
#define NECP_CLIENT_RESULT_FLAG_HAS_IPV4                                0x0004  // Supports IPv4
#define NECP_CLIENT_RESULT_FLAG_HAS_IPV6                                0x0008  // Supports IPv6
#define NECP_CLIENT_RESULT_FLAG_DEFUNCT                                 0x0010  // Defunct
#define NECP_CLIENT_RESULT_FLAG_SATISFIED                               0x0020  // Satisfied path
#define NECP_CLIENT_RESULT_FLAG_FLOW_ASSIGNED                   0x0040  // Assigned, the flow is active
#define NECP_CLIENT_RESULT_FLAG_FLOW_VIABLE                             0x0080  // Viable, the flow has a valid route
#define NECP_CLIENT_RESULT_FLAG_PROBE_CONNECTIVITY              0x0100  // Flow should probe connectivity
#define NECP_CLIENT_RESULT_FLAG_ECN_ENABLED                             0x0200  // ECN should be used
#define NECP_CLIENT_RESULT_FLAG_FAST_OPEN_BLOCKED               0x0400  // Fast open should not be used
#define NECP_CLIENT_RESULT_FLAG_LINK_QUALITY_ABORT              0x0800  // Link quality is very bad, recommend close connections
#define NECP_CLIENT_RESULT_FLAG_ALLOW_QOS_MARKING               0x1000  // QoS marking is allowed
#define NECP_CLIENT_RESULT_FLAG_HAS_NAT64                       0x2000  // Has NAT64 prefix
#define NECP_CLIENT_RESULT_FLAG_INTERFACE_LOW_POWER             0x4000  // Interface is in low-power mode
#define NECP_CLIENT_RESULT_FLAG_SPECIFIC_LISTENER               0x8000  // Listener should not listen on all interfaces

#define NECP_CLIENT_RESULT_FLAG_FORCE_UPDATE (NECP_CLIENT_RESULT_FLAG_HAS_IPV4 | NECP_CLIENT_RESULT_FLAG_HAS_IPV6 | NECP_CLIENT_RESULT_FLAG_HAS_NAT64 | NECP_CLIENT_RESULT_FLAG_INTERFACE_LOW_POWER)

#define NECP_CLIENT_RESULT_FAST_OPEN_SND_PROBE                  0x01    // DEPRECATED - Fast open send probe
#define NECP_CLIENT_RESULT_FAST_OPEN_RCV_PROBE                  0x02    // DEPRECATED - Fast open receive probe

#define NECP_CLIENT_RESULT_RECOMMENDED_MSS_NONE                 0x01
#define NECP_CLIENT_RESULT_RECOMMENDED_MSS_LOW                  0x02
#define NECP_CLIENT_RESULT_RECOMMENDED_MSS_MEDIUM               0x04

#define NECP_CLIENT_RESULT_REASON_EXPENSIVE_PROHIBITED          1  // Expensive networks were prohibited
#define NECP_CLIENT_RESULT_REASON_CONSTRAINED_PROHIBITED                2  // Constrained networks were prohibited
#define NECP_CLIENT_RESULT_REASON_CELLULAR_DENIED                3  // Denied by a cellular route rule
#define NECP_CLIENT_RESULT_REASON_WIFI_DENIED                4  // Denied by a wifi route rule

struct necp_interface_signature {
	u_int8_t signature[IFNET_SIGNATURELEN];
	u_int8_t signature_len;
};

struct necp_interface_details {
	char name[IFXNAMSIZ];
	u_int32_t index;
	u_int32_t generation;
	u_int32_t functional_type;
	u_int32_t delegate_index;
	u_int32_t flags; // see NECP_INTERFACE_FLAG_*
	u_int32_t mtu;
	struct necp_interface_signature ipv4_signature;
	struct necp_interface_signature ipv6_signature;
	u_int32_t ipv4_netmask;
	u_int32_t ipv4_broadcast;
};

#define NECP_INTERFACE_FLAG_EXPENSIVE                                   0x0001
#define NECP_INTERFACE_FLAG_TXSTART                                     0X0002
#define NECP_INTERFACE_FLAG_NOACKPRI                                    0x0004
#define NECP_INTERFACE_FLAG_3CARRIERAGG                                 0x0008
#define NECP_INTERFACE_FLAG_IS_LOW_POWER                                0x0010
#define NECP_INTERFACE_FLAG_MPK_LOG                                     0x0020 // Multi-layer Packet Logging
#define NECP_INTERFACE_FLAG_CONSTRAINED                                 0x0040
#define NECP_INTERFACE_FLAG_HAS_NETMASK                                 0x0080
#define NECP_INTERFACE_FLAG_HAS_BROADCAST                               0x0100

struct necp_client_parameter_netagent_type {
	char netagent_domain[32];
	char netagent_type[32];
};

struct necp_client_result_netagent {
	u_int32_t generation;
	uuid_t netagent_uuid;
};

struct necp_client_result_interface {
	u_int32_t generation;
	u_int32_t index;
};

#define NECP_USES_INTERFACE_OPTIONS_FOR_BROWSE 1

struct necp_client_interface_option {
	u_int32_t interface_index;
	u_int32_t interface_generation;
	uuid_t nexus_agent;
};

struct necp_client_endpoint {
	union {
		struct sockaddr sa;
		struct sockaddr_in sin;
		struct sockaddr_in6 sin6;
		struct {
			u_int8_t endpoint_length;
			u_int8_t endpoint_family; // Use AF_UNSPEC to target a name
			u_int16_t endpoint_port;
			u_int32_t endpoint_type; // Client-specific type
			char endpoint_data[0]; // Type-specific endpoint value
		} endpoint;
	} u;
};

struct necp_client_list {
	u_int32_t client_count;
	uuid_t clients[0];
};

struct kev_necp_policies_changed_data {
	u_int32_t               changed_count;  // Defaults to 0.
};

#define NECP_CLIENT_FLOW_FLAGS_ALLOW_NEXUS                      0x01    // Request a nexus instance upon adding a flow
#define NECP_CLIENT_FLOW_FLAGS_USE_CLIENT_ID            0x02    // Register the client ID rather than the flow registration ID with network agents
#define NECP_CLIENT_FLOW_FLAGS_BROWSE                      0x04    // Create request with a browse agent
#define NECP_CLIENT_FLOW_FLAGS_RESOLVE                      0x08    // Create request with a resolution agent

struct necp_client_flow_stats {
	u_int32_t stats_type; // NECP_CLIENT_STATISTICS_TYPE_*
	u_int32_t stats_version; // NECP_CLIENT_STATISTICS_TYPE_*_VER
	u_int32_t stats_size;
	mach_vm_address_t stats_addr;
};

struct necp_client_add_flow {
	uuid_t agent_uuid;
	uuid_t registration_id;
	u_int16_t flags; // NECP_CLIENT_FLOW_FLAGS_*
	u_int16_t stats_request_count;
	struct necp_client_flow_stats stats_requests[0];
} __attribute__((__packed__));

struct necp_agent_use_parameters {
	uuid_t agent_uuid;
	uint64_t out_use_count;
};

struct necp_client_flow_protoctl_event {
	uint32_t        protoctl_event_code;
	uint32_t        protoctl_event_val;
	/* TCP seq number is in host byte order */
	uint32_t        protoctl_event_tcp_seq_num;
};

#define NECP_CLIENT_UPDATE_TYPE_PARAMETERS              1       // Parameters, for a new client
#define NECP_CLIENT_UPDATE_TYPE_RESULT                  2       // Result, for a udpated client
#define NECP_CLIENT_UPDATE_TYPE_REMOVE                  3       // Empty, for a removed client

struct necp_client_observer_update {
	u_int32_t update_type;  // NECP_CLIENT_UPDATE_TYPE_*
	u_int8_t tlv_buffer[0]; // Parameters or result as TLVs, based on type
};

#define NECP_CLIENT_SIGN_TYPE_RESOLVER_ANSWER           1

struct necp_client_signable {
	uuid_t client_id;
	u_int32_t sign_type;
} __attribute__((__packed__));

struct necp_client_resolver_answer {
	uuid_t client_id;
	u_int32_t sign_type;
	union sockaddr_in_4_6 address_answer;
	u_int32_t hostname_length;
	// hostname
} __attribute__((__packed__));

#define NECP_FILTER_UNIT_NO_FILTER              UINT32_MAX // Reserved filter unit value that prohibits all filters and socket filters

/*
 * The sysctl "net.necp.necp_drop_dest_level" controls the global drop rule policy for
 * a set of destinations addresses at the given level -- the drop rule is the last one
 * to be evaluated at this level.
 */
#define MAX_NECP_DROP_DEST_LEVEL_ADDRS 8

struct necp_drop_dest_entry {
	u_int32_t                           level;          // priority level
	u_int32_t                           order;          // session order (read only via sysctl)
	struct necp_policy_condition_addr   cond_addr;
};

struct necp_drop_dest_policy {
	u_int32_t entry_count;
	struct necp_drop_dest_entry entries[MAX_NECP_DROP_DEST_LEVEL_ADDRS];
};




extern int necp_match_policy(const uint8_t *parameters, size_t parameters_size, struct necp_aggregate_result *returned_result);

extern int necp_open(int flags);

extern int necp_client_action(int necp_fd, uint32_t action, uuid_t client_id,
    size_t client_id_len, uint8_t *buffer, size_t buffer_size);

extern int necp_session_open(int flags);

extern int necp_session_action(int necp_fd, uint32_t action,
    uint8_t *in_buffer, size_t in_buffer_length,
    uint8_t *out_buffer, size_t out_buffer_length);



#endif
