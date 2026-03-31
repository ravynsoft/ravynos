/*
	Copyright (c) 2016-2019 Apple Inc. All rights reserved.
	
	dnssdutil is a command-line utility for testing the DNS-SD API.
*/

#include "DNSMessage.h"

#include <CoreUtils/CoreUtils.h>
#include <dns_sd.h>
#include <dns_sd_private.h>

#include CF_RUNTIME_HEADER

#if( TARGET_OS_DARWIN )
	#include <CFNetwork/CFHost.h>
	#include <CoreFoundation/CoreFoundation.h>
	#include <SystemConfiguration/SCPrivate.h>
	#include <dnsinfo.h>
	#include <libproc.h>
	#include <netdb.h>
	#include <pcap.h>
	#include <spawn.h>
	#include <sys/proc_info.h>
	#include <xpc/xpc.h>
#endif

#if( TARGET_OS_POSIX )
	#include <sys/resource.h>
#endif

#if( !defined( DNSSDUTIL_INCLUDE_DNSCRYPT ) )
	#define DNSSDUTIL_INCLUDE_DNSCRYPT		0
#endif

#if( DNSSDUTIL_INCLUDE_DNSCRYPT )
	#include "tweetnacl.h"	// TweetNaCl from <https://tweetnacl.cr.yp.to/software.html>.
#endif

#if( !defined( MDNSRESPONDER_PROJECT ) )
	#define MDNSRESPONDER_PROJECT		0
#endif

#if( MDNSRESPONDER_PROJECT )
	#include <dns_services.h>
	#include "mdns_private.h"
    #include "TestUtils.h"
#endif

//===========================================================================================================================
//	Versioning
//===========================================================================================================================

#define kDNSSDUtilNumVersion	NumVersionBuild( 2, 0, 0, kVersionStageBeta, 0 )

#if( !MDNSRESPONDER_PROJECT && !defined( DNSSDUTIL_SOURCE_VERSION ) )
	#define DNSSDUTIL_SOURCE_VERSION	"0.0.0"
#endif

#define kDNSSDUtilIdentifier		"com.apple.dnssdutil"

//===========================================================================================================================
//	DNS-SD
//===========================================================================================================================

// DNS-SD API flag descriptors

#define kDNSServiceFlagsDescriptors		\
	"\x00" "AutoTrigger\0"				\
	"\x01" "Add\0"						\
	"\x02" "Default\0"					\
	"\x03" "NoAutoRename\0"				\
	"\x04" "Shared\0"					\
	"\x05" "Unique\0"					\
	"\x06" "BrowseDomains\0"			\
	"\x07" "RegistrationDomains\0"		\
	"\x08" "LongLivedQuery\0"			\
	"\x09" "AllowRemoteQuery\0"			\
	"\x0A" "ForceMulticast\0"			\
	"\x0B" "KnownUnique\0"				\
	"\x0C" "ReturnIntermediates\0"		\
	"\x0D" "DenyConstrained\0"			\
	"\x0E" "ShareConnection\0"			\
	"\x0F" "SuppressUnusable\0"			\
	"\x10" "Timeout\0"					\
	"\x11" "IncludeP2P\0"				\
	"\x12" "WakeOnResolve\0"			\
	"\x13" "BackgroundTrafficClass\0"	\
	"\x14" "IncludeAWDL\0"				\
	"\x15" "Validate\0"					\
	"\x16" "UnicastResponse\0"			\
	"\x17" "ValidateOptional\0"			\
	"\x18" "WakeOnlyService\0"			\
	"\x19" "ThresholdOne\0"				\
	"\x1A" "ThresholdFinder\0"			\
	"\x1B" "DenyCellular\0"				\
	"\x1C" "ServiceIndex\0"				\
	"\x1D" "DenyExpensive\0"			\
	"\x1E" "PathEvaluationDone\0"		\
	"\x1F" "AllowExpiredAnswers\0"		\
	"\x00"

#define DNSServiceFlagsToAddRmvStr( FLAGS )		( ( (FLAGS) & kDNSServiceFlagsAdd ) ? "Add" : "Rmv" )

#define kDNSServiceProtocolDescriptors	\
	"\x00" "IPv4\0"						\
	"\x01" "IPv6\0"						\
	"\x04" "UDP\0"						\
	"\x05" "TCP\0"						\
	"\x00"

#define kBadDNSServiceRef		( (DNSServiceRef)(intptr_t) -1 )

//===========================================================================================================================
//	DNS
//===========================================================================================================================

#define kDNSPort					53
#define kDNSMaxUDPMessageSize		512
#define kDNSMaxTCPMessageSize		UINT16_MAX

#define kDNSRecordDataLengthMax		UINT16_MAX

//===========================================================================================================================
//	mDNS
//===========================================================================================================================

#define kMDNSPort		5353

#define kDefaultMDNSMessageID		0
#define kDefaultMDNSQueryFlags		0

#define kQClassUnicastResponseBit		( 1U << 15 )
#define kRRClassCacheFlushBit			( 1U << 15 )

// Recommended Resource Record TTL values. See <https://tools.ietf.org/html/rfc6762#section-10>.

#define kMDNSRecordTTL_Host			120		// TTL for resource records related to a host name, e.g., A, AAAA, SRV, etc.
#define kMDNSRecordTTL_Other		4500	// TTL for other resource records.

// Maximum mDNS Message Size. See <https://tools.ietf.org/html/rfc6762#section-17>.

#define kMDNSMessageSizeMax		8952	// 9000 B (Ethernet jumbo frame max size) - 40 B (IPv6 header) - 8 B (UDP header)

#define kLocalStr			"\x05" "local"
#define kLocalLabel			( (const uint8_t *) kLocalStr )
#define kLocalName			( (const uint8_t *) kLocalStr )
#define kLocalNameLen		sizeof( kLocalStr )

//===========================================================================================================================
//	Test Address Blocks
//===========================================================================================================================

// IPv4 address block 203.0.113.0/24 (TEST-NET-3) is reserved for documentation. See <https://tools.ietf.org/html/rfc5737>.

#define kDNSServerBaseAddrV4		UINT32_C( 0xCB007100 )	// 203.0.113.0/24

// IPv6 address block 2001:db8::/32 is reserved for documentation. See <https://tools.ietf.org/html/rfc3849>.

static const uint8_t		kDNSServerBaseAddrV6[] =
{
	0x20, 0x01, 0x0D, 0xB8, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// 2001:db8:1::/120
};

static const uint8_t		kMDNSReplierBaseAddrV6[] =
{
	0x20, 0x01, 0x0D, 0xB8, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// 2001:db8:2::/96
};

check_compile_time( sizeof( kDNSServerBaseAddrV6 )   == 16 );
check_compile_time( sizeof( kMDNSReplierBaseAddrV6 ) == 16 );

// Bad IPv4 and IPv6 Address Blocks
// Used by the DNS server when it needs to respond with intentionally "bad" A/AAAA record data, i.e., IP addresses neither
// in 203.0.113.0/24 nor 2001:db8:1::/120.

#define kDNSServerBadBaseAddrV4		UINT32_C( 0x00000000 )	// 0.0.0.0/24

static const uint8_t		kDNSServerBadBaseAddrV6[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00	// ::ffff:0:0/120
};

check_compile_time( sizeof( kDNSServerBadBaseAddrV6 ) == 16 );

//===========================================================================================================================
//	Soft Linking
//===========================================================================================================================

#if( TARGET_OS_DARWIN )
SOFT_LINK_LIBRARY_EX( "/usr/lib/system", system_dnssd );
SOFT_LINK_FUNCTION_EX( system_dnssd, DNSServiceSleepKeepalive_sockaddr,
	DNSServiceErrorType, (
		DNSServiceRef *					sdRef,
		DNSServiceFlags					flags,
		const struct sockaddr *			localAddr,
		const struct sockaddr *			remoteAddr,
		unsigned int					timeout,
		DNSServiceSleepKeepaliveReply	callBack,
		void *							context ),
	( sdRef, flags, localAddr, remoteAddr, timeout, callBack, context ) );
#endif

//===========================================================================================================================
//	Misc.
//===========================================================================================================================

#define kLowerAlphaNumericCharSet			"abcdefghijklmnopqrstuvwxyz0123456789"
#define kLowerAlphaNumericCharSetSize		sizeof_string( kLowerAlphaNumericCharSet )

#if( !defined( kWhiteSpaceCharSet ) )
	#define kWhiteSpaceCharSet		"\t\n\v\f\r "
#endif

// Note: strcpy_literal() appears in CoreUtils code, but isn't currently defined in framework headers.

#if( !defined( strcpy_literal ) )
	#define strcpy_literal( DST, SRC )		memcpy( DST, SRC, sizeof( SRC ) )
#endif

#define _RandomStringExact( CHAR_SET, CHAR_SET_SIZE, CHAR_COUNT, OUT_STRING ) \
	RandomString( CHAR_SET, CHAR_SET_SIZE, CHAR_COUNT, CHAR_COUNT, OUT_STRING )

#define kNoSuchRecordStr			"No Such Record"
#define kNoSuchRecordAStr			"No Such Record (A)"
#define kNoSuchRecordAAAAStr		"No Such Record (AAAA)"

#define kRootLabel		( (const uint8_t *) "" )

//===========================================================================================================================
//	Gerneral Command Options
//===========================================================================================================================

// Command option macros

#define Command( NAME, CALLBACK, SUB_OPTIONS, SHORT_HELP, IS_NOTCOMMON )											\
	CLI_COMMAND_EX( NAME, CALLBACK, SUB_OPTIONS, (IS_NOTCOMMON) ? kCLIOptionFlags_NotCommon : kCLIOptionFlags_None,	\
		(SHORT_HELP), NULL )

#define kRequiredOptionSuffix		" [REQUIRED]"

#define MultiStringOptionEx( SHORT_CHAR, LONG_NAME, VAL_PTR, VAL_COUNT_PTR, ARG_HELP, SHORT_HELP, IS_REQUIRED, LONG_HELP )	\
	CLI_OPTION_MULTI_STRING_EX( SHORT_CHAR, LONG_NAME, VAL_PTR, VAL_COUNT_PTR, ARG_HELP,									\
		(IS_REQUIRED) ? SHORT_HELP kRequiredOptionSuffix : SHORT_HELP,														\
		(IS_REQUIRED) ? kCLIOptionFlags_Required : kCLIOptionFlags_None, LONG_HELP )

#define MultiStringOption( SHORT_CHAR, LONG_NAME, VAL_PTR, VAL_COUNT_PTR, ARG_HELP, SHORT_HELP, IS_REQUIRED ) \
		MultiStringOptionEx( SHORT_CHAR, LONG_NAME, VAL_PTR, VAL_COUNT_PTR, ARG_HELP, SHORT_HELP, IS_REQUIRED, NULL )

#define IntegerOption( SHORT_CHAR, LONG_NAME, VAL_PTR, ARG_HELP, SHORT_HELP, IS_REQUIRED )	\
	CLI_OPTION_INTEGER_EX( SHORT_CHAR, LONG_NAME, VAL_PTR, ARG_HELP,						\
		(IS_REQUIRED) ? SHORT_HELP kRequiredOptionSuffix : SHORT_HELP,						\
		(IS_REQUIRED) ? kCLIOptionFlags_Required : kCLIOptionFlags_None, NULL )

#define DoubleOption( SHORT_CHAR, LONG_NAME, VAL_PTR, ARG_HELP, SHORT_HELP, IS_REQUIRED )	\
	CLI_OPTION_DOUBLE_EX( SHORT_CHAR, LONG_NAME, VAL_PTR, ARG_HELP,							\
		(IS_REQUIRED) ? SHORT_HELP kRequiredOptionSuffix : SHORT_HELP,						\
		(IS_REQUIRED) ? kCLIOptionFlags_Required : kCLIOptionFlags_None, NULL )

#define BooleanOption( SHORT_CHAR, LONG_NAME, VAL_PTR, SHORT_HELP ) \
	CLI_OPTION_BOOLEAN( (SHORT_CHAR), (LONG_NAME), (VAL_PTR), (SHORT_HELP), NULL )

#define StringOptionEx( SHORT_CHAR, LONG_NAME, VAL_PTR, ARG_HELP, SHORT_HELP, IS_REQUIRED, LONG_HELP )	\
	CLI_OPTION_STRING_EX( SHORT_CHAR, LONG_NAME, VAL_PTR, ARG_HELP,										\
		(IS_REQUIRED) ? SHORT_HELP kRequiredOptionSuffix : SHORT_HELP,									\
		(IS_REQUIRED) ? kCLIOptionFlags_Required : kCLIOptionFlags_None, LONG_HELP )

#define StringOption( SHORT_CHAR, LONG_NAME, VAL_PTR, ARG_HELP, SHORT_HELP, IS_REQUIRED ) \
	StringOptionEx( SHORT_CHAR, LONG_NAME, VAL_PTR, ARG_HELP, SHORT_HELP, IS_REQUIRED, NULL )

#define CFStringOption( SHORT_CHAR, LONG_NAME, VAL_PTR, ARG_HELP, SHORT_HELP, IS_REQUIRED )	\
	CLI_OPTION_CFSTRING_EX( SHORT_CHAR, LONG_NAME, VAL_PTR, ARG_HELP,						\
		(IS_REQUIRED) ? SHORT_HELP kRequiredOptionSuffix : SHORT_HELP,						\
		(IS_REQUIRED) ? kCLIOptionFlags_Required : kCLIOptionFlags_None, NULL )

// DNS-SD API flag options

static int		gDNSSDFlags						= 0;
static int		gDNSSDFlag_AllowExpiredAnswers	= false;
static int		gDNSSDFlag_BrowseDomains		= false;
static int		gDNSSDFlag_DenyCellular			= false;
static int		gDNSSDFlag_DenyConstrained		= false;
static int		gDNSSDFlag_DenyExpensive		= false;
static int		gDNSSDFlag_ForceMulticast		= false;
static int		gDNSSDFlag_IncludeAWDL			= false;
static int		gDNSSDFlag_KnownUnique			= false;
static int		gDNSSDFlag_NoAutoRename			= false;
static int		gDNSSDFlag_PathEvaluationDone	= false;
static int		gDNSSDFlag_RegistrationDomains	= false;
static int		gDNSSDFlag_ReturnIntermediates	= false;
static int		gDNSSDFlag_Shared				= false;
static int		gDNSSDFlag_SuppressUnusable		= false;
static int		gDNSSDFlag_Timeout				= false;
static int		gDNSSDFlag_UnicastResponse		= false;
static int		gDNSSDFlag_Unique				= false;
static int		gDNSSDFlag_WakeOnResolve		= false;

#define DNSSDFlagsOption()								\
	IntegerOption( 'f', "flags", &gDNSSDFlags, "flags",	\
		"DNSServiceFlags as an integer. This value is bitwise ORed with other single flag options.", false )

#define DNSSDFlagOption( SHORT_CHAR, FLAG_NAME ) \
	BooleanOption( SHORT_CHAR, # FLAG_NAME, &gDNSSDFlag_ ## FLAG_NAME, "Use kDNSServiceFlags" # FLAG_NAME "." )

#define DNSSDFlagsOption_AllowExpiredAnswers()		DNSSDFlagOption( 'X', AllowExpiredAnswers )
#define DNSSDFlagsOption_DenyCellular()				DNSSDFlagOption( 'C', DenyCellular )
#define DNSSDFlagsOption_DenyConstrained()			DNSSDFlagOption( 'R', DenyConstrained)
#define DNSSDFlagsOption_DenyExpensive()			DNSSDFlagOption( 'E', DenyExpensive )
#define DNSSDFlagsOption_ForceMulticast()			DNSSDFlagOption( 'M', ForceMulticast )
#define DNSSDFlagsOption_IncludeAWDL()				DNSSDFlagOption( 'A', IncludeAWDL )
#define DNSSDFlagsOption_KnownUnique()				DNSSDFlagOption( 'K', KnownUnique )
#define DNSSDFlagsOption_NoAutoRename()				DNSSDFlagOption( 'N', NoAutoRename )
#define DNSSDFlagsOption_PathEvalDone()				DNSSDFlagOption( 'P', PathEvaluationDone )
#define DNSSDFlagsOption_ReturnIntermediates()		DNSSDFlagOption( 'I', ReturnIntermediates )
#define DNSSDFlagsOption_Shared()					DNSSDFlagOption( 'S', Shared )
#define DNSSDFlagsOption_SuppressUnusable()			DNSSDFlagOption( 'S', SuppressUnusable )
#define DNSSDFlagsOption_Timeout()					DNSSDFlagOption( 'T', Timeout )
#define DNSSDFlagsOption_UnicastResponse()			DNSSDFlagOption( 'U', UnicastResponse )
#define DNSSDFlagsOption_Unique()					DNSSDFlagOption( 'U', Unique )
#define DNSSDFlagsOption_WakeOnResolve()			DNSSDFlagOption( 'W', WakeOnResolve )

// Interface option

static const char *		gInterface = NULL;

#define InterfaceOption()										\
	StringOption( 'i', "interface", &gInterface, "interface",	\
		"Network interface by name or index. Use index -1 for local-only.", false )

// Connection options

#define kConnectionArg_Normal			""
#define kConnectionArgPrefix_PID		"pid:"
#define kConnectionArgPrefix_UUID		"uuid:"

static const char *		gConnectionOpt = kConnectionArg_Normal;

#define ConnectionOptions()																						\
	{ kCLIOptionType_String, 0, "connection", &gConnectionOpt, NULL, (intptr_t) kConnectionArg_Normal, "type",	\
		kCLIOptionFlags_OptionalArgument, NULL, NULL, NULL, NULL,												\
		"Specifies the type of main connection to use. See " kConnectionSection_Name " below.", NULL }

#define kConnectionSection_Name		"Connection Option"
#define kConnectionSection_Text																							\
	"The default behavior is to create a main connection with DNSServiceCreateConnection() and perform operations on\n"	\
	"the main connection using the kDNSServiceFlagsShareConnection flag. This behavior can be explicitly invoked by\n"	\
	"specifying the connection option without an argument, i.e.,\n"														\
	"\n"																												\
	"    --connection\n"																								\
	"\n"																												\
	"To instead use a delegate connection created with DNSServiceCreateDelegateConnection(), use\n"						\
	"\n"																												\
	"    --connection=pid:<PID>\n"																						\
	"\n"																												\
	"to specify the delegator by PID, or use\n"																			\
	"\n"																												\
	"    --connection=uuid:<UUID>\n"																					\
	"\n"																												\
	"to specify the delegator by UUID.\n"																				\
	"\n"																												\
	"To not use a main connection at all, but instead perform operations on their own implicit connections, use\n"		\
	"\n"																												\
	"    --no-connection\n"

#define ConnectionSection()		CLI_SECTION( kConnectionSection_Name, kConnectionSection_Text )

// Help text for record data options

#define kRDataArgPrefix_Domain			"domain:"
#define kRDataArgPrefix_File			"file:"
#define kRDataArgPrefix_HexString		"hex:"
#define kRDataArgPrefix_IPv4			"ipv4:"
#define kRDataArgPrefix_IPv6			"ipv6:"
#define kRDataArgPrefix_SRV				"srv:"
#define kRDataArgPrefix_String			"string:"
#define kRDataArgPrefix_TXT				"txt:"

#define kRecordDataSection_Name		"Record Data Arguments"
#define kRecordDataSection_Text																							\
	"A record data argument is specified in one of the following formats:\n"											\
	"\n"																												\
	"Format                       Syntax                                   Example\n"									\
	"Domain name                  domain:<domain name>                     domain:demo._test._tcp.local\n"				\
	"File containing record data  file:<file path>                         file:/path/to/binary-rdata-file\n"			\
	"Hexadecimal string           hex:<hex string>                         hex:c0000201 or hex:'C0 00 02 01'\n"		    \
	"IPv4 address                 ipv4:<IPv4 address>                      ipv4:192.0.2.1\n"							\
	"IPv6 address                 ipv6:<IPv6 address>                      ipv6:2001:db8::1\n"							\
	"SRV record                   srv:<priority>,<weight>,<port>,<target>  srv:0,0,64206,example.local\n"				\
	"String                       string:<string>                          string:'\\x09color=red'\n"					\
	"TXT record strings           txt:<comma-delimited strings>            txt:'vers=1.0,lang=en\\,es\\,fr,passreq'\n"	\
	"\n"																												\
	"Note: The string format converts each \\xHH escape sequence into the octet represented by the HH hex digit pair.\n"

#define RecordDataSection()		CLI_SECTION( kRecordDataSection_Name, kRecordDataSection_Text )

//===========================================================================================================================
//	Output Formatting
//===========================================================================================================================

#define kOutputFormatStr_JSON		"json"
#define kOutputFormatStr_XML		"xml"
#define kOutputFormatStr_Binary		"binary"

typedef enum
{
	kOutputFormatType_Invalid	= 0,
	kOutputFormatType_JSON		= 1,
	kOutputFormatType_XML		= 2,
	kOutputFormatType_Binary	= 3
	
}	OutputFormatType;

#define FormatOption( SHORT_CHAR, LONG_NAME, VAL_PTR, SHORT_HELP, IS_REQUIRED )			\
	StringOptionEx( SHORT_CHAR, LONG_NAME, VAL_PTR, "format", SHORT_HELP, IS_REQUIRED,	\
		"\n"																			\
		"Use '" kOutputFormatStr_JSON   "' for JavaScript Object Notation (JSON).\n"	\
		"Use '" kOutputFormatStr_XML    "' for property list XML version 1.0.\n"		\
		"Use '" kOutputFormatStr_Binary "' for property list binary version 1.0.\n"		\
		"\n"																			\
	)

//===========================================================================================================================
//	Browse Command Options
//===========================================================================================================================

static char **			gBrowse_ServiceTypes		= NULL;
static size_t			gBrowse_ServiceTypesCount	= 0;
static const char *		gBrowse_Domain				= NULL;
static int				gBrowse_DoResolve			= false;
static int				gBrowse_QueryTXT			= false;
static int				gBrowse_TimeLimitSecs		= 0;

static CLIOption		kBrowseOpts[] =
{
	InterfaceOption(),
	MultiStringOption(	't', "type",	&gBrowse_ServiceTypes, &gBrowse_ServiceTypesCount, "service type", "Service type(s), e.g., \"_ssh._tcp\".", true ),
	StringOption(		'd', "domain",	&gBrowse_Domain, "domain", "Domain in which to browse for the service type(s).", false ),
	
	CLI_OPTION_GROUP( "Flags" ),
	DNSSDFlagsOption(),
	DNSSDFlagsOption_IncludeAWDL(),
	
	CLI_OPTION_GROUP( "Operation" ),
	ConnectionOptions(),
	BooleanOption(  0 , "resolve",		&gBrowse_DoResolve,		"Resolve service instances." ),
	BooleanOption(  0 , "queryTXT",		&gBrowse_QueryTXT,		"Query TXT records of service instances." ),
	IntegerOption( 'l', "timeLimit",	&gBrowse_TimeLimitSecs,	"seconds", "Specifies the max duration of the browse operation. Use '0' for no time limit.", false ),
	
	ConnectionSection(),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	GetAddrInfo Command Options
//===========================================================================================================================

static const char *		gGetAddrInfo_Name			= NULL;
static int				gGetAddrInfo_ProtocolIPv4	= false;
static int				gGetAddrInfo_ProtocolIPv6	= false;
static int				gGetAddrInfo_OneShot		= false;
static int				gGetAddrInfo_TimeLimitSecs	= 0;

static CLIOption		kGetAddrInfoOpts[] =
{
	InterfaceOption(),
	StringOption(  'n', "name", &gGetAddrInfo_Name,			"domain name", "Domain name to resolve.", true ),
	BooleanOption(  0 , "ipv4", &gGetAddrInfo_ProtocolIPv4,	"Use kDNSServiceProtocol_IPv4." ),
	BooleanOption(  0 , "ipv6", &gGetAddrInfo_ProtocolIPv6,	"Use kDNSServiceProtocol_IPv6." ),
	
	CLI_OPTION_GROUP( "Flags" ),
	DNSSDFlagsOption(),
	DNSSDFlagsOption_AllowExpiredAnswers(),
	DNSSDFlagsOption_DenyCellular(),
	DNSSDFlagsOption_DenyConstrained(),
	DNSSDFlagsOption_DenyExpensive(),
	DNSSDFlagsOption_PathEvalDone(),
	DNSSDFlagsOption_ReturnIntermediates(),
	DNSSDFlagsOption_SuppressUnusable(),
	DNSSDFlagsOption_Timeout(),
	
	CLI_OPTION_GROUP( "Operation" ),
	ConnectionOptions(),
	BooleanOption( 'o', "oneshot",		&gGetAddrInfo_OneShot,			"Finish after first set of results." ),
	IntegerOption( 'l', "timeLimit",	&gGetAddrInfo_TimeLimitSecs,	"seconds", "Maximum duration of the GetAddrInfo operation. Use '0' for no time limit.", false ),
	
	ConnectionSection(),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	QueryRecord Command Options
//===========================================================================================================================

static const char *		gQueryRecord_Name			= NULL;
static const char *		gQueryRecord_Type			= NULL;
static int				gQueryRecord_OneShot		= false;
static int				gQueryRecord_TimeLimitSecs	= 0;
static int				gQueryRecord_RawRData		= false;

static CLIOption		kQueryRecordOpts[] =
{
	InterfaceOption(),
	StringOption( 'n', "name", &gQueryRecord_Name, "domain name", "Full domain name of record to query.", true ),
	StringOption( 't', "type", &gQueryRecord_Type, "record type", "Record type by name (e.g., TXT, SRV, etc.) or number.", true ),
	
	CLI_OPTION_GROUP( "Flags" ),
	DNSSDFlagsOption(),
	DNSSDFlagsOption_AllowExpiredAnswers(),
	DNSSDFlagsOption_DenyCellular(),
	DNSSDFlagsOption_DenyConstrained(),
	DNSSDFlagsOption_DenyExpensive(),
	DNSSDFlagsOption_ForceMulticast(),
	DNSSDFlagsOption_IncludeAWDL(),
	DNSSDFlagsOption_PathEvalDone(),
	DNSSDFlagsOption_ReturnIntermediates(),
	DNSSDFlagsOption_SuppressUnusable(),
	DNSSDFlagsOption_Timeout(),
	DNSSDFlagsOption_UnicastResponse(),
	
	CLI_OPTION_GROUP( "Operation" ),
	ConnectionOptions(),
	BooleanOption( 'o', "oneshot",		&gQueryRecord_OneShot,			"Finish after first set of results." ),
	IntegerOption( 'l', "timeLimit",	&gQueryRecord_TimeLimitSecs,	"seconds", "Maximum duration of the query record operation. Use '0' for no time limit.", false ),
	BooleanOption(  0 , "raw",			&gQueryRecord_RawRData,			"Show record data as a hexdump." ),
	
	ConnectionSection(),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	Register Command Options
//===========================================================================================================================

static const char *			gRegister_Name			= NULL;
static const char *			gRegister_Type			= NULL;
static const char *			gRegister_Domain		= NULL;
static int					gRegister_Port			= 0;
static const char *			gRegister_TXT			= NULL;
static int					gRegister_LifetimeMs	= -1;
static const char **		gAddRecord_Types		= NULL;
static size_t				gAddRecord_TypesCount	= 0;
static const char **		gAddRecord_Data			= NULL;
static size_t				gAddRecord_DataCount	= 0;
static const char **		gAddRecord_TTLs			= NULL;
static size_t				gAddRecord_TTLsCount	= 0;
static const char *			gUpdateRecord_Data		= NULL;
static int					gUpdateRecord_DelayMs	= 0;
static int					gUpdateRecord_TTL		= 0;

static CLIOption		kRegisterOpts[] =
{
	InterfaceOption(),
	StringOption(  'n', "name",		&gRegister_Name,	"service name",	"Name of service.", false ),
	StringOption(  't', "type",		&gRegister_Type,	"service type",	"Service type, e.g., \"_ssh._tcp\".", true ),
	StringOption(  'd', "domain",	&gRegister_Domain,	"domain",		"Domain in which to advertise the service.", false ),
	IntegerOption( 'p', "port",		&gRegister_Port,	"port number",	"Service's port number.", true ),
	StringOption(   0 , "txt",		&gRegister_TXT,		"record data",	"The TXT record data. See " kRecordDataSection_Name " below.", false ),
	
	CLI_OPTION_GROUP( "Flags" ),
	DNSSDFlagsOption(),
	DNSSDFlagsOption_IncludeAWDL(),
	DNSSDFlagsOption_KnownUnique(),
	DNSSDFlagsOption_NoAutoRename(),
	
	CLI_OPTION_GROUP( "Operation" ),
	IntegerOption( 'l', "lifetime", &gRegister_LifetimeMs, "ms", "Lifetime of the service registration in milliseconds.", false ),
	
	CLI_OPTION_GROUP( "Options for updating the registered service's primary TXT record with DNSServiceUpdateRecord()\n" ),
	StringOption(  0 , "updateData",	&gUpdateRecord_Data,	"record data",	"Record data for the record update. See " kRecordDataSection_Name " below.", false ),
	IntegerOption( 0 , "updateDelay",	&gUpdateRecord_DelayMs,	"ms",			"Number of milliseconds after registration to wait before record update.", false ),
	IntegerOption( 0 , "updateTTL",		&gUpdateRecord_TTL,		"seconds",		"Time-to-live of the updated record.", false ),
	
	CLI_OPTION_GROUP( "Options for adding extra record(s) to the registered service with DNSServiceAddRecord()\n" ),
	MultiStringOption(   0 , "addType",	&gAddRecord_Types,	&gAddRecord_TypesCount,	"record type",	"Type of additional record by name (e.g., TXT, SRV, etc.) or number.", false ),
	MultiStringOptionEx( 0 , "addData",	&gAddRecord_Data,	&gAddRecord_DataCount,	"record data",	"Additional record's data. See " kRecordDataSection_Name " below.", false, NULL ),
	MultiStringOption(   0 , "addTTL",	&gAddRecord_TTLs,	&gAddRecord_TTLsCount,	"seconds",		"Time-to-live of additional record in seconds. Use '0' for default.", false ),
	
	RecordDataSection(),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	RegisterRecord Command Options
//===========================================================================================================================

static const char *		gRegisterRecord_Name			= NULL;
static const char *		gRegisterRecord_Type			= NULL;
static const char *		gRegisterRecord_Data			= NULL;
static int				gRegisterRecord_TTL				= 0;
static int				gRegisterRecord_LifetimeMs		= -1;
static const char *		gRegisterRecord_UpdateData		= NULL;
static int				gRegisterRecord_UpdateDelayMs	= 0;
static int				gRegisterRecord_UpdateTTL		= 0;

static CLIOption		kRegisterRecordOpts[] =
{
	InterfaceOption(),
	StringOption( 'n', "name",	&gRegisterRecord_Name,	"record name",	"Fully qualified domain name of record.", true ),
	StringOption( 't', "type",	&gRegisterRecord_Type,	"record type",	"Record type by name (e.g., TXT, PTR, A) or number.", true ),
	StringOption( 'd', "data",	&gRegisterRecord_Data,	"record data",	"The record data. See " kRecordDataSection_Name " below.", false ),
	IntegerOption( 0 , "ttl",	&gRegisterRecord_TTL,	"seconds",		"Time-to-live in seconds. Use '0' for default.", false ),
	
	CLI_OPTION_GROUP( "Flags" ),
	DNSSDFlagsOption(),
	DNSSDFlagsOption_IncludeAWDL(),
	DNSSDFlagsOption_KnownUnique(),
	DNSSDFlagsOption_Shared(),
	DNSSDFlagsOption_Unique(),
	
	CLI_OPTION_GROUP( "Operation" ),
	IntegerOption( 'l', "lifetime", &gRegisterRecord_LifetimeMs, "ms", "Lifetime of the service registration in milliseconds.", false ),
	
	CLI_OPTION_GROUP( "Options for updating the registered record with DNSServiceUpdateRecord()\n" ),
	StringOption(  0 , "updateData",	&gRegisterRecord_UpdateData,	"record data",	"Record data for the record update.", false ),
	IntegerOption( 0 , "updateDelay",	&gRegisterRecord_UpdateDelayMs,	"ms",			"Number of milliseconds after registration to wait before record update.", false ),
	IntegerOption( 0 , "updateTTL",		&gRegisterRecord_UpdateTTL,		"seconds",		"Time-to-live of the updated record.", false ),
	
	RecordDataSection(),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	Resolve Command Options
//===========================================================================================================================

static char *		gResolve_Name			= NULL;
static char *		gResolve_Type			= NULL;
static char *		gResolve_Domain			= NULL;
static int			gResolve_TimeLimitSecs	= 0;

static CLIOption		kResolveOpts[] =
{
	InterfaceOption(),
	StringOption( 'n', "name",		&gResolve_Name,		"service name", "Name of the service instance to resolve.", true ),
	StringOption( 't', "type",		&gResolve_Type,		"service type", "Type of the service instance to resolve.", true ),
	StringOption( 'd', "domain",	&gResolve_Domain,	"domain", "Domain of the service instance to resolve.", true ),
	
	CLI_OPTION_GROUP( "Flags" ),
	DNSSDFlagsOption(),
	DNSSDFlagsOption_ForceMulticast(),
	DNSSDFlagsOption_IncludeAWDL(),
	DNSSDFlagsOption_ReturnIntermediates(),
	DNSSDFlagsOption_WakeOnResolve(),
	
	CLI_OPTION_GROUP( "Operation" ),
	ConnectionOptions(),
	IntegerOption( 'l', "timeLimit", &gResolve_TimeLimitSecs, "seconds", "Maximum duration of the resolve operation. Use '0' for no time limit.", false ),
	
	ConnectionSection(),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	Reconfirm Command Options
//===========================================================================================================================

static const char *		gReconfirmRecord_Name	= NULL;
static const char *		gReconfirmRecord_Type	= NULL;
static const char *		gReconfirmRecord_Class	= NULL;
static const char *		gReconfirmRecord_Data	= NULL;

static CLIOption		kReconfirmOpts[] =
{
	InterfaceOption(),
	StringOption( 'n', "name",	&gReconfirmRecord_Name,		"record name",	"Full name of the record to reconfirm.", true ),
	StringOption( 't', "type",	&gReconfirmRecord_Type,		"record type",	"Type of the record to reconfirm.", true ),
	StringOption( 'c', "class",	&gReconfirmRecord_Class,	"record class",	"Class of the record to reconfirm. Default class is IN.", false ),
	StringOption( 'd', "data",	&gReconfirmRecord_Data,		"record data",	"The record data. See " kRecordDataSection_Name " below.", false ),
	
	CLI_OPTION_GROUP( "Flags" ),
	DNSSDFlagsOption(),
	
	RecordDataSection(),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	getaddrinfo-POSIX Command Options
//===========================================================================================================================

static const char *		gGAIPOSIX_HostName			= NULL;
static const char *		gGAIPOSIX_ServName			= NULL;
static const char *		gGAIPOSIX_Family			= NULL;
static int				gGAIPOSIXFlag_AddrConfig	= false;
static int				gGAIPOSIXFlag_All			= false;
static int				gGAIPOSIXFlag_CanonName		= false;
static int				gGAIPOSIXFlag_NumericHost	= false;
static int				gGAIPOSIXFlag_NumericServ	= false;
static int				gGAIPOSIXFlag_Passive		= false;
static int				gGAIPOSIXFlag_V4Mapped		= false;
#if( defined( AI_V4MAPPED_CFG ) )
static int				gGAIPOSIXFlag_V4MappedCFG	= false;
#endif
#if( defined( AI_DEFAULT ) )
static int				gGAIPOSIXFlag_Default		= false;
#endif
#if( defined( AI_UNUSABLE ) )
static int				gGAIPOSIXFlag_Unusable		= false;
#endif

static CLIOption		kGetAddrInfoPOSIXOpts[] =
{
	StringOption(	'n', "hostname",			&gGAIPOSIX_HostName,		"hostname", "Domain name to resolve or an IPv4 or IPv6 address.", true ),
	StringOption(	's', "servname",			&gGAIPOSIX_ServName,		"servname", "Port number in decimal or service name from services(5).", false ),
	
	CLI_OPTION_GROUP( "Hints" ),
	StringOptionEx(	'f', "family",				&gGAIPOSIX_Family,			"address family", "Address family to use for hints ai_family field.", false,
		"\n"
		"Possible address family values are 'inet' for AF_INET, 'inet6' for AF_INET6, or 'unspec' for AF_UNSPEC. If no\n"
		"address family is specified, then AF_UNSPEC is used.\n"
		"\n" ),
	BooleanOption(   0 , "flag-addrconfig",		&gGAIPOSIXFlag_AddrConfig,	"In hints ai_flags field, set AI_ADDRCONFIG." ),
	BooleanOption(   0 , "flag-all",			&gGAIPOSIXFlag_All,			"In hints ai_flags field, set AI_ALL." ),
	BooleanOption(   0 , "flag-canonname",		&gGAIPOSIXFlag_CanonName,	"In hints ai_flags field, set AI_CANONNAME." ),
	BooleanOption(   0 , "flag-numerichost",	&gGAIPOSIXFlag_NumericHost,	"In hints ai_flags field, set AI_NUMERICHOST." ),
	BooleanOption(   0 , "flag-numericserv",	&gGAIPOSIXFlag_NumericServ,	"In hints ai_flags field, set AI_NUMERICSERV." ),
	BooleanOption(   0 , "flag-passive",		&gGAIPOSIXFlag_Passive,		"In hints ai_flags field, set AI_PASSIVE." ),
	BooleanOption(   0 , "flag-v4mapped",		&gGAIPOSIXFlag_V4Mapped,	"In hints ai_flags field, set AI_V4MAPPED." ),
#if( defined( AI_V4MAPPED_CFG ) )
	BooleanOption(   0 , "flag-v4mappedcfg",	&gGAIPOSIXFlag_V4MappedCFG,	"In hints ai_flags field, set AI_V4MAPPED_CFG." ),
#endif
#if( defined( AI_DEFAULT ) )
	BooleanOption(   0 , "flag-default",		&gGAIPOSIXFlag_Default,		"In hints ai_flags field, set AI_DEFAULT." ),
#endif
#if( defined( AI_UNUSABLE ) )
	BooleanOption(   0 , "flag-unusable",		&gGAIPOSIXFlag_Unusable,	"In hints ai_flags field, set AI_UNUSABLE." ),
#endif
	
	CLI_SECTION( "Notes", "See getaddrinfo(3) man page for more details.\n" ),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	ReverseLookup Command Options
//===========================================================================================================================

static const char *		gReverseLookup_IPAddr			= NULL;
static int				gReverseLookup_OneShot			= false;
static int				gReverseLookup_TimeLimitSecs	= 0;

static CLIOption		kReverseLookupOpts[] =
{
	InterfaceOption(),
	StringOption( 'a', "address", &gReverseLookup_IPAddr, "IP address", "IPv4 or IPv6 address for which to perform a reverse IP lookup.", true ),
	
	CLI_OPTION_GROUP( "Flags" ),
	DNSSDFlagsOption(),
	DNSSDFlagsOption_ForceMulticast(),
	DNSSDFlagsOption_ReturnIntermediates(),
	DNSSDFlagsOption_SuppressUnusable(),
	
	CLI_OPTION_GROUP( "Operation" ),
	ConnectionOptions(),
	BooleanOption( 'o', "oneshot",		&gReverseLookup_OneShot,		"Finish after first set of results." ),
	IntegerOption( 'l', "timeLimit",	&gReverseLookup_TimeLimitSecs,	"seconds", "Specifies the max duration of the query record operation. Use '0' for no time limit.", false ),
	
	ConnectionSection(),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	PortMapping Command Options
//===========================================================================================================================

static int		gPortMapping_ProtocolTCP	= false;
static int		gPortMapping_ProtocolUDP	= false;
static int		gPortMapping_InternalPort	= 0;
static int		gPortMapping_ExternalPort	= 0;
static int		gPortMapping_TTL			= 0;

static CLIOption		kPortMappingOpts[] =
{
	InterfaceOption(),
	BooleanOption( 0, "tcp",			&gPortMapping_ProtocolTCP,	"Use kDNSServiceProtocol_TCP." ),
	BooleanOption( 0, "udp",			&gPortMapping_ProtocolUDP,	"Use kDNSServiceProtocol_UDP." ),
	IntegerOption( 0, "internalPort",	&gPortMapping_InternalPort,	"port number", "Internal port.", false ),
	IntegerOption( 0, "externalPort",	&gPortMapping_ExternalPort,	"port number", "Requested external port. Use '0' for any external port.", false ),
	IntegerOption( 0, "ttl",			&gPortMapping_TTL,			"seconds", "Requested TTL (renewal period) in seconds. Use '0' for a default value.", false ),
	
	CLI_OPTION_GROUP( "Flags" ),
	DNSSDFlagsOption(),
	
	CLI_OPTION_GROUP( "Operation" ),
	ConnectionOptions(),
	
	ConnectionSection(),
	CLI_OPTION_END()
};

#if( TARGET_OS_DARWIN )
//===========================================================================================================================
//	RegisterKA Command Options
//===========================================================================================================================

static const char *		gRegisterKA_LocalAddress	= NULL;
static const char *		gRegisterKA_RemoteAddress	= NULL;
static int				gRegisterKA_Timeout			= 0;

static CLIOption		kRegisterKA_Opts[] =
{
	DNSSDFlagsOption(),
	StringOption(  'l', "local",   &gRegisterKA_LocalAddress,  "IP addr+port", "TCP connection's local IPv4 or IPv6 address and port pair.", true ),
	StringOption(  'r', "remote",  &gRegisterKA_RemoteAddress, "IP addr+port", "TCP connection's remote IPv4 or IPv6 address and port pair.", true ),
	IntegerOption( 't', "timeout", &gRegisterKA_Timeout,       "timeout", "Keepalive record's timeout value, i.e., its 't=' value.", false ),
	CLI_OPTION_END()
};

static void	RegisterKACmd( void );
#endif

//===========================================================================================================================
//	BrowseAll Command Options
//===========================================================================================================================

static const char *		gBrowseAll_Domain				= NULL;
static const char **	gBrowseAll_ServiceTypes			= NULL;
static size_t			gBrowseAll_ServiceTypesCount	= 0;
static int				gBrowseAll_BrowseTimeSecs		= 5;
static int				gBrowseAll_ConnectTimeout		= 0;

static CLIOption		kBrowseAllOpts[] =
{
	InterfaceOption(),
	StringOption(	   'd', "domain", &gBrowseAll_Domain, "domain", "Domain in which to browse for the service.", false ),
	MultiStringOption( 't', "type",   &gBrowseAll_ServiceTypes, &gBrowseAll_ServiceTypesCount, "service type", "Service type(s), e.g., \"_ssh._tcp\". All services are browsed for if none is specified.", false ),
	
	CLI_OPTION_GROUP( "Flags" ),
	DNSSDFlagsOption_IncludeAWDL(),
	
	CLI_OPTION_GROUP( "Operation" ),
	IntegerOption( 'b', "browseTime",     &gBrowseAll_BrowseTimeSecs, "seconds", "Amount of time to spend browsing in seconds. (default: 5)", false ),
	IntegerOption( 'c', "connectTimeout", &gBrowseAll_ConnectTimeout, "seconds", "Timeout for connection attempts. If <= 0, no connections are attempted. (default: 0)", false ),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	GetNameInfo Command Options
//===========================================================================================================================

static void	GetNameInfoCmd( void );

static char *		gGetNameInfo_IPAddress			= NULL;
static int			gGetNameInfoFlag_DGram			= false;
static int			gGetNameInfoFlag_NameReqd		= false;
static int			gGetNameInfoFlag_NoFQDN			= false;
static int			gGetNameInfoFlag_NumericHost	= false;
static int			gGetNameInfoFlag_NumericScope	= false;
static int			gGetNameInfoFlag_NumericServ	= false;

static CLIOption		kGetNameInfoOpts[] =
{
	StringOption( 'a', "address",           &gGetNameInfo_IPAddress,        "IP address", "IPv4 or IPv6 address to use in sockaddr structure.", true ),
	
	CLI_OPTION_GROUP( "Flags" ),
	BooleanOption( 0 , "flag-dgram",        &gGetNameInfoFlag_DGram,        "Use NI_DGRAM flag." ),
	BooleanOption( 0 , "flag-namereqd",     &gGetNameInfoFlag_NameReqd,     "Use NI_NAMEREQD flag." ),
	BooleanOption( 0 , "flag-nofqdn",       &gGetNameInfoFlag_NoFQDN,       "Use NI_NOFQDN flag." ),
	BooleanOption( 0 , "flag-numerichost",  &gGetNameInfoFlag_NumericHost,  "Use NI_NUMERICHOST flag." ),
	BooleanOption( 0 , "flag-numericscope", &gGetNameInfoFlag_NumericScope, "Use NI_NUMERICSCOPE flag." ),
	BooleanOption( 0 , "flag-numericserv",  &gGetNameInfoFlag_NumericServ,  "Use NI_NUMERICSERV flag." ),
	
	CLI_SECTION( "Notes", "See getnameinfo(3) man page for more details.\n" ),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	GetAddrInfoStress Command Options
//===========================================================================================================================

static int		gGAIStress_TestDurationSecs	= 0;
static int		gGAIStress_ConnectionCount	= 0;
static int		gGAIStress_DurationMinMs	= 0;
static int		gGAIStress_DurationMaxMs	= 0;
static int		gGAIStress_RequestCountMax	= 0;

static CLIOption		kGetAddrInfoStressOpts[] =
{
	InterfaceOption(),
	
	CLI_OPTION_GROUP( "Flags" ),
	DNSSDFlagsOption_ReturnIntermediates(),
	DNSSDFlagsOption_SuppressUnusable(),
	
	CLI_OPTION_GROUP( "Operation" ),
	IntegerOption( 0, "testDuration",			&gGAIStress_TestDurationSecs,	"seconds",	"Stress test duration in seconds. Use '0' for forever.", false ),
	IntegerOption( 0, "connectionCount",		&gGAIStress_ConnectionCount,	"integer",	"Number of simultaneous DNS-SD connections.", true ),
	IntegerOption( 0, "requestDurationMin",		&gGAIStress_DurationMinMs,		"ms",		"Minimum duration of DNSServiceGetAddrInfo() request in milliseconds.", true ),
	IntegerOption( 0, "requestDurationMax",		&gGAIStress_DurationMaxMs,		"ms",		"Maximum duration of DNSServiceGetAddrInfo() request in milliseconds.", true ),
	IntegerOption( 0, "consecutiveRequestMax",	&gGAIStress_RequestCountMax,	"integer",	"Maximum number of requests on a connection before restarting it.", true ),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	DNSQuery Command Options
//===========================================================================================================================

static char *		gDNSQuery_Name			= NULL;
static char *		gDNSQuery_Type			= "A";
static char *		gDNSQuery_Server		= NULL;
static int			gDNSQuery_TimeLimitSecs	= 5;
static int			gDNSQuery_UseTCP		= false;
static int			gDNSQuery_Flags			= kDNSHeaderFlag_RecursionDesired;
static int			gDNSQuery_RawRData		= false;
static int			gDNSQuery_Verbose		= false;

#if( TARGET_OS_DARWIN )
	#define kDNSQueryServerOptionIsRequired		false
#else
	#define kDNSQueryServerOptionIsRequired		true
#endif

static CLIOption		kDNSQueryOpts[] =
{
	StringOption(  'n', "name",			&gDNSQuery_Name,			"name",	"Question name (QNAME) to put in DNS query message.", true ),
	StringOption(  't', "type",			&gDNSQuery_Type,			"type",	"Question type (QTYPE) to put in DNS query message. Default value is 'A'.", false ),
	StringOption(  's', "server",		&gDNSQuery_Server,			"IP address", "DNS server's IPv4 or IPv6 address.", kDNSQueryServerOptionIsRequired ),
	IntegerOption( 'l', "timeLimit",	&gDNSQuery_TimeLimitSecs,	"seconds", "Specifies query time limit. Use '-1' for no limit and '0' to exit immediately after sending.", false ),
	BooleanOption(  0 , "tcp",			&gDNSQuery_UseTCP,			"Send the DNS query via TCP instead of UDP." ),
	IntegerOption( 'f', "flags",		&gDNSQuery_Flags,			"flags", "16-bit value for DNS header flags/codes field. Default value is 0x0100 (Recursion Desired).", false ),
	BooleanOption(  0 , "raw",			&gDNSQuery_RawRData,		"Present record data as a hexdump." ),
	BooleanOption( 'v', "verbose",		&gDNSQuery_Verbose,			"Prints the DNS message to be sent to the server." ),
	CLI_OPTION_END()
};

#if( DNSSDUTIL_INCLUDE_DNSCRYPT )
//===========================================================================================================================
//	DNSCrypt Command Options
//===========================================================================================================================

static char *		gDNSCrypt_ProviderName	= NULL;
static char *		gDNSCrypt_ProviderKey	= NULL;
static char *		gDNSCrypt_Name			= NULL;
static char *		gDNSCrypt_Type			= NULL;
static char *		gDNSCrypt_Server		= NULL;
static int			gDNSCrypt_TimeLimitSecs	= 5;
static int			gDNSCrypt_RawRData		= false;
static int			gDNSCrypt_Verbose		= false;

static CLIOption		kDNSCryptOpts[] =
{
	StringOption(  'p', "providerName",	&gDNSCrypt_ProviderName,	"name", "The DNSCrypt provider name.", true ),
	StringOption(  'k', "providerKey",	&gDNSCrypt_ProviderKey,		"hex string", "The DNSCrypt provider's public signing key.", true ),
	StringOption(  'n', "name",			&gDNSCrypt_Name,			"name",	"Question name (QNAME) to put in DNS query message.", true ),
	StringOption(  't', "type",			&gDNSCrypt_Type,			"type",	"Question type (QTYPE) to put in DNS query message.", true ),
	StringOption(  's', "server",		&gDNSCrypt_Server,			"IP address", "DNS server's IPv4 or IPv6 address.", true ),
	IntegerOption( 'l', "timeLimit",	&gDNSCrypt_TimeLimitSecs,	"seconds", "Specifies query time limit. Use '-1' for no time limit and '0' to exit immediately after sending.", false ),
	BooleanOption(  0 , "raw",			&gDNSCrypt_RawRData,		"Present record data as a hexdump." ),
	BooleanOption( 'v', "verbose",		&gDNSCrypt_Verbose,			"Prints the DNS message to be sent to the server." ),
	CLI_OPTION_END()
};
#endif

//===========================================================================================================================
//	MDNSQuery Command Options
//===========================================================================================================================

static char *		gMDNSQuery_Name			= NULL;
static char *		gMDNSQuery_Type			= NULL;
static int			gMDNSQuery_SourcePort	= 0;
static int			gMDNSQuery_IsQU			= false;
static int			gMDNSQuery_RawRData		= false;
static int			gMDNSQuery_UseIPv4		= false;
static int			gMDNSQuery_UseIPv6		= false;
static int			gMDNSQuery_AllResponses	= false;
static int			gMDNSQuery_ReceiveSecs	= 1;

static CLIOption		kMDNSQueryOpts[] =
{
	StringOption(  'i', "interface",	&gInterface,				"name or index", "Network interface by name or index.", true ),
	StringOption(  'n', "name",			&gMDNSQuery_Name,			"name", "Question name (QNAME) to put in mDNS message.", true ),
	StringOption(  't', "type",			&gMDNSQuery_Type,			"type", "Question type (QTYPE) to put in mDNS message.", true ),
	IntegerOption( 'p', "sourcePort",	&gMDNSQuery_SourcePort,		"port number", "UDP source port to use when sending mDNS messages. Default is 5353 for QM questions.", false ),
	BooleanOption( 'u', "QU",			&gMDNSQuery_IsQU,			"Set the unicast-response bit, i.e., send a QU question." ),
	BooleanOption(  0 , "raw",			&gMDNSQuery_RawRData,		"Present record data as a hexdump." ),
	BooleanOption(  0 , "ipv4",			&gMDNSQuery_UseIPv4,		"Use IPv4." ),
	BooleanOption(  0 , "ipv6",			&gMDNSQuery_UseIPv6,		"Use IPv6." ),
	BooleanOption( 'a', "allResponses",	&gMDNSQuery_AllResponses,	"Print all received mDNS messages, not just those containing answers." ),
	IntegerOption( 'r', "receiveTime",	&gMDNSQuery_ReceiveSecs,	"seconds", "Amount of time to spend receiving messages after the query is sent. The default is one second. Use -1 for unlimited time.", false ),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	MDNSCollider Command Options
//===========================================================================================================================

#define kMDNSColliderProgramSection_Intro																				\
	"Programs dictate when the collider sends out unsolicited response messages for its record and how the collider\n"	\
	"ought to react to probe queries that match its record's name, if at all.\n"										\
	"\n"																												\
	"For example, suppose that the goal is to cause a specific unique record in the verified state to be renamed.\n"	\
	"The collider should be invoked such that its record's name is equal to that of the record being targeted. Also,\n"	\
	"the record's type and data should be such that no record with that name, type, and data combination currently\n"	\
	"exists. If the mDNS responder that owns the record follows sections 8.1 and 9 of RFC 6762, then the goal can be\n"	\
	"accomplished with the following program:\n"																		\
	"\n"																												\
	"    probes 3r; send; wait 5000\n"																					\
	"\n"																												\
	"The first command, 'probes 3r', tells the collider to respond to the next three probe queries that match its\n"	\
	"record's name. The second command, makes the collider send an unsolicited response message that contains its\n"	\
	"record in the answer section. The third command makes the collider wait for five seconds before exiting, which\n"	\
	"is more than enough time for the collider to respond to probe queries.\n"											\
	"\n"																												\
	"The send command will cause the targeted record to go into the probing state per section 9 since the collider's\n"	\
	"record conflicts with target record. Per the probes command, the subsequent probe query sent during the probing\n"	\
	"state will be answered by the collider, which will cause the record to be renamed per section 8.1.\n"

#define kMDNSColliderProgramSection_Probes																				\
	"The probes command defines how the collider ought to react to probe queries that match its record's name.\n"		\
	"\n"																												\
	"Usage: probes [<action-string>]\n"																					\
	"\n"																												\
	"The syntax for an action-string is\n"																				\
	"\n"																												\
	"    <action-string> ::= <action> | <action-string> \"-\" <action>\n"												\
	"    <action>        ::= [<repeat-count>] <action-code>\n"															\
	"    <repeat-count>  ::= \"1\" | \"2\" | ... | \"10\"\n"															\
	"    <action-code>   ::= \"n\" | \"r\" | \"u\" | \"m\" | \"p\"\n"													\
	"\n"																												\
	"An expanded action-string is defined as\n"																			\
	"\n"																												\
	"    <expanded-action-string> ::= <action-code> | <expanded-action-string> \"-\" <action-code>\n"					\
	"\n"																												\
	"The action-string argument is converted into an expanded-action-string by expanding each action with a\n"			\
	"repeat-count into an expanded-action-string consisting of exactly <repeat-count> <action-code>s. For example,\n"	\
	"2n-r expands to n-n-r. Action-strings that expand to expanded-action-strings with more than 10 action-codes\n"		\
	"are not allowed.\n"																								\
	"\n"																												\
	"When the probes command is executed, it does two things. Firstly, it resets to zero the collider's count of\n"		\
	"probe queries that match its record's name. Secondly, it defines how the collider ought to react to such probe\n"	\
	"queries based on the action-string argument. Specifically, the nth action-code in the expanded version of the\n"	\
	"action-string argument defines how the collider ought to react to the nth received probe query:\n"					\
	"\n"																												\
	"    Code  Action\n"																								\
	"    ----  ------\n"																								\
	"    n     Do nothing.\n"																							\
	"    r     Respond to the probe query.\n"																			\
	"    u     Respond to the probe query via unicast.\n"																\
	"    m     Respond to the probe query via multicast.\n"																\
	"    p     Multicast own probe query. (Useful for causing simultaneous probe scenarios.)\n"							\
	"\n"																												\
	"Note: If no action is defined for a received probe query, then the collider does nothing, i.e., it doesn't send\n"	\
	"a response nor does it multicast its own probe query.\n"

#define kMDNSColliderProgramSection_Send																				\
	"The send command multicasts an unsolicited mDNS response containing the collider's record in the answer\n"			\
	"section, which can be used to force unique records with the same record name into the probing state.\n"			\
	"\n"																												\
	"Usage: send\n"

#define kMDNSColliderProgramSection_Wait																				\
	"The wait command pauses program execution for the interval of time specified by its argument.\n"					\
	"\n"																												\
	"Usage: wait <milliseconds>\n"

#define kMDNSColliderProgramSection_Loop																				\
	"The loop command starts a counting loop. The done statement marks the end of the loop body. The loop command's\n"	\
	"argument specifies the number of loop iterations. Note: Loop nesting is supported up to a depth of 16.\n"			\
	"\n"																												\
	"Usage: loop <non-zero count>; ... ; done\n"																		\
	"\n"																												\
	"For example, the following program sends three unsolicited responses at an approximate rate of one per second:\n"	\
	"\n"																												\
	"    loop 3; wait 1000; send; done"

#define ConnectionSection()		CLI_SECTION( kConnectionSection_Name, kConnectionSection_Text )

static const char *		gMDNSCollider_Name			= NULL;
static const char *		gMDNSCollider_Type			= NULL;
static const char *		gMDNSCollider_RecordData	= NULL;
static int				gMDNSCollider_UseIPv4		= false;
static int				gMDNSCollider_UseIPv6		= false;
static const char *		gMDNSCollider_Program		= NULL;

static CLIOption		kMDNSColliderOpts[] =
{
	StringOption(  'i', "interface", &gInterface,               "name or index", "Network interface by name or index.", true ),
	StringOption(  'n', "name",      &gMDNSCollider_Name,       "name", "Collider's record name.", true ),
	StringOption(  't', "type",      &gMDNSCollider_Type,       "type", "Collider's record type.", true ),
	StringOption(  'd', "data",      &gMDNSCollider_RecordData, "record data", "Collider's record data. See " kRecordDataSection_Name " below.", true ),
	StringOption(  'p', "program",   &gMDNSCollider_Program,    "program", "Program to execute. See Program section below.", true ),
	BooleanOption(  0 , "ipv4",      &gMDNSCollider_UseIPv4,    "Use IPv4." ),
	BooleanOption(  0 , "ipv6",      &gMDNSCollider_UseIPv6,    "Use IPv6." ),
	
	RecordDataSection(),
	CLI_SECTION( "Program",					kMDNSColliderProgramSection_Intro ),
	CLI_SECTION( "Program Command: probes",	kMDNSColliderProgramSection_Probes ),
	CLI_SECTION( "Program Command: send",	kMDNSColliderProgramSection_Send ),
	CLI_SECTION( "Program Command: wait",	kMDNSColliderProgramSection_Wait ),
	CLI_SECTION( "Program Command: loop",	kMDNSColliderProgramSection_Loop ),
	CLI_OPTION_END()
};

static void	MDNSColliderCmd( void );

//===========================================================================================================================
//	PIDToUUID Command Options
//===========================================================================================================================

static int		gPIDToUUID_PID = 0;

static CLIOption		kPIDToUUIDOpts[] =
{
	IntegerOption( 'p', "pid", &gPIDToUUID_PID, "PID", "Process ID.", true ),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	DNSServer Command Options
//===========================================================================================================================

#define kDNSServerInfoText_Intro																						\
	"The DNS server answers certain queries in the d.test. domain. Responses are dynamically generated based on the\n"	\
	"presence of special labels in the query's QNAME. There are currently eight types of special labels that can be\n"	\
	"used to generate specific responses: Alias labels, Alias-TTL labels, Count labels, Tag labels, TTL labels, the\n"	\
	"IPv4 label, the IPv6 label, and SRV labels.\n"																		\
	"\n"																												\
	"Note: Sub-strings representing integers in domain name labels are in decimal notation and without leading zeros.\n"

#define kDNSServerInfoText_NameExistence																				\
	"A name is considered to exist if it's an Address name or an SRV name.\n"											\
	"\n"																												\
	"An Address name is defined as a name that ends with d.test., and the other labels, if any, and in no particular\n"	\
	"order, unless otherwise noted, consist of\n"																		\
	"\n"																												\
	"    1. at most one Alias or Alias-TTL label as the first label;\n"													\
	"    2. at most one Count label;\n"																					\
	"    3. zero or more Tag labels;\n"																					\
	"    4. at most one TTL label; and\n"																				\
	"    5. at most one IPv4 or IPv6 label.\n"																			\
	"\n"																												\
	"An SRV name is defined as a name with the following form:\n"														\
	"\n"																												\
	" _<service>._<proto>[.<parent domain>][.<SRV label 1>[.<target 1>][.<SRV label 2>[.<target 2>][...]]].d.test.\n"	\
	"\n"																												\
	"See \"SRV Names\" for details.\n"

#define kDNSServerInfoText_ResourceRecords																				\
	"Currently, the server only supports CNAME, A, AAAA, and SRV records.\n"											\
	"\n"																												\
	"Address names that begin with an Alias or Alias-TTL label are aliases of canonical names, i.e., they're the\n"		\
	"names of CNAME records. See \"Alias Labels\" and \"Alias-TTL Labels\" for details.\n"								\
	"\n"																												\
	"A canonical Address name can exclusively be the name of one or more A records, can exclusively be the name or\n"	\
	"one or more AAAA records, or can be the name of both A and AAAA records. Address names that contain an IPv4\n"		\
	"label have at least one A record, but no AAAA records. Address names that contain an IPv6 label, have at least\n"	\
	"one AAAA record, but no A records. All other Address names have at least one A record and at least one AAAA\n"		\
	"record. See \"Count Labels\" for how the number of address records for a given Address name is determined.\n"		\
	"\n"																												\
	"A records contain IPv4 addresses in the 203.0.113.0/24 block, while AAAA records contain IPv6 addresses in the\n"	\
	"2001:db8:1::/120 block. Both of these address blocks are reserved for documentation. See\n"						\
	"<https://tools.ietf.org/html/rfc5737> and <https://tools.ietf.org/html/rfc3849>.\n"								\
	"\n"																												\
	"SRV names are names of SRV records.\n"																				\
	"\n"																												\
	"Unless otherwise specified, all resource records will use a default TTL. The default TTL can be set with the\n"	\
	"--defaultTTL option. See \"Alias-TTL Labels\" and \"TTL Labels\" for details on how to query for CNAME, A, and\n"	\
	"AAAA records with specific TTL values.\n"

#define kDNSServerInfoText_AliasLabel																					\
	"Alias labels are of the form \"alias\" or \"alias-N\", where N is an integer in [2, 2^31 - 1].\n"					\
	"\n"																												\
	"If QNAME is an Address name and its first label is Alias label \"alias-N\", then the response will contain\n"		\
	"exactly N CNAME records:\n"																						\
	"\n"																												\
	"    1. For each i in [3, N], the response will contain a CNAME record whose name is identical to QNAME, except\n"	\
	"       that the first label is \"alias-i\" instead, and whose RDATA is the name of the other CNAME record whose\n"	\
	"       name has \"alias-(i - 1)\" as its first label.\n"															\
	"\n"																												\
	"    2. The response will contain a CNAME record whose name is identical to QNAME, except that the first label\n"	\
	"       is \"alias-2\" instead, and whose RDATA is the name identical to QNAME, except that the first label is\n"	\
	"       \"alias\" instead.\n"																						\
	"\n"																												\
	"    3. The response will contain a CNAME record whose name is identical to QNAME, except that the first label\n"	\
	"       is \"alias\" instead, and whose RDATA is the name identical to QNAME minus its first label.\n"				\
	"\n"																												\
	"If QNAME is an Address name and its first label is Alias label \"alias\", then the response will contain a\n"		\
	"single CNAME record. The CNAME record's name will be equal to QNAME and its RDATA will be the name identical to\n"	\
	"QNAME minus its first label.\n"																					\
	"\n"																												\
	"Example. A response to a query with a QNAME of alias-3.count-5.d.test will contain the following CNAME\n"			\
	"records:\n"																										\
	"\n"																												\
	"    alias-4.count-5.d.test.                        60    IN CNAME alias-3.count-5.d.test.\n"						\
	"    alias-3.count-5.d.test.                        60    IN CNAME alias-2.count-5.d.test.\n"						\
	"    alias-2.count-5.d.test.                        60    IN CNAME alias.count-5.d.test.\n"							\
	"    alias.count-5.d.test.                          60    IN CNAME count-5.d.test.\n"

#define kDNSServerInfoText_AliasTTLLabel																				\
	"Alias-TTL labels are of the form \"alias-ttl-T_1[-T_2[...-T_N]]\", where each T_i is an integer in\n"				\
	"[0, 2^31 - 1] and N is a positive integer bounded by the size of the maximum legal label length (63 octets).\n"	\
	"\n"																												\
	"If QNAME is an Address name and its first label is Alias-TTL label \"alias-ttl-T_1...-T_N\", then the response\n"	\
	"will contain exactly N CNAME records:\n"																			\
	"\n"																												\
	"    1. For each i in [1, N - 1], the response will contain a CNAME record whose name is identical to QNAME,\n"		\
	"       except that the first label is \"alias-ttl-T_i...-T_N\" instead, whose TTL value is T_i, and whose RDATA\n"	\
	"       is the name of the other CNAME record whose name has \"alias-ttl-T_(i+1)...-T_N\" as its first label.\n"	\
	"\n"																												\
	"    2. The response will contain a CNAME record whose name is identical to QNAME, except that the first label\n"	\
	"       is \"alias-ttl-T_N\", whose TTL is T_N, and whose RDATA is identical to QNAME stripped of its first\n"		\
	"       label.\n"																									\
	"\n"																												\
	"Example. A response to a query with a QNAME of alias-ttl-20-40-80.count-5.d.test will contain the following\n"		\
	"CNAME records:\n"																									\
	"\n"																												\
	"    alias-ttl-20-40-80.count-5.d.test.             20    IN CNAME alias-ttl-40-80.count-5.d.test.\n"				\
	"    alias-ttl-40-80.count-5.d.test.                40    IN CNAME alias-ttl-80.count-5.d.test.\n"					\
	"    alias-ttl-80.count-5.d.test.                   80    IN CNAME count-5.d.test.\n"

#define kDNSServerInfoText_CountLabel																					\
	"Count labels are of the form \"count-N_1\" or \"count-N_1-N_2\", where N_1 is an integer in [1, 255] and N_2 is\n"	\
	"an integer in [N_1, 255].\n"																						\
	"\n"																												\
	"If QNAME is an Address name, contains Count label \"count-N\", and has the type of address records specified by\n"	\
	"QTYPE, then the response will contain exactly N address records:\n"												\
	"\n"																												\
	"    1. For i in [1, N], the response will contain an address record of type QTYPE whose name is equal to QNAME\n"	\
	"       and whose RDATA is an address equal to a constant base address + i.\n"										\
	"\n"																												\
	"    2. The address records will be ordered by the address contained in RDATA in ascending order.\n"				\
	"\n"																												\
	"Example. A response to an A record query with a QNAME of alias.count-3.d.test will contain the following A\n"		\
	"records:\n"																										\
	"\n"																												\
	"    count-3.d.test.                                60    IN A     203.0.113.1\n"									\
	"    count-3.d.test.                                60    IN A     203.0.113.2\n"									\
	"    count-3.d.test.                                60    IN A     203.0.113.3\n"									\
	"\n"																												\
	"If QNAME is an Address name, contains Count label \"count-N_1-N_2\", and has the type of address records\n"		\
	"specified by QTYPE, then the response will contain exactly N_1 address records:\n"									\
	"\n"																												\
	"    1. Each of the address records will be of type QTYPE, have name equal to QNAME, and have as its RDATA a\n"		\
	"       unique address equal to a constant base address + i, where i is a randomly chosen integer in [1, N_2].\n"	\
	"\n"																												\
	"    2. The order of the address records will be random.\n"															\
	"\n"																												\
	"Example. A response to a AAAA record query with a QNAME of count-3-100.ttl-20.d.test could contain the\n"			\
	"following AAAA records:\n"																							\
	"\n"																												\
	"    count-3-100.ttl-20.d.test.                     20    IN AAAA  2001:db8:1::c\n"									\
	"    count-3-100.ttl-20.d.test.                     20    IN AAAA  2001:db8:1::3a\n"								\
	"    count-3-100.ttl-20.d.test.                     20    IN AAAA  2001:db8:1::4f\n"								\
	"\n"																												\
	"If QNAME is an Address name, but doesn't have the type of address records specified by QTYPE, then the response\n"	\
	"will contain no address records, regardless of whether it contains a Count label.\n"								\
	"\n"																												\
	"Address names that don't have a Count label are treated as though they contain a count label equal to\n"			\
	"count-1\".\n"

#define kDNSServerInfoText_TagLabel																						\
	"Tag labels are labels prefixed with \"tag-\" and contain zero or more arbitrary octets after the prefix.\n"		\
	"\n"																												\
	"This type of label exists to allow testers to \"uniquify\" domain names. Tag labels can also serve as padding\n"	\
	"to increase the sizes of domain names.\n"

#define kDNSServerInfoText_TTLLabel																						\
	"TTL labels are of the form \"ttl-T\", where T is an integer in [0, 2^31 - 1].\n"									\
	"\n"																												\
	"If QNAME is an Address name and contains TTL label \"ttl-T\", then all non-CNAME records contained in the\n"		\
	"response will have a TTL value equal to T.\n"

#define kDNSServerInfoText_IPv4Label \
	"The IPv4 label is \"ipv4\". See \"Resource Records\" for the affect of this label.\n"

#define kDNSServerInfoText_IPv6Label \
	"The IPv6 label is \"ipv6\". See \"Resource Records\" for the affect of this label.\n"

#define kDNSServerInfoText_SRVNames																						\
	"SRV labels are of the form \"srv-R-W-P\", where R, W, and P are integers in [0, 2^16 - 1].\n"						\
	"\n"																												\
	"After the first two labels, i.e., the service and protocol labels, the sequence of labels, which may be empty,\n"	\
	"leading up to the the first SRV label, if one exists, or the d.test. labels will be used as a parent domain for\n"	\
	"the target hostname of each of the SRV name's SRV records.\n"														\
	"\n"																												\
	"If QNAME is an SRV name and QTYPE is SRV, then for each SRV label, the response will contain an SRV record with\n"	\
	"priority R, weight W, port P, and target hostname <target>[.<parent domain>]., where <target> is the sequence\n"	\
	"of labels, which may be empty, that follows the SRV label leading up to either the next SRV label or the\n"		\
	"d.test. labels, whichever comes first.\n"																			\
	"\n"																												\
	"Example. A response to an SRV record query with a QNAME of\n"														\
	"_http._tcp.example.com.srv-0-0-80.www.srv-1-0-8080.www.d.test. will contain the following SRV records:\n"			\
	"\n"																												\
	"_http._tcp.example.com.srv-0-0-80.www.srv-1-0-8080.www.d.test.     60    IN SRV   0 0 80 www.example.com.\n"		\
	"_http._tcp.example.com.srv-0-0-80.www.srv-1-0-8080.www.d.test.     60    IN SRV   1 0 8080 www.example.com.\n"

#define kDNSServerInfoText_BadUDPMode \
	"The purpose of Bad UDP mode is to test mDNSResponder's TCP fallback mechanism by which mDNSResponder reissues a\n"	\
	"UDP query as a TCP query if the UDP response contains the expected QNAME, QTYPE, and QCLASS, but a message ID\n"	\
	"that's not equal to the query's message ID.\n"																		\
	"\n"																												\
	"This mode is identical to the normal mode except that all responses sent via UDP have a message ID equal to the\n"	\
	"query's message ID plus one. Also, in this mode, to aid in debugging, A records in responses sent via UDP have\n"	\
	"IPv4 addresses in the 0.0.0.0/24 block instead of the 203.0.113.0/24 block, i.e., 0.0.0.0 is used as the IPv4\n"	\
	"base address, and AAAA records in responses sent via UDP have IPv6 addresses in the ::ffff:0:0/120 block\n"		\
	"instead of the 2001:db8:1::/120 block, i.e., ::ffff:0:0 is used as the IPv6 base address.\n"

static int				gDNSServer_LoopbackOnly		= false;
static int				gDNSServer_Foreground		= false;
static int				gDNSServer_ResponseDelayMs	= 0;
static int				gDNSServer_DefaultTTL		= 60;
static int				gDNSServer_Port				= kDNSPort;
static const char *		gDNSServer_DomainOverride	= NULL;
#if( TARGET_OS_DARWIN )
static const char *		gDNSServer_FollowPID		= NULL;
#endif
static int				gDNSServer_BadUDPMode		= false;

static CLIOption		kDNSServerOpts[] =
{
	BooleanOption( 'l', "loopback",      &gDNSServer_LoopbackOnly,    "Bind to to the loopback interface." ),
	BooleanOption( 'f', "foreground",    &gDNSServer_Foreground,      "Direct log output to stdout instead of system logging." ),
	IntegerOption( 'd', "responseDelay", &gDNSServer_ResponseDelayMs, "ms", "The amount of additional delay in milliseconds to apply to responses. (default: 0)", false ),
	IntegerOption(  0 , "defaultTTL",    &gDNSServer_DefaultTTL,      "seconds", "Resource record TTL value to use when unspecified. (default: 60)", false ),
	IntegerOption( 'p', "port",          &gDNSServer_Port,            "port number", "UDP/TCP port number to use. Use 0 for any port. (default: 53)", false ),
	StringOption(   0 , "domain",        &gDNSServer_DomainOverride,  "domain", "Used to override 'd.test.' as the server's domain.", false ),
#if( TARGET_OS_DARWIN )
	StringOption(   0 , "follow",        &gDNSServer_FollowPID,       "pid", "Exit when the process, usually the parent process, specified by PID exits.", false ),
#endif
	BooleanOption(  0 , "badUDPMode",    &gDNSServer_BadUDPMode,      "Run in Bad UDP mode to trigger mDNSResponder's TCP fallback mechanism." ),
	
	CLI_SECTION( "Intro",				kDNSServerInfoText_Intro ),
	CLI_SECTION( "Name Existence",		kDNSServerInfoText_NameExistence ),
	CLI_SECTION( "Resource Records",	kDNSServerInfoText_ResourceRecords ),
	CLI_SECTION( "Alias Labels",		kDNSServerInfoText_AliasLabel ),
	CLI_SECTION( "Alias-TTL Labels",	kDNSServerInfoText_AliasTTLLabel ),
	CLI_SECTION( "Count Labels",		kDNSServerInfoText_CountLabel ),
	CLI_SECTION( "Tag Labels",			kDNSServerInfoText_TagLabel ),
	CLI_SECTION( "TTL Labels",			kDNSServerInfoText_TTLLabel ),
	CLI_SECTION( "IPv4 Label",			kDNSServerInfoText_IPv4Label ),
	CLI_SECTION( "IPv6 Label",			kDNSServerInfoText_IPv6Label ),
	CLI_SECTION( "SRV Names",			kDNSServerInfoText_SRVNames ),
	CLI_SECTION( "Bad UDP Mode",		kDNSServerInfoText_BadUDPMode ),
	CLI_OPTION_END()
};

static void	DNSServerCmd( void );

//===========================================================================================================================
//	MDNSReplier Command Options
//===========================================================================================================================

#define kMDNSReplierPortBase		50000

#define kMDNSReplierInfoText_Intro																						\
	"The mDNS replier answers mDNS queries for its authoritative records. These records are of class IN and of types\n"	\
	"PTR, SRV, TXT, A, and AAAA as described below.\n"																	\
	"\n"																												\
	"Note: Sub-strings representing integers in domain name labels are in decimal notation and without leading zeros.\n"

#define kMDNSReplierInfoText_Parameters																					\
	"There are five parameters that control the replier's set of authoritative records.\n"								\
	"\n"																												\
	"    1. <hostname> is the base name used for service instance names and the names of A and AAAA records. This\n"	\
	"       parameter is specified with the --hostname option.\n"														\
	"    2. <tag> is an arbitrary string used to uniquify service types. This parameter is specified with the --tag\n"	\
	"       option.\n"																									\
	"    3. N_max in an integer in [1, 65535] and limits service types to those that have no more than N_max\n"			\
	"       instances. It also limits the number of hostnames to N_max, i.e., <hostname>.local.,\n"						\
	"       <hostname>-1.local., ..., <hostname>-N_max.local. This parameter is specified with the\n"					\
	"       --maxInstanceCount option.\n"																				\
	"    4. N_a is an integer in [1, 255] and the number of A records per hostname. This parameter is specified\n"		\
	"       with the --countA option.\n"																				\
	"    5. N_aaaa is an integer in [1, 255] and the number of AAAA records per hostname. This parameter is\n"			\
	"       specified with the --countAAAA option.\n"

#define kMDNSReplierInfoText_PTR																						\
	"The replier's authoritative PTR records have names of the form _t-<tag>-<L>-<N>._tcp.local., where L is an\n"		\
	"integer in [1, 65535], and N is an integer in [1, N_max].\n"														\
	"\n"																												\
	"For a given L and N, the replier has exactly N authoritative PTR records:\n"										\
	"\n"																												\
	"    1. The first PTR record is defined as\n"																		\
	"\n"																												\
	"        NAME:  _t-<tag>-<L>-<N>._tcp.local.\n"																		\
	"        TYPE:  PTR\n"																								\
	"        CLASS: IN\n"																								\
	"        TTL:   4500\n"																								\
	"        RDATA: <hostname>._t-<tag>-<L>-<N>._tcp.local.\n"															\
	"\n"																												\
	"    2. For each i in [2, N], there is one PTR record defined as\n"													\
	"\n"																												\
	"        NAME:  _t-<tag>-<L>-<N>._tcp.local.\n"																		\
	"        TYPE:  PTR\n"																								\
	"        CLASS: IN\n"																								\
	"        TTL:   4500\n"																								\
	"        RDATA: \"<hostname> (<i>)._t-<tag>-<L>-<N>._tcp.local.\"\n"

#define kMDNSReplierInfoText_SRV																						\
	"The replier's authoritative SRV records have names of the form <instance name>._t-<tag>-<L>-<N>._tcp.local.,\n"	\
	"where L is an integer in [1, 65535], N is an integer in [1, N_max], and <instance name> is <hostname> or\n"		\
	"\"<hostname> (<i>)\", where i is in [2, N].\n"																		\
	"\n"																												\
	"For a given L and N, the replier has exactly N authoritative SRV records:\n"										\
	"\n"																												\
	"    1. The first SRV record is defined as\n"																		\
	"\n"																												\
	"        NAME:  <hostname>._t-<tag>-<L>-<N>._tcp.local.\n"															\
	"        TYPE:  SRV\n"																								\
	"        CLASS: IN\n"																								\
	"        TTL:   120\n"																								\
	"        RDATA:\n"																									\
	"            Priority: 0\n"																							\
	"            Weight:   0\n"																							\
	"            Port:     (50000 + L) mod 2^16\n"																		\
	"            Target:   <hostname>.local.\n"																			\
	"\n"																												\
	"    2. For each i in [2, N], there is one SRV record defined as:\n"												\
	"\n"																												\
	"        NAME:  \"<hostname> (<i>)._t-<tag>-<L>-<N>._tcp.local.\"\n"												\
	"        TYPE:  SRV\n"																								\
	"        CLASS: IN\n"																								\
	"        TTL:   120\n"																								\
	"        RDATA:\n"																									\
	"            Priority: 0\n"																							\
	"            Weight:   0\n"																							\
	"            Port:     (50000 + L) mod 2^16\n"																		\
	"            Target:   <hostname>-<i>.local.\n"

#define kMDNSReplierInfoText_TXT																						\
	"The replier's authoritative TXT records have names of the form <instance name>._t-<tag>-<L>-<N>._tcp.local.,\n"	\
	"where L is an integer in [1, 65535], N is an integer in [1, N_max], and <instance name> is <hostname> or\n"		\
	"\"<hostname> (<i>)\", where i is in [2, N].\n"																		\
	"\n"																												\
	"For a given L and N, the replier has exactly N authoritative TXT records:\n"										\
	"\n"																												\
	"    1. The first TXT record is defined as\n"																		\
	"\n"																												\
	"        NAME:     <hostname>._t-<tag>-<L>-<N>._tcp.local.\n"														\
	"        TYPE:     TXT\n"																							\
	"        CLASS:    IN\n"																							\
	"        TTL:      4500\n"																							\
	"        RDLENGTH: L\n"																								\
	"        RDATA:    <one or more strings with an aggregate length of L octets>\n"									\
	"\n"																												\
	"    2. For each i in [2, N], there is one TXT record:\n"															\
	"\n"																												\
	"        NAME:     \"<hostname> (<i>)._t-<tag>-<L>-<N>._tcp.local.\"\n"												\
	"        TYPE:     TXT\n"																							\
	"        CLASS:    IN\n"																							\
	"        TTL:      4500\n"																							\
	"        RDLENGTH: L\n"																								\
	"        RDATA:    <one or more strings with an aggregate length of L octets>\n"									\
	"\n"																												\
	"The RDATA of each TXT record is exactly L octets and consists of a repeating series of the 15-byte string\n"		\
	"\"hash=0x<32-bit FNV-1 hash of the record name as an 8-character hexadecimal string>\". The last instance of\n"	\
	"the string may be truncated to satisfy the TXT record data's size requirement.\n"

#define kMDNSReplierInfoText_A																							\
	"The replier has exactly N_max x N_a authoritative A records:\n"													\
	"\n"																												\
	"    1. For each j in [1, N_a], an A record is defined as\n"														\
	"\n"																												\
	"        NAME:     <hostname>.local.\n"																				\
	"        TYPE:     A\n"																								\
	"        CLASS:    IN\n"																							\
	"        TTL:      120\n"																							\
	"        RDLENGTH: 4\n"																								\
	"        RDATA:    0.0.1.<j>\n"																						\
	"\n"																												\
	"    2. For each i in [2, N_max], for each j in [1, N_a], an A record is defined as\n"								\
	"\n"																												\
	"        NAME:     <hostname>-<i>.local.\n"																			\
	"        TYPE:     A\n"																								\
	"        CLASS:    IN\n"																							\
	"        TTL:      120\n"																							\
	"        RDLENGTH: 4\n"																								\
	"        RDATA:    0.<ceil(i / 256)>.<i mod 256>.<j>\n"

#define kMDNSReplierInfoText_AAAA																						\
	"The replier has exactly N_max x N_aaaa authoritative AAAA records:\n"												\
	"\n"																												\
	"    1. For each j in [1, N_aaaa], a AAAA record is defined as\n"													\
	"\n"																												\
	"        NAME:     <hostname>.local.\n"																				\
	"        TYPE:     AAAA\n"																							\
	"        CLASS:    IN\n"																							\
	"        TTL:      120\n"																							\
	"        RDLENGTH: 16\n"																							\
	"        RDATA:    2001:db8:2::1:<j>\n"																				\
	"\n"																												\
	"    2. For each i in [2, N_max], for each j in [1, N_aaaa], a AAAA record is defined as\n"							\
	"\n"																												\
	"        NAME:     <hostname>-<i>.local.\n"																			\
	"        TYPE:     AAAA\n"																							\
	"        CLASS:    IN\n"																							\
	"        TTL:      120\n"																							\
	"        RDLENGTH: 16\n"																							\
	"        RDATA:    2001:db8:2::<i>:<j>\n"

#define kMDNSReplierInfoText_Responses																					\
	"When generating answers for a query message, any two records pertaining to the same hostname will be grouped\n"	\
	"together in the same response message, and any two records pertaining to different hostnames will be in\n"			\
	"separate response messages.\n"

static const char *		gMDNSReplier_Hostname			= NULL;
static const char *		gMDNSReplier_ServiceTypeTag		= NULL;
static int				gMDNSReplier_MaxInstanceCount	= 1000;
static int				gMDNSReplier_NoAdditionals		= false;
static int				gMDNSReplier_RecordCountA		= 1;
static int				gMDNSReplier_RecordCountAAAA	= 1;
static double			gMDNSReplier_UnicastDropRate	= 0.0;
static double			gMDNSReplier_MulticastDropRate	= 0.0;
static int				gMDNSReplier_MaxDropCount		= 0;
static int				gMDNSReplier_UseIPv4			= false;
static int				gMDNSReplier_UseIPv6			= false;
static int				gMDNSReplier_Foreground			= false;
static const char *		gMDNSReplier_FollowPID		    = NULL;

static CLIOption		kMDNSReplierOpts[] =
{
	StringOption(  'i', "interface",        &gInterface,                     "name or index", "Network interface by name or index.", true ),
	StringOption(  'n', "hostname",         &gMDNSReplier_Hostname,          "string", "Base name to use for hostnames and service instance names.", true ),
	StringOption(  't', "tag",              &gMDNSReplier_ServiceTypeTag,    "string", "Tag to use for service types, e.g., _t-<tag>-<TXT size>-<count>._tcp.", true ),
	IntegerOption( 'c', "maxInstanceCount", &gMDNSReplier_MaxInstanceCount,  "count", "Maximum number of service instances. (default: 1000)", false ),
	BooleanOption(  0 , "noAdditionals",    &gMDNSReplier_NoAdditionals,     "When answering queries, don't include any additional records." ),
	IntegerOption(  0 , "countA",           &gMDNSReplier_RecordCountA,      "count", "Number of A records per hostname. (default: 1)", false ),
	IntegerOption(  0 , "countAAAA",        &gMDNSReplier_RecordCountAAAA,   "count", "Number of AAAA records per hostname. (default: 1)", false ),
	DoubleOption(   0 , "udrop",            &gMDNSReplier_UnicastDropRate,   "probability", "Probability of dropping a unicast response. (default: 0.0)", false ),
	DoubleOption(   0 , "mdrop",            &gMDNSReplier_MulticastDropRate, "probability", "Probability of dropping a multicast query or response. (default: 0.0)", false ),
	IntegerOption(  0 , "maxDropCount",     &gMDNSReplier_MaxDropCount,      "count", "If > 0, drop probabilities are limted to first <count> responses from each instance. (default: 0)", false ),
	BooleanOption(  0 , "ipv4",             &gMDNSReplier_UseIPv4,           "Use IPv4." ),
	BooleanOption(  0 , "ipv6",             &gMDNSReplier_UseIPv6,           "Use IPv6." ),
	BooleanOption( 'f', "foreground",       &gMDNSReplier_Foreground,        "Direct log output to stdout instead of system logging." ),
#if( TARGET_OS_DARWIN )
	StringOption(   0 , "follow",           &gMDNSReplier_FollowPID,         "pid", "Exit when the process, usually the parent process, specified by PID exits.", false ),
#endif
	
	CLI_SECTION( "Intro",							kMDNSReplierInfoText_Intro ),
	CLI_SECTION( "Authoritative Record Parameters",	kMDNSReplierInfoText_Parameters ),
	CLI_SECTION( "Authoritative PTR Records",		kMDNSReplierInfoText_PTR ),
	CLI_SECTION( "Authoritative SRV Records",		kMDNSReplierInfoText_SRV ),
	CLI_SECTION( "Authoritative TXT Records",		kMDNSReplierInfoText_TXT ),
	CLI_SECTION( "Authoritative A Records",			kMDNSReplierInfoText_A ),
	CLI_SECTION( "Authoritative AAAA Records",		kMDNSReplierInfoText_AAAA ),
	CLI_SECTION( "Responses",						kMDNSReplierInfoText_Responses ),
	CLI_OPTION_END()
};

static void	MDNSReplierCmd( void );

//===========================================================================================================================
//	Test Command Options
//===========================================================================================================================

#define kTestExitStatusSection_Name		"Exit Status"
#define kTestExitStatusSection_Text																						\
	"This test command can exit with one of three status codes:\n"														\
	"\n"																												\
	"0 - The test ran to completion and passed.\n"																		\
	"1 - A fatal error prevented the test from completing.\n"															\
	"2 - The test ran to completion, but it or a subtest failed. See test output for details.\n"						\
	"\n"																												\
	"Note: The pass/fail status applies to the correctness or results. It does not necessarily imply anything about\n"	\
	"performance.\n"

#define TestExitStatusSection()		CLI_SECTION( kTestExitStatusSection_Name, kTestExitStatusSection_Text )

#define kGAIPerfTestSuiteName_Basic			"basic"
#define kGAIPerfTestSuiteName_Advanced		"advanced"

static const char *		gGAIPerf_TestSuite				= NULL;
static int				gGAIPerf_CallDelayMs			= 10;
static int				gGAIPerf_ServerDelayMs			= 10;
static int				gGAIPerf_SkipPathEvalulation	= false;
static int				gGAIPerf_BadUDPMode				= false;
static int				gGAIPerf_IterationCount			= 100;
static int				gGAIPerf_IterationTimeLimitMs	= 100;
static const char *		gGAIPerf_OutputFilePath			= NULL;
static const char *		gGAIPerf_OutputFormat			= kOutputFormatStr_JSON;
static int				gGAIPerf_OutputAppendNewline	= false;

static void	GAIPerfCmd( void );

#define kGAIPerfSectionText_TestSuiteBasic																					\
	"This test suite consists of the following three test cases:\n"															\
	"\n"																													\
	"Test Case #1: Resolve a domain name with\n"																			\
	"\n"																													\
	"    2 CNAME records, 4 A records, and 4 AAAA records\n"																\
	"\n"																													\
	"to its IPv4 and IPv6 addresses. Each iteration resolves a unique instance of such a domain name, which requires\n"		\
	"server queries.\n"																										\
	"\n"																													\
	"Test Case #2: Resolve a domain name with\n"																			\
	"\n"																													\
	"    2 CNAME records, 4 A records, and 4 AAAA records\n"																\
	"\n"																													\
	"to its IPv4 and IPv6 addresses. A preliminary iteration resolves a unique instance of such a domain name, which\n"		\
	"requires server queries. Each subsequent iteration resolves the same domain name as the preliminary iteration,\n"		\
	"which should ideally require no additional server queries, i.e., the results should come from the cache.\n"			\
	"\n"																													\
	"Unlike the preceding test case, this test case is concerned with DNSServiceGetAddrInfo() performance when the\n"		\
	"records of the domain name being resolved are already in the cache. Therefore, the time required to resolve the\n"		\
	"domain name in the preliminary iteration isn't counted in the performance stats.\n"									\
	"\n"																													\
	"Test Case #3: Each iteration resolves localhost to its IPv4 and IPv6 addresses.\n"

#define kGAIPerfSectionText_TestSuiteAdvanced																				\
	"This test suite consists of 33 test cases. Test cases 1 through 32 can be described in the following way\n"			\
	"\n"																													\
	"Test Case #N (where N is in [1, 32] and odd): Resolve a domain name with\n"											\
	"\n"																													\
	"    N_c CNAME records, N_a A records, and N_a AAAA records\n"															\
	"\n"																													\
	"to its IPv4 and IPv6 addresses. Each iteration resolves a unique instance of such a domain name, which requires\n"		\
	"server queries.\n"																										\
	"\n"																													\
	"Test Case #N (where N is in [1, 32] and even): Resolve a domain name with\n"											\
	"\n"																													\
	"    N_c CNAME records, N_a A records, and N_a AAAA records\n"															\
	"\n"																													\
	"to its IPv4 and IPv6 addresses. A preliminary iteration resolves a unique instance of such a domain name, which\n"		\
	"requires server queries. Each subsequent iteration resolves the same domain name as the preliminary iteration,\n"		\
	"which should ideally require no additional server queries, i.e., the results should come from the cache.\n"			\
	"\n"																													\
	"Unlike the preceding test case, this test case is concerned with DNSServiceGetAddrInfo() performance when the\n"		\
	"records of the domain name being resolved are already in the cache. Therefore, the time required to resolve the\n"		\
	"domain name in the preliminary iteration isn't counted in the performance stats.\n"									\
	"\n"																													\
	"N_c and N_a take on the following values, depending on the value of N:\n"												\
	"\n"																													\
	"    N_c is 0 if N is in [1, 8].\n"																						\
	"    N_c is 1 if N is in [9, 16].\n"																					\
	"    N_c is 2 if N is in [17, 24].\n"																					\
	"    N_c is 4 if N is in [25, 32].\n"																					\
	"\n"																													\
	"    N_a is 1 if N mod 8 is 1 or 2.\n"																					\
	"    N_a is 2 if N mod 8 is 3 or 4.\n"																					\
	"    N_a is 4 if N mod 8 is 5 or 6.\n"																					\
	"    N_a is 8 if N mod 8 is 7 or 0.\n"																					\
	"\n"																													\
	"Finally,\n"																											\
	"\n"																													\
	"Test Case #33: Each iteration resolves localhost to its IPv4 and IPv6 addresses.\n"

static CLIOption		kGAIPerfOpts[] =
{
	StringOptionEx( 's', "suite",         &gGAIPerf_TestSuite,            "name", "Name of the predefined test suite to run.", true,
		"\n"
		"There are currently two predefined test suites, '" kGAIPerfTestSuiteName_Basic "' and '" kGAIPerfTestSuiteName_Advanced "', which are described below.\n"
		"\n"
	),
	StringOption(   'o', "output",        &gGAIPerf_OutputFilePath,       "path", "Path of the file to write test results to instead of standard output (stdout).", false ),
	FormatOption(   'f', "format",        &gGAIPerf_OutputFormat,         "Specifies the test results output format. (default: " kOutputFormatStr_JSON ")", false ),
	BooleanOption(  'n', "appendNewline", &gGAIPerf_OutputAppendNewline,  "If the output format is JSON, output a trailing newline character." ),
	IntegerOption(  'i', "iterations",    &gGAIPerf_IterationCount,       "count", "The number of iterations per test case. (default: 100)", false ),
	IntegerOption(  'l', "timeLimit",     &gGAIPerf_IterationTimeLimitMs, "ms", "Time limit for each DNSServiceGetAddrInfo() operation in milliseconds. (default: 100)", false ),
	IntegerOption(   0 , "callDelay",     &gGAIPerf_CallDelayMs,          "ms", "Time to wait before calling DNSServiceGetAddrInfo() in milliseconds. (default: 10)", false ),
	BooleanOption(   0 , "skipPathEval",  &gGAIPerf_SkipPathEvalulation,  "Use kDNSServiceFlagsPathEvaluationDone when calling DNSServiceGetAddrInfo()." ),
	
	CLI_OPTION_GROUP( "DNS Server Options" ),
	IntegerOption(   0 , "responseDelay", &gGAIPerf_ServerDelayMs,        "ms", "Additional delay in milliseconds to have the server apply to responses. (default: 10)", false ),
	BooleanOption(   0 , "badUDPMode",    &gGAIPerf_BadUDPMode,           "Run server in Bad UDP mode to trigger mDNSResponder's TCP fallback mechanism." ),
	
	CLI_SECTION( "Test Suite \"Basic\"",	kGAIPerfSectionText_TestSuiteBasic ),
	CLI_SECTION( "Test Suite \"Advanced\"",	kGAIPerfSectionText_TestSuiteAdvanced ),
	TestExitStatusSection(),
	CLI_OPTION_END()
};

static void	MDNSDiscoveryTestCmd( void );

static int				gMDNSDiscoveryTest_InstanceCount		= 100;
static int				gMDNSDiscoveryTest_TXTSize				= 100;
static int				gMDNSDiscoveryTest_BrowseTimeSecs		= 2;
static int				gMDNSDiscoveryTest_FlushCache			= false;
static char *			gMDNSDiscoveryTest_Interface			= NULL;
static int				gMDNSDiscoveryTest_NoAdditionals		= false;
static int				gMDNSDiscoveryTest_RecordCountA			= 1;
static int				gMDNSDiscoveryTest_RecordCountAAAA		= 1;
static double			gMDNSDiscoveryTest_UnicastDropRate		= 0.0;
static double			gMDNSDiscoveryTest_MulticastDropRate	= 0.0;
static int				gMDNSDiscoveryTest_MaxDropCount			= 0;
static int				gMDNSDiscoveryTest_UseIPv4				= false;
static int				gMDNSDiscoveryTest_UseIPv6				= false;
static const char *		gMDNSDiscoveryTest_OutputFormat			= kOutputFormatStr_JSON;
static int				gMDNSDiscoveryTest_OutputAppendNewline	= false;
static const char *		gMDNSDiscoveryTest_OutputFilePath		= NULL;

static CLIOption		kMDNSDiscoveryTestOpts[] =
{
	IntegerOption( 'c', "instanceCount",  &gMDNSDiscoveryTest_InstanceCount,       "count", "Number of service instances to discover. (default: 100)", false ),
	IntegerOption( 's', "txtSize",        &gMDNSDiscoveryTest_TXTSize,             "bytes", "Desired size of each service instance's TXT record data. (default: 100)", false ),
	IntegerOption( 'b', "browseTime",     &gMDNSDiscoveryTest_BrowseTimeSecs,      "seconds", "Amount of time to spend browsing in seconds. (default: 2)", false ),
	BooleanOption(  0 , "flushCache",     &gMDNSDiscoveryTest_FlushCache,          "Flush mDNSResponder's record cache before browsing. Requires root privileges." ),
	
	CLI_OPTION_GROUP( "mDNS Replier Parameters" ),
	StringOption(  'i', "interface",      &gMDNSDiscoveryTest_Interface,           "name or index", "Network interface. If unspecified, any available mDNS-capable interface will be used.", false ),
	BooleanOption(  0 , "noAdditionals",  &gMDNSDiscoveryTest_NoAdditionals,       "When answering queries, don't include any additional records." ),
	IntegerOption(  0 , "countA",         &gMDNSDiscoveryTest_RecordCountA,        "count", "Number of A records per hostname. (default: 1)", false ),
	IntegerOption(  0 , "countAAAA",      &gMDNSDiscoveryTest_RecordCountAAAA,     "count", "Number of AAAA records per hostname. (default: 1)", false ),
	DoubleOption(   0 , "udrop",          &gMDNSDiscoveryTest_UnicastDropRate,     "probability", "Probability of dropping a unicast response. (default: 0.0)", false ),
	DoubleOption(   0 , "mdrop",          &gMDNSDiscoveryTest_MulticastDropRate,   "probability", "Probability of dropping a multicast query or response. (default: 0.0)", false ),
	IntegerOption(  0 , "maxDropCount",   &gMDNSDiscoveryTest_MaxDropCount,        "count", "If > 0, drop probabilities are limted to first <count> responses from each instance. (default: 0)", false ),
	BooleanOption(  0 , "ipv4",           &gMDNSDiscoveryTest_UseIPv4,             "Use IPv4." ),
	BooleanOption(  0 , "ipv6",           &gMDNSDiscoveryTest_UseIPv6,             "Use IPv6." ),
	
	CLI_OPTION_GROUP( "Results" ),
	FormatOption(   'f', "format",        &gMDNSDiscoveryTest_OutputFormat,        "Specifies the test results output format. (default: " kOutputFormatStr_JSON ")", false ),
	StringOption(   'o', "output",        &gMDNSDiscoveryTest_OutputFilePath,      "path", "Path of the file to write test results to instead of standard output (stdout).", false ),
	
	TestExitStatusSection(),
	CLI_OPTION_END()
};

static void	DotLocalTestCmd( void );

static const char *		gDotLocalTest_Interface			= NULL;
static const char *		gDotLocalTest_OutputFormat		= kOutputFormatStr_JSON;
static const char *		gDotLocalTest_OutputFilePath	= NULL;

#define kDotLocalTestSubtestDesc_GAIMDNSOnly	"GAI for a dotlocal name that has only MDNS A and AAAA records."
#define kDotLocalTestSubtestDesc_GAIDNSOnly		"GAI for a dotlocal name that has only DNS A and AAAA records."
#define kDotLocalTestSubtestDesc_GAIBoth		"GAI for a dotlocal name that has both mDNS and DNS A and AAAA records."
#define kDotLocalTestSubtestDesc_GAINeither		"GAI for a dotlocal name that has no A or AAAA records."
#define kDotLocalTestSubtestDesc_GAINoSuchRecord \
	"GAI for a dotlocal name that has no A or AAAA records, but is a subdomain name of a search domain."
#define kDotLocalTestSubtestDesc_QuerySRV		"SRV query for a dotlocal name that has only a DNS SRV record."

#define kDotLocalTestSectionText_Description																				\
	"The goal of the dotlocal test is to verify that mDNSResponder properly handles queries for domain names in the\n"		\
	"local domain when a local SOA record exists. As part of the test setup, a test DNS server and an mdnsreplier are\n"	\
	"spawned, and a dummy local SOA record is registered with DNSServiceRegisterRecord(). The server is invoked such\n"		\
	"that its domain is a second-level subdomain of the local domain, i.e., <some label>.local, while the mdnsreplier is\n"	\
	"invoked such that its base hostname is equal to the server's domain, e.g., if the server's domain is test.local.,\n"	\
	"then the mdnsreplier's base hostname is test.local.\n"																	\
	"\n"																													\
	"The dotlocal test consists of six subtests that perform either a DNSServiceGetAddrInfo (GAI) operation for a\n"		\
	"hostname in the local domain or a DNSServiceQueryRecord operation to query for an SRV record in the local domain:\n"	\
	"\n"																													\
	"1. " kDotLocalTestSubtestDesc_GAIMDNSOnly		"\n"																	\
	"2. " kDotLocalTestSubtestDesc_GAIDNSOnly		"\n"																	\
	"3. " kDotLocalTestSubtestDesc_GAIBoth			"\n"																	\
	"4. " kDotLocalTestSubtestDesc_GAINeither		"\n"																	\
	"5. " kDotLocalTestSubtestDesc_GAINoSuchRecord	"\n"																	\
	"6. " kDotLocalTestSubtestDesc_QuerySRV			"\n"																	\
	"\n"																													\
	"Each subtest runs for five seconds.\n"

static CLIOption		kDotLocalTestOpts[] =
{
	StringOption(  'i', "interface",     &gDotLocalTest_Interface,           "name or index", "mdnsreplier's network interface. If not set, any mDNS-capable interface will be used.", false ),
	
	CLI_OPTION_GROUP( "Results" ),
	FormatOption(  'f', "format",        &gDotLocalTest_OutputFormat,        "Specifies the test results output format. (default: " kOutputFormatStr_JSON ")", false ),
	StringOption(  'o', "output",        &gDotLocalTest_OutputFilePath,      "path", "Path of the file to write test results to instead of standard output (stdout).", false ),
	
	CLI_SECTION( "Description", kDotLocalTestSectionText_Description ),
	TestExitStatusSection(),
	CLI_OPTION_END()
};

static void	ProbeConflictTestCmd( void );

static const char *		gProbeConflictTest_Interface		= NULL;
static int				gProbeConflictTest_UseComputerName	= false;
static const char *		gProbeConflictTest_OutputFormat		= kOutputFormatStr_JSON;
static const char *		gProbeConflictTest_OutputFilePath	= NULL;

static CLIOption		kProbeConflictTestOpts[] =
{
	StringOption(  'i', "interface",       &gProbeConflictTest_Interface,           "name or index", "mdnsreplier's network interface. If not set, any mDNS-capable interface will be used.", false ),
	BooleanOption( 'c', "useComputerName", &gProbeConflictTest_UseComputerName,     "Use the device's \"computer name\" for the test service's name." ),
	
	CLI_OPTION_GROUP( "Results" ),
	FormatOption(  'f', "format",          &gProbeConflictTest_OutputFormat,        "Specifies the test report output format. (default: " kOutputFormatStr_JSON ")", false ),
	StringOption(  'o', "output",          &gProbeConflictTest_OutputFilePath,      "path", "Path of the file to write test report to instead of standard output (stdout).", false ),
	
	TestExitStatusSection(),
	CLI_OPTION_END()
};

static void	RegistrationTestCmd( void );

static int				gRegistrationTest_BATSEnvironment	= false;
static const char *		gRegistrationTest_OutputFormat		= kOutputFormatStr_JSON;
static const char *		gRegistrationTest_OutputFilePath	= NULL;

static CLIOption		kRegistrationTestOpts[] =
{
	CLI_OPTION_BOOLEAN( 0, "bats", &gRegistrationTest_BATSEnvironment, "Informs the test that it's running in a BATS environment.",
		"\n"
		"This option allows the test to take special measures while running in a BATS environment. Currently, this option\n"
		"only has an effect on watchOS. Because it has been observed that the Wi-Fi interface sometimes goes down during\n"
		"watchOS BATS testing, for watchOS, when a service is registered using kDNSServiceInterfaceIndexAny,\n"
		"\n"
		"    1. missing browse and query \"add\" results for Wi-Fi interfaces aren't enough for a subtest to fail; and\n"
		"    2. unexpected browse and query results for Wi-Fi interfaces are ignored.\n"
	),
	CLI_OPTION_GROUP( "Results" ),
	FormatOption( 'f', "format", &gRegistrationTest_OutputFormat,   "Specifies the test results output format. (default: " kOutputFormatStr_JSON ")", false ),
	StringOption( 'o', "output", &gRegistrationTest_OutputFilePath, "path", "Path of the file to write test results to instead of standard output (stdout).", false ),
	
	TestExitStatusSection(),
	CLI_OPTION_END()
};

static void ExpensiveConstrainedTestCmd( void );

static const char *     gExpensiveConstrainedTest_Interface                 = NULL;
static const char *     gExpensiveConstrainedTest_Name                      = NULL;
static Boolean          gExpensiveConstrainedTest_DenyExpensive             = false;
static Boolean          gExpensiveConstrainedTest_DenyConstrained           = false;
static Boolean          gExpensiveConstrainedTest_StartFromExpensive        = false;
static int              gExpensiveConstrainedTest_ProtocolIPv4              = false;
static int              gExpensiveConstrainedTest_ProtocolIPv6              = false;
static const char *     gExpensiveConstrainedTest_OutputFormat              = kOutputFormatStr_JSON;
static const char *     gExpensiveConstrainedTest_OutputFilePath            = NULL;

static CLIOption        kExpensiveConstrainedTestOpts[] =
{
    CLI_OPTION_GROUP( "Results" ),
    FormatOption( 'f', "format", &gExpensiveConstrainedTest_OutputFormat,              "Specifies the test results output format. (default: " kOutputFormatStr_JSON ")", false ),
    StringOption( 'o', "output", &gExpensiveConstrainedTest_OutputFilePath, "path",    "Path of the file to write test results to instead of standard output (stdout).", false ),

    TestExitStatusSection(),
    CLI_OPTION_END()
};

#if( MDNSRESPONDER_PROJECT )
static void XCTestCmd( void );

static const char *     gXCTest_Classname        = NULL;

static CLIOption        kXCTestOpts[] =
{
    StringOption(      'c', "class", &gXCTest_Classname, "classname", "The classname of the XCTest to run (from /AppleInternal/XCTests/com.apple.mDNSResponder/Tests.xctest)", true ),
    CLI_OPTION_END()
};
#endif

#if( TARGET_OS_DARWIN )
static void	KeepAliveTestCmd( void );

static const char *		gKeepAliveTest_OutputFormat		= kOutputFormatStr_JSON;
static const char *		gKeepAliveTest_OutputFilePath	= NULL;

static CLIOption		kKeepAliveTestOpts[] =
{
	CLI_OPTION_GROUP( "Results" ),
	FormatOption( 'f', "format", &gKeepAliveTest_OutputFormat,   "Specifies the test results output format. (default: " kOutputFormatStr_JSON ")", false ),
	StringOption( 'o', "output", &gKeepAliveTest_OutputFilePath, "path", "Path of the file to write test results to instead of standard output (stdout).", false ),
	
	TestExitStatusSection(),
	CLI_OPTION_END()
};
#endif

static CLIOption		kTestOpts[] =
{
	Command( "gaiperf",        GAIPerfCmd,           kGAIPerfOpts,            "Runs DNSServiceGetAddrInfo() performance tests.", false ),
	Command( "mdnsdiscovery",  MDNSDiscoveryTestCmd, kMDNSDiscoveryTestOpts,  "Tests mDNS service discovery for correctness.", false ),
	Command( "dotlocal",       DotLocalTestCmd,      kDotLocalTestOpts,       "Tests DNS and mDNS queries for domain names in the local domain.", false ),
	Command( "probeconflicts", ProbeConflictTestCmd, kProbeConflictTestOpts,  "Tests various probing conflict scenarios.", false ),
	Command( "registration",   RegistrationTestCmd,  kRegistrationTestOpts,   "Tests service registrations.", false ),
    Command( "expensive_constrained_updates", ExpensiveConstrainedTestCmd, kExpensiveConstrainedTestOpts, "Tests if the mDNSResponder can handle expensive and constrained property change correctly", false),
#if( MDNSRESPONDER_PROJECT )
    Command( "xctest",         XCTestCmd,            kXCTestOpts,              "Run a XCTest from /AppleInternal/XCTests/com.apple.mDNSResponder/Tests.xctest.", true ),
#endif
#if( TARGET_OS_DARWIN )
	Command( "keepalive",      KeepAliveTestCmd,     kKeepAliveTestOpts,      "Tests keepalive record registrations.", false ),
#endif
	CLI_OPTION_END()
};

//===========================================================================================================================
//	SSDP Command Options
//===========================================================================================================================

static int				gSSDPDiscover_MX			= 1;
static const char *		gSSDPDiscover_ST			= "ssdp:all";
static int				gSSDPDiscover_ReceiveSecs	= 1;
static int				gSSDPDiscover_UseIPv4		= false;
static int				gSSDPDiscover_UseIPv6		= false;
static int				gSSDPDiscover_Verbose		= false;

static CLIOption		kSSDPDiscoverOpts[] =
{
	StringOption(  'i', "interface",	&gInterface,				"name or index", "Network interface by name or index.", true ),
	IntegerOption( 'm', "mx",			&gSSDPDiscover_MX,			"seconds", "MX value in search request, i.e., max response delay in seconds. (Default: 1 second)", false ),
	StringOption(  's', "st",			&gSSDPDiscover_ST,			"string", "ST value in search request, i.e., the search target. (Default: \"ssdp:all\")", false ),
	IntegerOption( 'r', "receiveTime",	&gSSDPDiscover_ReceiveSecs,	"seconds", "Amount of time to spend receiving responses. -1 means unlimited. (Default: 1 second)", false ),
	BooleanOption(  0 , "ipv4",			&gSSDPDiscover_UseIPv4,		"Use IPv4, i.e., multicast to 239.255.255.250:1900." ),
	BooleanOption(  0 , "ipv6",			&gSSDPDiscover_UseIPv6,		"Use IPv6, i.e., multicast to [ff02::c]:1900" ),
	BooleanOption( 'v', "verbose",		&gSSDPDiscover_Verbose,		"Prints the search request(s) that were sent." ),
	CLI_OPTION_END()
};

static void	SSDPDiscoverCmd( void );

static CLIOption		kSSDPOpts[] =
{
	Command( "discover", SSDPDiscoverCmd, kSSDPDiscoverOpts, "Crafts and multicasts an SSDP search message.", false ),
	CLI_OPTION_END()
};

#if( TARGET_OS_DARWIN )
//===========================================================================================================================
//	res_query Command Options
//===========================================================================================================================

static void	ResQueryCmd( void );

static const char *		gResQuery_Name			= NULL;
static const char *		gResQuery_Type			= NULL;
static const char *		gResQuery_Class			= NULL;
static int				gResQuery_UseLibInfo	= false;

static CLIOption		kResQueryOpts[] =
{
	StringOption( 'n', "name",		&gResQuery_Name,		"domain name",	"Full domain name of record to query.", true ),
	StringOption( 't', "type",		&gResQuery_Type,		"record type",	"Record type by name (e.g., TXT, SRV, etc.) or number.", true ),
	StringOption( 'c', "class",		&gResQuery_Class,		"record class",	"Record class by name or number. Default class is IN.", false ),
	BooleanOption( 0 , "libinfo",	&gResQuery_UseLibInfo,	"Use res_query from libinfo instead of libresolv." ),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	dns_query Command Options
//===========================================================================================================================

static void ResolvDNSQueryCmd( void );

static const char *		gResolvDNSQuery_Name	= NULL;
static const char *		gResolvDNSQuery_Type	= NULL;
static const char *		gResolvDNSQuery_Class	= NULL;
static const char *		gResolvDNSQuery_Path	= NULL;

static CLIOption		kResolvDNSQueryOpts[] =
{
	StringOption( 'n', "name",	&gResolvDNSQuery_Name,	"domain name",	"Full domain name of record to query.", true ),
	StringOption( 't', "type",	&gResolvDNSQuery_Type,	"record type",	"Record type by name (e.g., TXT, SRV, etc.) or number.", true ),
	StringOption( 'c', "class",	&gResolvDNSQuery_Class,	"record class",	"Record class by name or number. Default class is IN.", false ),
	StringOption( 'p', "path",	&gResolvDNSQuery_Path,	"file path",	"The path argument to pass to dns_open() before calling dns_query(). Default value is NULL.", false ),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	CFHost Command Options
//===========================================================================================================================

static void	CFHostCmd( void );

static const char *		gCFHost_Name		= NULL;
static int				gCFHost_WaitSecs	= 0;

static CLIOption		kCFHostOpts[] =
{
	StringOption(  'n', "name", &gCFHost_Name,     "hostname", "Hostname to resolve.", true ),
	IntegerOption( 'w', "wait", &gCFHost_WaitSecs, "seconds",  "Time in seconds to wait before a normal exit. (default: 0)", false ),
	CLI_OPTION_END()
};

static CLIOption		kLegacyOpts[] =
{
	Command( "res_query", ResQueryCmd,       kResQueryOpts,       "Uses res_query() from either libresolv or libinfo to query for a record.", true ),
	Command( "dns_query", ResolvDNSQueryCmd, kResolvDNSQueryOpts, "Uses dns_query() from libresolv to query for a record.", true ),
	Command( "cfhost",    CFHostCmd,         kCFHostOpts,         "Uses CFHost to resolve a hostname.", true ),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	DNSConfigAdd Command Options
//===========================================================================================================================

static void	DNSConfigAddCmd( void );

static CFStringRef		gDNSConfigAdd_ID			= NULL;
static char **			gDNSConfigAdd_IPAddrArray	= NULL;
static size_t			gDNSConfigAdd_IPAddrCount	= 0;
static char **			gDNSConfigAdd_DomainArray	= NULL;
static size_t			gDNSConfigAdd_DomainCount	= 0;
static const char *		gDNSConfigAdd_Interface		= NULL;

static CLIOption		kDNSConfigAddOpts[] =
{
	CFStringOption(     0 , "id",        &gDNSConfigAdd_ID,                                      "ID", "Arbitrary ID to use for resolver entry.", true ),
	MultiStringOption( 'a', "address",   &gDNSConfigAdd_IPAddrArray, &gDNSConfigAdd_IPAddrCount, "IP address", "DNS server IP address(es). Can be specified more than once.", true ),
	MultiStringOption( 'd', "domain",    &gDNSConfigAdd_DomainArray, &gDNSConfigAdd_DomainCount, "domain", "Specific domain(s) for the resolver entry. Can be specified more than once.", false ),
	StringOption(      'i', "interface", &gDNSConfigAdd_Interface,                               "interface name", "Specific interface for the resolver entry.", false ),
	
	CLI_SECTION( "Notes", "Run 'scutil -d -v --dns' to see the current DNS configuration. See scutil(8) man page for more details.\n" ),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	DNSConfigRemove Command Options
//===========================================================================================================================

static void	DNSConfigRemoveCmd( void );

static CFStringRef		gDNSConfigRemove_ID = NULL;

static CLIOption		kDNSConfigRemoveOpts[] =
{
	CFStringOption( 0, "id", &gDNSConfigRemove_ID, "ID", "ID of resolver entry to remove.", true ),
	
	CLI_SECTION( "Notes", "Run 'scutil -d -v --dns' to see the current DNS configuration. See scutil(8) man page for more details.\n" ),
	CLI_OPTION_END()
};

static CLIOption		kDNSConfigOpts[] =
{
	Command( "add",    DNSConfigAddCmd,    kDNSConfigAddOpts,    "Add a supplemental resolver entry to the system's DNS configuration.", true ),
	Command( "remove", DNSConfigRemoveCmd, kDNSConfigRemoveOpts, "Remove a supplemental resolver entry from the system's DNS configuration.", true ),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	XPCSend
//===========================================================================================================================

static void	XPCSendCmd( void );

static const char *		gXPCSend_ServiceName	= NULL;
static const char *		gXPCSend_MessageStr		= NULL;

static const char		kXPCSendMessageSection_Name[] = "Message Argument";
static const char		kXPCSendMessageSection_Text[] =
	"XPC messages are described as a string using the following syntax.\n"
	"\n"
	"With the exception of the top-most XPC message dictionary, dictionaries begin with a '{' and end with a '}'.\n"
	"Key-value pairs are of the form <key>=<value>, where <key> is a string and <value> is a value of any of the\n"
	"currently supported XPC types.\n"
	"\n"
	"Arrays begin with a '[' and end with a ']'.\n"
	"\n"
	"The following non-container XPC types are supported:\n"
	"\n"
	"Type                              Syntax                      Example\n"
	"bool                              bool:<string>               bool:true (or yes/y/on/1), bool:false (or no/n/off/0)\n"
	"data                              data:<hex string>           data:C0000201\n"
	"int64  (signed 64-bit integer)    int:<pos. or neg. integer>  int:-1\n"
	"string                            string:<string>             string:hello\\ world\n"
	"uint64 (unsigned 64-bit integer)  uint:<pos. integer>         uint:1024 or uint:0x400\n"
	"UUID                              uuid:<UUID>                 uuid:dab10183-84b5-4859-9de6-4bee287cfea3\n"
	"\n"
	"Example 1: 'cmd=string:add make=string:Apple model=string:Macintosh aliases=[string:Mac string:Macintosh\\ 128K]'\n"
	"Example 2: 'cmd=string:search features={portable=bool:yes solar=bool:no} priceMin=uint:100 priceMax=uint:200'\n";

static CLIOption		kXPCSendOpts[] =
{
	StringOption( 's', "service", &gXPCSend_ServiceName, "service name", "XPC service name.", true ),
	StringOption( 'm', "message", &gXPCSend_MessageStr,  "message",      "XPC message as a string.", true ),
	
	CLI_SECTION( kXPCSendMessageSection_Name, kXPCSendMessageSection_Text ),
	CLI_OPTION_END()
};
#endif	// TARGET_OS_DARWIN

#if( MDNSRESPONDER_PROJECT )
//===========================================================================================================================
//	InterfaceMonitor
//===========================================================================================================================

static void InterfaceMonitorCmd( void );

static CLIOption		kInterfaceMonitorOpts[] =
{
	StringOption( 'i', "interface", &gInterface, "name or index", "Network interface by name or index.", true ),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	DNSProxy
//===========================================================================================================================

static void DNSProxyCmd( void );

static char **			gDNSProxy_InputInterfaces		= NULL;
static size_t			gDNSProxy_InputInterfaceCount	= 0;
static const char *		gDNSProxy_OutputInterface		= NULL;

static CLIOption		kDNSProxyOpts[] =
{
	MultiStringOption( 'i', "inputInterface",  &gDNSProxy_InputInterfaces, &gDNSProxy_InputInterfaceCount, "name or index", "Interface to accept queries on. Can be specified more than once.", true ),
	StringOption(      'o', "outputInterface", &gDNSProxy_OutputInterface, "name or index", "Interface to forward queries over. Use '0' for primary interface. (default: 0)", false ),
	CLI_OPTION_END()
};

#endif	// MDNSRESPONDER_PROJECT

//===========================================================================================================================
//	Command Table
//===========================================================================================================================

static OSStatus	VersionOptionCallback( CLIOption *inOption, const char *inArg, int inUnset );

static void	BrowseCmd( void );
static void	GetAddrInfoCmd( void );
static void	QueryRecordCmd( void );
static void	RegisterCmd( void );
static void	RegisterRecordCmd( void );
static void	ResolveCmd( void );
static void	ReconfirmCmd( void );
static void	GetAddrInfoPOSIXCmd( void );
static void	ReverseLookupCmd( void );
static void	PortMappingCmd( void );
static void	BrowseAllCmd( void );
static void	GetAddrInfoStressCmd( void );
static void	DNSQueryCmd( void );
#if( DNSSDUTIL_INCLUDE_DNSCRYPT )
static void	DNSCryptCmd( void );
#endif
static void	MDNSQueryCmd( void );
static void	PIDToUUIDCmd( void );
static void	DaemonVersionCmd( void );

static CLIOption		kGlobalOpts[] =
{
	CLI_OPTION_CALLBACK_EX( 'V', "version", VersionOptionCallback, NULL, NULL,
		kCLIOptionFlags_NoArgument | kCLIOptionFlags_GlobalOnly, "Displays the version of this tool.", NULL ),
	CLI_OPTION_HELP(),
	
	// Common commands.
	
	Command( "browse",				BrowseCmd,				kBrowseOpts,			"Uses DNSServiceBrowse() to browse for one or more service types.", false ),
	Command( "getAddrInfo",			GetAddrInfoCmd,			kGetAddrInfoOpts,		"Uses DNSServiceGetAddrInfo() to resolve a hostname to IP addresses.", false ),
	Command( "queryRecord",			QueryRecordCmd,			kQueryRecordOpts,		"Uses DNSServiceQueryRecord() to query for an arbitrary DNS record.", false ),
	Command( "register",			RegisterCmd,			kRegisterOpts,			"Uses DNSServiceRegister() to register a service.", false ),
	Command( "registerRecord",		RegisterRecordCmd,		kRegisterRecordOpts,	"Uses DNSServiceRegisterRecord() to register a record.", false ),
	Command( "resolve",				ResolveCmd,				kResolveOpts,			"Uses DNSServiceResolve() to resolve a service.", false ),
	Command( "reconfirm",			ReconfirmCmd,			kReconfirmOpts,			"Uses DNSServiceReconfirmRecord() to reconfirm a record.", false ),
	Command( "getaddrinfo-posix",	GetAddrInfoPOSIXCmd,	kGetAddrInfoPOSIXOpts,	"Uses getaddrinfo() to resolve a hostname to IP addresses.", false ),
	Command( "reverseLookup",		ReverseLookupCmd,		kReverseLookupOpts,		"Uses DNSServiceQueryRecord() to perform a reverse IP address lookup.", false ),
	Command( "portMapping",			PortMappingCmd,			kPortMappingOpts,		"Uses DNSServiceNATPortMappingCreate() to create a port mapping.", false ),
#if( TARGET_OS_DARWIN )
	Command( "registerKA",			RegisterKACmd,			kRegisterKA_Opts,		"Uses DNSServiceSleepKeepalive_sockaddr() to register a keep alive record.", false ),
#endif
	Command( "browseAll",			BrowseAllCmd,			kBrowseAllOpts,			"Browse and resolve all (or specific) services and, optionally, attempt connections.", false ),
	
	// Uncommon commands.
	
	Command( "getnameinfo",			GetNameInfoCmd,			kGetNameInfoOpts,		"Calls getnameinfo() and prints results.", true ),
	Command( "getAddrInfoStress",	GetAddrInfoStressCmd,	kGetAddrInfoStressOpts,	"Runs DNSServiceGetAddrInfo() stress testing.", true ),
	Command( "DNSQuery",			DNSQueryCmd,			kDNSQueryOpts,			"Crafts and sends a DNS query.", true ),
#if( DNSSDUTIL_INCLUDE_DNSCRYPT )
	Command( "DNSCrypt",			DNSCryptCmd,			kDNSCryptOpts,			"Crafts and sends a DNSCrypt query.", true ),
#endif
	Command( "mdnsquery",			MDNSQueryCmd,			kMDNSQueryOpts,			"Crafts and sends an mDNS query over the specified interface.", true ),
	Command( "mdnscollider",		MDNSColliderCmd,		kMDNSColliderOpts,		"Creates record name collision scenarios.", true ),
	Command( "pid2uuid",			PIDToUUIDCmd,			kPIDToUUIDOpts,			"Prints the UUID of a process.", true ),
	Command( "server",				DNSServerCmd,			kDNSServerOpts,			"DNS server for testing.", true ),
	Command( "mdnsreplier",			MDNSReplierCmd,			kMDNSReplierOpts,		"Responds to mDNS queries for a set of authoritative resource records.", true ),
	Command( "test",				NULL,					kTestOpts,				"Commands for testing DNS-SD.", true ),
	Command( "ssdp",				NULL,					kSSDPOpts,				"Simple Service Discovery Protocol (SSDP).", true ),
#if( TARGET_OS_DARWIN )
	Command( "legacy",				NULL,					kLegacyOpts,			"Legacy DNS API.", true ),
	Command( "dnsconfig",			NULL,					kDNSConfigOpts,			"Add/remove a supplemental resolver entry to/from the system's DNS configuration.", true ),
	Command( "xpcsend",				XPCSendCmd,				kXPCSendOpts,			"Sends a message to an XPC service.", true ),
#endif
#if( MDNSRESPONDER_PROJECT )
	Command( "interfaceMonitor",	InterfaceMonitorCmd,	kInterfaceMonitorOpts,	"mDNSResponder's interface monitor.", true ),
	Command( "dnsproxy",			DNSProxyCmd,			kDNSProxyOpts,			"Enables mDNSResponder's DNS proxy.", true ),
#endif
	Command( "daemonVersion",		DaemonVersionCmd,		NULL,					"Prints the version of the DNS-SD daemon.", true ),
	
	CLI_COMMAND_HELP(),
	CLI_OPTION_END()
};

//===========================================================================================================================
//	Helper Prototypes
//===========================================================================================================================

#define kExitReason_OneShotDone				"one-shot done"
#define kExitReason_ReceivedResponse		"received response"
#define kExitReason_SIGINT					"interrupt signal"
#define kExitReason_Timeout					"timeout"
#define kExitReason_TimeLimit				"time limit"

static void	Exit( void *inContext ) ATTRIBUTE_NORETURN;

static DNSServiceFlags	GetDNSSDFlagsFromOpts( void );

typedef enum
{
	kConnectionType_None			= 0,
	kConnectionType_Normal			= 1,
	kConnectionType_DelegatePID		= 2,
	kConnectionType_DelegateUUID	= 3
	
}	ConnectionType;

typedef struct
{
	ConnectionType		type;
	union
	{
		int32_t			pid;
		uint8_t			uuid[ 16 ];
		
	}	delegate;
	
}	ConnectionDesc;

static OSStatus
	CreateConnectionFromArgString(
		const char *			inString,
		dispatch_queue_t		inQueue,
		DNSServiceRef *			outSDRef,
		ConnectionDesc *		outDesc );
static OSStatus			InterfaceIndexFromArgString( const char *inString, uint32_t *outIndex );
static OSStatus			RecordDataFromArgString( const char *inString, uint8_t **outDataPtr, size_t *outDataLen );
static OSStatus			RecordTypeFromArgString( const char *inString, uint16_t *outValue );
static OSStatus			RecordClassFromArgString( const char *inString, uint16_t *outValue );

#define kInterfaceNameBufLen		( Max( IF_NAMESIZE, 16 ) + 1 )

static char *			InterfaceIndexToName( uint32_t inIfIndex, char inNameBuf[ kInterfaceNameBufLen ] );
static const char *		RecordTypeToString( unsigned int inValue );

static OSStatus
	DNSRecordDataToString(
		const void *	inRDataPtr,
		size_t			inRDataLen,
		unsigned int	inRDataType,
		const void *	inMsgPtr,
		size_t			inMsgLen,
		char **			outString );

static OSStatus
	DNSMessageToText(
		const uint8_t *	inMsgPtr,
		size_t			inMsgLen,
		Boolean			inIsMDNS,
		Boolean			inPrintRaw,
		char **			outText );

static OSStatus
	WriteDNSQueryMessage(
		uint8_t			inMsg[ kDNSQueryMessageMaxLen ],
		uint16_t		inMsgID,
		uint16_t		inFlags,
		const char *	inQName,
		uint16_t		inQType,
		uint16_t		inQClass,
		size_t *		outMsgLen );

// Dispatch helpers

typedef void ( *DispatchHandler )( void *inContext );

static OSStatus
	DispatchSignalSourceCreate(
		int					inSignal,
		dispatch_queue_t	inQueue,
		DispatchHandler		inEventHandler,
		void *				inContext,
		dispatch_source_t *	outSource );
static OSStatus
	DispatchSocketSourceCreate(
		SocketRef				inSock,
		dispatch_source_type_t	inType,
		dispatch_queue_t		inQueue,
		DispatchHandler			inEventHandler,
		DispatchHandler			inCancelHandler,
		void *					inContext,
		dispatch_source_t *		outSource );

#define DispatchReadSourceCreate( SOCK, QUEUE, EVENT_HANDLER, CANCEL_HANDLER, CONTEXT, OUT_SOURCE ) \
	DispatchSocketSourceCreate( SOCK, DISPATCH_SOURCE_TYPE_READ, QUEUE, EVENT_HANDLER, CANCEL_HANDLER, CONTEXT, OUT_SOURCE )

#define DispatchWriteSourceCreate( SOCK, QUEUE, EVENT_HANDLER, CANCEL_HANDLER, CONTEXT, OUT_SOURCE ) \
	DispatchSocketSourceCreate( SOCK, DISPATCH_SOURCE_TYPE_WRITE, QUEUE, EVENT_HANDLER, CANCEL_HANDLER, CONTEXT, OUT_SOURCE )

static OSStatus
	DispatchTimerCreate(
		dispatch_time_t		inStart,
		uint64_t			inIntervalNs,
		uint64_t			inLeewayNs,
		dispatch_queue_t	inQueue,
		DispatchHandler		inEventHandler,
		DispatchHandler		inCancelHandler,
		void *				inContext,
		dispatch_source_t *	outTimer );

#define DispatchTimerOneShotCreate( IN_START, IN_LEEWAY, IN_QUEUE, IN_EVENT_HANDLER, IN_CONTEXT, OUT_TIMER )	\
	DispatchTimerCreate( IN_START, DISPATCH_TIME_FOREVER, IN_LEEWAY, IN_QUEUE, IN_EVENT_HANDLER, NULL, IN_CONTEXT, OUT_TIMER )

static OSStatus
	DispatchProcessMonitorCreate(
		pid_t				inPID,
		unsigned long		inFlags,
		dispatch_queue_t	inQueue,
		DispatchHandler		inEventHandler,
		DispatchHandler		inCancelHandler,
		void *				inContext,
		dispatch_source_t *	outMonitor );

static const char *	ServiceTypeDescription( const char *inName );

typedef struct
{
	SocketRef		sock;			// Socket.
	void *			userContext;	// User context.
	int32_t			refCount;		// Reference count.
	
}	SocketContext;

static OSStatus			SocketContextCreate( SocketRef inSock, void * inUserContext, SocketContext **outContext );
static SocketContext *	SocketContextRetain( SocketContext *inContext );
static void				SocketContextRelease( SocketContext *inContext );
static void				SocketContextCancelHandler( void *inContext );

#define ForgetSocketContext( X )	ForgetCustom( X, SocketContextRelease )

static OSStatus		StringToInt32( const char *inString, int32_t *outValue );
static OSStatus		StringToUInt32( const char *inString, uint32_t *outValue );
#if( TARGET_OS_DARWIN )
static int64_t		_StringToInt64( const char *inString, OSStatus *outError );
static uint64_t		_StringToUInt64( const char *inString, OSStatus *outError );
static pid_t		_StringToPID( const char *inString, OSStatus *outError );
static OSStatus
	_ParseEscapedString(
		const char *	inSrc,
		const char *	inEnd,
		const char *	inDelimiters,
		char *			inBufPtr,
		size_t			inBufLen,
		size_t *		outCopiedLen,
		size_t *		outActualLen,
		const char **	outPtr );
#endif
static OSStatus		StringToARecordData( const char *inString, uint8_t **outPtr, size_t *outLen );
static OSStatus		StringToAAAARecordData( const char *inString, uint8_t **outPtr, size_t *outLen );
static OSStatus		StringToDomainName( const char *inString, uint8_t **outPtr, size_t *outLen );
#if( TARGET_OS_DARWIN )
static OSStatus		GetDefaultDNSServer( sockaddr_ip *outAddr );
#endif
static OSStatus
	_ServerSocketOpenEx2( 
		int				inFamily, 
		int				inType, 
		int				inProtocol, 
		const void *	inAddr, 
		int				inPort, 
		int *			outPort, 
		int				inRcvBufSize, 
		Boolean			inNoPortReuse,
		SocketRef *		outSock );

static const struct sockaddr *	GetMDNSMulticastAddrV4( void );
static const struct sockaddr *	GetMDNSMulticastAddrV6( void );

static OSStatus
	CreateMulticastSocket(
		const struct sockaddr *	inAddr,
		int						inPort,
		const char *			inIfName,
		uint32_t				inIfIndex,
		Boolean					inJoin,
		int *					outPort,
		SocketRef *				outSock );

static OSStatus	DecimalTextToUInt32( const char *inSrc, const char *inEnd, uint32_t *outValue, const char **outPtr );
static OSStatus	CheckIntegerArgument( int inArgValue, const char *inArgName, int inMin, int inMax );
static OSStatus	CheckDoubleArgument( double inArgValue, const char *inArgName, double inMin, double inMax );
static OSStatus	CheckRootUser( void );
static OSStatus	SpawnCommand( pid_t *outPID, const char *inFormat, ... );
static OSStatus	OutputFormatFromArgString( const char *inArgString, OutputFormatType *outFormat );
static OSStatus	OutputPropertyList( CFPropertyListRef inPList, OutputFormatType inType, const char *inOutputFilePath );
static OSStatus	CreateSRVRecordDataFromString( const char *inString, uint8_t **outPtr, size_t *outLen );
static OSStatus	CreateTXTRecordDataFromString( const char *inString, int inDelimiter, uint8_t **outPtr, size_t *outLen );
static OSStatus
	CreateNSECRecordData(
		const uint8_t *	inNextDomainName,
		uint8_t **		outPtr,
		size_t *		outLen,
		unsigned int	inTypeCount,
		... );
static OSStatus
	AppendSOARecord(
		DataBuffer *	inDB,
		const uint8_t *	inNamePtr,
		size_t			inNameLen,
		uint16_t		inType,
		uint16_t		inClass,
		uint32_t		inTTL,
		const uint8_t *	inMName,
		const uint8_t *	inRName,
		uint32_t		inSerial,
		uint32_t		inRefresh,
		uint32_t		inRetry,
		uint32_t		inExpire,
		uint32_t		inMinimumTTL,
		size_t *		outLen );
static OSStatus
	CreateSOARecordData(
		const uint8_t *	inMName,
		const uint8_t *	inRName,
		uint32_t		inSerial,
		uint32_t		inRefresh,
		uint32_t		inRetry,
		uint32_t		inExpire,
		uint32_t		inMinimumTTL,
		uint8_t **		outPtr,
		size_t *		outLen );
static OSStatus
	_DataBuffer_AppendDNSQuestion(
		DataBuffer *	inDB,
		const uint8_t *	inNamePtr,
		size_t			inNameLen,
		uint16_t		inType,
		uint16_t		inClass );
static OSStatus
	_DataBuffer_AppendDNSRecord(
		DataBuffer *	inDB,
		const uint8_t *	inNamePtr,
		size_t			inNameLen,
		uint16_t		inType,
		uint16_t		inClass,
		uint32_t		inTTL,
		const uint8_t *	inRDataPtr,
		size_t			inRDataLen );
static char *	_NanoTime64ToTimestamp( NanoTime64 inTime, char *inBuf, size_t inMaxLen );

typedef struct MDNSInterfaceItem		MDNSInterfaceItem;
struct MDNSInterfaceItem
{
	MDNSInterfaceItem *		next;
	char *					ifName;
	uint32_t				ifIndex;
	Boolean					hasIPv4;
	Boolean					hasIPv6;
	Boolean					isAWDL;
	Boolean					isWiFi;
};

typedef enum
{
	kMDNSInterfaceSubset_All		= 0,	// All mDNS-capable interfaces.
	kMDNSInterfaceSubset_AWDL		= 1,	// All mDNS-capable AWDL interfaces.
	kMDNSInterfaceSubset_NonAWDL	= 2		// All mDNS-capable non-AWDL iterfaces.
	
}	MDNSInterfaceSubset;

static OSStatus	_MDNSInterfaceListCreate( MDNSInterfaceSubset inSubset, size_t inItemSize, MDNSInterfaceItem **outList );
static void		_MDNSInterfaceListFree( MDNSInterfaceItem *inList );
#define _MDNSInterfaceListForget( X )		ForgetCustom( X, _MDNSInterfaceListFree )
static OSStatus _MDNSInterfaceGetAny( MDNSInterfaceSubset inSubset, char inNameBuf[ IF_NAMESIZE + 1 ], uint32_t *outIndex );

static OSStatus	_SetComputerName( CFStringRef inComputerName, CFStringEncoding inEncoding );
static OSStatus	_SetComputerNameWithUTF8CString( const char *inComputerName );
static OSStatus	_SetLocalHostName( CFStringRef inLocalHostName );
static OSStatus	_SetLocalHostNameWithUTF8CString( const char *inLocalHostName );

static OSStatus	_SocketWriteAll( SocketRef inSock, const void *inData, size_t inSize, int32_t inTimeoutSecs );
static OSStatus
	_StringToIPv4Address(
		const char *			inStr,
		StringToIPAddressFlags	inFlags,
		uint32_t *				outIP,
		int *					outPort,
		uint32_t *				outSubnet,
		uint32_t *				outRouter,
		const char **			outStr );
static void	_StringArray_Free( char **inArray, size_t inCount );
static OSStatus
	_StringToIPv6Address(
		const char *			inStr,
		StringToIPAddressFlags	inFlags,
		uint8_t					outIPv6[ 16 ],
		uint32_t *				outScope,
		int *					outPort,
		int *					outPrefix,
		const char **			outStr );
static Boolean
	_ParseQuotedEscapedString(
		const char *	inSrc,
		const char *	inEnd,
		const char *	inDelimiters,
		char *			inBuf,
		size_t			inMaxLen,
		size_t *		outCopiedLen,
		size_t *		outTotalLen,
		const char **	outSrc );
static void *	_memdup( const void *inPtr, size_t inLen );
static int		_memicmp( const void *inP1, const void *inP2, size_t inLen );
static uint32_t	_FNV1( const void *inData, size_t inSize );

#define Unused( X )		(void)(X)

//===========================================================================================================================
//	MDNSCollider
//===========================================================================================================================

typedef struct MDNSColliderPrivate *		MDNSColliderRef;

typedef uint32_t		MDNSColliderProtocols;
#define kMDNSColliderProtocol_None		0
#define kMDNSColliderProtocol_IPv4		( 1 << 0 )
#define kMDNSColliderProtocol_IPv6		( 1 << 1 )

typedef void ( *MDNSColliderStopHandler_f )( void *inContext, OSStatus inError );

static OSStatus	MDNSColliderCreate( dispatch_queue_t inQueue, MDNSColliderRef *outCollider );
static OSStatus	MDNSColliderStart( MDNSColliderRef inCollider );
static void		MDNSColliderStop( MDNSColliderRef inCollider );
static void		MDNSColliderSetProtocols( MDNSColliderRef inCollider, MDNSColliderProtocols inProtocols );
static void		MDNSColliderSetInterfaceIndex( MDNSColliderRef inCollider, uint32_t inInterfaceIndex );
static OSStatus	MDNSColliderSetProgram( MDNSColliderRef inCollider, const char *inProgramStr );
static void
	MDNSColliderSetStopHandler(
		MDNSColliderRef				inCollider,
		MDNSColliderStopHandler_f	inStopHandler,
		void *						inStopContext );
static OSStatus
	MDNSColliderSetRecord(
		MDNSColliderRef	inCollider,
		const uint8_t *	inName,
		uint16_t		inType,
		const void *	inRDataPtr,
		size_t			inRDataLen );
static CFTypeID	MDNSColliderGetTypeID( void );

//===========================================================================================================================
//	ServiceBrowser
//===========================================================================================================================

typedef struct ServiceBrowserPrivate *		ServiceBrowserRef;
typedef struct ServiceBrowserResults		ServiceBrowserResults;
typedef struct SBRDomain					SBRDomain;
typedef struct SBRServiceType				SBRServiceType;
typedef struct SBRServiceInstance			SBRServiceInstance;
typedef struct SBRIPAddress					SBRIPAddress;

typedef void ( *ServiceBrowserCallback_f )( ServiceBrowserResults *inResults, OSStatus inError, void *inContext );

struct ServiceBrowserResults
{
	SBRDomain *		domainList;	// List of domains in which services were found.
};

struct SBRDomain
{
	SBRDomain *				next;		// Next domain in list.
	char *					name;		// Name of domain represented by this object.
	SBRServiceType *		typeList;	// List of service types in this domain.
};

struct SBRServiceType
{
	SBRServiceType *			next;			// Next service type in list.
	char *						name;			// Name of service type represented by this object.
	SBRServiceInstance *		instanceList;	// List of service instances of this service type.
};

struct SBRServiceInstance
{
	SBRServiceInstance *		next;			// Next service instance in list.
	char *						name;			// Name of service instance represented by this object.
	char *						hostname;		// Target from service instance's SRV record.
	uint32_t					ifIndex;		// Index of interface over which this service instance was discovered.
	uint16_t					port;			// Port from service instance's SRV record.
	uint8_t *					txtPtr;			// Service instance's TXT record data.
	size_t						txtLen;			// Service instance's TXT record data length.
	SBRIPAddress *				ipaddrList;		// List of IP addresses that the hostname resolved to.
	uint64_t					discoverTimeUs;	// Time it took to discover this service instance in microseconds.
	uint64_t					resolveTimeUs;	// Time it took to resolve this service instance in microseconds.
};

struct SBRIPAddress
{
	SBRIPAddress *		next;			// Next IP address in list.
	sockaddr_ip			sip;			// IPv4 or IPv6 address.
	uint64_t			resolveTimeUs;	// Time it took to resolve this IP address in microseconds.
};

static CFTypeID	ServiceBrowserGetTypeID( void );
static OSStatus
	ServiceBrowserCreate(
		dispatch_queue_t	inQueue,
		uint32_t			inInterfaceIndex,
		const char *		inDomain,
		unsigned int		inBrowseTimeSecs,
		Boolean				inIncludeAWDL,
		ServiceBrowserRef *	outBrowser );
static void		ServiceBrowserStart( ServiceBrowserRef inBrowser );
static OSStatus	ServiceBrowserAddServiceType( ServiceBrowserRef inBrowser, const char *inServiceType );
static void
	ServiceBrowserSetCallback(
		ServiceBrowserRef			inBrowser,
		ServiceBrowserCallback_f	inCallback,
		void *						inContext );
static void		ServiceBrowserResultsRetain( ServiceBrowserResults *inResults );
static void		ServiceBrowserResultsRelease( ServiceBrowserResults *inResults );

#define ForgetServiceBrowserResults( X )		ForgetCustom( X, ServiceBrowserResultsRelease )

//===========================================================================================================================
//	main
//===========================================================================================================================

#define _PRINTF_EXTENSION_HANDLER_DECLARE( NAME )	\
	static int										\
		_PrintFExtension ## NAME ## Handler(		\
			PrintFContext *	inContext,				\
			PrintFFormat *	inFormat,				\
			PrintFVAList *	inArgs,					\
			void *			inUserContext )

_PRINTF_EXTENSION_HANDLER_DECLARE( Timestamp );
_PRINTF_EXTENSION_HANDLER_DECLARE( DNSMessage );
_PRINTF_EXTENSION_HANDLER_DECLARE( CallbackFlags );
_PRINTF_EXTENSION_HANDLER_DECLARE( DNSRecordData );

int	main( int argc, const char **argv )
{
	OSStatus		err;
	
	// Route DebugServices logging output to stderr.
	
	dlog_control( "DebugServices:output=file;stderr" );
	
	PrintFRegisterExtension( "du:time",    _PrintFExtensionTimestampHandler,     NULL );
	PrintFRegisterExtension( "du:dnsmsg",  _PrintFExtensionDNSMessageHandler,    NULL );
	PrintFRegisterExtension( "du:cbflags", _PrintFExtensionCallbackFlagsHandler, NULL );
	PrintFRegisterExtension( "du:rdata",   _PrintFExtensionDNSRecordDataHandler, NULL );
	CLIInit( argc, argv );
	err = CLIParse( kGlobalOpts, kCLIFlags_None );
	if( err ) exit( 1 );
	
	return( gExitCode );
}

//===========================================================================================================================
//	VersionOptionCallback
//===========================================================================================================================

static OSStatus	VersionOptionCallback( CLIOption *inOption, const char *inArg, int inUnset )
{
	const char *		srcVers;
#if( MDNSRESPONDER_PROJECT )
	char				srcStr[ 16 ];
#endif
	
	Unused( inOption );
	Unused( inArg );
	Unused( inUnset );
	
#if( MDNSRESPONDER_PROJECT )
	srcVers = SourceVersionToCString( _DNS_SD_H, srcStr );
#else
	srcVers = DNSSDUTIL_SOURCE_VERSION;
#endif
	FPrintF( stdout, "%s version %v (%s)\n", gProgramName, kDNSSDUtilNumVersion, srcVers );
	
	return( kEndingErr );
}

//===========================================================================================================================
//	BrowseCmd
//===========================================================================================================================

typedef struct BrowseResolveOp		BrowseResolveOp;

struct BrowseResolveOp
{
	BrowseResolveOp *		next;			// Next resolve operation in list.
	DNSServiceRef			sdRef;			// sdRef of the DNSServiceResolve or DNSServiceQueryRecord operation.
	char *					fullName;		// Full name of the service to resolve.
	uint32_t				interfaceIndex;	// Interface index of the DNSServiceResolve or DNSServiceQueryRecord operation.
};

typedef struct
{
	DNSServiceRef			mainRef;			// Main sdRef for shared connection.
	DNSServiceRef *			opRefs;				// Array of sdRefs for individual Browse operarions.
	size_t					opRefsCount;		// Count of array of sdRefs for non-shared connections.
	const char *			domain;				// Domain for DNSServiceBrowse operation(s).
	DNSServiceFlags			flags;				// Flags for DNSServiceBrowse operation(s).
	char **					serviceTypes;		// Array of service types to browse for.
	size_t					serviceTypesCount;	// Count of array of service types to browse for.
	int						timeLimitSecs;		// Time limit of DNSServiceBrowse operation in seconds.
	BrowseResolveOp *		resolveList;		// List of resolve and/or TXT record query operations.
	uint32_t				ifIndex;			// Interface index of DNSServiceBrowse operation(s).
	Boolean					printedHeader;		// True if results header has been printed.
	Boolean					doResolve;			// True if service instances are to be resolved.
	Boolean					doResolveTXTOnly;	// True if TXT records of service instances are to be queried.
	
}	BrowseContext;

static void		BrowsePrintPrologue( const BrowseContext *inContext );
static void		BrowseContextFree( BrowseContext *inContext );
static OSStatus	BrowseResolveOpCreate( const char *inFullName, uint32_t inInterfaceIndex, BrowseResolveOp **outOp );
static void		BrowseResolveOpFree( BrowseResolveOp *inOp );
static void DNSSD_API
	BrowseCallback(
		DNSServiceRef		inSDRef,
		DNSServiceFlags		inFlags,
		uint32_t			inInterfaceIndex,
		DNSServiceErrorType	inError,
		const char *		inName,
		const char *		inRegType,
		const char *		inDomain,
		void *				inContext );
static void DNSSD_API
	BrowseResolveCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		const char *			inHostname,
		uint16_t				inPort,
		uint16_t				inTXTLen,
		const unsigned char *	inTXTPtr,
		void *					inContext );
static void DNSSD_API
	BrowseQueryRecordCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		uint16_t				inType,
		uint16_t				inClass,
		uint16_t				inRDataLen,
		const void *			inRDataPtr,
		uint32_t				inTTL,
		void *					inContext );

static void	BrowseCmd( void )
{
	OSStatus				err;
	size_t					i;
	BrowseContext *			context			= NULL;
	dispatch_source_t		signalSource	= NULL;
	int						useMainConnection;
	
	// Set up SIGINT handler.
	
	signal( SIGINT, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGINT, dispatch_get_main_queue(), Exit, kExitReason_SIGINT, &signalSource );
	require_noerr( err, exit );
	dispatch_resume( signalSource );
	
	// Create context.
	
	context = (BrowseContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	context->opRefs = (DNSServiceRef *) calloc( gBrowse_ServiceTypesCount, sizeof( DNSServiceRef ) );
	require_action( context->opRefs, exit, err = kNoMemoryErr );
	context->opRefsCount = gBrowse_ServiceTypesCount;
	
	// Check command parameters.
	
	if( gBrowse_TimeLimitSecs < 0 )
	{
		FPrintF( stderr, "Invalid time limit: %d seconds.\n", gBrowse_TimeLimitSecs );
		err = kParamErr;
		goto exit;
	}
	
	// Create main connection.
	
	if( gConnectionOpt )
	{
		err = CreateConnectionFromArgString( gConnectionOpt, dispatch_get_main_queue(), &context->mainRef, NULL );
		require_noerr_quiet( err, exit );
		useMainConnection = true;
	}
	else
	{
		useMainConnection = false;
	}
	
	// Get flags.
	
	context->flags = GetDNSSDFlagsFromOpts();
	if( useMainConnection ) context->flags |= kDNSServiceFlagsShareConnection;
	
	// Get interface.
	
	err = InterfaceIndexFromArgString( gInterface, &context->ifIndex );
	require_noerr_quiet( err, exit );
	
	// Set remaining parameters.
	
	context->serviceTypes		= gBrowse_ServiceTypes;
	context->serviceTypesCount	= gBrowse_ServiceTypesCount;
	context->domain				= gBrowse_Domain;
	context->doResolve			= gBrowse_DoResolve	? true : false;
	context->timeLimitSecs		= gBrowse_TimeLimitSecs;
	context->doResolveTXTOnly	= gBrowse_QueryTXT	? true : false;
	
	// Print prologue.
	
	BrowsePrintPrologue( context );
	
	// Start operation(s).
	
	for( i = 0; i < context->serviceTypesCount; ++i )
	{
		DNSServiceRef		sdRef;
		
		sdRef = useMainConnection ? context->mainRef : kBadDNSServiceRef;
		err = DNSServiceBrowse( &sdRef, context->flags, context->ifIndex, context->serviceTypes[ i ], context->domain,
			BrowseCallback, context );
		require_noerr( err, exit );
		
		context->opRefs[ i ] = sdRef;
		if( !useMainConnection )
		{
			err = DNSServiceSetDispatchQueue( context->opRefs[ i ], dispatch_get_main_queue() );
			require_noerr( err, exit );
		}
	}
	
	// Set time limit.
	
	if( context->timeLimitSecs > 0 )
	{
		dispatch_after_f( dispatch_time_seconds( context->timeLimitSecs ), dispatch_get_main_queue(),
			kExitReason_TimeLimit, Exit );
	}
	dispatch_main();
	
exit:
	dispatch_source_forget( &signalSource );
	if( context ) BrowseContextFree( context );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	BrowsePrintPrologue
//===========================================================================================================================

static void	BrowsePrintPrologue( const BrowseContext *inContext )
{
	const int						timeLimitSecs	= inContext->timeLimitSecs;
	const char * const *			ptr				= (const char **) inContext->serviceTypes;
	const char * const * const		end				= (const char **) inContext->serviceTypes + inContext->serviceTypesCount;
	char							ifName[ kInterfaceNameBufLen ];
	
	InterfaceIndexToName( inContext->ifIndex, ifName );
	
	FPrintF( stdout, "Flags:         %#{flags}\n",	inContext->flags, kDNSServiceFlagsDescriptors );
	FPrintF( stdout, "Interface:     %d (%s)\n",	(int32_t) inContext->ifIndex, ifName );
	FPrintF( stdout, "Service types: %s",			*ptr++ );
	while( ptr < end ) FPrintF( stdout, ", %s",		*ptr++ );
	FPrintF( stdout, "\n" );
	FPrintF( stdout, "Domain:        %s\n",	inContext->domain ? inContext->domain : "<NULL> (default domains)" );
	FPrintF( stdout, "Time limit:    " );
	if( timeLimitSecs > 0 )	FPrintF( stdout, "%d second%?c\n", timeLimitSecs, timeLimitSecs != 1, 's' );
	else					FPrintF( stdout, "∞\n" );
	FPrintF( stdout, "Start time:    %{du:time}\n", NULL );
	FPrintF( stdout, "---\n" );
}

//===========================================================================================================================
//	BrowseContextFree
//===========================================================================================================================

static void	BrowseContextFree( BrowseContext *inContext )
{
	size_t		i;
	
	for( i = 0; i < inContext->opRefsCount; ++i )
	{
		DNSServiceForget( &inContext->opRefs[ i ] );
	}
	if( inContext->serviceTypes )
	{
		_StringArray_Free( inContext->serviceTypes, inContext->serviceTypesCount );
		inContext->serviceTypes			= NULL;
		inContext->serviceTypesCount	= 0;
	}
	DNSServiceForget( &inContext->mainRef );
	free( inContext );
}

//===========================================================================================================================
//	BrowseResolveOpCreate
//===========================================================================================================================

static OSStatus	BrowseResolveOpCreate( const char *inFullName, uint32_t inInterfaceIndex, BrowseResolveOp **outOp )
{
	OSStatus				err;
	BrowseResolveOp *		resolveOp;
	
	resolveOp = (BrowseResolveOp *) calloc( 1, sizeof( *resolveOp ) );
	require_action( resolveOp, exit, err = kNoMemoryErr );
	
	resolveOp->fullName = strdup( inFullName );
	require_action( resolveOp->fullName, exit, err = kNoMemoryErr );
	
	resolveOp->interfaceIndex = inInterfaceIndex;
	
	*outOp = resolveOp;
	resolveOp = NULL;
	err = kNoErr;
	
exit:
	if( resolveOp ) BrowseResolveOpFree( resolveOp );
	return( err );
}

//===========================================================================================================================
//	BrowseResolveOpFree
//===========================================================================================================================

static void	BrowseResolveOpFree( BrowseResolveOp *inOp )
{
	DNSServiceForget( &inOp->sdRef );
	ForgetMem( &inOp->fullName );
	free( inOp );
}

//===========================================================================================================================
//	BrowseCallback
//===========================================================================================================================

static void DNSSD_API
	BrowseCallback(
		DNSServiceRef		inSDRef,
		DNSServiceFlags		inFlags,
		uint32_t			inInterfaceIndex,
		DNSServiceErrorType	inError,
		const char *		inName,
		const char *		inRegType,
		const char *		inDomain,
		void *				inContext )
{
	BrowseContext * const		context = (BrowseContext *) inContext;
	OSStatus					err;
	BrowseResolveOp *			newOp = NULL;
	BrowseResolveOp **			p;
	char						fullName[ kDNSServiceMaxDomainName ];
	struct timeval				now;
	
	Unused( inSDRef );
	
	gettimeofday( &now, NULL );
	
	err = inError;
	require_noerr( err, exit );
	
	if( !context->printedHeader )
	{
		FPrintF( stdout, "%-26s  %-16s IF %-20s %-20s Instance Name\n", "Timestamp", "Flags", "Domain", "Service Type" );
		context->printedHeader = true;
	}
	FPrintF( stdout, "%{du:time}  %{du:cbflags} %2d %-20s %-20s %s\n",
		&now, inFlags, (int32_t) inInterfaceIndex, inDomain, inRegType, inName );
	
	if( !context->doResolve && !context->doResolveTXTOnly ) goto exit;
	
	err = DNSServiceConstructFullName( fullName, inName, inRegType, inDomain );
	require_noerr( err, exit );
	
	if( inFlags & kDNSServiceFlagsAdd )
	{
		DNSServiceRef		sdRef;
		DNSServiceFlags		flags;
		
		err = BrowseResolveOpCreate( fullName, inInterfaceIndex, &newOp );
		require_noerr( err, exit );
		
		if( context->mainRef )
		{
			sdRef = context->mainRef;
			flags = kDNSServiceFlagsShareConnection;
		}
		else
		{
			flags = 0;
		}
		if( context->doResolve )
		{
			err = DNSServiceResolve( &sdRef, flags, inInterfaceIndex, inName, inRegType, inDomain, BrowseResolveCallback,
				NULL );
			require_noerr( err, exit );
		}
		else
		{
			err = DNSServiceQueryRecord( &sdRef, flags, inInterfaceIndex, fullName, kDNSServiceType_TXT, kDNSServiceClass_IN,
				BrowseQueryRecordCallback, NULL );
			require_noerr( err, exit );
		}
		
		newOp->sdRef = sdRef;
		if( !context->mainRef )
		{
			err = DNSServiceSetDispatchQueue( newOp->sdRef, dispatch_get_main_queue() );
			require_noerr( err, exit );
		}
		for( p = &context->resolveList; *p; p = &( *p )->next ) {}
		*p = newOp;
		newOp = NULL;
	}
	else
	{
		BrowseResolveOp *		resolveOp;
		
		for( p = &context->resolveList; ( resolveOp = *p ) != NULL; p = &resolveOp->next )
		{
			if( ( resolveOp->interfaceIndex == inInterfaceIndex ) && ( strcasecmp( resolveOp->fullName, fullName ) == 0 ) )
			{
				break;
			}
		}
		if( resolveOp )
		{
			*p = resolveOp->next;
			BrowseResolveOpFree( resolveOp );
		}
	}
	
exit:
	if( newOp ) BrowseResolveOpFree( newOp );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	BrowseQueryRecordCallback
//===========================================================================================================================

static void DNSSD_API
	BrowseQueryRecordCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		uint16_t				inType,
		uint16_t				inClass,
		uint16_t				inRDataLen,
		const void *			inRDataPtr,
		uint32_t				inTTL,
		void *					inContext )
{
	OSStatus			err;
	struct timeval		now;
	
	Unused( inSDRef );
	Unused( inClass );
	Unused( inTTL );
	Unused( inContext );
	
	gettimeofday( &now, NULL );
	
	err = inError;
	require_noerr( err, exit );
	require_action( inType == kDNSServiceType_TXT, exit, err = kTypeErr );
	
	FPrintF( stdout, "%{du:time}  %s %s TXT on interface %d\n    TXT: %#{txt}\n",
		&now, DNSServiceFlagsToAddRmvStr( inFlags ), inFullName, (int32_t) inInterfaceIndex, inRDataPtr,
		(size_t) inRDataLen );
	
exit:
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	BrowseResolveCallback
//===========================================================================================================================

static void DNSSD_API
	BrowseResolveCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		const char *			inHostname,
		uint16_t				inPort,
		uint16_t				inTXTLen,
		const unsigned char *	inTXTPtr,
		void *					inContext )
{
	struct timeval		now;
	char				errorStr[ 64 ];
	
	Unused( inSDRef );
	Unused( inFlags );
	Unused( inContext );
	
	gettimeofday( &now, NULL );
	
	if( inError ) SNPrintF( errorStr, sizeof( errorStr ), " error %#m", inError );
	
	FPrintF( stdout, "%{du:time}  %s can be reached at %s:%u (interface %d)%?s\n",
		&now, inFullName, inHostname, ntohs( inPort ), (int32_t) inInterfaceIndex, inError, errorStr );
	if( inTXTLen == 1 )
	{
		FPrintF( stdout, " TXT record: %#H\n", inTXTPtr, (int) inTXTLen, INT_MAX );
	}
	else
	{
		FPrintF( stdout, " TXT record: %#{txt}\n", inTXTPtr, (size_t) inTXTLen );
	}
}

//===========================================================================================================================
//	GetAddrInfoCmd
//===========================================================================================================================

typedef struct
{
	DNSServiceRef			mainRef;		// Main sdRef for shared connection.
	DNSServiceRef			opRef;			// sdRef for the DNSServiceGetAddrInfo operation.
	const char *			name;			// Hostname to resolve.
	DNSServiceFlags			flags;			// Flags argument for DNSServiceGetAddrInfo().
	DNSServiceProtocol		protocols;		// Protocols argument for DNSServiceGetAddrInfo().
	uint32_t				ifIndex;		// Interface index argument for DNSServiceGetAddrInfo().
	int						timeLimitSecs;	// Time limit for the DNSServiceGetAddrInfo() operation in seconds.
	Boolean					printedHeader;	// True if the results header has been printed.
	Boolean					oneShotMode;	// True if command is done after the first set of results (one-shot mode).
	Boolean					needIPv4;		// True if in one-shot mode and an IPv4 result is needed.
	Boolean					needIPv6;		// True if in one-shot mode and an IPv6 result is needed.
	
}	GetAddrInfoContext;

static void	GetAddrInfoPrintPrologue( const GetAddrInfoContext *inContext );
static void	GetAddrInfoContextFree( GetAddrInfoContext *inContext );
static void DNSSD_API
	GetAddrInfoCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inHostname,
		const struct sockaddr *	inSockAddr,
		uint32_t				inTTL,
		void *					inContext );

static void	GetAddrInfoCmd( void )
{
	OSStatus					err;
	DNSServiceRef				sdRef;
	GetAddrInfoContext *		context			= NULL;
	dispatch_source_t			signalSource	= NULL;
	int							useMainConnection;
	
	// Set up SIGINT handler.
	
	signal( SIGINT, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGINT, dispatch_get_main_queue(), Exit, kExitReason_SIGINT, &signalSource );
	require_noerr( err, exit );
	dispatch_resume( signalSource );
	
	// Check command parameters.
	
	if( gGetAddrInfo_TimeLimitSecs < 0 )
	{
		FPrintF( stderr, "Invalid time limit: %d s.\n", gGetAddrInfo_TimeLimitSecs );
		err = kParamErr;
		goto exit;
	}
	
	// Create context.
	
	context = (GetAddrInfoContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	// Create main connection.
	
	if( gConnectionOpt )
	{
		err = CreateConnectionFromArgString( gConnectionOpt, dispatch_get_main_queue(), &context->mainRef, NULL );
		require_noerr_quiet( err, exit );
		useMainConnection = true;
	}
	else
	{
		useMainConnection = false;
	}
	
	// Get flags.
	
	context->flags = GetDNSSDFlagsFromOpts();
	if( useMainConnection ) context->flags |= kDNSServiceFlagsShareConnection;
	
	// Get interface.
	
	err = InterfaceIndexFromArgString( gInterface, &context->ifIndex );
	require_noerr_quiet( err, exit );
	
	// Set remaining parameters.
	
	context->name			= gGetAddrInfo_Name;
	context->timeLimitSecs	= gGetAddrInfo_TimeLimitSecs;
	if( gGetAddrInfo_ProtocolIPv4 ) context->protocols |= kDNSServiceProtocol_IPv4;
	if( gGetAddrInfo_ProtocolIPv6 ) context->protocols |= kDNSServiceProtocol_IPv6;
	if( gGetAddrInfo_OneShot )
	{
		context->oneShotMode	= true;
		context->needIPv4		= ( gGetAddrInfo_ProtocolIPv4 || !gGetAddrInfo_ProtocolIPv6 ) ? true : false;
		context->needIPv6		= ( gGetAddrInfo_ProtocolIPv6 || !gGetAddrInfo_ProtocolIPv4 ) ? true : false;
	}
	
	// Print prologue.
	
	GetAddrInfoPrintPrologue( context );
	
	// Start operation.
	
	sdRef = useMainConnection ? context->mainRef : kBadDNSServiceRef;
	err = DNSServiceGetAddrInfo( &sdRef, context->flags, context->ifIndex, context->protocols, context->name,
		GetAddrInfoCallback, context );
	require_noerr( err, exit );
	
	context->opRef = sdRef;
	if( !useMainConnection )
	{
		err = DNSServiceSetDispatchQueue( context->opRef, dispatch_get_main_queue() );
		require_noerr( err, exit );
	}
	
	// Set time limit.
	
	if( context->timeLimitSecs > 0 )
	{
		dispatch_after_f( dispatch_time_seconds( context->timeLimitSecs ), dispatch_get_main_queue(),
			kExitReason_TimeLimit, Exit );
	}
	dispatch_main();
	
exit:
	dispatch_source_forget( &signalSource );
	if( context ) GetAddrInfoContextFree( context );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	GetAddrInfoPrintPrologue
//===========================================================================================================================

static void	GetAddrInfoPrintPrologue( const GetAddrInfoContext *inContext )
{
	const int		timeLimitSecs = inContext->timeLimitSecs;
	char			ifName[ kInterfaceNameBufLen ];
	
	InterfaceIndexToName( inContext->ifIndex, ifName );
	
	FPrintF( stdout, "Flags:      %#{flags}\n",		inContext->flags, kDNSServiceFlagsDescriptors );
	FPrintF( stdout, "Interface:  %d (%s)\n",		(int32_t) inContext->ifIndex, ifName );
	FPrintF( stdout, "Protocols:  %#{flags}\n",		inContext->protocols, kDNSServiceProtocolDescriptors );
	FPrintF( stdout, "Name:       %s\n",			inContext->name );
	FPrintF( stdout, "Mode:       %s\n",			inContext->oneShotMode ? "one-shot" : "continuous" );
	FPrintF( stdout, "Time limit: " );
	if( timeLimitSecs > 0 )	FPrintF( stdout, "%d second%?c\n", timeLimitSecs, timeLimitSecs != 1, 's' );
	else					FPrintF( stdout, "∞\n" );
	FPrintF( stdout, "Start time: %{du:time}\n",	NULL );
	FPrintF( stdout, "---\n" );
}

//===========================================================================================================================
//	GetAddrInfoContextFree
//===========================================================================================================================

static void	GetAddrInfoContextFree( GetAddrInfoContext *inContext )
{
	DNSServiceForget( &inContext->opRef );
	DNSServiceForget( &inContext->mainRef );
	free( inContext );
}

//===========================================================================================================================
//	GetAddrInfoCallback
//===========================================================================================================================

static void DNSSD_API
	GetAddrInfoCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inHostname,
		const struct sockaddr *	inSockAddr,
		uint32_t				inTTL,
		void *					inContext )
{
	GetAddrInfoContext * const		context = (GetAddrInfoContext *) inContext;
	struct timeval					now;
	OSStatus						err;
	const char *					addrStr;
	char							addrStrBuf[ kSockAddrStringMaxSize ];
	
	Unused( inSDRef );
	
	gettimeofday( &now, NULL );
	
	switch( inError )
	{
		case kDNSServiceErr_NoError:
		case kDNSServiceErr_NoSuchRecord:
			err = kNoErr;
			break;
		
		case kDNSServiceErr_Timeout:
			Exit( kExitReason_Timeout );
		
		default:
			err = inError;
			goto exit;
	}
	
	if( ( inSockAddr->sa_family != AF_INET ) && ( inSockAddr->sa_family != AF_INET6 ) )
	{
		dlogassert( "Unexpected address family: %d", inSockAddr->sa_family );
		err = kTypeErr;
		goto exit;
	}
	
	if( !inError )
	{
		err = SockAddrToString( inSockAddr, kSockAddrStringFlagsNone, addrStrBuf );
		require_noerr( err, exit );
		addrStr = addrStrBuf;
	}
	else
	{
		addrStr = ( inSockAddr->sa_family == AF_INET ) ? kNoSuchRecordAStr : kNoSuchRecordAAAAStr;
	}
	
	if( !context->printedHeader )
	{
		FPrintF( stdout, "%-26s  %-16s IF %-30s %-34s %6s\n", "Timestamp", "Flags", "Hostname", "Address", "TTL" );
		context->printedHeader = true;
	}
	FPrintF( stdout, "%{du:time}  %{du:cbflags} %2d %-30s %-34s %6u\n",
		&now, inFlags, (int32_t) inInterfaceIndex, inHostname, addrStr, inTTL );
	
	if( context->oneShotMode )
	{
		if( inFlags & kDNSServiceFlagsAdd )
		{
			if( inSockAddr->sa_family == AF_INET )	context->needIPv4 = false;
			else									context->needIPv6 = false;
		}
		if( !( inFlags & kDNSServiceFlagsMoreComing ) && !context->needIPv4 && !context->needIPv6 )
		{
			Exit( kExitReason_OneShotDone );
		}
	}
	
exit:
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	QueryRecordCmd
//===========================================================================================================================

typedef struct
{
	DNSServiceRef		mainRef;		// Main sdRef for shared connection.
	DNSServiceRef		opRef;			// sdRef for the DNSServiceQueryRecord operation.
	const char *		recordName;		// Resource record name argument for DNSServiceQueryRecord().
	DNSServiceFlags		flags;			// Flags argument for DNSServiceQueryRecord().
	uint32_t			ifIndex;		// Interface index argument for DNSServiceQueryRecord().
	int					timeLimitSecs;	// Time limit for the DNSServiceQueryRecord() operation in seconds.
	uint16_t			recordType;		// Resource record type argument for DNSServiceQueryRecord().
	Boolean				printedHeader;	// True if the results header was printed.
	Boolean				oneShotMode;	// True if command is done after the first set of results (one-shot mode).
	Boolean				gotRecord;		// True if in one-shot mode and received at least one record of the desired type.
	Boolean				printRawRData;	// True if RDATA results are not to be formatted when printed.
	
}	QueryRecordContext;

static void	QueryRecordPrintPrologue( const QueryRecordContext *inContext );
static void	QueryRecordContextFree( QueryRecordContext *inContext );
static void DNSSD_API
	QueryRecordCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		uint16_t				inType,
		uint16_t				inClass,
		uint16_t				inRDataLen,
		const void *			inRDataPtr,
		uint32_t				inTTL,
		void *					inContext );

static void	QueryRecordCmd( void )
{
	OSStatus					err;
	DNSServiceRef				sdRef;
	QueryRecordContext *		context			= NULL;
	dispatch_source_t			signalSource	= NULL;
	int							useMainConnection;
	
	// Set up SIGINT handler.
	
	signal( SIGINT, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGINT, dispatch_get_main_queue(), Exit, kExitReason_SIGINT, &signalSource );
	require_noerr( err, exit );
	dispatch_resume( signalSource );
	
	// Create context.
	
	context = (QueryRecordContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	// Check command parameters.
	
	if( gQueryRecord_TimeLimitSecs < 0 )
	{
		FPrintF( stderr, "Invalid time limit: %d seconds.\n", gQueryRecord_TimeLimitSecs );
		err = kParamErr;
		goto exit;
	}
	
	// Create main connection.
	
	if( gConnectionOpt )
	{
		err = CreateConnectionFromArgString( gConnectionOpt, dispatch_get_main_queue(), &context->mainRef, NULL );
		require_noerr_quiet( err, exit );
		useMainConnection = true;
	}
	else
	{
		useMainConnection = false;
	}
	
	// Get flags.
	
	context->flags = GetDNSSDFlagsFromOpts();
	if( useMainConnection ) context->flags |= kDNSServiceFlagsShareConnection;
	
	// Get interface.
	
	err = InterfaceIndexFromArgString( gInterface, &context->ifIndex );
	require_noerr_quiet( err, exit );
	
	// Get record type.
	
	err = RecordTypeFromArgString( gQueryRecord_Type, &context->recordType );
	require_noerr( err, exit );
	
	// Set remaining parameters.
	
	context->recordName		= gQueryRecord_Name;
	context->timeLimitSecs	= gQueryRecord_TimeLimitSecs;
	context->oneShotMode	= gQueryRecord_OneShot	? true : false;
	context->printRawRData	= gQueryRecord_RawRData	? true : false;
	
	// Print prologue.
	
	QueryRecordPrintPrologue( context );
	
	// Start operation.
	
	sdRef = useMainConnection ? context->mainRef : kBadDNSServiceRef;
	err = DNSServiceQueryRecord( &sdRef, context->flags, context->ifIndex, context->recordName, context->recordType,
		kDNSServiceClass_IN, QueryRecordCallback, context );
	require_noerr( err, exit );
	
	context->opRef = sdRef;
	if( !useMainConnection )
	{
		err = DNSServiceSetDispatchQueue( context->opRef, dispatch_get_main_queue() );
		require_noerr( err, exit );
	}
	
	// Set time limit.
	
	if( context->timeLimitSecs > 0 )
	{
		dispatch_after_f( dispatch_time_seconds( context->timeLimitSecs ), dispatch_get_main_queue(), kExitReason_TimeLimit,
			Exit );
	}
	dispatch_main();
	
exit:
	dispatch_source_forget( &signalSource );
	if( context ) QueryRecordContextFree( context );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	QueryRecordContextFree
//===========================================================================================================================

static void	QueryRecordContextFree( QueryRecordContext *inContext )
{
	DNSServiceForget( &inContext->opRef );
	DNSServiceForget( &inContext->mainRef );
	free( inContext );
}

//===========================================================================================================================
//	QueryRecordPrintPrologue
//===========================================================================================================================

static void	QueryRecordPrintPrologue( const QueryRecordContext *inContext )
{
	const int		timeLimitSecs = inContext->timeLimitSecs;
	char			ifName[ kInterfaceNameBufLen ];
	
	InterfaceIndexToName( inContext->ifIndex, ifName );
	
	FPrintF( stdout, "Flags:       %#{flags}\n",	inContext->flags, kDNSServiceFlagsDescriptors );
	FPrintF( stdout, "Interface:   %d (%s)\n",		(int32_t) inContext->ifIndex, ifName );
	FPrintF( stdout, "Name:        %s\n",			inContext->recordName );
	FPrintF( stdout, "Type:        %s (%u)\n",		RecordTypeToString( inContext->recordType ), inContext->recordType );
	FPrintF( stdout, "Mode:        %s\n",			inContext->oneShotMode ? "one-shot" : "continuous" );
	FPrintF( stdout, "Time limit:  " );
	if( timeLimitSecs > 0 )	FPrintF( stdout, "%d second%?c\n", timeLimitSecs, timeLimitSecs != 1, 's' );
	else					FPrintF( stdout, "∞\n" );
	FPrintF( stdout, "Start time:  %{du:time}\n",	NULL );
	FPrintF( stdout, "---\n" );
	
}

//===========================================================================================================================
//	QueryRecordCallback
//===========================================================================================================================

static void DNSSD_API
	QueryRecordCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		uint16_t				inType,
		uint16_t				inClass,
		uint16_t				inRDataLen,
		const void *			inRDataPtr,
		uint32_t				inTTL,
		void *					inContext )
{
	QueryRecordContext * const		context		= (QueryRecordContext *) inContext;
	struct timeval					now;
	OSStatus						err;
	char *							rdataStr	= NULL;
	
	Unused( inSDRef );
	
	gettimeofday( &now, NULL );
	
	switch( inError )
	{
		case kDNSServiceErr_NoError:
		case kDNSServiceErr_NoSuchRecord:
			err = kNoErr;
			break;
		
		case kDNSServiceErr_Timeout:
			Exit( kExitReason_Timeout );
		
		default:
			err = inError;
			goto exit;
	}
	
	if( inError != kDNSServiceErr_NoSuchRecord )
	{
		if( !context->printRawRData ) DNSRecordDataToString( inRDataPtr, inRDataLen, inType, NULL, 0, &rdataStr );
		if( !rdataStr )
		{
			ASPrintF( &rdataStr, "%#H", inRDataPtr, inRDataLen, INT_MAX );
			require_action( rdataStr, exit, err = kNoMemoryErr );
		}
	}
	
	if( !context->printedHeader )
	{
		FPrintF( stdout, "%-26s  %-16s IF %-32s %-5s %-5s %6s RData\n",
			"Timestamp", "Flags", "Name", "Type", "Class", "TTL" );
		context->printedHeader = true;
	}
	FPrintF( stdout, "%{du:time}  %{du:cbflags} %2d %-32s %-5s %?-5s%?5u %6u %s\n",
		&now, inFlags, (int32_t) inInterfaceIndex, inFullName, RecordTypeToString( inType ),
		( inClass == kDNSServiceClass_IN ), "IN", ( inClass != kDNSServiceClass_IN ), inClass, inTTL,
		rdataStr ? rdataStr : kNoSuchRecordStr );
	
	if( context->oneShotMode )
	{
		if( ( inFlags & kDNSServiceFlagsAdd ) &&
			( ( context->recordType == kDNSServiceType_ANY ) || ( context->recordType == inType ) ) )
		{
			context->gotRecord = true;
		}
		if( !( inFlags & kDNSServiceFlagsMoreComing ) && context->gotRecord ) Exit( kExitReason_OneShotDone );
	}
	
exit:
	FreeNullSafe( rdataStr );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	RegisterCmd
//===========================================================================================================================

typedef struct
{
	DNSRecordRef		recordRef;	// Reference returned by DNSServiceAddRecord().
	uint8_t *			dataPtr;	// Record data.
	size_t				dataLen;	// Record data length.
	uint32_t			ttl;		// Record TTL value.
	uint16_t			type;		// Record type.
	
}	ExtraRecord;

typedef struct
{
	DNSServiceRef		opRef;				// sdRef for DNSServiceRegister operation.
	const char *		name;				// Service name argument for DNSServiceRegister().
	const char *		type;				// Service type argument for DNSServiceRegister().
	const char *		domain;				// Domain in which advertise the service.
	uint8_t *			txtPtr;				// Service TXT record data. (malloc'd)
	size_t				txtLen;				// Service TXT record data len.
	ExtraRecord *		extraRecords;		// Array of extra records to add to registered service.
	size_t				extraRecordsCount;	// Number of extra records.
	uint8_t *			updateTXTPtr;		// Pointer to record data for TXT record update. (malloc'd)
	size_t				updateTXTLen;		// Length of record data for TXT record update.
	uint32_t			updateTTL;			// TTL of updated TXT record.
	int					updateDelayMs;		// Post-registration TXT record update delay in milliseconds.
	DNSServiceFlags		flags;				// Flags argument for DNSServiceRegister().
	uint32_t			ifIndex;			// Interface index argument for DNSServiceRegister().
	int					lifetimeMs;			// Lifetime of the record registration in milliseconds.
	uint16_t			port;				// Service instance's port number.
	Boolean				printedHeader;		// True if results header was printed.
	Boolean				didRegister;		// True if service was registered.
	
}	RegisterContext;

static void	RegisterPrintPrologue( const RegisterContext *inContext );
static void	RegisterContextFree( RegisterContext *inContext );
static void DNSSD_API
	RegisterCallback(
		DNSServiceRef		inSDRef,
		DNSServiceFlags		inFlags,
		DNSServiceErrorType	inError,
		const char *		inName,
		const char *		inType,
		const char *		inDomain,
		void *				inContext );
static void	RegisterUpdate( void *inContext );

static void	RegisterCmd( void )
{
	OSStatus				err;
	RegisterContext *		context			= NULL;
	dispatch_source_t		signalSource	= NULL;
	
	// Set up SIGINT handler.
	
	signal( SIGINT, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGINT, dispatch_get_main_queue(), Exit, kExitReason_SIGINT, &signalSource );
	require_noerr( err, exit );
	dispatch_resume( signalSource );
	
	// Create context.
	
	context = (RegisterContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	// Check command parameters.
	
	if( ( gRegister_Port < 0 ) || ( gRegister_Port > UINT16_MAX ) )
	{
		FPrintF( stderr, "Port number %d is out-of-range.\n", gRegister_Port );
		err = kParamErr;
		goto exit;
	}
	
	if( ( gAddRecord_DataCount != gAddRecord_TypesCount ) || ( gAddRecord_TTLsCount != gAddRecord_TypesCount ) )
	{
		FPrintF( stderr, "There are missing additional record parameters.\n" );
		err = kParamErr;
		goto exit;
	}
	
	// Get flags.
	
	context->flags = GetDNSSDFlagsFromOpts();
	
	// Get interface index.
	
	err = InterfaceIndexFromArgString( gInterface, &context->ifIndex );
	require_noerr_quiet( err, exit );
	
	// Get TXT record data.
	
	if( gRegister_TXT )
	{
		err = RecordDataFromArgString( gRegister_TXT, &context->txtPtr, &context->txtLen );
		require_noerr_quiet( err, exit );
	}
	
	// Set remaining parameters.
	
	context->name		= gRegister_Name;
	context->type		= gRegister_Type;
	context->domain		= gRegister_Domain;
	context->port		= (uint16_t) gRegister_Port;
	context->lifetimeMs	= gRegister_LifetimeMs;
	
	if( gAddRecord_TypesCount > 0 )
	{
		size_t		i;
		
		context->extraRecords = (ExtraRecord *) calloc( gAddRecord_TypesCount, sizeof( ExtraRecord ) );
		require_action( context, exit, err = kNoMemoryErr );
		context->extraRecordsCount = gAddRecord_TypesCount;
		
		for( i = 0; i < gAddRecord_TypesCount; ++i )
		{
			ExtraRecord * const		extraRecord = &context->extraRecords[ i ];
			
			err = RecordTypeFromArgString( gAddRecord_Types[ i ], &extraRecord->type );
			require_noerr( err, exit );
			
			err = StringToUInt32( gAddRecord_TTLs[ i ], &extraRecord->ttl );
			if( err )
			{
				FPrintF( stderr, "Invalid TTL value: %s\n", gAddRecord_TTLs[ i ] );
				err = kParamErr;
				goto exit;
			}
			
			err = RecordDataFromArgString( gAddRecord_Data[ i ], &extraRecord->dataPtr, &extraRecord->dataLen );
			require_noerr_quiet( err, exit );
		}
	}
	
	if( gUpdateRecord_Data )
	{
		err = RecordDataFromArgString( gUpdateRecord_Data, &context->updateTXTPtr, &context->updateTXTLen );
		require_noerr_quiet( err, exit );
		
		context->updateTTL		= (uint32_t) gUpdateRecord_TTL;
		context->updateDelayMs	= gUpdateRecord_DelayMs;
	}
	
	// Print prologue.
	
	RegisterPrintPrologue( context );
	
	// Start operation.
	
	err = DNSServiceRegister( &context->opRef, context->flags, context->ifIndex, context->name, context->type,
		context->domain, NULL, htons( context->port ), (uint16_t) context->txtLen, context->txtPtr,
		RegisterCallback, context );
	ForgetMem( &context->txtPtr );
	require_noerr( err, exit );
	
	err = DNSServiceSetDispatchQueue( context->opRef, dispatch_get_main_queue() );
	require_noerr( err, exit );
	
	dispatch_main();
	
exit:
	dispatch_source_forget( &signalSource );
	if( context ) RegisterContextFree( context );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	RegisterPrintPrologue
//===========================================================================================================================

static void	RegisterPrintPrologue( const RegisterContext *inContext )
{
	size_t		i;
	int			infinite;
	char		ifName[ kInterfaceNameBufLen ];
	
	InterfaceIndexToName( inContext->ifIndex, ifName );
	
	FPrintF( stdout, "Flags:      %#{flags}\n",	inContext->flags, kDNSServiceFlagsDescriptors );
	FPrintF( stdout, "Interface:  %d (%s)\n",	(int32_t) inContext->ifIndex, ifName );
	FPrintF( stdout, "Name:       %s\n",		inContext->name ? inContext->name : "<NULL>" );
	FPrintF( stdout, "Type:       %s\n",		inContext->type );
	FPrintF( stdout, "Domain:     %s\n",		inContext->domain ? inContext->domain : "<NULL> (default domains)" );
	FPrintF( stdout, "Port:       %u\n",		inContext->port );
	FPrintF( stdout, "TXT data:   %#{txt}\n",	inContext->txtPtr, inContext->txtLen );
	infinite = ( inContext->lifetimeMs < 0 ) ? true : false;
	FPrintF( stdout, "Lifetime:   %?s%?d ms\n",	infinite, "∞", !infinite, inContext->lifetimeMs );
	if( inContext->updateTXTPtr )
	{
		FPrintF( stdout, "\nUpdate record:\n" );
		FPrintF( stdout, "    Delay:    %d ms\n",	( inContext->updateDelayMs > 0 ) ? inContext->updateDelayMs : 0 );
		FPrintF( stdout, "    TTL:      %u%?s\n",
			inContext->updateTTL, inContext->updateTTL == 0, " (system will use a default value.)" );
		FPrintF( stdout, "    TXT data: %#{txt}\n",	inContext->updateTXTPtr, inContext->updateTXTLen );
	}
	if( inContext->extraRecordsCount > 0 ) FPrintF( stdout, "\n" );
	for( i = 0; i < inContext->extraRecordsCount; ++i )
	{
		const ExtraRecord *		record = &inContext->extraRecords[ i ];
		
		FPrintF( stdout, "Extra record %zu:\n",		i + 1 );
		FPrintF( stdout, "    Type:  %s (%u)\n",	RecordTypeToString( record->type ), record->type );
		FPrintF( stdout, "    TTL:   %u%?s\n",		record->ttl, record->ttl == 0, " (system will use a default value.)" );
		FPrintF( stdout, "    RData: %#H\n\n",		record->dataPtr, (int) record->dataLen, INT_MAX );
	}
	FPrintF( stdout, "Start time: %{du:time}\n",	NULL );
	FPrintF( stdout, "---\n" );
}

//===========================================================================================================================
//	RegisterContextFree
//===========================================================================================================================

static void	RegisterContextFree( RegisterContext *inContext )
{
	ExtraRecord *					record;
	const ExtraRecord * const		end = inContext->extraRecords + inContext->extraRecordsCount;
	
	DNSServiceForget( &inContext->opRef );
	ForgetMem( &inContext->txtPtr );
	for( record = inContext->extraRecords; record < end; ++record )
	{
		check( !record->recordRef );
		ForgetMem( &record->dataPtr );
	}
	ForgetMem( &inContext->extraRecords );
	ForgetMem( &inContext->updateTXTPtr );
	free( inContext );
}

//===========================================================================================================================
//	RegisterCallback
//===========================================================================================================================

static void DNSSD_API
	RegisterCallback(
		DNSServiceRef		inSDRef,
		DNSServiceFlags		inFlags,
		DNSServiceErrorType	inError,
		const char *		inName,
		const char *		inType,
		const char *		inDomain,
		void *				inContext )
{
	RegisterContext * const		context = (RegisterContext *) inContext;
	OSStatus					err;
	struct timeval				now;
	
	Unused( inSDRef );
	
	gettimeofday( &now, NULL );
	
	if( !context->printedHeader )
	{
		FPrintF( stdout, "%-26s  %-16s Service\n", "Timestamp", "Flags" );
		context->printedHeader = true;
	}
	FPrintF( stdout, "%{du:time}  %{du:cbflags} %s.%s%s %?#m\n", &now, inFlags, inName, inType, inDomain, inError, inError );
	
	require_noerr_action_quiet( inError, exit, err = inError );
	
	if( !context->didRegister && ( inFlags & kDNSServiceFlagsAdd ) )
	{
		context->didRegister = true;
		if( context->updateTXTPtr )
		{
			if( context->updateDelayMs > 0 )
			{
				dispatch_after_f( dispatch_time_milliseconds( context->updateDelayMs ), dispatch_get_main_queue(),
					context, RegisterUpdate );
			}
			else
			{
				RegisterUpdate( context );
			}
		}
		if( context->extraRecordsCount > 0 )
		{
			ExtraRecord *					record;
			const ExtraRecord * const		end = context->extraRecords + context->extraRecordsCount;
			
			for( record = context->extraRecords; record < end; ++record )
			{
				err = DNSServiceAddRecord( context->opRef, &record->recordRef, 0, record->type,
					(uint16_t) record->dataLen, record->dataPtr, record->ttl );
				require_noerr( err, exit );
			}
		}
		if( context->lifetimeMs == 0 )
		{
			Exit( kExitReason_TimeLimit );
		}
		else if( context->lifetimeMs > 0 )
		{
			dispatch_after_f( dispatch_time_milliseconds( context->lifetimeMs ), dispatch_get_main_queue(),
				kExitReason_TimeLimit, Exit );
		}
	}
	err = kNoErr;
	
exit:
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	RegisterUpdate
//===========================================================================================================================

static void	RegisterUpdate( void *inContext )
{
	OSStatus					err;
	RegisterContext * const		context = (RegisterContext *) inContext;
	
	err = DNSServiceUpdateRecord( context->opRef, NULL, 0, (uint16_t) context->updateTXTLen, context->updateTXTPtr,
		context->updateTTL );
	require_noerr( err, exit );
	
exit:
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	RegisterRecordCmd
//===========================================================================================================================

typedef struct
{
	DNSServiceRef		conRef;			// sdRef to be initialized by DNSServiceCreateConnection().
	DNSRecordRef		recordRef;		// Registered record reference.
	const char *		recordName;		// Name of resource record.
	uint8_t *			dataPtr;		// Pointer to resource record data.
	size_t				dataLen;		// Length of resource record data.
	uint32_t			ttl;			// TTL value of resource record in seconds.
	uint32_t			ifIndex;		// Interface index argument for DNSServiceRegisterRecord().
	DNSServiceFlags		flags;			// Flags argument for DNSServiceRegisterRecord().
	int					lifetimeMs;		// Lifetime of the record registration in milliseconds.
	uint16_t			recordType;		// Resource record type.
	uint8_t *			updateDataPtr;	// Pointer to data for record update. (malloc'd)
	size_t				updateDataLen;	// Length of data for record update.
	uint32_t			updateTTL;		// TTL for updated record.
	int					updateDelayMs;	// Post-registration record update delay in milliseconds.
	Boolean				didRegister;	// True if the record was registered.
	
}	RegisterRecordContext;

static void	RegisterRecordPrintPrologue( const RegisterRecordContext *inContext );
static void	RegisterRecordContextFree( RegisterRecordContext *inContext );
static void DNSSD_API
	RegisterRecordCallback(
		DNSServiceRef		inSDRef,
		DNSRecordRef		inRecordRef,
		DNSServiceFlags		inFlags,
		DNSServiceErrorType	inError,
		void *				inContext );
static void	RegisterRecordUpdate( void *inContext );

static void	RegisterRecordCmd( void )
{
	OSStatus					err;
	RegisterRecordContext *		context			= NULL;
	dispatch_source_t			signalSource	= NULL;
	
	// Set up SIGINT handler.
	
	signal( SIGINT, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGINT, dispatch_get_main_queue(), Exit, kExitReason_SIGINT, &signalSource );
	require_noerr( err, exit );
	dispatch_resume( signalSource );
	
	// Create context.
	
	context = (RegisterRecordContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	// Create connection.
	
	err = CreateConnectionFromArgString( gConnectionOpt, dispatch_get_main_queue(), &context->conRef, NULL );
	require_noerr_quiet( err, exit );
	
	// Get flags.
	
	context->flags = GetDNSSDFlagsFromOpts();
	
	// Get interface.
	
	err = InterfaceIndexFromArgString( gInterface, &context->ifIndex );
	require_noerr_quiet( err, exit );
	
	// Get record type.
	
	err = RecordTypeFromArgString( gRegisterRecord_Type, &context->recordType );
	require_noerr( err, exit );
	
	// Get record data.
	
	if( gRegisterRecord_Data )
	{
		err = RecordDataFromArgString( gRegisterRecord_Data, &context->dataPtr, &context->dataLen );
		require_noerr_quiet( err, exit );
	}
	
	// Set remaining parameters.
	
	context->recordName	= gRegisterRecord_Name;
	context->ttl		= (uint32_t) gRegisterRecord_TTL;
	context->lifetimeMs	= gRegisterRecord_LifetimeMs;
	
	// Get update data.
	
	if( gRegisterRecord_UpdateData )
	{
		err = RecordDataFromArgString( gRegisterRecord_UpdateData, &context->updateDataPtr, &context->updateDataLen );
		require_noerr_quiet( err, exit );
		
		context->updateTTL		= (uint32_t) gRegisterRecord_UpdateTTL;
		context->updateDelayMs	= gRegisterRecord_UpdateDelayMs;
	}
	
	// Print prologue.
	
	RegisterRecordPrintPrologue( context );
	
	// Start operation.
	
	err = DNSServiceRegisterRecord( context->conRef, &context->recordRef, context->flags, context->ifIndex,
		context->recordName, context->recordType, kDNSServiceClass_IN, (uint16_t) context->dataLen, context->dataPtr,
		context->ttl, RegisterRecordCallback, context );
	if( err )
	{
		FPrintF( stderr, "DNSServiceRegisterRecord() returned %#m\n", err );
		goto exit;
	}
	
	dispatch_main();
	
exit:
	dispatch_source_forget( &signalSource );
	if( context ) RegisterRecordContextFree( context );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	RegisterRecordPrintPrologue
//===========================================================================================================================

static void	RegisterRecordPrintPrologue( const RegisterRecordContext *inContext )
{
	int			infinite;
	char		ifName[ kInterfaceNameBufLen ];
	
	InterfaceIndexToName( inContext->ifIndex, ifName );
	
	FPrintF( stdout, "Flags:       %#{flags}\n",	inContext->flags, kDNSServiceFlagsDescriptors );
	FPrintF( stdout, "Interface:   %d (%s)\n",		(int32_t) inContext->ifIndex, ifName );
	FPrintF( stdout, "Name:        %s\n",			inContext->recordName );
	FPrintF( stdout, "Type:        %s (%u)\n",		RecordTypeToString( inContext->recordType ), inContext->recordType );
	FPrintF( stdout, "TTL:         %u\n",			inContext->ttl );
	FPrintF( stdout, "Data:        %#H\n",			inContext->dataPtr, (int) inContext->dataLen, INT_MAX );
	infinite = ( inContext->lifetimeMs < 0 ) ? true : false;
	FPrintF( stdout, "Lifetime:    %?s%?d ms\n",	infinite, "∞", !infinite, inContext->lifetimeMs );
	if( inContext->updateDataPtr )
	{
		FPrintF( stdout, "\nUpdate record:\n" );
		FPrintF( stdout, "    Delay:    %d ms\n",	( inContext->updateDelayMs >= 0 ) ? inContext->updateDelayMs : 0 );
		FPrintF( stdout, "    TTL:      %u%?s\n",
			inContext->updateTTL, inContext->updateTTL == 0, " (system will use a default value.)" );
		FPrintF( stdout, "    RData:    %#H\n",		inContext->updateDataPtr, (int) inContext->updateDataLen, INT_MAX );
	}
	FPrintF( stdout, "Start time:  %{du:time}\n",	NULL );
	FPrintF( stdout, "---\n" );
}

//===========================================================================================================================
//	RegisterRecordContextFree
//===========================================================================================================================

static void	RegisterRecordContextFree( RegisterRecordContext *inContext )
{
	DNSServiceForget( &inContext->conRef );
	ForgetMem( &inContext->dataPtr );
	ForgetMem( &inContext->updateDataPtr );
	free( inContext );
}

//===========================================================================================================================
//	RegisterRecordCallback
//===========================================================================================================================

static void
	RegisterRecordCallback(
		DNSServiceRef		inSDRef,
		DNSRecordRef		inRecordRef,
		DNSServiceFlags		inFlags,
		DNSServiceErrorType	inError,
		void *				inContext )
{
	RegisterRecordContext *		context = (RegisterRecordContext *) inContext;
	struct timeval				now;
	
	Unused( inSDRef );
	Unused( inRecordRef );
	Unused( inFlags );
	Unused( context );
	
	gettimeofday( &now, NULL );
	FPrintF( stdout, "%{du:time} Record registration result (error %#m)\n", &now, inError );
	
	if( !context->didRegister && !inError )
	{
		context->didRegister = true;
		if( context->updateDataPtr )
		{
			if( context->updateDelayMs > 0 )
			{
				dispatch_after_f( dispatch_time_milliseconds( context->updateDelayMs ), dispatch_get_main_queue(),
					context, RegisterRecordUpdate );
			}
			else
			{
				RegisterRecordUpdate( context );
			}
		}
		if( context->lifetimeMs == 0 )
		{
			Exit( kExitReason_TimeLimit );
		}
		else if( context->lifetimeMs > 0 )
		{
			dispatch_after_f( dispatch_time_milliseconds( context->lifetimeMs ), dispatch_get_main_queue(),
				kExitReason_TimeLimit, Exit );
		}
	}
}

//===========================================================================================================================
//	RegisterRecordUpdate
//===========================================================================================================================

static void	RegisterRecordUpdate( void *inContext )
{
	OSStatus							err;
	RegisterRecordContext * const		context = (RegisterRecordContext *) inContext;
	
	err = DNSServiceUpdateRecord( context->conRef, context->recordRef, 0, (uint16_t) context->updateDataLen,
		context->updateDataPtr, context->updateTTL );
	require_noerr( err, exit );
	
exit:
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	ResolveCmd
//===========================================================================================================================

typedef struct
{
	DNSServiceRef		mainRef;		// Main sdRef for shared connections.
	DNSServiceRef		opRef;			// sdRef for the DNSServiceResolve operation.
	DNSServiceFlags		flags;			// Flags argument for DNSServiceResolve().
	const char *		name;			// Service name argument for DNSServiceResolve().
	const char *		type;			// Service type argument for DNSServiceResolve().
	const char *		domain;			// Domain argument for DNSServiceResolve().
	uint32_t			ifIndex;		// Interface index argument for DNSServiceResolve().
	int					timeLimitSecs;	// Time limit for the DNSServiceResolve operation in seconds.
	
}	ResolveContext;

static void	ResolvePrintPrologue( const ResolveContext *inContext );
static void	ResolveContextFree( ResolveContext *inContext );
static void DNSSD_API
	ResolveCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		const char *			inHostname,
		uint16_t				inPort,
		uint16_t				inTXTLen,
		const unsigned char *	inTXTPtr,
		void *					inContext );

static void	ResolveCmd( void )
{
	OSStatus				err;
	DNSServiceRef			sdRef;
	ResolveContext *		context			= NULL;
	dispatch_source_t		signalSource	= NULL;
	int						useMainConnection;
	
	// Set up SIGINT handler.
	
	signal( SIGINT, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGINT, dispatch_get_main_queue(), Exit, kExitReason_SIGINT, &signalSource );
	require_noerr( err, exit );
	dispatch_resume( signalSource );
	
	// Create context.
	
	context = (ResolveContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	// Check command parameters.
	
	if( gResolve_TimeLimitSecs < 0 )
	{
		FPrintF( stderr, "Invalid time limit: %d seconds.\n", gResolve_TimeLimitSecs );
		err = kParamErr;
		goto exit;
	}
	
	// Create main connection.
	
	if( gConnectionOpt )
	{
		err = CreateConnectionFromArgString( gConnectionOpt, dispatch_get_main_queue(), &context->mainRef, NULL );
		require_noerr_quiet( err, exit );
		useMainConnection = true;
	}
	else
	{
		useMainConnection = false;
	}
	
	// Get flags.
	
	context->flags = GetDNSSDFlagsFromOpts();
	if( useMainConnection ) context->flags |= kDNSServiceFlagsShareConnection;
	
	// Get interface index.
	
	err = InterfaceIndexFromArgString( gInterface, &context->ifIndex );
	require_noerr_quiet( err, exit );
	
	// Set remaining parameters.
	
	context->name			= gResolve_Name;
	context->type			= gResolve_Type;
	context->domain			= gResolve_Domain;
	context->timeLimitSecs	= gResolve_TimeLimitSecs;
	
	// Print prologue.
	
	ResolvePrintPrologue( context );
	
	// Start operation.
	
	sdRef = useMainConnection ? context->mainRef : kBadDNSServiceRef;
	err = DNSServiceResolve( &sdRef, context->flags, context->ifIndex, context->name, context->type, context->domain,
		ResolveCallback, NULL );
	require_noerr( err, exit );
	
	context->opRef = sdRef;
	if( !useMainConnection )
	{
		err = DNSServiceSetDispatchQueue( context->opRef, dispatch_get_main_queue() );
		require_noerr( err, exit );
	}
	
	// Set time limit.
	
	if( context->timeLimitSecs > 0 )
	{
		dispatch_after_f( dispatch_time_seconds( context->timeLimitSecs ), dispatch_get_main_queue(),
			kExitReason_TimeLimit, Exit );
	}
	dispatch_main();
	
exit:
	dispatch_source_forget( &signalSource );
	if( context ) ResolveContextFree( context );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	ReconfirmCmd
//===========================================================================================================================

static void	ReconfirmCmd( void )
{
	OSStatus			err;
	uint8_t *			rdataPtr = NULL;
	size_t				rdataLen = 0;
	DNSServiceFlags		flags;
	uint32_t			ifIndex;
	uint16_t			type, class;
	char				ifName[ kInterfaceNameBufLen ];
	
	// Get flags.
	
	flags = GetDNSSDFlagsFromOpts();
	
	// Get interface index.
	
	err = InterfaceIndexFromArgString( gInterface, &ifIndex );
	require_noerr_quiet( err, exit );
	
	// Get record type.
	
	err = RecordTypeFromArgString( gReconfirmRecord_Type, &type );
	require_noerr( err, exit );
	
	// Get record data.
	
	if( gReconfirmRecord_Data )
	{
		err = RecordDataFromArgString( gReconfirmRecord_Data, &rdataPtr, &rdataLen );
		require_noerr_quiet( err, exit );
	}
	
	// Get record class.
	
	if( gReconfirmRecord_Class )
	{
		err = RecordClassFromArgString( gReconfirmRecord_Class, &class );
		require_noerr( err, exit );
	}
	else
	{
		class = kDNSServiceClass_IN;
	}
	
	// Print prologue.
	
	FPrintF( stdout, "Flags:     %#{flags}\n",	flags, kDNSServiceFlagsDescriptors );
	FPrintF( stdout, "Interface: %d (%s)\n",	(int32_t) ifIndex, InterfaceIndexToName( ifIndex, ifName ) );
	FPrintF( stdout, "Name:      %s\n",			gReconfirmRecord_Name );
	FPrintF( stdout, "Type:      %s (%u)\n",	RecordTypeToString( type ), type );
	FPrintF( stdout, "Class:     %s (%u)\n",	( class == kDNSServiceClass_IN ) ? "IN" : "???", class );
	FPrintF( stdout, "Data:      %#H\n",		rdataPtr, (int) rdataLen, INT_MAX );
	FPrintF( stdout, "---\n" );
	
	err = DNSServiceReconfirmRecord( flags, ifIndex, gReconfirmRecord_Name, type, class, (uint16_t) rdataLen, rdataPtr );
	FPrintF( stdout, "Error:     %#m\n", err );
	
exit:
	FreeNullSafe( rdataPtr );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	ResolvePrintPrologue
//===========================================================================================================================

static void	ResolvePrintPrologue( const ResolveContext *inContext )
{
	const int		timeLimitSecs = inContext->timeLimitSecs;
	char			ifName[ kInterfaceNameBufLen ];
	
	InterfaceIndexToName( inContext->ifIndex, ifName );
	
	FPrintF( stdout, "Flags:      %#{flags}\n",		inContext->flags, kDNSServiceFlagsDescriptors );
	FPrintF( stdout, "Interface:  %d (%s)\n",		(int32_t) inContext->ifIndex, ifName );
	FPrintF( stdout, "Name:       %s\n",			inContext->name );
	FPrintF( stdout, "Type:       %s\n",			inContext->type );
	FPrintF( stdout, "Domain:     %s\n",			inContext->domain );
	FPrintF( stdout, "Time limit: " );
	if( timeLimitSecs > 0 )	FPrintF( stdout, "%d second%?c\n", timeLimitSecs, timeLimitSecs != 1, 's' );
	else					FPrintF( stdout, "∞\n" );
	FPrintF( stdout, "Start time: %{du:time}\n",	NULL );
	FPrintF( stdout, "---\n" );
}

//===========================================================================================================================
//	ResolveContextFree
//===========================================================================================================================

static void	ResolveContextFree( ResolveContext *inContext )
{
	DNSServiceForget( &inContext->opRef );
	DNSServiceForget( &inContext->mainRef );
	free( inContext );
}

//===========================================================================================================================
//	ResolveCallback
//===========================================================================================================================

static void DNSSD_API
	ResolveCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		const char *			inHostname,
		uint16_t				inPort,
		uint16_t				inTXTLen,
		const unsigned char *	inTXTPtr,
		void *					inContext )
{
	struct timeval		now;
	char				errorStr[ 64 ];
	
	Unused( inSDRef );
	Unused( inFlags );
	Unused( inContext );
	
	gettimeofday( &now, NULL );
	
	if( inError ) SNPrintF( errorStr, sizeof( errorStr ), " error %#m", inError );
	
	FPrintF( stdout, "%{du:time}: %s can be reached at %s:%u (interface %d)%?s\n",
		&now, inFullName, inHostname, ntohs( inPort ), (int32_t) inInterfaceIndex, inError, errorStr );
	if( inTXTLen == 1 )
	{
		FPrintF( stdout, " TXT record: %#H\n", inTXTPtr, (int) inTXTLen, INT_MAX );
	}
	else
	{
		FPrintF( stdout, " TXT record: %#{txt}\n", inTXTPtr, (size_t) inTXTLen );
	}
}

//===========================================================================================================================
//	GetAddrInfoPOSIXCmd
//===========================================================================================================================

#define AddressFamilyStr( X ) (				\
	( (X) == AF_INET )		? "inet"	:	\
	( (X) == AF_INET6 )		? "inet6"	:	\
	( (X) == AF_UNSPEC )	? "unspec"	:	\
							  "???" )

typedef struct
{
    unsigned int		flag;
    const char *        str;

}   FlagStringPair;

#define CaseFlagStringify( X )		{ (X), # X }

const FlagStringPair		kGAIPOSIXFlagStringPairs[] =
{
#if( defined( AI_UNUSABLE ) )
	CaseFlagStringify( AI_UNUSABLE ),
#endif
	CaseFlagStringify( AI_NUMERICSERV ),
	CaseFlagStringify( AI_V4MAPPED ),
	CaseFlagStringify( AI_ADDRCONFIG ),
#if( defined( AI_V4MAPPED_CFG ) )
	CaseFlagStringify( AI_V4MAPPED_CFG ),
#endif
	CaseFlagStringify( AI_ALL ),
	CaseFlagStringify( AI_NUMERICHOST ),
	CaseFlagStringify( AI_CANONNAME ),
	CaseFlagStringify( AI_PASSIVE ),
	{ 0, NULL }
};

static void	GetAddrInfoPOSIXCmd( void )
{
	OSStatus					err;
	struct addrinfo				hints;
	struct timeval				now;
	const struct addrinfo *		addrInfo;
	struct addrinfo *			addrInfoList = NULL;
	const FlagStringPair *		pair;
	
	memset( &hints, 0, sizeof( hints ) );
	hints.ai_socktype = SOCK_STREAM;
	
	// Set hints address family.
	
	if( !gGAIPOSIX_Family )										hints.ai_family = AF_UNSPEC;
	else if( strcasecmp( gGAIPOSIX_Family, "inet" ) == 0 )		hints.ai_family = AF_INET;
	else if( strcasecmp( gGAIPOSIX_Family, "inet6" ) == 0 )		hints.ai_family = AF_INET6;
	else if( strcasecmp( gGAIPOSIX_Family, "unspec" ) == 0 )	hints.ai_family = AF_UNSPEC;
	else
	{
		FPrintF( stderr, "Invalid address family: %s.\n", gGAIPOSIX_Family );
		err = kParamErr;
		goto exit;
	}
	
	// Set hints flags.
	
	if( gGAIPOSIXFlag_AddrConfig )	hints.ai_flags |= AI_ADDRCONFIG;
	if( gGAIPOSIXFlag_All )			hints.ai_flags |= AI_ALL;
	if( gGAIPOSIXFlag_CanonName )	hints.ai_flags |= AI_CANONNAME;
	if( gGAIPOSIXFlag_NumericHost )	hints.ai_flags |= AI_NUMERICHOST;
	if( gGAIPOSIXFlag_NumericServ )	hints.ai_flags |= AI_NUMERICSERV;
	if( gGAIPOSIXFlag_Passive )		hints.ai_flags |= AI_PASSIVE;
	if( gGAIPOSIXFlag_V4Mapped )	hints.ai_flags |= AI_V4MAPPED;
#if( defined( AI_V4MAPPED_CFG ) )
	if( gGAIPOSIXFlag_V4MappedCFG )	hints.ai_flags |= AI_V4MAPPED_CFG;
#endif
#if( defined( AI_DEFAULT ) )
	if( gGAIPOSIXFlag_Default )		hints.ai_flags |= AI_DEFAULT;
#endif
#if( defined( AI_UNUSABLE ) )
	if( gGAIPOSIXFlag_Unusable )	hints.ai_flags |= AI_UNUSABLE;
#endif
	
	// Print prologue.
	
	FPrintF( stdout, "Hostname:       %s\n",	gGAIPOSIX_HostName );
	FPrintF( stdout, "Servname:       %s\n",	gGAIPOSIX_ServName );
	FPrintF( stdout, "Address family: %s\n",	AddressFamilyStr( hints.ai_family ) );
	FPrintF( stdout, "Flags:          0x%X < ",	hints.ai_flags );
	for( pair = kGAIPOSIXFlagStringPairs; pair->str != NULL; ++pair )
	{
		if( ( (unsigned int) hints.ai_flags ) & pair->flag ) FPrintF( stdout, "%s ", pair->str );
	}
	FPrintF( stdout, ">\n" );
	FPrintF( stdout, "Start time:     %{du:time}\n", NULL );
	FPrintF( stdout, "---\n" );
	
	// Call getaddrinfo().
	
	err = getaddrinfo( gGAIPOSIX_HostName, gGAIPOSIX_ServName, &hints, &addrInfoList );
	gettimeofday( &now, NULL );
	if( err )
	{
		FPrintF( stderr, "Error %d: %s.\n", err, gai_strerror( err ) );
	}
	else
	{
		int		addrCount = 0;
		
		for( addrInfo = addrInfoList; addrInfo; addrInfo = addrInfo->ai_next ) { ++addrCount; }
		
		FPrintF( stdout, "Addresses (%d total):\n", addrCount );
		for( addrInfo = addrInfoList; addrInfo; addrInfo = addrInfo->ai_next )
		{
			FPrintF( stdout, "%##a\n", addrInfo->ai_addr );
		}
	}
	FPrintF( stdout, "---\n" );
	FPrintF( stdout, "End time:       %{du:time}\n", &now );
	
exit:
	if( addrInfoList ) freeaddrinfo( addrInfoList );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	ReverseLookupCmd
//===========================================================================================================================

#define kIP6ARPADomainStr		"ip6.arpa."

static void	ReverseLookupCmd( void )
{
	OSStatus					err;
	QueryRecordContext *		context			= NULL;
	DNSServiceRef				sdRef;
	dispatch_source_t			signalSource	= NULL;
	uint32_t					ipv4Addr;
	uint8_t						ipv6Addr[ 16 ];
	char						recordName[ ( 16 * 4 ) + sizeof( kIP6ARPADomainStr ) ];
	int							useMainConnection;
	const char *				endPtr;
	
	// Set up SIGINT handler.
	
	signal( SIGINT, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGINT, dispatch_get_main_queue(), Exit, kExitReason_SIGINT, &signalSource );
	require_noerr( err, exit );
	dispatch_resume( signalSource );
	
	// Create context.
	
	context = (QueryRecordContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	// Check command parameters.
	
	if( gReverseLookup_TimeLimitSecs < 0 )
	{
		FPrintF( stderr, "Invalid time limit: %d s.\n", gReverseLookup_TimeLimitSecs );
		err = kParamErr;
		goto exit;
	}
	
	// Create main connection.
	
	if( gConnectionOpt )
	{
		err = CreateConnectionFromArgString( gConnectionOpt, dispatch_get_main_queue(), &context->mainRef, NULL );
		require_noerr_quiet( err, exit );
		useMainConnection = true;
	}
	else
	{
		useMainConnection = false;
	}
	
	// Get flags.
	
	context->flags = GetDNSSDFlagsFromOpts();
	if( useMainConnection ) context->flags |= kDNSServiceFlagsShareConnection;
	
	// Get interface index.
	
	err = InterfaceIndexFromArgString( gInterface, &context->ifIndex );
	require_noerr_quiet( err, exit );
	
	// Create reverse lookup record name.
	
	err = _StringToIPv4Address( gReverseLookup_IPAddr, kStringToIPAddressFlagsNoPort | kStringToIPAddressFlagsNoPrefix,
		&ipv4Addr, NULL, NULL, NULL, &endPtr );
	if( err || ( *endPtr != '\0' ) )
	{
		char *		dst;
		int			i;
		
		err = _StringToIPv6Address( gReverseLookup_IPAddr,
			kStringToIPAddressFlagsNoPort | kStringToIPAddressFlagsNoPrefix | kStringToIPAddressFlagsNoScope,
			ipv6Addr, NULL, NULL, NULL, &endPtr );
		if( err || ( *endPtr != '\0' ) )
		{
			FPrintF( stderr, "Invalid IP address: \"%s\".\n", gReverseLookup_IPAddr );
			err = kParamErr;
			goto exit;
		}
		dst = recordName;
		for( i = 15; i >= 0; --i )
		{
			*dst++ = kHexDigitsLowercase[ ipv6Addr[ i ] & 0x0F ];
			*dst++ = '.';
			*dst++ = kHexDigitsLowercase[ ipv6Addr[ i ] >> 4 ];
			*dst++ = '.';
		}
		strcpy_literal( dst, kIP6ARPADomainStr );
		check( ( strlen( recordName ) + 1 ) <= sizeof( recordName ) );
	}
	else
	{
		SNPrintF( recordName, sizeof( recordName ), "%u.%u.%u.%u.in-addr.arpa.",
			  ipv4Addr         & 0xFF,
			( ipv4Addr >>  8 ) & 0xFF,
			( ipv4Addr >> 16 ) & 0xFF,
			( ipv4Addr >> 24 ) & 0xFF );
	}
	
	// Set remaining parameters.
	
	context->recordName		= recordName;
	context->recordType		= kDNSServiceType_PTR;
	context->timeLimitSecs	= gReverseLookup_TimeLimitSecs;
	context->oneShotMode	= gReverseLookup_OneShot ? true : false;
	
	// Print prologue.
	
	QueryRecordPrintPrologue( context );
	
	// Start operation.
	
	sdRef = useMainConnection ? context->mainRef : kBadDNSServiceRef;
	err = DNSServiceQueryRecord( &sdRef, context->flags, context->ifIndex, context->recordName, context->recordType,
		kDNSServiceClass_IN, QueryRecordCallback, context );
	require_noerr( err, exit );
	
	context->opRef = sdRef;
	if( !useMainConnection )
	{
		err = DNSServiceSetDispatchQueue( context->opRef, dispatch_get_main_queue() );
		require_noerr( err, exit );
	}
	
	// Set time limit.
	
	if( context->timeLimitSecs > 0 )
	{
		dispatch_after_f( dispatch_time_seconds( context->timeLimitSecs ), dispatch_get_main_queue(),
			kExitReason_TimeLimit, Exit );
	}
	dispatch_main();
	
exit:
	dispatch_source_forget( &signalSource );
	if( context ) QueryRecordContextFree( context );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	PortMappingCmd
//===========================================================================================================================

typedef struct
{
	DNSServiceRef			mainRef;		// Main sdRef for shared connection.
	DNSServiceRef			opRef;			// sdRef for the DNSServiceNATPortMappingCreate operation.
	DNSServiceFlags			flags;			// Flags for DNSServiceNATPortMappingCreate operation.
	uint32_t				ifIndex;		// Interface index argument for DNSServiceNATPortMappingCreate operation.
	DNSServiceProtocol		protocols;		// Protocols argument for DNSServiceNATPortMappingCreate operation.
	uint32_t				ttl;			// TTL argument for DNSServiceNATPortMappingCreate operation.
	uint16_t				internalPort;	// Internal port argument for DNSServiceNATPortMappingCreate operation.
	uint16_t				externalPort;	// External port argument for DNSServiceNATPortMappingCreate operation.
	Boolean					printedHeader;	// True if results header was printed.
	
}	PortMappingContext;

static void	PortMappingPrintPrologue( const PortMappingContext *inContext );
static void	PortMappingContextFree( PortMappingContext *inContext );
static void DNSSD_API
	PortMappingCallback(
		DNSServiceRef		inSDRef,
		DNSServiceFlags		inFlags,
		uint32_t			inInterfaceIndex,
		DNSServiceErrorType	inError,
		uint32_t			inExternalIPv4Address,
		DNSServiceProtocol	inProtocol,
		uint16_t			inInternalPort,
		uint16_t			inExternalPort,
		uint32_t			inTTL,
		void *				inContext );

static void	PortMappingCmd( void )
{
	OSStatus					err;
	PortMappingContext *		context			= NULL;
	DNSServiceRef				sdRef;
	dispatch_source_t			signalSource	= NULL;
	int							useMainConnection;
	
	// Set up SIGINT handler.
	
	signal( SIGINT, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGINT, dispatch_get_main_queue(), Exit, kExitReason_SIGINT, &signalSource );
	require_noerr( err, exit );
	dispatch_resume( signalSource );
	
	// Create context.
	
	context = (PortMappingContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	// Check command parameters.
	
	if( ( gPortMapping_InternalPort < 0 ) || ( gPortMapping_InternalPort > UINT16_MAX ) )
	{
		FPrintF( stderr, "Internal port number %d is out-of-range.\n", gPortMapping_InternalPort );
		err = kParamErr;
		goto exit;
	}
	
	if( ( gPortMapping_ExternalPort < 0 ) || ( gPortMapping_ExternalPort > UINT16_MAX ) )
	{
		FPrintF( stderr, "External port number %d is out-of-range.\n", gPortMapping_ExternalPort );
		err = kParamErr;
		goto exit;
	}
	
	// Create main connection.
	
	if( gConnectionOpt )
	{
		err = CreateConnectionFromArgString( gConnectionOpt, dispatch_get_main_queue(), &context->mainRef, NULL );
		require_noerr_quiet( err, exit );
		useMainConnection = true;
	}
	else
	{
		useMainConnection = false;
	}
	
	// Get flags.
	
	context->flags = GetDNSSDFlagsFromOpts();
	if( useMainConnection ) context->flags |= kDNSServiceFlagsShareConnection;
	
	// Get interface index.
	
	err = InterfaceIndexFromArgString( gInterface, &context->ifIndex );
	require_noerr_quiet( err, exit );
	
	// Set remaining parameters.
	
	if( gPortMapping_ProtocolTCP ) context->protocols |= kDNSServiceProtocol_TCP;
	if( gPortMapping_ProtocolUDP ) context->protocols |= kDNSServiceProtocol_UDP;
	context->ttl			= (uint32_t) gPortMapping_TTL;
	context->internalPort	= (uint16_t) gPortMapping_InternalPort;
	context->externalPort	= (uint16_t) gPortMapping_ExternalPort;
	
	// Print prologue.
	
	PortMappingPrintPrologue( context );
	
	// Start operation.
	
	sdRef = useMainConnection ? context->mainRef : kBadDNSServiceRef;
	err = DNSServiceNATPortMappingCreate( &sdRef, context->flags, context->ifIndex, context->protocols,
		htons( context->internalPort ), htons( context->externalPort ), context->ttl, PortMappingCallback, context );
	require_noerr( err, exit );
	
	context->opRef = sdRef;
	if( !useMainConnection )
	{
		err = DNSServiceSetDispatchQueue( context->opRef, dispatch_get_main_queue() );
		require_noerr( err, exit );
	}
	
	dispatch_main();
	
exit:
	dispatch_source_forget( &signalSource );
	if( context ) PortMappingContextFree( context );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	PortMappingPrintPrologue
//===========================================================================================================================

static void	PortMappingPrintPrologue( const PortMappingContext *inContext )
{
	char		ifName[ kInterfaceNameBufLen ];
	
	InterfaceIndexToName( inContext->ifIndex, ifName );
	
	FPrintF( stdout, "Flags:         %#{flags}\n",		inContext->flags, kDNSServiceFlagsDescriptors );
	FPrintF( stdout, "Interface:     %d (%s)\n",		(int32_t) inContext->ifIndex, ifName );
	FPrintF( stdout, "Protocols:     %#{flags}\n",		inContext->protocols, kDNSServiceProtocolDescriptors );
	FPrintF( stdout, "Internal Port: %u\n",				inContext->internalPort );
	FPrintF( stdout, "External Port: %u\n",				inContext->externalPort );
	FPrintF( stdout, "TTL:           %u%?s\n",			inContext->ttl, !inContext->ttl,
		" (system will use a default value.)" );
	FPrintF( stdout, "Start time:    %{du:time}\n",	NULL );
	FPrintF( stdout, "---\n" );
	
}

//===========================================================================================================================
//	PortMappingContextFree
//===========================================================================================================================

static void	PortMappingContextFree( PortMappingContext *inContext )
{
	DNSServiceForget( &inContext->opRef );
	DNSServiceForget( &inContext->mainRef );
	free( inContext );
}

//===========================================================================================================================
//	PortMappingCallback
//===========================================================================================================================

static void DNSSD_API
	PortMappingCallback(
		DNSServiceRef		inSDRef,
		DNSServiceFlags		inFlags,
		uint32_t			inInterfaceIndex,
		DNSServiceErrorType	inError,
		uint32_t			inExternalIPv4Address,
		DNSServiceProtocol	inProtocol,
		uint16_t			inInternalPort,
		uint16_t			inExternalPort,
		uint32_t			inTTL,
		void *				inContext )
{
	PortMappingContext * const		context = (PortMappingContext *) inContext;
	struct timeval					now;
	char							errorStr[ 128 ];
	
	Unused( inSDRef );
	Unused( inFlags );
	
	gettimeofday( &now, NULL );
	
	if( inError ) SNPrintF( errorStr, sizeof( errorStr ), " (error: %#m)", inError );
	if( !context->printedHeader )
	{
		FPrintF( stdout, "%-26s  IF %7s %15s %7s %6s Protocol\n", "Timestamp", "IntPort", "ExtAddr", "ExtPort", "TTL" );
		context->printedHeader = true;
	}
	FPrintF( stdout, "%{du:time}  %2u %7u %15.4a %7u %6u %#{flags}%?s\n",
		&now, inInterfaceIndex, ntohs( inInternalPort), &inExternalIPv4Address, ntohs( inExternalPort ), inTTL,
		inProtocol, kDNSServiceProtocolDescriptors, inError, errorStr );
}

#if( TARGET_OS_DARWIN )
//===========================================================================================================================
//	RegisterKACmd
//===========================================================================================================================

typedef struct
{
	dispatch_queue_t			queue;			// Serial queue for command's events.
	dispatch_semaphore_t		doneSem;		// Semaphore to signal when underlying command operation is done.
	sockaddr_ip					local;			// Connection's local IP address and port.
	sockaddr_ip					remote;			// Connection's remote IP address and port.
	DNSServiceFlags				flags;			// Flags to pass to DNSServiceSleepKeepalive_sockaddr().
	unsigned int				timeout;		// Timeout to pass to DNSServiceSleepKeepalive_sockaddr().
	DNSServiceRef				keepalive;		// DNSServiceSleepKeepalive_sockaddr operation.
	dispatch_source_t			sourceSigInt;	// Dispatch source for SIGINT.
	dispatch_source_t			sourceSigTerm;	// Dispatch source for SIGTERM.
	OSStatus					error;			// Command's error.
	
}	RegisterKACmdContext;

static void		_RegisterKACmdFree( RegisterKACmdContext *inCmd );
static void		_RegisterKACmdStart( void *inContext );
static OSStatus	_RegisterKACmdGetIPAddressArgument( const char *inArgStr, const char *inArgName, sockaddr_ip *outSA );

static void	RegisterKACmd( void )
{
	OSStatus					err;
	RegisterKACmdContext *		cmd = NULL;
	
	if( !SOFT_LINK_HAS_FUNCTION( system_dnssd, DNSServiceSleepKeepalive_sockaddr ) )
	{
		FPrintF( stderr, "error: Failed to soft link DNSServiceSleepKeepalive_sockaddr from libsystem_dnssd.\n" );
		err = kNotFoundErr;
		goto exit;
	}
	cmd = (RegisterKACmdContext *) calloc( 1, sizeof( *cmd ) );
	require_action( cmd, exit, err = kNoMemoryErr );
	
	err = _RegisterKACmdGetIPAddressArgument( gRegisterKA_LocalAddress, "local IP address", &cmd->local );
	require_noerr_quiet( err, exit );
	
	err = _RegisterKACmdGetIPAddressArgument( gRegisterKA_RemoteAddress, "remote IP address", &cmd->remote );
	require_noerr_quiet( err, exit );
	
	err = CheckIntegerArgument( gRegisterKA_Timeout, "timeout", 0, INT_MAX );
	require_noerr_quiet( err, exit );
	
	cmd->flags		= GetDNSSDFlagsFromOpts();
	cmd->timeout	= (unsigned int) gRegisterKA_Timeout;
	
	// Start command.
	
	cmd->queue = dispatch_queue_create( "com.apple.dnssdutil.registerka-command", DISPATCH_QUEUE_SERIAL );
	require_action( cmd->queue, exit, err = kNoResourcesErr );
	
	cmd->doneSem = dispatch_semaphore_create( 0 );
	require_action( cmd->doneSem, exit, err = kNoResourcesErr );
	
	dispatch_async_f( cmd->queue, cmd, _RegisterKACmdStart );
    dispatch_semaphore_wait( cmd->doneSem, DISPATCH_TIME_FOREVER );
	if( cmd->error ) err = cmd->error;
	
	FPrintF( stdout, "---\n" );
	FPrintF( stdout, "End time:   %{du:time}\n", NULL );
	
exit:
	if( cmd ) _RegisterKACmdFree( cmd );
	gExitCode = err ? 1 : 0;
}

//===========================================================================================================================

static void	_RegisterKACmdFree( RegisterKACmdContext *inCmd )
{
	check( !inCmd->keepalive );
	check( !inCmd->sourceSigInt );
	check( !inCmd->sourceSigTerm );
	dispatch_forget( &inCmd->queue );
	dispatch_forget( &inCmd->doneSem );
	free( inCmd );
}

//===========================================================================================================================

static void				_RegisterKACmdStop( RegisterKACmdContext *inCmd, OSStatus inError );
static void				_RegisterKACmdSignalHandler( void *inContext );
static void DNSSD_API	_RegisterKACmdKeepaliveCallback( DNSServiceRef inSDRef, DNSServiceErrorType inError, void *inCtx );

static void	_RegisterKACmdStart( void *inContext )
{
	OSStatus							err;
	RegisterKACmdContext * const		cmd = (RegisterKACmdContext *) inContext;
	
	signal( SIGINT, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGINT, cmd->queue, _RegisterKACmdSignalHandler, cmd, &cmd->sourceSigInt );
	require_noerr( err, exit );
	dispatch_resume( cmd->sourceSigInt );
	
	signal( SIGTERM, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGTERM, cmd->queue, _RegisterKACmdSignalHandler, cmd, &cmd->sourceSigTerm );
	require_noerr( err, exit );
	dispatch_resume( cmd->sourceSigTerm );
	
	FPrintF( stdout, "Flags:      %#{flags}\n",		cmd->flags, kDNSServiceFlagsDescriptors );
	FPrintF( stdout, "Local:      %##a\n",			&cmd->local.sa );
	FPrintF( stdout, "Remote:     %##a\n",			&cmd->remote.sa );
	FPrintF( stdout, "Timeout:    %u\n",			cmd->timeout );
	FPrintF( stdout, "Start time: %{du:time}\n",	NULL );
	FPrintF( stdout, "---\n" );
	
	err = soft_DNSServiceSleepKeepalive_sockaddr( &cmd->keepalive, cmd->flags, &cmd->local.sa, &cmd->remote.sa,
		cmd->timeout, _RegisterKACmdKeepaliveCallback, cmd );
	require_noerr( err, exit );
	
	err = DNSServiceSetDispatchQueue( cmd->keepalive, cmd->queue );
	require_noerr( err, exit );
	
exit:
	if( err ) _RegisterKACmdStop( cmd, err );
}

//===========================================================================================================================

static OSStatus	_RegisterKACmdGetIPAddressArgument( const char *inArgStr, const char *inArgName, sockaddr_ip *outSA )
{
	OSStatus		err;
	sockaddr_ip		sip;
	
	err = StringToSockAddr( inArgStr, &sip, sizeof( sip ), NULL );
	if( !err && ( ( sip.sa.sa_family == AF_INET ) || ( sip.sa.sa_family == AF_INET6 ) ) )
	{
		if( outSA ) SockAddrCopy( &sip, outSA );
	}
	else
	{
		FPrintF( stderr, "error: Invalid %s: '%s'\n", inArgName, inArgStr );
		err = kParamErr;
	}
	return( err );
}

//===========================================================================================================================

static void	_RegisterKACmdStop( RegisterKACmdContext *inCmd, OSStatus inError )
{
	if( !inCmd->error ) inCmd->error = inError;
	DNSServiceForget( &inCmd->keepalive );
	dispatch_source_forget( &inCmd->sourceSigInt );
	dispatch_source_forget( &inCmd->sourceSigTerm );
	dispatch_semaphore_signal( inCmd->doneSem );
}

//===========================================================================================================================

static void	_RegisterKACmdSignalHandler( void *inContext )
{
	RegisterKACmdContext * const		cmd = (RegisterKACmdContext *) inContext;
	
	_RegisterKACmdStop( cmd, kNoErr );
}

//===========================================================================================================================

static void DNSSD_API	_RegisterKACmdKeepaliveCallback( DNSServiceRef inSDRef, DNSServiceErrorType inError, void *inCtx )
{
	RegisterKACmdContext * const		cmd = (RegisterKACmdContext *) inCtx;
	
	Unused( inSDRef );
	
	FPrintF( stdout, "%{du:time} Record registration result: %#m\n", NULL, inError );
	if( !cmd->error ) cmd->error = inError;
}
#endif	// TARGET_OS_DARWIN

//===========================================================================================================================
//	BrowseAllCmd
//===========================================================================================================================

typedef struct BrowseAllConnection		BrowseAllConnection;

typedef struct
{
	ServiceBrowserRef			browser;				// Service browser.
	ServiceBrowserResults *		results;				// Results from the service browser.
	BrowseAllConnection *		connectionList;			// List of connections.
	dispatch_source_t			connectionTimer;		// Timer for connection timeout.
	int							connectionPendingCount;	// Number of pending connections.
	int							connectionTimeoutSecs;	// Timeout value for connections in seconds.
	
}	BrowseAllContext;

struct BrowseAllConnection
{
	BrowseAllConnection *		next;				// Next connection object in list.
	sockaddr_ip					sip;				// IPv4 or IPv6 address to connect to.
	uint16_t					port;				// TCP port to connect to.
	AsyncConnectionRef			asyncCnx;			// AsyncConnection object to handle the actual connection.
	OSStatus					status;				// Status of connection. NoErr means connection succeeded.
	CFTimeInterval				connectTimeSecs;	// Time it took to connect in seconds.
	int32_t						refCount;			// This object's reference count.
	BrowseAllContext *			context;			// Back pointer to parent context.
};

static void	_BrowseAllContextFree( BrowseAllContext *inContext );
static void	_BrowseAllServiceBrowserCallback( ServiceBrowserResults *inResults, OSStatus inError, void *inContext );
static OSStatus
	_BrowseAllConnectionCreate(
		const struct sockaddr *	inSockAddr,
		uint16_t				inPort,
		BrowseAllContext *		inContext,
		BrowseAllConnection **	outConnection );
static void _BrowseAllConnectionRetain( BrowseAllConnection *inConnection );
static void	_BrowseAllConnectionRelease( BrowseAllConnection *inConnection );
static void	_BrowseAllConnectionProgress( int inPhase, const void *inDetails, void *inArg );
static void	_BrowseAllConnectionHandler( SocketRef inSock, OSStatus inError, void *inArg );
static void	_BrowseAllExit( void *inContext );

static Boolean	_IsServiceTypeTCP( const char *inServiceType );

static void	BrowseAllCmd( void )
{
	OSStatus				err;
	BrowseAllContext *		context = NULL;
	size_t					i;
	uint32_t				ifIndex;
	char					ifName[ kInterfaceNameBufLen ];
	
	// Check parameters.
	
	if( gBrowseAll_BrowseTimeSecs <= 0 )
	{
		FPrintF( stdout, "Invalid browse time: %d seconds.\n", gBrowseAll_BrowseTimeSecs );
		err = kParamErr;
		goto exit;
	}
	
	context = (BrowseAllContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	context->connectionTimeoutSecs	= gBrowseAll_ConnectTimeout;
#if( TARGET_OS_POSIX )
	// Increase the open file descriptor limit for connection sockets.
	
	if( context->connectionTimeoutSecs > 0 )
	{
		struct rlimit		fdLimits;
		
		err = getrlimit( RLIMIT_NOFILE, &fdLimits );
		err = map_global_noerr_errno( err );
		require_noerr( err, exit );
		
		if( fdLimits.rlim_cur < 4096 )
		{
			fdLimits.rlim_cur = 4096;
			err = setrlimit( RLIMIT_NOFILE, &fdLimits );
			err = map_global_noerr_errno( err );
			require_noerr( err, exit );
		}
	}
#endif
	
	// Get interface index.
	
	err = InterfaceIndexFromArgString( gInterface, &ifIndex );
	require_noerr_quiet( err, exit );
	
	// Print prologue.
	
	FPrintF( stdout, "Interface:       %d (%s)\n",	(int32_t) ifIndex, InterfaceIndexToName( ifIndex, ifName ) );
	FPrintF( stdout, "Service types:   ");
	if( gBrowseAll_ServiceTypesCount > 0 )
	{
		FPrintF( stdout, "%s", gBrowseAll_ServiceTypes[ 0 ] );
		for( i = 1; i < gBrowseAll_ServiceTypesCount; ++i )
		{
			FPrintF( stdout, ", %s", gBrowseAll_ServiceTypes[ i ] );
		}
		FPrintF( stdout, "\n" );
	}
	else
	{
		FPrintF( stdout, "all services\n" );
	}
	FPrintF( stdout, "Domain:          %s\n", gBrowseAll_Domain ? gBrowseAll_Domain : "default domains" );
	FPrintF( stdout, "Browse time:     %d second%?c\n", gBrowseAll_BrowseTimeSecs, gBrowseAll_BrowseTimeSecs != 1, 's' );
	FPrintF( stdout, "Connect timeout: %d second%?c\n",
		context->connectionTimeoutSecs, context->connectionTimeoutSecs != 1, 's' );
	FPrintF( stdout, "IncludeAWDL:     %s\n", gDNSSDFlag_IncludeAWDL ? "yes" : "no" );
	FPrintF( stdout, "Start time:      %{du:time}\n", NULL );
	FPrintF( stdout, "---\n" );
	
	err = ServiceBrowserCreate( dispatch_get_main_queue(), ifIndex, gBrowseAll_Domain,
		(unsigned int) gBrowseAll_BrowseTimeSecs, gDNSSDFlag_IncludeAWDL ? true : false, &context->browser );
	require_noerr( err, exit );
	
	for( i = 0; i < gBrowseAll_ServiceTypesCount; ++i )
	{
		err = ServiceBrowserAddServiceType( context->browser, gBrowseAll_ServiceTypes[ i ] );
		require_noerr( err, exit );
	}
	ServiceBrowserSetCallback( context->browser, _BrowseAllServiceBrowserCallback, context );
	ServiceBrowserStart( context->browser );
	dispatch_main();
	
exit:
	if( context ) _BrowseAllContextFree( context );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	_BrowseAllContextFree
//===========================================================================================================================

static void	_BrowseAllContextFree( BrowseAllContext *inContext )
{
	check( !inContext->browser );
	check( !inContext->connectionTimer );
	check( !inContext->connectionList );
	ForgetServiceBrowserResults( &inContext->results );
	free( inContext );
}

//===========================================================================================================================
//	_BrowseAllServiceBrowserCallback
//===========================================================================================================================

#define kDiscardProtocolPort		9

static void	_BrowseAllServiceBrowserCallback( ServiceBrowserResults *inResults, OSStatus inError, void *inContext )
{
	OSStatus						err;
	BrowseAllContext * const		context = (BrowseAllContext *) inContext;
	SBRDomain *						domain;
	SBRServiceType *				type;
	SBRServiceInstance *			instance;
	SBRIPAddress *					ipaddr;
	
	Unused( inError );
	
	require_action( inResults, exit, err = kUnexpectedErr );
	
	check( !context->results );
	context->results = inResults;
	ServiceBrowserResultsRetain( context->results );
	
	check( context->connectionPendingCount == 0 );
	if( context->connectionTimeoutSecs > 0 )
	{
		BrowseAllConnection *			connection;
		BrowseAllConnection **			connectionPtr = &context->connectionList;
		char							destination[ kSockAddrStringMaxSize ];
		
		for( domain = context->results->domainList; domain; domain = domain->next )
		{
			for( type = domain->typeList; type; type = type->next )
			{
				if( !_IsServiceTypeTCP( type->name ) ) continue;
				for( instance = type->instanceList; instance; instance = instance->next )
				{
					if( instance->port == kDiscardProtocolPort ) continue;
					for( ipaddr = instance->ipaddrList; ipaddr; ipaddr = ipaddr->next )
					{
						err = _BrowseAllConnectionCreate( &ipaddr->sip.sa, instance->port, context, &connection );
						require_noerr( err, exit );
						
						*connectionPtr = connection;
						 connectionPtr = &connection->next;
						
						err = SockAddrToString( &ipaddr->sip, kSockAddrStringFlagsNoPort, destination );
						check_noerr( err );
						if( !err )
						{
							err = AsyncConnection_Connect( &connection->asyncCnx, destination, -instance->port,
								kAsyncConnectionFlag_P2P, kAsyncConnectionNoTimeout,
								kSocketBufferSize_DontSet, kSocketBufferSize_DontSet,
								_BrowseAllConnectionProgress, connection, _BrowseAllConnectionHandler, connection,
								dispatch_get_main_queue() );
							check_noerr( err );
						}
						if( !err )
						{
							_BrowseAllConnectionRetain( connection );
							connection->status = kInProgressErr;
							++context->connectionPendingCount;
						}
						else
						{
							connection->status = err;
						}
					}
				}
			}
		}
	}
	
	if( context->connectionPendingCount > 0 )
	{
		check( !context->connectionTimer );
		err = DispatchTimerCreate( dispatch_time_seconds( context->connectionTimeoutSecs ), DISPATCH_TIME_FOREVER,
			100 * kNanosecondsPerMillisecond, NULL, _BrowseAllExit, NULL, context, &context->connectionTimer );
		require_noerr( err, exit );
		dispatch_resume( context->connectionTimer );
	}
	else
	{
		dispatch_async_f( dispatch_get_main_queue(), context, _BrowseAllExit );
	}
	err = kNoErr;
	
exit:
	ForgetCF( &context->browser );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	_BrowseAllConnectionCreate
//===========================================================================================================================

static OSStatus
	_BrowseAllConnectionCreate(
		const struct sockaddr *	inSockAddr,
		uint16_t				inPort,
		BrowseAllContext *		inContext,
		BrowseAllConnection **	outConnection )
{
	OSStatus					err;
	BrowseAllConnection *		obj;
	
	obj = (BrowseAllConnection *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	obj->refCount	= 1;
	SockAddrCopy( inSockAddr, &obj->sip );
	obj->port		= inPort;
	obj->context	= inContext;
	
	*outConnection = obj;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_BrowseAllConnectionRetain
//===========================================================================================================================

static void _BrowseAllConnectionRetain( BrowseAllConnection *inConnection )
{
	++inConnection->refCount;
}

//===========================================================================================================================
//	_BrowseAllConnectionRelease
//===========================================================================================================================

static void	_BrowseAllConnectionRelease( BrowseAllConnection *inConnection )
{
	if( --inConnection->refCount == 0 ) free( inConnection );
}

//===========================================================================================================================
//	_BrowseAllConnectionProgress
//===========================================================================================================================

static void	_BrowseAllConnectionProgress( int inPhase, const void *inDetails, void *inArg )
{
	BrowseAllConnection * const		connection = (BrowseAllConnection *) inArg;
	
	if( inPhase == kAsyncConnectionPhase_Connected )
	{
		const AsyncConnectedInfo * const		info = (AsyncConnectedInfo *) inDetails;
		
		connection->connectTimeSecs = info->connectSecs;
	}
}

//===========================================================================================================================
//	_BrowseAllConnectionHandler
//===========================================================================================================================

static void	_BrowseAllConnectionHandler( SocketRef inSock, OSStatus inError, void *inArg )
{
	BrowseAllConnection * const		connection	= (BrowseAllConnection *) inArg;
	BrowseAllContext * const		context		= connection->context;
	
	connection->status = inError;
	ForgetSocket( &inSock );
	if( context )
	{
		check( context->connectionPendingCount > 0 );
		if( ( --context->connectionPendingCount == 0 ) && context->connectionTimer )
		{
			dispatch_source_forget( &context->connectionTimer );
			dispatch_async_f( dispatch_get_main_queue(), context, _BrowseAllExit );
		}
	}
	_BrowseAllConnectionRelease( connection );
}

//===========================================================================================================================
//	_BrowseAllExit
//===========================================================================================================================

#define Indent( X )		( (X) * 4 ), ""

static void	_BrowseAllExit( void *inContext )
{
	BrowseAllContext * const		context		= (BrowseAllContext *) inContext;
	SBRDomain *						domain;
	SBRServiceType *				type;
	SBRServiceInstance *			instance;
	SBRIPAddress *					ipaddr;
	char							textBuf[ 512 ];
#if( TARGET_OS_POSIX )
	const Boolean					useColor	= isatty( STDOUT_FILENO ) ? true : false;
#endif
	
	dispatch_source_forget( &context->connectionTimer );
	
	for( domain = context->results->domainList; domain; domain = domain->next )
	{
		FPrintF( stdout, "%s\n\n", domain->name );
		
		for( type = domain->typeList; type; type = type->next )
		{
			const char *		description;
			const Boolean		serviceTypeIsTCP = _IsServiceTypeTCP( type->name );
			
			description = ServiceTypeDescription( type->name );
			if( description )	FPrintF( stdout, "%*s" "%s (%s)\n\n",	Indent( 1 ), description, type->name );
			else				FPrintF( stdout, "%*s" "%s\n\n",		Indent( 1 ), type->name );
			
			for( instance = type->instanceList; instance; instance = instance->next )
			{
				char *				dst = textBuf;
				char * const		lim = &textBuf[ countof( textBuf ) ];
				char				ifname[ IF_NAMESIZE + 1 ];
				
				SNPrintF_Add( &dst, lim, "%s via ", instance->name );
				if( instance->ifIndex == 0 )
				{
					SNPrintF_Add( &dst, lim, "the Internet" );
				}
				else if( if_indextoname( instance->ifIndex, ifname ) )
				{
					NetTransportType		netType;
					
					SocketGetInterfaceInfo( kInvalidSocketRef, ifname, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &netType );
					SNPrintF_Add( &dst, lim, "%s (%s)",
						( netType == kNetTransportType_Ethernet ) ? "Ethernet" : NetTransportTypeToString( netType ),
						ifname );
				}
				else
				{
					SNPrintF_Add( &dst, lim, "interface index %u", instance->ifIndex );
				}
				FPrintF( stdout, "%*s" "%-55s %4llu.%03llu ms\n\n",
					Indent( 2 ), textBuf, instance->discoverTimeUs / 1000, instance->discoverTimeUs % 1000 );
				
				if( instance->hostname )
				{
					SNPrintF( textBuf, sizeof( textBuf ), "%s:%u", instance->hostname, instance->port );
					FPrintF( stdout, "%*s" "%-51s %4llu.%03llu ms\n",
						Indent( 3 ), textBuf, instance->resolveTimeUs / 1000, instance->resolveTimeUs % 1000 );
				}
				else
				{
					FPrintF( stdout, "%*s" "%s:%u\n", Indent( 3 ), instance->hostname, instance->port );
				}
				
				for( ipaddr = instance->ipaddrList; ipaddr; ipaddr = ipaddr->next )
				{
					BrowseAllConnection *		conn;
					BrowseAllConnection **		connPtr;
					
					FPrintF( stdout, "%*s" "%-##47a %4llu.%03llu ms",
						Indent( 4 ), &ipaddr->sip.sa, ipaddr->resolveTimeUs / 1000, ipaddr->resolveTimeUs % 1000 );
					
					conn = NULL;
					if( serviceTypeIsTCP && ( instance->port != kDiscardProtocolPort ) )
					{
						for( connPtr = &context->connectionList; ( conn = *connPtr ) != NULL; connPtr = &conn->next )
						{
							if( ( conn->port == instance->port ) &&
								( SockAddrCompareAddr( &conn->sip, &ipaddr->sip ) == 0 ) ) break;
						}
						if( conn )
						{
							if( conn->status == kInProgressErr ) conn->status = kTimeoutErr;
							*connPtr = conn->next;
							conn->context = NULL;
							AsyncConnection_Forget( &conn->asyncCnx );
						}
					}
					
					if( conn )
					{
						if( conn->status == kNoErr )
						{
							FPrintF( stdout, " (%sconnected%s in %.3f ms)\n",
								useColor ? kANSIGreen : "", useColor ? kANSINormal : "", conn->connectTimeSecs * 1000 );
						}
						else
						{
							FPrintF( stdout, " (%scould not connect%s: %m)\n",
								useColor ? kANSIRed : "", useColor ? kANSINormal : "", conn->status );
						}
						_BrowseAllConnectionRelease( conn );
					}
					else
					{
						FPrintF( stdout, " (no connection attempted)\n" );
					}
				}
				
				FPrintF( stdout, "\n" );
				if( instance->txtLen == 0 ) continue;
				
				FPrintF( stdout, "%*s" "TXT record (%zu byte%?c):\n",
					Indent( 3 ), instance->txtLen, instance->txtLen != 1, 's' );
				if( instance->txtLen > 1 )
				{
					FPrintF( stdout, "%3{txt}", instance->txtPtr, instance->txtLen );
				}
				else
				{
					FPrintF( stdout, "%*s" "%#H\n", Indent( 3 ), instance->txtPtr, (int) instance->txtLen, INT_MAX );
				}
				FPrintF( stdout, "\n" );
			}
			FPrintF( stdout, "\n" );
		}
	}
	
	_BrowseAllContextFree( context );
	Exit( NULL );
}

//===========================================================================================================================
//	_IsServiceTypeTCP
//===========================================================================================================================

static Boolean	_IsServiceTypeTCP( const char *inServiceType )
{
	OSStatus			err;
	const uint8_t *		secondLabel;
	uint8_t				name[ kDomainNameLengthMax ];
	
	err = DomainNameFromString( name, inServiceType, NULL );
	if( !err )
	{
		secondLabel = DomainNameGetNextLabel( name );
		if( secondLabel && DomainNameEqual( secondLabel, (const uint8_t *) "\x04" "_tcp" ) ) return( true );
	}
	return( false );
}

//===========================================================================================================================
//	GetNameInfoCmd
//===========================================================================================================================

const FlagStringPair		kGetNameInfoFlagStringPairs[] =
{
	CaseFlagStringify( NI_NUMERICSCOPE ),
	CaseFlagStringify( NI_DGRAM ),
	CaseFlagStringify( NI_NUMERICSERV ),
	CaseFlagStringify( NI_NAMEREQD ),
	CaseFlagStringify( NI_NUMERICHOST ),
	CaseFlagStringify( NI_NOFQDN ),
	{ 0, NULL }
};

static void	GetNameInfoCmd( void )
{
	OSStatus					err;
	sockaddr_ip					sip;
	size_t						sockAddrLen;
	unsigned int				flags;
	const FlagStringPair *		pair;
	struct timeval				now;
	char						host[ NI_MAXHOST ];
	char						serv[ NI_MAXSERV ];
	
	err = StringToSockAddr( gGetNameInfo_IPAddress, &sip, sizeof( sip ), &sockAddrLen );
	check_noerr( err );
	if( err )
	{
		FPrintF( stderr, "Failed to convert \"%s\" to a sockaddr.\n", gGetNameInfo_IPAddress );
		goto exit;
	}
	
	flags = 0;
	if( gGetNameInfoFlag_DGram )		flags |= NI_DGRAM;
	if( gGetNameInfoFlag_NameReqd )		flags |= NI_NAMEREQD;
	if( gGetNameInfoFlag_NoFQDN )		flags |= NI_NOFQDN;
	if( gGetNameInfoFlag_NumericHost )	flags |= NI_NUMERICHOST;
	if( gGetNameInfoFlag_NumericScope )	flags |= NI_NUMERICSCOPE;
	if( gGetNameInfoFlag_NumericServ )	flags |= NI_NUMERICSERV;
	
	// Print prologue.
	
	FPrintF( stdout, "SockAddr:   %##a\n",	&sip.sa );
	FPrintF( stdout, "Flags:      0x%X < ",	flags );
	for( pair = kGetNameInfoFlagStringPairs; pair->str != NULL; ++pair )
	{
		if( flags & pair->flag ) FPrintF( stdout, "%s ", pair->str );
	}
	FPrintF( stdout, ">\n" );
	FPrintF( stdout, "Start time: %{du:time}\n", NULL );
	FPrintF( stdout, "---\n" );
	
	// Call getnameinfo().
	
	err = getnameinfo( &sip.sa, (socklen_t) sockAddrLen, host, (socklen_t) sizeof( host ), serv, (socklen_t) sizeof( serv ),
		(int) flags );
	gettimeofday( &now, NULL );
	if( err )
	{
		FPrintF( stderr, "Error %d: %s.\n", err, gai_strerror( err ) );
	}
	else
	{
		FPrintF( stdout, "host: %s\n", host );
		FPrintF( stdout, "serv: %s\n", serv );
	}
	FPrintF( stdout, "---\n" );
	FPrintF( stdout, "End time:   %{du:time}\n", &now );
	
exit:
	gExitCode = err ? 1 : 0;
}

//===========================================================================================================================
//	GetAddrInfoStressCmd
//===========================================================================================================================

typedef struct
{
	DNSServiceRef			mainRef;
	DNSServiceRef			sdRef;
	DNSServiceFlags			flags;
	unsigned int			interfaceIndex;
	unsigned int			connectionNumber;
	unsigned int			requestCount;
	unsigned int			requestCountMax;
	unsigned int			requestCountLimit;
	unsigned int			durationMinMs;
	unsigned int			durationMaxMs;
	
}	GAIStressContext;

static void	GetAddrInfoStressEvent( void *inContext );
static void	DNSSD_API
	GetAddrInfoStressCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inHostname,
		const struct sockaddr *	inSockAddr,
		uint32_t				inTTL,
		void *					inContext );

static void	GetAddrInfoStressCmd( void )
{
	OSStatus				err;
	GAIStressContext *		context = NULL;
	int						i;
	DNSServiceFlags			flags;
	uint32_t				ifIndex;
	char					ifName[ kInterfaceNameBufLen ];
	
	if( gGAIStress_TestDurationSecs < 0 )
	{
		FPrintF( stdout, "Invalid test duration: %d s.\n", gGAIStress_TestDurationSecs );
		err = kParamErr;
		goto exit;
	}
	if( gGAIStress_ConnectionCount <= 0 )
	{
		FPrintF( stdout, "Invalid simultaneous connection count: %d.\n", gGAIStress_ConnectionCount );
		err = kParamErr;
		goto exit;
	}
	if( gGAIStress_DurationMinMs <= 0 )
	{
		FPrintF( stdout, "Invalid minimum DNSServiceGetAddrInfo() duration: %d ms.\n", gGAIStress_DurationMinMs );
		err = kParamErr;
		goto exit;
	}
	if( gGAIStress_DurationMaxMs <= 0 )
	{
		FPrintF( stdout, "Invalid maximum DNSServiceGetAddrInfo() duration: %d ms.\n", gGAIStress_DurationMaxMs );
		err = kParamErr;
		goto exit;
	}
	if( gGAIStress_DurationMinMs > gGAIStress_DurationMaxMs )
	{
		FPrintF( stdout, "Invalid minimum and maximum DNSServiceGetAddrInfo() durations: %d ms and %d ms.\n",
			gGAIStress_DurationMinMs, gGAIStress_DurationMaxMs );
		err = kParamErr;
		goto exit;
	}
	if( gGAIStress_RequestCountMax <= 0 )
	{
		FPrintF( stdout, "Invalid maximum request count: %d.\n", gGAIStress_RequestCountMax );
		err = kParamErr;
		goto exit;
	}
	
	// Set flags.
	
	flags = GetDNSSDFlagsFromOpts();
	
	// Set interface index.
	
	err = InterfaceIndexFromArgString( gInterface, &ifIndex );
	require_noerr_quiet( err, exit );
	
	for( i = 0; i < gGAIStress_ConnectionCount; ++i )
	{
		context = (GAIStressContext *) calloc( 1, sizeof( *context ) );
		require_action( context, exit, err = kNoMemoryErr );
		
		context->flags				= flags;
		context->interfaceIndex		= ifIndex;
		context->connectionNumber	= (unsigned int)( i + 1 );
		context->requestCountMax	= (unsigned int) gGAIStress_RequestCountMax;
		context->durationMinMs		= (unsigned int) gGAIStress_DurationMinMs;
		context->durationMaxMs		= (unsigned int) gGAIStress_DurationMaxMs;
		
		dispatch_async_f( dispatch_get_main_queue(), context, GetAddrInfoStressEvent );
		context = NULL;
	}
	
	if( gGAIStress_TestDurationSecs > 0 )
	{
		dispatch_after_f( dispatch_time_seconds( gGAIStress_TestDurationSecs ), dispatch_get_main_queue(), NULL, Exit );
	}
	
	FPrintF( stdout, "Flags:                %#{flags}\n",	flags, kDNSServiceFlagsDescriptors );
	FPrintF( stdout, "Interface:            %d (%s)\n",		(int32_t) ifIndex, InterfaceIndexToName( ifIndex, ifName ) );
	FPrintF( stdout, "Test duration:        " );
	if( gGAIStress_TestDurationSecs == 0 )
	{
		FPrintF( stdout, "∞\n" );
	}
	else
	{
		FPrintF( stdout, "%d s\n", gGAIStress_TestDurationSecs );
	}
	FPrintF( stdout, "Connection count:     %d\n",			gGAIStress_ConnectionCount );
	FPrintF( stdout, "Request duration min: %d ms\n",		gGAIStress_DurationMinMs );
	FPrintF( stdout, "Request duration max: %d ms\n",		gGAIStress_DurationMaxMs );
	FPrintF( stdout, "Request count max:    %d\n",			gGAIStress_RequestCountMax );
	FPrintF( stdout, "Start time:           %{du:time}\n",	NULL);
	FPrintF( stdout, "---\n" );
	
	dispatch_main();
	
exit:
	FreeNullSafe( context );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	GetAddrInfoStressEvent
//===========================================================================================================================

#define kStressRandStrLen		5

#define kLowercaseAlphaCharSet		"abcdefghijklmnopqrstuvwxyz"

static void	GetAddrInfoStressEvent( void *inContext )
{
	GAIStressContext * const		context = (GAIStressContext *) inContext;
	OSStatus						err;
	DNSServiceRef					sdRef;
	unsigned int					nextMs;
	char							randomStr[ kStressRandStrLen + 1 ];
	char							hostname[ kStressRandStrLen + 4 + 1 ];
	Boolean							isConnectionNew	= false;
	static Boolean					printedHeader	= false;
	
	if( !context->mainRef || ( context->requestCount >= context->requestCountLimit ) )
	{
		DNSServiceForget( &context->mainRef );
		context->sdRef				= NULL;
		context->requestCount		= 0;
		context->requestCountLimit	= RandomRange( 1, context->requestCountMax );
		
		err = DNSServiceCreateConnection( &context->mainRef );
		require_noerr( err, exit );
		
		err = DNSServiceSetDispatchQueue( context->mainRef, dispatch_get_main_queue() );
		require_noerr( err, exit );
		
		isConnectionNew = true;
	}
	
	RandomString( kLowercaseAlphaCharSet, sizeof_string( kLowercaseAlphaCharSet ), 2, kStressRandStrLen, randomStr );
	SNPrintF( hostname, sizeof( hostname ), "%s.com", randomStr );
	
	nextMs = RandomRange( context->durationMinMs, context->durationMaxMs );
	
	if( !printedHeader )
	{
		FPrintF( stdout, "%-26s Conn  Hostname Dur (ms)\n", "Timestamp" );
		printedHeader = true;
	}
	FPrintF( stdout, "%{du:time} %3u%c %9s %8u\n",
		NULL, context->connectionNumber, isConnectionNew ? '*': ' ', hostname, nextMs );
	
	DNSServiceForget( &context->sdRef );
	sdRef = context->mainRef;
	err = DNSServiceGetAddrInfo( &sdRef, context->flags | kDNSServiceFlagsShareConnection, context->interfaceIndex,
		kDNSServiceProtocol_IPv4 | kDNSServiceProtocol_IPv6, hostname, GetAddrInfoStressCallback, NULL );
	require_noerr( err, exit );
	context->sdRef = sdRef;
	
	context->requestCount++;
	
	dispatch_after_f( dispatch_time_milliseconds( nextMs ), dispatch_get_main_queue(), context, GetAddrInfoStressEvent );
	
exit:
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	GetAddrInfoStressCallback
//===========================================================================================================================

static void DNSSD_API
	GetAddrInfoStressCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inHostname,
		const struct sockaddr *	inSockAddr,
		uint32_t				inTTL,
		void *					inContext )
{
	Unused( inSDRef );
	Unused( inFlags );
	Unused( inInterfaceIndex );
	Unused( inError );
	Unused( inHostname );
	Unused( inSockAddr );
	Unused( inTTL );
	Unused( inContext );
}

//===========================================================================================================================
//	DNSQueryCmd
//===========================================================================================================================

typedef struct
{
	sockaddr_ip				serverAddr;
	uint64_t				sendTicks;
	uint8_t *				msgPtr;
	size_t					msgLen;
	size_t					msgOffset;
	const char *			name;
	dispatch_source_t		readSource;
	SocketRef				sock;
	int						timeLimitSecs;
	uint16_t				queryID;
	uint16_t				type;
	Boolean					haveTCPLen;
	Boolean					useTCP;
	Boolean					printRawRData;	// True if RDATA results are not to be formatted.
	uint8_t					msgBuf[ 512 ];
	
}	DNSQueryContext;

static void	DNSQueryPrintPrologue( const DNSQueryContext *inContext );
static void	DNSQueryReadHandler( void *inContext );
static void	DNSQueryCancelHandler( void *inContext );

static void	DNSQueryCmd( void )
{
	OSStatus				err;
	DNSQueryContext *		context = NULL;
	uint8_t *				msgPtr;
	size_t					msgLen, sendLen;
	
	// Check command parameters.
	
	if( gDNSQuery_TimeLimitSecs < -1 )
	{
		FPrintF( stdout, "Invalid time limit: %d seconds.\n", gDNSQuery_TimeLimitSecs );
		err = kParamErr;
		goto exit;
	}
	if( ( gDNSQuery_Flags < INT16_MIN ) || ( gDNSQuery_Flags > UINT16_MAX ) )
	{
		FPrintF( stdout, "DNS flags-and-codes value is out of the unsigned 16-bit range: 0x%08X.\n", gDNSQuery_Flags );
		err = kParamErr;
		goto exit;
	}
	
	// Create context.
	
	context = (DNSQueryContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	context->name			= gDNSQuery_Name;
	context->sock			= kInvalidSocketRef;
	context->timeLimitSecs	= gDNSQuery_TimeLimitSecs;
	context->queryID		= (uint16_t) Random32();
	context->useTCP			= gDNSQuery_UseTCP	 ? true : false;
	context->printRawRData	= gDNSQuery_RawRData ? true : false;
	
#if( TARGET_OS_DARWIN )
	if( gDNSQuery_Server )
#endif
	{
		err = StringToSockAddr( gDNSQuery_Server, &context->serverAddr, sizeof( context->serverAddr ), NULL );
		require_noerr( err, exit );
	}
#if( TARGET_OS_DARWIN )
	else
	{
		err = GetDefaultDNSServer( &context->serverAddr );
		require_noerr( err, exit );
	}
#endif
	if( SockAddrGetPort( &context->serverAddr ) == 0 ) SockAddrSetPort( &context->serverAddr, kDNSPort );
	
	err = RecordTypeFromArgString( gDNSQuery_Type, &context->type );
	require_noerr( err, exit );
	
	// Write query message.
	
	check_compile_time_code( sizeof( context->msgBuf ) >= ( kDNSQueryMessageMaxLen + 2 ) );
	
	msgPtr = context->useTCP ? &context->msgBuf[ 2 ] : context->msgBuf;
	err = WriteDNSQueryMessage( msgPtr, context->queryID, (uint16_t) gDNSQuery_Flags, context->name, context->type,
		kDNSServiceClass_IN, &msgLen );
	require_noerr( err, exit );
	check( msgLen <= UINT16_MAX );
	
	if( context->useTCP )
	{
		WriteBig16( context->msgBuf, msgLen );
		sendLen = 2 + msgLen;
	}
	else
	{
		sendLen = msgLen;
	}
	
	DNSQueryPrintPrologue( context );
	
	if( gDNSQuery_Verbose )
	{
		FPrintF( stdout, "DNS message to send:\n\n%{du:dnsmsg}", msgPtr, msgLen );
		FPrintF( stdout, "---\n" );
	}
	
	if( context->useTCP )
	{
		// Create TCP socket.
		
		context->sock = socket( context->serverAddr.sa.sa_family, SOCK_STREAM, IPPROTO_TCP );
		err = map_socket_creation_errno( context->sock );
		require_noerr( err, exit );
		
		err = SocketConnect( context->sock, &context->serverAddr, 5 );
		require_noerr( err, exit );
	}
	else
	{
		// Create UDP socket.
		
		err = UDPClientSocketOpen( AF_UNSPEC, &context->serverAddr, 0, -1, NULL, &context->sock );
		require_noerr( err, exit );
	}
	
	context->sendTicks = UpTicks();
	err = _SocketWriteAll( context->sock, context->msgBuf, sendLen, 5 );
	require_noerr( err, exit );
	
	if( context->timeLimitSecs == 0 ) goto exit;
	
	err = DispatchReadSourceCreate( context->sock, NULL, DNSQueryReadHandler, DNSQueryCancelHandler, context,
		&context->readSource );
	require_noerr( err, exit );
	dispatch_resume( context->readSource );
	
	if( context->timeLimitSecs > 0 )
	{
		dispatch_after_f( dispatch_time_seconds( context->timeLimitSecs ), dispatch_get_main_queue(), kExitReason_Timeout,
			Exit );
	}
	dispatch_main();
	
exit:
	if( context )
	{
		dispatch_source_forget( &context->readSource );
		ForgetSocket( &context->sock );
		free( context );
	}
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	DNSQueryPrintPrologue
//===========================================================================================================================

static void	DNSQueryPrintPrologue( const DNSQueryContext *inContext )
{
	const int		timeLimitSecs = inContext->timeLimitSecs;
	
	FPrintF( stdout, "Name:        %s\n",		inContext->name );
	FPrintF( stdout, "Type:        %s (%u)\n",	RecordTypeToString( inContext->type ), inContext->type );
	FPrintF( stdout, "Server:      %##a\n",		&inContext->serverAddr );
	FPrintF( stdout, "Transport:   %s\n",		inContext->useTCP ? "TCP" : "UDP" );
	FPrintF( stdout, "Time limit:  " );
	if( timeLimitSecs >= 0 )	FPrintF( stdout, "%d second%?c\n", timeLimitSecs, timeLimitSecs != 1, 's' );
	else						FPrintF( stdout, "∞\n" );
	FPrintF( stdout, "Start time:  %{du:time}\n", NULL );
	FPrintF( stdout, "---\n" );
}

//===========================================================================================================================
//	DNSQueryReadHandler
//===========================================================================================================================

static void	DNSQueryReadHandler( void *inContext )
{
	OSStatus					err;
	struct timeval				now;
	const uint64_t				nowTicks	= UpTicks();
	DNSQueryContext * const		context		= (DNSQueryContext *) inContext;
	
	gettimeofday( &now, NULL );
	
	if( context->useTCP )
	{
		if( !context->haveTCPLen )
		{
			err = SocketReadData( context->sock, &context->msgBuf, 2, &context->msgOffset );
			if( err == EWOULDBLOCK ) { err = kNoErr; goto exit; }
			require_noerr( err, exit );
			
			context->msgOffset	= 0;
			context->msgLen		= ReadBig16( context->msgBuf );
			context->haveTCPLen	= true;
			if( context->msgLen <= sizeof( context->msgBuf ) )
			{
				context->msgPtr = context->msgBuf;
			}
			else
			{
				context->msgPtr = (uint8_t *) malloc( context->msgLen );
				require_action( context->msgPtr, exit, err = kNoMemoryErr );
			}
		}
		
		err = SocketReadData( context->sock, context->msgPtr, context->msgLen, &context->msgOffset );
		if( err == EWOULDBLOCK ) { err = kNoErr; goto exit; }
		require_noerr( err, exit );
		context->msgOffset	= 0;
		context->haveTCPLen	= false;
	}
	else
	{
		sockaddr_ip		fromAddr;
		
		context->msgPtr = context->msgBuf;
		err = SocketRecvFrom( context->sock, context->msgPtr, sizeof( context->msgBuf ), &context->msgLen, &fromAddr,
			sizeof( fromAddr ), NULL, NULL, NULL, NULL );
		require_noerr( err, exit );
		
		check( SockAddrCompareAddr( &fromAddr, &context->serverAddr ) == 0 );
	}
	
	FPrintF( stdout, "Receive time: %{du:time}\n",	&now );
	FPrintF( stdout, "Source:       %##a\n",		&context->serverAddr );
	FPrintF( stdout, "Message size: %zu\n",			context->msgLen );
	FPrintF( stdout, "RTT:          %llu ms\n\n",	UpTicksToMilliseconds( nowTicks - context->sendTicks ) );
	FPrintF( stdout, "%.*{du:dnsmsg}", context->printRawRData ? 1 : 0, context->msgPtr, context->msgLen );
	
	if( ( context->msgLen >= kDNSHeaderLength ) && ( DNSHeaderGetID( (DNSHeader *) context->msgPtr ) == context->queryID ) )
	{
		Exit( kExitReason_ReceivedResponse );
	}
	
exit:
	if( err ) dispatch_source_forget( &context->readSource );
}

//===========================================================================================================================
//	DNSQueryCancelHandler
//===========================================================================================================================

static void	DNSQueryCancelHandler( void *inContext )
{
	DNSQueryContext * const		context = (DNSQueryContext *) inContext;
	
	check( !context->readSource );
	ForgetSocket( &context->sock );
	if( context->msgPtr != context->msgBuf ) ForgetMem( &context->msgPtr );
	free( context );
	dispatch_async_f( dispatch_get_main_queue(), NULL, Exit );
}

#if( DNSSDUTIL_INCLUDE_DNSCRYPT )
//===========================================================================================================================
//	DNSCryptCmd
//===========================================================================================================================

#define kDNSCryptPort		443

#define kDNSCryptMinPadLength				8
#define kDNSCryptMaxPadLength				256
#define kDNSCryptBlockSize					64
#define kDNSCryptCertMinimumLength			124
#define kDNSCryptClientMagicLength			8
#define kDNSCryptResolverMagicLength		8
#define kDNSCryptHalfNonceLength			12
#define kDNSCryptCertMagicLength			4

check_compile_time( ( kDNSCryptHalfNonceLength * 2 ) == crypto_box_NONCEBYTES );

static const uint8_t		kDNSCryptCertMagic[ kDNSCryptCertMagicLength ] = { 'D', 'N', 'S', 'C' };
static const uint8_t		kDNSCryptResolverMagic[ kDNSCryptResolverMagicLength ] =
{
	0x72, 0x36, 0x66, 0x6e, 0x76, 0x57, 0x6a, 0x38
};

typedef struct
{
	uint8_t		certMagic[ kDNSCryptCertMagicLength ];
	uint8_t		esVersion[ 2 ];
	uint8_t		minorVersion[ 2 ];
	uint8_t		signature[ crypto_sign_BYTES ];
	uint8_t		publicKey[ crypto_box_PUBLICKEYBYTES ];
	uint8_t		clientMagic[ kDNSCryptClientMagicLength ];
	uint8_t		serial[ 4 ];
	uint8_t		startTime[ 4 ];
	uint8_t		endTime[ 4 ];
	uint8_t		extensions[ 1 ];	// Variably-sized extension data.
	
}	DNSCryptCert;

check_compile_time( offsetof( DNSCryptCert, extensions ) == kDNSCryptCertMinimumLength );

typedef struct
{
	uint8_t		clientMagic[ kDNSCryptClientMagicLength ];
	uint8_t		clientPublicKey[ crypto_box_PUBLICKEYBYTES ];
	uint8_t		clientNonce[ kDNSCryptHalfNonceLength ];
	uint8_t		poly1305MAC[ 16 ];
	
}	DNSCryptQueryHeader;

check_compile_time( sizeof( DNSCryptQueryHeader ) == 68 );
check_compile_time( sizeof( DNSCryptQueryHeader ) >= crypto_box_ZEROBYTES );
check_compile_time( ( sizeof( DNSCryptQueryHeader ) - crypto_box_ZEROBYTES + crypto_box_BOXZEROBYTES ) ==
	offsetof( DNSCryptQueryHeader, poly1305MAC ) );

typedef struct
{
	uint8_t		resolverMagic[ kDNSCryptResolverMagicLength ];
	uint8_t		clientNonce[ kDNSCryptHalfNonceLength ];
	uint8_t		resolverNonce[ kDNSCryptHalfNonceLength ];
	uint8_t		poly1305MAC[ 16 ];
	
}	DNSCryptResponseHeader;

check_compile_time( sizeof( DNSCryptResponseHeader ) == 48 );
check_compile_time( offsetof( DNSCryptResponseHeader, poly1305MAC ) >= crypto_box_BOXZEROBYTES );
check_compile_time( ( offsetof( DNSCryptResponseHeader, poly1305MAC ) - crypto_box_BOXZEROBYTES + crypto_box_ZEROBYTES ) ==
	sizeof( DNSCryptResponseHeader ) );

typedef struct
{
	sockaddr_ip				serverAddr;
	uint64_t				sendTicks;
	const char *			providerName;
	const char *			qname;
	const uint8_t *			certPtr;
	size_t					certLen;
	dispatch_source_t		readSource;
	size_t					msgLen;
	int						timeLimitSecs;
	uint16_t				queryID;
	uint16_t				qtype;
	Boolean					printRawRData;
	uint8_t					serverPublicSignKey[ crypto_sign_PUBLICKEYBYTES ];
	uint8_t					serverPublicKey[ crypto_box_PUBLICKEYBYTES ];
	uint8_t					clientPublicKey[ crypto_box_PUBLICKEYBYTES ];
	uint8_t					clientSecretKey[ crypto_box_SECRETKEYBYTES ];
	uint8_t					clientMagic[ kDNSCryptClientMagicLength ];
	uint8_t					clientNonce[ kDNSCryptHalfNonceLength ];
	uint8_t					nmKey[ crypto_box_BEFORENMBYTES ];
	uint8_t					msgBuf[ 512 ];
	
}	DNSCryptContext;

static void		DNSCryptReceiveCertHandler( void *inContext );
static void		DNSCryptReceiveResponseHandler( void *inContext );
static void		DNSCryptProceed( void *inContext );
static OSStatus	DNSCryptProcessCert( DNSCryptContext *inContext );
static OSStatus	DNSCryptBuildQuery( DNSCryptContext *inContext );
static OSStatus	DNSCryptSendQuery( DNSCryptContext *inContext );
static void		DNSCryptPrintCertificate( const DNSCryptCert *inCert, size_t inLen );

static void	DNSCryptCmd( void )
{
	OSStatus				err;
	DNSCryptContext *		context		= NULL;
	size_t					writtenBytes;
	size_t					totalBytes;
	SocketContext *			sockCtx;
	SocketRef				sock		= kInvalidSocketRef;
	const char *			ptr;
	
	// Check command parameters.
	
	if( gDNSCrypt_TimeLimitSecs < -1 )
	{
		FPrintF( stdout, "Invalid time limit: %d seconds.\n", gDNSCrypt_TimeLimitSecs );
		err = kParamErr;
		goto exit;
	}
	
	// Create context.
	
	context = (DNSCryptContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	context->providerName	= gDNSCrypt_ProviderName;
	context->qname			= gDNSCrypt_Name;
	context->timeLimitSecs	= gDNSCrypt_TimeLimitSecs;
	context->printRawRData	= gDNSCrypt_RawRData ? true : false;
	
	err = crypto_box_keypair( context->clientPublicKey, context->clientSecretKey );
	require_noerr( err, exit );
	
	err = HexToData( gDNSCrypt_ProviderKey, kSizeCString, kHexToData_DefaultFlags,
		context->serverPublicSignKey, sizeof( context->serverPublicSignKey ), &writtenBytes, &totalBytes, &ptr );
	if( err || ( *ptr != '\0' ) )
	{
		FPrintF( stderr, "Failed to parse public signing key hex string (%s).\n", gDNSCrypt_ProviderKey );
		goto exit;
	}
	else if( totalBytes != sizeof( context->serverPublicSignKey ) )
	{
		FPrintF( stderr, "Public signing key contains incorrect number of hex bytes (%zu != %zu)\n",
			totalBytes, sizeof( context->serverPublicSignKey ) );
		err = kSizeErr;
		goto exit;
	}
	check( writtenBytes == totalBytes );
	
	err = StringToSockAddr( gDNSCrypt_Server, &context->serverAddr, sizeof( context->serverAddr ), NULL );
	require_noerr( err, exit );
	if( SockAddrGetPort( &context->serverAddr ) == 0 ) SockAddrSetPort( &context->serverAddr, kDNSCryptPort );
	
	err = RecordTypeFromArgString( gDNSCrypt_Type, &context->qtype );
	require_noerr( err, exit );
	
	// Write query message.
	
	context->queryID = (uint16_t) Random32();
	err = WriteDNSQueryMessage( context->msgBuf, context->queryID, kDNSHeaderFlag_RecursionDesired, context->providerName,
		kDNSServiceType_TXT, kDNSServiceClass_IN, &context->msgLen );
	require_noerr( err, exit );
	
	// Create UDP socket.
	
	err = UDPClientSocketOpen( AF_UNSPEC, &context->serverAddr, 0, -1, NULL, &sock );
	require_noerr( err, exit );
	
	// Send DNS query.
	
	context->sendTicks = UpTicks();
	err = _SocketWriteAll( sock, context->msgBuf, context->msgLen, 5 );
	require_noerr( err, exit );
	
	err = SocketContextCreate( sock, context, &sockCtx );
	require_noerr( err, exit );
	sock = kInvalidSocketRef;
	
	err = DispatchReadSourceCreate( sockCtx->sock, NULL, DNSCryptReceiveCertHandler, SocketContextCancelHandler, sockCtx,
		&context->readSource );
	if( err ) ForgetSocketContext( &sockCtx );
	require_noerr( err, exit );
	
	dispatch_resume( context->readSource );
	
	if( context->timeLimitSecs > 0 )
	{
		dispatch_after_f( dispatch_time_seconds( context->timeLimitSecs ), dispatch_get_main_queue(), kExitReason_Timeout,
			Exit );
	}
	dispatch_main();
	
exit:
	if( context ) free( context );
	ForgetSocket( &sock );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	DNSCryptReceiveCertHandler
//===========================================================================================================================

static void	DNSCryptReceiveCertHandler( void *inContext )
{
	OSStatus					err;
	struct timeval				now;
	const uint64_t				nowTicks	= UpTicks();
	SocketContext * const		sockCtx		= (SocketContext *) inContext;
	DNSCryptContext * const		context		= (DNSCryptContext *) sockCtx->userContext;
	const DNSHeader *			hdr;
	sockaddr_ip					fromAddr;
	const uint8_t *				ptr;
	const uint8_t *				txtPtr;
	size_t						txtLen;
	unsigned int				answerCount, i;
	uint8_t						targetName[ kDomainNameLengthMax ];
	
	gettimeofday( &now, NULL );
	
	dispatch_source_forget( &context->readSource );
	
	err = SocketRecvFrom( sockCtx->sock, context->msgBuf, sizeof( context->msgBuf ), &context->msgLen,
		&fromAddr, sizeof( fromAddr ), NULL, NULL, NULL, NULL );
	require_noerr( err, exit );
	check( SockAddrCompareAddr( &fromAddr, &context->serverAddr ) == 0 );
	
	FPrintF( stdout, "Receive time: %{du:time}\n",	&now );
	FPrintF( stdout, "Source:       %##a\n",		&context->serverAddr );
	FPrintF( stdout, "Message size: %zu\n",			context->msgLen );
	FPrintF( stdout, "RTT:          %llu ms\n\n",	UpTicksToMilliseconds( nowTicks - context->sendTicks ) );
	FPrintF( stdout, "%.*{du:dnsmsg}", context->printRawRData ? 1 : 0, context->msgBuf, context->msgLen );
	
	require_action_quiet( context->msgLen >= kDNSHeaderLength, exit, err = kSizeErr );
	
	hdr = (DNSHeader *) context->msgBuf;
	require_action_quiet( DNSHeaderGetID( hdr ) == context->queryID, exit, err = kMismatchErr );
	
	err = DNSMessageGetAnswerSection( context->msgBuf, context->msgLen, &ptr );
	require_noerr( err, exit );
	
	err = DomainNameFromString( targetName, context->providerName, NULL );
	require_noerr( err, exit );
	
	answerCount = DNSHeaderGetAnswerCount( hdr );
	for( i = 0; i < answerCount; ++i )
	{
		uint16_t		type;
		uint16_t		class;
		uint8_t			name[ kDomainNameLengthMax ];
		
		err = DNSMessageExtractRecord( context->msgBuf, context->msgLen, ptr, name, &type, &class, NULL, &txtPtr, &txtLen,
			&ptr );
		require_noerr( err, exit );
		
		if( ( type == kDNSServiceType_TXT ) && ( class == kDNSServiceClass_IN ) && DomainNameEqual( name, targetName ) )
		{
			break;
		}
	}
	
	if( txtLen < ( 1 + kDNSCryptCertMinimumLength ) )
	{
		FPrintF( stderr, "TXT record length is too short (%u < %u)\n", txtLen, kDNSCryptCertMinimumLength + 1 );
		err = kSizeErr;
		goto exit;
	}
	if( txtPtr[ 0 ] < kDNSCryptCertMinimumLength )
	{
		FPrintF( stderr, "TXT record value length is too short (%u < %u)\n", txtPtr[ 0 ], kDNSCryptCertMinimumLength );
		err = kSizeErr;
		goto exit;
	}
	
	context->certLen = txtPtr[ 0 ];
	context->certPtr = &txtPtr[ 1 ];
	
	dispatch_async_f( dispatch_get_main_queue(), context, DNSCryptProceed );
	
exit:
	if( err ) Exit( NULL );
}

//===========================================================================================================================
//	DNSCryptReceiveResponseHandler
//===========================================================================================================================

static void	DNSCryptReceiveResponseHandler( void *inContext )
{
	OSStatus						err;
	struct timeval					now;
	const uint64_t					nowTicks	= UpTicks();
	SocketContext * const			sockCtx		= (SocketContext *) inContext;
	DNSCryptContext * const			context		= (DNSCryptContext *) sockCtx->userContext;
	sockaddr_ip						fromAddr;
	DNSCryptResponseHeader *		hdr;
	const uint8_t *					end;
	uint8_t *						ciphertext;
	uint8_t *						plaintext;
	const uint8_t *					response;
	uint8_t							nonce[ crypto_box_NONCEBYTES ];
	
	gettimeofday( &now, NULL );
	
	dispatch_source_forget( &context->readSource );
	
	err = SocketRecvFrom( sockCtx->sock, context->msgBuf, sizeof( context->msgBuf ), &context->msgLen,
		&fromAddr, sizeof( fromAddr ), NULL, NULL, NULL, NULL );
	require_noerr( err, exit );
	check( SockAddrCompareAddr( &fromAddr, &context->serverAddr ) == 0 );
	
	FPrintF( stdout, "Receive time: %{du:time}\n",	&now );
	FPrintF( stdout, "Source:       %##a\n",		&context->serverAddr );
	FPrintF( stdout, "Message size: %zu\n",			context->msgLen );
	FPrintF( stdout, "RTT:          %llu ms\n\n",	UpTicksToMilliseconds( nowTicks - context->sendTicks ) );
	
	if( context->msgLen < sizeof( DNSCryptResponseHeader ) )
	{
		FPrintF( stderr, "DNSCrypt response is too short.\n" );
		err = kSizeErr;
		goto exit;
	}
	
	hdr = (DNSCryptResponseHeader *) context->msgBuf;
	
	if( memcmp( hdr->resolverMagic, kDNSCryptResolverMagic, kDNSCryptResolverMagicLength ) != 0 )
	{
		FPrintF( stderr, "DNSCrypt response resolver magic %#H != %#H\n",
			hdr->resolverMagic,		kDNSCryptResolverMagicLength, INT_MAX,
			kDNSCryptResolverMagic, kDNSCryptResolverMagicLength, INT_MAX );
		err = kValueErr;
		goto exit;
	}
	
	if( memcmp( hdr->clientNonce, context->clientNonce, kDNSCryptHalfNonceLength ) != 0 )
	{
		FPrintF( stderr, "DNSCrypt response client nonce mismatch.\n" );
		err = kValueErr;
		goto exit;
	}
	
	memcpy( nonce, hdr->clientNonce, crypto_box_NONCEBYTES );
	
	ciphertext = hdr->poly1305MAC - crypto_box_BOXZEROBYTES;
	memset( ciphertext, 0, crypto_box_BOXZEROBYTES );
	
	plaintext = (uint8_t *)( hdr + 1 ) - crypto_box_ZEROBYTES;
	check( plaintext == ciphertext );
	
	end = context->msgBuf + context->msgLen;
	
	err = crypto_box_open_afternm( plaintext, ciphertext, (size_t)( end - ciphertext ), nonce, context->nmKey );
	require_noerr( err, exit );
	
	response = plaintext + crypto_box_ZEROBYTES;
	FPrintF( stdout, "%.*{du:dnsmsg}", context->printRawRData ? 1 : 0, response, (size_t)( end - response ) );
	Exit( kExitReason_ReceivedResponse );
	
exit:
	if( err ) Exit( NULL );
}

//===========================================================================================================================
//	DNSCryptProceed
//===========================================================================================================================

static void	DNSCryptProceed( void *inContext )
{
	OSStatus					err;
	DNSCryptContext * const		context = (DNSCryptContext *) inContext;
	
	err = DNSCryptProcessCert( context );
	require_noerr_quiet( err, exit );
	
	err = DNSCryptBuildQuery( context );
	require_noerr_quiet( err, exit );
	
	err = DNSCryptSendQuery( context );
	require_noerr_quiet( err, exit );
	
exit:
	if( err ) Exit( NULL );
}

//===========================================================================================================================
//	DNSCryptProcessCert
//===========================================================================================================================

static OSStatus	DNSCryptProcessCert( DNSCryptContext *inContext )
{
	OSStatus						err;
	const DNSCryptCert * const		cert	= (DNSCryptCert *) inContext->certPtr;
	const uint8_t * const			certEnd	= inContext->certPtr + inContext->certLen;
	struct timeval					now;
	time_t							startTimeSecs, endTimeSecs;
	size_t							signedLen;
	uint8_t *						tempBuf;
	unsigned long long				tempLen;
	
	DNSCryptPrintCertificate( cert, inContext->certLen );
	
	if( memcmp( cert->certMagic, kDNSCryptCertMagic, kDNSCryptCertMagicLength ) != 0 )
	{
		FPrintF( stderr, "DNSCrypt certificate magic %#H != %#H\n",
			cert->certMagic,	kDNSCryptCertMagicLength, INT_MAX,
			kDNSCryptCertMagic, kDNSCryptCertMagicLength, INT_MAX );
		err = kValueErr;
		goto exit;
	}
	
	startTimeSecs	= (time_t) ReadBig32( cert->startTime );
	endTimeSecs		= (time_t) ReadBig32( cert->endTime );
	
	gettimeofday( &now, NULL );
	if( now.tv_sec < startTimeSecs )
	{
		FPrintF( stderr, "DNSCrypt certificate start time is in the future.\n" );
		err = kDateErr;
		goto exit;
	}
	if( now.tv_sec >= endTimeSecs )
	{
		FPrintF( stderr, "DNSCrypt certificate has expired.\n" );
		err = kDateErr;
		goto exit;
	}
	
	signedLen = (size_t)( certEnd - cert->signature );
	tempBuf = (uint8_t *) malloc( signedLen );
	require_action( tempBuf, exit, err = kNoMemoryErr );
	err = crypto_sign_open( tempBuf, &tempLen, cert->signature, signedLen, inContext->serverPublicSignKey );
	free( tempBuf );
	if( err )
	{
		FPrintF( stderr, "DNSCrypt certificate failed verification.\n" );
		err = kAuthenticationErr;
		goto exit;
	}
	
	memcpy( inContext->serverPublicKey,	cert->publicKey,	crypto_box_PUBLICKEYBYTES );
	memcpy( inContext->clientMagic,		cert->clientMagic,	kDNSCryptClientMagicLength );
	
	err = crypto_box_beforenm( inContext->nmKey, inContext->serverPublicKey, inContext->clientSecretKey );
	require_noerr( err, exit );
	
	inContext->certPtr	= NULL;
	inContext->certLen	= 0;
	inContext->msgLen	= 0;
	
exit:
	return( err );
}

//===========================================================================================================================
//	DNSCryptBuildQuery
//===========================================================================================================================

static OSStatus	DNSCryptPadQuery( uint8_t *inMsgPtr, size_t inMsgLen, size_t inMaxLen, size_t *outPaddedLen );

static OSStatus	DNSCryptBuildQuery( DNSCryptContext *inContext )
{
	OSStatus						err;
	DNSCryptQueryHeader * const		hdr			= (DNSCryptQueryHeader *) inContext->msgBuf;
	uint8_t * const					queryPtr	= (uint8_t *)( hdr + 1 );
	size_t							queryLen;
	size_t							paddedQueryLen;
	const uint8_t * const			msgLimit	= inContext->msgBuf + sizeof( inContext->msgBuf );
	const uint8_t *					padLimit;
	uint8_t							nonce[ crypto_box_NONCEBYTES ];
	
	check_compile_time_code( sizeof( inContext->msgBuf ) >= ( sizeof( DNSCryptQueryHeader ) + kDNSQueryMessageMaxLen ) );
	
	inContext->queryID = (uint16_t) Random32();
	err = WriteDNSQueryMessage( queryPtr, inContext->queryID, kDNSHeaderFlag_RecursionDesired, inContext->qname,
		inContext->qtype, kDNSServiceClass_IN, &queryLen );
	require_noerr( err, exit );
	
	padLimit = &queryPtr[ queryLen + kDNSCryptMaxPadLength ];
	if( padLimit > msgLimit ) padLimit = msgLimit;
	
	err = DNSCryptPadQuery( queryPtr, queryLen, (size_t)( padLimit - queryPtr ), &paddedQueryLen );
	require_noerr( err, exit );
	
	memset( queryPtr - crypto_box_ZEROBYTES, 0, crypto_box_ZEROBYTES );
	RandomBytes( inContext->clientNonce, kDNSCryptHalfNonceLength );
	memcpy( nonce, inContext->clientNonce, kDNSCryptHalfNonceLength );
	memset( &nonce[ kDNSCryptHalfNonceLength ], 0, kDNSCryptHalfNonceLength );
	
	err = crypto_box_afternm( queryPtr - crypto_box_ZEROBYTES, queryPtr - crypto_box_ZEROBYTES,
		paddedQueryLen + crypto_box_ZEROBYTES, nonce, inContext->nmKey );
	require_noerr( err, exit );
	
	memcpy( hdr->clientMagic,		inContext->clientMagic,		kDNSCryptClientMagicLength );
	memcpy( hdr->clientPublicKey,	inContext->clientPublicKey,	crypto_box_PUBLICKEYBYTES );
	memcpy( hdr->clientNonce,		nonce,						kDNSCryptHalfNonceLength );
	
	inContext->msgLen = (size_t)( &queryPtr[ paddedQueryLen ] - inContext->msgBuf );
	
exit:
	return( err );
}

static OSStatus	DNSCryptPadQuery( uint8_t *inMsgPtr, size_t inMsgLen, size_t inMaxLen, size_t *outPaddedLen )
{
	OSStatus		err;
	size_t			paddedLen;
	
	require_action_quiet( ( inMsgLen + kDNSCryptMinPadLength ) <= inMaxLen, exit, err = kSizeErr );
	
	paddedLen = inMsgLen + kDNSCryptMinPadLength +
		arc4random_uniform( (uint32_t)( inMaxLen - ( inMsgLen + kDNSCryptMinPadLength ) + 1 ) );
	paddedLen += ( kDNSCryptBlockSize - ( paddedLen % kDNSCryptBlockSize ) );
	if( paddedLen > inMaxLen ) paddedLen = inMaxLen;
	
	inMsgPtr[ inMsgLen ] = 0x80;
	memset( &inMsgPtr[ inMsgLen + 1 ], 0, paddedLen - ( inMsgLen + 1 ) );
	
	if( outPaddedLen ) *outPaddedLen = paddedLen;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	DNSCryptSendQuery
//===========================================================================================================================

static OSStatus	DNSCryptSendQuery( DNSCryptContext *inContext )
{
	OSStatus			err;
	SocketContext *		sockCtx;
	SocketRef			sock = kInvalidSocketRef;
	
	check( inContext->msgLen > 0 );
	check( !inContext->readSource );
	
	err = UDPClientSocketOpen( AF_UNSPEC, &inContext->serverAddr, 0, -1, NULL, &sock );
	require_noerr( err, exit );
	
	inContext->sendTicks = UpTicks();
	err = _SocketWriteAll( sock, inContext->msgBuf, inContext->msgLen, 5 );
	require_noerr( err, exit );
	
	err = SocketContextCreate( sock, inContext, &sockCtx );
	require_noerr( err, exit );
	sock = kInvalidSocketRef;
	
	err = DispatchReadSourceCreate( sockCtx->sock, NULL, DNSCryptReceiveResponseHandler, SocketContextCancelHandler, sockCtx,
		&inContext->readSource );
	if( err ) ForgetSocketContext( &sockCtx );
	require_noerr( err, exit );
	
	dispatch_resume( inContext->readSource );
	
exit:
	ForgetSocket( &sock );
	return( err );
}

//===========================================================================================================================
//	DNSCryptPrintCertificate
//===========================================================================================================================

#define kCertTimeStrBufLen		32

static char *	CertTimeStr( time_t inTime, char inBuffer[ kCertTimeStrBufLen ] );

static void	DNSCryptPrintCertificate( const DNSCryptCert *inCert, size_t inLen )
{
	time_t		startTime, endTime;
	int			extLen;
	char		timeBuf[ kCertTimeStrBufLen ];
	
	check( inLen >= kDNSCryptCertMinimumLength );
	
	startTime	= (time_t) ReadBig32( inCert->startTime );
	endTime		= (time_t) ReadBig32( inCert->endTime );
	
	FPrintF( stdout, "DNSCrypt certificate (%zu bytes):\n", inLen );
	FPrintF( stdout, "Cert Magic:    %#H\n", inCert->certMagic, kDNSCryptCertMagicLength, INT_MAX );
	FPrintF( stdout, "ES Version:    %u\n",	ReadBig16( inCert->esVersion ) );
	FPrintF( stdout, "Minor Version: %u\n",	ReadBig16( inCert->minorVersion ) );
	FPrintF( stdout, "Signature:     %H\n",	inCert->signature, crypto_sign_BYTES / 2, INT_MAX );
	FPrintF( stdout, "               %H\n",	&inCert->signature[ crypto_sign_BYTES / 2 ], crypto_sign_BYTES / 2, INT_MAX );
	FPrintF( stdout, "Public Key:    %H\n", inCert->publicKey, sizeof( inCert->publicKey ), INT_MAX );
	FPrintF( stdout, "Client Magic:  %H\n", inCert->clientMagic, kDNSCryptClientMagicLength, INT_MAX );
	FPrintF( stdout, "Serial:        %u\n",	ReadBig32( inCert->serial ) );
	FPrintF( stdout, "Start Time:    %u (%s)\n", (uint32_t) startTime, CertTimeStr( startTime, timeBuf ) );
	FPrintF( stdout, "End Time:      %u (%s)\n", (uint32_t) endTime, CertTimeStr( endTime, timeBuf ) );
	
	if( inLen > kDNSCryptCertMinimumLength )
	{
		extLen = (int)( inLen - kDNSCryptCertMinimumLength );
		FPrintF( stdout, "Extensions:    %.1H\n", inCert->extensions, extLen, extLen );
	}
	FPrintF( stdout, "\n" );
}

static char *	CertTimeStr( time_t inTime, char inBuffer[ kCertTimeStrBufLen ] )
{
	struct tm *		tm;
	
	tm = localtime( &inTime );
	if( !tm )
	{
		dlogassert( "localtime() returned a NULL pointer.\n" );
		*inBuffer = '\0';
	}
	else
	{
		strftime( inBuffer, kCertTimeStrBufLen, "%a %b %d %H:%M:%S %Z %Y", tm );
	}
	
	return( inBuffer );
}

#endif	// DNSSDUTIL_INCLUDE_DNSCRYPT

//===========================================================================================================================
//	MDNSQueryCmd
//===========================================================================================================================

typedef struct
{
	const char *			qnameStr;							// Name (QNAME) of the record being queried as a C string.
	dispatch_source_t		readSourceV4;						// Read dispatch source for IPv4 socket.
	dispatch_source_t		readSourceV6;						// Read dispatch source for IPv6 socket.
	int						localPort;							// The port number to which the sockets are bound.
	int						receiveSecs;						// After send, the amount of time to spend receiving.
	uint32_t				ifIndex;							// Index of the interface over which to send the query.
	uint16_t				qtype;								// The type (QTYPE) of the record being queried.
	Boolean					isQU;								// True if the query is QU, i.e., requests unicast responses.
	Boolean					allResponses;						// True if all mDNS messages received should be printed.
	Boolean					printRawRData;						// True if RDATA should be printed as hexdumps.
	Boolean					useIPv4;							// True if the query should be sent via IPv4 multicast.
	Boolean					useIPv6;							// True if the query should be sent via IPv6 multicast.
	char					ifName[ IF_NAMESIZE + 1 ];			// Name of the interface over which to send the query.
	uint8_t					qname[ kDomainNameLengthMax ];		// Buffer to hold the QNAME in DNS label format.
	uint8_t					msgBuf[ kMDNSMessageSizeMax ];		// mDNS message buffer.
	
}	MDNSQueryContext;

static void	MDNSQueryPrintPrologue( const MDNSQueryContext *inContext );
static void	MDNSQueryReadHandler( void *inContext );

static void	MDNSQueryCmd( void )
{
	OSStatus				err;
	MDNSQueryContext *		context;
	SocketRef				sockV4 = kInvalidSocketRef;
	SocketRef				sockV6 = kInvalidSocketRef;
	ssize_t					n;
	const char *			ifname;
	size_t					msgLen;
	unsigned int			sendCount;
	
	// Check command parameters.
	
	if( gMDNSQuery_ReceiveSecs < -1 )
	{
		FPrintF( stdout, "Invalid receive time value: %d seconds.\n", gMDNSQuery_ReceiveSecs );
		err = kParamErr;
		goto exit;
	}
	
	context = (MDNSQueryContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	context->qnameStr		= gMDNSQuery_Name;
	context->receiveSecs	= gMDNSQuery_ReceiveSecs;
	context->isQU			= gMDNSQuery_IsQU		  ? true : false;
	context->allResponses	= gMDNSQuery_AllResponses ? true : false;
	context->printRawRData	= gMDNSQuery_RawRData	  ? true : false;
	context->useIPv4		= ( gMDNSQuery_UseIPv4 || !gMDNSQuery_UseIPv6 ) ? true : false;
	context->useIPv6		= ( gMDNSQuery_UseIPv6 || !gMDNSQuery_UseIPv4 ) ? true : false;
	
	err = InterfaceIndexFromArgString( gInterface, &context->ifIndex );
	require_noerr_quiet( err, exit );
	
	ifname = if_indextoname( context->ifIndex, context->ifName );
	require_action( ifname, exit, err = kNameErr );
	
	err = RecordTypeFromArgString( gMDNSQuery_Type, &context->qtype );
	require_noerr( err, exit );
	
	// Set up IPv4 socket.
	
	if( context->useIPv4 )
	{
		err = CreateMulticastSocket( GetMDNSMulticastAddrV4(),
			gMDNSQuery_SourcePort ? gMDNSQuery_SourcePort : ( context->isQU ? context->localPort : kMDNSPort ),
			ifname, context->ifIndex, !context->isQU, &context->localPort, &sockV4 );
		require_noerr( err, exit );
	}
	
	// Set up IPv6 socket.
	
	if( context->useIPv6 )
	{
		err = CreateMulticastSocket( GetMDNSMulticastAddrV6(),
			gMDNSQuery_SourcePort ? gMDNSQuery_SourcePort : ( context->isQU ? context->localPort : kMDNSPort ),
			ifname, context->ifIndex, !context->isQU, &context->localPort, &sockV6 );
		require_noerr( err, exit );
	}
	
	// Craft mDNS query message.
	
	check_compile_time_code( sizeof( context->msgBuf ) >= kDNSQueryMessageMaxLen );
	err = WriteDNSQueryMessage( context->msgBuf, kDefaultMDNSMessageID, kDefaultMDNSQueryFlags, context->qnameStr,
		context->qtype, context->isQU ? ( kDNSServiceClass_IN | kQClassUnicastResponseBit ) : kDNSServiceClass_IN, &msgLen );
	require_noerr( err, exit );
	
	// Print prologue.
	
	MDNSQueryPrintPrologue( context );
	
	// Send mDNS query message.
	
	sendCount = 0;
	if( IsValidSocket( sockV4 ) )
	{
		const struct sockaddr * const		mcastAddr4 = GetMDNSMulticastAddrV4();
		
		n = sendto( sockV4, context->msgBuf, msgLen, 0, mcastAddr4, SockAddrGetSize( mcastAddr4 ) );
		err = map_socket_value_errno( sockV4, n == (ssize_t) msgLen, n );
		if( err )
		{
			FPrintF( stderr, "*** Failed to send query on IPv4 socket with error %#m\n", err );
			ForgetSocket( &sockV4 );
		}
		else
		{
			++sendCount;
		}
	}
	if( IsValidSocket( sockV6 ) )
	{
		const struct sockaddr * const		mcastAddr6 = GetMDNSMulticastAddrV6();
		
		n = sendto( sockV6, context->msgBuf, msgLen, 0, mcastAddr6, SockAddrGetSize( mcastAddr6 ) );
		err = map_socket_value_errno( sockV6, n == (ssize_t) msgLen, n );
		if( err )
		{
			FPrintF( stderr, "*** Failed to send query on IPv6 socket with error %#m\n", err );
			ForgetSocket( &sockV6 );
		}
		else
		{
			++sendCount;
		}
	}
	require_action_quiet( sendCount > 0, exit, err = kUnexpectedErr );
	
	// If there's no wait period after the send, then exit.
	
	if( context->receiveSecs == 0 ) goto exit;
	
	// Create dispatch read sources for socket(s).
	
	if( IsValidSocket( sockV4 ) )
	{
		SocketContext *		sockCtx;
		
		err = SocketContextCreate( sockV4, context, &sockCtx );
		require_noerr( err, exit );
		sockV4 = kInvalidSocketRef;
		
		err = DispatchReadSourceCreate( sockCtx->sock, NULL, MDNSQueryReadHandler, SocketContextCancelHandler, sockCtx,
			&context->readSourceV4 );
		if( err ) ForgetSocketContext( &sockCtx );
		require_noerr( err, exit );
		
		dispatch_resume( context->readSourceV4 );
	}
	
	if( IsValidSocket( sockV6 ) )
	{
		SocketContext *		sockCtx;
		
		err = SocketContextCreate( sockV6, context, &sockCtx );
		require_noerr( err, exit );
		sockV6 = kInvalidSocketRef;
		
		err = DispatchReadSourceCreate( sockCtx->sock, NULL, MDNSQueryReadHandler, SocketContextCancelHandler, sockCtx,
			&context->readSourceV6 );
		if( err ) ForgetSocketContext( &sockCtx );
		require_noerr( err, exit );
		
		dispatch_resume( context->readSourceV6 );
	}
	
	if( context->receiveSecs > 0 )
	{
		dispatch_after_f( dispatch_time_seconds( context->receiveSecs ), dispatch_get_main_queue(), kExitReason_Timeout,
			Exit );
	}
	dispatch_main();
	
exit:
	ForgetSocket( &sockV4 );
	ForgetSocket( &sockV6 );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	MDNSColliderCmd
//===========================================================================================================================

static void	_MDNSColliderCmdStopHandler( void *inContext, OSStatus inError );

static void	MDNSColliderCmd( void )
{
	OSStatus					err;
	MDNSColliderRef				collider = NULL;
	uint8_t *					rdataPtr = NULL;
	size_t						rdataLen = 0;
	const char *				ifname;
	uint32_t					ifIndex;
	MDNSColliderProtocols		protocols;
	uint16_t					type;
	char						ifName[ IF_NAMESIZE + 1 ];
	uint8_t						name[ kDomainNameLengthMax ];
	
	err = InterfaceIndexFromArgString( gInterface, &ifIndex );
	require_noerr_quiet( err, exit );
	
	ifname = if_indextoname( ifIndex, ifName );
	if( !ifname )
	{
		FPrintF( stderr, "error: Invalid interface name or index: %s\n", gInterface );
		err = kNameErr;
		goto exit;
	}
	
	err = DomainNameFromString( name, gMDNSCollider_Name, NULL );
	if( err )
	{
		FPrintF( stderr, "error: Invalid record name: %s\n", gMDNSCollider_Name );
		goto exit;
	}
	
	err = RecordTypeFromArgString( gMDNSCollider_Type, &type );
	require_noerr_quiet( err, exit );
	
	if( gMDNSCollider_RecordData )
	{
		err = RecordDataFromArgString( gMDNSCollider_RecordData, &rdataPtr, &rdataLen );
		require_noerr_quiet( err, exit );
	}
	
	err = MDNSColliderCreate( dispatch_get_main_queue(), &collider );
	require_noerr( err, exit );
	
	err = MDNSColliderSetProgram( collider, gMDNSCollider_Program );
	if( err )
	{
		FPrintF( stderr, "error: Failed to set program string: '%s'\n", gMDNSCollider_Program );
		goto exit;
	}
	
	err = MDNSColliderSetRecord( collider, name, type, rdataPtr, rdataLen );
	require_noerr( err, exit );
	ForgetMem( &rdataPtr );
	
	protocols = kMDNSColliderProtocol_None;
	if( gMDNSCollider_UseIPv4 || !gMDNSCollider_UseIPv6 ) protocols |= kMDNSColliderProtocol_IPv4;
	if( gMDNSCollider_UseIPv6 || !gMDNSCollider_UseIPv4 ) protocols |= kMDNSColliderProtocol_IPv6;
	MDNSColliderSetProtocols( collider, protocols );
	MDNSColliderSetInterfaceIndex( collider, ifIndex );
	MDNSColliderSetStopHandler( collider, _MDNSColliderCmdStopHandler, collider );
	
	err = MDNSColliderStart( collider );
	require_noerr( err, exit );
	
	dispatch_main();
	
exit:
	FreeNullSafe( rdataPtr );
	CFReleaseNullSafe( collider );
	if( err ) exit( 1 );
}

static void	_MDNSColliderCmdStopHandler( void *inContext, OSStatus inError )
{
	MDNSColliderRef const		collider = (MDNSColliderRef) inContext;
	
	CFRelease( collider );
	exit( inError ? 1 : 0 );
}

//===========================================================================================================================
//	MDNSQueryPrintPrologue
//===========================================================================================================================

static void	MDNSQueryPrintPrologue( const MDNSQueryContext *inContext )
{
	const int		receiveSecs = inContext->receiveSecs;
	
	FPrintF( stdout, "Interface:        %d (%s)\n",		(int32_t) inContext->ifIndex, inContext->ifName );
	FPrintF( stdout, "Name:             %s\n",			inContext->qnameStr );
	FPrintF( stdout, "Type:             %s (%u)\n",		RecordTypeToString( inContext->qtype ), inContext->qtype );
	FPrintF( stdout, "Class:            IN (%s)\n",		inContext->isQU ? "QU" : "QM" );
	FPrintF( stdout, "Local port:       %d\n",			inContext->localPort );
	FPrintF( stdout, "IP protocols:     %?s%?s%?s\n",
		inContext->useIPv4, "IPv4", ( inContext->useIPv4 && inContext->useIPv6 ), ", ", inContext->useIPv6, "IPv6" );
	FPrintF( stdout, "Receive duration: " );
	if( receiveSecs >= 0 )	FPrintF( stdout, "%d second%?c\n", receiveSecs, receiveSecs != 1, 's' );
	else					FPrintF( stdout, "∞\n" );
	FPrintF( stdout, "Start time:       %{du:time}\n",	NULL );
}

//===========================================================================================================================
//	MDNSQueryReadHandler
//===========================================================================================================================

static void	MDNSQueryReadHandler( void *inContext )
{
	OSStatus						err;
	struct timeval					now;
	SocketContext * const			sockCtx = (SocketContext *) inContext;
	MDNSQueryContext * const		context = (MDNSQueryContext *) sockCtx->userContext;
	size_t							msgLen;
	sockaddr_ip						fromAddr;
	Boolean							foundAnswer	= false;
	
	gettimeofday( &now, NULL );
	
	err = SocketRecvFrom( sockCtx->sock, context->msgBuf, sizeof( context->msgBuf ), &msgLen, &fromAddr,
		sizeof( fromAddr ), NULL, NULL, NULL, NULL );
	require_noerr( err, exit );
	
	if( !context->allResponses && ( msgLen >= kDNSHeaderLength ) )
	{
		const uint8_t *				ptr;
		const DNSHeader * const		hdr = (DNSHeader *) context->msgBuf;
		unsigned int				rrCount, i;
		uint16_t					type, class;
		uint8_t						name[ kDomainNameLengthMax ];
		
		err = DNSMessageGetAnswerSection( context->msgBuf, msgLen, &ptr );
		require_noerr( err, exit );
		
		if( context->qname[ 0 ] == 0 )
		{
			err = DomainNameAppendString( context->qname, context->qnameStr, NULL );
			require_noerr( err, exit );
		}
		
		rrCount = DNSHeaderGetAnswerCount( hdr ) + DNSHeaderGetAuthorityCount( hdr ) + DNSHeaderGetAdditionalCount( hdr );
		for( i = 0; i < rrCount; ++i )
		{
			err = DNSMessageExtractRecord( context->msgBuf, msgLen, ptr, name, &type, &class, NULL, NULL, NULL, &ptr );
			require_noerr( err, exit );
			
			if( ( ( context->qtype == kDNSServiceType_ANY ) || ( type == context->qtype ) ) &&
				DomainNameEqual( name, context->qname ) )
			{
				foundAnswer = true;
				break;
			}
		}
	}
	if( context->allResponses || foundAnswer )
	{
		FPrintF( stdout, "---\n" );
		FPrintF( stdout, "Receive time: %{du:time}\n",	&now );
		FPrintF( stdout, "Source:       %##a\n",		&fromAddr );
		FPrintF( stdout, "Message size: %zu\n\n%#.*{du:dnsmsg}",
			msgLen, context->printRawRData ? 1 : 0, context->msgBuf, msgLen );
	}
	
exit:
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	PIDToUUIDCmd
//===========================================================================================================================

static void	PIDToUUIDCmd( void )
{
	OSStatus							err;
	int									n;
	struct proc_uniqidentifierinfo		info;
	
	n = proc_pidinfo( gPIDToUUID_PID, PROC_PIDUNIQIDENTIFIERINFO, 1, &info, sizeof( info ) );
	require_action_quiet( n == (int) sizeof( info ), exit, err = kUnknownErr );
	
	FPrintF( stdout, "%#U\n", info.p_uuid );
	err = kNoErr;
	
exit:
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	DNSServerCmd
//===========================================================================================================================

typedef struct DNSServerPrivate *		DNSServerRef;

typedef struct
{
	DNSServerRef			server;			// Reference to the DNS server.
	dispatch_source_t		sigIntSource;	// Dispatch SIGINT source.
	dispatch_source_t		sigTermSource;	// Dispatch SIGTERM source.
	const char *			domainOverride;	// If non-NULL, the server is to use this domain instead of "d.test.".
#if( TARGET_OS_DARWIN )
	dispatch_source_t		processMonitor;	// Process monitor source for process being followed, if any.
	pid_t					followPID;		// PID of process being followed, if any. (If it exits, we exit).
	Boolean					addedResolver;	// True if system DNS settings contains a resolver entry for server.
#endif
	Boolean					loopbackOnly;	// True if the server should be bound to the loopback interface.
	
}	DNSServerCmdContext;

typedef enum
{
	kDNSServerEvent_Started	= 1,
	kDNSServerEvent_Stopped	= 2
	
}	DNSServerEventType;

typedef void ( *DNSServerEventHandler_f )( DNSServerEventType inType, uintptr_t inEventData, void *inContext );

CFTypeID	DNSServerGetTypeID( void );
static OSStatus
	DNSServerCreate(
		dispatch_queue_t		inQueue,
		DNSServerEventHandler_f	inEventHandler,
		void *					inEventContext,
		unsigned int			inResponseDelayMs,
		uint32_t				inDefaultTTL,
		int						inPort,
		Boolean					inLoopbackOnly,
		const char *			inDomain,
		Boolean					inBadUDPMode,
		DNSServerRef *			outServer );
static void	DNSServerStart( DNSServerRef inServer );
static void	DNSServerStop( DNSServerRef inServer );

#define ForgetDNSServer( X )		ForgetCustomEx( X, DNSServerStop, CFRelease )

static void	DNSServerCmdContextFree( DNSServerCmdContext *inContext );
static void	DNSServerCmdEventHandler( DNSServerEventType inType, uintptr_t inEventData, void *inContext );
static void	DNSServerCmdSigIntHandler( void *inContext );
static void	DNSServerCmdSigTermHandler( void *inContext );
#if( TARGET_OS_DARWIN )
static void	DNSServerCmdFollowedProcessHandler( void *inContext );
#endif

ulog_define_ex( kDNSSDUtilIdentifier, DNSServer, kLogLevelInfo, kLogFlags_None, "DNSServer", NULL );
#define ds_ulog( LEVEL, ... )		ulog( &log_category_from_name( DNSServer ), (LEVEL), __VA_ARGS__ )

static void	DNSServerCmd( void )
{
	OSStatus					err;
	DNSServerCmdContext *		context = NULL;
	
	if( gDNSServer_Foreground )
	{
		LogControl( "DNSServer:output=file;stdout,DNSServer:flags=time;prefix" );
	}
	
	err = CheckIntegerArgument( gDNSServer_ResponseDelayMs, "response delay (ms)", 0, INT_MAX );
	require_noerr_quiet( err, exit );
	
	err = CheckIntegerArgument( gDNSServer_DefaultTTL, "default TTL", 0, INT32_MAX );
	require_noerr_quiet( err, exit );
	
	err = CheckIntegerArgument( gDNSServer_Port, "port number", -UINT16_MAX, UINT16_MAX );
	require_noerr_quiet( err, exit );
	
	context = (DNSServerCmdContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	context->domainOverride	= gDNSServer_DomainOverride;
	context->loopbackOnly	= gDNSServer_LoopbackOnly ? true : false;
	
#if( TARGET_OS_DARWIN )
	if( gDNSServer_FollowPID )
	{
		context->followPID = _StringToPID( gDNSServer_FollowPID, &err );
		if( err || ( context->followPID < 0 ) )
		{
			FPrintF( stderr, "error: Invalid follow PID: %s\n", gDNSServer_FollowPID );
			err = kParamErr;
			goto exit;
		}
		
		err = DispatchProcessMonitorCreate( context->followPID, DISPATCH_PROC_EXIT, dispatch_get_main_queue(),
			DNSServerCmdFollowedProcessHandler, NULL, context, &context->processMonitor );
		require_noerr( err, exit );
		dispatch_resume( context->processMonitor );
	}
	else
	{
		context->followPID = -1;
	}
#endif
	
	signal( SIGINT, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGINT, dispatch_get_main_queue(), DNSServerCmdSigIntHandler, context,
		&context->sigIntSource );
	require_noerr( err, exit );
	dispatch_resume( context->sigIntSource );
	
	signal( SIGTERM, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGTERM, dispatch_get_main_queue(), DNSServerCmdSigTermHandler, context,
		&context->sigTermSource );
	require_noerr( err, exit );
	dispatch_resume( context->sigTermSource );
	
	err = DNSServerCreate( dispatch_get_main_queue(), DNSServerCmdEventHandler, context,
		(unsigned int) gDNSServer_ResponseDelayMs, (uint32_t) gDNSServer_DefaultTTL, gDNSServer_Port, context->loopbackOnly,
		context->domainOverride, gDNSServer_BadUDPMode ? true : false, &context->server );
	require_noerr( err, exit );
	
	DNSServerStart( context->server );
	dispatch_main();
	
exit:
	FPrintF( stderr, "Failed to start DNS server: %#m\n", err );
	if( context ) DNSServerCmdContextFree( context );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	DNSServerCmdContextFree
//===========================================================================================================================

static void	DNSServerCmdContextFree( DNSServerCmdContext *inContext )
{
	ForgetCF( &inContext->server );
	dispatch_source_forget( &inContext->sigIntSource );
	dispatch_source_forget( &inContext->sigTermSource );
#if( TARGET_OS_DARWIN )
	dispatch_source_forget( &inContext->processMonitor );
#endif
	free( inContext );
}

//===========================================================================================================================
//	DNSServerCmdEventHandler
//===========================================================================================================================

#if( TARGET_OS_DARWIN )
static OSStatus	_DNSServerCmdLoopbackResolverAdd( const char *inDomain, int inPort );
static OSStatus	_DNSServerCmdLoopbackResolverRemove( void );
#endif

static void	DNSServerCmdEventHandler( DNSServerEventType inType, uintptr_t inEventData, void *inContext )
{
	OSStatus						err;
	DNSServerCmdContext * const		context = (DNSServerCmdContext *) inContext;
	
	if( inType == kDNSServerEvent_Started )
	{
	#if( TARGET_OS_DARWIN )
		const int		port = (int) inEventData;
		
		err = _DNSServerCmdLoopbackResolverAdd( context->domainOverride ? context->domainOverride : "d.test.", port );
		if( err )
		{
			ds_ulog( kLogLevelError, "Failed to add loopback resolver to DNS configuration for \"d.test.\" domain: %#m\n",
				err );
			if( context->loopbackOnly ) ForgetDNSServer( &context->server );
		}
		else
		{
			context->addedResolver = true;
		}
	#endif
	}
	else if( inType == kDNSServerEvent_Stopped )
	{
		const OSStatus		stopError = (OSStatus) inEventData;
		
		if( stopError ) ds_ulog( kLogLevelError, "The server stopped unexpectedly with error: %#m.\n", stopError );
		
		err = kNoErr;
	#if( TARGET_OS_DARWIN )
		if( context->addedResolver )
		{
			err = _DNSServerCmdLoopbackResolverRemove();
			if( err )
			{
				ds_ulog( kLogLevelError, "Failed to remove loopback resolver from DNS configuration: %#m\n", err );
			}
			else
			{
				context->addedResolver = false;
			}
		}
		else if( context->loopbackOnly )
		{
			err = kUnknownErr;
		}
	#endif
		DNSServerCmdContextFree( context );
		exit( ( stopError || err ) ? 1 : 0 );
	}
}

#if( TARGET_OS_DARWIN )
//===========================================================================================================================
//	_DNSServerCmdLoopbackResolverAdd
//===========================================================================================================================

static OSStatus	_DNSServerCmdLoopbackResolverAdd( const char *inDomain, int inPort )
{
	OSStatus				err;
	SCDynamicStoreRef		store;
	CFPropertyListRef		plist		= NULL;
	CFStringRef				key			= NULL;
	const uint32_t			loopbackV4	= htonl( INADDR_LOOPBACK );
	Boolean					success;
	
	store = SCDynamicStoreCreate( NULL, CFSTR( kDNSSDUtilIdentifier ), NULL, NULL );
	err = map_scerror( store );
	require_noerr( err, exit );
	
	err = CFPropertyListCreateFormatted( kCFAllocatorDefault, &plist,
		"{"
			"%kO="
			"["
				"%s"
			"]"
			"%kO="
			"["
				"%.4a"
				"%.16a"
			"]"
			"%kO=%i"
			"%kO=%O"
			"%kO=%O"
		"}",
		kSCPropNetDNSSupplementalMatchDomains,	inDomain,
		kSCPropNetDNSServerAddresses,			&loopbackV4, in6addr_loopback.s6_addr,
		kSCPropNetDNSServerPort,				inPort,
		kSCPropInterfaceName,					CFSTR( "lo0" ),
	kSCPropNetDNSConfirmedServiceID,			CFSTR( "com.apple.dnssdutil.server" ) );
	require_noerr( err, exit );
	
	key = SCDynamicStoreKeyCreateNetworkServiceEntity( NULL, kSCDynamicStoreDomainState,
		CFSTR( "com.apple.dnssdutil.server" ), kSCEntNetDNS );
	require_action( key, exit, err = kUnknownErr );
	
	success = SCDynamicStoreSetValue( store, key, plist );
	require_action( success, exit, err = kUnknownErr );
	
exit:
	CFReleaseNullSafe( store );
	CFReleaseNullSafe( plist );
	CFReleaseNullSafe( key );
	return( err );
}

//===========================================================================================================================
//	_DNSServerCmdLoopbackResolverRemove
//===========================================================================================================================

static OSStatus	_DNSServerCmdLoopbackResolverRemove( void )
{
	OSStatus				err;
	SCDynamicStoreRef		store;
	CFStringRef				key = NULL;
	Boolean					success;
	
	store = SCDynamicStoreCreate( NULL, CFSTR( kDNSSDUtilIdentifier ), NULL, NULL );
	err = map_scerror( store );
	require_noerr( err, exit );
	
	key = SCDynamicStoreKeyCreateNetworkServiceEntity( NULL, kSCDynamicStoreDomainState,
		CFSTR( "com.apple.dnssdutil.server" ), kSCEntNetDNS );
	require_action( key, exit, err = kUnknownErr );
	
	success = SCDynamicStoreRemoveValue( store, key );
	require_action( success, exit, err = kUnknownErr );
	
exit:
	CFReleaseNullSafe( store );
	CFReleaseNullSafe( key );
	return( err );
}
#endif

//===========================================================================================================================
//	DNSServerCmdSigIntHandler
//===========================================================================================================================

static void	_DNSServerCmdShutdown( DNSServerCmdContext *inContext, int inSignal );

static void	DNSServerCmdSigIntHandler( void *inContext )
{
	_DNSServerCmdShutdown( (DNSServerCmdContext *) inContext, SIGINT );
}

//===========================================================================================================================
//	DNSServerCmdSigTermHandler
//===========================================================================================================================

static void	DNSServerCmdSigTermHandler( void *inContext )
{
	_DNSServerCmdShutdown( (DNSServerCmdContext *) inContext, SIGTERM );
}

#if( TARGET_OS_DARWIN )
//===========================================================================================================================
//	DNSServerCmdFollowedProcessHandler
//===========================================================================================================================

static void	DNSServerCmdFollowedProcessHandler( void *inContext )
{
	DNSServerCmdContext * const		context = (DNSServerCmdContext *) inContext;
	
	if( dispatch_source_get_data( context->processMonitor ) & DISPATCH_PROC_EXIT ) _DNSServerCmdShutdown( context, 0 );
}
#endif

//===========================================================================================================================
//	_DNSServerCmdExternalExit
//===========================================================================================================================

#define SignalNumberToString( X ) (		\
	( (X) == SIGINT )  ? "SIGINT"  :	\
	( (X) == SIGTERM ) ? "SIGTERM" :	\
						 "???" )

static void	_DNSServerCmdShutdown( DNSServerCmdContext *inContext, int inSignal )
{
	dispatch_source_forget( &inContext->sigIntSource );
	dispatch_source_forget( &inContext->sigTermSource );
#if( TARGET_OS_DARWIN )
	dispatch_source_forget( &inContext->processMonitor );
	
	if( inSignal == 0 )
	{
		ds_ulog( kLogLevelNotice, "Exiting: followed process (%lld) exited\n", (int64_t) inContext->followPID );
	}
	else
#endif
	{
		ds_ulog( kLogLevelNotice, "Exiting: received signal %d (%s)\n", inSignal, SignalNumberToString( inSignal ) );
	}
	
	ForgetDNSServer( &inContext->server );
}

//===========================================================================================================================
//	DNSServerCreate
//===========================================================================================================================

#define kDDotTestDomainName		(const uint8_t *) "\x01" "d" "\x04" "test"

typedef struct DNSDelayedResponse		DNSDelayedResponse;
struct DNSDelayedResponse
{
	DNSDelayedResponse *		next;
	sockaddr_ip					destAddr;
	uint64_t					targetTicks;
	uint8_t *					msgPtr;
	size_t						msgLen;
};

struct DNSServerPrivate
{
	CFRuntimeBase				base;				// CF object base.
	uint8_t *					domain;				// Parent domain of server's resource records.
	dispatch_queue_t			queue;				// Queue for DNS server's events.
	dispatch_source_t			readSourceUDPv4;	// Read source for IPv4 UDP socket.
	dispatch_source_t			readSourceUDPv6;	// Read source for IPv6 UDP socket.
	dispatch_source_t			readSourceTCPv4;	// Read source for IPv4 TCP socket.
	dispatch_source_t			readSourceTCPv6;	// Read source for IPv6 TCP socket.
	SocketRef					sockUDPv4;
	SocketRef					sockUDPv6;
	DNSServerEventHandler_f		eventHandler;
	void *						eventContext;
	DNSDelayedResponse *		responseList;
	dispatch_source_t			responseTimer;
	unsigned int				responseDelayMs;
	uint32_t					defaultTTL;
	uint32_t					serial;				// Serial number for SOA record.
	int							port;				// Port to use for receiving and sending DNS messages.
	OSStatus					stopError;
	Boolean						stopped;
	Boolean						loopbackOnly;
	Boolean						badUDPMode;			// True if the server runs in Bad UDP mode.
};

static void	_DNSServerUDPReadHandler( void *inContext );
static void	_DNSServerTCPReadHandler( void *inContext );
static void	_DNSDelayedResponseFree( DNSDelayedResponse *inResponse );
static void	_DNSDelayedResponseFreeList( DNSDelayedResponse *inList );

CF_CLASS_DEFINE( DNSServer );

static OSStatus
	DNSServerCreate(
		dispatch_queue_t		inQueue,
		DNSServerEventHandler_f	inEventHandler,
		void *					inEventContext,
		unsigned int			inResponseDelayMs,
		uint32_t				inDefaultTTL,
		int						inPort,
		Boolean					inLoopbackOnly,
		const char *			inDomain,
		Boolean					inBadUDPMode,
		DNSServerRef *			outServer )
{
	OSStatus			err;
	DNSServerRef		obj = NULL;
	
	require_action_quiet( inDefaultTTL <= INT32_MAX, exit, err = kRangeErr );
	
	CF_OBJECT_CREATE( DNSServer, obj, err, exit );
	
	ReplaceDispatchQueue( &obj->queue, inQueue );
	obj->eventHandler		= inEventHandler;
	obj->eventContext		= inEventContext;
	obj->responseDelayMs	= inResponseDelayMs;
	obj->defaultTTL			= inDefaultTTL;
	obj->port				= inPort;
	obj->loopbackOnly		= inLoopbackOnly;
	obj->badUDPMode			= inBadUDPMode;
	
	if( inDomain )
	{
		err = StringToDomainName( inDomain, &obj->domain, NULL );
		require_noerr_quiet( err, exit );
	}
	else
	{
		err = DomainNameDup( kDDotTestDomainName, &obj->domain, NULL );
		require_noerr_quiet( err, exit );
	}
	
	*outServer = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	CFReleaseNullSafe( obj );
	return( err );
}

//===========================================================================================================================
//	_DNSServerFinalize
//===========================================================================================================================

static void	_DNSServerFinalize( CFTypeRef inObj )
{
	DNSServerRef const		me = (DNSServerRef) inObj;
	
	check( !me->readSourceUDPv4 );
	check( !me->readSourceUDPv6 );
	check( !me->readSourceTCPv4 );
	check( !me->readSourceTCPv6 );
	check( !me->responseTimer );
	ForgetMem( &me->domain );
	dispatch_forget( &me->queue );
}

//===========================================================================================================================
//	DNSServerStart
//===========================================================================================================================

static void	_DNSServerStart( void *inContext );
static void	_DNSServerStop( void *inContext, OSStatus inError );

static void	DNSServerStart( DNSServerRef me )
{
	CFRetain( me );
	dispatch_async_f( me->queue, me, _DNSServerStart );
}

static void	_DNSServerStart( void *inContext )
{
	OSStatus				err;
	struct timeval			now;
	DNSServerRef const		me			= (DNSServerRef) inContext;
	SocketRef				sock		= kInvalidSocketRef;
	SocketContext *			sockCtx		= NULL;
	const uint32_t			loopbackV4	= htonl( INADDR_LOOPBACK );
	int						year, month, day;
	
	// Create IPv4 UDP socket.
	// Initially, me->port is the port requested by the user. If it's 0, then the user wants any available ephemeral port.
	// If it's negative, then the user would like a port number equal to its absolute value, but will settle for any
	// available ephemeral port, if it's not available. The actual port number that was used will be stored in me->port and
	// used for the remaining sockets.
	
	err = _ServerSocketOpenEx2( AF_INET, SOCK_DGRAM, IPPROTO_UDP, me->loopbackOnly ? &loopbackV4 : NULL,
		me->port, &me->port, kSocketBufferSize_DontSet, me->loopbackOnly ? true : false, &sock );
	require_noerr( err, exit );
	check( me->port > 0 );
	
	// Create read source for IPv4 UDP socket.
	
	err = SocketContextCreate( sock, me, &sockCtx );
	require_noerr( err, exit );
	sock = kInvalidSocketRef;
	
	err = DispatchReadSourceCreate( sockCtx->sock, me->queue, _DNSServerUDPReadHandler, SocketContextCancelHandler, sockCtx,
		&me->readSourceUDPv4 );
	require_noerr( err, exit );
	dispatch_resume( me->readSourceUDPv4 );
	me->sockUDPv4 = sockCtx->sock;
	sockCtx = NULL;
	
	// Create IPv6 UDP socket.
	
	err = _ServerSocketOpenEx2( AF_INET6, SOCK_DGRAM, IPPROTO_UDP, me->loopbackOnly ? &in6addr_loopback : NULL,
		me->port, NULL, kSocketBufferSize_DontSet, me->loopbackOnly ? true : false, &sock );
	require_noerr( err, exit );
	
	// Create read source for IPv6 UDP socket.
	
	err = SocketContextCreate( sock, me, &sockCtx );
	require_noerr( err, exit );
	sock = kInvalidSocketRef;
	
	err = DispatchReadSourceCreate( sockCtx->sock, me->queue, _DNSServerUDPReadHandler, SocketContextCancelHandler, sockCtx,
		&me->readSourceUDPv6 );
	require_noerr( err, exit );
	dispatch_resume( me->readSourceUDPv6 );
	me->sockUDPv6 = sockCtx->sock;
	sockCtx = NULL;
	
	// Create IPv4 TCP socket.
	
	err = _ServerSocketOpenEx2( AF_INET, SOCK_STREAM, IPPROTO_TCP, me->loopbackOnly ? &loopbackV4 : NULL,
		me->port, NULL, kSocketBufferSize_DontSet, false, &sock );
	require_noerr( err, exit );
	
	// Create read source for IPv4 TCP socket.
	
	err = SocketContextCreate( sock, me, &sockCtx );
	require_noerr( err, exit );
	sock = kInvalidSocketRef;
	
	err = DispatchReadSourceCreate( sockCtx->sock, me->queue, _DNSServerTCPReadHandler, SocketContextCancelHandler, sockCtx,
		&me->readSourceTCPv4 );
	require_noerr( err, exit );
	dispatch_resume( me->readSourceTCPv4 );
	sockCtx = NULL;
	
	// Create IPv6 TCP socket.
	
	err = _ServerSocketOpenEx2( AF_INET6, SOCK_STREAM, IPPROTO_TCP, me->loopbackOnly ? &in6addr_loopback : NULL,
		me->port, NULL, kSocketBufferSize_DontSet, false, &sock );
	require_noerr( err, exit );
	
	// Create read source for IPv6 TCP socket.
	
	err = SocketContextCreate( sock, me, &sockCtx );
	require_noerr( err, exit );
	sock = kInvalidSocketRef;
	
	err = DispatchReadSourceCreate( sockCtx->sock, me->queue, _DNSServerTCPReadHandler, SocketContextCancelHandler, sockCtx,
		&me->readSourceTCPv6 );
	require_noerr( err, exit );
	dispatch_resume( me->readSourceTCPv6 );
	sockCtx = NULL;
	
	ds_ulog( kLogLevelInfo, "Server is using port %d.\n", me->port );
	if( me->eventHandler ) me->eventHandler( kDNSServerEvent_Started, (uintptr_t) me->port, me->eventContext );
	
	// Create the serial number for the server's SOA record in the YYYMMDDnn convention recommended by
	// <https://tools.ietf.org/html/rfc1912#section-2.2> using the current time.
	
	gettimeofday( &now, NULL );
	SecondsToYMD_HMS( ( INT64_C_safe( kDaysToUnixEpoch ) * kSecondsPerDay ) + now.tv_sec, &year, &month, &day,
		NULL, NULL, NULL );
	me->serial = (uint32_t)( ( year * 1000000 ) + ( month * 10000 ) + ( day * 100 ) + 1 );
	
exit:
	ForgetSocket( &sock );
	if( sockCtx ) SocketContextRelease( sockCtx );
	if( err ) _DNSServerStop( me, err );
}

//===========================================================================================================================
//	DNSServerStop
//===========================================================================================================================

static void	_DNSServerUserStop( void *inContext );
static void	_DNSServerStop2( void *inContext );

static void	DNSServerStop( DNSServerRef me )
{
	CFRetain( me );
	dispatch_async_f( me->queue, me, _DNSServerUserStop );
}

static void	_DNSServerUserStop( void *inContext )
{
	DNSServerRef const		me = (DNSServerRef) inContext;
	
	_DNSServerStop( me, kNoErr );
	CFRelease( me );
}

static void	_DNSServerStop( void *inContext, OSStatus inError )
{
	DNSServerRef const		me = (DNSServerRef) inContext;
	
	me->stopError = inError;
	dispatch_source_forget( &me->readSourceUDPv4 );
	dispatch_source_forget( &me->readSourceUDPv6 );
	dispatch_source_forget( &me->readSourceTCPv4 );
	dispatch_source_forget( &me->readSourceTCPv6 );
	dispatch_source_forget( &me->responseTimer );
	me->sockUDPv4 = kInvalidSocketRef;
	me->sockUDPv6 = kInvalidSocketRef;
	
	if( me->responseList )
	{
		_DNSDelayedResponseFreeList( me->responseList );
		me->responseList = NULL;
	}
	dispatch_async_f( me->queue, me, _DNSServerStop2 );
}

static void	_DNSServerStop2( void *inContext )
{
	DNSServerRef const		me = (DNSServerRef) inContext;
	
	if( !me->stopped )
	{
		me->stopped = true;
		if( me->eventHandler ) me->eventHandler( kDNSServerEvent_Stopped, (uintptr_t) me->stopError, me->eventContext );
		CFRelease( me );
	}
	CFRelease( me );
}

//===========================================================================================================================
//	_DNSDelayedResponseFree
//===========================================================================================================================

static void	_DNSDelayedResponseFree( DNSDelayedResponse *inResponse )
{
	ForgetMem( &inResponse->msgPtr );
	free( inResponse );
}

//===========================================================================================================================
//	_DNSDelayedResponseFreeList
//===========================================================================================================================

static void	_DNSDelayedResponseFreeList( DNSDelayedResponse *inList )
{
	DNSDelayedResponse *		response;
	
	while( ( response = inList ) != NULL )
	{
		inList = response->next;
		_DNSDelayedResponseFree( response );
	}
}

//===========================================================================================================================
//	_DNSServerUDPReadHandler
//===========================================================================================================================

static OSStatus
	_DNSServerAnswerQuery(
		DNSServerRef	inServer,
		const uint8_t *	inQueryPtr,
		size_t			inQueryLen,
		Boolean			inForTCP,
		uint8_t **		outResponsePtr,
		size_t *		outResponseLen );

#define _DNSServerAnswerQueryForUDP( IN_SERVER, IN_QUERY_PTR, IN_QUERY_LEN, IN_RESPONSE_PTR, IN_RESPONSE_LEN ) \
	_DNSServerAnswerQuery( IN_SERVER, IN_QUERY_PTR, IN_QUERY_LEN, false, IN_RESPONSE_PTR, IN_RESPONSE_LEN )

#define _DNSServerAnswerQueryForTCP( IN_SERVER, IN_QUERY_PTR, IN_QUERY_LEN, IN_RESPONSE_PTR, IN_RESPONSE_LEN ) \
	_DNSServerAnswerQuery( IN_SERVER, IN_QUERY_PTR, IN_QUERY_LEN, true, IN_RESPONSE_PTR, IN_RESPONSE_LEN )

static OSStatus
	_DNSServerScheduleDelayedResponse(
		DNSServerRef			inServer,
		const struct sockaddr *	inDestAddr,
		uint8_t *				inMsgPtr,
		size_t					inMsgLen );
static void	_DNSServerUDPDelayedSend( void *inContext );

static void	_DNSServerUDPReadHandler( void *inContext )
{
	OSStatus					err;
	SocketContext * const		sockCtx		= (SocketContext *) inContext;
	DNSServerRef const			me			= (DNSServerRef) sockCtx->userContext;
	struct timeval				now;
	ssize_t						n;
	sockaddr_ip					clientAddr;
	socklen_t					clientAddrLen;
	uint8_t *					responsePtr	= NULL;	// malloc'd
	size_t						responseLen;
	uint8_t						msg[ 512 ];
	
	gettimeofday( &now, NULL );
	
	// Receive message.
	
	clientAddrLen = (socklen_t) sizeof( clientAddr );
	n = recvfrom( sockCtx->sock, (char *) msg, sizeof( msg ), 0, &clientAddr.sa, &clientAddrLen );
	err = map_socket_value_errno( sockCtx->sock, n >= 0, n );
	require_noerr( err, exit );
	
	ds_ulog( kLogLevelInfo, "UDP server received %zd bytes from %##a at %{du:time}.\n", n, &clientAddr, &now );
	
	if( n < kDNSHeaderLength )
	{
		ds_ulog( kLogLevelInfo, "UDP DNS message is too small (%zd < %d).\n", n, kDNSHeaderLength );
		goto exit;
	}
	
	ds_ulog( kLogLevelInfo, "UDP received message:\n\n%1{du:dnsmsg}", msg, (size_t) n );
	
	// Create response.
	
	err = _DNSServerAnswerQueryForUDP( me, msg, (size_t) n, &responsePtr, &responseLen );
	require_noerr_quiet( err, exit );
	
	// Schedule response.
	
	if( me->responseDelayMs > 0 )
	{
		err = _DNSServerScheduleDelayedResponse( me, &clientAddr.sa, responsePtr, responseLen );
		require_noerr( err, exit );
		responsePtr = NULL;
	}
	else
	{
		ds_ulog( kLogLevelInfo, "UDP sending %zu byte response:\n\n%1{du:dnsmsg}", responseLen, responsePtr, responseLen );
		
		n = sendto( sockCtx->sock, (char *) responsePtr, responseLen, 0, &clientAddr.sa, clientAddrLen );
		err = map_socket_value_errno( sockCtx->sock, n == (ssize_t) responseLen, n );
		require_noerr( err, exit );
	}
	
exit:
	FreeNullSafe( responsePtr );
	return;
}

static OSStatus
	_DNSServerScheduleDelayedResponse(
		DNSServerRef			me,
		const struct sockaddr *	inDestAddr,
		uint8_t *				inMsgPtr,
		size_t					inMsgLen )
{
	OSStatus					err;
	DNSDelayedResponse *		response;
	DNSDelayedResponse **		responsePtr;
	DNSDelayedResponse *		newResponse;
	uint64_t					targetTicks;
	
	targetTicks = UpTicks() + MillisecondsToUpTicks( me->responseDelayMs );
	
	newResponse = (DNSDelayedResponse *) calloc( 1, sizeof( *newResponse ) );
	require_action( newResponse, exit, err = kNoMemoryErr );
	
	if( !me->responseList || ( targetTicks < me->responseList->targetTicks ) )
	{
		dispatch_source_forget( &me->responseTimer );
		
		err = DispatchTimerCreate( dispatch_time_milliseconds( me->responseDelayMs ), DISPATCH_TIME_FOREVER,
			( (uint64_t) me->responseDelayMs ) * kNanosecondsPerMillisecond / 10, me->queue, _DNSServerUDPDelayedSend,
			NULL, me, &me->responseTimer );
		require_noerr( err, exit );
		dispatch_resume( me->responseTimer );
	}
	
	SockAddrCopy( inDestAddr, &newResponse->destAddr );
	newResponse->targetTicks	= targetTicks;
	newResponse->msgPtr			= inMsgPtr;
	newResponse->msgLen			= inMsgLen;
	
	for( responsePtr = &me->responseList; ( response = *responsePtr ) != NULL; responsePtr = &response->next )
	{
		if( newResponse->targetTicks < response->targetTicks ) break;
	}
	newResponse->next = response;
	*responsePtr = newResponse;
	newResponse = NULL;
	err = kNoErr;
	
exit:
	if( newResponse ) _DNSDelayedResponseFree( newResponse );
	return( err );
}

static void	_DNSServerUDPDelayedSend( void *inContext )
{
	OSStatus					err;
	DNSServerRef const			me			= (DNSServerRef) inContext;
	DNSDelayedResponse *		response;
	SocketRef					sock;
	ssize_t						n;
	uint64_t					nowTicks;
	uint64_t					remainingNs;
	DNSDelayedResponse *		freeList	= NULL;
	
	dispatch_source_forget( &me->responseTimer );
	
	nowTicks = UpTicks();
	while( ( ( response = me->responseList ) != NULL ) && ( response->targetTicks <= nowTicks ) )
	{
		me->responseList = response->next;
		
		ds_ulog( kLogLevelInfo, "UDP sending %zu byte response (delayed):\n\n%1{du:dnsmsg}",
			response->msgLen, response->msgPtr, response->msgLen );
		
		sock = ( response->destAddr.sa.sa_family == AF_INET ) ? me->sockUDPv4 : me->sockUDPv6;
		n = sendto( sock, (char *) response->msgPtr, response->msgLen, 0, &response->destAddr.sa,
			SockAddrGetSize( &response->destAddr ) );
		err = map_socket_value_errno( sock, n == (ssize_t) response->msgLen, n );
		check_noerr( err );
		
		response->next	= freeList;
		freeList		= response;
		nowTicks = UpTicks();
	}
	
	if( response )
	{
		check( response->targetTicks > nowTicks );
		remainingNs = UpTicksToNanoseconds( response->targetTicks - nowTicks );
		if( remainingNs > INT64_MAX ) remainingNs = INT64_MAX;
		
		err = DispatchTimerCreate( dispatch_time( DISPATCH_TIME_NOW, (int64_t) remainingNs ), DISPATCH_TIME_FOREVER, 0,
			me->queue, _DNSServerUDPDelayedSend, NULL, me, &me->responseTimer );
		require_noerr( err, exit );
		dispatch_resume( me->responseTimer );
	}
	
exit:
	if( freeList ) _DNSDelayedResponseFreeList( freeList );
}

//===========================================================================================================================
//	_DNSServerAnswerQuery
//===========================================================================================================================

#define kLabelPrefix_Alias			"alias"
#define kLabelPrefix_AliasTTL		"alias-ttl"
#define kLabelPrefix_Count			"count-"
#define kLabelPrefix_Tag			"tag-"
#define kLabelPrefix_TTL			"ttl-"
#define kLabel_IPv4					"ipv4"
#define kLabel_IPv6					"ipv6"
#define kLabelPrefix_SRV			"srv-"

#define kMaxAliasTTLCount		( ( kDomainLabelLengthMax - sizeof_string( kLabelPrefix_AliasTTL ) ) / 2 )
#define kMaxParsedSRVCount		( kDomainNameLengthMax / ( 1 + sizeof_string( kLabelPrefix_SRV ) + 5 ) )

typedef struct
{
	uint16_t			priority;	// Priority from SRV label.
	uint16_t			weight;		// Weight from SRV label.
	uint16_t			port;		// Port number from SRV label.
	uint16_t			targetLen;	// Total length of the target hostname labels that follow an SRV label.
	const uint8_t *		targetPtr;	// Pointer to the target hostname embedded in a domain name.
	
}	ParsedSRV;

static OSStatus
	_DNSServerInitializeResponseMessage(
		DataBuffer *	inDB,
		unsigned int	inID,
		unsigned int	inFlags,
		const uint8_t *	inQName,
		unsigned int	inQType,
		unsigned int	inQClass );
static OSStatus
	_DNSServerAnswerQueryDynamically(
		DNSServerRef	inServer,
		const uint8_t *	inQName,
		unsigned int	inQType,
		unsigned int	inQClass,
		Boolean			inForTCP,
		DataBuffer *	inDB );
static Boolean
	_DNSServerNameIsSRVName(
		DNSServerRef		inServer,
		const uint8_t *		inName,
		const uint8_t **	outDomainPtr,
		size_t *			outDomainLen,
		ParsedSRV			inSRVArray[ kMaxParsedSRVCount ],
		size_t *			outSRVCount );
static Boolean
	_DNSServerNameIsHostname(
		DNSServerRef	inServer,
		const uint8_t *	inName,
		uint32_t *		outAliasCount,
		uint32_t		inAliasTTLs[ kMaxAliasTTLCount ],
		size_t *		outAliasTTLCount,
		unsigned int *	outCount,
		unsigned int *	outRandCount,
		uint32_t *		outTTL,
		Boolean *		outHasA,
		Boolean *		outHasAAAA,
		Boolean *		outHasSOA );

static OSStatus
	_DNSServerAnswerQuery(
		DNSServerRef			me,
		const uint8_t * const	inQueryPtr,
		const size_t			inQueryLen,
		Boolean					inForTCP,
		uint8_t **				outResponsePtr,
		size_t *				outResponseLen )
{
	OSStatus								err;
	DataBuffer								dataBuf;
	const uint8_t *							ptr;
	const uint8_t * const					queryEnd = &inQueryPtr[ inQueryLen ];
	const DNSHeader *						qhdr;
	const dns_fixed_fields_question *		fields;
	unsigned int							msgID, qflags, qtype, qclass, rflags;
	uint8_t									qname[ kDomainNameLengthMax ];
	
	DataBuffer_Init( &dataBuf, NULL, 0, kDNSMaxTCPMessageSize );
	
	require_action_quiet( inQueryLen >= kDNSHeaderLength, exit, err = kUnderrunErr );
	
	qhdr	= (const DNSHeader *) inQueryPtr;
	msgID	= DNSHeaderGetID( qhdr );
	qflags	= DNSHeaderGetFlags( qhdr );
	
	// Minimal checking of the query message's header.
	
	if( ( qflags & kDNSHeaderFlag_Response ) ||					// The message must be a query, not a response.
		( DNSFlagsGetOpCode( qflags ) != kDNSOpCode_Query ) ||	// OPCODE must be QUERY (standard query).
		( DNSHeaderGetQuestionCount( qhdr ) != 1 ) )			// There should be a single question.
	{
		err = kRequestErr;
		goto exit;
	}
	
	// Get QNAME.
	
	ptr = (const uint8_t *) &qhdr[ 1 ];
	err = DNSMessageExtractDomainName( inQueryPtr, inQueryLen, ptr, qname, &ptr );
	require_noerr( err, exit );
	
	// Get QTYPE and QCLASS.
	
	require_action_quiet( ( (size_t)( queryEnd - ptr ) ) >= sizeof( *fields ), exit, err = kUnderrunErr );
	fields	= (const dns_fixed_fields_question *) ptr;
	qtype	= dns_fixed_fields_question_get_type( fields );
	qclass	= dns_fixed_fields_question_get_class( fields );
	
	// Create a tentative response message.
	
	rflags = kDNSHeaderFlag_Response;
	if( qflags & kDNSHeaderFlag_RecursionDesired ) rflags |= kDNSHeaderFlag_RecursionDesired;
	DNSFlagsSetOpCode( rflags, kDNSOpCode_Query );
	
	if( me->badUDPMode && !inForTCP ) msgID = (uint16_t)( msgID + 1 );
	err = _DNSServerInitializeResponseMessage( &dataBuf, msgID, rflags, qname, qtype, qclass );
	require_noerr( err, exit );
	
	err = _DNSServerAnswerQueryDynamically( me, qname, qtype, qclass, inForTCP, &dataBuf );
	if( err )
	{
		DNSFlagsSetRCode( rflags, kDNSRCode_ServerFailure );
		err = _DNSServerInitializeResponseMessage( &dataBuf, msgID, rflags, qname, qtype, qclass );
		require_noerr( err, exit );
	}
	
	err = DataBuffer_Detach( &dataBuf, outResponsePtr, outResponseLen );
	require_noerr( err, exit );
	
exit:
	DataBuffer_Free( &dataBuf );
	return( err );
}

static OSStatus
	_DNSServerInitializeResponseMessage(
		DataBuffer *	inDB,
		unsigned int	inID,
		unsigned int	inFlags,
		const uint8_t *	inQName,
		unsigned int	inQType,
		unsigned int	inQClass )
{
	OSStatus		err;
	DNSHeader		header;
	
	DataBuffer_Reset( inDB );
	
	memset( &header, 0, sizeof( header ) );
	DNSHeaderSetID( &header, inID );
	DNSHeaderSetFlags( &header, inFlags );
	DNSHeaderSetQuestionCount( &header, 1 );
	
	err = DataBuffer_Append( inDB, &header, sizeof( header ) );
	require_noerr( err, exit );
	
	err = _DataBuffer_AppendDNSQuestion( inDB, inQName, DomainNameLength( inQName ), (uint16_t) inQType,
		(uint16_t) inQClass );
	require_noerr( err, exit );
	
exit:
	return( err );
}

static OSStatus
	_DNSServerAnswerQueryDynamically(
		DNSServerRef			me,
		const uint8_t * const	inQName,
		const unsigned int		inQType,
		const unsigned int		inQClass,
		const Boolean			inForTCP,
		DataBuffer * const		inDB )
{
	OSStatus					err;
	DNSHeader *					hdr;
	unsigned int				flags, rcode;
	uint32_t					aliasCount, i;
	uint32_t					aliasTTLs[ kMaxAliasTTLCount ];
	size_t						aliasTTLCount;
	unsigned int				addrCount, randCount;
	uint32_t					ttl;
	ParsedSRV					srvArray[ kMaxParsedSRVCount ];
	size_t						srvCount;
	const uint8_t *				srvDomainPtr;
	size_t						srvDomainLen;
	unsigned int				answerCount;
	Boolean						notImplemented, truncated;
	Boolean						useAliasTTLs, nameExists, nameHasA, nameHasAAAA, nameHasSRV, nameHasSOA;
	uint8_t						namePtr[ 2 ];
	dns_fixed_fields_record		fields;
	
	answerCount	= 0;
	truncated	= false;
	nameExists	= false;
	require_action_quiet( inQClass == kDNSServiceClass_IN, done, notImplemented = true );
	
	notImplemented	= false;
	aliasCount		= 0;
	nameHasA		= false;
	nameHasAAAA		= false;
	nameHasSOA		= false;
	useAliasTTLs	= false;
	nameHasSRV		= false;
	srvDomainLen	= 0;
	srvCount		= 0;
	
	if( _DNSServerNameIsHostname( me, inQName, &aliasCount, aliasTTLs, &aliasTTLCount, &addrCount, &randCount, &ttl,
		&nameHasA, &nameHasAAAA, &nameHasSOA ) )
	{
		check( !( ( aliasCount > 0 ) && ( aliasTTLCount > 0 ) ) );
		check( ( addrCount >= 1 ) && ( addrCount <= 255 ) );
		check( ( randCount == 0 ) || ( ( randCount >= addrCount ) && ( randCount <= 255 ) ) );
		check( nameHasA || nameHasAAAA );
		
		if( aliasTTLCount > 0 )
		{
			aliasCount		= (uint32_t) aliasTTLCount;
			useAliasTTLs	= true;
		}
		nameExists = true;
	}
	else if( _DNSServerNameIsSRVName( me, inQName, &srvDomainPtr, &srvDomainLen, srvArray, &srvCount ) )
	{
		nameHasSRV = true;
		nameExists = true;
	}
	require_quiet( nameExists, done );
	
	if( aliasCount > 0 )
	{
		size_t				nameOffset;
		uint8_t				rdataLabel[ 1 + kDomainLabelLengthMax + 1 ];
		
		// If aliasCount is non-zero, then the first label of QNAME is either "alias" or "alias-<N>". superPtr is a name
		// compression pointer to the second label of QNAME, i.e., the immediate superdomain name of QNAME. It's used for
		// the RDATA of CNAME records whose canonical name ends with the superdomain name. It may also be used to construct
		// CNAME record names, when the offset to the previous CNAME's RDATA doesn't fit in a compression pointer.
		
		const uint8_t		superPtr[ 2 ] = { 0xC0, (uint8_t)( kDNSHeaderLength + 1 + inQName[ 0 ] ) };
		
		// The name of the first CNAME record is equal to QNAME, so nameOffset is set to offset of QNAME.
		
		nameOffset = kDNSHeaderLength;
		
		for( i = aliasCount; i >= 1; --i )
		{
			size_t			nameLen;
			size_t			rdataLen;
			uint32_t		j;
			uint32_t		aliasTTL;
			uint8_t			nameLabel[ 1 + kDomainLabelLengthMax ];
			
			if( nameOffset <= kDNSCompressionOffsetMax )
			{
				DNSMessageWriteLabelPointer( namePtr, nameOffset );
				nameLen = sizeof( namePtr );
			}
			else
			{
				memcpy( nameLabel, rdataLabel, 1 + rdataLabel[ 0 ] );
				nameLen = 1 + nameLabel[ 0 ] + sizeof( superPtr );
			}
			
			if( i >= 2 )
			{
				char *				dst = (char *) &rdataLabel[ 1 ];
				char * const		lim = (char *) &rdataLabel[ countof( rdataLabel ) ];
				
				if( useAliasTTLs )
				{
					err = SNPrintF_Add( &dst, lim, kLabelPrefix_AliasTTL );
					require_noerr( err, exit );
					
					for( j = aliasCount - ( i - 1 ); j < aliasCount; ++j )
					{
						err = SNPrintF_Add( &dst, lim, "-%u", aliasTTLs[ j ] );
						require_noerr( err, exit );
					}
				}
				else
				{
					err = SNPrintF_Add( &dst, lim, kLabelPrefix_Alias "%?{end}-%u", i == 2, i - 1 );
					require_noerr( err, exit );
				}
				rdataLabel[ 0 ]	= (uint8_t)( dst - (char *) &rdataLabel[ 1 ] );
				rdataLen		= 1 + rdataLabel[ 0 ] + sizeof( superPtr );
			}
			else
			{
				rdataLen = sizeof( superPtr );
			}
			
			if( !inForTCP )
			{
				size_t		recordLen = nameLen + sizeof( fields ) + rdataLen;
				
				if( ( DataBuffer_GetLen( inDB ) + recordLen ) > kDNSMaxUDPMessageSize )
				{
					truncated = true;
					goto done;
				}
			}
			++answerCount;
			
			// Set CNAME record's NAME.
			
			if( nameOffset <= kDNSCompressionOffsetMax )
			{
				err = DataBuffer_Append( inDB, namePtr, sizeof( namePtr ) );
				require_noerr( err, exit );
			}
			else
			{
				err = DataBuffer_Append( inDB, nameLabel, 1 + nameLabel[ 0 ] );
				require_noerr( err, exit );
				
				err = DataBuffer_Append( inDB, superPtr, sizeof( superPtr ) );
				require_noerr( err, exit );
			}
			
			// Set CNAME record's TYPE, CLASS, TTL, and RDLENGTH.
			
			aliasTTL = useAliasTTLs ? aliasTTLs[ aliasCount - i ] : me->defaultTTL;
			dns_fixed_fields_record_init( &fields, kDNSServiceType_CNAME, kDNSServiceClass_IN, aliasTTL,
				(uint16_t) rdataLen );
			err = DataBuffer_Append( inDB, &fields, sizeof( fields ) );
			require_noerr( err, exit );
			
			// Save offset of CNAME record's RDATA, which may be used for the name of the next CNAME record.
			
			nameOffset = DataBuffer_GetLen( inDB );
			
			// Set CNAME record's RDATA.
			
			if( i >= 2 )
			{
				err = DataBuffer_Append( inDB, rdataLabel, 1 + rdataLabel[ 0 ] );
				require_noerr( err, exit );
			}
			err = DataBuffer_Append( inDB, superPtr, sizeof( superPtr ) );
			require_noerr( err, exit );
		}
		
		namePtr[ 0 ] = superPtr[ 0 ];
		namePtr[ 1 ] = superPtr[ 1 ];
	}
	else
	{
		// There are no aliases, so initialize the name compression pointer to point to QNAME.
		
		DNSMessageWriteLabelPointer( namePtr, kDNSHeaderLength );
	}
	
	if( ( inQType == kDNSServiceType_A ) || ( inQType == kDNSServiceType_AAAA ) )
	{
		uint8_t *		lsb;					// Pointer to the least significant byte of record data.
		size_t			recordLen;				// Length of the entire record.
		size_t			rdataLen;				// Length of record's RDATA.
		uint8_t			rdata[ 16 ];			// A buffer that's big enough for either A or AAAA RDATA.
		uint8_t			randIntegers[ 255 ];	// Array for random integers in [1, 255].
		const int		useBadAddrs = ( me->badUDPMode && !inForTCP ) ? true : false;
		
		if( inQType == kDNSServiceType_A )
		{
			const uint32_t		baseAddrV4 = useBadAddrs ? kDNSServerBadBaseAddrV4 : kDNSServerBaseAddrV4;
			
			require_quiet( nameHasA, done );
			
			rdataLen = 4;
			WriteBig32( rdata, baseAddrV4 );
			lsb = &rdata[ 3 ];
		}
		else
		{
			const uint8_t * const		baseAddrV6 = useBadAddrs ? kDNSServerBadBaseAddrV6 : kDNSServerBaseAddrV6;
			
			require_quiet( nameHasAAAA, done );
			
			rdataLen = 16;
			memcpy( rdata, baseAddrV6, 16 );
			lsb = &rdata[ 15 ];
		}
		
		if( randCount > 0 )
		{
			// Populate the array with all integers between 1 and <randCount>, inclusive.
			
			for( i = 0; i < randCount; ++i ) randIntegers[ i ] = (uint8_t)( i + 1 );
			
			// Prevent dubious static analyzer warning.
			// Note: _DNSServerNameIsHostname() already enforces randCount >= addrCount. Also, this require_fatal() check
			// needs to be placed right before the next for-loop. Any earlier, and the static analyzer warning will persist
			// for some reason.
			
			require_fatal( addrCount <= randCount, "Invalid Count label values: addrCount %u > randCount %u",
				addrCount, randCount );
			
			// Create a contiguous subarray starting at index 0 that contains <addrCount> randomly chosen integers between
			// 1 and <randCount>, inclusive.
			// Loop invariant 1: Array elements with indexes in [0, i - 1] have been randomly chosen.
			// Loop invariant 2: Array elements with indexes in [i, randCount - 1] are candidates for being chosen.
			
			for( i = 0; i < addrCount; ++i )
			{
				uint8_t			tmp;
				uint32_t		j;
				
				j = RandomRange( i, randCount - 1 );
				if( i != j )
				{
					tmp = randIntegers[ i ];
					randIntegers[ i ] = randIntegers[ j ];
					randIntegers[ j ] = tmp;
				}
			}
		}
		
		recordLen = sizeof( namePtr ) + sizeof( fields ) + rdataLen;
		for( i = 0; i < addrCount; ++i )
		{
			if( !inForTCP && ( ( DataBuffer_GetLen( inDB ) + recordLen ) > kDNSMaxUDPMessageSize ) )
			{
				truncated = true;
				goto done;
			}
			
			// Set record NAME.
			
			err = DataBuffer_Append( inDB, namePtr, sizeof( namePtr ) );
			require_noerr( err, exit );
			
			// Set record TYPE, CLASS, TTL, and RDLENGTH.
			
			dns_fixed_fields_record_init( &fields, (uint16_t) inQType, kDNSServiceClass_IN, ttl, (uint16_t) rdataLen );
			err = DataBuffer_Append( inDB, &fields, sizeof( fields ) );
			require_noerr( err, exit );
			
			// Set record RDATA.
			
			*lsb = ( randCount > 0 ) ? randIntegers[ i ] : ( *lsb + 1 );
			
			err = DataBuffer_Append( inDB, rdata, rdataLen );
			require_noerr( err, exit );
			
			++answerCount;
		}
	}
	else if( inQType == kDNSServiceType_SRV )
	{
		require_quiet( nameHasSRV, done );
		
		dns_fixed_fields_record_init( &fields, kDNSServiceType_SRV, kDNSServiceClass_IN, me->defaultTTL, 0 );
		
		for( i = 0; i < srvCount; ++i )
		{
			dns_fixed_fields_srv		fieldsSRV;
			size_t						rdataLen;
			size_t						recordLen;
			const ParsedSRV * const		srv = &srvArray[ i ];
			
			rdataLen  = sizeof( fieldsSRV ) + srvDomainLen + srv->targetLen + 1;
			recordLen = sizeof( namePtr ) + sizeof( fields ) + rdataLen;
			
			if( !inForTCP && ( ( DataBuffer_GetLen( inDB ) + recordLen ) > kDNSMaxUDPMessageSize ) )
			{
				truncated = true;
				goto done;
			}
			
			// Append record NAME.
			
			err = DataBuffer_Append( inDB, namePtr, sizeof( namePtr ) );
			require_noerr( err, exit );
			
			// Append record TYPE, CLASS, TTL, and RDLENGTH.
			
			WriteBig16( fields.rdlength, rdataLen );
			err = DataBuffer_Append( inDB, &fields, sizeof( fields ) );
			require_noerr( err, exit );
			
			// Append SRV RDATA.
			
			dns_fixed_fields_srv_init( &fieldsSRV, srv->priority, srv->weight, srv->port );
			
			err = DataBuffer_Append( inDB, &fieldsSRV, sizeof( fieldsSRV ) );
			require_noerr( err, exit );
			
			if( srv->targetLen > 0 )
			{
				err = DataBuffer_Append( inDB, srv->targetPtr, srv->targetLen );
				require_noerr( err, exit );
			}
			
			if( srvDomainLen > 0 )
			{
				err = DataBuffer_Append( inDB, srvDomainPtr, srvDomainLen );
				require_noerr( err, exit );
			}
			
			err = DataBuffer_Append( inDB, "", 1 );	// Append root label.
			require_noerr( err, exit );
			
			++answerCount;
		}
	}
	else if( inQType == kDNSServiceType_SOA )
	{
		size_t		nameLen, recordLen;
		
		require_quiet( nameHasSOA, done );
		
		nameLen	= DomainNameLength( me->domain );
		if( !inForTCP )
		{
			err = AppendSOARecord( NULL, me->domain, nameLen, 0, 0, 0, kRootLabel, kRootLabel, 0, 0, 0, 0, 0, &recordLen );
			require_noerr( err, exit );
			
			if( ( DataBuffer_GetLen( inDB ) + recordLen ) > kDNSMaxUDPMessageSize )
			{
				truncated = true;
				goto done;
			}
		}
		
		err = AppendSOARecord( inDB, me->domain, nameLen, kDNSServiceType_SOA, kDNSServiceClass_IN, me->defaultTTL,
			kRootLabel, kRootLabel, me->serial, 1 * kSecondsPerDay, 2 * kSecondsPerHour, 1000 * kSecondsPerHour,
			me->defaultTTL, NULL );
		require_noerr( err, exit );
		
		++answerCount;
	}
	
done:
	hdr = (DNSHeader *) DataBuffer_GetPtr( inDB );
	flags = DNSHeaderGetFlags( hdr );
	if( notImplemented )
	{
		rcode = kDNSRCode_NotImplemented;
	}
	else
	{
		flags |= kDNSHeaderFlag_AuthAnswer;
		if( truncated ) flags |= kDNSHeaderFlag_Truncation;
		rcode = nameExists ? kDNSRCode_NoError : kDNSRCode_NXDomain;
	}
	DNSFlagsSetRCode( flags, rcode );
	DNSHeaderSetFlags( hdr, flags );
	DNSHeaderSetAnswerCount( hdr, answerCount );
	err = kNoErr;
	
exit:
	return( err );
}

static Boolean
	_DNSServerNameIsHostname(
		DNSServerRef	me,
		const uint8_t *	inName,
		uint32_t *		outAliasCount,
		uint32_t		inAliasTTLs[ kMaxAliasTTLCount ],
		size_t *		outAliasTTLCount,
		unsigned int *	outCount,
		unsigned int *	outRandCount,
		uint32_t *		outTTL,
		Boolean *		outHasA,
		Boolean *		outHasAAAA,
		Boolean *		outHasSOA )
{
	OSStatus			err;
	const uint8_t *		label;
    const uint8_t *		nextLabel;
	uint32_t			aliasCount		= 0;	// Arg from Alias label. Valid values are in [2, 2^31 - 1].
	unsigned int		count			= 0;	// First arg from Count label. Valid values are in [1, 255].
	unsigned int		randCount		= 0;	// Second arg from Count label. Valid values are in [count, 255].
	int32_t				ttl				= -1;	// Arg from TTL label. Valid values are in [0, 2^31 - 1].
	size_t				aliasTTLCount	= 0;	// Count of TTL args from Alias-TTL label.
	int					hasTagLabel		= false;
	int					hasIPv4Label	= false;
	int					hasIPv6Label	= false;
	int					isNameValid		= false;
	
	for( label = inName; label[ 0 ]; label = nextLabel )
	{
		uint32_t		arg;
		
		nextLabel = &label[ 1 + label[ 0 ] ];
		
		// Check if the first label is a valid alias TTL sequence label.
		
		if( ( label == inName ) && ( strnicmp_prefix( &label[ 1 ], label[ 0 ], kLabelPrefix_AliasTTL ) == 0 ) )
		{
			const char *			ptr = (const char *) &label[ 1 + sizeof_string( kLabelPrefix_AliasTTL ) ];
			const char * const		end = (const char *) nextLabel;
			const char *			next;
			
			check( label[ 0 ] <= kDomainLabelLengthMax );
			
			while( ptr < end )
			{
				if( *ptr != '-' ) break;
				++ptr;
				err = DecimalTextToUInt32( ptr, end, &arg, &next );
				if( err || ( arg > INT32_MAX ) ) break;	// TTL must be in [0, 2^31 - 1].
				inAliasTTLs[ aliasTTLCount++ ] = arg;
				ptr = next;
			}
			if( ( aliasTTLCount == 0 ) || ( ptr != end ) ) break;
		}
		
		// Check if the first label is a valid alias label.
		
		else if( ( label == inName ) && ( strnicmp_prefix( &label[ 1 ], label[ 0 ], kLabelPrefix_Alias ) == 0 ) )
		{
			const char *			ptr = (const char *) &label[ 1 + sizeof_string( kLabelPrefix_Alias ) ];
			const char * const		end = (const char *) nextLabel;
			
			if( ptr < end )
			{
				if( *ptr++ != '-' ) break;
				err = DecimalTextToUInt32( ptr, end, &arg, &ptr );
				if( err || ( arg < 2 ) || ( arg > INT32_MAX ) ) break;	// Alias count must be in [2, 2^31 - 1].
				aliasCount = arg;
				if( ptr != end ) break;
			}
			else
			{
				aliasCount = 1;
			}
		}
		
		// Check if this label is a valid count label.
		
		else if( strnicmp_prefix( &label[ 1 ], label[ 0 ], kLabelPrefix_Count ) == 0  )
		{
			const char *			ptr = (const char *) &label[ 1 + sizeof_string( kLabelPrefix_Count ) ];
			const char * const		end = (const char *) nextLabel;
			
			if( count > 0 ) break;	// Count cannot be specified more than once.
			
			err = DecimalTextToUInt32( ptr, end, &arg, &ptr );
			if( err || ( arg < 1 ) || ( arg > 255 ) ) break;	// Count must be in [1, 255].
			count = (unsigned int) arg;
			
			if( ptr < end )
			{
				if( *ptr++ != '-' ) break;
				err = DecimalTextToUInt32( ptr, end, &arg, &ptr );
				if( err || ( arg < (uint32_t) count ) || ( arg > 255 ) ) break;	// Rand count must be in [count, 255].
				randCount = (unsigned int) arg;
				if( ptr != end ) break;
			}
		}
		
		// Check if this label is a valid TTL label.
		
		else if( strnicmp_prefix( &label[ 1 ], label[ 0 ], kLabelPrefix_TTL ) == 0  )
		{
			const char *			ptr = (const char *) &label[ 1 + sizeof_string( kLabelPrefix_TTL ) ];
			const char * const		end = (const char *) nextLabel;
			
			if( ttl >= 0 ) break;	// TTL cannot be specified more than once.
			
			err = DecimalTextToUInt32( ptr, end, &arg, &ptr );
			if( err || ( arg > INT32_MAX ) ) break;	// TTL must be in [0, 2^31 - 1].
			ttl = (int32_t) arg;
			if( ptr != end ) break;
		}
		
		// Check if this label is a valid IPv4 label.
		
		else if( strnicmpx( &label[ 1 ], label[ 0 ], kLabel_IPv4 ) == 0 )
		{
			if( hasIPv4Label || hasIPv6Label ) break;	// Valid names have at most one IPv4 or IPv6 label.
			hasIPv4Label = true;
		}
		
		// Check if this label is a valid IPv6 label.
		
		else if( strnicmpx( &label[ 1 ], label[ 0 ], kLabel_IPv6 ) == 0 )
		{
			if( hasIPv4Label || hasIPv6Label ) break;	// Valid names have at most one IPv4 or IPv6 label.
			hasIPv6Label = true;
		}
		
		// Check if this label is a valid tag label.
		
		else if( strnicmp_prefix( &label[ 1 ], label[ 0 ], kLabelPrefix_Tag ) == 0  )
		{
			hasTagLabel = true;
		}
		
		// If this and the remaining labels are equal to "d.test.", then the name exists. Otherwise, this label is invalid.
		// In both cases, there are no more labels to check.
		
		else
		{
			if( DomainNameEqual( label, me->domain ) ) isNameValid = true;
			break;
		}
	}
	require_quiet( isNameValid, exit );
	
	if( outAliasCount )		*outAliasCount		= aliasCount;
	if( outAliasTTLCount )	*outAliasTTLCount	= aliasTTLCount;
	if( outCount )			*outCount			= ( count > 0 ) ? count : 1;
	if( outRandCount )		*outRandCount		= randCount;
	if( outTTL )			*outTTL				= ( ttl >= 0 ) ? ( (uint32_t) ttl ) : me->defaultTTL;
	if( outHasA )			*outHasA			= ( hasIPv4Label || !hasIPv6Label ) ? true : false;
	if( outHasAAAA )		*outHasAAAA			= ( hasIPv6Label || !hasIPv4Label ) ? true : false;
	if( outHasSOA )
	{
		*outHasSOA = ( !count && ( ttl < 0 ) && !hasIPv4Label && !hasIPv6Label && !hasTagLabel ) ? true : false;
	}
	
exit:
	return( isNameValid ? true : false );
}

static Boolean
	_DNSServerNameIsSRVName(
		DNSServerRef		me,
		const uint8_t *		inName,
		const uint8_t **	outDomainPtr,
		size_t *			outDomainLen,
		ParsedSRV			inSRVArray[ kMaxParsedSRVCount ],
		size_t *			outSRVCount )
{
	OSStatus			err;
	const uint8_t *		label;
	const uint8_t *		domainPtr;
	size_t				domainLen;
	size_t				srvCount;
	uint32_t			arg;
	int					isNameValid = false;
	
	label = inName;
	
	// Ensure that first label, i.e, the service label, begins with a '_' character.
	
	require_quiet( ( label[ 0 ] > 0 ) && ( label[ 1 ] == '_' ), exit );
	label = DomainNameGetNextLabel( label );
	
	// Ensure that the second label, i.e., the proto label, begins with a '_' character (usually _tcp or _udp).
	
	require_quiet( ( label[ 0 ] > 0 ) && ( label[ 1 ] == '_' ), exit );
	label = DomainNameGetNextLabel( label );
	
	// Parse the domain name, if any.
	
	domainPtr = label;
	while( *label )
	{
		if( DomainNameEqual( label, me->domain ) ||
			( strnicmp_prefix( &label[ 1 ], label[ 0 ], kLabelPrefix_SRV ) == 0 ) ) break;
		label = DomainNameGetNextLabel( label );
	}
	require_quiet( *label, exit );
	
	domainLen = (size_t)( label - domainPtr );
	
	// Parse SRV labels, if any.
	
	srvCount = 0;
	while( strnicmp_prefix( &label[ 1 ], label[ 0 ], kLabelPrefix_SRV ) == 0 )
	{
		const uint8_t * const	nextLabel	= DomainNameGetNextLabel( label );
		const char *			ptr			= (const char *) &label[ 1 + sizeof_string( kLabelPrefix_SRV ) ];
		const char * const		end			= (const char *) nextLabel;
		const uint8_t *			target;
		unsigned int			priority, weight, port;
		
		err = DecimalTextToUInt32( ptr, end, &arg, &ptr );
		require_quiet( !err && ( arg <= UINT16_MAX ), exit );
		priority = (unsigned int) arg;
		
		require_quiet( ( ptr < end ) && ( *ptr == '-' ), exit );
		++ptr;
		
		err = DecimalTextToUInt32( ptr, end, &arg, &ptr );
		require_quiet( !err && ( arg <= UINT16_MAX ), exit );
		weight = (unsigned int) arg;
		
		require_quiet( ( ptr < end ) && ( *ptr == '-' ), exit );
		++ptr;
		
		err = DecimalTextToUInt32( ptr, end, &arg, &ptr );
		require_quiet( !err && ( arg <= UINT16_MAX ), exit );
		port = (unsigned int) arg;
		
		require_quiet( ptr == end, exit );
		
		target = nextLabel;
		for( label = nextLabel; *label; label = DomainNameGetNextLabel( label ) )
		{
			if( DomainNameEqual( label, me->domain ) ||
				( strnicmp_prefix( &label[ 1 ], label[ 0 ], kLabelPrefix_SRV ) == 0 ) ) break;
		}
		require_quiet( *label, exit );
		
		if( inSRVArray )
		{
			inSRVArray[ srvCount ].priority		= (uint16_t) priority;
			inSRVArray[ srvCount ].weight		= (uint16_t) weight;
			inSRVArray[ srvCount ].port			= (uint16_t) port;
			inSRVArray[ srvCount ].targetPtr	= target;
			inSRVArray[ srvCount ].targetLen	= (uint16_t)( label - target );
		}
		++srvCount;
	}
	require_quiet( DomainNameEqual( label, me->domain ), exit );
	isNameValid = true;
	
	if( outDomainPtr )	*outDomainPtr	= domainPtr;
	if( outDomainLen )	*outDomainLen	= domainLen;
	if( outSRVCount )	*outSRVCount	= srvCount;
	
exit:
	return( isNameValid ? true : false );
}

//===========================================================================================================================
//	_DNSServerTCPReadHandler
//===========================================================================================================================

typedef struct
{
	DNSServerRef			server;			// Reference to DNS server object.
	sockaddr_ip				clientAddr;		// Client's address.
	dispatch_source_t		readSource;		// Dispatch read source for client socket.
	dispatch_source_t		writeSource;	// Dispatch write source for client socket.
	size_t					offset;			// Offset into receive buffer.
	void *					msgPtr;			// Pointer to dynamically allocated message buffer.
	size_t					msgLen;			// Length of message buffer.
	Boolean					readSuspended;	// True if the read source is currently suspended.
	Boolean					writeSuspended;	// True if the write source is currently suspended.
	Boolean					receivedLength;	// True if receiving DNS message as opposed to the message length.
	uint8_t					lenBuf[ 2 ];	// Buffer for two-octet message length field.
	iovec_t					iov[ 2 ];		// IO vector for writing response message.
	iovec_t *				iovPtr;			// Vector pointer for SocketWriteData().
	int						iovCount;		// Vector count for SocketWriteData().
	
}	TCPConnectionContext;

static void	TCPConnectionStop( TCPConnectionContext *inContext );
static void	TCPConnectionContextFree( TCPConnectionContext *inContext );
static void	TCPConnectionReadHandler( void *inContext );
static void	TCPConnectionWriteHandler( void *inContext );

#define	TCPConnectionForget( X )		ForgetCustomEx( X, TCPConnectionStop, TCPConnectionContextFree )

static void	_DNSServerTCPReadHandler( void *inContext )
{
	OSStatus					err;
	SocketContext * const		sockCtx		= (SocketContext *) inContext;
	DNSServerRef const			me			= (DNSServerRef) sockCtx->userContext;
	TCPConnectionContext *		connection;
	socklen_t					clientAddrLen;
	SocketRef					newSock		= kInvalidSocketRef;
	SocketContext *				newSockCtx	= NULL;
	
	connection = (TCPConnectionContext *) calloc( 1, sizeof( *connection ) );
	require_action( connection, exit, err = kNoMemoryErr );
	
	CFRetain( me );
	connection->server = me;
	
	clientAddrLen = (socklen_t) sizeof( connection->clientAddr );
	newSock = accept( sockCtx->sock, &connection->clientAddr.sa, &clientAddrLen );
	err = map_socket_creation_errno( newSock );
	require_noerr( err, exit );
	
	err = SocketContextCreate( newSock, connection, &newSockCtx );
	require_noerr( err, exit );
	newSock = kInvalidSocketRef;
	
	err = DispatchReadSourceCreate( newSockCtx->sock, me->queue, TCPConnectionReadHandler, SocketContextCancelHandler,
		newSockCtx, &connection->readSource );
	require_noerr( err, exit );
	SocketContextRetain( newSockCtx );
	dispatch_resume( connection->readSource );
	
	err = DispatchWriteSourceCreate( newSockCtx->sock, me->queue, TCPConnectionWriteHandler, SocketContextCancelHandler,
		newSockCtx, &connection->writeSource );
	require_noerr( err, exit );
	SocketContextRetain( newSockCtx );
	connection->writeSuspended = true;
	connection = NULL;
	
exit:
	ForgetSocket( &newSock );
	SocketContextRelease( newSockCtx );
	TCPConnectionForget( &connection );
}

//===========================================================================================================================
//	TCPConnectionStop
//===========================================================================================================================

static void	TCPConnectionStop( TCPConnectionContext *inContext )
{
	dispatch_source_forget_ex( &inContext->readSource, &inContext->readSuspended );
	dispatch_source_forget_ex( &inContext->writeSource, &inContext->writeSuspended );
}

//===========================================================================================================================
//	TCPConnectionContextFree
//===========================================================================================================================

static void	TCPConnectionContextFree( TCPConnectionContext *inContext )
{
	check( !inContext->readSource );
	check( !inContext->writeSource );
	ForgetCF( &inContext->server );
	ForgetMem( &inContext->msgPtr );
	free( inContext );
}

//===========================================================================================================================
//	TCPConnectionReadHandler
//===========================================================================================================================

static void	TCPConnectionReadHandler( void *inContext )
{
	OSStatus					err;
	SocketContext * const		sockCtx		= (SocketContext *) inContext;
	TCPConnectionContext *		connection	= (TCPConnectionContext *) sockCtx->userContext;
	struct timeval				now;
	uint8_t *					responsePtr	= NULL;	// malloc'd
	size_t						responseLen;
	
	// Receive message length.
	
	if( !connection->receivedLength )
	{
		err = SocketReadData( sockCtx->sock, connection->lenBuf, sizeof( connection->lenBuf ), &connection->offset );
		if( err == EWOULDBLOCK ) goto exit;
		require_noerr( err, exit );
		
		connection->offset = 0;
		connection->msgLen = ReadBig16( connection->lenBuf );
		connection->msgPtr = malloc( connection->msgLen );
		require_action( connection->msgPtr, exit, err = kNoMemoryErr );
		connection->receivedLength = true;
	}
	
	// Receive message.
	
	err = SocketReadData( sockCtx->sock, connection->msgPtr, connection->msgLen, &connection->offset );
	if( err == EWOULDBLOCK ) goto exit;
	require_noerr( err, exit );
	
	gettimeofday( &now, NULL );
	dispatch_suspend( connection->readSource );
	connection->readSuspended = true;
	
	ds_ulog( kLogLevelInfo, "TCP server received %zu bytes from %##a at %{du:time}.\n",
		connection->msgLen, &connection->clientAddr, &now );
	
	if( connection->msgLen < kDNSHeaderLength )
	{
		ds_ulog( kLogLevelInfo, "TCP DNS message is too small (%zu < %d).\n", connection->msgLen, kDNSHeaderLength );
		goto exit;
	}
	
	ds_ulog( kLogLevelInfo, "TCP received message:\n\n%1{du:dnsmsg}", connection->msgPtr, connection->msgLen );
	
	// Create response.
	
	err = _DNSServerAnswerQueryForTCP( connection->server, connection->msgPtr, connection->msgLen, &responsePtr,
		&responseLen );
	require_noerr_quiet( err, exit );
	
	// Send response.
	
	ds_ulog( kLogLevelInfo, "TCP sending %zu byte response:\n\n%1{du:dnsmsg}", responseLen, responsePtr, responseLen );
	
	free( connection->msgPtr );
	connection->msgPtr = responsePtr;
	connection->msgLen = responseLen;
	responsePtr = NULL;
	
	check( connection->msgLen <= UINT16_MAX );
	WriteBig16( connection->lenBuf, connection->msgLen );
	connection->iov[ 0 ].iov_base	= connection->lenBuf;
	connection->iov[ 0 ].iov_len	= sizeof( connection->lenBuf );
	connection->iov[ 1 ].iov_base	= connection->msgPtr;
	connection->iov[ 1 ].iov_len	= connection->msgLen;
	
	connection->iovPtr		= connection->iov;
	connection->iovCount	= 2;
	
	check( connection->writeSuspended );
	dispatch_resume( connection->writeSource );
	connection->writeSuspended = false;
	
exit:
	FreeNullSafe( responsePtr );
	if( err && ( err != EWOULDBLOCK ) ) TCPConnectionForget( &connection );
}

//===========================================================================================================================
//	TCPConnectionWriteHandler
//===========================================================================================================================

static void	TCPConnectionWriteHandler( void *inContext )
{
	OSStatus					err;
	SocketContext * const		sockCtx		= (SocketContext *) inContext;
	TCPConnectionContext *		connection	= (TCPConnectionContext *) sockCtx->userContext;
	
	err = SocketWriteData( sockCtx->sock, &connection->iovPtr, &connection->iovCount );
	if( err == EWOULDBLOCK ) goto exit;
	check_noerr( err );
	
	TCPConnectionForget( &connection );
	
exit:
	return;
}

//===========================================================================================================================
//	MDNSReplierCmd
//===========================================================================================================================

typedef struct
{
	uint8_t *				hostname;			// Used as the base name for hostnames and service names.
	uint8_t *				serviceLabel;		// Label containing the base service name.
	unsigned int			maxInstanceCount;	// Maximum number of service instances and hostnames.
	uint64_t *				bitmaps;			// Array of 64-bit bitmaps for keeping track of needed responses.
	size_t					bitmapCount;		// Number of 64-bit bitmaps.
	dispatch_source_t		readSourceV4;		// Read dispatch source for IPv4 socket.
	dispatch_source_t		readSourceV6;		// Read dispatch source for IPv6 socket.
	uint32_t				ifIndex;			// Index of the interface to run on.
	unsigned int			recordCountA;		// Number of A records per hostname.
	unsigned int			recordCountAAAA;	// Number of AAAA records per hostname.
	unsigned int			maxDropCount;		// If > 0, the drop rates apply to only the first <maxDropCount> responses.
	double					ucastDropRate;		// Probability of dropping a unicast response.
	double					mcastDropRate;		// Probability of dropping a multicast query or response.
	uint8_t *				dropCounters;		// If maxDropCount > 0, array of <maxInstanceCount> response drop counters.
	Boolean					noAdditionals;		// True if responses are to not include additional records.
	Boolean					useIPv4;			// True if the replier is to use IPv4.
	Boolean					useIPv6;			// True if the replier is to use IPv6.
	uint8_t					msgBuf[ kMDNSMessageSizeMax ];	// Buffer for received mDNS message.
#if( TARGET_OS_DARWIN )
	dispatch_source_t		processMonitor;		// Process monitor source for process being followed, if any.
	pid_t					followPID;			// PID of process being followed, if any. (If it exits, we exit).
#endif
	
}	MDNSReplierContext;

typedef struct MRResourceRecord		MRResourceRecord;
struct MRResourceRecord
{
	MRResourceRecord *		next;		// Next item in list.
	uint8_t *				name;		// Resource record name.
	uint16_t				type;		// Resource record type.
	uint16_t				class;		// Resource record class.
	uint32_t				ttl;		// Resource record TTL.
	uint16_t				rdlength;	// Resource record data length.
	uint8_t *				rdata;		// Resource record data.
	const uint8_t *			target;		// For SRV records, pointer to target in RDATA.
};

typedef struct MRNameOffsetItem		MRNameOffsetItem;
struct MRNameOffsetItem
{
	MRNameOffsetItem *	next;		// Next item in list.
	uint16_t			offset;		// Offset of domain name in response message.
	uint8_t				name[ 1 ];	// Variable-length array for domain name.
};

#if( TARGET_OS_DARWIN )
static void		_MDNSReplierFollowedProcessHandler( void *inContext );
#endif
static void		_MDNSReplierReadHandler( void *inContext );
static OSStatus
	_MDNSReplierAnswerQuery(
		MDNSReplierContext *	inContext,
		const uint8_t *			inQueryPtr,
		size_t					inQueryLen,
		sockaddr_ip *			inSender,
		SocketRef				inSock,
		unsigned int			inIndex );
static OSStatus
	_MDNSReplierAnswerListAdd(
		MDNSReplierContext *	inContext,
		MRResourceRecord **		inAnswerList,
		unsigned int			inIndex,
		const uint8_t *			inName,
		unsigned int			inType,
		unsigned int			inClass );
static void
	_MDNSReplierAnswerListRemovePTR(
		MRResourceRecord **	inAnswerListPtr,
		const uint8_t *		inName,
		const uint8_t *		inRData );
static OSStatus
	_MDNSReplierSendOrDropResponse(
		MDNSReplierContext *	inContext,
		MRResourceRecord *		inAnswerList,
		sockaddr_ip *			inQuerier,
		SocketRef				inSock,
		unsigned int			inIndex,
		Boolean					inUnicast );
static OSStatus
	_MDNSReplierCreateResponse(
		MDNSReplierContext *	inContext,
		MRResourceRecord *		inAnswerList,
		unsigned int			inIndex,
		uint8_t **				outResponsePtr,
		size_t *				outResponseLen );
static OSStatus
	_MDNSReplierAppendNameToResponse(
		DataBuffer *		inResponse,
		const uint8_t *		inName,
		MRNameOffsetItem **	inNameOffsetListPtr );
static Boolean
	_MDNSReplierServiceTypeMatch(
		const MDNSReplierContext *	inContext,
		const uint8_t *				inName,
		unsigned int *				outTXTSize,
		unsigned int *				outCount );
static Boolean
	_MDNSReplierServiceInstanceNameMatch(
		const MDNSReplierContext *	inContext,
		const uint8_t *				inName,
		unsigned int *				outIndex,
		unsigned int *				outTXTSize,
		unsigned int *				outCount );
static Boolean	_MDNSReplierAboutRecordNameMatch( const MDNSReplierContext *inContext, const uint8_t *inName );
static Boolean
	_MDNSReplierHostnameMatch(
		const MDNSReplierContext *	inContext,
		const uint8_t *				inName,
		unsigned int *				outIndex );
static OSStatus	_MDNSReplierCreateTXTRecord( const uint8_t *inRecordName, size_t inSize, uint8_t **outTXT );
static OSStatus
	_MRResourceRecordCreate(
		uint8_t *			inName,
		uint16_t			inType,
		uint16_t			inClass,
		uint32_t			inTTL,
		uint16_t			inRDLength,
		uint8_t *			inRData,
		MRResourceRecord **	outRecord );
static void		_MRResourceRecordFree( MRResourceRecord *inRecord );
static void		_MRResourceRecordFreeList( MRResourceRecord *inList );
static OSStatus	_MRNameOffsetItemCreate( const uint8_t *inName, uint16_t inOffset, MRNameOffsetItem **outItem );
static void		_MRNameOffsetItemFree( MRNameOffsetItem *inItem );
static void		_MRNameOffsetItemFreeList( MRNameOffsetItem *inList );

ulog_define_ex( kDNSSDUtilIdentifier, MDNSReplier, kLogLevelInfo, kLogFlags_None, "MDNSReplier", NULL );
#define mr_ulog( LEVEL, ... )		ulog( &log_category_from_name( MDNSReplier ), (LEVEL), __VA_ARGS__ )

static void	MDNSReplierCmd( void )
{
	OSStatus					err;
	MDNSReplierContext *		context;
	SocketRef					sockV4	= kInvalidSocketRef;
	SocketRef					sockV6	= kInvalidSocketRef;
	const char *				ifname;
	size_t						len;
	uint8_t						name[ 1 + kDomainLabelLengthMax + 1 ];
	char						ifnameBuf[ IF_NAMESIZE + 1 ];
	
	err = CheckIntegerArgument( gMDNSReplier_MaxInstanceCount, "max instance count", 1, UINT16_MAX );
	require_noerr_quiet( err, exit );
	
	err = CheckIntegerArgument( gMDNSReplier_RecordCountA, "A record count", 0, 255 );
	require_noerr_quiet( err, exit );
	
	err = CheckIntegerArgument( gMDNSReplier_RecordCountAAAA, "AAAA record count", 0, 255 );
	require_noerr_quiet( err, exit );
	
	err = CheckDoubleArgument( gMDNSReplier_UnicastDropRate, "unicast drop rate", 0.0, 1.0 );
	require_noerr_quiet( err, exit );
	
	err = CheckDoubleArgument( gMDNSReplier_MulticastDropRate, "multicast drop rate", 0.0, 1.0 );
	require_noerr_quiet( err, exit );
	
	err = CheckIntegerArgument( gMDNSReplier_MaxDropCount, "drop count", 0, 255 );
	require_noerr_quiet( err, exit );
	
	if( gMDNSReplier_Foreground )
	{
		LogControl( "MDNSReplier:output=file;stdout,MDNSReplier:flags=time;prefix" );
	}
	
	context = (MDNSReplierContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	context->maxInstanceCount	= (unsigned int) gMDNSReplier_MaxInstanceCount;
	context->recordCountA		= (unsigned int) gMDNSReplier_RecordCountA;
	context->recordCountAAAA	= (unsigned int) gMDNSReplier_RecordCountAAAA;
	context->maxDropCount		= (unsigned int) gMDNSReplier_MaxDropCount;
	context->ucastDropRate		= gMDNSReplier_UnicastDropRate;
	context->mcastDropRate		= gMDNSReplier_MulticastDropRate;
	context->noAdditionals		= gMDNSReplier_NoAdditionals ? true : false;
	context->useIPv4			= ( gMDNSReplier_UseIPv4 || !gMDNSReplier_UseIPv6 ) ? true : false;
	context->useIPv6			= ( gMDNSReplier_UseIPv6 || !gMDNSReplier_UseIPv4 ) ? true : false;
	context->bitmapCount		= ( context->maxInstanceCount + 63 ) / 64;
	
#if( TARGET_OS_DARWIN )
	if( gMDNSReplier_FollowPID )
	{
		context->followPID = _StringToPID( gMDNSReplier_FollowPID, &err );
		if( err || ( context->followPID < 0 ) )
		{
			FPrintF( stderr, "error: Invalid follow PID: %s\n", gMDNSReplier_FollowPID );
			goto exit;
		}
		
		err = DispatchProcessMonitorCreate( context->followPID, DISPATCH_PROC_EXIT, dispatch_get_main_queue(),
			_MDNSReplierFollowedProcessHandler, NULL, context, &context->processMonitor );
		require_noerr( err, exit );
		dispatch_resume( context->processMonitor );
	}
	else
	{
		context->followPID = -1;
	}
#endif
	
	if( context->maxDropCount > 0 )
	{
		context->dropCounters = (uint8_t *) calloc( context->maxInstanceCount, sizeof( *context->dropCounters ) );
		require_action( context->dropCounters, exit, err = kNoMemoryErr );
	}
	
	context->bitmaps = (uint64_t *) calloc( context->bitmapCount, sizeof( *context->bitmaps ) );
	require_action( context->bitmaps, exit, err = kNoMemoryErr );
	
	// Create the base hostname label.
	
	len = strlen( gMDNSReplier_Hostname );
	if( context->maxInstanceCount > 1 )
	{
		unsigned int		maxInstanceCount, digitCount;
		
		// When there's more than one instance, extra bytes are needed to append " (<instance index>)" or
		// "-<instance index>" to the base hostname.
		
		maxInstanceCount = context->maxInstanceCount;
		for( digitCount = 0; maxInstanceCount > 0; ++digitCount ) maxInstanceCount /= 10;
		len += ( 3 + digitCount );
	}
	
	if( len <= kDomainLabelLengthMax )
	{
		uint8_t *		dst = &name[ 1 ];
		uint8_t *		lim = &name[ countof( name ) ];
		
		SNPrintF_Add( (char **) &dst, (char *) lim, "%s", gMDNSReplier_Hostname );
		name[ 0 ] = (uint8_t)( dst - &name[ 1 ] );
		
		err = DomainNameDupLower( name, &context->hostname, NULL );
		require_noerr( err, exit );
	}
	else
	{
		FPrintF( stderr, "error: Base name \"%s\" is too long for max instance count of %u.\n",
			gMDNSReplier_Hostname, context->maxInstanceCount );
		goto exit;
	}
	
	// Create the service label.
	
	len = strlen( gMDNSReplier_ServiceTypeTag ) + 3;	// We need three extra bytes for the service type prefix "_t-".
	if( len <= kDomainLabelLengthMax )
	{
		uint8_t *		dst = &name[ 1 ];
		uint8_t *		lim = &name[ countof( name ) ];
		
		SNPrintF_Add( (char **) &dst, (char *) lim, "_t-%s", gMDNSReplier_ServiceTypeTag );
		name[ 0 ] = (uint8_t)( dst - &name[ 1 ] );
		
		err = DomainNameDupLower( name, &context->serviceLabel, NULL );
		require_noerr( err, exit );
	}
	else
	{
		FPrintF( stderr, "error: Service type tag is too long.\n" );
		goto exit;
	}
	
	err = InterfaceIndexFromArgString( gInterface, &context->ifIndex );
	require_noerr_quiet( err, exit );
	
	ifname = if_indextoname( context->ifIndex, ifnameBuf );
	require_action( ifname, exit, err = kNameErr );
	
	// Set up IPv4 socket.
	
	if( context->useIPv4 )
	{
		err = CreateMulticastSocket( GetMDNSMulticastAddrV4(), kMDNSPort, ifname, context->ifIndex, true, NULL, &sockV4 );
		require_noerr( err, exit );
	}
	
	// Set up IPv6 socket.
	
	if( context->useIPv6 )
	{
		err = CreateMulticastSocket( GetMDNSMulticastAddrV6(), kMDNSPort, ifname, context->ifIndex, true, NULL, &sockV6 );
		require_noerr( err, exit );
	}
	
	// Create dispatch read sources for socket(s).
	
	if( IsValidSocket( sockV4 ) )
	{
		SocketContext *		sockCtx;
		
		err = SocketContextCreate( sockV4, context, &sockCtx );
		require_noerr( err, exit );
		sockV4 = kInvalidSocketRef;
		
		err = DispatchReadSourceCreate( sockCtx->sock, NULL, _MDNSReplierReadHandler, SocketContextCancelHandler, sockCtx,
			&context->readSourceV4 );
		if( err ) ForgetSocketContext( &sockCtx );
		require_noerr( err, exit );
		
		dispatch_resume( context->readSourceV4 );
	}
	
	if( IsValidSocket( sockV6 ) )
	{
		SocketContext *		sockCtx;
		
		err = SocketContextCreate( sockV6, context, &sockCtx );
		require_noerr( err, exit );
		sockV6 = kInvalidSocketRef;
		
		err = DispatchReadSourceCreate( sockCtx->sock, NULL, _MDNSReplierReadHandler, SocketContextCancelHandler, sockCtx,
			&context->readSourceV6 );
		if( err ) ForgetSocketContext( &sockCtx );
		require_noerr( err, exit );
		
		dispatch_resume( context->readSourceV6 );
	}
	
	dispatch_main();
	
exit:
	ForgetSocket( &sockV4 );
	ForgetSocket( &sockV6 );
	exit( 1 );
}

#if( TARGET_OS_DARWIN )
//===========================================================================================================================
//	_MDNSReplierFollowedProcessHandler
//===========================================================================================================================

static void	_MDNSReplierFollowedProcessHandler( void *inContext )
{
	MDNSReplierContext * const		context = (MDNSReplierContext *) inContext;
	
	if( dispatch_source_get_data( context->processMonitor ) & DISPATCH_PROC_EXIT )
	{
		mr_ulog( kLogLevelNotice, "Exiting: followed process (%lld) exited.\n", (int64_t) context->followPID );
		exit( 0 );
	}
}
#endif

//===========================================================================================================================
//	_MDNSReplierReadHandler
//===========================================================================================================================

#define ShouldDrop( P )		( ( (P) > 0.0 ) && ( ( (P) >= 1.0 ) || RandomlyTrue( P ) ) )

static void	_MDNSReplierReadHandler( void *inContext )
{
	OSStatus						err;
	SocketContext * const			sockCtx = (SocketContext *) inContext;
	MDNSReplierContext * const		context = (MDNSReplierContext *) sockCtx->userContext;
	size_t							msgLen;
	sockaddr_ip						sender;
	const DNSHeader *				hdr;
	unsigned int					flags, questionCount, i, j;
	const uint8_t *					ptr;
	int								drop, isMetaQuery;
	
	err = SocketRecvFrom( sockCtx->sock, context->msgBuf, sizeof( context->msgBuf ), &msgLen, &sender, sizeof( sender ),
		NULL, NULL, NULL, NULL );
	require_noerr( err, exit );
	
	if( msgLen < kDNSHeaderLength )
	{
		mr_ulog( kLogLevelInfo, "Message is too small (%zu < %d).\n", msgLen, kDNSHeaderLength );
		goto exit;
	}
	
	// Perform header field checks.
	// The message ID and most flag bits are silently ignored (see <https://tools.ietf.org/html/rfc6762#section-18>).
	
	hdr = (DNSHeader *) context->msgBuf;
	flags = DNSHeaderGetFlags( hdr );
	require_quiet( ( flags & kDNSHeaderFlag_Response ) == 0, exit );		// Reject responses.
	require_quiet( DNSFlagsGetOpCode( flags ) == kDNSOpCode_Query, exit );	// Reject opcodes other than standard query.
	require_quiet( DNSFlagsGetRCode( flags )  == kDNSRCode_NoError, exit );	// Reject non-zero rcodes.
	
	drop = ( !context->maxDropCount && ShouldDrop( context->mcastDropRate ) ) ? true : false;
	
	mr_ulog( kLogLevelInfo, "Received %zu byte message from %##a%?s:\n\n%#1{du:dnsmsg}",
		msgLen, &sender, drop, " (dropping)", context->msgBuf, msgLen );
	
	// Based on the QNAMEs in the query message, determine from which sets of records we may possibly need answers.
	
	questionCount = DNSHeaderGetQuestionCount( hdr );
	require_quiet( questionCount > 0, exit );
	
	memset( context->bitmaps, 0, context->bitmapCount * sizeof_element( context->bitmaps ) );
	
	isMetaQuery = false;
	ptr = (const uint8_t *) &hdr[ 1 ];
	for( i = 0; i < questionCount; ++i )
	{
		unsigned int		count, index;
		uint16_t			qtype, qclass;
		uint8_t				qname[ kDomainNameLengthMax ];
		
		err = DNSMessageExtractQuestion( context->msgBuf, msgLen, ptr, qname, &qtype, &qclass, &ptr );
		require_noerr_quiet( err, exit );
		
		if( ( qclass & ~kQClassUnicastResponseBit ) != kDNSServiceClass_IN ) continue;
		
		if( _MDNSReplierHostnameMatch( context, qname, &index ) ||
			_MDNSReplierServiceInstanceNameMatch( context, qname, &index, NULL, NULL ) )
		{
			if( ( index >= 1 ) && ( index <= context->maxInstanceCount ) )
			{
				context->bitmaps[ ( index - 1 ) / 64 ] |= ( UINT64_C( 1 ) << ( ( index - 1 ) % 64 ) );
			}
		}
		else if( _MDNSReplierServiceTypeMatch( context, qname, NULL, &count ) )
		{
			if( ( count >= 1 ) && ( count <= context->maxInstanceCount ) )
			{
				for( j = 0; j < (unsigned int) context->bitmapCount; ++j )
				{
					if( count < 64 )
					{
						context->bitmaps[ j ] |= ( ( UINT64_C( 1 ) << count ) - 1 );
						break;
					}
					else
					{
						context->bitmaps[ j ] = ~UINT64_C( 0 );
						count -= 64;
					}
				}
			}
		}
		else if( _MDNSReplierAboutRecordNameMatch( context, qname ) )
		{
			isMetaQuery = true;
		}
	}
	
	// Attempt to answer the query message using selected record sets.
	
	if( isMetaQuery )
	{
		err = _MDNSReplierAnswerQuery( context, context->msgBuf, msgLen, &sender, sockCtx->sock, 0 );
		check_noerr( err );
	}
	if( drop ) goto exit;
	
	for( i = 0; i < context->bitmapCount; ++i )
	{
		for( j = 0; ( context->bitmaps[ i ] != 0 ) && ( j < 64 ); ++j )
		{
			const uint64_t		bitmask = UINT64_C( 1 ) << j;
			
			if( context->bitmaps[ i ] & bitmask )
			{
				context->bitmaps[ i ] &= ~bitmask;
				
				err = _MDNSReplierAnswerQuery( context, context->msgBuf, msgLen, &sender, sockCtx->sock,
					( i * 64 ) + j + 1 );
				check_noerr( err );
			}
		}
	}
	
exit:
	return;
}

//===========================================================================================================================
//	_MDNSReplierAnswerQuery
//===========================================================================================================================

static OSStatus
	_MDNSReplierAnswerQuery(
		MDNSReplierContext *	inContext,
		const uint8_t *			inQueryPtr,
		size_t					inQueryLen,
		sockaddr_ip *			inSender,
		SocketRef				inSock,
		unsigned int			inIndex )
{
	OSStatus				err;
	const DNSHeader *		hdr;
	const uint8_t *			ptr;
	unsigned int			questionCount, answerCount, i;
	MRResourceRecord *		ucastAnswerList = NULL;
	MRResourceRecord *		mcastAnswerList = NULL;
	
	require_action( inIndex <= inContext->maxInstanceCount, exit, err = kRangeErr );
	
	// Get answers for questions.
	
	check( inQueryLen >= kDNSHeaderLength );
	hdr = (const DNSHeader *) inQueryPtr;
	questionCount = DNSHeaderGetQuestionCount( hdr );
	
	ptr = (const uint8_t *) &hdr[ 1 ];
	for( i = 0; i < questionCount; ++i )
	{
		MRResourceRecord **		answerListPtr;
		uint16_t				qtype, qclass;
		uint8_t					qname[ kDomainNameLengthMax ];
		
		err = DNSMessageExtractQuestion( inQueryPtr, inQueryLen, ptr, qname, &qtype, &qclass, &ptr );
		require_noerr_quiet( err, exit );
		
		if( qclass & kQClassUnicastResponseBit )
		{
			qclass &= ~kQClassUnicastResponseBit;
			answerListPtr = &ucastAnswerList;
		}
		else
		{
			answerListPtr = &mcastAnswerList;
		}
		
		err = _MDNSReplierAnswerListAdd( inContext, answerListPtr, inIndex, qname, qtype, qclass );
		require_noerr( err, exit );
	}
	require_action_quiet( mcastAnswerList || ucastAnswerList, exit, err = kNoErr );
	
	// Suppress known answers.
	// Records in the Answer section of the query message are known answers, so remove them from the answer lists.
	// See <https://tools.ietf.org/html/rfc6762#section-7.1>.
	
	answerCount = DNSHeaderGetAnswerCount( hdr );
	for( i = 0; i < answerCount; ++i )
	{
		const uint8_t *		rdataPtr;
		const uint8_t *		recordPtr;
		uint16_t			type, class;
		uint8_t				name[ kDomainNameLengthMax ];
		uint8_t				instance[ kDomainNameLengthMax ];
		
		recordPtr = ptr;
		err = DNSMessageExtractRecord( inQueryPtr, inQueryLen, ptr, NULL, &type, &class, NULL, NULL, NULL, &ptr );
		require_noerr_quiet( err, exit );
		
		if( ( type != kDNSServiceType_PTR ) || ( class != kDNSServiceClass_IN ) ) continue;
		
		err = DNSMessageExtractRecord( inQueryPtr, inQueryLen, recordPtr, name, NULL, NULL, NULL, &rdataPtr, NULL, NULL );
		require_noerr( err, exit );
		
		err = DNSMessageExtractDomainName( inQueryPtr, inQueryLen, rdataPtr, instance, NULL );
		require_noerr_quiet( err, exit );
		
		if( ucastAnswerList ) _MDNSReplierAnswerListRemovePTR( &ucastAnswerList, name, instance );
		if( mcastAnswerList ) _MDNSReplierAnswerListRemovePTR( &mcastAnswerList, name, instance );
	}
	require_action_quiet( mcastAnswerList || ucastAnswerList, exit, err = kNoErr );
	
	// Send or drop responses.
	
	if( ucastAnswerList )
	{
		err = _MDNSReplierSendOrDropResponse( inContext, ucastAnswerList, inSender, inSock, inIndex, true );
		require_noerr( err, exit );
	}
	
	if( mcastAnswerList )
	{
		err = _MDNSReplierSendOrDropResponse( inContext, mcastAnswerList, inSender, inSock, inIndex, false );
		require_noerr( err, exit );
	}
	err = kNoErr;
	
exit:
	_MRResourceRecordFreeList( ucastAnswerList );
	_MRResourceRecordFreeList( mcastAnswerList );
	return( err );
}

//===========================================================================================================================
//	_MDNSReplierAnswerListAdd
//===========================================================================================================================

static OSStatus
	_MDNSReplierAnswerListAdd(
		MDNSReplierContext *	inContext,
		MRResourceRecord **		inAnswerList,
		unsigned int			inIndex,
		const uint8_t *			inName,
		unsigned int			inType,
		unsigned int			inClass )
{
	OSStatus					err;
	uint8_t *					recordName	= NULL;
	uint8_t *					rdataPtr	= NULL;
	size_t						rdataLen;
	MRResourceRecord *			answer;
	MRResourceRecord **			answerPtr;
	const uint8_t * const		hostname	= inContext->hostname;
	unsigned int				i;
	uint32_t					index;
	unsigned int				count, txtSize;
	
	require_action( inIndex <= inContext->maxInstanceCount, exit, err = kRangeErr );
	require_action_quiet( inClass == kDNSServiceClass_IN, exit, err = kNoErr );
	
	for( answerPtr = inAnswerList; ( answer = *answerPtr ) != NULL; answerPtr = &answer->next )
	{
		if( ( answer->type == inType ) && DomainNameEqual( answer->name, inName ) )
		{
			err = kNoErr;
			goto exit;
		}
	}
	
	// Index 0 is reserved for answering queries about the mdnsreplier, while all other index values up to the maximum
	// instance count are for answering queries about service instances.
	
	if( inIndex == 0 )
	{
		if( _MDNSReplierAboutRecordNameMatch( inContext, inName ) )
		{
			int		listHasTXT = false;
			
			if( inType == kDNSServiceType_ANY )
			{
				for( answer = *inAnswerList; answer; answer = answer->next )
				{
					if( ( answer->type == kDNSServiceType_TXT ) && DomainNameEqual( answer->name, inName ) )
					{
						listHasTXT = true;
						break;
					}
				}
			}
			
			if( ( inType == kDNSServiceType_TXT ) || ( ( inType == kDNSServiceType_ANY ) && !listHasTXT ) )
			{
				err = DomainNameDupLower( inName, &recordName, NULL );
				require_noerr( err, exit );
				
				err = CreateTXTRecordDataFromString( "ready=yes", ',', &rdataPtr, &rdataLen );
				require_noerr( err, exit );
				
				err = _MRResourceRecordCreate( recordName, kDNSServiceType_TXT, kDNSServiceClass_IN, kMDNSRecordTTL_Other,
					(uint16_t) rdataLen, rdataPtr, &answer );
				require_noerr( err, exit );
				recordName	= NULL;
				rdataPtr	= NULL;
				
				*answerPtr = answer;
			}
			else if( inType == kDNSServiceType_NSEC )
			{
				err = DomainNameDupLower( inName, &recordName, NULL );
				require_noerr( err, exit );
				
				err = CreateNSECRecordData( recordName, &rdataPtr, &rdataLen, 1, kDNSServiceType_TXT );
				require_noerr( err, exit );
				
				err = _MRResourceRecordCreate( recordName, kDNSServiceType_NSEC, kDNSServiceClass_IN, kMDNSRecordTTL_Host,
					(uint16_t) rdataLen, rdataPtr, &answer );
				require_noerr( err, exit );
				recordName	= NULL;
				rdataPtr	= NULL;
				
				*answerPtr = answer;
			}
		}
	}
	else if( _MDNSReplierHostnameMatch( inContext, inName, &index ) && ( index == inIndex ) )
	{
		int		listHasA	= false;
		int		listHasAAAA	= false;
		
		if( inType == kDNSServiceType_ANY )
		{
			for( answer = *inAnswerList; answer; answer = answer->next )
			{
				if( answer->type == kDNSServiceType_A )
				{
					if( !listHasA && DomainNameEqual( answer->name, inName ) ) listHasA = true;
				}
				else if( answer->type == kDNSServiceType_AAAA )
				{
					if( !listHasAAAA && DomainNameEqual( answer->name, inName ) ) listHasAAAA = true;
				}
				if( listHasA && listHasAAAA ) break;
			}
		}
		
		if( ( inType == kDNSServiceType_A ) || ( ( inType == kDNSServiceType_ANY ) && !listHasA ) )
		{
			for( i = 1; i <= inContext->recordCountA; ++i )
			{
				err = DomainNameDupLower( inName, &recordName, NULL );
				require_noerr( err, exit );
				
				rdataLen = 4;
				rdataPtr = (uint8_t *) malloc( rdataLen );
				require_action( rdataPtr, exit, err = kNoMemoryErr );
				
				rdataPtr[ 0 ] = 0;
				WriteBig16( &rdataPtr[ 1 ], inIndex );
				rdataPtr[ 3 ] = (uint8_t) i;
				
				err = _MRResourceRecordCreate( recordName, kDNSServiceType_A, kDNSServiceClass_IN, kMDNSRecordTTL_Host,
					(uint16_t) rdataLen, rdataPtr, &answer );
				require_noerr( err, exit );
				recordName	= NULL;
				rdataPtr	= NULL;
				
				*answerPtr = answer;
				 answerPtr = &answer->next;
			}
		}
		
		if( ( inType == kDNSServiceType_AAAA ) || ( ( inType == kDNSServiceType_ANY ) && !listHasAAAA ) )
		{
			for( i = 1; i <= inContext->recordCountAAAA; ++i )
			{
				err = DomainNameDupLower( inName, &recordName, NULL );
				require_noerr( err, exit );
				
				rdataLen = 16;
				rdataPtr = (uint8_t *) _memdup( kMDNSReplierBaseAddrV6, rdataLen );
				require_action( rdataPtr, exit, err = kNoMemoryErr );
				
				WriteBig16( &rdataPtr[ 12 ], inIndex );
				rdataPtr[ 15 ] = (uint8_t) i;
				
				err = _MRResourceRecordCreate( recordName, kDNSServiceType_AAAA, kDNSServiceClass_IN, kMDNSRecordTTL_Host,
					(uint16_t) rdataLen, rdataPtr, &answer );
				require_noerr( err, exit );
				recordName	= NULL;
				rdataPtr	= NULL;
				
				*answerPtr = answer;
				 answerPtr = &answer->next;
			}
		}
		else if( inType == kDNSServiceType_NSEC )
		{
			err = DomainNameDupLower( inName, &recordName, NULL );
			require_noerr( err, exit );
			
			if( ( inContext->recordCountA > 0 ) && ( inContext->recordCountAAAA > 0 ) )
			{
				err = CreateNSECRecordData( recordName, &rdataPtr, &rdataLen, 2, kDNSServiceType_A, kDNSServiceType_AAAA );
				require_noerr( err, exit );
			}
			else if( inContext->recordCountA > 0 )
			{
				err = CreateNSECRecordData( recordName, &rdataPtr, &rdataLen, 1, kDNSServiceType_A );
				require_noerr( err, exit );
			}
			else if( inContext->recordCountAAAA > 0 )
			{
				err = CreateNSECRecordData( recordName, &rdataPtr, &rdataLen, 1, kDNSServiceType_AAAA );
				require_noerr( err, exit );
			}
			else
			{
				err = CreateNSECRecordData( recordName, &rdataPtr, &rdataLen, 0 );
				require_noerr( err, exit );
			}
			
			err = _MRResourceRecordCreate( recordName, kDNSServiceType_NSEC, kDNSServiceClass_IN, kMDNSRecordTTL_Host,
				(uint16_t) rdataLen, rdataPtr, &answer );
			require_noerr( err, exit );
			recordName	= NULL;
			rdataPtr	= NULL;
			
			*answerPtr = answer;
		}
	}
	else if( _MDNSReplierServiceTypeMatch( inContext, inName, NULL, &count ) && ( count >= inIndex ) )
	{
		int		listHasPTR = false;
		
		if( inType == kDNSServiceType_ANY )
		{
			for( answer = *inAnswerList; answer; answer = answer->next )
			{
				if( ( answer->type == kDNSServiceType_PTR ) && DomainNameEqual( answer->name, inName ) )
				{
					listHasPTR = true;
					break;
				}
			}
		}
		
		if( ( inType == kDNSServiceType_PTR ) || ( ( inType == kDNSServiceType_ANY ) && !listHasPTR ) )
		{
			size_t				recordNameLen;
			uint8_t *			ptr;
			uint8_t *			lim;
			
			err = DomainNameDupLower( inName, &recordName, &recordNameLen );
			require_noerr( err, exit );
			
			rdataLen = 1 + hostname[ 0 ] + 10 + recordNameLen;
			rdataPtr = (uint8_t *) malloc( rdataLen );
			require_action( rdataPtr, exit, err = kNoMemoryErr );
			
			lim = &rdataPtr[ rdataLen ];
			
			ptr = &rdataPtr[ 1 ];
			memcpy( ptr, &hostname[ 1 ], hostname[ 0 ] );
			ptr += hostname[ 0 ];
			if( inIndex != 1 ) SNPrintF_Add( (char **) &ptr, (char *) lim, " (%u)", inIndex );
			rdataPtr[ 0 ] = (uint8_t)( ptr - &rdataPtr[ 1 ] );
			
			check( (size_t)( lim - ptr ) >= recordNameLen );
			memcpy( ptr, recordName, recordNameLen );
			ptr += recordNameLen;
			
			rdataLen = (size_t)( ptr - rdataPtr );
			
			err = _MRResourceRecordCreate( recordName, kDNSServiceType_PTR, kDNSServiceClass_IN, kMDNSRecordTTL_Other,
				(uint16_t) rdataLen, rdataPtr, &answer );
			require_noerr( err, exit );
			recordName	= NULL;
			rdataPtr	= NULL;
			
			*answerPtr = answer;
		}
	}
	else if( _MDNSReplierServiceInstanceNameMatch( inContext, inName, &index, &txtSize, &count ) &&
		( index == inIndex ) && ( count >= inIndex ) )
	{
		int		listHasSRV = false;
		int		listHasTXT = false;
		
		if( inType == kDNSServiceType_ANY )
		{
			for( answer = *inAnswerList; answer; answer = answer->next )
			{
				if( answer->type == kDNSServiceType_SRV )
				{
					if( !listHasSRV && DomainNameEqual( answer->name, inName ) ) listHasSRV = true;
				}
				else if( answer->type == kDNSServiceType_TXT )
				{
					if( !listHasTXT && DomainNameEqual( answer->name, inName ) ) listHasTXT = true;
				}
				if( listHasSRV && listHasTXT ) break;
			}
		}
		
		if( ( inType == kDNSServiceType_SRV ) || ( ( inType == kDNSServiceType_ANY ) && !listHasSRV ) )
		{
			dns_fixed_fields_srv *		fields;
			uint8_t *					ptr;
			uint8_t *					lim;
			uint8_t *					targetPtr;
			
			err = DomainNameDupLower( inName, &recordName, NULL );
			require_noerr( err, exit );
			
			rdataLen = sizeof( dns_fixed_fields_srv ) + 1 + hostname[ 0 ] + 10 + kLocalNameLen;
			rdataPtr = (uint8_t *) malloc( rdataLen );
			require_action( rdataPtr, exit, err = kNoMemoryErr );
			
			lim = &rdataPtr[ rdataLen ];
			
			fields = (dns_fixed_fields_srv *) rdataPtr;
			dns_fixed_fields_srv_init( fields, 0, 0, (uint16_t)( kMDNSReplierPortBase + txtSize ) );
			
			targetPtr = (uint8_t *) &fields[ 1 ];
			
			ptr = &targetPtr[ 1 ];
			memcpy( ptr, &hostname[ 1 ], hostname[ 0 ] );
			ptr += hostname[ 0 ];
			if( inIndex != 1 ) SNPrintF_Add( (char **) &ptr, (char *) lim, "-%u", inIndex );
			targetPtr[ 0 ] = (uint8_t)( ptr - &targetPtr[ 1 ] );
			
			check( (size_t)( lim - ptr ) >= kLocalNameLen );
			memcpy( ptr, kLocalName, kLocalNameLen );
			ptr += kLocalNameLen;
			
			rdataLen = (size_t)( ptr - rdataPtr );
			
			err = _MRResourceRecordCreate( recordName, kDNSServiceType_SRV, kDNSServiceClass_IN, kMDNSRecordTTL_Host,
				(uint16_t) rdataLen, rdataPtr, &answer );
			require_noerr( err, exit );
			recordName	= NULL;
			rdataPtr	= NULL;
			
			*answerPtr = answer;
			 answerPtr = &answer->next;
		}
		
		if( ( inType == kDNSServiceType_TXT ) || ( ( inType == kDNSServiceType_ANY ) && !listHasTXT ) )
		{
			err = DomainNameDupLower( inName, &recordName, NULL );
			require_noerr( err, exit );
			
			rdataLen = txtSize;
			err = _MDNSReplierCreateTXTRecord( inName, rdataLen, &rdataPtr );
			require_noerr( err, exit );
			
			err = _MRResourceRecordCreate( recordName, kDNSServiceType_TXT, kDNSServiceClass_IN, kMDNSRecordTTL_Other,
				(uint16_t) rdataLen, rdataPtr, &answer );
			require_noerr( err, exit );
			recordName	= NULL;
			rdataPtr	= NULL;
			
			*answerPtr = answer;
		}
		else if( inType == kDNSServiceType_NSEC )
		{
			err = DomainNameDupLower( inName, &recordName, NULL );
			require_noerr( err, exit );
			
			err = CreateNSECRecordData( recordName, &rdataPtr, &rdataLen, 2, kDNSServiceType_TXT, kDNSServiceType_SRV );
			require_noerr( err, exit );
			
			err = _MRResourceRecordCreate( recordName, kDNSServiceType_NSEC, kDNSServiceClass_IN, kMDNSRecordTTL_Host,
				(uint16_t) rdataLen, rdataPtr, &answer );
			require_noerr( err, exit );
			recordName	= NULL;
			rdataPtr	= NULL;
			
			*answerPtr = answer;
		}
	}
	err = kNoErr;
	
exit:
	FreeNullSafe( recordName );
	FreeNullSafe( rdataPtr );
	return( err );
}

//===========================================================================================================================
//	_MDNSReplierAnswerListRemovePTR
//===========================================================================================================================

static void
	_MDNSReplierAnswerListRemovePTR(
		MRResourceRecord **	inAnswerListPtr,
		const uint8_t *		inName,
		const uint8_t *		inRData )
{
	MRResourceRecord *		answer;
	MRResourceRecord **		answerPtr;
	
	for( answerPtr = inAnswerListPtr; ( answer = *answerPtr ) != NULL; answerPtr = &answer->next )
	{
		if( ( answer->type == kDNSServiceType_PTR ) && ( answer->class == kDNSServiceClass_IN ) &&
			DomainNameEqual( answer->name, inName ) && DomainNameEqual( answer->rdata, inRData ) ) break;
	}
	if( answer )
	{
		*answerPtr = answer->next;
		_MRResourceRecordFree( answer );
	}
}

//===========================================================================================================================
//	_MDNSReplierSendOrDropResponse
//===========================================================================================================================

static OSStatus
	_MDNSReplierSendOrDropResponse(
		MDNSReplierContext *	inContext,
		MRResourceRecord *		inAnswerList,
		sockaddr_ip *			inQuerier,
		SocketRef				inSock,
		unsigned int			inIndex,
		Boolean					inUnicast )
{
	OSStatus					err;
	uint8_t *					responsePtr	= NULL;
	size_t						responseLen;
	const struct sockaddr *		destAddr;
	ssize_t						n;
	const double				dropRate	= inUnicast ? inContext->ucastDropRate : inContext->mcastDropRate;
	int							drop;
	
	check( inIndex <= inContext->maxInstanceCount );
	
	// If maxDropCount > 0, then the drop rates apply only to the first maxDropCount responses. Otherwise, all messages are
	// subject to their respective drop rate. Also, responses to queries about mDNS replier itself (indicated by index 0),
	// as opposed to those for service instance records, are never dropped.
	
	drop = false;
	if( inIndex > 0 )
	{
		if( inContext->maxDropCount > 0 )
		{
			uint8_t * const		dropCount = &inContext->dropCounters[ inIndex - 1 ];
			
			if( *dropCount < inContext->maxDropCount )
			{
				if( ShouldDrop( dropRate ) ) drop = true;
				*dropCount += 1;
			}
		}
		else if( ShouldDrop( dropRate ) )
		{
			drop = true;
		}
	}
	
	err = _MDNSReplierCreateResponse( inContext, inAnswerList, inIndex, &responsePtr, &responseLen );
	require_noerr( err, exit );
	
	if( inUnicast )
	{
		destAddr = &inQuerier->sa;
	}
	else
	{
		destAddr = ( inQuerier->sa.sa_family == AF_INET ) ? GetMDNSMulticastAddrV4() : GetMDNSMulticastAddrV6();
	}
	
	mr_ulog( kLogLevelInfo, "%s %zu byte response to %##a:\n\n%#1{du:dnsmsg}",
		drop ? "Dropping" : "Sending", responseLen, destAddr, responsePtr, responseLen );
	
	if( !drop )
	{
		n = sendto( inSock, (char *) responsePtr, responseLen, 0, destAddr, SockAddrGetSize( destAddr ) );
		err = map_socket_value_errno( inSock, n == (ssize_t) responseLen, n );
		require_noerr( err, exit );
	}
	
exit:
	FreeNullSafe( responsePtr );
	return( err );
}

//===========================================================================================================================
//	_MDNSReplierCreateResponse
//===========================================================================================================================

static OSStatus
	_MDNSReplierCreateResponse(
		MDNSReplierContext *	inContext,
		MRResourceRecord *		inAnswerList,
		unsigned int			inIndex,
		uint8_t **				outResponsePtr,
		size_t *				outResponseLen )
{
	OSStatus				err;
	DataBuffer				responseDB;
	DNSHeader				hdr;
	MRResourceRecord *		answer;
	uint8_t *				responsePtr;
	size_t					responseLen, len;
	unsigned int			answerCount, recordCount;
	MRNameOffsetItem *		nameOffsetList = NULL;
	
	DataBuffer_Init( &responseDB, NULL, 0, SIZE_MAX );
	
	// The current answers in the answer list will make up the response's Answer Record Section.
	
	answerCount = 0;
	for( answer = inAnswerList; answer; answer = answer->next ) { ++answerCount; }
	
	// Unless configured not to, add any additional answers to the answer list for the Additional Record Section.
	
	if( !inContext->noAdditionals )
	{
		for( answer = inAnswerList; answer; answer = answer->next )
		{
			switch( answer->type )
			{
				case kDNSServiceType_PTR:
					err = _MDNSReplierAnswerListAdd( inContext, &inAnswerList, inIndex, answer->rdata, kDNSServiceType_SRV,
						answer->class );
					require_noerr( err, exit );
					
					err = _MDNSReplierAnswerListAdd( inContext, &inAnswerList, inIndex, answer->rdata, kDNSServiceType_TXT,
						answer->class );
					require_noerr( err, exit );
					break;
				
				case kDNSServiceType_SRV:
					err = _MDNSReplierAnswerListAdd( inContext, &inAnswerList, inIndex, answer->target, kDNSServiceType_A,
						answer->class );
					require_noerr( err, exit );
					
					err = _MDNSReplierAnswerListAdd( inContext, &inAnswerList, inIndex, answer->target, kDNSServiceType_AAAA,
						answer->class );
					require_noerr( err, exit );
					
					err = _MDNSReplierAnswerListAdd( inContext, &inAnswerList, inIndex, answer->name, kDNSServiceType_NSEC,
						answer->class );
					require_noerr( err, exit );
					break;
				
				case kDNSServiceType_TXT:
					err = _MDNSReplierAnswerListAdd( inContext, &inAnswerList, inIndex, answer->name, kDNSServiceType_NSEC,
						answer->class );
					require_noerr( err, exit );
					break;
				
				case kDNSServiceType_A:
					err = _MDNSReplierAnswerListAdd( inContext, &inAnswerList, inIndex, answer->name, kDNSServiceType_AAAA,
						answer->class );
					require_noerr( err, exit );
					
					err = _MDNSReplierAnswerListAdd( inContext, &inAnswerList, inIndex, answer->name, kDNSServiceType_NSEC,
						answer->class );
					require_noerr( err, exit );
					break;
				
				case kDNSServiceType_AAAA:
					err = _MDNSReplierAnswerListAdd( inContext, &inAnswerList, inIndex, answer->name, kDNSServiceType_A,
						answer->class );
					require_noerr( err, exit );
					
					err = _MDNSReplierAnswerListAdd( inContext, &inAnswerList, inIndex, answer->name, kDNSServiceType_NSEC,
						answer->class );
					require_noerr( err, exit );
					break;
				
				default:
					break;
			}
		}
	}
	
	// Append a provisional header to the response message.
	
	memset( &hdr, 0, sizeof( hdr ) );
	DNSHeaderSetFlags( &hdr, kDNSHeaderFlag_Response | kDNSHeaderFlag_AuthAnswer );
	
	err = DataBuffer_Append( &responseDB, &hdr, sizeof( hdr ) );
	require_noerr( err, exit );
	
	// Append answers to response message.
	
	responseLen = DataBuffer_GetLen( &responseDB );
	recordCount = 0;
	for( answer = inAnswerList; answer; answer = answer->next )
	{
		dns_fixed_fields_record		fields;
		unsigned int				class;
		
		// Append record NAME.
		
		err = _MDNSReplierAppendNameToResponse( &responseDB, answer->name, &nameOffsetList );
		require_noerr( err, exit );
		
		// Append record TYPE, CLASS, TTL, and provisional RDLENGTH.
		
		class = answer->class;
		if( ( answer->type == kDNSServiceType_SRV ) || ( answer->type == kDNSServiceType_TXT )  ||
			( answer->type == kDNSServiceType_A )   || ( answer->type == kDNSServiceType_AAAA ) ||
			( answer->type == kDNSServiceType_NSEC ) )
		{
			class |= kRRClassCacheFlushBit;
		}
		
		dns_fixed_fields_record_init( &fields, answer->type, (uint16_t) class, answer->ttl, (uint16_t) answer->rdlength );
		err = DataBuffer_Append( &responseDB, &fields, sizeof( fields ) );
		require_noerr( err, exit );
		
		// Append record RDATA.
		// The RDATA of PTR, SRV, and NSEC records contain domain names, which are subject to name compression.
		
		if( ( answer->type == kDNSServiceType_PTR ) || ( answer->type == kDNSServiceType_SRV ) ||
			( answer->type == kDNSServiceType_NSEC ) )
		{
			size_t				rdlength;
			uint8_t *			rdLengthPtr;
			const size_t		rdLengthOffset	= DataBuffer_GetLen( &responseDB ) - 2;
			const size_t		rdataOffset		= DataBuffer_GetLen( &responseDB );
			
			if( answer->type == kDNSServiceType_PTR )
			{
				err = _MDNSReplierAppendNameToResponse( &responseDB, answer->rdata, &nameOffsetList );
				require_noerr( err, exit );
			}
			else if( answer->type == kDNSServiceType_SRV )
			{
				require_fatal( answer->target == &answer->rdata[ 6 ], "Bad SRV record target pointer." );
				
				err = DataBuffer_Append( &responseDB, answer->rdata, (size_t)( answer->target - answer->rdata ) );
				require_noerr( err, exit );
				
				err = _MDNSReplierAppendNameToResponse( &responseDB, answer->target, &nameOffsetList );
				require_noerr( err, exit );
			}
			else
			{
				const size_t		nameLen = DomainNameLength( answer->rdata );
				
				err = _MDNSReplierAppendNameToResponse( &responseDB, answer->rdata, &nameOffsetList );
				require_noerr( err, exit );
				
				require_fatal( answer->rdlength > nameLen, "Bad NSEC record data length." );
				
				err = DataBuffer_Append( &responseDB, &answer->rdata[ nameLen ], answer->rdlength - nameLen );
				require_noerr( err, exit );
			}
			
			// Set the actual RDLENGTH, which may be less than the original due to name compression.
			
			rdlength = DataBuffer_GetLen( &responseDB ) - rdataOffset;
			check( rdlength <= UINT16_MAX );
			
			rdLengthPtr = DataBuffer_GetPtr( &responseDB ) + rdLengthOffset;
			WriteBig16( rdLengthPtr, rdlength );
		}
		else
		{
			err = DataBuffer_Append( &responseDB, answer->rdata, answer->rdlength );
			require_noerr( err, exit );
		}
		
		if( DataBuffer_GetLen( &responseDB ) > kMDNSMessageSizeMax ) break;
		responseLen = DataBuffer_GetLen( &responseDB );
		++recordCount;
	}
	
	// Set the response header's Answer and Additional record counts.
	// Note: recordCount may be less than answerCount if including all answerCount records would cause the size of the
	// response message to exceed the maximum mDNS message size.
	
	if( recordCount <= answerCount )
	{
		DNSHeaderSetAnswerCount( (DNSHeader *) DataBuffer_GetPtr( &responseDB ), recordCount );
	}
	else
	{
		DNSHeaderSetAnswerCount( (DNSHeader *) DataBuffer_GetPtr( &responseDB ), answerCount );
		DNSHeaderSetAdditionalCount( (DNSHeader *) DataBuffer_GetPtr( &responseDB ), recordCount - answerCount );
	}
	
	err = DataBuffer_Detach( &responseDB, &responsePtr, &len );
	require_noerr( err, exit );
	
	if( outResponsePtr ) *outResponsePtr = responsePtr;
	if( outResponseLen ) *outResponseLen = responseLen;
	
exit:
	_MRNameOffsetItemFreeList( nameOffsetList );
	DataBuffer_Free( &responseDB );
	return( err );
}

//===========================================================================================================================
//	_MDNSReplierAppendNameToResponse
//===========================================================================================================================

static OSStatus
	_MDNSReplierAppendNameToResponse(
		DataBuffer *		inResponse,
		const uint8_t *		inName,
		MRNameOffsetItem **	inNameOffsetListPtr )
{
	OSStatus				err;
	const uint8_t *			subname;
	const uint8_t *			limit;
	size_t					nameOffset;
	MRNameOffsetItem *		item;
	uint8_t					compressionPtr[ 2 ];
	
	nameOffset = DataBuffer_GetLen( inResponse );
	
	// Find the name's longest subname (more accurately, its longest sub-FQDN) in the name compression list.
	
	for( subname = inName; subname[ 0 ] != 0; subname += ( 1 + subname[ 0 ] ) )
	{
		for( item = *inNameOffsetListPtr; item; item = item->next )
		{
			if( DomainNameEqual( item->name, subname ) ) break;
		}
		
		// If an item was found for this subname, then append a name compression pointer and we're done. Otherwise, append
		// the subname's first label.
		
		if( item )
		{
			DNSMessageWriteLabelPointer( compressionPtr, item->offset );
			
			err = DataBuffer_Append( inResponse, compressionPtr, sizeof( compressionPtr ) );
			require_noerr( err, exit );
			break;
		}
		else
		{
			err = DataBuffer_Append( inResponse, subname, 1 + subname[ 0 ] );
			require_noerr( err, exit );
		}
	}
		
	// If we made it to the root label, then no subname was able to be compressed. All of the name's labels up to the root
	// label were appended to the response message, so a root label is needed to terminate the complete name.
	
	if( subname[ 0 ] == 0 )
	{
		err = DataBuffer_Append( inResponse, "", 1 );
		require_noerr( err, exit );
	}
	
	// Add subnames that weren't able to be compressed and their offsets to the name compression list.
	
	limit = subname;
	for( subname = inName; subname < limit; subname += ( 1 + subname[ 0 ] ) )
	{
		const size_t		subnameOffset = nameOffset + (size_t)( subname - inName );
		
		if( subnameOffset > kDNSCompressionOffsetMax ) break;
		
		err = _MRNameOffsetItemCreate( subname, (uint16_t) subnameOffset, &item );
		require_noerr( err, exit );
		
		item->next = *inNameOffsetListPtr;
		*inNameOffsetListPtr = item;
	}
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_MDNSReplierServiceTypeMatch
//===========================================================================================================================

static Boolean
	_MDNSReplierServiceTypeMatch(
		const MDNSReplierContext *	inContext,
		const uint8_t *				inName,
		unsigned int *				outTXTSize,
		unsigned int *				outCount )
{
	OSStatus					err;
	const char *				ptr;
	const char *				end;
	uint32_t					txtSize, count;
	const uint8_t * const		serviceLabel	= inContext->serviceLabel;
	int							nameMatches		= false;
	
	require_quiet( inName[ 0 ] >= serviceLabel[ 0 ], exit );
	if( _memicmp( &inName[ 1 ], &serviceLabel[ 1 ], serviceLabel[ 0 ] ) != 0 ) goto exit;
	
	ptr = (const char *) &inName[ 1 + serviceLabel[ 0 ] ];
	end = (const char *) &inName[ 1 + inName[ 0 ] ];
	
	require_quiet( ( ptr < end ) && ( *ptr == '-' ), exit );
	++ptr;
	
	err = DecimalTextToUInt32( ptr, end, &txtSize, &ptr );
	require_noerr_quiet( err, exit );
	require_quiet( txtSize <= UINT16_MAX, exit );
	
	require_quiet( ( ptr < end ) && ( *ptr == '-' ), exit );
	++ptr;
	
	err = DecimalTextToUInt32( ptr, end, &count, &ptr );
	require_noerr_quiet( err, exit );
	require_quiet( count <= UINT16_MAX, exit );
	require_quiet( ptr == end, exit );
	
	if( !DomainNameEqual( (const uint8_t *) ptr, (const uint8_t *) "\x04" "_tcp" "\x05" "local" ) ) goto exit;
	nameMatches = true;
	
	if( outTXTSize )	*outTXTSize	= txtSize;
	if( outCount )		*outCount	= count;
	
exit:
	return( nameMatches ? true : false );
}

//===========================================================================================================================
//	_MDNSReplierServiceInstanceNameMatch
//===========================================================================================================================

static Boolean
	_MDNSReplierServiceInstanceNameMatch(
		const MDNSReplierContext *	inContext,
		const uint8_t *				inName,
		unsigned int *				outIndex,
		unsigned int *				outTXTSize,
		unsigned int *				outCount )
{
	OSStatus					err;
	const uint8_t *				ptr;
	const uint8_t *				end;
	uint32_t					index;
	unsigned int				txtSize, count;
	const uint8_t * const		hostname	= inContext->hostname;
	int							nameMatches	= false;
	
	require_quiet( inName[ 0 ] >= hostname[ 0 ], exit );
	if( _memicmp( &inName[ 1 ], &hostname[ 1 ], hostname[ 0 ] ) != 0 ) goto exit;
	
	ptr = &inName[ 1 + hostname[ 0 ] ];
	end = &inName[ 1 + inName[ 0 ] ];
	if( ptr < end )
	{
		require_quiet( ( end - ptr ) >= 2, exit );
		require_quiet( ( ptr[ 0 ] == ' ' ) && ( ptr[ 1 ] == '(' ), exit );
		ptr += 2;
		
        err = DecimalTextToUInt32( (const char *) ptr, (const char *) end, &index, (const char **) &ptr );
		require_noerr_quiet( err, exit );
		require_quiet( ( index >= 2 ) && ( index <= UINT16_MAX ), exit );
		
		require_quiet( ( ( end - ptr ) == 1 ) && ( *ptr == ')' ), exit );
		++ptr;
	}
	else
	{
		index = 1;
	}
	
	if( !_MDNSReplierServiceTypeMatch( inContext, ptr, &txtSize, &count ) ) goto exit;
	nameMatches = true;
	
	if( outIndex )		*outIndex	= index;
	if( outTXTSize )	*outTXTSize	= txtSize;
	if( outCount )		*outCount	= count;
	
exit:
	return( nameMatches ? true : false );
}

//===========================================================================================================================
//	_MDNSReplierAboutRecordNameMatch
//===========================================================================================================================

#define _MemIEqual( PTR1, LEN1, PTR2, LEN2 ) \
	( ( ( LEN1 ) == ( LEN2 ) ) && ( _memicmp( ( PTR1 ), ( PTR2 ), ( LEN1 ) ) == 0 ) )

static Boolean	_MDNSReplierAboutRecordNameMatch( const MDNSReplierContext *inContext, const uint8_t *inName )
{
	const uint8_t *				subname;
	const uint8_t * const		hostname	= inContext->hostname;
	int							nameMatches	= false;
	
	if( strnicmpx( &inName[ 1 ], inName[ 0 ], "about" ) != 0 ) goto exit;
	subname = DomainNameGetNextLabel( inName );
	
	if( !_MemIEqual( &subname[ 1 ], subname[ 0 ], &hostname[ 1 ], hostname[ 0 ] ) ) goto exit;
	subname = DomainNameGetNextLabel( subname );
	
	if( !DomainNameEqual( subname, kLocalName ) ) goto exit;
	nameMatches = true;
	
exit:
	return( nameMatches ? true : false );
}

//===========================================================================================================================
//	_MDNSReplierHostnameMatch
//===========================================================================================================================

static Boolean
	_MDNSReplierHostnameMatch(
		const MDNSReplierContext *	inContext,
		const uint8_t *				inName,
		unsigned int *				outIndex )
{
	OSStatus					err;
	const uint8_t *				ptr;
	const uint8_t *				end;
	uint32_t					index;
	const uint8_t * const		hostname	= inContext->hostname;
	int							nameMatches	= false;
	
	require_quiet( inName[ 0 ] >= hostname[ 0 ], exit );
	if( _memicmp( &inName[ 1 ], &hostname[ 1 ], hostname[ 0 ] ) != 0 ) goto exit;
	
	ptr = &inName[ 1 + hostname[ 0 ] ];
	end = &inName[ 1 + inName[ 0 ] ];
	if( ptr < end )
	{
		require_quiet( *ptr == '-', exit );
		++ptr;
		
		err = DecimalTextToUInt32( (const char *) ptr, (const char *) end, &index, (const char **) &ptr );
		require_noerr_quiet( err, exit );
		require_quiet( ( index >= 2 ) && ( index <= UINT16_MAX ), exit );
		require_quiet( ptr == end, exit );
	}
	else
	{
		index = 1;
	}
	
	if( !DomainNameEqual( ptr, kLocalName ) ) goto exit;
	nameMatches = true;
	
	if( outIndex ) *outIndex = index;
	
exit:
	return( nameMatches ? true : false );
}

//===========================================================================================================================
//	_MDNSReplierCreateTXTRecord
//===========================================================================================================================

static OSStatus	_MDNSReplierCreateTXTRecord( const uint8_t *inRecordName, size_t inSize, uint8_t **outTXT )
{
	OSStatus		err;
	uint8_t *		txt;
	uint8_t *		ptr;
	size_t			i, wholeCount, remCount;
	uint32_t		hash;
	int				n;
	uint8_t			txtStr[ 16 ];
	
	require_action_quiet( inSize > 0, exit, err = kSizeErr );
	
	txt = (uint8_t *) malloc( inSize );
	require_action( txt, exit, err = kNoMemoryErr );
	
	hash = _FNV1( inRecordName, DomainNameLength( inRecordName ) );
	
	txtStr[ 0 ] = 15;
	n = MemPrintF( &txtStr[ 1 ], 15, "hash=0x%08X", hash );
	check( n == 15 );
	
	ptr = txt;
	wholeCount = inSize / 16;
	for( i = 0; i < wholeCount; ++i )
	{
		memcpy( ptr, txtStr, 16 );
		ptr += 16;
	}
	
	remCount = inSize % 16;
	if( remCount > 0 )
	{
		txtStr[ 0 ] = (uint8_t)( remCount - 1 );
		memcpy( ptr, txtStr, remCount );
		ptr += remCount;
	}
	check( ptr == &txt[ inSize ] );
	
	*outTXT = txt;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_MRResourceRecordCreate
//===========================================================================================================================

static OSStatus
	_MRResourceRecordCreate(
		uint8_t *			inName,
		uint16_t			inType,
		uint16_t			inClass,
		uint32_t			inTTL,
		uint16_t			inRDLength,
		uint8_t *			inRData,
		MRResourceRecord **	outRecord )
{
	OSStatus				err;
	MRResourceRecord *		obj;
	
	obj = (MRResourceRecord *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	obj->name		= inName;
	obj->type		= inType;
	obj->class		= inClass;
	obj->ttl		= inTTL;
	obj->rdlength	= inRDLength;
	obj->rdata		= inRData;
	
	if( inType == kDNSServiceType_SRV )
	{
		require_action_quiet( obj->rdlength > sizeof( dns_fixed_fields_srv ), exit, err = kMalformedErr );
		obj->target = obj->rdata + sizeof( dns_fixed_fields_srv );
	}
	
	*outRecord = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	FreeNullSafe( obj );
	return( err );
}

//===========================================================================================================================
//	_MRResourceRecordFree
//===========================================================================================================================

static void	_MRResourceRecordFree( MRResourceRecord *inRecord )
{
	ForgetMem( &inRecord->name );
	ForgetMem( &inRecord->rdata );
	free( inRecord );
}

//===========================================================================================================================
//	_MRResourceRecordFreeList
//===========================================================================================================================

static void	_MRResourceRecordFreeList( MRResourceRecord *inList )
{
	MRResourceRecord *		record;
	
	while( ( record = inList ) != NULL )
	{
		inList = record->next;
		_MRResourceRecordFree( record );
	}
}

//===========================================================================================================================
//	_MRNameOffsetItemCreate
//===========================================================================================================================

static OSStatus	_MRNameOffsetItemCreate( const uint8_t *inName, uint16_t inOffset, MRNameOffsetItem **outItem )
{
	OSStatus				err;
	MRNameOffsetItem *		obj;
	size_t					nameLen;
	
	require_action_quiet( inOffset <= kDNSCompressionOffsetMax, exit, err = kSizeErr );
	
	nameLen = DomainNameLength( inName );
	obj = (MRNameOffsetItem *) calloc( 1, offsetof( MRNameOffsetItem, name ) + nameLen );
	require_action( obj, exit, err = kNoMemoryErr );
	
	obj->offset = inOffset;
	memcpy( obj->name, inName, nameLen );
	
	*outItem = obj;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_MRNameOffsetItemFree
//===========================================================================================================================

static void	_MRNameOffsetItemFree( MRNameOffsetItem *inItem )
{
	free( inItem );
}

//===========================================================================================================================
//	_MRNameOffsetItemFreeList
//===========================================================================================================================

static void	_MRNameOffsetItemFreeList( MRNameOffsetItem *inList )
{
	MRNameOffsetItem *		item;
	
	while( ( item = inList ) != NULL )
	{
		inList = item->next;
		_MRNameOffsetItemFree( item );
	}
}

//===========================================================================================================================
//	GAIPerfCmd
//===========================================================================================================================

#define kGAIPerfStandardTTL		( 1 * kSecondsPerHour )

typedef struct GAITesterPrivate *		GAITesterRef;
typedef struct GAITestCase				GAITestCase;

typedef struct
{
	const char *		name;				// Domain name that was resolved.
	uint64_t			connectionTimeUs;	// Time in microseconds that it took to create a DNS-SD connection.
	uint64_t			firstTimeUs;		// Time in microseconds that it took to get the first address result.
	uint64_t			timeUs;				// Time in microseconds that it took to get all expected address results.
	OSStatus			error;
	
}	GAITestItemResult;

typedef void ( *GAITesterStopHandler_f )( void *inContext, OSStatus inError );
typedef void
	( *GAITesterResultsHandler_f )(
		const char *				inCaseTitle,
		NanoTime64					inCaseStartTime,
		NanoTime64					inCaseEndTime,
		const GAITestItemResult *	inResultArray,
		size_t						inResultCount,
		void *						inContext );

typedef unsigned int		GAITestAddrType;
#define kGAITestAddrType_None		0
#define kGAITestAddrType_IPv4		( 1U << 0 )
#define kGAITestAddrType_IPv6		( 1U << 1 )
#define kGAITestAddrType_Both		( kGAITestAddrType_IPv4 | kGAITestAddrType_IPv6 )

#define GAITestAddrTypeIsValid( X ) \
	( ( (X) & kGAITestAddrType_Both ) && ( ( (X) & ~kGAITestAddrType_Both ) == 0 ) )

typedef struct
{
	GAITesterRef			tester;				// GAI tester object.
	CFMutableArrayRef		testCaseResults;	// Array of test case results.
	unsigned int			iterTimeLimitMs;	// Amount of time to allow each iteration to complete.
	unsigned int			callDelayMs;		// Amount of time to wait before calling DNSServiceGetAddrInfo().
	unsigned int			serverDelayMs;		// Amount of additional time to have server delay its responses.
	unsigned int			defaultIterCount;	// Default test case iteration count.
	dispatch_source_t		sigIntSource;		// Dispatch source for SIGINT.
	dispatch_source_t		sigTermSource;		// Dispatch source for SIGTERM.
	char *					outputFilePath;		// File to write test results to. If NULL, then write to stdout.
	OutputFormatType		outputFormat;		// Format of test results output.
	Boolean					skipPathEval;		// True if DNSServiceGetAddrInfo() path evaluation is to be skipped.
	Boolean					badUDPMode;			// True if the test DNS server is to run in Bad UDP mode.
	Boolean					testFailed;			// True if at least one test case iteration failed.
	
}	GAIPerfContext;

static void		GAIPerfContextFree( GAIPerfContext *inContext );
static OSStatus	GAIPerfAddAdvancedTestCases( GAIPerfContext *inContext );
static OSStatus	GAIPerfAddBasicTestCases( GAIPerfContext *inContext );
static void		GAIPerfTesterStopHandler( void *inContext, OSStatus inError );
static void
	GAIPerfResultsHandler(
		const char *				inCaseTitle,
		NanoTime64					inCaseStartTime,
		NanoTime64					inCaseEndTime,
		const GAITestItemResult *	inResultArray,
		size_t						inResultCount,
		void *						inContext );
static void		GAIPerfSignalHandler( void *inContext );

CFTypeID		GAITesterGetTypeID( void );
static OSStatus
	GAITesterCreate(
		dispatch_queue_t	inQueue,
		unsigned int		inCallDelayMs,
		int					inServerDelayMs,
		int					inServerDefaultTTL,
		Boolean				inSkipPathEvaluation,
		Boolean				inBadUDPMode,
		GAITesterRef *		outTester );
static void		GAITesterStart( GAITesterRef inTester );
static void		GAITesterStop( GAITesterRef inTester );
static OSStatus	GAITesterAddTestCase( GAITesterRef inTester, GAITestCase *inCase );
static void
	GAITesterSetStopHandler(
		GAITesterRef			inTester,
		GAITesterStopHandler_f	inEventHandler,
		void *					inEventContext );
static void
	GAITesterSetResultsHandler(
		GAITesterRef				inTester,
		GAITesterResultsHandler_f	inResultsHandler,
		void *						inResultsContext );

static OSStatus	GAITestCaseCreate( const char *inTitle, GAITestCase **outCase );
static void		GAITestCaseFree( GAITestCase *inCase );
static OSStatus
	GAITestCaseAddItem(
		GAITestCase *	inCase,
		unsigned int	inAliasCount,
		unsigned int	inAddressCount,
		int				inTTL,
		GAITestAddrType	inHasAddrs,
		GAITestAddrType	inWantAddrs,
		unsigned int	inTimeLimitMs,
		unsigned int	inItemCount );
static OSStatus
	GAITestCaseAddLocalHostItem(
		GAITestCase *	inCase,
		GAITestAddrType	inWantAddrs,
		unsigned int	inTimeLimitMs,
		unsigned int	inItemCount );

static void	GAIPerfCmd( void )
{
	OSStatus				err;
	GAIPerfContext *		context = NULL;
	
	err = CheckRootUser();
	require_noerr_quiet( err, exit );
	
	err = CheckIntegerArgument( gGAIPerf_CallDelayMs, "call delay (ms)", 0, INT_MAX );
	require_noerr_quiet( err, exit );
	
	err = CheckIntegerArgument( gGAIPerf_ServerDelayMs, "server delay (ms)", 0, INT_MAX );
	require_noerr_quiet( err, exit );
	
	err = CheckIntegerArgument( gGAIPerf_IterationCount, "iteration count", 1, INT_MAX );
	require_noerr_quiet( err, exit );
	
	err = CheckIntegerArgument( gGAIPerf_IterationTimeLimitMs, "iteration time limit (ms)", 0, INT_MAX );
	require_noerr_quiet( err, exit );
	
	context = (GAIPerfContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	context->testCaseResults = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	require_action( context->testCaseResults, exit, err = kNoMemoryErr );
	
	context->iterTimeLimitMs	= (unsigned int) gGAIPerf_IterationTimeLimitMs;
	context->callDelayMs		= (unsigned int) gGAIPerf_CallDelayMs;
	context->serverDelayMs		= (unsigned int) gGAIPerf_ServerDelayMs;
	context->defaultIterCount	= (unsigned int) gGAIPerf_IterationCount;
	context->skipPathEval		= gGAIPerf_SkipPathEvalulation	? true : false;
	context->badUDPMode			= gGAIPerf_BadUDPMode			? true : false;
	
	if( gGAIPerf_OutputFilePath )
	{
		context->outputFilePath = strdup( gGAIPerf_OutputFilePath );
		require_action( context->outputFilePath, exit, err = kNoMemoryErr );
	}
	
	err = OutputFormatFromArgString( gGAIPerf_OutputFormat, &context->outputFormat );
	require_noerr_quiet( err, exit );
	
	err = GAITesterCreate( dispatch_get_main_queue(), context->callDelayMs, (int) context->serverDelayMs,
		kGAIPerfStandardTTL, context->skipPathEval, context->badUDPMode, &context->tester );
	require_noerr( err, exit );
	
	check( gGAIPerf_TestSuite );
	if( strcasecmp( gGAIPerf_TestSuite, kGAIPerfTestSuiteName_Basic ) == 0 )
	{
		err = GAIPerfAddBasicTestCases( context );
		require_noerr( err, exit );
	}
	else if( strcasecmp( gGAIPerf_TestSuite, kGAIPerfTestSuiteName_Advanced ) == 0 )
	{
		err = GAIPerfAddAdvancedTestCases( context );
		require_noerr( err, exit );
	}
	else
	{
		FPrintF( stderr, "error: Invalid test suite name: %s.\n", gGAIPerf_TestSuite );
		goto exit;
	}
	
	GAITesterSetStopHandler( context->tester, GAIPerfTesterStopHandler, context );
	GAITesterSetResultsHandler( context->tester, GAIPerfResultsHandler, context );
	
	signal( SIGINT, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGINT, dispatch_get_main_queue(), GAIPerfSignalHandler, context,
		&context->sigIntSource );
	require_noerr( err, exit );
	dispatch_resume( context->sigIntSource );
	
	signal( SIGTERM, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGTERM, dispatch_get_main_queue(), GAIPerfSignalHandler, context,
		&context->sigTermSource );
	require_noerr( err, exit );
	dispatch_resume( context->sigTermSource );
	
	GAITesterStart( context->tester );
	dispatch_main();
	
exit:
	if( context ) GAIPerfContextFree( context );
	exit( 1 );
}

//===========================================================================================================================
//	GAIPerfContextFree
//===========================================================================================================================

static void	GAIPerfContextFree( GAIPerfContext *inContext )
{
	ForgetCF( &inContext->tester );
	ForgetCF( &inContext->testCaseResults );
	ForgetMem( &inContext->outputFilePath );
	dispatch_source_forget( &inContext->sigIntSource );
	dispatch_source_forget( &inContext->sigTermSource );
	free( inContext );
}

//===========================================================================================================================
//	GAIPerfAddAdvancedTestCases
//===========================================================================================================================

#define kTestCaseTitleBufferSize		128

static void
	_GAIPerfWriteTestCaseTitle(
		char			inBuffer[ kTestCaseTitleBufferSize ],
		unsigned int	inCNAMERecordCount,
		unsigned int	inARecordCount,
		unsigned int	inAAAARecordCount,
		GAITestAddrType	inRequested,
		unsigned int	inIterationCount,
		Boolean			inIterationsAreUnique );
static void
	_GAIPerfWriteLocalHostTestCaseTitle(
		char			inBuffer[ kTestCaseTitleBufferSize ],
		GAITestAddrType	inRequested,
		unsigned int	inIterationCount );

#define kGAIPerfAdvancedTestSuite_MaxAliasCount		4
#define kGAIPerfAdvancedTestSuite_MaxAddrCount		8

static OSStatus	GAIPerfAddAdvancedTestCases( GAIPerfContext *inContext )
{
	OSStatus			err;
	unsigned int		aliasCount, addressCount, i;
	GAITestCase *		testCase = NULL;
	char				title[ kTestCaseTitleBufferSize ];
	
	aliasCount = 0;
	while( aliasCount <= kGAIPerfAdvancedTestSuite_MaxAliasCount )
	{
		for( addressCount = 1; addressCount <= kGAIPerfAdvancedTestSuite_MaxAddrCount; addressCount *= 2 )
		{
			// Add a test case to resolve a domain name with
			//
			//     <aliasCount> CNAME records, <addressCount> A records, and <addressCount> AAAA records
			//
			// to its IPv4 and IPv6 addresses. Each iteration resolves a unique instance of such a domain name, which
			// requires server queries.
			
			_GAIPerfWriteTestCaseTitle( title, aliasCount, addressCount, addressCount, kGAITestAddrType_Both,
				inContext->defaultIterCount, true );
			
			err = GAITestCaseCreate( title, &testCase );
			require_noerr( err, exit );
			
			for( i = 0; i < inContext->defaultIterCount; ++i )
			{
				err = GAITestCaseAddItem( testCase, aliasCount, addressCount, kGAIPerfStandardTTL,
					kGAITestAddrType_Both, kGAITestAddrType_Both, inContext->iterTimeLimitMs, 1 );
				require_noerr( err, exit );
			}
			
			err = GAITesterAddTestCase( inContext->tester, testCase );
			require_noerr( err, exit );
			testCase = NULL;
			
			// Add a test case to resolve a domain name with
			//
			//     <aliasCount> CNAME records, <addressCount> A records, and <addressCount> AAAA records
			//
			// to its IPv4 and IPv6 addresses. A preliminary iteration resolves a unique domain name, which requires a server
			// query. The subsequent iterations resolve the same domain name as the preliminary iteration, which should
			// ideally require no server queries, i.e., the results should come from the cache.
			
			_GAIPerfWriteTestCaseTitle( title, aliasCount, addressCount, addressCount, kGAITestAddrType_Both,
				inContext->defaultIterCount, false );
			
			err = GAITestCaseCreate( title, &testCase );
			require_noerr( err, exit );
			
			err = GAITestCaseAddItem( testCase, aliasCount, addressCount, kGAIPerfStandardTTL,
				kGAITestAddrType_Both, kGAITestAddrType_Both, inContext->iterTimeLimitMs, inContext->defaultIterCount + 1 );
			require_noerr( err, exit );
			
			err = GAITesterAddTestCase( inContext->tester, testCase );
			require_noerr( err, exit );
			testCase = NULL;
		}
		
		aliasCount = ( aliasCount == 0 ) ? 1 : ( 2 * aliasCount );
	}
	
	// Finally, add a test case to resolve localhost to its IPv4 and IPv6 addresses.
	
	_GAIPerfWriteLocalHostTestCaseTitle( title, kGAITestAddrType_Both, inContext->defaultIterCount );
	
	err = GAITestCaseCreate( title, &testCase );
	require_noerr( err, exit );
	
	err = GAITestCaseAddLocalHostItem( testCase, kGAITestAddrType_Both, inContext->iterTimeLimitMs,
		inContext->defaultIterCount );
	require_noerr( err, exit );
	
	err = GAITesterAddTestCase( inContext->tester, testCase );
	require_noerr( err, exit );
	testCase = NULL;
	
exit:
	if( testCase ) GAITestCaseFree( testCase );
	return( err );
}

//===========================================================================================================================
//	_GAIPerfWriteTestCaseTitle
//===========================================================================================================================

#define GAITestAddrTypeToRequestKeyValue( X ) (				\
	( (X) == kGAITestAddrType_Both ) ? "ipv4\\,ipv6"	:	\
	( (X) == kGAITestAddrType_IPv4 ) ? "ipv4"			:	\
	( (X) == kGAITestAddrType_IPv6 ) ? "ipv6"			:	\
									   "" )

static void
	_GAIPerfWriteTestCaseTitle(
		char			inBuffer[ kTestCaseTitleBufferSize ],
		unsigned int	inCNAMERecordCount,
		unsigned int	inARecordCount,
		unsigned int	inAAAARecordCount,
		GAITestAddrType	inRequested,
		unsigned int	inIterationCount,
		Boolean			inIterationsAreUnique )
{
	SNPrintF( inBuffer, kTestCaseTitleBufferSize, "name=dynamic,cname=%u,a=%u,aaaa=%u,req=%s,iterations=%u%?s",
		inCNAMERecordCount, inARecordCount, inAAAARecordCount, GAITestAddrTypeToRequestKeyValue( inRequested ),
		inIterationCount, inIterationsAreUnique, ",unique" );
}

//===========================================================================================================================
//	_GAIPerfWriteLocalHostTestCaseTitle
//===========================================================================================================================

static void
	_GAIPerfWriteLocalHostTestCaseTitle(
		char			inBuffer[ kTestCaseTitleBufferSize ],
		GAITestAddrType	inRequested,
		unsigned int	inIterationCount )
{
	SNPrintF( inBuffer, kTestCaseTitleBufferSize, "name=localhost,req=%s,iterations=%u",
		GAITestAddrTypeToRequestKeyValue( inRequested ), inIterationCount );
}

//===========================================================================================================================
//	GAIPerfAddBasicTestCases
//===========================================================================================================================

#define kGAIPerfBasicTestSuite_AliasCount		2
#define kGAIPerfBasicTestSuite_AddrCount		4

static OSStatus	GAIPerfAddBasicTestCases( GAIPerfContext *inContext )
{
	OSStatus			err;
	GAITestCase *		testCase = NULL;
	char				title[ kTestCaseTitleBufferSize ];
	unsigned int		i;
	
	// Test Case #1:
	// Resolve a domain name with
	//
	//     2 CNAME records, 4 A records, and 4 AAAA records
	//
	// to its IPv4 and IPv6 addresses. Each of the iterations resolves a unique domain name, which requires server
	// queries.
	
	_GAIPerfWriteTestCaseTitle( title, kGAIPerfBasicTestSuite_AliasCount,
		kGAIPerfBasicTestSuite_AddrCount, kGAIPerfBasicTestSuite_AddrCount, kGAITestAddrType_Both,
		inContext->defaultIterCount, true );
	
	err = GAITestCaseCreate( title, &testCase );
	require_noerr( err, exit );
	
	for( i = 0; i < inContext->defaultIterCount; ++i )
	{
		err = GAITestCaseAddItem( testCase, kGAIPerfBasicTestSuite_AliasCount, kGAIPerfBasicTestSuite_AddrCount,
			kGAIPerfStandardTTL, kGAITestAddrType_Both, kGAITestAddrType_Both, inContext->iterTimeLimitMs, 1 );
		require_noerr( err, exit );
	}
	
	err = GAITesterAddTestCase( inContext->tester, testCase );
	require_noerr( err, exit );
	testCase = NULL;
	
	// Test Case #2:
	// Resolve a domain name with
	//
	//     2 CNAME records, 4 A records, and 4 AAAA records
	//
	// to its IPv4 and IPv6 addresses. A preliminary iteration resolves a unique instance of such a domain name, which
	// requires server queries. Each of the subsequent iterations resolves the same domain name as the preliminary
	// iteration, which should ideally require no additional server queries, i.e., the results should come from the cache.
	
	_GAIPerfWriteTestCaseTitle( title, kGAIPerfBasicTestSuite_AliasCount,
		kGAIPerfBasicTestSuite_AddrCount, kGAIPerfBasicTestSuite_AddrCount, kGAITestAddrType_Both,
		inContext->defaultIterCount, false );
	
	err = GAITestCaseCreate( title, &testCase );
	require_noerr( err, exit );
	
	err = GAITestCaseAddItem( testCase, kGAIPerfBasicTestSuite_AliasCount, kGAIPerfBasicTestSuite_AddrCount,
		kGAIPerfStandardTTL, kGAITestAddrType_Both, kGAITestAddrType_Both, inContext->iterTimeLimitMs,
		inContext->defaultIterCount + 1 );
	require_noerr( err, exit );
	
	err = GAITesterAddTestCase( inContext->tester, testCase );
	require_noerr( err, exit );
	testCase = NULL;
	
	// Test Case #3:
	// Each iteration resolves localhost to its IPv4 and IPv6 addresses.
	
	_GAIPerfWriteLocalHostTestCaseTitle( title, kGAITestAddrType_Both, inContext->defaultIterCount );
	
	err = GAITestCaseCreate( title, &testCase );
	require_noerr( err, exit );
	
	err = GAITestCaseAddLocalHostItem( testCase, kGAITestAddrType_Both, inContext->iterTimeLimitMs,
		inContext->defaultIterCount );
	require_noerr( err, exit );
	
	err = GAITesterAddTestCase( inContext->tester, testCase );
	require_noerr( err, exit );
	testCase = NULL;
	
exit:
	if( testCase ) GAITestCaseFree( testCase );
	return( err );
}

//===========================================================================================================================
//	GAIPerfTesterStopHandler
//===========================================================================================================================

#define kGAIPerfResultsKey_Info				CFSTR( "info" )
#define kGAIPerfResultsKey_TestCases		CFSTR( "testCases" )
#define kGAIPerfResultsKey_Success			CFSTR( "success" )

#define kGAIPerfInfoKey_CallDelay			CFSTR( "callDelayMs" )
#define kGAIPerfInfoKey_ServerDelay			CFSTR( "serverDelayMs" )
#define kGAIPerfInfoKey_SkippedPathEval		CFSTR( "skippedPathEval" )
#define kGAIPerfInfoKey_UsedBadUDPMode		CFSTR( "usedBadUPDMode" )

static void	GAIPerfTesterStopHandler( void *inContext, OSStatus inError )
{
	OSStatus					err;
	GAIPerfContext * const		context = (GAIPerfContext *) inContext;
	CFPropertyListRef			plist;
	int							exitCode;
	
	err = inError;
	require_noerr_quiet( err, exit );
	
	err = CFPropertyListCreateFormatted( kCFAllocatorDefault, &plist,
		"{"
			"%kO="			// info
			"{"
				"%kO=%lli"	// callDelayMs
				"%kO=%lli"	// serverDelayMs
				"%kO=%b"	// skippedPathEval
				"%kO=%b"	// usedBadUPDMode
			"}"
			"%kO=%O"		// testCases
			"%kO=%b"		// success
		"}",
		kGAIPerfResultsKey_Info,
		kGAIPerfInfoKey_CallDelay,			(int64_t) context->callDelayMs,
		kGAIPerfInfoKey_ServerDelay,		(int64_t) context->serverDelayMs,
		kGAIPerfInfoKey_SkippedPathEval,	context->skipPathEval,
		kGAIPerfInfoKey_UsedBadUDPMode,		context->badUDPMode,
		kGAIPerfResultsKey_TestCases,		context->testCaseResults,
		kGAIPerfResultsKey_Success,			!context->testFailed );
	require_noerr( err, exit );
	
	err = OutputPropertyList( plist, context->outputFormat, context->outputFilePath );
	CFRelease( plist );
	require_noerr( err, exit );
	
exit:
	exitCode = err ? 1 : ( context->testFailed ? 2 : 0 );
	GAIPerfContextFree( context );
	exit( exitCode );
}

//===========================================================================================================================
//	GAIPerfResultsHandler
//===========================================================================================================================

// Keys for test case dictionary

#define kGAIPerfTestCaseKey_Title				CFSTR( "title" )
#define kGAIPerfTestCaseKey_StartTime			CFSTR( "startTime" )
#define kGAIPerfTestCaseKey_EndTime				CFSTR( "endTime" )
#define kGAIPerfTestCaseKey_Results				CFSTR( "results" )
#define kGAIPerfTestCaseKey_FirstStats			CFSTR( "firstStats" )
#define kGAIPerfTestCaseKey_ConnectionStats		CFSTR( "connectionStats" )
#define kGAIPerfTestCaseKey_Stats				CFSTR( "stats" )

// Keys for test case results array entry dictionaries

#define kGAIPerfTestCaseResultKey_Name					CFSTR( "name" )
#define kGAIPerfTestCaseResultKey_ConnectionTime		CFSTR( "connectionTimeUs" )
#define kGAIPerfTestCaseResultKey_FirstTime				CFSTR( "firstTimeUs" )
#define kGAIPerfTestCaseResultKey_Time					CFSTR( "timeUs" )

// Keys for test case stats dictionaries

#define kGAIPerfTestCaseStatsKey_Count		CFSTR( "count" )
#define kGAIPerfTestCaseStatsKey_Min		CFSTR( "min" )
#define kGAIPerfTestCaseStatsKey_Max		CFSTR( "max" )
#define kGAIPerfTestCaseStatsKey_Mean		CFSTR( "mean" )
#define kGAIPerfTestCaseStatsKey_StdDev		CFSTR( "sd" )

typedef struct
{
	double		min;
	double		max;
	double		mean;
	double		stdDev;
	
}	GAIPerfStats;

#define GAIPerfStatsInit( X ) \
	do { (X)->min = DBL_MAX; (X)->max = DBL_MIN; (X)->mean = 0.0; (X)->stdDev = 0.0; } while( 0 )

static void
	GAIPerfResultsHandler(
		const char *				inCaseTitle,
		NanoTime64					inCaseStartTime,
		NanoTime64					inCaseEndTime,
		const GAITestItemResult *	inResultArray,
		size_t						inResultCount,
		void *						inContext )
{
	OSStatus						err;
	GAIPerfContext * const			context	= (GAIPerfContext *) inContext;
	int								namesAreDynamic, namesAreUnique;
	const char *					ptr;
	size_t							count, startIndex;
	CFMutableArrayRef				results	= NULL;
	GAIPerfStats					stats, firstStats, connStats;
	double							sum, firstSum, connSum;
	size_t							keyValueLen, i;
	char							keyValue[ 16 ];	// Size must be at least strlen( "name=dynamic" ) + 1 bytes.
	char							startTime[ 32 ];
	char							endTime[ 32 ];
	const GAITestItemResult *		result;
	
	// If this test case resolves the same "d.test." name in each iteration (title contains the "name=dynamic" key-value
	// pair, but not the "unique" key), then don't count the first iteration, whose purpose is to populate the cache with
	// the domain name's CNAME, A, and AAAA records.
	
	namesAreDynamic	= false;
	namesAreUnique	= false;
	ptr = inCaseTitle;
	while( _ParseQuotedEscapedString( ptr, NULL, ",", keyValue, sizeof( keyValue ), &keyValueLen, NULL, &ptr ) )
	{
		if( strnicmpx( keyValue, keyValueLen, "name=dynamic" ) == 0 )
		{
			namesAreDynamic = true;
		}
		else if( strnicmpx( keyValue, keyValueLen, "unique" ) == 0 )
		{
			namesAreUnique = true;
		}
		if( namesAreDynamic && namesAreUnique ) break;
	}
	
	startIndex = ( ( inResultCount > 0 ) && namesAreDynamic && !namesAreUnique ) ? 1 : 0;
	results = CFArrayCreateMutable( NULL, (CFIndex)( inResultCount - startIndex ), &kCFTypeArrayCallBacks );
	require_action( results, exit, err = kNoMemoryErr );
	
	GAIPerfStatsInit( &stats );
	GAIPerfStatsInit( &firstStats );
	GAIPerfStatsInit( &connStats );
	
	sum			= 0.0;
	firstSum	= 0.0;
	connSum		= 0.0;
	count		= 0;
	for( i = startIndex; i < inResultCount; ++i )
	{
		double		value;
		
		result = &inResultArray[ i ];
		
		err = CFPropertyListAppendFormatted( kCFAllocatorDefault, results,
			"{"
				"%kO=%s"	// name
				"%kO=%lli"	// connectionTimeUs
				"%kO=%lli"	// firstTimeUs
				"%kO=%lli"	// timeUs
				"%kO=%lli"	// error
			"}",
			kGAIPerfTestCaseResultKey_Name,				result->name,
			kGAIPerfTestCaseResultKey_ConnectionTime,	(int64_t) result->connectionTimeUs,
			kGAIPerfTestCaseResultKey_FirstTime,		(int64_t) result->firstTimeUs,
			kGAIPerfTestCaseResultKey_Time,				(int64_t) result->timeUs,
			CFSTR( "error" ),							(int64_t) result->error );
		require_noerr( err, exit );
		
		if( !result->error )
		{
			value = (double) result->timeUs;
			if( value < stats.min ) stats.min = value;
			if( value > stats.max ) stats.max = value;
			sum += value;
			
			value = (double) result->firstTimeUs;
			if( value < firstStats.min ) firstStats.min = value;
			if( value > firstStats.max ) firstStats.max = value;
			firstSum += value;
			
			value = (double) result->connectionTimeUs;
			if( value < connStats.min ) connStats.min = value;
			if( value > connStats.max ) connStats.max = value;
			connSum += value;
			
			++count;
		}
		else
		{
			context->testFailed = true;
		}
	}
	
	if( count > 0 )
	{
		stats.mean		= sum      / count;
		firstStats.mean	= firstSum / count;
		connStats.mean	= connSum  / count;
		
		sum			= 0.0;
		firstSum	= 0.0;
		connSum		= 0.0;
		for( i = startIndex; i < inResultCount; ++i )
		{
			double		diff;
			
			result = &inResultArray[ i ];
			if( result->error ) continue;
			
			diff		 = stats.mean - (double) result->timeUs;
			sum			+= ( diff * diff );
			
			diff		 = firstStats.mean - (double) result->firstTimeUs;
			firstSum	+= ( diff * diff );
			
			diff		 = connStats.mean - (double) result->connectionTimeUs;
			connSum		+= ( diff * diff );
		}
		stats.stdDev		= sqrt( sum      / count );
		firstStats.stdDev	= sqrt( firstSum / count );
		connStats.stdDev	= sqrt( connSum  / count );
	}
	
	err = CFPropertyListAppendFormatted( kCFAllocatorDefault, context->testCaseResults,
		"{"
			"%kO=%s"
			"%kO=%s"
			"%kO=%s"
			"%kO=%O"
			"%kO="
			"{"
				"%kO=%lli"
				"%kO=%f"
				"%kO=%f"
				"%kO=%f"
				"%kO=%f"
			"}"
			"%kO="
			"{"
				"%kO=%lli"
				"%kO=%f"
				"%kO=%f"
				"%kO=%f"
				"%kO=%f"
			"}"
			"%kO="
			"{"
				"%kO=%lli"
				"%kO=%f"
				"%kO=%f"
				"%kO=%f"
				"%kO=%f"
			"}"
		"}",
		kGAIPerfTestCaseKey_Title,			inCaseTitle,
		kGAIPerfTestCaseKey_StartTime,		_NanoTime64ToTimestamp( inCaseStartTime, startTime, sizeof( startTime ) ),
		kGAIPerfTestCaseKey_EndTime,		_NanoTime64ToTimestamp( inCaseEndTime, endTime, sizeof( endTime ) ),
		kGAIPerfTestCaseKey_Results,		results,
		kGAIPerfTestCaseKey_Stats,
		kGAIPerfTestCaseStatsKey_Count,		(int64_t) count,
		kGAIPerfTestCaseStatsKey_Min,		stats.min,
		kGAIPerfTestCaseStatsKey_Max,		stats.max,
		kGAIPerfTestCaseStatsKey_Mean,		stats.mean,
		kGAIPerfTestCaseStatsKey_StdDev,	stats.stdDev,
		kGAIPerfTestCaseKey_FirstStats,
		kGAIPerfTestCaseStatsKey_Count,		(int64_t) count,
		kGAIPerfTestCaseStatsKey_Min,		firstStats.min,
		kGAIPerfTestCaseStatsKey_Max,		firstStats.max,
		kGAIPerfTestCaseStatsKey_Mean,		firstStats.mean,
		kGAIPerfTestCaseStatsKey_StdDev,	firstStats.stdDev,
		kGAIPerfTestCaseKey_ConnectionStats,
		kGAIPerfTestCaseStatsKey_Count,		(int64_t) count,
		kGAIPerfTestCaseStatsKey_Min,		connStats.min,
		kGAIPerfTestCaseStatsKey_Max,		connStats.max,
		kGAIPerfTestCaseStatsKey_Mean,		connStats.mean,
		kGAIPerfTestCaseStatsKey_StdDev,	connStats.stdDev );
	require_noerr( err, exit );
	
exit:
	CFReleaseNullSafe( results );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	GAIPerfSignalHandler
//===========================================================================================================================

static void	GAIPerfSignalHandler( void *inContext )
{
	GAIPerfContext * const		context = (GAIPerfContext *) inContext;
	
	if( !context->tester ) exit( 1 );
	GAITesterStop( context->tester );
	context->tester = NULL;
}

//===========================================================================================================================
//	GAITesterCreate
//===========================================================================================================================

// A character set of lower-case alphabet characters and digits and a string length of six allows for 36^6 = 2,176,782,336
// possible strings to use in the Tag label.

#define kGAITesterTagStringLen		6

typedef struct GAITestItem		GAITestItem;
struct GAITestItem
{
	GAITestItem *		next;				// Next test item in list.
	char *				name;				// Domain name to resolve.
	uint64_t			connectionTimeUs;	// Time in microseconds that it took to create a DNS-SD connection.
	uint64_t			firstTimeUs;		// Time in microseconds that it took to get the first address result.
	uint64_t			timeUs;				// Time in microseconds that it took to get all expected address results.
	unsigned int		addressCount;		// Address count of the domain name, i.e., the Count label argument.
	OSStatus			error;				// Current status/error.
	unsigned int		timeLimitMs;		// Time limit in milliseconds for the test item's completion.
	Boolean				hasV4;				// True if the domain name has one or more IPv4 addresses.
	Boolean				hasV6;				// True if the domain name has one or more IPv6 addresses.
	Boolean				wantV4;				// True if DNSServiceGetAddrInfo() should be called to get IPv4 addresses.
	Boolean				wantV6;				// True if DNSServiceGetAddrInfo() should be called to get IPv6 addresses.
};

struct GAITestCase
{
	GAITestCase *		next;		// Next test case in list.
	GAITestItem *		itemList;	// List of test items.
	char *				title;		// Title of the test case.
};

struct GAITesterPrivate
{
	CFRuntimeBase					base;				// CF object base.
	dispatch_queue_t				queue;				// Serial work queue.
	DNSServiceRef					connection;			// Reference to the shared DNS-SD connection.
	DNSServiceRef					getAddrInfo;		// Reference to the current DNSServiceGetAddrInfo operation.
	GAITestCase *					caseList;			// List of test cases.
	GAITestCase *					currentCase;		// Pointer to the current test case.
	GAITestItem *					currentItem;		// Pointer to the current test item.
	NanoTime64						caseStartTime;		// Start time of current test case in Unix time as nanoseconds.
	NanoTime64						caseEndTime;		// End time of current test case in Unix time as nanoseconds.
	unsigned int					callDelayMs;		// Amount of time to wait before calling DNSServiceGetAddrInfo().
	Boolean							skipPathEval;		// True if DNSServiceGetAddrInfo() path evaluation is to be skipped.
	Boolean							stopped;			// True if the tester has been stopped.
	Boolean							badUDPMode;			// True if the test DNS server is to run in Bad UDP mode.
	dispatch_source_t				timer;				// Timer for enforcing a test item's time limit.
	pcap_t *						pcap;				// Captures traffic between mDNSResponder and test DNS server.
	pid_t							serverPID;			// PID of the test DNS server.
	int								serverDelayMs;		// Additional time to have the server delay its responses by.
	int								serverDefaultTTL;	// Default TTL for the server's records.
	GAITesterStopHandler_f			stopHandler;		// User's stop handler.
	void *							stopContext;		// User's event handler context.
	GAITesterResultsHandler_f		resultsHandler;		// User's results handler.
	void *							resultsContext;		// User's results handler context.
	
	// Variables for current test item.
	
	uint64_t						bitmapV4;		// Bitmap of IPv4 results that have yet to be received.
	uint64_t						bitmapV6;		// Bitmap of IPv6 results that have yet to be received.
	uint64_t						startTicks;		// Start ticks of DNSServiceGetAddrInfo().
	uint64_t						connTicks;		// Ticks when the connection was created.
	uint64_t						firstTicks;		// Ticks when the first DNSServiceGetAddrInfo result was received.
	uint64_t						endTicks;		// Ticks when the last DNSServiceGetAddrInfo result was received.
	Boolean							gotFirstResult;	// True if the first result has been received.
};

CF_CLASS_DEFINE( GAITester );

static void		_GAITesterStartNextTest( GAITesterRef inTester );
static OSStatus	_GAITesterCreatePacketCapture( pcap_t **outPCap );
static void		_GAITesterFirstGAITimeout( void *inContext );
static void		_GAITesterTimeout( void *inContext );
static void DNSSD_API
	_GAITesterFirstGAICallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inHostname,
		const struct sockaddr *	inSockAddr,
		uint32_t				inTTL,
		void *					inContext );
static void DNSSD_API
	_GAITesterGetAddrInfoCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inHostname,
		const struct sockaddr *	inSockAddr,
		uint32_t				inTTL,
		void *					inContext );
static void		_GAITesterCompleteCurrentTest( GAITesterRef inTester, OSStatus inError );

#define ForgetPacketCapture( X )		ForgetCustom( X, pcap_close )

static OSStatus
	GAITestItemCreate(
		const char *	inName,
		unsigned int	inAddressCount,
		GAITestAddrType	inHasAddrs,
		GAITestAddrType	inWantAddrs,
		unsigned int	inTimeLimitMs,
		GAITestItem **	outItem );
static OSStatus	GAITestItemDup( const GAITestItem *inItem, GAITestItem **outItem );
static void		GAITestItemFree( GAITestItem *inItem );

static OSStatus
	GAITesterCreate(
		dispatch_queue_t	inQueue,
		unsigned int		inCallDelayMs,
		int					inServerDelayMs,
		int					inServerDefaultTTL,
		Boolean				inSkipPathEvaluation,
		Boolean				inBadUDPMode,
		GAITesterRef *		outTester )
{
	OSStatus			err;
	GAITesterRef		obj = NULL;
	
	CF_OBJECT_CREATE( GAITester, obj, err, exit );
	
	ReplaceDispatchQueue( &obj->queue, inQueue );
	obj->callDelayMs		= inCallDelayMs;
	obj->serverPID			= -1;
	obj->serverDelayMs		= inServerDelayMs;
	obj->serverDefaultTTL	= inServerDefaultTTL;
	obj->skipPathEval		= inSkipPathEvaluation;
	obj->badUDPMode			= inBadUDPMode;
	
	*outTester = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	CFReleaseNullSafe( obj );
	return( err );
}

//===========================================================================================================================
//	_GAITesterFinalize
//===========================================================================================================================

static void	_GAITesterFinalize( CFTypeRef inObj )
{
	GAITesterRef const		me = (GAITesterRef) inObj;
	GAITestCase *			testCase;
	
	check( !me->getAddrInfo );
	check( !me->connection );
	check( !me->timer );
	dispatch_forget( &me->queue );
	while( ( testCase = me->caseList ) != NULL )
	{
		me->caseList = testCase->next;
		GAITestCaseFree( testCase );
	}
}

//===========================================================================================================================
//	GAITesterStart
//===========================================================================================================================

static void	_GAITesterStart( void *inContext );
static void	_GAITesterStop( GAITesterRef me, OSStatus inError );

static void	GAITesterStart( GAITesterRef me )
{
	CFRetain( me );
	dispatch_async_f( me->queue, me, _GAITesterStart );
}

#define kGAITesterFirstGAITimeoutSecs		4

static void	_GAITesterStart( void *inContext )
{
	OSStatus				err;
	GAITesterRef const		me = (GAITesterRef) inContext;
	DNSServiceFlags			flags;
	char					name[ 64 ];
	char					tag[ kGAITesterTagStringLen + 1 ];
	
	err = SpawnCommand( &me->serverPID, "dnssdutil server --loopback --follow %lld%?s%?d%?s%?d%?s",
		(int64_t) getpid(),
		me->serverDefaultTTL >= 0,	" --defaultTTL ",
		me->serverDefaultTTL >= 0,	me->serverDefaultTTL,
		me->serverDelayMs    >= 0,	" --responseDelay ",
		me->serverDelayMs    >= 0,	me->serverDelayMs,
		me->badUDPMode,				" --badUDPMode" );
	require_noerr_quiet( err, exit );
	
	SNPrintF( name, sizeof( name ), "tag-gaitester-probe-%s.ipv4.d.test",
		_RandomStringExact( kLowerAlphaNumericCharSet, kLowerAlphaNumericCharSetSize, sizeof( tag ) - 1, tag ) );
	
	flags = 0;
	if( me->skipPathEval ) flags |= kDNSServiceFlagsPathEvaluationDone;
	
	err = DNSServiceGetAddrInfo( &me->getAddrInfo, flags, kDNSServiceInterfaceIndexAny, kDNSServiceProtocol_IPv4, name,
		_GAITesterFirstGAICallback, me );
	require_noerr( err, exit );
	
	err = DNSServiceSetDispatchQueue( me->getAddrInfo, me->queue );
	require_noerr( err, exit );
	
	err = DispatchTimerOneShotCreate( dispatch_time_seconds( kGAITesterFirstGAITimeoutSecs ),
		UINT64_C_safe( kGAITesterFirstGAITimeoutSecs ) * kNanosecondsPerSecond / 10, me->queue,
		_GAITesterFirstGAITimeout, me, &me->timer );
	require_noerr( err, exit );
	dispatch_resume( me->timer );
	
exit:
	if( err ) _GAITesterStop( me, err );
}

//===========================================================================================================================
//	GAITesterStop
//===========================================================================================================================

static void	_GAITesterUserStop( void *inContext );

static void	GAITesterStop( GAITesterRef me )
{
	CFRetain( me );
	dispatch_async_f( me->queue, me, _GAITesterUserStop );
}

static void	_GAITesterUserStop( void *inContext )
{
	GAITesterRef const		me = (GAITesterRef) inContext;
	
	_GAITesterStop( me, kCanceledErr );
	CFRelease( me );
}

static void	_GAITesterStop( GAITesterRef me, OSStatus inError )
{
	OSStatus		err;
	
	ForgetPacketCapture( &me->pcap );
	dispatch_source_forget( &me->timer );
	DNSServiceForget( &me->getAddrInfo );
	DNSServiceForget( &me->connection );
	if( me->serverPID != -1 )
	{
		err = kill( me->serverPID, SIGTERM );
		err = map_global_noerr_errno( err );
		check_noerr( err );
		me->serverPID = -1;
	}
	
	if( !me->stopped )
	{
		me->stopped = true;
		if( me->stopHandler ) me->stopHandler( me->stopContext, inError );
		CFRelease( me );
	}
}

//===========================================================================================================================
//	GAITesterAddTestCase
//===========================================================================================================================

static OSStatus	GAITesterAddTestCase( GAITesterRef me, GAITestCase *inCase )
{
	OSStatus			err;
	GAITestCase **		ptr;
	
	require_action_quiet( inCase->itemList, exit, err = kCountErr );
	
	for( ptr = &me->caseList; *ptr; ptr = &( *ptr )->next ) {}
	*ptr = inCase;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	GAITesterSetStopHandler
//===========================================================================================================================

static void	GAITesterSetStopHandler( GAITesterRef me, GAITesterStopHandler_f inStopHandler, void *inStopContext )
{
	me->stopHandler = inStopHandler;
	me->stopContext = inStopContext;
}

//===========================================================================================================================
//	GAITesterSetResultsHandler
//===========================================================================================================================

static void	GAITesterSetResultsHandler( GAITesterRef me, GAITesterResultsHandler_f inResultsHandler, void *inResultsContext )
{
	me->resultsHandler = inResultsHandler;
	me->resultsContext = inResultsContext;
}

//===========================================================================================================================
//	_GAITesterStartNextTest
//===========================================================================================================================

static void	_GAITesterStartNextTest( GAITesterRef me )
{
	OSStatus				err;
	GAITestItem *			item;
	DNSServiceFlags			flags;
	DNSServiceProtocol		protocols;
	int						done = false;
	
	if( me->currentItem ) me->currentItem = me->currentItem->next;
	
	if( !me->currentItem )
	{
		if( me->currentCase )
		{
			// No more test items means that the current test case has completed.
			
			me->caseEndTime = NanoTimeGetCurrent();
			
			if( me->resultsHandler )
			{
				size_t					resultCount, i;
				GAITestItemResult *		resultArray;
				
				resultCount	= 0;
				for( item = me->currentCase->itemList; item; item = item->next ) ++resultCount;
				check( resultCount > 0 );
				
				resultArray = (GAITestItemResult *) calloc( resultCount, sizeof( *resultArray ) );
				require_action( resultArray, exit, err = kNoMemoryErr );
				
				item = me->currentCase->itemList;
				for( i = 0; i < resultCount; ++i )
				{
					resultArray[ i ].name				= item->name;
					resultArray[ i ].connectionTimeUs	= item->connectionTimeUs;
					resultArray[ i ].firstTimeUs		= item->firstTimeUs;
					resultArray[ i ].timeUs				= item->timeUs;
					resultArray[ i ].error				= item->error;
					item = item->next;
				}
				me->resultsHandler( me->currentCase->title, me->caseStartTime, me->caseEndTime, resultArray, resultCount,
					me->resultsContext );
				ForgetMem( &resultArray );
			}
			
			me->currentCase = me->currentCase->next;
			if( !me->currentCase )
			{
				done = true;
				err = kNoErr;
				goto exit;
			}
		}
		else
		{
			me->currentCase = me->caseList;
		}
		require_action_quiet( me->currentCase->itemList, exit, err = kInternalErr );
		me->currentItem = me->currentCase->itemList;
	}
	
	item = me->currentItem;
	check( ( item->addressCount >= 1 ) && ( item->addressCount <= 64 ) );
	
	if(      !item->wantV4 )			me->bitmapV4 = 0;
	else if( !item->hasV4 )				me->bitmapV4 = 1;
	else if(  item->addressCount < 64 )	me->bitmapV4 = ( UINT64_C( 1 ) << item->addressCount ) - 1;
	else								me->bitmapV4 =  ~UINT64_C( 0 );
	
	if(      !item->wantV6 )			me->bitmapV6 = 0;
	else if( !item->hasV6 )				me->bitmapV6 = 1;
	else if(  item->addressCount < 64 )	me->bitmapV6 = ( UINT64_C( 1 ) << item->addressCount ) - 1;
	else								me->bitmapV6 =  ~UINT64_C( 0 );
	check( ( me->bitmapV4 != 0 ) || ( me->bitmapV6 != 0 ) );
	me->gotFirstResult = false;
	
	// Perform preliminary tasks if this is the start of a new test case.
	
	if( item == me->currentCase->itemList )
	{
		// Flush mDNSResponder's cache.
		
		err = systemf( NULL, "killall -HUP mDNSResponder" );
		require_noerr( err, exit );
		sleep( 1 );
		
		me->caseStartTime	= NanoTimeGetCurrent();
		me->caseEndTime		= kNanoTime_Invalid;
	}
	
	// Start a packet capture.
	
	check( !me->pcap );
	err = _GAITesterCreatePacketCapture( &me->pcap );
	require_noerr( err, exit );
	
	// Start timer for test item's time limit.
	
	check( !me->timer );
	if( item->timeLimitMs > 0 )
	{
		unsigned int		timeLimitMs;
		
		timeLimitMs = item->timeLimitMs;
		if( me->callDelayMs   > 0 ) timeLimitMs += (unsigned int) me->callDelayMs;
		if( me->serverDelayMs > 0 ) timeLimitMs += (unsigned int) me->serverDelayMs;
		
		err = DispatchTimerCreate( dispatch_time_milliseconds( timeLimitMs ), DISPATCH_TIME_FOREVER,
			( (uint64_t) timeLimitMs ) * kNanosecondsPerMillisecond / 10,
			me->queue, _GAITesterTimeout, NULL, me, &me->timer );
		require_noerr( err, exit );
		dispatch_resume( me->timer );
	}
	
	// Call DNSServiceGetAddrInfo().
	
	if( me->callDelayMs > 0 ) usleep( ( (useconds_t) me->callDelayMs ) * kMicrosecondsPerMillisecond );
	
	flags = kDNSServiceFlagsShareConnection | kDNSServiceFlagsReturnIntermediates;
	if( me->skipPathEval ) flags |= kDNSServiceFlagsPathEvaluationDone;
	
	protocols = 0;
	if( item->wantV4 ) protocols |= kDNSServiceProtocol_IPv4;
	if( item->wantV6 ) protocols |= kDNSServiceProtocol_IPv6;
	
	me->startTicks = UpTicks();
	
	check( !me->connection );
	err = DNSServiceCreateConnection( &me->connection );
	require_noerr( err, exit );
	
	err = DNSServiceSetDispatchQueue( me->connection, me->queue );
	require_noerr( err, exit );
	
	me->connTicks = UpTicks();
	
	check( !me->getAddrInfo );
	me->getAddrInfo = me->connection;
	err = DNSServiceGetAddrInfo( &me->getAddrInfo, flags, kDNSServiceInterfaceIndexAny, protocols, item->name,
		_GAITesterGetAddrInfoCallback, me );
	require_noerr( err, exit );
	
exit:
	if( err || done ) _GAITesterStop( me, err );
}

//===========================================================================================================================
//	_GAITesterCreatePacketCapture
//===========================================================================================================================

static OSStatus	_GAITesterCreatePacketCapture( pcap_t **outPCap )
{
	OSStatus				err;
	pcap_t *				pcap;
	struct bpf_program		program;
	char					errBuf[ PCAP_ERRBUF_SIZE ];
	
	pcap = pcap_create( "lo0", errBuf );
	require_action_string( pcap, exit, err = kUnknownErr, errBuf );
	
	err = pcap_set_buffer_size( pcap, 512 * kBytesPerKiloByte );
	require_noerr_action( err, exit, err = kUnknownErr );
	
	err = pcap_set_snaplen( pcap, 512 );
	require_noerr_action( err, exit, err = kUnknownErr );
	
	err = pcap_set_immediate_mode( pcap, 0 );
	require_noerr_action_string( err, exit, err = kUnknownErr, pcap_geterr( pcap ) );
	
	err = pcap_activate( pcap );
	require_noerr_action_string( err, exit, err = kUnknownErr, pcap_geterr( pcap ) );
	
	err = pcap_setdirection( pcap, PCAP_D_INOUT );
	require_noerr_action_string( err, exit, err = kUnknownErr, pcap_geterr( pcap ) );
	
	err = pcap_setnonblock( pcap, 1, errBuf );
	require_noerr_action_string( err, exit, err = kUnknownErr, errBuf );
	
	err = pcap_compile( pcap, &program, "udp port 53", 1, PCAP_NETMASK_UNKNOWN );
	require_noerr_action_string( err, exit, err = kUnknownErr, pcap_geterr( pcap ) );
	
	err = pcap_setfilter( pcap, &program );
	pcap_freecode( &program );
	require_noerr_action_string( err, exit, err = kUnknownErr, pcap_geterr( pcap ) );
	
	*outPCap = pcap;
	pcap = NULL;
	
exit:
	if( pcap ) pcap_close( pcap );
	return( err );
}

//===========================================================================================================================
//	_GAITesterFirstGAITimeout
//===========================================================================================================================

static void	_GAITesterFirstGAITimeout( void *inContext )
{
	GAITesterRef const		me = (GAITesterRef) inContext;
	
	_GAITesterStop( me, kNoResourcesErr );
}

//===========================================================================================================================
//	_GAITesterTimeout
//===========================================================================================================================

static void	_GAITesterTimeout( void *inContext )
{
	GAITesterRef const		me = (GAITesterRef) inContext;
	
	_GAITesterCompleteCurrentTest( me, kTimeoutErr );
}

//===========================================================================================================================
//	_GAITesterFirstGAICallback
//===========================================================================================================================

static void DNSSD_API
	_GAITesterFirstGAICallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inHostname,
		const struct sockaddr *	inSockAddr,
		uint32_t				inTTL,
		void *					inContext )
{
	GAITesterRef const		me = (GAITesterRef) inContext;
	
	Unused( inSDRef );
	Unused( inInterfaceIndex );
	Unused( inHostname );
	Unused( inSockAddr );
	Unused( inTTL );
	
	if( ( inFlags & kDNSServiceFlagsAdd ) && !inError )
	{
		dispatch_source_forget( &me->timer );
		DNSServiceForget( &me->getAddrInfo );
		
		_GAITesterStartNextTest( me );
	}
}

//===========================================================================================================================
//	_GAITesterGetAddrInfoCallback
//===========================================================================================================================

static void DNSSD_API
	_GAITesterGetAddrInfoCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inHostname,
		const struct sockaddr *	inSockAddr,
		uint32_t				inTTL,
		void *					inContext )
{
	OSStatus						err;
	GAITesterRef const				me		= (GAITesterRef) inContext;
	GAITestItem * const				item	= me->currentItem;
	const sockaddr_ip * const		sip		= (const sockaddr_ip *) inSockAddr;
	uint64_t						nowTicks;
	uint64_t *						bitmapPtr;
	uint64_t						bitmask;
	int								hasAddr;
	
	Unused( inSDRef );
	Unused( inInterfaceIndex );
	Unused( inHostname );
	Unused( inTTL );
	
	nowTicks = UpTicks();
	
	require_action_quiet( inFlags & kDNSServiceFlagsAdd, exit, err = kFlagErr );
	
	// Check if we were expecting an IP address result of this type.
	
	if( sip->sa.sa_family == AF_INET )
	{
		bitmapPtr	= &me->bitmapV4;
		hasAddr		= item->hasV4;
	}
	else if( sip->sa.sa_family == AF_INET6 )
	{
		bitmapPtr	= &me->bitmapV6;
		hasAddr		= item->hasV6;
	}
	else
	{
		err = kTypeErr;
		goto exit;
	}
	
	bitmask = 0;
	if( hasAddr )
	{
		uint32_t		addrOffset;
		
		require_noerr_action_quiet( inError, exit, err = inError );
		
		if( sip->sa.sa_family == AF_INET )
		{
			const uint32_t		addrV4 = ntohl( sip->v4.sin_addr.s_addr );
			
			if( strcasecmp( item->name, "localhost." ) == 0 )
			{
				if( addrV4 == INADDR_LOOPBACK ) bitmask = 1;
			}
			else
			{
				addrOffset = addrV4 - kDNSServerBaseAddrV4;
				if( ( addrOffset >= 1 ) && ( addrOffset <= item->addressCount ) )
				{
					bitmask = UINT64_C( 1 ) << ( addrOffset - 1 );
				}
			}
		}
		else
		{
			const uint8_t * const		addrV6 = sip->v6.sin6_addr.s6_addr;
			
			if( strcasecmp( item->name, "localhost." ) == 0 )
			{
				if( memcmp( addrV6, in6addr_loopback.s6_addr, 16 ) == 0 ) bitmask = 1;
			}
			else if( memcmp( addrV6, kDNSServerBaseAddrV6, 15 ) == 0 )
			{
				addrOffset = addrV6[ 15 ];
				if( ( addrOffset >= 1 ) && ( addrOffset <= item->addressCount ) )
				{
					bitmask = UINT64_C( 1 ) << ( addrOffset - 1 );
				}
			}
		}
	}
	else
	{
		require_action_quiet( inError == kDNSServiceErr_NoSuchRecord, exit, err = inError ? inError : kUnexpectedErr );
		bitmask = 1;
	}
	require_action_quiet( bitmask != 0, exit, err = kValueErr );
	require_action_quiet( *bitmapPtr & bitmask, exit, err = kDuplicateErr );
	
	*bitmapPtr &= ~bitmask;
	if( !me->gotFirstResult )
	{
		me->firstTicks		= nowTicks;
		me->gotFirstResult	= true;
	}
	err = kNoErr;
	
exit:
	if( err || ( ( me->bitmapV4 == 0 ) && ( me->bitmapV6 == 0 ) ) )
	{
		me->endTicks = nowTicks;
		_GAITesterCompleteCurrentTest( me, err );
	}
}

//===========================================================================================================================
//	_GAITesterCompleteCurrentTest
//===========================================================================================================================

static OSStatus
	_GAITesterGetDNSMessageFromPacket(
		const uint8_t *		inPacketPtr,
		size_t				inPacketLen,
		const uint8_t **	outMsgPtr,
		size_t *			outMsgLen );

static void	_GAITesterCompleteCurrentTest( GAITesterRef me, OSStatus inError )
{
	OSStatus				err;
	GAITestItem * const		item	= me->currentItem;
	struct timeval			timeStamps[ 4 ];
	struct timeval *		tsPtr;
	struct timeval *		tsQA	= NULL;
	struct timeval *		tsQAAAA	= NULL;
	struct timeval *		tsRA	= NULL;
	struct timeval *		tsRAAAA	= NULL;
	struct timeval *		t1;
	struct timeval *		t2;
	int64_t					idleTimeUs;
	uint8_t					name[ kDomainNameLengthMax ];
	
	dispatch_source_forget( &me->timer );
	DNSServiceForget( &me->getAddrInfo );
	DNSServiceForget( &me->connection );
	
	item->error = inError;
	if( item->error )
	{
		err = kNoErr;
		goto exit;
	}
	
	err = DomainNameFromString( name, item->name, NULL );
	require_noerr( err, exit );
	
	tsPtr = &timeStamps[ 0 ];
	for( ;; )
	{
		int							status;
		struct pcap_pkthdr *		pktHdr;
		const uint8_t *				packet;
		const uint8_t *				msgPtr;
		size_t						msgLen;
		const DNSHeader *			hdr;
		unsigned int				flags;
		const uint8_t *				ptr;
		uint16_t					qtype, qclass;
		uint8_t						qname[ kDomainNameLengthMax ];
		
		status = pcap_next_ex( me->pcap, &pktHdr, &packet );
		if( status != 1 ) break;
		if( _GAITesterGetDNSMessageFromPacket( packet, pktHdr->caplen, &msgPtr, &msgLen ) != kNoErr ) continue;
		if( msgLen < kDNSHeaderLength ) continue;
		
		hdr = (const DNSHeader *) msgPtr;
		flags = DNSHeaderGetFlags( hdr );
		if( DNSFlagsGetOpCode( flags ) != kDNSOpCode_Query ) continue;
		if( DNSHeaderGetQuestionCount( hdr ) < 1 ) continue;
		
		ptr = (const uint8_t *) &hdr[ 1 ];
		if( DNSMessageExtractQuestion( msgPtr, msgLen, ptr, qname, &qtype, &qclass, NULL ) != kNoErr ) continue;
		if( qclass != kDNSServiceClass_IN ) continue;
		if( !DomainNameEqual( qname, name ) ) continue;
		
		if( item->wantV4 && ( qtype == kDNSServiceType_A ) )
		{
			if( flags & kDNSHeaderFlag_Response )
			{
				if( tsQA && !tsRA )
				{
					tsRA  = tsPtr++;
					*tsRA = pktHdr->ts;
				}
			}
			else if( !tsQA )
			{
				tsQA  = tsPtr++;
				*tsQA = pktHdr->ts;
			}
		}
		else if( item->wantV6 && ( qtype == kDNSServiceType_AAAA ) )
		{
			if( flags & kDNSHeaderFlag_Response )
			{
				if( tsQAAAA && !tsRAAAA )
				{
					tsRAAAA  = tsPtr++;
					*tsRAAAA = pktHdr->ts;
				}
			}
			else if( !tsQAAAA )
			{
				tsQAAAA  = tsPtr++;
				*tsQAAAA = pktHdr->ts;
			}
		}
	}
	
	// t1 is the time when the last query was sent.
	
	if( tsQA && tsQAAAA )	t1 = TIMEVAL_GT( *tsQA, *tsQAAAA ) ? tsQA : tsQAAAA;
	else					t1 = tsQA ? tsQA : tsQAAAA;
	
	// t2 is when the first response was received.
	
	if( tsRA && tsRAAAA )	t2 = TIMEVAL_LT( *tsRA, *tsRAAAA ) ? tsRA : tsRAAAA;
	else					t2 = tsRA ? tsRA : tsRAAAA;
	
	if( t1 && t2 )
	{
		idleTimeUs = TIMEVAL_USEC64_DIFF( *t2, *t1 );
		if( idleTimeUs < 0 ) idleTimeUs = 0;
	}
	else
	{
		idleTimeUs = 0;
	}
	
	item->connectionTimeUs	= UpTicksToMicroseconds( me->connTicks  - me->startTicks );
	item->firstTimeUs		= UpTicksToMicroseconds( me->firstTicks - me->connTicks  ) - (uint64_t) idleTimeUs;
	item->timeUs			= UpTicksToMicroseconds( me->endTicks   - me->connTicks  ) - (uint64_t) idleTimeUs;
	
exit:
	ForgetPacketCapture( &me->pcap );
	if( err )	_GAITesterStop( me, err );
	else		_GAITesterStartNextTest( me );
}

//===========================================================================================================================
//	_GAITesterGetDNSMessageFromPacket
//===========================================================================================================================

#define kHeaderSizeNullLink		 4
#define kHeaderSizeIPv4Min		20
#define kHeaderSizeIPv6			40
#define kHeaderSizeUDP			 8

#define kIPProtocolUDP		0x11

static OSStatus
	_GAITesterGetDNSMessageFromPacket(
		const uint8_t *		inPacketPtr,
		size_t				inPacketLen,
		const uint8_t **	outMsgPtr,
		size_t *			outMsgLen )
{
	OSStatus					err;
	const uint8_t *				nullLink;
	uint32_t					addressFamily;
	const uint8_t *				ip;
	int							ipHeaderLen;
	int							protocol;
	const uint8_t *				msg;
	const uint8_t * const		end = &inPacketPtr[ inPacketLen ];
	
	nullLink = &inPacketPtr[ 0 ];
	require_action_quiet( ( end - nullLink ) >= kHeaderSizeNullLink, exit, err = kUnderrunErr );
	addressFamily = ReadHost32( &nullLink[ 0 ] );
	
	ip = &nullLink[ kHeaderSizeNullLink ];
	if( addressFamily == AF_INET )
	{
		require_action_quiet( ( end - ip ) >= kHeaderSizeIPv4Min, exit, err = kUnderrunErr );
		ipHeaderLen	= ( ip[ 0 ] & 0x0F ) * 4;
		protocol	=   ip[ 9 ];
	}
	else if( addressFamily == AF_INET6 )
	{
		require_action_quiet( ( end - ip ) >= kHeaderSizeIPv6, exit, err = kUnderrunErr );
		ipHeaderLen	= kHeaderSizeIPv6;
		protocol	= ip[ 6 ];
	}
	else
	{
		err = kTypeErr;
		goto exit;
	}
	require_action_quiet( protocol == kIPProtocolUDP, exit, err = kTypeErr );
	require_action_quiet( ( end - ip ) >= ( ipHeaderLen + kHeaderSizeUDP ), exit, err = kUnderrunErr );
	
	msg = &ip[ ipHeaderLen + kHeaderSizeUDP ];
	
	*outMsgPtr = msg;
	*outMsgLen = (size_t)( end - msg );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	GAITestCaseCreate
//===========================================================================================================================

static OSStatus	GAITestCaseCreate( const char *inTitle, GAITestCase **outCase )
{
	OSStatus			err;
	GAITestCase *		obj;
	
	obj = (GAITestCase *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	obj->title = strdup( inTitle );
	require_action( obj->title, exit, err = kNoMemoryErr );
	
	*outCase = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	if( obj ) GAITestCaseFree( obj );
	return( err );
}

//===========================================================================================================================
//	GAITestCaseFree
//===========================================================================================================================

static void	GAITestCaseFree( GAITestCase *inCase )
{
	GAITestItem *		item;
	
	while( ( item = inCase->itemList ) != NULL )
	{
		inCase->itemList = item->next;
		GAITestItemFree( item );
	}
	ForgetMem( &inCase->title );
	free( inCase );
}

//===========================================================================================================================
//	GAITestCaseAddItem
//===========================================================================================================================

static OSStatus
	GAITestCaseAddItem(
		GAITestCase *	inCase,
		unsigned int	inAliasCount,
		unsigned int	inAddressCount,
		int				inTTL,
		GAITestAddrType	inHasAddrs,
		GAITestAddrType	inWantAddrs,
		unsigned int	inTimeLimitMs,
		unsigned int	inItemCount )
{
	OSStatus			err;
	GAITestItem *		item;
	GAITestItem *		item2;
	GAITestItem *		newItemList = NULL;
	GAITestItem **		itemPtr;
	char *				ptr;
	char *				end;
	unsigned int		i;
	char				name[ 64 ];
	char				tag[ kGAITesterTagStringLen + 1 ];
	
	require_action_quiet( inItemCount > 0, exit, err = kNoErr );
	
	// Limit address count to 64 because we use 64-bit bitmaps for keeping track of addresses.
	
	require_action_quiet( ( inAddressCount >= 1 ) && ( inAddressCount <= 64 ), exit, err = kCountErr );
	require_action_quiet( ( inAliasCount >= 0 ) && ( inAliasCount <= INT32_MAX ), exit, err = kCountErr );
	require_action_quiet( GAITestAddrTypeIsValid( inHasAddrs ), exit, err = kValueErr );
	
	ptr = &name[ 0 ];
	end = &name[ countof( name ) ];
	
	// Add Alias label.
	
	if(      inAliasCount == 1 ) SNPrintF_Add( &ptr, end, "alias." );
	else if( inAliasCount >= 2 ) SNPrintF_Add( &ptr, end, "alias-%u.", inAliasCount );
	
	// Add Count label.
	
	SNPrintF_Add( &ptr, end, "count-%u.", inAddressCount );
	
	// Add TTL label.
	
	if( inTTL >= 0 ) SNPrintF_Add( &ptr, end, "ttl-%d.", inTTL );
	
	// Add Tag label.
	
	SNPrintF_Add( &ptr, end, "tag-%s.",
		_RandomStringExact( kLowerAlphaNumericCharSet, kLowerAlphaNumericCharSetSize, sizeof( tag ) - 1, tag ) );
	
	// Add IPv4 or IPv6 label if necessary.
	
	if(      inHasAddrs == kGAITestAddrType_IPv4 ) SNPrintF_Add( &ptr, end, "ipv4." );
	else if( inHasAddrs == kGAITestAddrType_IPv6 ) SNPrintF_Add( &ptr, end, "ipv6." );
	
	// Finally, add the d.test. labels.
	
	SNPrintF_Add( &ptr, end, "d.test." );
	
	// Create item.
	
	err = GAITestItemCreate( name, inAddressCount, inHasAddrs, inWantAddrs, inTimeLimitMs, &item );
	require_noerr( err, exit );
	
	newItemList	= item;
	itemPtr		= &item->next;
	
	// Create repeat items.
	
	for( i = 1; i < inItemCount; ++i )
	{
		err = GAITestItemDup( item, &item2 );
		require_noerr( err, exit );
		
		*itemPtr	= item2;
		itemPtr		= &item2->next;
	}
	
	// Append to test case's item list.
	
	for( itemPtr = &inCase->itemList; *itemPtr; itemPtr = &( *itemPtr )->next ) {}
	*itemPtr	= newItemList;
	newItemList	= NULL;
	
exit:
	while( ( item = newItemList ) != NULL )
	{
		newItemList = item->next;
		GAITestItemFree( item );
	}
	return( err );
}

//===========================================================================================================================
//	GAITestCaseAddLocalHostItem
//===========================================================================================================================

static OSStatus
	GAITestCaseAddLocalHostItem(
		GAITestCase *	inCase,
		GAITestAddrType	inWantAddrs,
		unsigned int	inTimeLimitMs,
		unsigned int	inItemCount )
{
	OSStatus			err;
	GAITestItem *		item;
	GAITestItem *		item2;
	GAITestItem *		newItemList = NULL;
	GAITestItem **		itemPtr;
	unsigned int		i;
	
	require_action_quiet( inItemCount > 1, exit, err = kNoErr );
	
	err = GAITestItemCreate( "localhost.", 1, kGAITestAddrType_Both, inWantAddrs, inTimeLimitMs, &item );
	require_noerr( err, exit );
	
	newItemList	= item;
	itemPtr		= &item->next;
	
	// Create repeat items.
	
	for( i = 1; i < inItemCount; ++i )
	{
		err = GAITestItemDup( item, &item2 );
		require_noerr( err, exit );
		
		*itemPtr	= item2;
		itemPtr		= &item2->next;
	}
	
	for( itemPtr = &inCase->itemList; *itemPtr; itemPtr = &( *itemPtr )->next ) {}
	*itemPtr	= newItemList;
	newItemList	= NULL;
	
exit:
	while( ( item = newItemList ) != NULL )
	{
		newItemList = item->next;
		GAITestItemFree( item );
	}
	return( err );
}

//===========================================================================================================================
//	GAITestItemCreate
//===========================================================================================================================

static OSStatus
	GAITestItemCreate(
		const char *	inName,
		unsigned int	inAddressCount,
		GAITestAddrType	inHasAddrs,
		GAITestAddrType	inWantAddrs,
		unsigned int	inTimeLimitMs,
		GAITestItem **	outItem )
{
	OSStatus			err;
	GAITestItem *		obj = NULL;
	
	require_action_quiet( inAddressCount >= 1, exit, err = kCountErr );
	require_action_quiet( GAITestAddrTypeIsValid( inHasAddrs ), exit, err = kValueErr );
	require_action_quiet( GAITestAddrTypeIsValid( inWantAddrs ), exit, err = kValueErr );
	
	obj = (GAITestItem *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	obj->name = strdup( inName );
	require_action( obj->name, exit, err = kNoMemoryErr );
	
	obj->addressCount	= inAddressCount;
	obj->hasV4			= ( inHasAddrs  & kGAITestAddrType_IPv4 ) ? true : false;
	obj->hasV6			= ( inHasAddrs  & kGAITestAddrType_IPv6 ) ? true : false;
	obj->wantV4			= ( inWantAddrs & kGAITestAddrType_IPv4 ) ? true : false;
	obj->wantV6			= ( inWantAddrs & kGAITestAddrType_IPv6 ) ? true : false;
	obj->error			= kInProgressErr;
	obj->timeLimitMs	= inTimeLimitMs;
	
	*outItem = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	if( obj ) GAITestItemFree( obj );
	return( err );
}

//===========================================================================================================================
//	GAITestItemDup
//===========================================================================================================================

static OSStatus	GAITestItemDup( const GAITestItem *inItem, GAITestItem **outItem )
{
	OSStatus			err;
	GAITestItem *		obj;
	
	obj = (GAITestItem *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	*obj = *inItem;
	obj->next = NULL;
	if( inItem->name )
	{
		obj->name = strdup( inItem->name );
		require_action( obj->name, exit, err = kNoMemoryErr );
	}
	
	*outItem = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	if( obj ) GAITestItemFree( obj );
	return( err );
}

//===========================================================================================================================
//	GAITestItemFree
//===========================================================================================================================

static void	GAITestItemFree( GAITestItem *inItem )
{
	ForgetMem( &inItem->name );
	free( inItem );
}

//===========================================================================================================================
//	MDNSDiscoveryTestCmd
//===========================================================================================================================

#define kMDNSDiscoveryTestFirstQueryTimeoutSecs		4

typedef struct
{
	DNSServiceRef			query;					// Reference to DNSServiceQueryRecord for replier's "about" TXT record.
	dispatch_source_t		queryTimer;				// Used to time out the "about" TXT record query.
	NanoTime64				startTime;				// When the test started.
	NanoTime64				endTime;				// When the test ended.
	pid_t					replierPID;				// PID of mDNS replier.
	uint32_t				ifIndex;				// Index of interface to run the replier on.
	unsigned int			instanceCount;			// Desired number of service instances.
	unsigned int			txtSize;				// Desired size of each service instance's TXT record data.
	unsigned int			recordCountA;			// Desired number of A records per replier hostname.
	unsigned int			recordCountAAAA;		// Desired number of AAAA records per replier hostname.
	unsigned int			maxDropCount;			// Replier's --maxDropCount option argument.
	double					ucastDropRate;			// Replier's probability of dropping a unicast response.
	double					mcastDropRate;			// Replier's probability of dropping a multicast query or response.
	Boolean					noAdditionals;			// True if the replier is to not include additional records in responses.
	Boolean					useIPv4;				// True if the replier is to use IPv4.
	Boolean					useIPv6;				// True if the replier is to use IPv6.
	Boolean					flushedCache;			// True if mDNSResponder's record cache was flushed before testing.
	char *					replierCommand;			// Command used to run the replier.
	char *					serviceType;			// Type of services to browse for.
	ServiceBrowserRef		browser;				// Service browser.
	unsigned int			browseTimeSecs;			// Amount of time to spend browsing in seconds.
	const char *			outputFilePath;			// File to write test results to. If NULL, then write to stdout.
	OutputFormatType		outputFormat;			// Format of test results output.
	Boolean					outputAppendNewline;	// True if a newline character should be appended to JSON output.
	char					hostname[ 32 + 1 ];		// Base hostname that the replier is to use for instance and host names.
	char					tag[ 4 + 1 ];			// Tag that the replier is to use in its service types.
	
}	MDNSDiscoveryTestContext;

static void		_MDNSDiscoveryTestFirstQueryTimeout( void *inContext );
static void DNSSD_API
	_MDNSDiscoveryTestAboutQueryCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		uint16_t				inType,
		uint16_t				inClass,
		uint16_t				inRDataLen,
		const void *			inRDataPtr,
		uint32_t				inTTL,
		void *					inContext );
static void
	_MDNSDiscoveryTestServiceBrowserCallback(
		ServiceBrowserResults *	inResults,
		OSStatus				inError,
		void *					inContext );
static Boolean	_MDNSDiscoveryTestTXTRecordIsValid( const uint8_t *inRecordName, const uint8_t *inTXTPtr, size_t inTXTLen );

static void	MDNSDiscoveryTestCmd( void )
{
	OSStatus						err;
	MDNSDiscoveryTestContext *		context;
	char							queryName[ sizeof_field( MDNSDiscoveryTestContext, hostname ) + 15 ];
	
	context = (MDNSDiscoveryTestContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	err = CheckIntegerArgument( gMDNSDiscoveryTest_InstanceCount, "instance count", 1, UINT16_MAX );
	require_noerr_quiet( err, exit );
	
	err = CheckIntegerArgument( gMDNSDiscoveryTest_TXTSize, "TXT size", 1, UINT16_MAX );
	require_noerr_quiet( err, exit );
	
	err = CheckIntegerArgument( gMDNSDiscoveryTest_BrowseTimeSecs, "browse time (seconds)", 1, INT_MAX );
	require_noerr_quiet( err, exit );
	
	err = CheckIntegerArgument( gMDNSDiscoveryTest_RecordCountA, "A record count", 0, 64 );
	require_noerr_quiet( err, exit );
	
	err = CheckIntegerArgument( gMDNSDiscoveryTest_RecordCountAAAA, "AAAA record count", 0, 64 );
	require_noerr_quiet( err, exit );
	
	err = CheckDoubleArgument( gMDNSDiscoveryTest_UnicastDropRate, "unicast drop rate", 0.0, 1.0 );
	require_noerr_quiet( err, exit );
	
	err = CheckDoubleArgument( gMDNSDiscoveryTest_MulticastDropRate, "multicast drop rate", 0.0, 1.0 );
	require_noerr_quiet( err, exit );
	
	err = CheckIntegerArgument( gMDNSDiscoveryTest_MaxDropCount, "drop count", 0, 255 );
	require_noerr_quiet( err, exit );
	
	context->replierPID				= -1;
	context->instanceCount			= (unsigned int) gMDNSDiscoveryTest_InstanceCount;
	context->txtSize				= (unsigned int) gMDNSDiscoveryTest_TXTSize;
	context->browseTimeSecs			= (unsigned int) gMDNSDiscoveryTest_BrowseTimeSecs;
	context->recordCountA			= (unsigned int) gMDNSDiscoveryTest_RecordCountA;
	context->recordCountAAAA		= (unsigned int) gMDNSDiscoveryTest_RecordCountAAAA;
	context->ucastDropRate			= gMDNSDiscoveryTest_UnicastDropRate;
	context->mcastDropRate			= gMDNSDiscoveryTest_MulticastDropRate;
	context->maxDropCount			= (unsigned int) gMDNSDiscoveryTest_MaxDropCount;
	context->outputFilePath			= gMDNSDiscoveryTest_OutputFilePath;
	context->outputAppendNewline	= gMDNSDiscoveryTest_OutputAppendNewline ? true : false;
	context->noAdditionals			= gMDNSDiscoveryTest_NoAdditionals       ? true : false;
	context->useIPv4				= ( gMDNSDiscoveryTest_UseIPv4 || !gMDNSDiscoveryTest_UseIPv6 ) ? true : false;
	context->useIPv6				= ( gMDNSDiscoveryTest_UseIPv6 || !gMDNSDiscoveryTest_UseIPv4 ) ? true : false;
	
	if( gMDNSDiscoveryTest_Interface )
	{
		err = InterfaceIndexFromArgString( gMDNSDiscoveryTest_Interface, &context->ifIndex );
		require_noerr_quiet( err, exit );
	}
	else
	{
		err = _MDNSInterfaceGetAny( kMDNSInterfaceSubset_All, NULL, &context->ifIndex );
		require_noerr_quiet( err, exit );
	}
	
	err = OutputFormatFromArgString( gMDNSDiscoveryTest_OutputFormat, &context->outputFormat );
	require_noerr_quiet( err, exit );
	
	if( gMDNSDiscoveryTest_FlushCache )
	{
		err = CheckRootUser();
		require_noerr_quiet( err, exit );
		
		err = systemf( NULL, "killall -HUP mDNSResponder" );
		require_noerr( err, exit );
		sleep( 1 );
		context->flushedCache = true;
	}
	
	_RandomStringExact( kLowerAlphaNumericCharSet, kLowerAlphaNumericCharSetSize, sizeof( context->hostname ) - 1,
		context->hostname );
	_RandomStringExact( kLowerAlphaNumericCharSet, kLowerAlphaNumericCharSetSize, sizeof( context->tag ) - 1, context->tag );
	
	ASPrintF( &context->serviceType, "_t-%s-%u-%u._tcp", context->tag, context->txtSize, context->instanceCount );
	require_action( context->serviceType, exit, err = kUnknownErr );
	
	ASPrintF( &context->replierCommand,
		"dnssdutil mdnsreplier --follow %lld --interface %u --hostname %s --tag %s --maxInstanceCount %u "
		"--countA %u --countAAAA %u --udrop %.1f --mdrop %.1f --maxDropCount %u %?s%?s%?s",
		(int64_t) getpid(),
		context->ifIndex,
		context->hostname,
		context->tag,
		context->instanceCount,
		context->recordCountA,
		context->recordCountAAAA,
		context->ucastDropRate,
		context->mcastDropRate,
		context->maxDropCount,
		context->noAdditionals,	" --noAdditionals",
		context->useIPv4,		" --ipv4",
		context->useIPv6,		" --ipv6" );
	require_action_quiet( context->replierCommand, exit, err = kUnknownErr );
	
	err = SpawnCommand( &context->replierPID, "%s", context->replierCommand );
	require_noerr_quiet( err, exit );
	
	// Query for the replier's about TXT record. A response means it's fully up and running.
	
	SNPrintF( queryName, sizeof( queryName ), "about.%s.local.", context->hostname );
	err = DNSServiceQueryRecord( &context->query, kDNSServiceFlagsForceMulticast, context->ifIndex, queryName,
		kDNSServiceType_TXT, kDNSServiceClass_IN, _MDNSDiscoveryTestAboutQueryCallback, context );
	require_noerr( err, exit );
	
	err = DNSServiceSetDispatchQueue( context->query, dispatch_get_main_queue() );
	require_noerr( err, exit );
	
	err = DispatchTimerCreate( dispatch_time_seconds( kMDNSDiscoveryTestFirstQueryTimeoutSecs ),
		DISPATCH_TIME_FOREVER, UINT64_C_safe( kMDNSDiscoveryTestFirstQueryTimeoutSecs ) * kNanosecondsPerSecond / 10, NULL,
		_MDNSDiscoveryTestFirstQueryTimeout, NULL, context, &context->queryTimer );
	require_noerr( err, exit );
	dispatch_resume( context->queryTimer );
	
	context->startTime = NanoTimeGetCurrent();
	dispatch_main();
	
exit:
	exit( 1 );
}

//===========================================================================================================================
//	_MDNSDiscoveryTestFirstQueryTimeout
//===========================================================================================================================

static void	_MDNSDiscoveryTestFirstQueryTimeout( void *inContext )
{
	MDNSDiscoveryTestContext * const		context = (MDNSDiscoveryTestContext *) inContext;
	
	dispatch_source_forget( &context->queryTimer );
	
	FPrintF( stderr, "error: Query for mdnsreplier's \"about\" TXT record timed out.\n" );
	exit( 1 );
}

//===========================================================================================================================
//	_MDNSDiscoveryTestAboutQueryCallback
//===========================================================================================================================

static void DNSSD_API
	_MDNSDiscoveryTestAboutQueryCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		uint16_t				inType,
		uint16_t				inClass,
		uint16_t				inRDataLen,
		const void *			inRDataPtr,
		uint32_t				inTTL,
		void *					inContext )
{
	OSStatus								err;
	MDNSDiscoveryTestContext * const		context = (MDNSDiscoveryTestContext *) inContext;
	
	Unused( inSDRef );
	Unused( inInterfaceIndex );
	Unused( inFullName );
	Unused( inType );
	Unused( inClass );
	Unused( inRDataLen );
	Unused( inRDataPtr );
	Unused( inTTL );
	
	err = inError;
	require_noerr( err, exit );
	require_quiet( inFlags & kDNSServiceFlagsAdd, exit );
	
	DNSServiceForget( &context->query );
	dispatch_source_forget( &context->queryTimer );
	
	err = ServiceBrowserCreate( dispatch_get_main_queue(), 0, "local.", context->browseTimeSecs, false, &context->browser );
	require_noerr( err, exit );
	
	err = ServiceBrowserAddServiceType( context->browser, context->serviceType );
	require_noerr( err, exit );
	
	ServiceBrowserSetCallback( context->browser, _MDNSDiscoveryTestServiceBrowserCallback, context );
	ServiceBrowserStart( context->browser );
	
exit:
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	_MDNSDiscoveryTestServiceBrowserCallback
//===========================================================================================================================

#define kMDNSDiscoveryTestResultsKey_ReplierInfo					CFSTR( "replierInfo" )
#define kMDNSDiscoveryTestResultsKey_StartTime						CFSTR( "startTime" )
#define kMDNSDiscoveryTestResultsKey_EndTime						CFSTR( "endTime" )
#define kMDNSDiscoveryTestResultsKey_BrowseTimeSecs					CFSTR( "browseTimeSecs" )
#define kMDNSDiscoveryTestResultsKey_ServiceType					CFSTR( "serviceType" )
#define kMDNSDiscoveryTestResultsKey_FlushedCache					CFSTR( "flushedCache" )
#define kMDNSDiscoveryTestResultsKey_UnexpectedInstances			CFSTR( "unexpectedInstances" )
#define kMDNSDiscoveryTestResultsKey_MissingInstances				CFSTR( "missingInstances" )
#define kMDNSDiscoveryTestResultsKey_IncorrectInstances				CFSTR( "incorrectInstances" )
#define kMDNSDiscoveryTestResultsKey_Success						CFSTR( "success" )
#define kMDNSDiscoveryTestResultsKey_TotalResolveTime				CFSTR( "totalResolveTimeUs" )

#define kMDNSDiscoveryTestReplierInfoKey_Command					CFSTR( "command" )
#define kMDNSDiscoveryTestReplierInfoKey_InstanceCount				CFSTR( "instanceCount" )
#define kMDNSDiscoveryTestReplierInfoKey_TXTSize					CFSTR( "txtSize" )
#define kMDNSDiscoveryTestReplierInfoKey_RecordCountA				CFSTR( "recordCountA" )
#define kMDNSDiscoveryTestReplierInfoKey_RecordCountAAAA			CFSTR( "recordCountAAAA" )
#define kMDNSDiscoveryTestReplierInfoKey_Hostname					CFSTR( "hostname" )
#define kMDNSDiscoveryTestReplierInfoKey_NoAdditionals				CFSTR( "noAdditionals" )
#define kMDNSDiscoveryTestReplierInfoKey_UnicastDropRate			CFSTR( "ucastDropRate" )
#define kMDNSDiscoveryTestReplierInfoKey_MulticastDropRate			CFSTR( "mcastDropRate" )
#define kMDNSDiscoveryTestReplierInfoKey_MaxDropCount				CFSTR( "maxDropCount" )

#define kMDNSDiscoveryTestUnexpectedInstanceKey_Name				CFSTR( "name" )
#define kMDNSDiscoveryTestUnexpectedInstanceKey_InterfaceIndex		CFSTR( "interfaceIndex" )

#define kMDNSDiscoveryTestIncorrectInstanceKey_Name					CFSTR( "name" )
#define kMDNSDiscoveryTestIncorrectInstanceKey_DidResolve			CFSTR( "didResolve" )
#define kMDNSDiscoveryTestIncorrectInstanceKey_BadHostname			CFSTR( "badHostname" )
#define kMDNSDiscoveryTestIncorrectInstanceKey_BadPort				CFSTR( "badPort" )
#define kMDNSDiscoveryTestIncorrectInstanceKey_BadTXT				CFSTR( "badTXT" )
#define kMDNSDiscoveryTestIncorrectInstanceKey_UnexpectedAddrs		CFSTR( "unexpectedAddrs" )
#define kMDNSDiscoveryTestIncorrectInstanceKey_MissingAddrs			CFSTR( "missingAddrs" )

static void	_MDNSDiscoveryTestServiceBrowserCallback( ServiceBrowserResults *inResults, OSStatus inError, void *inContext )
{
	OSStatus								err;
	MDNSDiscoveryTestContext * const		context			= (MDNSDiscoveryTestContext *) inContext;
	const SBRDomain *						domain;
	const SBRServiceType *					type;
	const SBRServiceInstance *				instance;
	const SBRServiceInstance **				instanceArray	= NULL;
	const SBRIPAddress *					ipaddr;
	size_t									hostnameLen;
	const char *							ptr;
	const char *							end;
	unsigned int							i;
	uint32_t								u32;
	CFMutableArrayRef						unexpectedInstances;
	CFMutableArrayRef						missingInstances;
	CFMutableArrayRef						incorrectInstances;
	CFMutableDictionaryRef					plist			= NULL;
	CFMutableDictionaryRef					badDict			= NULL;
	CFMutableArrayRef						unexpectedAddrs	= NULL;
	CFMutableArrayRef						missingAddrs	= NULL;
	uint64_t								maxResolveTimeUs;
	int										success			= false;
	char									startTime[ 32 ];
	char									endTime[ 32 ];
	
	context->endTime = NanoTimeGetCurrent();
	
	err = inError;
	require_noerr( err, exit );
	
	_NanoTime64ToTimestamp( context->startTime, startTime, sizeof( startTime ) );
	_NanoTime64ToTimestamp( context->endTime, endTime, sizeof( endTime ) );
	err = CFPropertyListCreateFormatted( kCFAllocatorDefault, &plist,
		"{"
			"%kO="
			"{"
				"%kO=%s"	// replierCommand
				"%kO=%lli"	// txtSize
				"%kO=%lli"	// instanceCount
				"%kO=%lli"	// recordCountA
				"%kO=%lli"	// recordCountAAAA
				"%kO=%s"	// hostname
				"%kO=%b"	// noAdditionals
				"%kO=%f"	// ucastDropRate
				"%kO=%f"	// mcastDropRate
				"%kO=%i"	// maxDropCount
			"}"
			"%kO=%s"	// startTime
			"%kO=%s"	// endTime
			"%kO=%lli"	// browseTimeSecs
			"%kO=%s"	// serviceType
			"%kO=%b"	// flushedCache
			"%kO=[%@]"	// unexpectedInstances
			"%kO=[%@]"	// missingInstances
			"%kO=[%@]"	// incorrectInstances
		"}",
		kMDNSDiscoveryTestResultsKey_ReplierInfo,
		kMDNSDiscoveryTestReplierInfoKey_Command,			context->replierCommand,
		kMDNSDiscoveryTestReplierInfoKey_InstanceCount,		(int64_t) context->instanceCount,
		kMDNSDiscoveryTestReplierInfoKey_TXTSize,			(int64_t) context->txtSize,
		kMDNSDiscoveryTestReplierInfoKey_RecordCountA,		(int64_t) context->recordCountA,
		kMDNSDiscoveryTestReplierInfoKey_RecordCountAAAA,	(int64_t) context->recordCountAAAA,
		kMDNSDiscoveryTestReplierInfoKey_Hostname,			context->hostname,
		kMDNSDiscoveryTestReplierInfoKey_NoAdditionals,		context->noAdditionals,
		kMDNSDiscoveryTestReplierInfoKey_UnicastDropRate,	context->ucastDropRate,
		kMDNSDiscoveryTestReplierInfoKey_MulticastDropRate,	context->mcastDropRate,
		kMDNSDiscoveryTestReplierInfoKey_MaxDropCount,		context->maxDropCount,
		kMDNSDiscoveryTestResultsKey_StartTime,				startTime,
		kMDNSDiscoveryTestResultsKey_EndTime,				endTime,
		kMDNSDiscoveryTestResultsKey_BrowseTimeSecs,		(int64_t) context->browseTimeSecs,
		kMDNSDiscoveryTestResultsKey_ServiceType,			context->serviceType,
		kMDNSDiscoveryTestResultsKey_FlushedCache,			context->flushedCache,
		kMDNSDiscoveryTestResultsKey_UnexpectedInstances,	&unexpectedInstances,
		kMDNSDiscoveryTestResultsKey_MissingInstances,		&missingInstances,
		kMDNSDiscoveryTestResultsKey_IncorrectInstances,	&incorrectInstances );
	require_noerr( err, exit );
	
	for( domain = inResults->domainList; domain && ( strcasecmp( domain->name, "local." ) != 0 ); domain = domain->next ) {}
	require_action( domain, exit, err = kInternalErr );
	
	for( type = domain->typeList; type && ( strcasecmp( type->name, context->serviceType ) != 0 ); type = type->next ) {}
	require_action( type, exit, err = kInternalErr );
	
	instanceArray = (const SBRServiceInstance **) calloc( context->instanceCount, sizeof( *instanceArray ) );
	require_action( instanceArray, exit, err = kNoMemoryErr );
	
	hostnameLen = strlen( context->hostname );
	for( instance = type->instanceList; instance; instance = instance->next )
	{
		unsigned int		instanceNumber = 0;
		
		if( strcmp_prefix( instance->name, context->hostname ) == 0 )
		{
			ptr = &instance->name[ hostnameLen ];
			if( ( ptr[ 0 ] == ' ' ) && ( ptr[ 1 ] == '(' ) )
			{
				ptr += 2;
				for( end = ptr; isdigit_safe( *end ); ++end ) {}
				if( DecimalTextToUInt32( ptr, end, &u32, &ptr ) == kNoErr )
				{
					if( ( u32 >= 2 ) && ( u32 <= context->instanceCount ) && ( ptr[ 0 ] == ')' ) && ( ptr[ 1 ] == '\0' ) )
					{
						instanceNumber = u32;
					}
				}
			}
			else if( *ptr == '\0' )
			{
				instanceNumber = 1;
			}
		}
		if( ( instanceNumber != 0 ) && ( instance->ifIndex == context->ifIndex ) )
		{
			check( !instanceArray[ instanceNumber - 1 ] );
			instanceArray[ instanceNumber - 1 ] = instance;
		}
		else
		{
			err = CFPropertyListAppendFormatted( kCFAllocatorDefault, unexpectedInstances,
				"{"
					"%kO=%s"
					"%kO=%lli"
				"}",
				kMDNSDiscoveryTestUnexpectedInstanceKey_Name,			instance->name,
				kMDNSDiscoveryTestUnexpectedInstanceKey_InterfaceIndex,	(int64_t) instance->ifIndex );
			require_noerr( err, exit );
		}
	}
	
	maxResolveTimeUs = 0;
	for( i = 1; i <= context->instanceCount; ++i )
	{
		int		isHostnameValid;
		int		isTXTValid;
		
		instance = instanceArray[ i - 1 ];
		if( !instance )
		{
			if( i == 1 )
			{
				err = CFPropertyListAppendFormatted( kCFAllocatorDefault, missingInstances, "%s", context->hostname );
				require_noerr( err, exit );
			}
			else
			{
				char *		instanceName = NULL;
				
				ASPrintF( &instanceName, "%s (%u)", context->hostname, i );
				require_action( instanceName, exit, err = kUnknownErr );
				
				err = CFPropertyListAppendFormatted( kCFAllocatorDefault, missingInstances, "%s", instanceName );
				free( instanceName );
				require_noerr( err, exit );
			}
			continue;
		}
		
		if( !instance->hostname )
		{
			err = CFPropertyListAppendFormatted( kCFAllocatorDefault, incorrectInstances,
				"{"
					"%kO=%s"
					"%kO=%b"
				"}",
				kMDNSDiscoveryTestIncorrectInstanceKey_Name,		instance->name,
				kMDNSDiscoveryTestIncorrectInstanceKey_DidResolve,	false );
			require_noerr( err, exit );
			continue;
		}
		
		badDict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		require_action( badDict, exit, err = kNoMemoryErr );
		
		isHostnameValid = false;
		if( strcmp_prefix( instance->hostname, context->hostname ) == 0 )
		{
			ptr = &instance->hostname[ hostnameLen ];
			if( i == 1 )
			{
				if( strcmp( ptr, ".local." ) == 0 ) isHostnameValid = true;
			}
			else if( *ptr == '-' )
			{
				++ptr;
				for( end = ptr; isdigit_safe( *end ); ++end ) {}
				if( DecimalTextToUInt32( ptr, end, &u32, &ptr ) == kNoErr )
				{
					if( ( u32 == i ) && ( strcmp( ptr, ".local." ) == 0 ) ) isHostnameValid = true;
				}
			}
		}
		if( !isHostnameValid )
		{
			err = CFDictionarySetCString( badDict, kMDNSDiscoveryTestIncorrectInstanceKey_BadHostname, instance->hostname,
				kSizeCString );
			require_noerr( err, exit );
		}
		
		if( instance->port != (uint16_t)( kMDNSReplierPortBase + context->txtSize ) )
		{
			err = CFDictionarySetInt64( badDict, kMDNSDiscoveryTestIncorrectInstanceKey_BadPort, instance->port );
			require_noerr( err, exit );
		}
		
		isTXTValid = false;
		if( instance->txtLen == context->txtSize )
		{
			uint8_t		name[ kDomainNameLengthMax ];
			
			err = DomainNameFromString( name, instance->name, NULL );
			require_noerr( err, exit );
			
			err = DomainNameAppendString( name, type->name, NULL );
			require_noerr( err, exit );
			
			err = DomainNameAppendString( name, "local", NULL );
			require_noerr( err, exit );
			
			if( _MDNSDiscoveryTestTXTRecordIsValid( name, instance->txtPtr, instance->txtLen ) ) isTXTValid = true;
		}
		if( !isTXTValid )
		{
			char *		hexStr = NULL;
			
			ASPrintF( &hexStr, "%.4H", instance->txtPtr, (int) instance->txtLen, (int) instance->txtLen );
			require_action( hexStr, exit, err = kUnknownErr );
			
			err = CFDictionarySetCString( badDict, kMDNSDiscoveryTestIncorrectInstanceKey_BadTXT, hexStr, kSizeCString );
			free( hexStr );
			require_noerr( err, exit );
		}
		
		if( isHostnameValid )
		{
			uint64_t			addrV4Bitmap, addrV6Bitmap, bitmask, resolveTimeUs;
			unsigned int		j;
			uint8_t				addrV4[ 4 ];
			uint8_t				addrV6[ 16 ];
			
			if( context->recordCountA < 64 )	addrV4Bitmap = ( UINT64_C( 1 ) << context->recordCountA ) - 1;
			else								addrV4Bitmap =  ~UINT64_C( 0 );
			
			if( context->recordCountAAAA < 64 ) addrV6Bitmap = ( UINT64_C( 1 ) << context->recordCountAAAA ) - 1;
			else								addrV6Bitmap =  ~UINT64_C( 0 );
			
			addrV4[ 0 ] = 0;
			WriteBig16( &addrV4[ 1 ], i );
			addrV4[ 3 ] = 0;
			
			memcpy( addrV6, kMDNSReplierBaseAddrV6, 16 );
			WriteBig16( &addrV6[ 12 ], i );
			
			unexpectedAddrs = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
			require_action( unexpectedAddrs, exit, err = kNoMemoryErr );
			
			resolveTimeUs = 0;
			for( ipaddr = instance->ipaddrList; ipaddr; ipaddr = ipaddr->next )
			{
				const uint8_t *		addrPtr;
				unsigned int		lsb;
				int					isAddrValid = false;
				
				if( ipaddr->sip.sa.sa_family == AF_INET )
				{
					addrPtr	= (const uint8_t *) &ipaddr->sip.v4.sin_addr.s_addr;
					lsb		= addrPtr[ 3 ];
					if( ( memcmp( addrPtr, addrV4, 3 ) == 0 ) && ( lsb >= 1 ) && ( lsb <= context->recordCountA ) )
					{
						bitmask = UINT64_C( 1 ) << ( lsb - 1 );
						addrV4Bitmap &= ~bitmask;
						isAddrValid = true;
					}
				}
				else if( ipaddr->sip.sa.sa_family == AF_INET6 )
				{
					addrPtr	= ipaddr->sip.v6.sin6_addr.s6_addr;
					lsb		= addrPtr[ 15 ];
					if( ( memcmp( addrPtr, addrV6, 15 ) == 0 ) && ( lsb >= 1 ) && ( lsb <= context->recordCountAAAA ) )
					{
						bitmask = UINT64_C( 1 ) << ( lsb - 1 );
						addrV6Bitmap &= ~bitmask;
						isAddrValid = true;
					}
				}
				if( isAddrValid )
				{
					if( ipaddr->resolveTimeUs > resolveTimeUs ) resolveTimeUs = ipaddr->resolveTimeUs;
				}
				else
				{
					err = CFPropertyListAppendFormatted( kCFAllocatorDefault, unexpectedAddrs, "%##a", &ipaddr->sip );
					require_noerr( err, exit );
				}
			}
			
			resolveTimeUs += ( instance->discoverTimeUs + instance->resolveTimeUs );
			if( resolveTimeUs > maxResolveTimeUs ) maxResolveTimeUs = resolveTimeUs;
			
			if( CFArrayGetCount( unexpectedAddrs ) > 0 )
			{
				CFDictionarySetValue( badDict, kMDNSDiscoveryTestIncorrectInstanceKey_UnexpectedAddrs, unexpectedAddrs );
			}
			ForgetCF( &unexpectedAddrs );
			
			missingAddrs = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
			require_action( missingAddrs, exit, err = kNoMemoryErr );
			
			for( j = 1; addrV4Bitmap != 0; ++j )
			{
				bitmask = UINT64_C( 1 ) << ( j - 1 );
				if( addrV4Bitmap & bitmask )
				{
					addrV4Bitmap &= ~bitmask;
					addrV4[ 3 ] = (uint8_t) j;
					err = CFPropertyListAppendFormatted( kCFAllocatorDefault, missingAddrs, "%.4a", addrV4 );
					require_noerr( err, exit );
				}
			}
			for( j = 1; addrV6Bitmap != 0; ++j )
			{
				bitmask = UINT64_C( 1 ) << ( j - 1 );
				if( addrV6Bitmap & bitmask )
				{
					addrV6Bitmap &= ~bitmask;
					addrV6[ 15 ] = (uint8_t) j;
					err = CFPropertyListAppendFormatted( kCFAllocatorDefault, missingAddrs, "%.16a", addrV6 );
					require_noerr( err, exit );
				}
			}
			
			if( CFArrayGetCount( missingAddrs ) > 0 )
			{
				CFDictionarySetValue( badDict, kMDNSDiscoveryTestIncorrectInstanceKey_MissingAddrs, missingAddrs );
			}
			ForgetCF( &missingAddrs );
		}
		
		if( CFDictionaryGetCount( badDict ) > 0 )
		{
			err = CFDictionarySetCString( badDict, kMDNSDiscoveryTestIncorrectInstanceKey_Name, instance->name,
				kSizeCString );
			require_noerr( err, exit );
			
			CFDictionarySetBoolean( badDict, kMDNSDiscoveryTestIncorrectInstanceKey_DidResolve, true );
			CFArrayAppendValue( incorrectInstances, badDict );
		}
		ForgetCF( &badDict );
	}
	
	if( ( CFArrayGetCount( unexpectedInstances ) == 0 ) &&
		( CFArrayGetCount( missingInstances )    == 0 ) &&
		( CFArrayGetCount( incorrectInstances )  == 0 ) )
	{
		err = CFDictionarySetInt64( plist, kMDNSDiscoveryTestResultsKey_TotalResolveTime, (int64_t) maxResolveTimeUs );
		require_noerr( err, exit );
		success = true;
	}
	else
	{
		success = false;
	}
	CFDictionarySetBoolean( plist, kMDNSDiscoveryTestResultsKey_Success, success );
	
	err = OutputPropertyList( plist, context->outputFormat, context->outputFilePath );
	require_noerr_quiet( err, exit );
	
exit:
	ForgetCF( &context->browser );
	if( context->replierPID != -1 )
	{
		kill( context->replierPID, SIGTERM );
		context->replierPID = -1;
	}
	FreeNullSafe( instanceArray );
	CFReleaseNullSafe( plist );
	CFReleaseNullSafe( badDict );
	CFReleaseNullSafe( unexpectedAddrs );
	CFReleaseNullSafe( missingAddrs );
	exit( err ? 1 : ( success ? 0 : 2 ) );
}

//===========================================================================================================================
//	_MDNSDiscoveryTestTXTRecordIsValid
//===========================================================================================================================

static Boolean	_MDNSDiscoveryTestTXTRecordIsValid( const uint8_t *inRecordName, const uint8_t *inTXTPtr, size_t inTXTLen )
{
	uint32_t			hash;
	int					n;
	const uint8_t *		ptr;
	size_t				i, wholeCount, remCount;
	uint8_t				txtStr[ 16 ];
	
	if( inTXTLen == 0 ) return( false );
	
	hash = _FNV1( inRecordName, DomainNameLength( inRecordName ) );
	
	txtStr[ 0 ] = 15;
	n = MemPrintF( &txtStr[ 1 ], 15, "hash=0x%08X", hash );
	check( n == 15 );
	
	ptr = inTXTPtr;
	wholeCount = inTXTLen / 16;
	for( i = 0; i < wholeCount; ++i )
	{
		if( memcmp( ptr, txtStr, 16 ) != 0 ) return( false );
		ptr += 16;
	}
	
	remCount = inTXTLen % 16;
	if( remCount > 0 )
	{
		txtStr[ 0 ] = (uint8_t)( remCount - 1 );
		if( memcmp( ptr, txtStr, remCount ) != 0 ) return( false );
		ptr += remCount;
	}
	check( ptr == &inTXTPtr[ inTXTLen ] );
	return( true );
}

//===========================================================================================================================
//	DotLocalTestCmd
//===========================================================================================================================

#define kDotLocalTestPreparationTimeLimitSecs		5
#define kDotLocalTestSubtestDurationSecs			5

// Constants for SRV record query subtest.

#define kDotLocalTestSRV_Priority		1
#define kDotLocalTestSRV_Weight			0
#define kDotLocalTestSRV_Port			80
#define kDotLocalTestSRV_TargetName		( (const uint8_t *) "\x03" "www" "\x07" "example" "\x03" "com" )
#define kDotLocalTestSRV_TargetStr		"www.example.com."
#define kDotLocalTestSRV_ResultStr		"1 0 80 " kDotLocalTestSRV_TargetStr

typedef enum
{
	kDotLocalTestState_Unset				= 0,
	kDotLocalTestState_Preparing			= 1,
	kDotLocalTestState_GAIMDNSOnly			= 2,
	kDotLocalTestState_GAIDNSOnly			= 3,
	kDotLocalTestState_GAIBoth				= 4,
	kDotLocalTestState_GAINeither			= 5,
	kDotLocalTestState_GAINoSuchRecord		= 6,
	kDotLocalTestState_QuerySRV				= 7,
	kDotLocalTestState_Done					= 8
	
}	DotLocalTestState;

typedef struct
{
	const char *			testDesc;			// Description of the current subtest.
	char *					queryName;			// Query name for GetAddrInfo or QueryRecord operation.
	dispatch_source_t		timer;				// Timer used for limiting the time for each subtest.
	NanoTime64				startTime;			// Timestamp of when the subtest started.
	NanoTime64				endTime;			// Timestamp of when the subtest ended.
	CFMutableArrayRef		correctResults;		// Operation results that were expected.
	CFMutableArrayRef		duplicateResults;	// Operation results that were expected, but were already received.
	CFMutableArrayRef		unexpectedResults;	// Operation results that were unexpected.
	OSStatus				error;				// Subtest's error code.
	uint32_t				addrDNSv4;			// If hasDNSv4 is true, the expected DNS IPv4 address for queryName.
	uint32_t				addrMDNSv4;			// If hasMDNSv4 is true, the expected MDNS IPv4 address for queryName.
	uint8_t					addrDNSv6[ 16 ];	// If hasDNSv6 is true, the expected DNS IPv6 address for queryName.
	uint8_t					addrMDNSv6[ 16 ];	// If hasMDNSv6 is true, the expected MDNS IPv6 address for queryName.
	Boolean					hasDNSv4;			// True if queryName has a DNS IPv4 address.
	Boolean					hasDNSv6;			// True if queryName has a DNS IPv6 address.
	Boolean					hasMDNSv4;			// True if queryName has an MDNS IPv4 address.
	Boolean					hasMDNSv6;			// True if queryName has an MDNS IPv6 address.
	Boolean					needDNSv4;			// True if operation is expecting, but hasn't received a DNS IPv4 result.
	Boolean					needDNSv6;			// True if operation is expecting, but hasn't received a DNS IPv6 result.
	Boolean					needMDNSv4;			// True if operation is expecting, but hasn't received an MDNS IPv4 result.
	Boolean					needMDNSv6;			// True if operation is expecting, but hasn't received an MDNS IPv6 result.
	Boolean					needSRV;			// True if operation is expecting, but hasn't received an SRV result.
	
}	DotLocalSubtest;

typedef struct
{
	dispatch_source_t		timer;				// Timer used for limiting the time for each state/subtest.
	DotLocalSubtest *		subtest;			// Current subtest's state.
	DNSServiceRef			connection;			// Shared connection for DNS-SD operations.
	DNSServiceRef			op;					// Reference for the current DNS-SD operation.
	DNSServiceRef			op2;				// Reference for mdnsreplier probe query used during preparing state.
	DNSRecordRef			localSOARef;		// Reference returned by DNSServiceRegisterRecord() for local. SOA record.
	char *					replierCmd;			// Command used to invoke the mdnsreplier.
	char *					serverCmd;			// Command used to invoke the test DNS server.
	CFMutableArrayRef		reportsGAI;			// Reports for subtests that use DNSServiceGetAddrInfo.
	CFMutableArrayRef		reportsQuerySRV;	// Reports for subtests that use DNSServiceQueryRecord for SRV records.
	NanoTime64				startTime;			// Timestamp for when the test started.
	NanoTime64				endTime;			// Timestamp for when the test ended.
	DotLocalTestState		state;				// The test's current state.
	pid_t					replierPID;			// PID of spawned mdnsreplier.
	pid_t					serverPID;			// PID of spawned test DNS server.
	uint32_t				ifIndex;			// Interface index used for mdnsreplier.
	char *					outputFilePath;		// File to write test results to. If NULL, then write to stdout.
	OutputFormatType		outputFormat;		// Format of test results output.
	Boolean					registeredSOA;		// True if the dummy local. SOA record was successfully registered.
	Boolean					serverIsReady;		// True if response was received for test DNS server probe query.
	Boolean					replierIsReady;		// True if response was received for mdnsreplier probe query.
	Boolean					testFailed;			// True if at least one subtest failed.
	char					labelStr[ 20 + 1 ];	// Unique label string used for for making the query names used by subtests.
												// The format of this string is "dotlocal-test-<six random chars>".
}	DotLocalTestContext;

static void	_DotLocalTestStateMachine( DotLocalTestContext *inContext );
static void DNSSD_API
	_DotLocalTestProbeQueryRecordCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		uint16_t				inType,
		uint16_t				inClass,
		uint16_t				inRDataLen,
		const void *			inRDataPtr,
		uint32_t				inTTL,
		void *					inContext );
static void DNSSD_API
	_DotLocalTestRegisterRecordCallback(
		DNSServiceRef		inSDRef,
		DNSRecordRef		inRecordRef,
		DNSServiceFlags		inFlags,
		DNSServiceErrorType	inError,
		void *				inContext );
static void	_DotLocalTestTimerHandler( void *inContext );
static void DNSSD_API
	_DotLocalTestGAICallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inHostname,
		const struct sockaddr *	inSockAddr,
		uint32_t				inTTL,
		void *					inContext );
static void DNSSD_API
	_DotLocalTestQueryRecordCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		uint16_t				inType,
		uint16_t				inClass,
		uint16_t				inRDataLen,
		const void *			inRDataPtr,
		uint32_t				inTTL,
		void *					inContext );

static void	DotLocalTestCmd( void )
{
	OSStatus					err;
	DotLocalTestContext *		context;
	uint8_t *					rdataPtr;
	size_t						rdataLen;
	DNSServiceFlags				flags;
	char						queryName[ 64 ];
	char						randBuf[ 6 + 1 ];	// Large enough for four and six character random strings below.
	
	context = (DotLocalTestContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	context->startTime	= NanoTimeGetCurrent();
	context->endTime	= kNanoTime_Invalid;
	
	context->state = kDotLocalTestState_Preparing;
	
	if( gDotLocalTest_Interface )
	{
		err = InterfaceIndexFromArgString( gDotLocalTest_Interface, &context->ifIndex );
		require_noerr_quiet( err, exit );
	}
	else
	{
		err = _MDNSInterfaceGetAny( kMDNSInterfaceSubset_All, NULL, &context->ifIndex );
		require_noerr_quiet( err, exit );
	}
	
	if( gDotLocalTest_OutputFilePath )
	{
		context->outputFilePath = strdup( gDotLocalTest_OutputFilePath );
		require_action( context->outputFilePath, exit, err = kNoMemoryErr );
	}
	
	err = OutputFormatFromArgString( gDotLocalTest_OutputFormat, &context->outputFormat );
	require_noerr_quiet( err, exit );
	
	context->reportsGAI = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	require_action( context->reportsGAI, exit, err = kNoMemoryErr );
	
	context->reportsQuerySRV = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	require_action( context->reportsQuerySRV, exit, err = kNoMemoryErr );
	
	SNPrintF( context->labelStr, sizeof( context->labelStr ), "dotlocal-test-%s",
		_RandomStringExact( kLowerAlphaNumericCharSet, kLowerAlphaNumericCharSetSize, 6, randBuf ) );
	
	// Spawn an mdnsreplier.
	
	ASPrintF( &context->replierCmd,
		"dnssdutil mdnsreplier --follow %lld --interface %u --hostname %s --tag %s --maxInstanceCount 2 --countA 1"
		" --countAAAA 1",
		(int64_t) getpid(), context->ifIndex, context->labelStr,
		_RandomStringExact( kLowerAlphaNumericCharSet, kLowerAlphaNumericCharSetSize, 4, randBuf ) );
	require_action_quiet( context->replierCmd, exit, err = kUnknownErr );
	
	err = SpawnCommand( &context->replierPID, "%s", context->replierCmd );
	require_noerr( err, exit );
	
	// Spawn a test DNS server
	
	ASPrintF( &context->serverCmd,
		"dnssdutil server --loopback --follow %lld --port 0 --defaultTTL 300 --domain %s.local.",
		(int64_t) getpid(), context->labelStr );
	require_action_quiet( context->serverCmd, exit, err = kUnknownErr );
	
	err = SpawnCommand( &context->serverPID, "%s", context->serverCmd );
	require_noerr( err, exit );
	
	// Create a shared DNS-SD connection.
	
	err = DNSServiceCreateConnection( &context->connection );
	require_noerr( err, exit );
	
	err = DNSServiceSetDispatchQueue( context->connection, dispatch_get_main_queue() );
	require_noerr( err, exit );
	
	// Create probe query for DNS server, i.e., query for any name that has an A record.
	
	SNPrintF( queryName, sizeof( queryName ), "tag-dotlocal-test-probe.ipv4.%s.local.", context->labelStr );
	
	flags = kDNSServiceFlagsShareConnection;
#if( TARGET_OS_WATCH )
	flags |= kDNSServiceFlagsPathEvaluationDone;
#endif
	
	context->op = context->connection;
	err = DNSServiceQueryRecord( &context->op, flags, kDNSServiceInterfaceIndexAny, queryName, kDNSServiceType_A,
		kDNSServiceClass_IN, _DotLocalTestProbeQueryRecordCallback, context );
	require_noerr( err, exit );
	
	// Create probe query for mdnsreplier's "about" TXT record.
	
	SNPrintF( queryName, sizeof( queryName ), "about.%s.local.", context->labelStr );
	
	flags = kDNSServiceFlagsShareConnection | kDNSServiceFlagsForceMulticast;
#if( TARGET_OS_WATCH )
	flags |= kDNSServiceFlagsPathEvaluationDone;
#endif
	
	context->op2 = context->connection;
	err = DNSServiceQueryRecord( &context->op2, flags, context->ifIndex, queryName, kDNSServiceType_TXT, kDNSServiceClass_IN,
		_DotLocalTestProbeQueryRecordCallback, context );
	require_noerr( err, exit );
	
	// Register a dummy local. SOA record.
	
	err = CreateSOARecordData( kRootLabel, kRootLabel, 1976040101, 1 * kSecondsPerDay, 2 * kSecondsPerHour,
		1000 * kSecondsPerHour, 2 * kSecondsPerDay, &rdataPtr, &rdataLen );
	require_noerr( err, exit );
	
	err = DNSServiceRegisterRecord( context->connection, &context->localSOARef, kDNSServiceFlagsUnique,
		kDNSServiceInterfaceIndexLocalOnly, "local.", kDNSServiceType_SOA, kDNSServiceClass_IN, 1,
		rdataPtr, 1 * kSecondsPerHour, _DotLocalTestRegisterRecordCallback, context );
	require_noerr( err, exit );
	
	// Start timer for probe responses and SOA record registration.
	
	err = DispatchTimerOneShotCreate( dispatch_time_seconds( kDotLocalTestPreparationTimeLimitSecs ),
		INT64_C_safe( kDotLocalTestPreparationTimeLimitSecs ) * kNanosecondsPerSecond / 10, dispatch_get_main_queue(),
		_DotLocalTestTimerHandler, context, &context->timer );
	require_noerr( err, exit );
	dispatch_resume( context->timer );
	
	dispatch_main();
	
exit:
	if( err ) ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//	_DotLocalTestStateMachine
//===========================================================================================================================

static OSStatus	_DotLocalSubtestCreate( DotLocalSubtest **outSubtest );
static void		_DotLocalSubtestFree( DotLocalSubtest *inSubtest );
static OSStatus	_DotLocalTestStartSubtest( DotLocalTestContext *inContext );
static OSStatus	_DotLocalTestFinalizeSubtest( DotLocalTestContext *inContext );
static void		_DotLocalTestFinalizeAndExit( DotLocalTestContext *inContext ) ATTRIBUTE_NORETURN;

static void	_DotLocalTestStateMachine( DotLocalTestContext *inContext )
{
	OSStatus				err;
	DotLocalTestState		nextState;
	
	DNSServiceForget( &inContext->op );
	DNSServiceForget( &inContext->op2 );
	dispatch_source_forget( &inContext->timer );
	
	switch( inContext->state )
	{
		case kDotLocalTestState_Preparing:			nextState = kDotLocalTestState_GAIMDNSOnly;		break;
		case kDotLocalTestState_GAIMDNSOnly:		nextState = kDotLocalTestState_GAIDNSOnly;		break;
		case kDotLocalTestState_GAIDNSOnly:			nextState = kDotLocalTestState_GAIBoth;			break;
		case kDotLocalTestState_GAIBoth:			nextState = kDotLocalTestState_GAINeither;		break;
		case kDotLocalTestState_GAINeither:			nextState = kDotLocalTestState_GAINoSuchRecord;	break;
		case kDotLocalTestState_GAINoSuchRecord:	nextState = kDotLocalTestState_QuerySRV;		break;
		case kDotLocalTestState_QuerySRV:			nextState = kDotLocalTestState_Done;			break;
		default:									err = kStateErr;								goto exit;
	}
	
	if( inContext->state == kDotLocalTestState_Preparing )
	{
		if( !inContext->registeredSOA || !inContext->serverIsReady || !inContext->replierIsReady )
		{
			FPrintF( stderr, "Preparation timed out: Registered SOA? %s. Server ready? %s. mdnsreplier ready? %s.\n",
				YesNoStr( inContext->registeredSOA ),
				YesNoStr( inContext->serverIsReady ),
				YesNoStr( inContext->replierIsReady ) );
			err = kNotPreparedErr;
			goto exit;
		}
	}
	else
	{
		err = _DotLocalTestFinalizeSubtest( inContext );
		require_noerr( err, exit );
	}
	
	inContext->state = nextState;
	if( inContext->state == kDotLocalTestState_Done ) _DotLocalTestFinalizeAndExit( inContext );
	err = _DotLocalTestStartSubtest( inContext );
	
exit:
	if( err ) ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//	_DotLocalSubtestCreate
//===========================================================================================================================

static OSStatus	_DotLocalSubtestCreate( DotLocalSubtest **outSubtest )
{
	OSStatus				err;
	DotLocalSubtest *		obj;
	
	obj = (DotLocalSubtest *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	obj->correctResults = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	require_action( obj->correctResults, exit, err = kNoMemoryErr );
	
	obj->duplicateResults = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	require_action( obj->duplicateResults, exit, err = kNoMemoryErr );
	
	obj->unexpectedResults = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	require_action( obj->unexpectedResults, exit, err = kNoMemoryErr );
	
	*outSubtest = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	if( obj ) _DotLocalSubtestFree( obj );
	return( err );
}

//===========================================================================================================================
//	_DotLocalSubtestFree
//===========================================================================================================================

static void	_DotLocalSubtestFree( DotLocalSubtest *inSubtest )
{
	ForgetMem( &inSubtest->queryName );
	ForgetCF( &inSubtest->correctResults );
	ForgetCF( &inSubtest->duplicateResults );
	ForgetCF( &inSubtest->unexpectedResults );
	free( inSubtest );
}

//===========================================================================================================================
//	_DotLocalTestStartSubtest
//===========================================================================================================================

static OSStatus	_DotLocalTestStartSubtest( DotLocalTestContext *inContext )
{
	OSStatus				err;
	DotLocalSubtest *		subtest	= NULL;
	DNSServiceRef			op		= NULL;
	DNSServiceFlags			flags;
	
	err = _DotLocalSubtestCreate( &subtest );
	require_noerr( err, exit );
	
	if( inContext->state == kDotLocalTestState_GAIMDNSOnly )
	{
		ASPrintF( &subtest->queryName, "%s-2.local.", inContext->labelStr );
		require_action_quiet( subtest->queryName, exit, err = kNoMemoryErr );
		
		subtest->hasMDNSv4 = subtest->needMDNSv4 = true;
		subtest->hasMDNSv6 = subtest->needMDNSv6 = true;
		
		subtest->addrMDNSv4 = htonl( 0x00000201 );					// 0.0.2.1
		memcpy( subtest->addrMDNSv6, kMDNSReplierBaseAddrV6, 16 );	// 2001:db8:2::2:1
		subtest->addrMDNSv6[ 13 ] = 2;
		subtest->addrMDNSv6[ 15 ] = 1;
		
		subtest->testDesc = kDotLocalTestSubtestDesc_GAIMDNSOnly;
	}
	
	else if( inContext->state == kDotLocalTestState_GAIDNSOnly )
	{
		ASPrintF( &subtest->queryName, "tag-dns-only.%s.local.", inContext->labelStr );
		require_action_quiet( subtest->queryName, exit, err = kNoMemoryErr );
		
		subtest->hasDNSv4 = subtest->needDNSv4 = true;
		subtest->hasDNSv6 = subtest->needDNSv6 = true;
		
		subtest->addrDNSv4 = htonl( kDNSServerBaseAddrV4 + 1 );		// 203.0.113.1
		memcpy( subtest->addrDNSv6, kDNSServerBaseAddrV6, 16 );		// 2001:db8:1::1
		subtest->addrDNSv6[ 15 ] = 1;
		
		subtest->testDesc = kDotLocalTestSubtestDesc_GAIDNSOnly;
	}
	
	else if( inContext->state == kDotLocalTestState_GAIBoth )
	{
		ASPrintF( &subtest->queryName, "%s.local.", inContext->labelStr );
		require_action_quiet( subtest->queryName, exit, err = kNoMemoryErr );
		
		subtest->hasDNSv4	= subtest->needDNSv4	= true;
		subtest->hasDNSv6	= subtest->needDNSv6	= true;
		subtest->hasMDNSv4	= subtest->needMDNSv4	= true;
		subtest->hasMDNSv6	= subtest->needMDNSv6	= true;
		
		subtest->addrDNSv4 = htonl( kDNSServerBaseAddrV4 + 1 );		// 203.0.113.1
		memcpy( subtest->addrDNSv6, kDNSServerBaseAddrV6, 16 );		// 2001:db8:1::1
		subtest->addrDNSv6[ 15 ] = 1;
		
		subtest->addrMDNSv4 = htonl( 0x00000101 );					// 0.0.1.1
		memcpy( subtest->addrMDNSv6, kMDNSReplierBaseAddrV6, 16 );	// 2001:db8:2::1:1
		subtest->addrMDNSv6[ 13 ] = 1;
		subtest->addrMDNSv6[ 15 ] = 1;
		
		subtest->testDesc = kDotLocalTestSubtestDesc_GAIBoth;
	}
	
	else if( inContext->state == kDotLocalTestState_GAINeither )
	{
		ASPrintF( &subtest->queryName, "doesnotexit-%s.local.", inContext->labelStr );
		require_action_quiet( subtest->queryName, exit, err = kNoMemoryErr );
		
		subtest->testDesc = kDotLocalTestSubtestDesc_GAINeither;
	}
	
	else if( inContext->state == kDotLocalTestState_GAINoSuchRecord )
	{
		ASPrintF( &subtest->queryName, "doesnotexit-dns.%s.local.", inContext->labelStr );
		require_action_quiet( subtest->queryName, exit, err = kNoMemoryErr );
		
		subtest->hasDNSv4 = subtest->needDNSv4 = true;
		subtest->hasDNSv6 = subtest->needDNSv6 = true;
		subtest->testDesc = kDotLocalTestSubtestDesc_GAINoSuchRecord;
	}
	
	else if( inContext->state == kDotLocalTestState_QuerySRV )
	{
		ASPrintF( &subtest->queryName, "_http._tcp.srv-%u-%u-%u.%s%s.local.",
			kDotLocalTestSRV_Priority, kDotLocalTestSRV_Weight, kDotLocalTestSRV_Port, kDotLocalTestSRV_TargetStr,
			inContext->labelStr );
		require_action_quiet( subtest->queryName, exit, err = kNoMemoryErr );
		
		subtest->needSRV	= true;
		subtest->testDesc	= kDotLocalTestSubtestDesc_QuerySRV;
	}
	
	else
	{
		err = kStateErr;
		goto exit;
	}
	
	// Start new operation.
	
	flags = kDNSServiceFlagsShareConnection | kDNSServiceFlagsReturnIntermediates;
#if( TARGET_OS_WATCH )
	flags |= kDNSServiceFlagsPathEvaluationDone;
#endif
	
	subtest->startTime	= NanoTimeGetCurrent();
	subtest->endTime	= kNanoTime_Invalid;
	
	if( inContext->state == kDotLocalTestState_QuerySRV )
	{
		op = inContext->connection;
		err = DNSServiceQueryRecord( &op, flags, kDNSServiceInterfaceIndexAny, subtest->queryName,
			kDNSServiceType_SRV, kDNSServiceClass_IN, _DotLocalTestQueryRecordCallback, inContext );
		require_noerr( err, exit );
	}
	else
	{
		op = inContext->connection;
		err = DNSServiceGetAddrInfo( &op, flags, kDNSServiceInterfaceIndexAny,
			kDNSServiceProtocol_IPv4 | kDNSServiceProtocol_IPv6, subtest->queryName, _DotLocalTestGAICallback, inContext );
		require_noerr( err, exit );
	}
	
	// Start timer.
	
	check( !inContext->timer );
	err = DispatchTimerOneShotCreate( dispatch_time_seconds( kDotLocalTestSubtestDurationSecs ),
		INT64_C_safe( kDotLocalTestSubtestDurationSecs ) * kNanosecondsPerSecond / 10, dispatch_get_main_queue(),
		_DotLocalTestTimerHandler, inContext, &inContext->timer );
	require_noerr( err, exit );
	dispatch_resume( inContext->timer );
	
	check( !inContext->op );
	inContext->op = op;
	op = NULL;
	
	check( !inContext->subtest );
	inContext->subtest = subtest;
	subtest = NULL;
	
exit:
	if( subtest )	_DotLocalSubtestFree( subtest );
	if( op )		DNSServiceRefDeallocate( op );
	return( err );
}

//===========================================================================================================================
//	_DotLocalTestFinalizeSubtest
//===========================================================================================================================

#define kDotLocalTestReportKey_StartTime				CFSTR( "startTime" )		// String.
#define kDotLocalTestReportKey_EndTime					CFSTR( "endTime" )			// String.
#define kDotLocalTestReportKey_Success					CFSTR( "success" )			// Boolean.
#define kDotLocalTestReportKey_MDNSReplierCmd			CFSTR( "replierCmd" )		// String.
#define kDotLocalTestReportKey_DNSServerCmd				CFSTR( "serverCmd" )		// String.
#define kDotLocalTestReportKey_GetAddrInfoTests			CFSTR( "testsGAI" )			// Array of Dictionaries.
#define kDotLocalTestReportKey_QuerySRVTests			CFSTR( "testsQuerySRV" )	// Array of Dictionaries.
#define kDotLocalTestReportKey_Description				CFSTR( "description" )		// String.
#define kDotLocalTestReportKey_QueryName				CFSTR( "queryName" )		// String.
#define kDotLocalTestReportKey_Error					CFSTR( "error" )			// Integer.
#define kDotLocalTestReportKey_Results					CFSTR( "results" )			// Dictionary of Arrays.
#define kDotLocalTestReportKey_CorrectResults			CFSTR( "correct" )			// Array of Strings
#define kDotLocalTestReportKey_DuplicateResults			CFSTR( "duplicates" )		// Array of Strings.
#define kDotLocalTestReportKey_UnexpectedResults		CFSTR( "unexpected" )		// Array of Strings.
#define kDotLocalTestReportKey_MissingResults			CFSTR( "missing" )			// Array of Strings.

static OSStatus	_DotLocalTestFinalizeSubtest( DotLocalTestContext *inContext )
{
	OSStatus					err;
	DotLocalSubtest *			subtest;
	CFMutableDictionaryRef		reportDict;
	CFMutableDictionaryRef		resultsDict;
	CFMutableArrayRef			missingResults, reportArray;
	char						startTime[ 32 ];
	char						endTime[ 32 ];
	
	subtest = inContext->subtest;
	inContext->subtest = NULL;
	
	subtest->endTime = NanoTimeGetCurrent();
	_NanoTime64ToTimestamp( subtest->startTime, startTime, sizeof( startTime ) );
	_NanoTime64ToTimestamp( subtest->endTime, endTime, sizeof( endTime ) );
	
	reportDict = NULL;
	err = CFPropertyListCreateFormatted( kCFAllocatorDefault, &reportDict,
		"{"
			"%kO=%s"	// startTime
			"%kO=%s"	// endTime
			"%kO=%s"	// queryName
			"%kO=%s"	// description
			"%kO={%@}"	// results
		"}",
		kDotLocalTestReportKey_StartTime,	startTime,
		kDotLocalTestReportKey_EndTime,		endTime,
		kDotLocalTestReportKey_QueryName,	subtest->queryName,
		kDotLocalTestReportKey_Description,	subtest->testDesc,
		kDotLocalTestReportKey_Results,		&resultsDict );
	require_noerr( err, exit );
	
	missingResults = NULL;
	switch( inContext->state )
	{
		case kDotLocalTestState_GAIMDNSOnly:
		case kDotLocalTestState_GAIDNSOnly:
		case kDotLocalTestState_GAIBoth:
		case kDotLocalTestState_GAINeither:
			if( subtest->needDNSv4 || subtest->needDNSv6 || subtest->needMDNSv4 || subtest->needMDNSv6 )
			{
				err = CFPropertyListCreateFormatted( kCFAllocatorDefault, &missingResults,
					"["
						"%.4a"	// Expected DNS IPv4 address
						"%.16a"	// Expected DNS IPv6 address
						"%.4a"	// Expected MDNS IPv4 address
						"%.16a"	// Expected MDNS IPv6 address
					"]",
					subtest->needDNSv4  ? &subtest->addrDNSv4  : NULL,
					subtest->needDNSv6  ?  subtest->addrDNSv6  : NULL,
					subtest->needMDNSv4 ? &subtest->addrMDNSv4 : NULL,
					subtest->needMDNSv6 ?  subtest->addrMDNSv6 : NULL );
				require_noerr( err, exit );
			}
			break;
		
		case kDotLocalTestState_QuerySRV:
			if( subtest->needSRV )
			{
				err = CFPropertyListCreateFormatted( kCFAllocatorDefault, &missingResults,
					"["
						"%s"	// Expected SRV record data as a string.
					"]",
					kDotLocalTestSRV_ResultStr );
				require_noerr( err, exit );
			}
			break;
		
		case kDotLocalTestState_GAINoSuchRecord:
			if( subtest->needDNSv4 || subtest->needDNSv6 )
			{
				err = CFPropertyListCreateFormatted( kCFAllocatorDefault, &missingResults,
					"["
						"%s" // No Such Record (A)
						"%s" // No Such Record (AAAA)
					"]",
					subtest->needDNSv4 ? kNoSuchRecordAStr    : NULL,
					subtest->needDNSv6 ? kNoSuchRecordAAAAStr : NULL );
				require_noerr( err, exit );
			}
			break;
		
		default:
			err = kStateErr;
			goto exit;
	}
	
	CFDictionarySetValue( resultsDict, kDotLocalTestReportKey_CorrectResults, subtest->correctResults );
	
	if( missingResults )
	{
		CFDictionarySetValue( resultsDict, kDotLocalTestReportKey_MissingResults, missingResults );
		ForgetCF( &missingResults );
		if( !subtest->error ) subtest->error = kNotFoundErr;
	}
	
	if( CFArrayGetCount( subtest->unexpectedResults ) > 0 )
	{
		CFDictionarySetValue( resultsDict, kDotLocalTestReportKey_UnexpectedResults, subtest->unexpectedResults );
		if( !subtest->error ) subtest->error = kUnexpectedErr;
	}
	
	if( CFArrayGetCount( subtest->duplicateResults ) > 0 )
	{
		CFDictionarySetValue( resultsDict, kDotLocalTestReportKey_DuplicateResults, subtest->duplicateResults );
		if( !subtest->error ) subtest->error = kDuplicateErr;
	}
	
	if( subtest->error ) inContext->testFailed = true;
	err = CFDictionarySetInt64( reportDict, kDotLocalTestReportKey_Error, subtest->error );
	require_noerr( err, exit );
	
	reportArray = ( inContext->state == kDotLocalTestState_QuerySRV ) ? inContext->reportsQuerySRV : inContext->reportsGAI;
	CFArrayAppendValue( reportArray, reportDict );
	
exit:
	_DotLocalSubtestFree( subtest );
	CFReleaseNullSafe( reportDict );
	return( err );
}

//===========================================================================================================================
//	_DotLocalTestFinalizeAndExit
//===========================================================================================================================

static void	_DotLocalTestFinalizeAndExit( DotLocalTestContext *inContext )
{
	OSStatus				err;
	CFPropertyListRef		plist;
	char					timestampStart[ 32 ];
	char					timestampEnd[ 32 ];
	
	check( !inContext->subtest );
	inContext->endTime = NanoTimeGetCurrent();
	
	if( inContext->replierPID != -1 )
	{
		kill( inContext->replierPID, SIGTERM );
		inContext->replierPID = -1;
	}
	if( inContext->serverPID != -1 )
	{
		kill( inContext->serverPID, SIGTERM );
		inContext->serverPID = -1;
	}
	err = DNSServiceRemoveRecord( inContext->connection, inContext->localSOARef, 0 );
	require_noerr( err, exit );
	
	_NanoTime64ToTimestamp( inContext->startTime, timestampStart, sizeof( timestampStart ) );
	_NanoTime64ToTimestamp( inContext->endTime, timestampEnd, sizeof( timestampEnd ) );
	
	err = CFPropertyListCreateFormatted( kCFAllocatorDefault, &plist,
		"{"
			"%kO=%s"	// startTime
			"%kO=%s"	// endTime
			"%kO=%O"	// testsGAI
			"%kO=%O"	// testsQuerySRV
			"%kO=%b"	// success
			"%kO=%s"	// replierCmd
			"%kO=%s"	// serverCmd
		"}",
		kDotLocalTestReportKey_StartTime,			timestampStart,
		kDotLocalTestReportKey_EndTime,				timestampEnd,
		kDotLocalTestReportKey_GetAddrInfoTests,	inContext->reportsGAI,
		kDotLocalTestReportKey_QuerySRVTests,		inContext->reportsQuerySRV,
		kDotLocalTestReportKey_Success,				inContext->testFailed ? false : true,
		kDotLocalTestReportKey_MDNSReplierCmd,		inContext->replierCmd,
		kDotLocalTestReportKey_DNSServerCmd,		inContext->serverCmd );
	require_noerr( err, exit );
	
	ForgetCF( &inContext->reportsGAI );
	ForgetCF( &inContext->reportsQuerySRV );
	
	err = OutputPropertyList( plist, inContext->outputFormat, inContext->outputFilePath );
	CFRelease( plist );
	require_noerr( err, exit );
	
	exit( inContext->testFailed ? 2 : 0 );
	
exit:
	ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//	_DotLocalTestProbeQueryRecordCallback
//===========================================================================================================================

static void DNSSD_API
	_DotLocalTestProbeQueryRecordCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		uint16_t				inType,
		uint16_t				inClass,
		uint16_t				inRDataLen,
		const void *			inRDataPtr,
		uint32_t				inTTL,
		void *					inContext )
{
	DotLocalTestContext * const		context = (DotLocalTestContext *) inContext;
	
	Unused( inInterfaceIndex );
	Unused( inFullName );
	Unused( inType );
	Unused( inClass );
	Unused( inRDataLen );
	Unused( inRDataPtr );
	Unused( inTTL );
	
	check( context->state == kDotLocalTestState_Preparing );
	
	require_quiet( ( inFlags & kDNSServiceFlagsAdd ) && !inError, exit );
	
	if( inSDRef == context->op )
	{
		DNSServiceForget( &context->op );
		context->serverIsReady = true;
	}
	else if( inSDRef == context->op2 )
	{
		DNSServiceForget( &context->op2 );
		context->replierIsReady = true;
	}
	
	if( context->registeredSOA && context->serverIsReady && context->replierIsReady )
	{
		_DotLocalTestStateMachine( context );
	}
	
exit:
	return;
}

//===========================================================================================================================
//	_DotLocalTestRegisterRecordCallback
//===========================================================================================================================

static void DNSSD_API
	_DotLocalTestRegisterRecordCallback(
		DNSServiceRef		inSDRef,
		DNSRecordRef		inRecordRef,
		DNSServiceFlags		inFlags,
		DNSServiceErrorType	inError,
		void *				inContext )
{
	DotLocalTestContext * const		context = (DotLocalTestContext *) inContext;
	
	Unused( inSDRef );
	Unused( inRecordRef );
	Unused( inFlags );
	
	if( inError ) ErrQuit( 1, "error: local. SOA record registration failed: %#m\n", inError );
	
	if( !context->registeredSOA )
	{
		context->registeredSOA = true;
		if( context->serverIsReady && context->replierIsReady ) _DotLocalTestStateMachine( context );
	}
}

//===========================================================================================================================
//	_DotLocalTestTimerHandler
//===========================================================================================================================

static void	_DotLocalTestTimerHandler( void *inContext )
{
	_DotLocalTestStateMachine( (DotLocalTestContext *) inContext );
}

//===========================================================================================================================
//	_DotLocalTestGAICallback
//===========================================================================================================================

static void DNSSD_API
	_DotLocalTestGAICallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inHostname,
		const struct sockaddr *	inSockAddr,
		uint32_t				inTTL,
		void *					inContext )
{
	OSStatus						err;
	DotLocalTestContext * const		context = (DotLocalTestContext *) inContext;
	DotLocalSubtest * const			subtest	= context->subtest;
	const sockaddr_ip * const		sip		= (const sockaddr_ip *) inSockAddr;
	
	Unused( inSDRef );
	Unused( inInterfaceIndex );
	Unused( inHostname );
	Unused( inTTL );
	
	require_action_quiet( inFlags & kDNSServiceFlagsAdd, exit, err = kFlagErr );
	require_action_quiet( ( sip->sa.sa_family == AF_INET ) || ( sip->sa.sa_family == AF_INET6 ), exit, err = kTypeErr );
	
	if( context->state == kDotLocalTestState_GAINoSuchRecord )
	{
		if( inError == kDNSServiceErr_NoSuchRecord )
		{
			CFMutableArrayRef		array = NULL;	
			const char *			noSuchRecordStr;
			
			if( sip->sa.sa_family == AF_INET )
			{
				array = subtest->needDNSv4 ? subtest->correctResults : subtest->duplicateResults;
				subtest->needDNSv4 = false;
				
				noSuchRecordStr = kNoSuchRecordAStr;
			}
			else
			{
				array = subtest->needDNSv6 ? subtest->correctResults : subtest->duplicateResults;
				subtest->needDNSv6 = false;
				
				noSuchRecordStr = kNoSuchRecordAAAAStr;
			}
			err = CFPropertyListAppendFormatted( kCFAllocatorDefault, array, "%s", noSuchRecordStr );
			require_noerr( err, fatal );
		}
		else if( !inError )
		{
			err = CFPropertyListAppendFormatted( kCFAllocatorDefault, subtest->unexpectedResults, "%##a", sip );
			require_noerr( err, fatal );
		}
		else
		{
			err = inError;
			goto exit;
		}
	}
	else
	{
		if( !inError )
		{
			CFMutableArrayRef		array = NULL;	
			
			if( sip->sa.sa_family == AF_INET )
			{
				const uint32_t		addrV4 = sip->v4.sin_addr.s_addr;
				
				if( subtest->hasDNSv4 && ( addrV4 == subtest->addrDNSv4 ) )
				{
					array = subtest->needDNSv4 ? subtest->correctResults : subtest->duplicateResults;
					subtest->needDNSv4 = false;
				}
				else if( subtest->hasMDNSv4 && ( addrV4 == subtest->addrMDNSv4 ) )
				{
					array = subtest->needMDNSv4 ? subtest->correctResults : subtest->duplicateResults;
					subtest->needMDNSv4 = false;
				}
			}
			else
			{
				const uint8_t * const		addrV6 = sip->v6.sin6_addr.s6_addr;
				
				if( subtest->hasDNSv6 && ( memcmp( addrV6, subtest->addrDNSv6, 16 ) == 0 ) )
				{
					array = subtest->needDNSv6 ? subtest->correctResults : subtest->duplicateResults;
					subtest->needDNSv6 = false;
				}
				else if( subtest->hasMDNSv6 && ( memcmp( addrV6, subtest->addrMDNSv6, 16 ) == 0 ) )
				{
					array = subtest->needMDNSv6 ? subtest->correctResults : subtest->duplicateResults;
					subtest->needMDNSv6 = false;
				}
			}
			if( !array ) array = subtest->unexpectedResults;
			err = CFPropertyListAppendFormatted( kCFAllocatorDefault, array, "%##a", sip );
			require_noerr( err, fatal );
		}
		else if( inError == kDNSServiceErr_NoSuchRecord )
		{
			err = CFPropertyListAppendFormatted( kCFAllocatorDefault, subtest->unexpectedResults, "%s",
				( sip->sa.sa_family == AF_INET ) ? kNoSuchRecordAStr : kNoSuchRecordAAAAStr );
			require_noerr( err, fatal );
		}
		else
		{
			err = inError;
			goto exit;
		}
	}
	
exit:
	if( err )
	{
		subtest->error = err;
		_DotLocalTestStateMachine( context );
	}
	return;
	
fatal:
	ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//	_DotLocalTestQueryRecordCallback
//===========================================================================================================================

static void DNSSD_API
	_DotLocalTestQueryRecordCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		uint16_t				inType,
		uint16_t				inClass,
		uint16_t				inRDataLen,
		const void *			inRDataPtr,
		uint32_t				inTTL,
		void *					inContext )
{
	OSStatus							err;
	DotLocalTestContext * const			context = (DotLocalTestContext *) inContext;
	DotLocalSubtest * const				subtest = context->subtest;
	const dns_fixed_fields_srv *		fields;
	const uint8_t *						target;
	const uint8_t *						ptr;
	const uint8_t *						end;
	char *								rdataStr;
	unsigned int						priority, weight, port;
	CFMutableArrayRef					array;
	
	Unused( inSDRef );
	Unused( inInterfaceIndex );
	Unused( inFullName );
	Unused( inTTL );
	
	check( context->state == kDotLocalTestState_QuerySRV );
	
	err = inError;
	require_noerr_quiet( err, exit );
	require_action_quiet( inFlags & kDNSServiceFlagsAdd, exit, err = kFlagErr );
	require_action_quiet( ( inType == kDNSServiceType_SRV ) && ( inClass == kDNSServiceClass_IN ), exit, err = kTypeErr );
	require_action_quiet( inRDataLen > sizeof( dns_fixed_fields_srv ), exit, err = kSizeErr );
	
	fields		= (const dns_fixed_fields_srv *) inRDataPtr;
	priority	= dns_fixed_fields_srv_get_priority( fields );
	weight		= dns_fixed_fields_srv_get_weight( fields );
	port		= dns_fixed_fields_srv_get_port( fields );
	target		= (const uint8_t *) &fields[ 1 ];
	end			= ( (const uint8_t *) inRDataPtr ) + inRDataLen;
	for( ptr = target; ( ptr < end ) && ( *ptr != 0 ); ptr += ( 1 + *ptr ) ) {}
	
	if( ( priority == kDotLocalTestSRV_Priority ) &&
		( weight   == kDotLocalTestSRV_Weight )   &&
		( port     == kDotLocalTestSRV_Port )     &&
		( ptr < end ) && DomainNameEqual( target, kDotLocalTestSRV_TargetName ) )
	{
		array = subtest->needSRV ? subtest->correctResults : subtest->duplicateResults;
		subtest->needSRV = false;
	}
	else
	{
		array = subtest->unexpectedResults;
	}
	
	rdataStr = NULL;
	DNSRecordDataToString( inRDataPtr, inRDataLen, kDNSServiceType_SRV, NULL, 0, &rdataStr );
	if( !rdataStr )
	{
		ASPrintF( &rdataStr, "%#H", inRDataPtr, inRDataLen, inRDataLen );
		require_action( rdataStr, fatal, err = kNoMemoryErr );
	}
	
	err = CFPropertyListAppendFormatted( kCFAllocatorDefault, array, "%s", rdataStr );
	free( rdataStr );
	require_noerr( err, fatal );
	
exit:
	if( err )
	{
		subtest->error = err;
		_DotLocalTestStateMachine( context );
	}
	return;
	
fatal:
	ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//	ProbeConflictTestCmd
//===========================================================================================================================

#define kProbeConflictTestService_DefaultName		"pctest-name"
#define kProbeConflictTestService_Port				60000

#define kProbeConflictTestTXTPtr		"\x13" "PROBE-CONFLICT-TEST"
#define kProbeConflictTestTXTLen		sizeof_string( kProbeConflictTestTXTPtr )

typedef struct
{
	const char *		description;
	const char *		program;
	Boolean				expectsRename;
	
}	ProbeConflictTestCase;

#define kPCTProgPreWait			"wait 1000;"	// Wait 1 second before sending gratuitous response.
#define kPCTProgPostWait		"wait 8000;"	// Wait 8 seconds after sending gratuitous response.
												// This allows ~2.75 seconds for probing and ~5 seconds for a rename.

static const ProbeConflictTestCase		kProbeConflictTestCases[] =
{
	// No conflicts
	
	{ "No probe conflicts.",                       kPCTProgPreWait "probes n-n-n;"       "send;" kPCTProgPostWait, false },
	
	// One multicast probe conflict
	
	{ "One multicast probe conflict (1).",         kPCTProgPreWait "probes m;"           "send;" kPCTProgPostWait, false },
	{ "One multicast probe conflict (2).",         kPCTProgPreWait "probes n-m;"         "send;" kPCTProgPostWait, false },
	{ "One multicast probe conflict (3).",         kPCTProgPreWait "probes n-n-m;"       "send;" kPCTProgPostWait, false },
	
	// One unicast probe conflict
	
	{ "One unicast probe conflict (1).",           kPCTProgPreWait "probes u;"           "send;" kPCTProgPostWait, true },
	{ "One unicast probe conflict (2).",           kPCTProgPreWait "probes n-u;"         "send;" kPCTProgPostWait, true },
	{ "One unicast probe conflict (3).",           kPCTProgPreWait "probes n-n-u;"       "send;" kPCTProgPostWait, true },
	
	// One multicast and one unicast probe conflict
	
	{ "Multicast and unicast probe conflict (1).", kPCTProgPreWait "probes m-u;"         "send;" kPCTProgPostWait, true },
	{ "Multicast and unicast probe conflict (2).", kPCTProgPreWait "probes m-n-u;"       "send;" kPCTProgPostWait, true },
	{ "Multicast and unicast probe conflict (3).", kPCTProgPreWait "probes m-n-n-u;"     "send;" kPCTProgPostWait, true },
	{ "Multicast and unicast probe conflict (4).", kPCTProgPreWait "probes n-m-u;"       "send;" kPCTProgPostWait, true },
	{ "Multicast and unicast probe conflict (5).", kPCTProgPreWait "probes n-m-n-u;"     "send;" kPCTProgPostWait, true },
	{ "Multicast and unicast probe conflict (6).", kPCTProgPreWait "probes n-m-n-n-u;"   "send;" kPCTProgPostWait, true },
	{ "Multicast and unicast probe conflict (7).", kPCTProgPreWait "probes n-n-m-u;"     "send;" kPCTProgPostWait, true },
	{ "Multicast and unicast probe conflict (8).", kPCTProgPreWait "probes n-n-m-n-u;"   "send;" kPCTProgPostWait, true },
	{ "Multicast and unicast probe conflict (9).", kPCTProgPreWait "probes n-n-m-n-n-u;" "send;" kPCTProgPostWait, true },
	
	// Two multicast probe conflicts
	
	{ "Two multicast probe conflicts (1).",        kPCTProgPreWait "probes m-m;"         "send;" kPCTProgPostWait, true },
	{ "Two multicast probe conflicts (2).",        kPCTProgPreWait "probes m-n-m;"       "send;" kPCTProgPostWait, true },
	{ "Two multicast probe conflicts (3).",        kPCTProgPreWait "probes m-n-n-m;"     "send;" kPCTProgPostWait, true },
	{ "Two multicast probe conflicts (4).",        kPCTProgPreWait "probes n-m-m;"       "send;" kPCTProgPostWait, true },
	{ "Two multicast probe conflicts (5).",        kPCTProgPreWait "probes n-m-n-m-n;"   "send;" kPCTProgPostWait, true },
	{ "Two multicast probe conflicts (6).",        kPCTProgPreWait "probes n-m-n-n-m;"   "send;" kPCTProgPostWait, true },
	{ "Two multicast probe conflicts (7).",        kPCTProgPreWait "probes n-n-m-m;"     "send;" kPCTProgPostWait, true },
	{ "Two multicast probe conflicts (8).",        kPCTProgPreWait "probes n-n-m-n-m;"   "send;" kPCTProgPostWait, true },
	{ "Two multicast probe conflicts (9).",        kPCTProgPreWait "probes n-n-m-n-n-m;" "send;" kPCTProgPostWait, true },
};

#define kProbeConflictTestCaseCount		countof( kProbeConflictTestCases )

typedef struct
{
	DNSServiceRef			registration;	// Test service registration.
	NanoTime64				testStartTime;	// Test's start time.
	NanoTime64				startTime;		// Current test case's start time.
	MDNSColliderRef			collider;		// mDNS collider object.
	CFMutableArrayRef		results;		// Array of test case results.
	char *					serviceName;	// Test service's instance name as a string. (malloced)
	char *					serviceType;	// Test service's service type as a string. (malloced)
	uint8_t *				recordName;		// FQDN of collider's record (same as test service's SRV+TXT records). (malloced)
	unsigned int			testCaseIndex;	// Index of the current test case.
	uint32_t				ifIndex;		// Index of the interface that the collider is to operate on.
	char *					outputFilePath;	// File to write test results to. If NULL, then write to stdout. (malloced)
	OutputFormatType		outputFormat;	// Format of test report output.
	Boolean					registered;		// True if the test service instance is currently registered.
	Boolean					testFailed;		// True if at least one test case failed.
	
}	ProbeConflictTestContext;

static void DNSSD_API
	_ProbeConflictTestRegisterCallback(
		DNSServiceRef		inSDRef,
		DNSServiceFlags		inFlags,
		DNSServiceErrorType	inError,
		const char *		inName,
		const char *		inType,
		const char *		inDomain,
		void *				inContext );
static void		_ProbeConflictTestColliderStopHandler( void *inContext, OSStatus inError );
static OSStatus	_ProbeConflictTestStartNextTest( ProbeConflictTestContext *inContext );
static OSStatus	_ProbeConflictTestStopCurrentTest( ProbeConflictTestContext *inContext, Boolean inRenamed );
static void		_ProbeConflictTestFinalizeAndExit( ProbeConflictTestContext *inContext ) ATTRIBUTE_NORETURN;

static void	ProbeConflictTestCmd( void )
{
	OSStatus						err;
	ProbeConflictTestContext *		context;
	const char *					serviceName;
	char							tag[ 6 + 1 ];
	
	context = (ProbeConflictTestContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	if( gProbeConflictTest_Interface )
	{
		err = InterfaceIndexFromArgString( gProbeConflictTest_Interface, &context->ifIndex );
		require_noerr_quiet( err, exit );
	}
	else
	{
		err = _MDNSInterfaceGetAny( kMDNSInterfaceSubset_All, NULL, &context->ifIndex );
		require_noerr_quiet( err, exit );
	}
	
	if( gProbeConflictTest_OutputFilePath )
	{
		context->outputFilePath = strdup( gProbeConflictTest_OutputFilePath );
		require_action( context->outputFilePath, exit, err = kNoMemoryErr );
	}
	
	err = OutputFormatFromArgString( gProbeConflictTest_OutputFormat, &context->outputFormat );
	require_noerr_quiet( err, exit );
	
	context->results = CFArrayCreateMutable( NULL, kProbeConflictTestCaseCount, &kCFTypeArrayCallBacks );
	require_action( context->results, exit, err = kNoMemoryErr );
	
	serviceName = gProbeConflictTest_UseComputerName ? NULL : kProbeConflictTestService_DefaultName;
	
	ASPrintF( &context->serviceType, "_pctest-%s._udp",
		_RandomStringExact( kLowerAlphaNumericCharSet, kLowerAlphaNumericCharSetSize, sizeof( tag ) - 1, tag ) );
	require_action( context->serviceType, exit, err = kNoMemoryErr );
	
	context->testStartTime = NanoTimeGetCurrent();
	err = DNSServiceRegister( &context->registration, 0, context->ifIndex, serviceName, context->serviceType, "local.",
		NULL, htons( kProbeConflictTestService_Port ), 0, NULL, _ProbeConflictTestRegisterCallback, context );
	require_noerr( err, exit );
	
	err = DNSServiceSetDispatchQueue( context->registration, dispatch_get_main_queue() );
	require_noerr( err, exit );
	
	dispatch_main();
	
exit:
	exit( 1 );
}

//===========================================================================================================================
//	_ProbeConflictTestRegisterCallback
//===========================================================================================================================

static void DNSSD_API
	_ProbeConflictTestRegisterCallback(
		DNSServiceRef		inSDRef,
		DNSServiceFlags		inFlags,
		DNSServiceErrorType	inError,
		const char *		inName,
		const char *		inType,
		const char *		inDomain,
		void *				inContext )
{
	OSStatus								err;
	ProbeConflictTestContext * const		context = (ProbeConflictTestContext *) inContext;
	
	Unused( inSDRef );
	Unused( inType );
	Unused( inDomain );
	
	err = inError;
	require_noerr( err, exit );
	
	if( !context->registered )
	{
		if( inFlags & kDNSServiceFlagsAdd )
		{
			uint8_t *			ptr;
			size_t				recordNameLen;
			unsigned int		len;
			uint8_t				name[ kDomainNameLengthMax ];
			
			context->registered = true;
			
			FreeNullSafe( context->serviceName );
			context->serviceName = strdup( inName );
			require_action( context->serviceName, exit, err = kNoMemoryErr );
			
			err = DomainNameFromString( name, context->serviceName, NULL );
			require_noerr( err, exit );
			
			err = DomainNameAppendString( name, context->serviceType, NULL );
			require_noerr( err, exit );
			
			err = DomainNameAppendString( name, "local", NULL );
			require_noerr( err, exit );
			
			ForgetMem( &context->recordName );
			err = DomainNameDup( name, &context->recordName, &recordNameLen );
			require_noerr( err, exit );
			require_fatal( recordNameLen > 0, "Record name length is zero." );	// Prevents dubious static analyzer warning.
			
			// Make the first label all caps so that it's easier to spot in system logs.
			
			ptr = context->recordName;
			for( len = *ptr++; len > 0; --len, ++ptr ) *ptr = (uint8_t) toupper_safe( *ptr );
			
			err = _ProbeConflictTestStartNextTest( context );
			require_noerr( err, exit );
		}
	}
	else
	{
		if( !( inFlags & kDNSServiceFlagsAdd ) )
		{
			context->registered = false;
			err = _ProbeConflictTestStopCurrentTest( context, true );
			require_noerr( err, exit );
		}
	}
	err = kNoErr;
	
exit:
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	_ProbeConflictTestColliderStopHandler
//===========================================================================================================================

static void	_ProbeConflictTestColliderStopHandler( void *inContext, OSStatus inError )
{
	OSStatus								err;
	ProbeConflictTestContext * const		context = (ProbeConflictTestContext *) inContext;
	
	err = inError;
	require_noerr_quiet( err, exit );
	
	ForgetCF( &context->collider );
	
	err = _ProbeConflictTestStopCurrentTest( context, false );
	require_noerr( err, exit );
	
	err = _ProbeConflictTestStartNextTest( context );
	require_noerr( err, exit );
	
exit:
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	_ProbeConflictTestStartNextTest
//===========================================================================================================================

static OSStatus	_ProbeConflictTestStartNextTest( ProbeConflictTestContext *inContext )
{
	OSStatus							err;
	const ProbeConflictTestCase *		testCase;
	
	check( !inContext->collider );
	
	if( inContext->testCaseIndex < kProbeConflictTestCaseCount )
	{
		testCase = &kProbeConflictTestCases[ inContext->testCaseIndex ];
	}
	else
	{
		_ProbeConflictTestFinalizeAndExit( inContext );
	}
	
	err = MDNSColliderCreate( dispatch_get_main_queue(), &inContext->collider );
	require_noerr( err, exit );
	
	err = MDNSColliderSetProgram( inContext->collider, testCase->program );
	require_noerr( err, exit );
	
	err = MDNSColliderSetRecord( inContext->collider, inContext->recordName, kDNSServiceType_TXT,
		kProbeConflictTestTXTPtr, kProbeConflictTestTXTLen );
	require_noerr( err, exit );
	
	MDNSColliderSetProtocols( inContext->collider, kMDNSColliderProtocol_IPv4 );
	MDNSColliderSetInterfaceIndex( inContext->collider, inContext->ifIndex );
	MDNSColliderSetStopHandler( inContext->collider, _ProbeConflictTestColliderStopHandler, inContext );
	
	inContext->startTime = NanoTimeGetCurrent();
	err = MDNSColliderStart( inContext->collider );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_ProbeConflictTestStopCurrentTest
//===========================================================================================================================

#define kProbeConflictTestCaseResultKey_Description			CFSTR( "description" )
#define kProbeConflictTestCaseResultKey_StartTime			CFSTR( "startTime" )
#define kProbeConflictTestCaseResultKey_EndTime				CFSTR( "endTime" )
#define kProbeConflictTestCaseResultKey_ExpectedRename		CFSTR( "expectedRename" )
#define kProbeConflictTestCaseResultKey_ServiceName			CFSTR( "serviceName" )
#define kProbeConflictTestCaseResultKey_Passed				CFSTR( "passed" )

static OSStatus	_ProbeConflictTestStopCurrentTest( ProbeConflictTestContext *inContext, Boolean inRenamed )
{
	OSStatus							err;
	const ProbeConflictTestCase *		testCase;
	NanoTime64							now;
	Boolean								passed;
	char								startTime[ 32 ];
	char								endTime[ 32 ];
	
	now = NanoTimeGetCurrent();
	
	if( inContext->collider )
	{
		MDNSColliderSetStopHandler( inContext->collider, NULL, NULL );
		MDNSColliderStop( inContext->collider );
		CFRelease( inContext->collider );
		inContext->collider = NULL;
	}
	
	testCase = &kProbeConflictTestCases[ inContext->testCaseIndex ];
	passed = ( ( testCase->expectsRename && inRenamed ) || ( !testCase->expectsRename && !inRenamed ) ) ? true : false;
	if( !passed ) inContext->testFailed = true;
	
	_NanoTime64ToTimestamp( inContext->startTime, startTime, sizeof( startTime ) );
	_NanoTime64ToTimestamp( now, endTime, sizeof( endTime ) );
	
	err = CFPropertyListAppendFormatted( kCFAllocatorDefault, inContext->results,
		"{"
			"%kO=%s"	// description
			"%kO=%b"	// expectedRename
			"%kO=%s"	// startTime
			"%kO=%s"	// endTime
			"%kO=%s"	// serviceName
			"%kO=%b"	// passed
		"}",
		kProbeConflictTestCaseResultKey_Description,	testCase->description,
		kProbeConflictTestCaseResultKey_ExpectedRename,	testCase->expectsRename,
		kProbeConflictTestCaseResultKey_StartTime,		startTime,
		kProbeConflictTestCaseResultKey_EndTime,		endTime,
		kProbeConflictTestCaseResultKey_ServiceName,	inContext->serviceName,
		kProbeConflictTestCaseResultKey_Passed,			passed );
	require_noerr( err, exit );
	
	++inContext->testCaseIndex;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_ProbeConflictTestFinalizeAndExit
//===========================================================================================================================

#define kProbeConflictTestReportKey_StartTime		CFSTR( "startTime" )
#define kProbeConflictTestReportKey_EndTime			CFSTR( "endTime" )
#define kProbeConflictTestReportKey_ServiceType		CFSTR( "serviceType" )
#define kProbeConflictTestReportKey_Results			CFSTR( "results" )
#define kProbeConflictTestReportKey_Passed			CFSTR( "passed" )

static void	_ProbeConflictTestFinalizeAndExit( ProbeConflictTestContext *inContext )
{
	OSStatus				err;
	CFPropertyListRef		plist;
	NanoTime64				now;
	char					startTime[ 32 ];
	char					endTime[ 32 ];
	
	now = NanoTimeGetCurrent();
	
	check( !inContext->collider );
	
	_NanoTime64ToTimestamp( inContext->testStartTime, startTime, sizeof( startTime ) );
	_NanoTime64ToTimestamp( now, endTime, sizeof( endTime ) );
	
	err = CFPropertyListCreateFormatted( kCFAllocatorDefault, &plist,
		"{"
			"%kO=%s"	// startTime
			"%kO=%s"	// endTime
			"%kO=%s"	// serviceType
			"%kO=%O"	// results
			"%kO=%b"	// passed
		"}",
		kProbeConflictTestReportKey_StartTime,		startTime,
		kProbeConflictTestReportKey_EndTime,		endTime,
		kProbeConflictTestReportKey_ServiceType,	inContext->serviceType,
		kProbeConflictTestReportKey_Results,		inContext->results,
		kProbeConflictTestReportKey_Passed,			inContext->testFailed ? false : true );
	require_noerr( err, exit );
	ForgetCF( &inContext->results );
	
	err = OutputPropertyList( plist, inContext->outputFormat, inContext->outputFilePath );
	CFRelease( plist );
	require_noerr( err, exit );
	
	exit( inContext->testFailed ? 2 : 0 );
	
exit:
	ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//    ExpensiveConstrainedsTestCmd
//===========================================================================================================================

#define NOTIFICATION_TIME_THRESHOLD 1500    // The maximum wating time allowed before notification happens
#define TEST_REPETITION             2       // the number of repetition that one test has to passed
#define LOOPBACK_INTERFACE_NAME     "lo0"
#define WIFI_TEST_QUESTION_NAME     "www.example.com"
#define EXPENSIVE_CONSTRAINED_MAX_RETRIES 1
#define EXPENSIVE_CONSTRAINED_TEST_INTERVAL 5
// Use "-n tag-expensive-test.ttl-86400.d.test." to run the test locally
// #define LOOPBACK_TEST_QUESTION_NAME "tag-expensive-test.ttl-86400.d.test."

#define EXPENSIVE_CONSTRAINED_TEST_REPORT_KEY_START_TIME                CFSTR( "Start Time" )
#define EXPENSIVE_CONSTRAINED_TEST_REPORT_KEY_END_TIME                  CFSTR( "End Time" )
#define EXPENSIVE_CONSTRAINED_TEST_REPORT_KEY_ALL_PASSED                CFSTR( "All Tests Passed" )
#define EXPENSIVE_CONSTRAINED_TEST_REPORT_KEY_SUBTEST_RESULT            CFSTR( "Subtest Results" )

#define EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_START_TIME             CFSTR( "Start Time" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_END_TIME               CFSTR( "End Time" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_QNAME                  CFSTR( "Question Name" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_FLAGS                  CFSTR( "DNS Service Flags" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_PROTOCOLS              CFSTR( "Protocols" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_INTERFACE_INDEX        CFSTR( "Interface Index" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_INTERFACE_NAME         CFSTR( "Interface Name" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_RESULT                 CFSTR( "Result" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_ERROR                  CFSTR( "Error Description" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_TEST_PROGRESS          CFSTR( "Test Progress" )

#define EXPENSIVE_CONSTRAINED_SUBTEST_PROGRESS_KEY_START_TIME           CFSTR( "Start Time" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_PROGRESS_KEY_END_TIME             CFSTR( "End Time" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_PROGRESS_KEY_STATE                CFSTR( "State" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_PROGRESS_KEY_EXPECT_RESULT        CFSTR( "Expected Result" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_PROGRESS_KEY_ACTUAL_RESULT        CFSTR( "Actual Result" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_PROGRESS_KEY_EXPENSIVE_PREV_NOW   CFSTR( "Expensive Prev->Now" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_PROGRESS_KEY_CONSTRAINED_PREV_NOW CFSTR( "Constrained Prev->Now" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_PROGRESS_KEY_CALL_BACK            CFSTR( "Call Back" )

#define EXPENSIVE_CONSTRAINED_SUBTEST_ACTUAL_RESULT_KEY_TIMESTAMP       CFSTR( "Timestamp" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_ACTUAL_RESULT_KEY_NAME            CFSTR( "Answer Name" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_ACTUAL_RESULT_KEY_FLAGS           CFSTR( "Add or Remove" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_ACTUAL_RESULT_KEY_INTERFACE       CFSTR( "Interface Index" )
#define EXPENSIVE_CONSTRAINED_SUBTEST_ACTUAL_RESULT_KEY_ADDRESS         CFSTR( "Address" )

// All the states that ends with _PREPARE represents the state where the test state is reset and initialized.
enum ExpensiveConstrainedTestState
{
    TEST_BEGIN,
    TEST_EXPENSIVE_PREPARE,
    TEST_EXPENSIVE,                     // Test if mDNSResponder can handle "expensive" status change of the corresponding interface
    TEST_CONSTRAINED_PREPARE,
    TEST_CONSTRAINED,                   // Test if mDNSResponder can handle "constrained" status change of the corresponding interface
    TEST_EXPENSIVE_CONSTRAINED_PREPARE,
    TEST_EXPENSIVE_CONSTRAINED,          // Test if mDNSResponder can handle "expensive" and "constrained" status change of the corresponding interface at the same time
    TEST_FAILED,
    TEST_SUCCEEDED
};
enum ExpensiveConstrainedTestOperation
{
    RESULT_ADD, // received response for the given query, which means mDNSResponder is able to send out the query over the interface, because the interface status is changed.
    RESULT_RMV, // received negative response for the given query, which means mDNSResponder is not able to send out the query over the interface, because the interface status is changed.
    NO_UPDATE   // no status update notification
};

typedef struct
{
    uint32_t                    subtestIndex;           // The index of parameter for the subtest
    DNSServiceRef               opRef;                  // sdRef for the DNSServiceGetAddrInfo operation.
    const char *                name;                   // Hostname to resolve.
    DNSServiceFlags             flags;                  // Flags argument for DNSServiceGetAddrInfo().
    DNSServiceProtocol          protocols;              // Protocols argument for DNSServiceGetAddrInfo().
    uint32_t                    ifIndex;                // Interface index argument for DNSServiceGetAddrInfo().
    char                        ifName[IFNAMSIZ];       // Interface name for the given interface index.
    dispatch_source_t           timer;                  // The test will check if the current behavior is valid, which is called by
                                                        // the timer per 2s.
    pid_t                       serverPID;
    Boolean                     isExpensivePrev;        // If the interface is expensive in the previous test step.
    Boolean                     isExpensiveNow;         // If the interface is expensive now.
    Boolean                     isConstrainedPrev;      // If the interface is constrained in the previous test step.
    Boolean                     isConstrainedNow;       // If the interface is constrained now.
    Boolean                     startFromExpensive;     // All the test will start from expensive/constrained interface, so there won's be an answer until the interface is changed.
    uint8_t                     numOfRetries;           // the number of retries we can have if the test fail
    struct timeval              updateTime;             // The time when interface status(expensive or constrained) is changed.
    struct timeval              notificationTime;       // The time when callback function, which is passed to DNSServiceGetAddrInfo, gets called.
    uint32_t                    counter;                // To record how many times the test has repeated.
    enum ExpensiveConstrainedTestState          state;              // The current test state.
    enum ExpensiveConstrainedTestOperation      expectedOperation;  // the test expects this kind of notification
    enum ExpensiveConstrainedTestOperation      operation;          // represents what notification the callback function gets.

    NanoTime64                  testReport_startTime;       // when the entire test starts
    CFMutableArrayRef           subtestReport;              // stores the log message for every subtest
    NanoTime64                  subtestReport_startTime;    // when the subtest starts
    CFMutableArrayRef           subtestProgress;            // one test iteration
    NanoTime64                  subtestProgress_startTime;  // when the test iteration starts
    CFMutableArrayRef           subtestProgress_callBack;   // array of ADD/REMOVE events
    char *                      outputFilePath;             // File to write test results to. If NULL, then write to stdout. (malloced)
    OutputFormatType            outputFormat;               // Format of test report output.
} ExpensiveConstrainedContext;

// structure that controls how the subtest is run
typedef struct
{
    const char  *qname;                 // the name of the query, when the ends with ".d.test.", test will send query to local DNS server
    Boolean     deny_expensive;         // if the query should avoid using expensive interface
    Boolean     deny_constrained;       // if the query should avoid using constrained interface
    Boolean     start_from_expensive;   // if the query should starts from using an expensive interface
    Boolean     ipv4_query;             // only allow IPv4 query
    Boolean     ipv6_query;             // only allow IPv6 query
    int8_t      test_passed;            // if the subtest passes
} ExpensiveConstrainedTestParams;

static ExpensiveConstrainedTestParams   ExpensiveConstrainedSubtestParams[] =
{
//  qname                                                   deny_expensive  deny_constrained    start_from_expensive    ipv4_query  ipv6_query
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    true,           false,              false,                  true,       true,       -1},
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    true,           false,              true,                   true,       true,       -1},
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    false,          true,               false,                  true,       true,       -1},
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    false,          true,               true,                   true,       true,       -1},
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    true,           true,               false,                  true,       true,       -1},
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    true,           true,               true,                   true,       true,       -1},
// IPv4 Only
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    true,           false,              false,                  true,       false,      -1},
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    true,           false,              true,                   true,       false,      -1},
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    false,          true,               false,                  true,       false,      -1},
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    false,          true,               true,                   true,       false,      -1},
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    true,           true,               false,                  true,       false,      -1},
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    true,           true,               true,                   true,       false,      -1},
// IPv6 Only
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    true,           false,              false,                  false,      true,       -1},
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    true,           false,              true,                   false,      true,       -1},
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    false,          true,               false,                  false,      true,       -1},
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    false,          true,               true,                   false,      true,       -1},
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    true,           true,               false,                  false,      true,       -1},
    {"tag-expensive_constrained-test.ttl-86400.d.test.",    true,           true,               true,                   false,      true,       -1}
};

static void ExpensiveConstrainedSetupLocalDNSServer( ExpensiveConstrainedContext *context );
static void ExpensiveConstrainedStartTestHandler( ExpensiveConstrainedContext *context );
static void ExpensiveConstrainedStopTestHandler( ExpensiveConstrainedContext *context );
static void ExpensiveConstrainedSetupTimer( ExpensiveConstrainedContext *context, uint32_t second );
static void ExpensiveConstrainedTestTimerEventHandler( ExpensiveConstrainedContext *context );
static void DNSSD_API
    ExpensiveConstrainedCallback(
        DNSServiceRef           inSDRef,
        DNSServiceFlags         inFlags,
        uint32_t                inInterfaceIndex,
        DNSServiceErrorType     inError,
        const char *            inHostname,
        const struct sockaddr * inSockAddr,
        uint32_t                inTTL,
        void *                  inContext );
static void ExpensiveConstrainedInitializeContext( ExpensiveConstrainedContext *context );
static void ExpensiveConstrainedStopAndCleanTheTest( ExpensiveConstrainedContext *context );
static void ExpensiveConstrainedSubtestProgressReport( ExpensiveConstrainedContext *context );
static void ExpensiveConstrainedSubtestReport( ExpensiveConstrainedContext *context, const char *error_description );
static void ExpensiveConstrainedFinalResultReport( ExpensiveConstrainedContext *context, Boolean allPassed );
static const char *ExpensiveConstrainedProtocolString(DNSServiceProtocol protocol);
static const char *ExpensiveConstrainedStateString(enum ExpensiveConstrainedTestState state);
static const char *ExpensiveConstrainedOperationString(enum ExpensiveConstrainedTestOperation operation);
static Boolean expensiveConstrainedEndsWith( const char *str, const char *suffix );

//===========================================================================================================================
//    ExpensiveConstrainedTestCmd
//===========================================================================================================================

static void ExpensiveConstrainedTestCmd( void )
{
    OSStatus                        err;
    dispatch_source_t               signalSource   = NULL;
    ExpensiveConstrainedContext *   context        = NULL;

    // Set up SIGINT handler.
    signal( SIGINT, SIG_IGN );
    err = DispatchSignalSourceCreate( SIGINT, dispatch_get_main_queue(), Exit, kExitReason_SIGINT, &signalSource );
    require_noerr( err, exit );
    dispatch_resume( signalSource );

    // create the test context
    context = (ExpensiveConstrainedContext *) calloc( 1, sizeof(*context) );
    require_action( context, exit, err = kNoMemoryErr );

    // get the command line option
    err = OutputFormatFromArgString( gExpensiveConstrainedTest_OutputFormat, &context->outputFormat );
    require_noerr_quiet( err, exit );
    if ( gExpensiveConstrainedTest_OutputFilePath )
    {
        context->outputFilePath = strdup( gExpensiveConstrainedTest_OutputFilePath );
        require_noerr_quiet( context->outputFilePath, exit );
    }

    // initialize context
    context->subtestIndex = 0;
    context->numOfRetries = EXPENSIVE_CONSTRAINED_MAX_RETRIES;

    // initialize the CFArray used to store the log
    context->subtestReport = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
    context->testReport_startTime = NanoTimeGetCurrent();

    // setup local DNS server
    ExpensiveConstrainedSetupLocalDNSServer( context );

    ExpensiveConstrainedStartTestHandler( context );

    dispatch_main();

exit:
    exit( 1 );
}

//===========================================================================================================================
//    ExpensiveConstrainedSetupLocalDNSServer
//===========================================================================================================================

static void ExpensiveConstrainedSetupLocalDNSServer( ExpensiveConstrainedContext *context )
{
    pid_t current_pid = getpid();
    OSStatus err = SpawnCommand( &context->serverPID, "dnssdutil server -l --follow %d", current_pid );
    if (err != 0)
    {
        FPrintF( stdout, "dnssdutil server -l --follow <PID> failed, error: %d\n", err );
        exit( 1 );
    }
    sleep(2);
}

//===========================================================================================================================
//    ExpensiveConstrainedStartTestHandler
//===========================================================================================================================

static void ExpensiveConstrainedStartTestHandler( ExpensiveConstrainedContext *context )
{
    // setup 3s timer
    ExpensiveConstrainedSetupTimer( context, EXPENSIVE_CONSTRAINED_TEST_INTERVAL );

    // set the event handler for the 3s timer
    dispatch_source_set_event_handler( context->timer, ^{
        ExpensiveConstrainedTestTimerEventHandler( context );
    } );

    dispatch_resume( context->timer );
}

//===========================================================================================================================
//    ExpensiveConstrainedStartTestHandler
//===========================================================================================================================

static void ExpensiveConstrainedStopTestHandler( ExpensiveConstrainedContext *context )
{
    dispatch_cancel( context->timer );
    dispatch_release( context->timer );
    context->timer = NULL;
}

//===========================================================================================================================
//    ExpensiveConstrainedSetupTimer
//===========================================================================================================================

static void ExpensiveConstrainedSetupTimer( ExpensiveConstrainedContext *context, uint32_t second )
{
    // set the timer source, the event handler will be called for every "second" seconds
    context->timer = dispatch_source_create( DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue() );
    if ( context->timer == NULL )
    {
        FPrintF( stdout, "dispatch_source_create:DISPATCH_SOURCE_TYPE_TIMER failed\n" );
        exit( 1 );
    }
    // the first block will be put into the queue "second"s after calling dispatch_resume
    dispatch_source_set_timer( context->timer, dispatch_time( DISPATCH_TIME_NOW, second * NSEC_PER_SEC ),
                               (unsigned long long)(second) * NSEC_PER_SEC, 100ull * NSEC_PER_MSEC );
}

//===========================================================================================================================
//    ExpensiveConstrainedTestTimerEventHandler
//===========================================================================================================================

static void ExpensiveConstrainedTestTimerEventHandler( ExpensiveConstrainedContext *context )
{
    OSStatus    err;
    char        buffer[ 1024 ];
    const char *errorDescription = NULL;

    // do not log the state if we are in transition state
    if (context->state != TEST_BEGIN
        && context->state != TEST_SUCCEEDED
        && context->state != TEST_CONSTRAINED_PREPARE
        && context->state != TEST_EXPENSIVE_CONSTRAINED_PREPARE)
        ExpensiveConstrainedSubtestProgressReport( context );

    switch ( context->state ) {
        case TEST_BEGIN:
        {
            ExpensiveConstrainedStopTestHandler( context );

            // clear mDNSResponder cache
            err = systemf( NULL, "killall -HUP mDNSResponder" );
            require_noerr_action( err, test_failed, errorDescription = "systemf failed");

            // initialize the global parameters
            ExpensiveConstrainedInitializeContext( context );

            // The local DNS server is set up on the local only interface.
            gExpensiveConstrainedTest_Interface = LOOPBACK_INTERFACE_NAME;
            strncpy( context->ifName, gExpensiveConstrainedTest_Interface, sizeof( context->ifName ) );

            // The local DNS server is unscoped, so we must set our question to unscoped.
            context->ifIndex = kDNSServiceInterfaceIndexAny;

            // The question name must end with "d.test.", "tag-expensive-test.ttl-86400.d.test." for example, then the test will
            // use the local dns server set up previously to run the test locally.
            require_action( gExpensiveConstrainedTest_Name != NULL && expensiveConstrainedEndsWith( gExpensiveConstrainedTest_Name, "d.test." ), test_failed,
                           SNPrintF( buffer, sizeof( buffer ), "The question name (%s) must end with \"d.test.\".\n", gExpensiveConstrainedTest_Name );
                           errorDescription = buffer );

            // get the quesion name
            context->name = gExpensiveConstrainedTest_Name;

            // set the initial state for the interface
            context->startFromExpensive = gExpensiveConstrainedTest_StartFromExpensive;
            err = systemf( NULL, "ifconfig %s %sexpensive && ifconfig %s -constrained", context->ifName, context->startFromExpensive ? "" : "-", context->ifName );
            require_noerr_action( err, test_failed, errorDescription = "systemf failed");
            sleep( 5 ); // wait for 5s to allow the interface change event de delivered to others

            // get question flag
            if ( gExpensiveConstrainedTest_DenyExpensive )    context->flags     |= kDNSServiceFlagsDenyExpensive;
            if ( gExpensiveConstrainedTest_DenyConstrained )  context->flags     |= kDNSServiceFlagsDenyConstrained;
            if ( gExpensiveConstrainedTest_ProtocolIPv4 )     context->protocols |= kDNSServiceProtocol_IPv4;
            if ( gExpensiveConstrainedTest_ProtocolIPv6 )     context->protocols |= kDNSServiceProtocol_IPv6;

            // prevent mDNSResponder from doing extra path evaluation and changing the interface to others(such as Bluetooth)
            #if( TARGET_OS_WATCH )
            context->flags |= kDNSServiceFlagsPathEvaluationDone;
            #endif

            // start the query
            DNSServiceGetAddrInfo( &context->opRef, context->flags, context->ifIndex, context->protocols, context->name, ExpensiveConstrainedCallback, context );

            // set the initial test status
            context->subtestReport_startTime    = NanoTimeGetCurrent();
            context->subtestProgress_startTime  = NanoTimeGetCurrent();
            context->state                      = TEST_EXPENSIVE_PREPARE; // start from expensive test
            context->isExpensiveNow             = context->startFromExpensive ? true : false;
            context->isConstrainedNow           = false;
            context->expectedOperation          = context->isExpensiveNow && ( context->flags & kDNSServiceFlagsDenyExpensive ) ? NO_UPDATE : RESULT_ADD;
            context->operation                  = NO_UPDATE;
            context->subtestProgress            = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks);
            require_action( context->subtestProgress != NULL, test_failed, errorDescription = "CFArrayCreateMutable failed" );
            context->subtestProgress_callBack   = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks);
            require_action( context->subtestProgress != NULL, test_failed, errorDescription = "CFArrayCreateMutable failed" );

            // set the queue where the callback will be called when there is an answer for the query
            err = DNSServiceSetDispatchQueue( context->opRef, dispatch_get_main_queue() );
            require_noerr( err, test_failed );

            ExpensiveConstrainedStartTestHandler( context );
        }
            break;
        case TEST_EXPENSIVE_PREPARE:
            require_action( context->isConstrainedNow == false, test_failed,
                            SNPrintF( buffer, sizeof( buffer ), "Interface %s should be unconstrained.\n", context->ifName );
                            errorDescription = buffer );
            require_action( context->expectedOperation == context->operation, test_failed,
                            errorDescription = "Operation is not expected" );

            context->subtestProgress_startTime  = NanoTimeGetCurrent();
            context->state                      = TEST_EXPENSIVE;   // begin to test expensive flag
            context->counter                    = 0;                // the number of test repetition that has passed
            context->isExpensivePrev            = context->isExpensiveNow;
            context->isExpensiveNow             = !context->isExpensiveNow; // flip the expensive status
            context->isConstrainedPrev          = false;            // the interface is currently unconstrained
            context->isConstrainedNow           = false;            // the interface will be unconstrained in the current test
            if ( gExpensiveConstrainedTest_DenyExpensive )
                context->expectedOperation      = context->isExpensiveNow ? RESULT_RMV : RESULT_ADD;
            else
                context->expectedOperation      = NO_UPDATE;
            context->operation                  = NO_UPDATE;        // NO_UPDATE means the call back function has not been called
            context->subtestProgress_callBack   = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
            require_action( context->subtestProgress_callBack != NULL, test_failed, errorDescription = "CFArrayCreateMutable failed" );

            err = systemf( NULL, "ifconfig %s %sexpensive", context->ifName, context->isExpensiveNow ? "" : "-" );
            require_noerr_action( err, test_failed, errorDescription = "systemf failed" );

            // record the starting timestamp
            gettimeofday( &context->updateTime, NULL );

            break;
        case TEST_EXPENSIVE:
            // Since we are testing expensive flag, we should always turn the expensive flag on and off.
            require_action( context->isExpensivePrev ^ context->isExpensiveNow, test_failed,
                            SNPrintF( buffer, sizeof( buffer ), "The current expensive status should be different with the previous one: %d -> %d\n", context->isExpensivePrev, context->isExpensiveNow);
                            errorDescription = buffer );
            // constrained flag is always turned off when testing expensive
            require_action( context->isConstrainedNow == false, test_failed,
                            SNPrintF( buffer, sizeof( buffer ), "The interface %s should be unconstrained when testing \"expensive\"\n", context->ifName );
                            errorDescription = buffer );
            require_action( context->expectedOperation == context->operation, test_failed, errorDescription = "Operation is not expected" );

            context->counter++; // one test repetition has passed
            if ( context->counter == TEST_REPETITION ) // expensive test finished
            {
                // prepare to test constrained flag
                context->state = TEST_CONSTRAINED_PREPARE;

                // reset the interface
                err = systemf( NULL, "ifconfig %s -expensive && ifconfig %s -constrained", context->ifName, context->ifName );
                require_noerr_action( err, test_failed, errorDescription = "systemf failed" );

                context->isExpensiveNow = false;
                context->isConstrainedNow = false;
                gettimeofday( &context->updateTime, NULL );
            }
            else
            {
                context->subtestProgress_startTime  = NanoTimeGetCurrent();
                context->isExpensivePrev            = context->isExpensiveNow;
                context->isExpensiveNow             = !context->isExpensiveNow; // flip the expensive status
                if ( gExpensiveConstrainedTest_DenyExpensive )
                    context->expectedOperation      = context->isExpensiveNow ? RESULT_RMV : RESULT_ADD;
                else
                    context->expectedOperation      = NO_UPDATE;
                context->operation                  = NO_UPDATE;
                context->subtestProgress_callBack   = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
                require_action( context->subtestProgress_callBack != NULL, test_failed, errorDescription = "CFArrayCreateMutable failed" );

                err = systemf( NULL, "ifconfig %s %sexpensive", context->ifName, context->isExpensiveNow ? "" : "-" );
                require_noerr_action( err, test_failed, errorDescription = "systemf failed" );

                gettimeofday( &context->updateTime, NULL );
            }
            break;
        case TEST_CONSTRAINED_PREPARE:
            // The interface should be inexpensive and unconstrained when the constrained test starts
            require_action( context->isExpensiveNow == false, test_failed, SNPrintF( buffer, sizeof( buffer ), "Interface %s should be inexpensive.", context->ifName );
                            errorDescription = buffer );
            require_action( context->isConstrainedNow == false, test_failed, SNPrintF( buffer, sizeof( buffer ), "Interface %s should be unconstrained.\n", context->ifName );
                            errorDescription = buffer );

            context->subtestProgress_startTime  = NanoTimeGetCurrent();
            context->state                      = TEST_CONSTRAINED; // constrained interface is now under testing
            context->counter                    = 0;
            context->isExpensivePrev            = false;
            context->isExpensiveNow             = false;
            context->isConstrainedPrev          = false;
            context->isConstrainedNow           = true;             // will set constrained flag on the interface
            if ( gExpensiveConstrainedTest_DenyConstrained )
                context->expectedOperation      = RESULT_RMV;
            else
                context->expectedOperation      = NO_UPDATE;
            context->operation                  = NO_UPDATE;
            context->subtestProgress_callBack   = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
            require_action( context->subtestProgress_callBack != NULL, test_failed, errorDescription = "CFArrayCreateMutable failed" );

            // change interface to the constrained one
            err = systemf( NULL, "ifconfig %s -expensive && ifconfig %s constrained", context->ifName, context->ifName );
            require_noerr_action( err, test_failed, errorDescription = "systemf failed" );

            gettimeofday( &context->updateTime, NULL );
            break;
        case TEST_CONSTRAINED:
            // Since we are testing constrained flag, we should always turn the constrained flag on and off.
            require_action( context->isConstrainedPrev ^ context->isConstrainedNow, test_failed,
                            SNPrintF( buffer, sizeof( buffer ), "The current constrained status should be different with the previous one: %d -> %d\n", context->isConstrainedPrev, context->isConstrainedNow );
                            errorDescription = buffer );
            require_action( context->isExpensiveNow == false, test_failed,
                            SNPrintF( buffer, sizeof( buffer ), "The interface %s should be inexpensive when testing \"constrained\"\n", context->ifName );
                            errorDescription = buffer );
            require_action( context->expectedOperation == context->operation, test_failed, errorDescription = "Operation is not expected");

            context->counter++;
            if (context->counter == TEST_REPETITION)
            {
                // test changing expensive and constrained flags at the same time
                context->state = TEST_EXPENSIVE_CONSTRAINED_PREPARE;

                // reset interface
                err = systemf( NULL, "ifconfig %s -expensive && ifconfig %s -constrained", context->ifName, context->ifName );
                require_noerr_action( err, test_failed, errorDescription = "systemf failed" );

                context->isExpensiveNow = false;
                context->isConstrainedNow = false;
                gettimeofday( &context->updateTime, NULL );
            }
            else
            {
                context->subtestProgress_startTime  = NanoTimeGetCurrent();
                context->isConstrainedPrev          = context->isConstrainedNow;
                context->isConstrainedNow           = !context->isConstrainedNow; // flip constrained flag
                if ( gExpensiveConstrainedTest_DenyConstrained )
                    context->expectedOperation      = context->isConstrainedNow ? RESULT_RMV : RESULT_ADD;
                else
                    context->expectedOperation      = NO_UPDATE;
                context->operation                  = NO_UPDATE;
                context->subtestProgress_callBack   = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
                require_action( context->subtestProgress_callBack != NULL, test_failed, errorDescription = "CFArrayCreateMutable failed" );

                err = systemf( NULL, "ifconfig %s %sconstrained", context->ifName, context->isConstrainedNow ? "" : "-" );
                require_noerr_action( err, test_failed, errorDescription = "systemf failed" );

                gettimeofday(&context->updateTime, NULL);
            }
            break;
        case TEST_EXPENSIVE_CONSTRAINED_PREPARE:
            // The interface should be inexpensive and unconstrained when the constrained test starts
            require_action( context->isExpensiveNow == false,   test_failed,
                            SNPrintF( buffer, sizeof( buffer ), "Interface %s should be inexpensive.\n", context->ifName );
                            errorDescription = buffer );
            require_action( context->isConstrainedNow == false, test_failed,
                            SNPrintF(buffer, sizeof( buffer ), "Interface %s should be unconstrained.\n", context->ifName );
                            errorDescription = buffer );

            // now flip expensive and constrained at the same time
            context->subtestProgress_startTime  = NanoTimeGetCurrent();
            context->state                      = TEST_EXPENSIVE_CONSTRAINED;
            context->counter                    = 0;
            context->isExpensivePrev            = false;
            context->isExpensiveNow             = true;
            context->isConstrainedPrev          = false;
            context->isConstrainedNow           = true;
            if (gExpensiveConstrainedTest_DenyConstrained || gExpensiveConstrainedTest_DenyExpensive)
                context->expectedOperation      = RESULT_RMV;
            else
                context->expectedOperation      = NO_UPDATE;
            context->operation                  = NO_UPDATE;
            context->subtestProgress_callBack   = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
            require_action( context->subtestProgress_callBack != NULL, test_failed, errorDescription = "CFArrayCreateMutable failed" );

            err = systemf(NULL, "ifconfig %s expensive && ifconfig %s constrained", context->ifName, context->ifName );
            require_noerr_action( err, test_failed, errorDescription = "systemf failed" );

            gettimeofday( &context->updateTime, NULL );
            break;
        case TEST_EXPENSIVE_CONSTRAINED:
            // expensive and constrained flag should always be changed
            require_action( ( context->isExpensivePrev ^ context->isExpensiveNow ) && ( context->isConstrainedPrev ^ context->isConstrainedNow ), test_failed,
                            SNPrintF( buffer, sizeof( buffer ), "Both expensive and constrained status need to be changed" );
                            errorDescription = buffer );
            require_action( context->isExpensiveNow == context->isConstrainedNow, test_failed, errorDescription = "context->isExpensiveNow != context->isConstrainedNow" );
            require_action( context->expectedOperation == context->operation, test_failed, errorDescription = "Operation is not expected" );

            context->counter++;
            if ( context->counter == TEST_REPETITION )
            {
                context->state = TEST_SUCCEEDED;
            }
            else
            {
                context->subtestProgress_startTime  = NanoTimeGetCurrent();
                context->isExpensivePrev            = context->isExpensiveNow;
                context->isExpensiveNow             = !context->isExpensiveNow;
                context->isConstrainedPrev          = context->isConstrainedNow;
                context->isConstrainedNow           = !context->isConstrainedNow;
                if (gExpensiveConstrainedTest_DenyConstrained || gExpensiveConstrainedTest_DenyExpensive)
                    context->expectedOperation      = context->isExpensiveNow ? RESULT_RMV : RESULT_ADD;
                else
                    context->expectedOperation      = NO_UPDATE;
                context->operation                  = NO_UPDATE;
                context->subtestProgress_callBack   = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
                require_action( context->subtestProgress_callBack != NULL, test_failed, errorDescription = "CFArrayCreateMutable failed" );

                err = systemf( NULL, "ifconfig %s %sexpensive && ifconfig %s %sconstrained", context->ifName, context->isExpensiveNow ? "" : "-", context->ifName, context->isConstrainedNow ? "" : "-" );
                require_noerr_action( err, test_failed, errorDescription = "systemf failed" );

                gettimeofday( &context->updateTime, NULL );
            }
            break;
        case TEST_FAILED:
        test_failed:
            ExpensiveConstrainedSubtestReport( context, errorDescription );
            ExpensiveConstrainedStopAndCleanTheTest( context );
            if ( context->numOfRetries > 0 )
            {
                context->state = TEST_BEGIN;
                context->numOfRetries--;
                break;
            }
            ExpensiveConstrainedSubtestParams[context->subtestIndex++].test_passed = 0;
            if (context->subtestIndex == (int) countof( ExpensiveConstrainedSubtestParams ))
            {
                ExpensiveConstrainedFinalResultReport( context, false );
                exit( 2 );
            }
            if (context->timer == NULL)
            {
                // If timer is NULL, it means that we encounter error before we set up the test handler, which is unrecoverable.
                ExpensiveConstrainedFinalResultReport( context, false );
                exit( 1 );
            }
            context->state = TEST_BEGIN;
            break;
        case TEST_SUCCEEDED:
            ExpensiveConstrainedSubtestReport( context, NULL );
            ExpensiveConstrainedStopAndCleanTheTest( context );
            ExpensiveConstrainedSubtestParams[context->subtestIndex++].test_passed = 1;
            if (context->subtestIndex == (int) countof( ExpensiveConstrainedSubtestParams ))
            {
                // all the subtests have been run
                Boolean hasFailed = false;
                for ( int i = 0; i < (int) countof( ExpensiveConstrainedSubtestParams ) && !hasFailed; i++ )
                    hasFailed = ( ExpensiveConstrainedSubtestParams[i].test_passed != 1 );

                ExpensiveConstrainedFinalResultReport( context, !hasFailed );
                exit( hasFailed ? 2 : 0 );
            }
            context->state = TEST_BEGIN;
            break;
        default:
            FPrintF( stdout, "unknown error\n" );
            exit( 1 );
    }
}

//===========================================================================================================================
//    ExpensiveConstrainedCallback
//===========================================================================================================================

static void DNSSD_API
    ExpensiveConstrainedCallback(
        __unused DNSServiceRef  inSDRef,
        DNSServiceFlags         inFlags,
        uint32_t                inInterfaceIndex,
        DNSServiceErrorType     inError,
        const char *            inHostname,
        const struct sockaddr * inSockAddr,
        __unused uint32_t       inTTL,
        void *                  inContext )
{
    ExpensiveConstrainedContext * const   context = (ExpensiveConstrainedContext *)inContext;
    OSStatus                                    err;
    const char *                                addrStr;
    char                                        addrStrBuf[ kSockAddrStringMaxSize ];
    char                                        inFlagsDescription[ 128 ];
    NanoTime64                                  now;
    char                                        nowTimestamp[ 32 ];

    switch ( inError ) {
        case kDNSServiceErr_NoError:
        case kDNSServiceErr_NoSuchRecord:
            break;

        case kDNSServiceErr_Timeout:
            Exit( kExitReason_Timeout );

        default:
            err = inError;
            goto exit;
    }

    if( ( inSockAddr->sa_family != AF_INET ) && ( inSockAddr->sa_family != AF_INET6 ) )
    {
        dlogassert( "Unexpected address family: %d", inSockAddr->sa_family );
        err = kTypeErr;
        goto exit;
    }

    if( !inError )
    {
        err = SockAddrToString( inSockAddr, kSockAddrStringFlagsNone, addrStrBuf );
        require_noerr( err, exit );
        addrStr = addrStrBuf;
    }
    else
    {
        addrStr = ( inSockAddr->sa_family == AF_INET ) ? kNoSuchRecordAStr : kNoSuchRecordAAAAStr;
    }

    now = NanoTimeGetCurrent();
    _NanoTime64ToTimestamp( now, nowTimestamp, sizeof( nowTimestamp ) );
    SNPrintF( inFlagsDescription, sizeof( inFlagsDescription ), "%{du:cbflags}", inFlags );
    err = CFPropertyListAppendFormatted( kCFAllocatorDefault,  context->subtestProgress_callBack,
        "{"
            "%kO=%s"
            "%kO=%s"
            "%kO=%s"
            "%kO=%lli"
            "%kO=%s"
        "}",
        EXPENSIVE_CONSTRAINED_SUBTEST_ACTUAL_RESULT_KEY_TIMESTAMP,  nowTimestamp,
        EXPENSIVE_CONSTRAINED_SUBTEST_ACTUAL_RESULT_KEY_NAME,       inHostname,
        EXPENSIVE_CONSTRAINED_SUBTEST_ACTUAL_RESULT_KEY_FLAGS,      inFlagsDescription,
        EXPENSIVE_CONSTRAINED_SUBTEST_ACTUAL_RESULT_KEY_INTERFACE,  (int64_t) inInterfaceIndex,
        EXPENSIVE_CONSTRAINED_SUBTEST_ACTUAL_RESULT_KEY_ADDRESS,    addrStr
    );
    require_noerr_quiet( err, exit );

    if ( inFlags & kDNSServiceFlagsMoreComing )
        return;

    if ( inFlags & kDNSServiceFlagsAdd )
        context->operation = RESULT_ADD;
    else
        context->operation = RESULT_RMV;

    gettimeofday(&context->notificationTime, NULL);
exit:
    if( err ) exit( 1 );
}

//===========================================================================================================================
//    ExpensiveConstrainedInitializeContext
//===========================================================================================================================

static void ExpensiveConstrainedInitializeContext( ExpensiveConstrainedContext *context )
{
    // clear the flags of the previous subtest
    context->flags      = 0;
    context->protocols  = 0;

    // get the parameter for the current subtest
    const ExpensiveConstrainedTestParams *param     = &ExpensiveConstrainedSubtestParams[context->subtestIndex];
    gExpensiveConstrainedTest_Name                  = param->qname;
    gExpensiveConstrainedTest_DenyExpensive         = param->deny_expensive;
    gExpensiveConstrainedTest_DenyConstrained       = param->deny_constrained;
    gExpensiveConstrainedTest_StartFromExpensive    = param->start_from_expensive;
    gExpensiveConstrainedTest_ProtocolIPv4          = param->ipv4_query;
    gExpensiveConstrainedTest_ProtocolIPv6          = param->ipv6_query;
}

//===========================================================================================================================
//    ExpensiveConstrainedStopAndCleanTheTest
//===========================================================================================================================

static void ExpensiveConstrainedStopAndCleanTheTest( ExpensiveConstrainedContext *context )
{
    // Stop the ongoing query
    if ( context->opRef != NULL )
        DNSServiceRefDeallocate( context->opRef );

    context->opRef      = NULL;
    context->flags      = 0;
    context->protocols  = 0;
}

//===========================================================================================================================
//    ExpensiveConstrainedSubtestProgressReport
//===========================================================================================================================

static void ExpensiveConstrainedSubtestProgressReport( ExpensiveConstrainedContext *context )
{
    OSStatus            err;
    NanoTime64          now;
    char                startTime[ 32 ];
    char                endTime[ 32 ];
    char                expensive[ 32 ];
    char                constrained[ 32 ];

    now = NanoTimeGetCurrent();
    _NanoTime64ToTimestamp( context->subtestProgress_startTime, startTime, sizeof( startTime ) );
    _NanoTime64ToTimestamp( now, endTime, sizeof( endTime ) );

    snprintf( expensive, sizeof( expensive ), "%s -> %s", context->isExpensivePrev ? "True" : "False", context->isExpensiveNow ? "True" : "False" );
    snprintf( constrained, sizeof( constrained ), "%s -> %s", context->isConstrainedPrev ? "True" : "False", context->isConstrainedNow ? "True" : "False" );

    err = CFPropertyListAppendFormatted( kCFAllocatorDefault, context->subtestProgress,
        "{"
            "%kO=%s"
            "%kO=%s"
            "%kO=%s"
            "%kO=%s"
            "%kO=%s"
            "%kO=%s"
            "%kO=%s"
            "%kO=%O"
        "}",
        EXPENSIVE_CONSTRAINED_SUBTEST_PROGRESS_KEY_START_TIME,              startTime,
        EXPENSIVE_CONSTRAINED_SUBTEST_PROGRESS_KEY_END_TIME,                endTime,
        EXPENSIVE_CONSTRAINED_SUBTEST_PROGRESS_KEY_STATE,                   ExpensiveConstrainedStateString(context->state),
        EXPENSIVE_CONSTRAINED_SUBTEST_PROGRESS_KEY_EXPECT_RESULT,           ExpensiveConstrainedOperationString(context->expectedOperation),
        EXPENSIVE_CONSTRAINED_SUBTEST_PROGRESS_KEY_ACTUAL_RESULT,           ExpensiveConstrainedOperationString(context->operation),
        EXPENSIVE_CONSTRAINED_SUBTEST_PROGRESS_KEY_EXPENSIVE_PREV_NOW,      expensive,
        EXPENSIVE_CONSTRAINED_SUBTEST_PROGRESS_KEY_CONSTRAINED_PREV_NOW,    constrained,
        EXPENSIVE_CONSTRAINED_SUBTEST_PROGRESS_KEY_CALL_BACK,               context->subtestProgress_callBack
    );
    require_noerr( err, exit );
    ForgetCF( &context->subtestProgress_callBack );
    return;

exit:
    ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//    ExpensiveConstrainedFinalSubtestReport
//===========================================================================================================================

static void ExpensiveConstrainedSubtestReport( ExpensiveConstrainedContext *context, const char *error_description )
{
    OSStatus            err;
    NanoTime64          now;
    char                startTime[ 32 ];
    char                endTime[ 32 ];
    char                flagDescription[ 1024 ];

    now = NanoTimeGetCurrent();
    _NanoTime64ToTimestamp( context->subtestReport_startTime, startTime, sizeof( startTime ) );
    _NanoTime64ToTimestamp( now, endTime, sizeof( endTime ) );
    SNPrintF( flagDescription, sizeof( flagDescription ), "%#{flags}", context->flags, kDNSServiceFlagsDescriptors );

    if (error_description != NULL)
    {
        err = CFPropertyListAppendFormatted( kCFAllocatorDefault, context->subtestReport,
            "{"
                "%kO=%s"
                "%kO=%s"
                "%kO=%s"
                "%kO=%s"
                "%kO=%s"
                "%kO=%lli"
                "%kO=%s"
                "%kO=%O"
                "%kO=%s"
                "%kO=%O"
            "}",
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_START_TIME,        startTime,
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_END_TIME,          endTime,
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_QNAME,             context->name,
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_FLAGS,             flagDescription,
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_PROTOCOLS,         ExpensiveConstrainedProtocolString( context->protocols ),
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_INTERFACE_INDEX,   (int64_t) context->ifIndex,
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_INTERFACE_NAME,    context->ifName,
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_RESULT,            CFSTR( "Fail" ),
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_ERROR,             error_description,
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_TEST_PROGRESS,     context->subtestProgress
        );
    }
    else
    {
        err = CFPropertyListAppendFormatted( kCFAllocatorDefault, context->subtestReport,
            "{"
                "%kO=%s"
                "%kO=%s"
                "%kO=%s"
                "%kO=%s"
                "%kO=%s"
                "%kO=%lli"
                "%kO=%s"
                "%kO=%O"
                "%kO=%O"
            "}",
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_START_TIME,        startTime,
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_END_TIME,          endTime,
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_QNAME,             context->name,
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_FLAGS,             flagDescription,
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_PROTOCOLS,         ExpensiveConstrainedProtocolString( context->protocols ),
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_INTERFACE_INDEX,   (int64_t) context->ifIndex,
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_INTERFACE_NAME,    context->ifName,
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_RESULT,            CFSTR( "Pass" ),
            EXPENSIVE_CONSTRAINED_SUBTEST_REPORT_KEY_TEST_PROGRESS,     context->subtestProgress
        );
    }

    require_noerr( err, exit );
    ForgetCF( &context->subtestProgress );
    return;
exit:
    ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//    ExpensiveConstrainedFinalResultReport
//===========================================================================================================================

static void ExpensiveConstrainedFinalResultReport( ExpensiveConstrainedContext *context, Boolean allPassed )
{
    OSStatus            err;
    CFPropertyListRef   plist;
    NanoTime64          now;
    char                startTime[ 32 ];
    char                endTime[ 32 ];

    now = NanoTimeGetCurrent();
    _NanoTime64ToTimestamp( context->testReport_startTime, startTime, sizeof( startTime ) );
    _NanoTime64ToTimestamp( now, endTime, sizeof( endTime ) );

    err = CFPropertyListCreateFormatted( kCFAllocatorDefault, &plist,
        "{"
            "%kO=%s"
            "%kO=%s"
            "%kO=%b"
            "%kO=%O"
        "}",
        EXPENSIVE_CONSTRAINED_TEST_REPORT_KEY_START_TIME,       startTime,
        EXPENSIVE_CONSTRAINED_TEST_REPORT_KEY_END_TIME,         endTime,
        EXPENSIVE_CONSTRAINED_TEST_REPORT_KEY_ALL_PASSED,       allPassed,
        EXPENSIVE_CONSTRAINED_TEST_REPORT_KEY_SUBTEST_RESULT,   context->subtestReport
    );
    require_noerr( err, exit );
    ForgetCF( &context->subtestReport );

    err = OutputPropertyList( plist, context->outputFormat, context->outputFilePath );
    CFRelease( plist );
    require_noerr( err, exit );

    return;
exit:
    ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//    ExpensiveConstrainedProtocolString
//===========================================================================================================================

static const char *ExpensiveConstrainedProtocolString( DNSServiceProtocol protocol )
{
    const char *str = NULL;
    switch ( protocol ) {
        case kDNSServiceProtocol_IPv4:
            str = "IPv4";
            break;
        case kDNSServiceProtocol_IPv6:
            str = "IPv6";
            break;
        case kDNSServiceProtocol_IPv4 | kDNSServiceProtocol_IPv6:
            str = "IPv4 & IPv6";
            break;
        default:
            break;
    }
    return str;
}

//===========================================================================================================================
//    ExpensiveConstrainedStateString
//===========================================================================================================================

static const char *ExpensiveConstrainedStateString( enum ExpensiveConstrainedTestState state )
{
    const char *str = NULL;
    switch ( state ) {
        case TEST_BEGIN:
            str = "TEST_BEGIN";
            break;
        case TEST_EXPENSIVE_PREPARE:
            str = "TEST_EXPENSIVE_PREPARE";
            break;
        case TEST_EXPENSIVE:
            str = "TEST_EXPENSIVE";
            break;
        case TEST_CONSTRAINED_PREPARE:
            str = "TEST_CONSTRAINED_PREPARE";
            break;
        case TEST_CONSTRAINED:
            str = "TEST_CONSTRAINED";
            break;
        case TEST_EXPENSIVE_CONSTRAINED_PREPARE:
            str = "TEST_EXPENSIVE_CONSTRAINED_PREPARE";
            break;
        case TEST_EXPENSIVE_CONSTRAINED:
            str = "TEST_EXPENSIVE_CONSTRAINED";
            break;
        case TEST_FAILED:
            str = "TEST_FAILED";
            break;
        case TEST_SUCCEEDED:
            str = "TEST_SUCCEEDED";
            break;
        default:
            str = "UNKNOWN";
            break;
    }

    return str;
}

//===========================================================================================================================
//    ExpensiveConstrainedOperationString
//===========================================================================================================================

static const char *ExpensiveConstrainedOperationString( enum ExpensiveConstrainedTestOperation operation )
{
    const char *str = NULL;
    switch ( operation ) {
        case RESULT_ADD:
            str = "RESULT_ADD";
            break;
        case RESULT_RMV:
            str = "RESULT_RMV";
            break;
        case NO_UPDATE:
            str = "NO_UPDATE";
            break;
        default:
            str = "UNKNOWN";
            break;
    }
    return str;
}

//===========================================================================================================================
//    expensiveConstrainedEndsWith
//===========================================================================================================================
static Boolean expensiveConstrainedEndsWith( const char *str, const char *suffix )
{
    if ( !str || !suffix )
        return false;
    size_t lenstr = strlen( str );
    size_t lensuffix = strlen( suffix );
    if ( lensuffix > lenstr )
        return false;
    return strncmp( str + lenstr - lensuffix, suffix, lensuffix ) == 0;
}

//===========================================================================================================================
//	RegistrationTestCmd
//===========================================================================================================================

typedef struct RegistrationSubtest		RegistrationSubtest;

typedef struct
{
	CFMutableArrayRef			subtestReports;				// Array of subtest reports.
	dispatch_source_t			timer;						// Timer to enforce subtest durations.
	dispatch_source_t			sigSourceINT;				// SIGINT signal handler for a clean test exit.
	dispatch_source_t			sigSourceTERM;				// SIGTERM signal handler for a clean test exit.
	RegistrationSubtest *		subtest;					// Current subtest.
	char *						outputFilePath;				// Path of test result output file. If NULL, stdout will be used.
	OutputFormatType			outputFormat;				// Format of test results output.
	CFStringRef					computerNamePrev;			// Previous ComputerName.
	CFStringRef					localHostNamePrev;			// Previous LocalHostName.
	NanoTime64					startTime;					// Test's start time.
	char *						computerName;				// Temporary ComputerName to set during testing. (malloc'd)
	char *						localHostName;				// Temporary LocalHostName to set during testing. (malloc'd)
	CFStringEncoding			computerNamePrevEncoding;	// Previous ComputerName's encoding.
	int							subtestIndex;				// Index of current subtest.
	Boolean						computerNameSet;			// True if a temporary ComputerName was set.
	Boolean						localHostNameSet;			// True if a temporary LocalHostName was set.
	Boolean						failed;						// True if at least one non-skipped subtest failed.
	Boolean						forBATS;					// True if the test is running in a BATS environment.
	
}	RegistrationTest;

typedef enum
{
	kRegistrationInterfaceSet_Null			= 0,
	kRegistrationInterfaceSet_All			= 1,
	kRegistrationInterfaceSet_AllPlusAWDL	= 2,
	kRegistrationInterfaceSet_LoopbackOnly	= 3,
	kRegistrationInterfaceSet_AWDLOnly		= 4
	
}	RegistrationInterfaceSet;

typedef struct
{
	RegistrationInterfaceSet		interfaceSet;	// Interfaces to register the service over.
	Boolean							useDefaultName;	// True if registration is to use the default service name.
	Boolean							useLODiscovery;	// True if discovery is to use kDNSServiceInterfaceIndexLocalOnly.
	
}	RegistrationSubtestParams;

static const RegistrationSubtestParams		kRegistrationSubtestParams[] =
{
	{ kRegistrationInterfaceSet_All,			true,	false },
	{ kRegistrationInterfaceSet_All,			false,	false },
	{ kRegistrationInterfaceSet_AllPlusAWDL,	true,	false },
	{ kRegistrationInterfaceSet_AllPlusAWDL,	false,	false },
	{ kRegistrationInterfaceSet_LoopbackOnly,	true,	false },
	{ kRegistrationInterfaceSet_LoopbackOnly,	false,	false },
	{ kRegistrationInterfaceSet_AWDLOnly,		true,	false },
	{ kRegistrationInterfaceSet_AWDLOnly,		false,	false },
	{ kRegistrationInterfaceSet_All,			true,	true  },
	{ kRegistrationInterfaceSet_All,			false,	true  },
	{ kRegistrationInterfaceSet_AllPlusAWDL,	true,	true  },
	{ kRegistrationInterfaceSet_AllPlusAWDL,	false,	true  },
	{ kRegistrationInterfaceSet_LoopbackOnly,	true,	true  },
	{ kRegistrationInterfaceSet_LoopbackOnly,	false,	true  },
	{ kRegistrationInterfaceSet_AWDLOnly,		true,	true  },
	{ kRegistrationInterfaceSet_AWDLOnly,		false,	true  }
};

typedef struct
{
	NanoTime64		browseResultTime;	// Per-interface browse result time.
	NanoTime64		querySRVResultTime;	// Per-interface SRV record query result time.
	NanoTime64		queryTXTResultTime;	// Per-interface TXT record query result time.
	
}	RegistrationResultTimes;

typedef struct
{
	MDNSInterfaceItem			base;	// Underlying MDNSInterface linked-list item.
	RegistrationResultTimes		times;	// Per-interface result times.
	
}	RegistrationInterfaceItem;

struct RegistrationSubtest
{
	DNSServiceRef					registration;		// DNS-SD service registration.
	DNSServiceRef					connection;			// Shared DNS-SD connection.
	DNSServiceRef					browse;				// DNS-SD browse for service's type.
	DNSServiceRef					querySRV;			// DNS-SD query request for service's SRV record.
	DNSServiceRef					queryTXT;			// DNS-SD query request for service's TXT record.
	CFMutableArrayRef				unexpected;			// Array of unexpected registration, browse, and query results.
#if( TARGET_OS_WATCH )
	CFMutableArrayRef				ignored;			// Array of unexpected, but ignored, browse and query results.
#endif
	const char *					serviceName;		// Service's name.
	char *							serviceNameCustom;	// Service's name if using a custom name. (malloc'd)
	char *							serviceType;		// Service's service type. (malloc'd)
	size_t							serviceTypeLen;		// C string length of service's service type.
	char *							serviceFQDN;		// Service's FQDN, i.e., name of its SRV and TXT records.
	uint8_t *						txtPtr;				// Pointer to service's TXT record data. (malloc'd)
	size_t							txtLen;				// Length of service's TXT record data.
	RegistrationInterfaceItem *		ifList;				// If ifIndex == 0, interfaces that service should register over.
	RegistrationResultTimes			ifTimes;			// If ifIndex != 0, result times for interface with that index.
	RegistrationTest *				test;				// Pointer to parent test.
	NanoTime64						startTime;			// Subtest's start time.
	char *							description;		// Subtest's description. (malloc'd)
	uint32_t						ifIndex;			// Interface index used for service registration.
	uint16_t						port;				// Service's port number.
	Boolean							useLODiscovery;		// True if discovery is to use kDNSServiceInterfaceIndexLocalOnly.
	Boolean							includeAWDL;		// True if the IncludeAWDL flag was used during registration.
	Boolean							ifIsAWDL;			// True if ifIndex is the index of an AWDL interface.
	Boolean							skipped;			// True if this subtest is to be skipped.
	Boolean							registered;			// True if the test service was successfully registered.
	Boolean							useDefaultName;		// True if the service is to use the default service name.
};

static OSStatus	_RegistrationTestCreate( RegistrationTest **outTest );
static void		_RegistrationTestFree( RegistrationTest *inTest );
static void		_RegistrationTestBegin( void *inContext );
static void		_RegistrationTestProceed( RegistrationTest *inTest );
static OSStatus	_RegistrationTestStart( RegistrationTest *inTest );
static void		_RegistrationTestStop( RegistrationTest *inTest );
#define _RegistrationTestForget( X )		ForgetCustomEx( X, _RegistrationTestStop, _RegistrationTestFree )
static OSStatus
	_RegistrationTestStartSubtest(
		RegistrationTest *					inTest,
		const RegistrationSubtestParams *	inParams,
		Boolean *							outSkipped );
static OSStatus	_RegistrationTestEndSubtest( RegistrationTest *inTest );
static void		_RegistrationTestEnd( RegistrationTest *inTest ) ATTRIBUTE_NORETURN;
static void		_RegistrationTestExit( RegistrationTest *inTest, OSStatus inError ) ATTRIBUTE_NORETURN;
static OSStatus	_RegistrationSubtestCreate( RegistrationSubtest **outSubtest );
static void		_RegistrationSubtestStop( RegistrationSubtest *inSubtest );
static void		_RegistrationSubtestFree( RegistrationSubtest *inSubtest );
#define _RegistrationSubtestForget( X )		ForgetCustomEx( X, _RegistrationSubtestStop, _RegistrationSubtestFree )
static OSStatus	_RegistrationTestInterfaceListCreate( Boolean inIncludeAWDL, RegistrationInterfaceItem **outList );
static OSStatus
	_RegistrationTestCreateRandomTXTRecord(
		size_t		inMinLen,
		size_t		inMaxLen,
		uint8_t **	outTXTPtr,
		size_t *	outTXTLen );
static void DNSSD_API
	_RegistrationSubtestRegisterCallback(
		DNSServiceRef		inSDRef,
		DNSServiceFlags		inFlags,
		DNSServiceErrorType	inError,
		const char *		inName,
		const char *		inType,
		const char *		inDomain,
		void *				inContext );
static void DNSSD_API
	_RegistrationSubtestBrowseCallback(
		DNSServiceRef		inSDRef,
		DNSServiceFlags		inFlags,
		uint32_t			inIfIndex,
		DNSServiceErrorType	inError,
		const char *		inServiceName,
		const char *		inServiceType,
		const char *		inDomain,
		void *				inContext );
static void DNSSD_API
	_RegistrationSubtestQueryCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inIfIndex,
		DNSServiceErrorType		inError,
		const char *			inName,
		uint16_t				inType,
		uint16_t				inClass,
		uint16_t				inRDataLen,
		const void *			inRDataPtr,
		uint32_t				inTTL,
		void *					inContext );
static Boolean	_RegistrationSubtestValidServiceType( const RegistrationSubtest *inSubtest, const char *inServiceType );
static RegistrationResultTimes *
	_RegistrationSubtestGetInterfaceResultTimes(
		RegistrationSubtest *	inSubtest,
		uint32_t				inIfIndex,
		Boolean *				outIsAWDL );
static void		_RegistrationTestTimerHandler( void *inContext );
#if( TARGET_OS_WATCH )
static Boolean	_RegistrationTestInterfaceIsWiFi( const char *inIfName );
#endif

static void	RegistrationTestCmd( void )
{
	OSStatus				err;
	RegistrationTest *		test;
	
	err = _RegistrationTestCreate( &test );
	require_noerr( err, exit );
	
	if( gRegistrationTest_BATSEnvironment ) test->forBATS = true;
	if( gRegistrationTest_OutputFilePath )
	{
		test->outputFilePath = strdup( gRegistrationTest_OutputFilePath );
		require_action( test->outputFilePath, exit, err = kNoMemoryErr );
	}
	
	err = OutputFormatFromArgString( gRegistrationTest_OutputFormat, &test->outputFormat );
	require_noerr_quiet( err, exit );
	
	dispatch_async_f( dispatch_get_main_queue(), test, _RegistrationTestBegin );
	dispatch_main();
	
exit:
	if( test ) _RegistrationTestFree( test );
	ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================

static OSStatus	_RegistrationTestCreate( RegistrationTest **outTest )
{
	OSStatus				err;
	RegistrationTest *		obj;
	
	obj = (RegistrationTest *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	obj->subtestReports = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	require_action( obj->subtestReports, exit, err = kNoMemoryErr );
	
	*outTest = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	if( obj ) _RegistrationTestFree( obj );
	return( err );
}

//===========================================================================================================================

static void	_RegistrationTestFree( RegistrationTest *inTest )
{
	check( !inTest->timer );
	check( !inTest->sigSourceINT );
	check( !inTest->sigSourceTERM );
	check( !inTest->computerNameSet );
	check( !inTest->localHostNameSet );
	check( !inTest->subtest );
	ForgetCF( &inTest->subtestReports );
	ForgetMem( &inTest->outputFilePath );
	ForgetCF( &inTest->computerNamePrev );
	ForgetCF( &inTest->localHostNamePrev );
	ForgetMem( &inTest->computerName );
	ForgetMem( &inTest->localHostName );
}

//===========================================================================================================================

static void	_RegistrationTestBegin( void *inContext )
{
	_RegistrationTestProceed( (RegistrationTest *) inContext );
}

//===========================================================================================================================

static void	_RegistrationTestProceed( RegistrationTest *inTest )
{
	OSStatus		err;
	Boolean			skippedSubtest;
	
	do
	{
		int		subtestIndex;
		
		if( !inTest->startTime )
		{
			err = _RegistrationTestStart( inTest );
			require_noerr_quiet( err, exit );
			
			inTest->startTime = NanoTimeGetCurrent();
		}
		else
		{
			err = _RegistrationTestEndSubtest( inTest );
			require_noerr( err, exit );
			
			++inTest->subtestIndex;
		}
		
		subtestIndex = inTest->subtestIndex;
		if( subtestIndex < (int) countof( kRegistrationSubtestParams ) )
		{
			err = _RegistrationTestStartSubtest( inTest, &kRegistrationSubtestParams[ subtestIndex ], &skippedSubtest );
			require_noerr_quiet( err, exit );
		}
		else
		{
			_RegistrationTestEnd( inTest );
		}
		
	}	while( skippedSubtest );
	
exit:
	if( err ) _RegistrationTestExit( inTest, err );
}

//===========================================================================================================================

static void	_RegistrationTestSignalHandler( void *inContext );

static OSStatus	_RegistrationTestStart( RegistrationTest *inTest )
{
	OSStatus		err;
	char			tag[ 6 + 1 ];
	
	// Save original ComputerName and LocalHostName.
	
	check( !inTest->computerNamePrev );
	inTest->computerNamePrev = SCDynamicStoreCopyComputerName( NULL, &inTest->computerNamePrevEncoding );
	err = map_scerror( inTest->computerNamePrev );
	require_noerr( err, exit );
	
	check( !inTest->localHostNamePrev );
	inTest->localHostNamePrev = SCDynamicStoreCopyLocalHostName( NULL );
	err = map_scerror( inTest->localHostNamePrev );
	require_noerr( err, exit );
	
	// Generate a unique test ComputerName.
	
	check( !inTest->computerName ); 
	ASPrintF( &inTest->computerName, "dnssdutil-regtest-computer-name-%s",
		_RandomStringExact( kLowerAlphaNumericCharSet, kLowerAlphaNumericCharSetSize, sizeof( tag ) - 1, tag ) );
	require_action( inTest->computerName, exit, err = kNoMemoryErr );
	
	// Generate a unique test LocalHostName.
	
	check( !inTest->localHostName ); 
	ASPrintF( &inTest->localHostName, "dnssdutil-regtest-local-hostname-%s",
		_RandomStringExact( kLowerAlphaNumericCharSet, kLowerAlphaNumericCharSetSize, sizeof( tag ) - 1, tag ) );
	require_action( inTest->localHostName, exit, err = kNoMemoryErr );
	
	// Set up SIGINT signal handler.
	
	signal( SIGINT, SIG_IGN );
	check( !inTest->sigSourceINT ); 
	err = DispatchSignalSourceCreate( SIGINT, dispatch_get_main_queue(), _RegistrationTestSignalHandler, inTest,
		&inTest->sigSourceINT );
	require_noerr( err, exit );
	dispatch_resume( inTest->sigSourceINT );
	
	// Set up SIGTERM signal handler.
	
	signal( SIGTERM, SIG_IGN );
	check( !inTest->sigSourceTERM ); 
	err = DispatchSignalSourceCreate( SIGTERM, dispatch_get_main_queue(), _RegistrationTestSignalHandler, inTest,
		&inTest->sigSourceTERM );
	require_noerr( err, exit );
	dispatch_resume( inTest->sigSourceTERM );
	
	// Set test ComputerName.
	
	check( !inTest->computerNameSet );
	err = _SetComputerNameWithUTF8CString( inTest->computerName );
	require_noerr( err, exit );
	inTest->computerNameSet = true;
	
	// Set test LocalHostName.
	
	check( !inTest->localHostNameSet );
	err = _SetLocalHostNameWithUTF8CString( inTest->localHostName );
	require_noerr( err, exit );
	inTest->localHostNameSet = true;
	
exit:
	if( err ) _RegistrationTestStop( inTest );
	return( err );
}

static void	_RegistrationTestSignalHandler( void *inContext )
{
	RegistrationTest * const		test = (RegistrationTest *) inContext;
	
	FPrintF( stderr, "Registration test got a SIGINT or SIGTERM signal, exiting..." );
	
	_RegistrationTestExit( test, kCanceledErr );
}

//===========================================================================================================================

static void	_RegistrationTestStop( RegistrationTest *inTest )
{
	OSStatus		err;
	
	dispatch_source_forget( &inTest->timer );
	dispatch_source_forget( &inTest->sigSourceINT );
	dispatch_source_forget( &inTest->sigSourceTERM );
	_RegistrationSubtestForget( &inTest->subtest );
	if( inTest->computerNameSet )
	{
		err = _SetComputerName( inTest->computerNamePrev, inTest->computerNamePrevEncoding );
		check_noerr( err );
		if( !err ) inTest->computerNameSet = false;
	}
	if( inTest->localHostNameSet )
	{
		err = _SetLocalHostName( inTest->localHostNamePrev );
		check_noerr( err );
		if( !err ) inTest->localHostNameSet = false;
	}
}

//===========================================================================================================================

#define kRegistrationTestSubtestDurationSecs		5

static OSStatus
	_RegistrationTestStartSubtest(
		RegistrationTest *					inTest,
		const RegistrationSubtestParams *	inParams,
		Boolean *							outSkipped )
{
	OSStatus					err;
	RegistrationSubtest *		subtest;
	const char *				interfaceDesc;
	DNSServiceFlags				flags;
	char						tag[ 6 + 1 ];
	
	subtest	= NULL;
	err = _RegistrationSubtestCreate( &subtest );
	require_noerr( err, exit );
	
	subtest->test			= inTest;
	subtest->useDefaultName	= inParams->useDefaultName;
	subtest->useLODiscovery	= inParams->useLODiscovery;
	
	// Determine registration interfaces.
	
	switch( inParams->interfaceSet )
	{
		case kRegistrationInterfaceSet_All:
			subtest->ifIndex = kDNSServiceInterfaceIndexAny;
			
			if( !subtest->useLODiscovery )
			{
				err = _RegistrationTestInterfaceListCreate( false, &subtest->ifList );
				require_noerr( err, exit );
			}
			interfaceDesc = "all interfaces (excluding AWDL)";
			break;
		
		case kRegistrationInterfaceSet_AllPlusAWDL:
			subtest->ifIndex		= kDNSServiceInterfaceIndexAny;
			subtest->includeAWDL	= true;
			
			if( !subtest->useLODiscovery )
			{
				err = _RegistrationTestInterfaceListCreate( true, &subtest->ifList );
				require_noerr( err, exit );
			}
			interfaceDesc = "all interfaces (including AWDL)";
			break;
		
		case kRegistrationInterfaceSet_LoopbackOnly:
			subtest->ifIndex = if_nametoindex( "lo0" );
			if( subtest->ifIndex == 0 )
			{
				FPrintF( stderr, "Failed to get index for loopback interface lo0.\n" );
				err = kNoResourcesErr;
				goto exit;
			}
			interfaceDesc = "loopback interface";
			break;
		
		case kRegistrationInterfaceSet_AWDLOnly:
			err = _MDNSInterfaceGetAny( kMDNSInterfaceSubset_AWDL, NULL, &subtest->ifIndex );
			if( err == kNotFoundErr )
			{
				FPrintF( stderr, "Warning: No mDNS-capable AWDL interface is available.\n" );
				subtest->skipped = true;
				err = kNoErr;
			}
			require_noerr( err, exit );
			
			subtest->ifIsAWDL = true;
			interfaceDesc = "AWDL interface";
			break;
		
		default:
			err = kParamErr;
			goto exit;
	}
	
	// Create description.
	
	ASPrintF( &subtest->description, "Service registration over %s using %s service name.%s",
		interfaceDesc, subtest->useDefaultName ? "default" : "custom",
		subtest->useLODiscovery ? " (LocalOnly discovery)" : "" );
	require_action( subtest->description, exit, err = kNoMemoryErr );
	
	if( subtest->skipped )
	{
		subtest->startTime = NanoTimeGetCurrent();
	}
	else
	{
		// Generate a service name.
		
		if( subtest->useDefaultName )
		{
			subtest->serviceName = inTest->computerName;
		}
		else
		{
			ASPrintF( &subtest->serviceNameCustom, "dnssdutil-regtest-service-name-%s",
				_RandomStringExact( kLowerAlphaNumericCharSet, kLowerAlphaNumericCharSetSize, sizeof( tag ) - 1, tag ) );
			require_action( subtest->serviceNameCustom, exit, err = kNoMemoryErr );
			
			subtest->serviceName = subtest->serviceNameCustom;
		}
		
		// Generate a service type.
		
		ASPrintF( &subtest->serviceType, "_regtest-%s._udp",
			_RandomStringExact( kLowerAlphaNumericCharSet, kLowerAlphaNumericCharSetSize, sizeof( tag ) - 1, tag ) );
		require_action( subtest->serviceType, exit, err = kNoMemoryErr );
		
		subtest->serviceTypeLen = strlen( subtest->serviceType );
		
		// Create SRV and TXT record name FQDN.
		
		ASPrintF( &subtest->serviceFQDN, "%s.%s.local.", subtest->serviceName, subtest->serviceType );
		require_action( subtest->serviceFQDN, exit, err = kNoMemoryErr );
		
		// Generate a port number.
		
		subtest->port = (uint16_t) RandomRange( 60000, 65535 );
		
		// Generate TXT record data.
		
		err = _RegistrationTestCreateRandomTXTRecord( 100, 1000, &subtest->txtPtr, &subtest->txtLen );
		require_noerr( err, exit );
		
		// Register service.
		
		subtest->startTime = NanoTimeGetCurrent();
		
		flags = kDNSServiceFlagsNoAutoRename;
		if( subtest->includeAWDL ) flags |= kDNSServiceFlagsIncludeAWDL;
		err = DNSServiceRegister( &subtest->registration, flags, subtest->ifIndex,
			subtest->useDefaultName ? NULL : subtest->serviceNameCustom, subtest->serviceType, "local.",
			NULL, htons( subtest->port ), (uint16_t) subtest->txtLen, subtest->txtPtr,
			_RegistrationSubtestRegisterCallback, subtest );
		require_noerr( err, exit );
		
		err = DNSServiceSetDispatchQueue( subtest->registration, dispatch_get_main_queue() );
		require_noerr( err, exit );
		
		// Start timer.
		
		check( !inTest->timer );
		err = DispatchTimerOneShotCreate( dispatch_time_seconds( kRegistrationTestSubtestDurationSecs ),
			INT64_C_safe( kRegistrationTestSubtestDurationSecs ) * kNanosecondsPerSecond / 10, dispatch_get_main_queue(),
			_RegistrationTestTimerHandler, inTest, &inTest->timer );
		require_noerr( err, exit );
		dispatch_resume( inTest->timer );
	}
	
	*outSkipped = subtest->skipped;
	
	check( !inTest->subtest );
	inTest->subtest = subtest;
	subtest = NULL;
	
exit:
	_RegistrationSubtestForget( &subtest );
	return( err );
}

//===========================================================================================================================

#define kRegistrationTestReportKey_ComputerName			CFSTR( "computerName" )			// String
#define kRegistrationTestReportKey_Description			CFSTR( "description" )			// String
#define kRegistrationTestReportKey_Domain				CFSTR( "domain" )				// String
#define kRegistrationTestReportKey_EndTime				CFSTR( "endTime" )				// String
#define kRegistrationTestReportKey_Error				CFSTR( "error" )				// Integer
#define kRegistrationTestReportKey_Flags				CFSTR( "flags" )				// Integer
#define kRegistrationTestReportKey_IgnoredResults		CFSTR( "ignoredResults" )		// Array of dictionaries
#define kRegistrationTestReportKey_InterfaceIndex		CFSTR( "ifIndex" )				// Integer
#define kRegistrationTestReportKey_InterfaceName		CFSTR( "ifName" )				// String
#define kRegistrationTestReportKey_LocalHostName		CFSTR( "localHostName" )		// String
#define kRegistrationTestReportKey_MissingResults		CFSTR( "missingResults" )		// Array of dictionaries
#define kRegistrationTestReportKey_Pass					CFSTR( "pass" )					// Boolean
#define kRegistrationTestReportKey_Port					CFSTR( "port" )					// Integer
#define kRegistrationTestReportKey_RDataFormatted		CFSTR( "rdataFormatted" )		// String
#define kRegistrationTestReportKey_RDataHexString		CFSTR( "rdataHexString" )		// String
#define kRegistrationTestReportKey_RecordClass			CFSTR( "recordClass" )			// Integer
#define kRegistrationTestReportKey_RecordType			CFSTR( "recordType" )			// Integer
#define kRegistrationTestReportKey_Registered			CFSTR( "registered" )			// Boolean
#define kRegistrationTestReportKey_ResultType			CFSTR( "resultType" )			// String
#define kRegistrationTestReportKey_ServiceFQDN			CFSTR( "serviceFQDN" )			// String
#define kRegistrationTestReportKey_ServiceName			CFSTR( "serviceName" )			// String
#define kRegistrationTestReportKey_ServiceType			CFSTR( "serviceType" )			// String
#define kRegistrationTestReportKey_Skipped				CFSTR( "skipped" )				// Boolean
#define kRegistrationTestReportKey_StartTime			CFSTR( "startTime" )			// String
#define kRegistrationTestReportKey_Subtests				CFSTR( "subtests" )				// Array of dictionaries
#define kRegistrationTestReportKey_Timestamp			CFSTR( "timestamp" )			// String
#define kRegistrationTestReportKey_TXT					CFSTR( "txt" )					// String
#define kRegistrationTestReportKey_UnexpectedResults	CFSTR( "unexpectedResults" )	// Array of dictionaries
#define kRegistrationTestReportKey_UsedDefaultName		CFSTR( "usedDefaultName" )		// Boolean
#define kRegistrationTestReportKey_UsedLODiscovery		CFSTR( "usedLODiscovery" )		// Boolean

#define kRegistrationTestResultType_Browse				CFSTR( "browse" )
#define kRegistrationTestResultType_Query				CFSTR( "query" )
#define kRegistrationTestResultType_QuerySRV			CFSTR( "querySRV" )
#define kRegistrationTestResultType_QueryTXT			CFSTR( "queryTXT" )
#define kRegistrationTestResultType_Registration		CFSTR( "registration" )

static OSStatus
	_RegistrationTestAppendMissingResults(
		CFMutableArrayRef				inMissingResults,
		const RegistrationResultTimes *	inTimes,
		uint32_t						inIfIndex,
		const char *					inIfName );

static OSStatus	_RegistrationTestEndSubtest( RegistrationTest *inTest )
{
	OSStatus					err;
	RegistrationSubtest *		subtest;
	CFMutableDictionaryRef		subtestReport;
	CFMutableArrayRef			missing;
	char *						txtStr;
	NanoTime64					now;
	Boolean						subtestFailed;
	char						startTime[ 32 ];
	char						endTime[ 32 ];
	char						ifNameBuf[ IF_NAMESIZE + 1 ];
	
	now = NanoTimeGetCurrent();
	
	subtest = inTest->subtest;
	inTest->subtest = NULL;
	_RegistrationSubtestStop( subtest );
	
	missing			= NULL;
	subtestReport	= NULL;
	txtStr			= NULL;
	if( subtest->txtPtr )
	{
		err = DNSRecordDataToString( subtest->txtPtr, subtest->txtLen, kDNSServiceType_TXT, NULL, 0, &txtStr );
		require_noerr( err, exit );
	}
	_NanoTime64ToTimestamp( subtest->startTime, startTime, sizeof( startTime ) );
	_NanoTime64ToTimestamp( now, endTime, sizeof( endTime ) );
	err = CFPropertyListCreateFormatted( kCFAllocatorDefault, &subtestReport,
		"{"
			"%kO=%s"	// description
			"%kO=%s"	// startTime
			"%kO=%s"	// endTime
			"%kO=%s"	// serviceFQDN
			"%kO=%lli"	// ifIndex
			"%kO=%s"	// ifName
			"%kO=%lli"	// port
			"%kO=%s"	// txt
			"%kO=%b"	// registered
			"%kO=%b"	// usedDefaultName
			"%kO=%b"	// usedLODiscovery
		"}",
		kRegistrationTestReportKey_Description,		subtest->description,
		kRegistrationTestReportKey_StartTime,		startTime,
		kRegistrationTestReportKey_EndTime,			endTime,
		kRegistrationTestReportKey_ServiceFQDN,		subtest->serviceFQDN,
		kRegistrationTestReportKey_InterfaceIndex,	(int64_t) subtest->ifIndex,
		kRegistrationTestReportKey_InterfaceName,	if_indextoname( subtest->ifIndex, ifNameBuf ),
		kRegistrationTestReportKey_Port,			(int64_t) subtest->port,
		kRegistrationTestReportKey_TXT,				txtStr,
		kRegistrationTestReportKey_Registered,		(int) subtest->registered,
		kRegistrationTestReportKey_UsedDefaultName,	(int) subtest->useDefaultName,
		kRegistrationTestReportKey_UsedLODiscovery,	(int) subtest->useLODiscovery );
	ForgetMem( &txtStr );
	require_noerr( err, exit );
	
	if( !subtest->skipped && subtest->registered )
	{
		missing = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
		require_action( missing, exit, err = kNoMemoryErr );
		
		if( subtest->ifList )
		{
			RegistrationInterfaceItem *		item;
			
			for( item = subtest->ifList; item; item = (RegistrationInterfaceItem *) item->base.next )
			{
			#if( TARGET_OS_WATCH )
				if( inTest->forBATS && item->base.isWiFi ) continue;
			#endif
				err = _RegistrationTestAppendMissingResults( missing, &item->times, item->base.ifIndex, item->base.ifName );
				require_noerr( err, exit );
			}
		}
		else
		{
			err = _RegistrationTestAppendMissingResults( missing, &subtest->ifTimes, subtest->ifIndex, NULL );
			require_noerr( err, exit );
		}
		
		subtestFailed = false;
		if( CFArrayGetCount( missing ) > 0 )
		{
			subtestFailed = true;
			CFDictionarySetValue( subtestReport, kRegistrationTestReportKey_MissingResults, missing );
		}
		if( CFArrayGetCount( subtest->unexpected ) > 0 )
		{
			subtestFailed = true;
			CFDictionarySetValue( subtestReport, kRegistrationTestReportKey_UnexpectedResults, subtest->unexpected );
		}
	#if( TARGET_OS_WATCH )
		if( CFArrayGetCount( subtest->ignored ) > 0 )
		{
			CFDictionarySetValue( subtestReport, kRegistrationTestReportKey_IgnoredResults, subtest->ignored );
		}
	#endif
	}
	else
	{
		subtestFailed = true;
	}
	
	CFDictionarySetBoolean( subtestReport, kRegistrationTestReportKey_Pass, subtestFailed ? false : true );
	if( subtestFailed )
	{
		CFDictionarySetBoolean( subtestReport, kRegistrationTestReportKey_Skipped, subtest->skipped );
		if( !subtest->skipped ) inTest->failed = true;
	}
	CFArrayAppendValue( inTest->subtestReports, subtestReport );
	
exit:
	CFReleaseNullSafe( missing );
	CFReleaseNullSafe( subtestReport );
	_RegistrationSubtestFree( subtest );
	return( err );
}

static OSStatus
	_RegistrationTestAppendMissingResult(
		CFMutableArrayRef	inMissingResults,
		CFStringRef			inType,
		uint32_t			inIfIndex,
		const char *		inIfName );

static OSStatus
	_RegistrationTestAppendMissingResults(
		CFMutableArrayRef				inMissingResults,
		const RegistrationResultTimes *	inTimes,
		uint32_t						inIfIndex,
		const char *					inIfName )
{
	OSStatus		err;
	
	if( !inTimes->browseResultTime )
	{
		err = _RegistrationTestAppendMissingResult( inMissingResults, kRegistrationTestResultType_Browse,
			inIfIndex, inIfName );
		require_noerr( err, exit );
	}
	if( !inTimes->querySRVResultTime )
	{
		err = _RegistrationTestAppendMissingResult( inMissingResults, kRegistrationTestResultType_QuerySRV,
			inIfIndex, inIfName );
		require_noerr( err, exit );
	}
	if( !inTimes->queryTXTResultTime )
	{
		err = _RegistrationTestAppendMissingResult( inMissingResults, kRegistrationTestResultType_QueryTXT,
			inIfIndex, inIfName );
		require_noerr( err, exit );
	}
	err = kNoErr;
	
exit:
	return( err );
}

static OSStatus
	_RegistrationTestAppendMissingResult(
		CFMutableArrayRef	inMissingResults,
		CFStringRef			inType,
		uint32_t			inIfIndex,
		const char *		inIfName )
{
	OSStatus		err;
	char			ifName[ IF_NAMESIZE + 1 ];
	
	err = CFPropertyListAppendFormatted( kCFAllocatorDefault, inMissingResults,
		"{"
			"%kO=%O"	// resultType
			"%kO=%lli"	// ifIndex
			"%kO=%s"	// ifName
		"}",
		kRegistrationTestReportKey_ResultType,		inType,
		kRegistrationTestReportKey_InterfaceIndex,	(int64_t) inIfIndex,
		kRegistrationTestReportKey_InterfaceName,	inIfName ? inIfName : if_indextoname( inIfIndex, ifName ) );
	return( err );
}

//===========================================================================================================================

static void	_RegistrationTestEnd( RegistrationTest *inTest )
{
	OSStatus				err;
	NanoTime64				now;
	CFPropertyListRef		plist;
	char					startTime[ 32 ];
	char					endTime[ 32 ];
	
	now = NanoTimeGetCurrent();
	_NanoTime64ToTimestamp( inTest->startTime, startTime, sizeof( startTime ) );
	_NanoTime64ToTimestamp( now, endTime, sizeof( endTime ) );
	
	err = CFPropertyListCreateFormatted( kCFAllocatorDefault, &plist,
		"{"
			"%kO=%s"	// startTime
			"%kO=%s"	// endTime
			"%kO=%s"	// computerName
			"%kO=%s"	// localHostName
			"%kO=%O"	// subtests
			"%kO=%b"	// pass
		"}",
		kRegistrationTestReportKey_StartTime,		startTime,
		kRegistrationTestReportKey_EndTime,			endTime,
		kRegistrationTestReportKey_ComputerName,	inTest->computerName,
		kRegistrationTestReportKey_LocalHostName,	inTest->localHostName,
		kRegistrationTestReportKey_Subtests,		inTest->subtestReports,
		kRegistrationTestReportKey_Pass,			inTest->failed ? false : true );
	require_noerr( err, exit );
	
	err = OutputPropertyList( plist, inTest->outputFormat, inTest->outputFilePath );
	CFRelease( plist );
	require_noerr( err, exit );
	
exit:
	_RegistrationTestExit( inTest, err );
}

//===========================================================================================================================

static void	_RegistrationTestExit( RegistrationTest *inTest, OSStatus inError )
{
	int		exitCode;
	
	if( inError )
	{
		FPrintF( stderr, "error: %#m\n", inError );
		exitCode = 1;
	}
	else
	{
		exitCode = inTest->failed ? 2 : 0;
	}
	_RegistrationTestForget( &inTest );
	exit( exitCode );
}

//===========================================================================================================================

static OSStatus	_RegistrationSubtestCreate( RegistrationSubtest **outSubtest )
{
	OSStatus					err;
	RegistrationSubtest *		obj;
	
	obj = (RegistrationSubtest *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	obj->unexpected = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	require_action( obj->unexpected, exit, err = kNoMemoryErr );
	
#if( TARGET_OS_WATCH )
	obj->ignored = CFArrayCreateMutable( NULL, 0, &kCFTypeArrayCallBacks );
	require_action( obj->ignored, exit, err = kNoMemoryErr );
#endif
	
	*outSubtest = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	if( obj ) _RegistrationSubtestFree( obj );
	return( err );
}

//===========================================================================================================================

static void	_RegistrationSubtestFree( RegistrationSubtest *inSubtest )
{
	check( !inSubtest->registration );
	check( !inSubtest->browse );
	check( !inSubtest->querySRV );
	check( !inSubtest->queryTXT );
	check( !inSubtest->connection );
	ForgetMem( &inSubtest->serviceNameCustom );
	ForgetMem( &inSubtest->serviceType );
	ForgetMem( &inSubtest->serviceFQDN );
	ForgetMem( &inSubtest->txtPtr );
	ForgetCF( &inSubtest->unexpected );
#if( TARGET_OS_WATCH )
	ForgetCF( &inSubtest->ignored );
#endif
	_MDNSInterfaceListForget( (MDNSInterfaceItem **) &inSubtest->ifList );
	ForgetMem( &inSubtest->description );
	free( inSubtest );
}

//===========================================================================================================================

static void	_RegistrationSubtestStop( RegistrationSubtest *inSubtest )
{
	DNSServiceForget( &inSubtest->registration );
	DNSServiceForget( &inSubtest->browse );
	DNSServiceForget( &inSubtest->querySRV );
	DNSServiceForget( &inSubtest->queryTXT );
	DNSServiceForget( &inSubtest->connection );
}

//===========================================================================================================================

static OSStatus	_RegistrationTestInterfaceListCreate( Boolean inIncludeAWDL, RegistrationInterfaceItem **outList )
{
	OSStatus						err;
	RegistrationInterfaceItem *		list;
	const MDNSInterfaceSubset		subset = inIncludeAWDL ? kMDNSInterfaceSubset_All : kMDNSInterfaceSubset_NonAWDL;
	
	err = _MDNSInterfaceListCreate( subset, sizeof( *list ), (MDNSInterfaceItem **) &list );
	require_noerr( err, exit );
	
	*outList = list;
	
exit:
	return( err );
}

//===========================================================================================================================

static OSStatus
	_RegistrationTestCreateRandomTXTRecord(
		size_t		inMinLen,
		size_t		inMaxLen,
		uint8_t **	outTXTPtr,
		size_t *	outTXTLen )
{
	OSStatus			err;
	uint8_t *			ptr;
	const uint8_t *		txtEnd;
	uint8_t *			txtPtr = NULL;
	size_t				txtLen;
	
	require_action_quiet( inMinLen <= inMaxLen, exit, err = kSizeErr );
	
	txtLen = RandomRange( inMinLen, inMaxLen );
	txtPtr = (uint8_t *) malloc( txtLen + 1 );
	require_action( txtPtr, exit, err = kNoMemoryErr );
	
	_RandomStringExact( kAlphaNumericCharSet, sizeof_string( kAlphaNumericCharSet ), txtLen, (char *)txtPtr );
	
	ptr		= txtPtr;
	txtEnd	= &txtPtr[ txtLen ];
	while( ptr < txtEnd )
	{
		size_t		maxLen, len;
		
		maxLen = ( (size_t)( txtEnd - ptr ) ) - 1;
		len = RandomRange( 1, 255 );
		if( len > maxLen ) len = maxLen;
		
		*ptr = (uint8_t) len;
		ptr += ( 1 + len );
	}
	check( ptr == txtEnd );
	
	if( outTXTPtr )
	{
		*outTXTPtr = txtPtr;
		txtPtr = NULL;
	}
	if( outTXTLen ) *outTXTLen = txtLen;
	err = kNoErr;
	
exit:
	FreeNullSafe( txtPtr );
	return( err );
}

//===========================================================================================================================

static void DNSSD_API
	_RegistrationSubtestRegisterCallback(
		DNSServiceRef		inSDRef,
		DNSServiceFlags		inFlags,
		DNSServiceErrorType	inError,
		const char *		inServiceName,
		const char *		inServiceType,
		const char *		inDomain,
		void *				inContext )
{
	OSStatus						err;
	const NanoTime64				now		= NanoTimeGetCurrent();
	RegistrationSubtest * const		subtest	= (RegistrationSubtest *) inContext;
	
	Unused( inSDRef );
	
	if( ( inFlags & kDNSServiceFlagsAdd ) && !inError &&
		( strcasecmp( inServiceName, subtest->serviceName ) == 0 ) &&
		_RegistrationSubtestValidServiceType( subtest, inServiceType ) &&
		( strcasecmp( inDomain, "local." ) == 0 ) )
	{
		if( !subtest->registered )
		{
			DNSServiceRef				sdRef;
			const DNSServiceFlags		flags = kDNSServiceFlagsShareConnection | kDNSServiceFlagsIncludeAWDL;
			
			subtest->registered = true;
			
			// Create shared connection.
			
			check( !subtest->connection );
			err = DNSServiceCreateConnection( &subtest->connection );
			require_noerr( err, exit );
			
			err = DNSServiceSetDispatchQueue( subtest->connection, dispatch_get_main_queue() );
			require_noerr( err, exit );
			
			// Start browse.
			
			check( !subtest->browse );
			sdRef = subtest->connection;
			err = DNSServiceBrowse( &sdRef, flags,
				subtest->useLODiscovery ? kDNSServiceInterfaceIndexLocalOnly : kDNSServiceInterfaceIndexAny,
				subtest->serviceType, "local.", _RegistrationSubtestBrowseCallback, subtest );
			require_noerr( err, exit );
			
			subtest->browse = sdRef;
		}
	}
	else
	{
		char		timestamp[ 32 ];
		
		err = CFPropertyListAppendFormatted( kCFAllocatorDefault, subtest->unexpected,
			"{"
				"%kO=%O"	// resultType
				"%kO=%s"	// timestamp
				"%kO=%lli"	// flags
				"%kO=%lli"	// error
				"%kO=%s"	// serviceName
				"%kO=%s"	// serviceType
				"%kO=%s"	// domain
			"}",
			kRegistrationTestReportKey_ResultType,	kRegistrationTestResultType_Registration,
			kRegistrationTestReportKey_Timestamp,	_NanoTime64ToTimestamp( now, timestamp, sizeof( timestamp ) ),
			kRegistrationTestReportKey_Flags,		(int64_t) inFlags,
			kRegistrationTestReportKey_Error,		(int64_t) inError,
			kRegistrationTestReportKey_ServiceName,	inServiceName,
			kRegistrationTestReportKey_ServiceType,	inServiceType,
			kRegistrationTestReportKey_Domain,		inDomain );
		require_noerr( err, exit );
	}
	err = kNoErr;
	
exit:
	if( err ) _RegistrationTestExit( subtest->test, err );
}

//===========================================================================================================================

static void DNSSD_API
	_RegistrationSubtestBrowseCallback(
		DNSServiceRef		inSDRef,
		DNSServiceFlags		inFlags,
		uint32_t			inIfIndex,
		DNSServiceErrorType	inError,
		const char *		inServiceName,
		const char *		inServiceType,
		const char *		inDomain,
		void *				inContext )
{
	OSStatus						err;
	NanoTime64						now;
	RegistrationSubtest * const		subtest = (RegistrationSubtest *) inContext;
	Boolean							serviceIsCorrect, resultIsExpected;
	
	Unused( inSDRef );
	
	now = NanoTimeGetCurrent();
	if( !inError && ( strcasecmp( inServiceName, subtest->serviceName ) == 0 ) &&
		_RegistrationSubtestValidServiceType( subtest, inServiceType ) && ( strcasecmp( inDomain, "local." ) == 0 ) )
	{
		serviceIsCorrect = true;
	}
	else
	{
		serviceIsCorrect = false;
	}
	
	resultIsExpected = false;
	if( serviceIsCorrect && ( inFlags & kDNSServiceFlagsAdd ) )
	{
		RegistrationResultTimes *		times;
		
		times = _RegistrationSubtestGetInterfaceResultTimes( subtest, inIfIndex, NULL );
		if( times )
		{
			DNSServiceRef				sdRef;
			const DNSServiceFlags		flags = kDNSServiceFlagsShareConnection | kDNSServiceFlagsIncludeAWDL;
			uint32_t					ifIndex;
			
			resultIsExpected = true;
			if( !times->browseResultTime ) times->browseResultTime = now;
			
			ifIndex = subtest->useLODiscovery ? kDNSServiceInterfaceIndexLocalOnly : kDNSServiceInterfaceIndexAny;
			if( !subtest->querySRV )
			{
				// Start SRV record query.
				
				sdRef = subtest->connection;
				err = DNSServiceQueryRecord( &sdRef, flags, ifIndex, subtest->serviceFQDN, kDNSServiceType_SRV,
					kDNSServiceClass_IN, _RegistrationSubtestQueryCallback, subtest );
				require_noerr( err, exit );
				
				subtest->querySRV = sdRef;
			}
			if( !subtest->queryTXT )
			{
				// Start TXT record query.
				
				sdRef = subtest->connection;
				err = DNSServiceQueryRecord( &sdRef, flags, ifIndex, subtest->serviceFQDN, kDNSServiceType_TXT,
					kDNSServiceClass_IN, _RegistrationSubtestQueryCallback, subtest );
				require_noerr( err, exit );
				
				subtest->queryTXT = sdRef;
			}
		}
	}
	
	if( !resultIsExpected )
	{
		CFMutableArrayRef		resultArray;
		char					timestamp[ 32 ];
		const char *			ifNamePtr;
		char					ifNameBuf[ IF_NAMESIZE + 1 ];
		
		ifNamePtr = if_indextoname( inIfIndex, ifNameBuf );
		resultArray = subtest->unexpected;
	#if( TARGET_OS_WATCH )
		if( subtest->test->forBATS && ( subtest->ifIndex == kDNSServiceInterfaceIndexAny ) &&
			ifNamePtr && _RegistrationTestInterfaceIsWiFi( ifNamePtr ) && serviceIsCorrect )
		{
			resultArray = subtest->ignored;
		}
	#endif
		err = CFPropertyListAppendFormatted( kCFAllocatorDefault, resultArray,
			"{"
				"%kO=%O"	// resultType
				"%kO=%s"	// timestamp
				"%kO=%lli"	// flags
				"%kO=%lli"	// ifIndex
				"%kO=%s"	// ifName
				"%kO=%lli"	// error
				"%kO=%s"	// serviceName
				"%kO=%s"	// serviceType
				"%kO=%s"	// domain
			"}",
			kRegistrationTestReportKey_ResultType,		kRegistrationTestResultType_Browse,
			kRegistrationTestReportKey_Timestamp,		_NanoTime64ToTimestamp( now, timestamp, sizeof( timestamp ) ),
			kRegistrationTestReportKey_Flags,			(int64_t) inFlags,
			kRegistrationTestReportKey_InterfaceIndex,	(int64_t) inIfIndex,
			kRegistrationTestReportKey_InterfaceName,	ifNamePtr,
			kRegistrationTestReportKey_Error,			(int64_t) inError,
			kRegistrationTestReportKey_ServiceName,		inServiceName,
			kRegistrationTestReportKey_ServiceType,		inServiceType,
			kRegistrationTestReportKey_Domain,			inDomain );
		require_noerr( err, exit );
	}
	err = kNoErr;
	
exit:
	if( err ) _RegistrationTestExit( subtest->test, err );
}

//===========================================================================================================================

static Boolean
	_RegistrationSubtestIsSRVRecordDataValid(
		RegistrationSubtest *	inSubtest,
		const uint8_t *			inRDataPtr,
		size_t					inRDataLen,
		Boolean					inExpectRandHostname );

static void DNSSD_API
	_RegistrationSubtestQueryCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inIfIndex,
		DNSServiceErrorType		inError,
		const char *			inName,
		uint16_t				inType,
		uint16_t				inClass,
		uint16_t				inRDataLen,
		const void *			inRDataPtr,
		uint32_t				inTTL,
		void *					inContext )
{
	OSStatus						err;
	NanoTime64						now;
	RegistrationSubtest * const		subtest = (RegistrationSubtest *) inContext;
	Boolean							resultIsExpected;
	
	Unused( inSDRef );
	Unused( inTTL );
	
	now = NanoTimeGetCurrent();
	resultIsExpected = false;
	if( ( inFlags & kDNSServiceFlagsAdd ) && !inError && ( strcasecmp( inName, subtest->serviceFQDN ) == 0 ) &&
		( inClass == kDNSServiceClass_IN ) )
	{
		RegistrationResultTimes *		times;
		Boolean							isAWDL;
		
		times = _RegistrationSubtestGetInterfaceResultTimes( subtest, inIfIndex, &isAWDL );
		if( times )
		{
			if( inType == kDNSServiceType_SRV )
			{
				Boolean		expectRandHostname;
				
				if( isAWDL || ( ( subtest->ifIndex == kDNSServiceInterfaceIndexAny ) && subtest->includeAWDL ) )
				{
					expectRandHostname = true;
				}
				else
				{
					expectRandHostname = false;
				}
				if( _RegistrationSubtestIsSRVRecordDataValid( subtest, inRDataPtr, inRDataLen, expectRandHostname ) )
				{
					resultIsExpected = true;
					if( !times->querySRVResultTime ) times->querySRVResultTime = now;
				}
			}
			else if( inType == kDNSServiceType_TXT )
			{
				if( MemEqual( inRDataPtr, inRDataLen, subtest->txtPtr, subtest->txtLen ) )
				{
					resultIsExpected = true;
					if( !times->queryTXTResultTime ) times->queryTXTResultTime = now;
				}
			}
		}
	}
	
	if( !resultIsExpected )
	{
		CFMutableArrayRef			resultArray;
		CFMutableDictionaryRef		resultDict;
		CFStringRef					rdataKey;
		char *						rdataStr;
		const char *				ifNamePtr;
		char						timestamp[ 32 ];
		char						ifNameBuf[ IF_NAMESIZE + 1 ];
		
		ifNamePtr = if_indextoname( inIfIndex, ifNameBuf );
		resultArray = subtest->unexpected;
	#if( TARGET_OS_WATCH )
		if( subtest->test->forBATS && ( subtest->ifIndex == kDNSServiceInterfaceIndexAny ) &&
			ifNamePtr && _RegistrationTestInterfaceIsWiFi( ifNamePtr ) && !inError &&
			( strcasecmp( inName, subtest->serviceFQDN ) == 0 ) )
		{
			if( inType == kDNSServiceType_SRV )
			{
				const Boolean		expectRandHostname = subtest->includeAWDL ? true : false;
				
				if( _RegistrationSubtestIsSRVRecordDataValid( subtest, inRDataPtr, inRDataLen, expectRandHostname ) )
				{
					resultArray = subtest->ignored;
				}
			}
			else if( inType == kDNSServiceType_TXT )
			{
				if( MemEqual( inRDataPtr, inRDataLen, subtest->txtPtr, subtest->txtLen ) )
				{
					resultArray = subtest->ignored;
				}
			}
		}
	#endif
		err = CFPropertyListCreateFormatted( kCFAllocatorDefault, &resultDict,
			"{"
				"%kO=%O"	// resultType
				"%kO=%s"	// timestamp
				"%kO=%lli"	// flags
				"%kO=%lli"	// ifIndex
				"%kO=%s"	// ifName
				"%kO=%lli"	// error
				"%kO=%s"	// serviceFQDN
				"%kO=%lli"	// recordType
				"%kO=%lli"	// class
			"}",
			kRegistrationTestReportKey_ResultType,		kRegistrationTestResultType_Query,
			kRegistrationTestReportKey_Timestamp,		_NanoTime64ToTimestamp( now, timestamp, sizeof( timestamp ) ),
			kRegistrationTestReportKey_Flags,			(int64_t) inFlags,
			kRegistrationTestReportKey_InterfaceIndex,	(int64_t) inIfIndex,
			kRegistrationTestReportKey_InterfaceName,	ifNamePtr,
			kRegistrationTestReportKey_Error,			(int64_t) inError,
			kRegistrationTestReportKey_ServiceFQDN,		inName,
			kRegistrationTestReportKey_RecordType,		(int64_t) inType,
			kRegistrationTestReportKey_RecordClass,		(int64_t) inClass );
		require_noerr( err, exit );
		
		rdataStr = NULL;
		DNSRecordDataToString( inRDataPtr, inRDataLen, inType, NULL, 0, &rdataStr );
		if( rdataStr )
		{
			rdataKey = kRegistrationTestReportKey_RDataFormatted;
		}
		else
		{
			ASPrintF( &rdataStr, "%#H", inRDataPtr, inRDataLen, inRDataLen );
			require_action( rdataStr, exit, err = kNoMemoryErr );
			
			rdataKey = kRegistrationTestReportKey_RDataHexString;
		}
		err = CFDictionarySetCString( resultDict, rdataKey, rdataStr, kSizeCString );
		ForgetMem( &rdataStr );
		if( err ) CFRelease( resultDict );
		require_noerr( err, exit );
		
		CFArrayAppendValue( resultArray, resultDict );
		CFRelease( resultDict );
	}
	err = kNoErr;
	
exit:
	if( err ) _RegistrationTestExit( subtest->test, err );
}

static Boolean
	_RegistrationSubtestIsSRVRecordDataValid(
		RegistrationSubtest *	inSubtest,
		const uint8_t *			inRDataPtr,
		size_t					inRDataLen,
		Boolean					inExpectRandHostname )
{
	const dns_fixed_fields_srv *		fields;
	const uint8_t * const				end		= &inRDataPtr[ inRDataLen ];
	const uint8_t *						label;
	size_t								len;
	uint16_t							port;
	Boolean								isValid;
	
	isValid = false;
	require_quiet( inRDataLen >= sizeof( dns_fixed_fields_srv ), exit );
	
	fields	= (const dns_fixed_fields_srv *) inRDataPtr;
	port	= dns_fixed_fields_srv_get_port( fields );
	require_quiet( port == inSubtest->port, exit );
	
	// First target label should be a UUID string for the AWDL interface.
	
	label = (const uint8_t *) &fields[ 1 ];
	require_quiet( ( end - label ) >= 1, exit );
	
	len = label[ 0 ];
	require_quiet( ( (size_t)( end - label ) ) >= ( 1 + len ), exit );
	
	if( inExpectRandHostname )
	{
		if( StringToUUID( (const char *) &label[ 1 ], len, false, NULL ) != kNoErr ) goto exit;
	}
	else
	{
		if( strnicmpx( &label[ 1 ], len, inSubtest->test->localHostName ) != 0 ) goto exit;
	}
	
	// Second target label should be "local".
	
	label = &label[ 1 + len ];
	require_quiet( ( end - label ) >= 1, exit );
	
	len = label[ 0 ];
	require_quiet( ( (size_t)( end - label ) ) >= ( 1 + len ), exit );
	
	if( ( len != kLocalLabel[ 0 ] ) || ( _memicmp( &label[ 1 ], &kLocalLabel[ 1 ], kLocalLabel[ 0 ] ) != 0 ) ) goto exit;
	
	// Third target label should be the root label.
	
	label = &label[ 1 + len ];
	require_quiet( ( end - label ) >= 1, exit );
	
	len = label[ 0 ];
	if( len != 0 ) goto exit;
	
	isValid = true;
	
exit:
	return( isValid );
}

//===========================================================================================================================

static Boolean	_RegistrationSubtestValidServiceType( const RegistrationSubtest *inSubtest, const char *inServiceType )
{
	if( stricmp_prefix( inServiceType, inSubtest->serviceType ) == 0 )
	{
		const char * const		ptr = &inServiceType[ inSubtest->serviceTypeLen ];
		
		if( ( ptr[ 0 ] == '\0' ) || ( ( ptr[ 0 ] == '.' ) && ( ptr[ 1 ] == '\0' ) ) ) return( true );
	}
	return( false );
}

//===========================================================================================================================

static RegistrationResultTimes *
	_RegistrationSubtestGetInterfaceResultTimes(
		RegistrationSubtest *	inSubtest,
		uint32_t				inIfIndex,
		Boolean *				outIsAWDL )
{
	if( inSubtest->ifList )
	{
		RegistrationInterfaceItem *		item;
		
		for( item = inSubtest->ifList; item; item = (RegistrationInterfaceItem *) item->base.next )
		{
			if( inIfIndex == item->base.ifIndex )
			{
				if( outIsAWDL ) *outIsAWDL = item->base.isAWDL ? true : false;
				return( &item->times );
			}
		}
	}
	else
	{
		if( inIfIndex == inSubtest->ifIndex )
		{
			if( outIsAWDL ) *outIsAWDL = inSubtest->ifIsAWDL ? true : false;
			return( &inSubtest->ifTimes );
		}
	}
	return( NULL );
}

//===========================================================================================================================

static void	_RegistrationTestTimerHandler( void *inContext )
{
	RegistrationTest * const		test = (RegistrationTest *) inContext;
	
	dispatch_source_forget( &test->timer );
	_RegistrationTestProceed( test );
}

//===========================================================================================================================

#if( TARGET_OS_WATCH )
static Boolean	_RegistrationTestInterfaceIsWiFi( const char *inIfName )
{
	NetTransportType		type = kNetTransportType_Undefined;
	
	SocketGetInterfaceInfo( kInvalidSocketRef, inIfName, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &type );
	return( ( type == kNetTransportType_WiFi ) ? true : false );
}
#endif

#if( TARGET_OS_DARWIN )
//===========================================================================================================================
//	KeepAliveTestCmd
//===========================================================================================================================

typedef enum
{
	kKeepAliveCallVariant_Null				= 0,
	kKeepAliveCallVariant_TakesSocket		= 1, // DNSServiceSleepKeepalive(), which takes a connected socket.
	kKeepAliveCallVariant_TakesSockAddrs	= 2, // DNSServiceSleepKeepalive_sockaddr(), which takes connection's sockaddrs.
	
}	KeepAliveCallVariant;

typedef struct
{
	int							family;			// TCP connection's address family.
	KeepAliveCallVariant		callVariant;	// Describes which DNSServiceSleepKeepalive* call to use.
	const char *				description;
	
}	KeepAliveSubtestParams;

const KeepAliveSubtestParams		kKeepAliveSubtestParams[] =
{
	{ AF_INET,  kKeepAliveCallVariant_TakesSocket,    "Calls DNSServiceSleepKeepalive() for IPv4 TCP connection." },
	{ AF_INET,  kKeepAliveCallVariant_TakesSockAddrs, "Calls DNSServiceSleepKeepalive_sockaddr() for IPv4 TCP connection." },
	{ AF_INET6, kKeepAliveCallVariant_TakesSocket,    "Calls DNSServiceSleepKeepalive() for IPv6 TCP connection." },
	{ AF_INET6, kKeepAliveCallVariant_TakesSockAddrs, "Calls DNSServiceSleepKeepalive_sockaddr() for IPv6 TCP connection." }
};

typedef struct
{
	sockaddr_ip			local;			// TCP connection's local address and port.
	sockaddr_ip			remote;			// TCP connection's remote address and port.
	NanoTime64			startTime;		// Subtest's start time.
	NanoTime64			endTime;		// Subtest's end time.
	SocketRef			clientSock;		// Socket for client-side of TCP connection.
	SocketRef			serverSock;		// Socket for server-side of TCP connection.
	char *				recordName;		// Keepalive record's name.
	char *				dataStr;		// Data expected to be contained in keepalive record's data.
	const char *		description;	// Subtests's description.
	unsigned int		timeoutKA;		// Randomly-generated timeout value that gets put in keepalive record's rdata.
	OSStatus			error;			// Subtest's error.
	
}	KeepAliveSubtest;

typedef struct KeepAliveTest *		KeepAliveTestRef;

typedef struct
{
	KeepAliveTestRef		test;	// Weak back pointer to test.
	
}	KeepAliveTestConnectionContext;

struct KeepAliveTest
{
	dispatch_queue_t						queue;			// Serial queue for test events.
	dispatch_semaphore_t					doneSem;		// Semaphore to signal when the test is done.
	dispatch_source_t						readSource;		// Read source for TCP listener socket.
	DNSServiceRef							keepalive;		// DNSServiceSleepKeepalive{,2} operation.
	DNSServiceRef							query;			// Query to verify registered keepalive record.
	dispatch_source_t						timer;			// Timer to put time limit on query.
	AsyncConnectionRef						connection;		// Establishes current subtest's TCP connection.
	KeepAliveTestConnectionContext *		connectionCtx;	// Weak pointer to connection's context.
	NanoTime64								startTime;		// Test's start time.
	NanoTime64								endTime;		// Test's end time.
	OSStatus								error;			// Test's error.
	size_t									subtestIdx;		// Index of current subtest.
	KeepAliveSubtest						subtests[ 4 ];	// Subtest array.
};
check_compile_time( countof_field( struct KeepAliveTest, subtests ) == countof( kKeepAliveSubtestParams ) );

ulog_define_ex( kDNSSDUtilIdentifier, KeepAliveTest, kLogLevelInfo, kLogFlags_None, "KeepAliveTest", NULL );
#define kat_ulog( LEVEL, ... )		ulog( &log_category_from_name( KeepAliveTest ), (LEVEL), __VA_ARGS__ )

static OSStatus	_KeepAliveTestCreate( KeepAliveTestRef *outTest );
static OSStatus	_KeepAliveTestRun( KeepAliveTestRef inTest );
static void		_KeepAliveTestFree( KeepAliveTestRef inTest );

static void	KeepAliveTestCmd( void )
{
	OSStatus				err;
	OutputFormatType		outputFormat;
	KeepAliveTestRef		test		= NULL;
	CFPropertyListRef		plist		= NULL;
	CFMutableArrayRef		subtests;
	size_t					i;
	size_t					subtestFailCount;
	Boolean					testPassed	= false;
	char					startTime[ 32 ];
	char					endTime[ 32 ];
	
	err = OutputFormatFromArgString( gKeepAliveTest_OutputFormat, &outputFormat );
	require_noerr_quiet( err, exit );
	
	err = _KeepAliveTestCreate( &test );
	require_noerr( err, exit );
	
	err = _KeepAliveTestRun( test );
	require_noerr( err, exit );
	
	_NanoTime64ToTimestamp( test->startTime, startTime, sizeof( startTime ) );
	_NanoTime64ToTimestamp( test->endTime, endTime, sizeof( endTime ) );
	err = CFPropertyListCreateFormatted( kCFAllocatorDefault, &plist,
		"{"
			"%kO=%s"	// startTime
			"%kO=%s"	// endTime
			"%kO=[%@]"	// subtests
		"}",
		CFSTR( "startTime" ),	startTime,
		CFSTR( "endTime" ),		endTime,
		CFSTR( "subtests" ),	&subtests );
	require_noerr( err, exit );
	
	subtestFailCount = 0;
	check( test->subtestIdx == countof( test->subtests ) );
	for( i = 0; i < countof( test->subtests ); ++i )
	{
		KeepAliveSubtest * const		subtest = &test->subtests[ i ];
		char							errorDesc[ 128 ];
		
		_NanoTime64ToTimestamp( subtest->startTime, startTime, sizeof( startTime ) );
		_NanoTime64ToTimestamp( subtest->endTime, endTime, sizeof( endTime ) );
		SNPrintF( errorDesc, sizeof( errorDesc ), "%m", subtest->error );
		err = CFPropertyListAppendFormatted( kCFAllocatorDefault, subtests,
			"{"
				"%kO=%s"		// startTime
				"%kO=%s"		// endTime
				"%kO=%s"		// description
				"%kO=%##a"		// localAddr
				"%kO=%##a"		// remoteAddr
				"%kO=%s"		// recordName
				"%kO=%s"		// expectedRData
				"%kO="			// error
				"{"
					"%kO=%lli"	// code
					"%kO=%s"	// description
				"}"
			"}",
			CFSTR( "startTime" ),		startTime,
			CFSTR( "endTime" ),			endTime,
			CFSTR( "description" ),		subtest->description,
			CFSTR( "localAddr" ),		&subtest->local.sa,
			CFSTR( "remoteAddr" ),		&subtest->remote.sa,
			CFSTR( "recordName" ),		subtest->recordName,
			CFSTR( "expectedRData" ),	subtest->dataStr,
			CFSTR( "error" ),
			CFSTR( "code" ),			(int64_t) subtest->error,
			CFSTR( "description" ),		errorDesc
		);
		require_noerr( err, exit );
		if( subtest->error ) ++subtestFailCount;
	}
	if( subtestFailCount == 0 ) testPassed = true;
	CFPropertyListAppendFormatted( kCFAllocatorDefault, plist, "%kO=%b", CFSTR( "pass" ), testPassed );
	
	err = OutputPropertyList( plist, outputFormat, gKeepAliveTest_OutputFilePath );
	require_noerr( err, exit );
	
exit:
	if( test ) _KeepAliveTestFree( test );
	CFReleaseNullSafe( plist );
    gExitCode = err ? 1 : ( testPassed ? 0 : 2 );
}

//===========================================================================================================================

static void					_KeepAliveTestStart( void *inCtx );
static void					_KeepAliveTestStop( KeepAliveTestRef inTest, OSStatus inError );
static OSStatus				_KeepAliveTestStartSubtest( KeepAliveTestRef inTest );
static void					_KeepAliveTestStopSubtest( KeepAliveTestRef inTest );
static KeepAliveSubtest *	_KeepAliveTestGetSubtest( KeepAliveTestRef inTest );
static const char *			_KeepAliveTestGetSubtestLogPrefix( KeepAliveTestRef inTest, char *inBufPtr, size_t inBufLen );
static OSStatus				_KeepAliveTestContinue( KeepAliveTestRef inTest, OSStatus inSubtestError, Boolean *outDone );
static void					_KeepAliveTestTCPAcceptHandler( void *inCtx );
static void					_KeepAliveTestConnectionHandler( SocketRef inSock, OSStatus inError, void *inArg );
static void					_KeepAliveTestHandleConnection( KeepAliveTestRef inTest, SocketRef inSock, OSStatus inError );
static void					_KeepAliveTestForgetConnection( KeepAliveTestRef inTest );
static void DNSSD_API		_KeepAliveTestKeepaliveCallback( DNSServiceRef inSDRef, DNSServiceErrorType inErr, void *inCtx );
static void					_KeepAliveTestQueryTimerHandler( void *inCtx );
static void DNSSD_API
	_KeepAliveTestQueryRecordCallback(
		DNSServiceRef		inSDRef,
		DNSServiceFlags		inFlags,
		uint32_t			inInterfaceIndex,
		DNSServiceErrorType	inError,
		const char *		inFullName,
		uint16_t			inType,
		uint16_t			inClass,
		uint16_t			inRDataLen,
		const void *		inRDataPtr,
		uint32_t			inTTL,
		void *				inCtx );

static OSStatus	_KeepAliveTestCreate( KeepAliveTestRef *outTest )
{
	OSStatus				err;
	KeepAliveTestRef		test;
	size_t					i;
	
	test = (KeepAliveTestRef) calloc( 1, sizeof( *test ) );
	require_action( test, exit, err = kNoMemoryErr );
	
	test->error = kInProgressErr;
	for( i = 0; i < countof( test->subtests ); ++i )
	{
		KeepAliveSubtest * const		subtest = &test->subtests[ i ];
		
		subtest->local.sa.sa_family		= AF_UNSPEC;
		subtest->remote.sa.sa_family	= AF_UNSPEC;
		subtest->clientSock				= kInvalidSocketRef;
		subtest->serverSock				= kInvalidSocketRef;
	}
	test->queue = dispatch_queue_create( "com.apple.dnssdutil.keepalive-test", DISPATCH_QUEUE_SERIAL );
	require_action( test->queue, exit, err = kNoResourcesErr );
	
	test->doneSem = dispatch_semaphore_create( 0 );
	require_action( test->doneSem, exit, err = kNoResourcesErr );
	
	*outTest = test;
	test = NULL;
	err = kNoErr;
	
exit:
	if( test ) _KeepAliveTestFree( test );
	return( err );
}

//===========================================================================================================================

static OSStatus	_KeepAliveTestRun( KeepAliveTestRef inTest )
{
	dispatch_async_f( inTest->queue, inTest, _KeepAliveTestStart );
	dispatch_semaphore_wait( inTest->doneSem, DISPATCH_TIME_FOREVER );
	return( inTest->error );
}

//===========================================================================================================================

static void	_KeepAliveTestFree( KeepAliveTestRef inTest )
{
	size_t		i;
	
	check( !inTest->readSource );
	check( !inTest->query );
	check( !inTest->timer );
	check( !inTest->keepalive );
	check( !inTest->connection );
	check( !inTest->connectionCtx );
	dispatch_forget( &inTest->queue );
	dispatch_forget( &inTest->doneSem );
	for( i = 0; i < countof( inTest->subtests ); ++i )
	{
		KeepAliveSubtest * const		subtest = &inTest->subtests[ i ];
		
		check( !IsValidSocket( subtest->clientSock ) );
		check( !IsValidSocket( subtest->serverSock ) );
		ForgetMem( &subtest->recordName );
		ForgetMem( &subtest->dataStr );
	}
	free( inTest );
}

//===========================================================================================================================

static void _KeepAliveTestStart( void *inCtx )
{
	OSStatus					err;
	const KeepAliveTestRef		test = (KeepAliveTestRef) inCtx;
	
	test->error		= kInProgressErr;
	test->startTime	= NanoTimeGetCurrent();
	err = _KeepAliveTestStartSubtest( test );
	require_noerr( err, exit );
	
exit:
	if( err ) _KeepAliveTestStop( test, err );
}

//===========================================================================================================================

static void	_KeepAliveTestStop( KeepAliveTestRef inTest, OSStatus inError )
{
	size_t		i;
	
	inTest->error	= inError;
	inTest->endTime	= NanoTimeGetCurrent();
	_KeepAliveTestStopSubtest( inTest );
	for( i = 0; i < countof( inTest->subtests ); ++i )
	{
		KeepAliveSubtest * const		subtest = &inTest->subtests[ i ];
		
		ForgetSocket( &subtest->clientSock );
		ForgetSocket( &subtest->serverSock );
	}
	dispatch_semaphore_signal( inTest->doneSem );
}

//===========================================================================================================================

static OSStatus	_KeepAliveTestStartSubtest( KeepAliveTestRef inTest )
{
	OSStatus									err;
	KeepAliveSubtest * const					subtest		= _KeepAliveTestGetSubtest( inTest );
	const KeepAliveSubtestParams * const		params		= &kKeepAliveSubtestParams[ inTest->subtestIdx ];
	int											port;
	SocketRef									sock		= kInvalidSocketRef;
	const uint32_t								loopbackV4	= htonl( INADDR_LOOPBACK );
	SocketContext *								sockCtx		= NULL;
	KeepAliveTestConnectionContext *			cnxCtx		= NULL;
	Boolean										useIPv4;
	char										serverAddrStr[ 64 ];
	char										prefix[ 64 ];
	
	subtest->error			= kInProgressErr;
	subtest->startTime		= NanoTimeGetCurrent();
	subtest->description	= params->description;
	
	require_action( ( params->family == AF_INET ) || ( params->family == AF_INET6 ), exit, err = kInternalErr );
	
	// Create TCP listener socket.
	
	useIPv4 = ( params->family == AF_INET ) ? true : false;
	err = ServerSocketOpenEx( params->family, SOCK_STREAM, IPPROTO_TCP,
		useIPv4 ? ( (const void *) &loopbackV4 ) : ( (const void *) &in6addr_loopback ), kSocketPort_Auto, &port,
		kSocketBufferSize_DontSet, &sock );
	require_noerr( err, exit );
	
	if( useIPv4 )	SNPrintF( serverAddrStr, sizeof( serverAddrStr ), "%.4a:%d", &loopbackV4, port );
	else			SNPrintF( serverAddrStr, sizeof( serverAddrStr ), "[%.16a]:%d", in6addr_loopback.s6_addr, port );
	_KeepAliveTestGetSubtestLogPrefix( inTest, prefix, sizeof( prefix ) );
	kat_ulog( kLogLevelInfo, "%s: Will listen for connections on %s\n", prefix, serverAddrStr );
	
	err = SocketContextCreate( sock, inTest, &sockCtx );
	require_noerr( err, exit );
	sock = kInvalidSocketRef;
	
	// Create read source for TCP listener socket.
	
	check( !inTest->readSource );
	err = DispatchReadSourceCreate( sockCtx->sock, inTest->queue, _KeepAliveTestTCPAcceptHandler,
		SocketContextCancelHandler, sockCtx, &inTest->readSource );
	require_noerr( err, exit );
	sockCtx = NULL;
	dispatch_resume( inTest->readSource );
	
	cnxCtx = (KeepAliveTestConnectionContext *) calloc( 1, sizeof( *cnxCtx ) );
	require_action( cnxCtx, exit, err = kNoMemoryErr );
	
	// Start asynchronous connection to listener socket.
	
	kat_ulog( kLogLevelInfo, "%s: Will connect to %s\n", prefix, serverAddrStr );
	
	check( !inTest->connection );
	err = AsyncConnection_Connect( &inTest->connection, serverAddrStr, 0, kAsyncConnectionFlags_None,
		5 * UINT64_C_safe( kNanosecondsPerSecond ), kSocketBufferSize_DontSet, kSocketBufferSize_DontSet,
		NULL, NULL, _KeepAliveTestConnectionHandler, cnxCtx, inTest->queue );
	require_noerr( err, exit );
	
	cnxCtx->test = inTest;
	check( !inTest->connectionCtx );
	inTest->connectionCtx = cnxCtx;
	cnxCtx = NULL;
	
exit:
	ForgetSocket( &sock );
	if( sockCtx ) SocketContextRelease( sockCtx );
	FreeNullSafe( cnxCtx );
	return( err );
}

//===========================================================================================================================

static void	_KeepAliveTestStopSubtest( KeepAliveTestRef inTest )
{
	dispatch_source_forget( &inTest->readSource );
	DNSServiceForget( &inTest->keepalive );
	DNSServiceForget( &inTest->query );
	dispatch_source_forget( &inTest->timer );
	_KeepAliveTestForgetConnection( inTest );
}

//===========================================================================================================================

static KeepAliveSubtest *	_KeepAliveTestGetSubtest( KeepAliveTestRef inTest )
{
	return( ( inTest->subtestIdx < countof( inTest->subtests ) ) ? &inTest->subtests[ inTest->subtestIdx ] : NULL );
}

//===========================================================================================================================

static const char *	_KeepAliveTestGetSubtestLogPrefix( KeepAliveTestRef inTest, char *inBufPtr, size_t inBufLen )
{
	SNPrintF( inBufPtr, inBufLen, "Subtest %zu/%zu", inTest->subtestIdx + 1, countof( inTest->subtests ) );
	return( inBufPtr );
}

//===========================================================================================================================

static OSStatus	_KeepAliveTestContinue( KeepAliveTestRef inTest, OSStatus inSubtestError, Boolean *outDone )
{
	OSStatus				err;
	KeepAliveSubtest *		subtest;
	
	require_action( inTest->subtestIdx <= countof( inTest->subtests ), exit, err = kInternalErr );
	
	if( inTest->subtestIdx < countof( inTest->subtests ) )
	{
		subtest = _KeepAliveTestGetSubtest( inTest );
		_KeepAliveTestStopSubtest( inTest );
		subtest->endTime	= NanoTimeGetCurrent();
		subtest->error		= inSubtestError;
		if( ++inTest->subtestIdx < countof( inTest->subtests ) )
		{
			err = _KeepAliveTestStartSubtest( inTest );
			require_noerr_quiet( err, exit );
		}
	}
	err = kNoErr;
	
exit:
	if( outDone ) *outDone = ( !err && ( inTest->subtestIdx == countof( inTest->subtests ) ) ) ? true : false;
	return( err );
}

//===========================================================================================================================

static void	_KeepAliveTestTCPAcceptHandler( void *inCtx )
{
	OSStatus						err;
	const SocketContext * const		sockCtx	= (SocketContext *) inCtx;
	const KeepAliveTestRef			test	= (KeepAliveTestRef) sockCtx->userContext;
	KeepAliveSubtest * const		subtest	= _KeepAliveTestGetSubtest( test );
	sockaddr_ip						peer;
	socklen_t						len;
	char							prefix[ 64 ];
	
	check( !IsValidSocket( subtest->serverSock ) );
	len = (socklen_t) sizeof( peer );
	subtest->serverSock = accept( sockCtx->sock, &peer.sa, &len );
	err = map_socket_creation_errno( subtest->serverSock );
	require_noerr( err, exit );
	
	_KeepAliveTestGetSubtestLogPrefix( test, prefix, sizeof( prefix ) );
	kat_ulog( kLogLevelInfo, "%s: Accepted connection from %##a\n", prefix, &peer.sa );
	
	dispatch_source_forget( &test->readSource );
	
exit:
	if( err ) _KeepAliveTestStop( test, err );
}

//===========================================================================================================================

static void	_KeepAliveTestConnectionHandler( SocketRef inSock, OSStatus inError, void *inArg )
{
	KeepAliveTestConnectionContext *		ctx		= (KeepAliveTestConnectionContext *) inArg;
	const KeepAliveTestRef					test	= ctx->test;
	
	if( test )
	{
		_KeepAliveTestForgetConnection( test );
		_KeepAliveTestHandleConnection( test, inSock, inError );
		inSock = kInvalidSocketRef;
	}
	ForgetSocket( &inSock );
	free( ctx );
}

//===========================================================================================================================

#define kKeepAliveTestQueryTimeoutSecs		5

static void	_KeepAliveTestHandleConnection( KeepAliveTestRef inTest, SocketRef inSock, OSStatus inError )
{
	OSStatus								err;
	KeepAliveSubtest * const				subtest			= _KeepAliveTestGetSubtest( inTest );
	const KeepAliveSubtestParams * const	params			= &kKeepAliveSubtestParams[ inTest->subtestIdx ];
	socklen_t								len;
    uint32_t								value;
	int										family, i;
	Boolean									subtestFailed	= false;
	Boolean									done;
	char									prefix[ 64 ];
	
	require_noerr_action( inError, exit, err = inError );
	
	check( !IsValidSocket( subtest->clientSock ) );
	subtest->clientSock = inSock;
	inSock = kInvalidSocketRef;
	
	// Get local and remote IP addresses.
	
	len = (socklen_t) sizeof( subtest->local );
	err = getsockname( subtest->clientSock, &subtest->local.sa, &len );
	err = map_global_noerr_errno( err );
	require_noerr( err, exit );
	
	len = (socklen_t) sizeof( subtest->remote );
	err = getpeername( subtest->clientSock, &subtest->remote.sa, &len );
	err = map_global_noerr_errno( err );
	require_noerr( err, exit );
	
	_KeepAliveTestGetSubtestLogPrefix( inTest, prefix, sizeof( prefix ) );
	kat_ulog( kLogLevelInfo, "%s: Connection established: %##a <-> %##a\n",
		prefix, &subtest->local.sa, &subtest->remote.sa );
	
	// Call either DNSServiceSleepKeepalive() or DNSServiceSleepKeepalive_sockaddr().
	
	check( subtest->timeoutKA == 0 );
	subtest->timeoutKA = (unsigned int) RandomRange( 1, UINT_MAX );
	
    switch( params->callVariant )
	{
		case kKeepAliveCallVariant_TakesSocket:
			kat_ulog( kLogLevelInfo, "%s: Will call DNSServiceSleepKeepalive() for client-side socket\n", prefix );
			check( !inTest->keepalive );
			err = DNSServiceSleepKeepalive( &inTest->keepalive, 0, subtest->clientSock,
				subtest->timeoutKA, _KeepAliveTestKeepaliveCallback, inTest );
			require_noerr( err, exit );
			
			err = DNSServiceSetDispatchQueue( inTest->keepalive, inTest->queue );
			require_noerr( err, exit );
			break;
		
		case kKeepAliveCallVariant_TakesSockAddrs:
			kat_ulog( kLogLevelInfo,
				"%s: Will call DNSServiceSleepKeepalive_sockaddr() for local and remote sockaddrs\n", prefix );
			if( !SOFT_LINK_HAS_FUNCTION( system_dnssd, DNSServiceSleepKeepalive_sockaddr ) )
			{
				kat_ulog( kLogLevelError,
					"%s: Failed to soft link DNSServiceSleepKeepalive_sockaddr from libsystem_dnssd.\n", prefix );
				subtestFailed = true;
				err = kUnsupportedErr;
				goto exit;
			}
			check( !inTest->keepalive );
			err = soft_DNSServiceSleepKeepalive_sockaddr( &inTest->keepalive, 0, &subtest->local.sa, &subtest->remote.sa,
				subtest->timeoutKA, _KeepAliveTestKeepaliveCallback, inTest );
			require_noerr( err, exit );
			
			err = DNSServiceSetDispatchQueue( inTest->keepalive, inTest->queue );
			require_noerr( err, exit );
			break;
		
		default:
			kat_ulog( kLogLevelError, "%s: Invalid KeepAliveCallVariant value %d\n", prefix, (int) params->callVariant );
			err = kInternalErr;
			goto exit;
	}
	// Use the same logic that the DNSServiceSleepKeepalive functions use to derive a record name and rdata.
	
	value = 0;
	family = subtest->local.sa.sa_family;
	if( family == AF_INET )
	{
		const struct sockaddr_in * const		sin = &subtest->local.v4;
		const uint8_t *							ptr;
		
		check_compile_time_code( sizeof( sin->sin_addr.s_addr ) == 4 );
		ptr = (const uint8_t *) &sin->sin_addr.s_addr;
		for( i = 0; i < 4; ++i ) value += ptr[ i ];
		value += sin->sin_port;	// Note: No ntohl(). This is what DNSServiceSleepKeepalive does.
		
		check( subtest->remote.sa.sa_family == AF_INET );
		ASPrintF( &subtest->dataStr, "t=%u h=%.4a d=%.4a l=%u r=%u",
			subtest->timeoutKA, &subtest->local.v4.sin_addr.s_addr, &subtest->remote.v4.sin_addr.s_addr,
			ntohs( subtest->local.v4.sin_port ), ntohs( subtest->remote.v4.sin_port ) );
		require_action( subtest->dataStr, exit, err = kNoMemoryErr );
	}
	else if( family == AF_INET6 )
	{
		const struct sockaddr_in6 * const		sin6 = &subtest->local.v6;
		
		check_compile_time_code( countof( sin6->sin6_addr.s6_addr ) == 16 );
		for( i = 0; i < 16; ++i ) value += sin6->sin6_addr.s6_addr[ i ];
		value += sin6->sin6_port; // Note: No ntohl(). This is what DNSServiceSleepKeepalive does.
		
		check( subtest->remote.sa.sa_family == AF_INET6 );
		ASPrintF( &subtest->dataStr, "t=%u H=%.16a D=%.16a l=%u r=%u",
			subtest->timeoutKA, subtest->local.v6.sin6_addr.s6_addr, subtest->remote.v6.sin6_addr.s6_addr,
			ntohs( subtest->local.v6.sin6_port ), ntohs( subtest->remote.v6.sin6_port ) );
		require_action( subtest->dataStr, exit, err = kNoMemoryErr );
	}
	else
	{
		kat_ulog( kLogLevelError, "%s: Unexpected local address family %d\n", prefix, family );
		err = kInternalErr;
		goto exit;
	}
	
	// Start query for the new keepalive record.
	
	check( !subtest->recordName );
	ASPrintF( &subtest->recordName, "%u._keepalive._dns-sd._udp.local.", value );
	require_action( subtest->recordName, exit, err = kNoMemoryErr );
	
	kat_ulog( kLogLevelInfo, "%s: Will query for %s NULL record\n", prefix, subtest->recordName );
	check( !inTest->query );
	err = DNSServiceQueryRecord( &inTest->query, 0, kDNSServiceInterfaceIndexLocalOnly, subtest->recordName,
		kDNSServiceType_NULL, kDNSServiceClass_IN, _KeepAliveTestQueryRecordCallback, inTest );
	require_noerr( err, exit );
	
	err = DNSServiceSetDispatchQueue( inTest->query, inTest->queue );
	require_noerr( err, exit );
	
	// Start timer to enforce a time limit on the query.
	
	check( !inTest->timer );
	err = DispatchTimerOneShotCreate( dispatch_time_seconds( kKeepAliveTestQueryTimeoutSecs ),
		kKeepAliveTestQueryTimeoutSecs * ( INT64_C_safe( kNanosecondsPerSecond ) / 20 ), inTest->queue,
		_KeepAliveTestQueryTimerHandler, inTest, &inTest->timer );
	require_noerr( err, exit );
	dispatch_resume( inTest->timer );
	
exit:
	ForgetSocket( &inSock );
	if( subtestFailed )
	{
		err = _KeepAliveTestContinue( inTest, err, &done );
		check_noerr( err );
	}
	else
	{
		done = false;
	}
	if( err || done ) _KeepAliveTestStop( inTest, err );
}

//===========================================================================================================================

static void	_KeepAliveTestForgetConnection( KeepAliveTestRef inTest )
{
	if( inTest->connection )
	{
		check( inTest->connectionCtx );
		inTest->connectionCtx->test	= NULL; // Unset the connection's back pointer to test.
		inTest->connectionCtx		= NULL; // Context will be freed by the connection's handler.
		AsyncConnection_Forget( &inTest->connection );
	}
}

//===========================================================================================================================

static void DNSSD_API	_KeepAliveTestKeepaliveCallback( DNSServiceRef inSDRef, DNSServiceErrorType inError, void *inCtx )
{
	OSStatus					err;
	const KeepAliveTestRef		test	= (KeepAliveTestRef) inCtx;
	char						prefix[ 64 ];
	
	Unused( inSDRef );
	
	_KeepAliveTestGetSubtestLogPrefix( test, prefix, sizeof( prefix ) );
	kat_ulog( kLogLevelInfo, "%s: Keepalive callback error: %#m\n", prefix, inError );
	
	if( inError )
	{
		Boolean		done;
		
		err = _KeepAliveTestContinue( test, inError, &done );
		check_noerr( err );
		if( err || done ) _KeepAliveTestStop( test, err );
	}
}

//===========================================================================================================================

static void	_KeepAliveTestQueryTimerHandler( void *inCtx )
{
	OSStatus						err;
	const KeepAliveTestRef			test	= (KeepAliveTestRef) inCtx;
	KeepAliveSubtest * const		subtest	= _KeepAliveTestGetSubtest( test );
	Boolean							done;
	char							prefix[ 64 ];
	
	_KeepAliveTestGetSubtestLogPrefix( test, prefix, sizeof( prefix ) );
	kat_ulog( kLogLevelInfo, "%s: Query for \"%s\" timed out.\n", prefix, subtest->recordName );
	
	err = _KeepAliveTestContinue( test, kTimeoutErr, &done );
	check_noerr( err );
	if( err || done ) _KeepAliveTestStop( test, err );
}

//===========================================================================================================================

static void DNSSD_API
	_KeepAliveTestQueryRecordCallback(
		DNSServiceRef		inSDRef,
		DNSServiceFlags		inFlags,
		uint32_t			inInterfaceIndex,
		DNSServiceErrorType	inError,
		const char *		inFullName,
		uint16_t			inType,
		uint16_t			inClass,
		uint16_t			inRDataLen,
		const void *		inRDataPtr,
		uint32_t			inTTL,
		void *				inCtx )
{
	OSStatus						err;
	const KeepAliveTestRef			test	= (KeepAliveTestRef) inCtx;
	KeepAliveSubtest * const		subtest	= _KeepAliveTestGetSubtest( test );
	const uint8_t *					ptr;
	size_t							dataStrLen, minLen;
	Boolean							done;
	char							prefix[ 64 ];
	
	Unused( inSDRef );
	Unused( inInterfaceIndex );
	Unused( inTTL );
	
	_KeepAliveTestGetSubtestLogPrefix( test, prefix, sizeof( prefix ) );
	if( strcasecmp( inFullName, subtest->recordName ) != 0 )
	{
		kat_ulog( kLogLevelError, "%s: QueryRecord(%s) result: Got unexpected record name \"%s\".\n",
			prefix, subtest->recordName, inFullName );
		err = kUnexpectedErr;
		goto exit;
	}
	if( inType != kDNSServiceType_NULL )
	{
		kat_ulog( kLogLevelError, "%s: QueryRecord(%s) result: Got unexpected record type %d (%s) != %d (NULL).\n",
			prefix, subtest->recordName, inType, RecordTypeToString( inType ), kDNSServiceType_NULL );
		err = kUnexpectedErr;
		goto exit;
	}
	if( inClass != kDNSServiceClass_IN )
	{
		kat_ulog( kLogLevelError, "%s: QueryRecord(%s) result: Got unexpected record class %d != %d (IN).\n",
			prefix, subtest->recordName, inClass, kDNSServiceClass_IN );
		err = kUnexpectedErr;
		goto exit;
	}
	if( inError )
	{
		kat_ulog( kLogLevelError, "%s: QueryRecord(%s) result: Got unexpected error %#m.\n",
			prefix, subtest->recordName, inError );
		err = inError;
		goto exit;
	}
	if( ( inFlags & kDNSServiceFlagsAdd ) == 0 )
	{
		kat_ulog( kLogLevelError, "%s: QueryRecord(%s) result: Missing Add flag.\n", prefix, subtest->recordName );
		err = kUnexpectedErr;
		goto exit;
	}
	kat_ulog( kLogLevelInfo, "%s: QueryRecord(%s) result rdata: %#H\n",
		prefix, subtest->recordName, inRDataPtr, inRDataLen, inRDataLen );
	
	dataStrLen	= strlen( subtest->dataStr ) + 1;	// There's a NUL terminator at the end of the rdata.
	minLen		= 1 + dataStrLen;					// The first byte of the rdata is a length byte.
	if( inRDataLen < minLen )
	{
		kat_ulog( kLogLevelError, "%s: QueryRecord(%s) result: rdata length (%d) is less than expected minimum (%zu).\n",
			prefix, subtest->recordName, inRDataLen, minLen );
		err = kUnexpectedErr;
		goto exit;
	}
	ptr = (const uint8_t *) inRDataPtr;
	if( ptr[ 0 ] < dataStrLen )
	{
		kat_ulog( kLogLevelError,
			"%s: QueryRecord(%s) result: rdata length byte value (%d) is less than expected minimum (%zu).\n",
			prefix, subtest->recordName, ptr[ 0 ], dataStrLen );
		err = kUnexpectedErr;
		goto exit;
	}
	if( memcmp( &ptr[ 1 ], subtest->dataStr, dataStrLen - 1 ) != 0 )
	{
		kat_ulog( kLogLevelError, "%s: QueryRecord(%s) result: rdata body doesn't contain '%s'.\n",
			prefix, subtest->recordName, subtest->dataStr );
	}
	err = kNoErr;
	
exit:
	err = _KeepAliveTestContinue( test, err, &done );
	check_noerr( err );
	if( err || done ) _KeepAliveTestStop( test, kNoErr );
}
#endif	// TARGET_OS_DARWIN

//===========================================================================================================================
//	SSDPDiscoverCmd
//===========================================================================================================================

#define kSSDPPort		1900

typedef struct
{
	HTTPHeader				header;			// HTTP header object for sending and receiving.
	dispatch_source_t		readSourceV4;	// Read dispatch source for IPv4 socket.
	dispatch_source_t		readSourceV6;	// Read dispatch source for IPv6 socket.
	int						receiveSecs;	// After send, the amount of time to spend receiving.
	uint32_t				ifindex;		// Index of the interface over which to send the query.
	Boolean					useIPv4;		// True if the query should be sent via IPv4 multicast.
	Boolean					useIPv6;		// True if the query should be sent via IPv6 multicast.
	
}	SSDPDiscoverContext;

static void		SSDPDiscoverPrintPrologue( const SSDPDiscoverContext *inContext );
static void		SSDPDiscoverReadHandler( void *inContext );
static int		SocketToPortNumber( SocketRef inSock );
static OSStatus	WriteSSDPSearchRequest( HTTPHeader *inHeader, const void *inHostSA, int inMX, const char *inST );

static void	SSDPDiscoverCmd( void )
{
	OSStatus					err;
	struct timeval				now;
	SSDPDiscoverContext *		context;
	dispatch_source_t			signalSource	= NULL;
	SocketRef					sockV4			= kInvalidSocketRef;
	SocketRef					sockV6			= kInvalidSocketRef;
	ssize_t						n;
	int							sendCount;
	
	// Set up SIGINT handler.
	
	signal( SIGINT, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGINT, dispatch_get_main_queue(), Exit, kExitReason_SIGINT, &signalSource );
	require_noerr( err, exit );
	dispatch_resume( signalSource );
	
	// Check command parameters.
	
	if( gSSDPDiscover_ReceiveSecs < -1 )
	{
		FPrintF( stdout, "Invalid receive time: %d seconds.\n", gSSDPDiscover_ReceiveSecs );
		err = kParamErr;
		goto exit;
	}
	
	// Create context.
	
	context = (SSDPDiscoverContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	context->receiveSecs	= gSSDPDiscover_ReceiveSecs;
	context->useIPv4		= ( gSSDPDiscover_UseIPv4 || !gSSDPDiscover_UseIPv6 ) ? true : false;
	context->useIPv6		= ( gSSDPDiscover_UseIPv6 || !gSSDPDiscover_UseIPv4 ) ? true : false;
	
	err = InterfaceIndexFromArgString( gInterface, &context->ifindex );
	require_noerr_quiet( err, exit );
	
	// Set up IPv4 socket.
	
	if( context->useIPv4 )
	{
		int port;
		err = UDPClientSocketOpen( AF_INET, NULL, 0, -1, &port, &sockV4 );
		require_noerr( err, exit );
		
		err = SocketSetMulticastInterface( sockV4, NULL, context->ifindex );
		require_noerr( err, exit );
		
		err = setsockopt( sockV4, IPPROTO_IP, IP_MULTICAST_LOOP, (char *) &(uint8_t){ 1 }, (socklen_t) sizeof( uint8_t ) );
		err = map_socket_noerr_errno( sockV4, err );
		require_noerr( err, exit );
	}
	
	// Set up IPv6 socket.
	
	if( context->useIPv6 )
	{
		err = UDPClientSocketOpen( AF_INET6, NULL, 0, -1, NULL, &sockV6 );
		require_noerr( err, exit );
		
		err = SocketSetMulticastInterface( sockV6, NULL, context->ifindex );
		require_noerr( err, exit );
		
		err = setsockopt( sockV6, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, (char *) &(int){ 1 }, (socklen_t) sizeof( int ) );
		err = map_socket_noerr_errno( sockV6, err );
		require_noerr( err, exit );
	}
	
	// Print prologue.
	
	SSDPDiscoverPrintPrologue( context );
	
	// Send mDNS query message.
	
	sendCount = 0;
	if( IsValidSocket( sockV4 ) )
	{
		struct sockaddr_in		mcastAddr4;
		
		memset( &mcastAddr4, 0, sizeof( mcastAddr4 ) );
		SIN_LEN_SET( &mcastAddr4 );
		mcastAddr4.sin_family		= AF_INET;
		mcastAddr4.sin_port			= htons( kSSDPPort );
		mcastAddr4.sin_addr.s_addr	= htonl( 0xEFFFFFFA );	// 239.255.255.250
		
		err = WriteSSDPSearchRequest( &context->header, &mcastAddr4, gSSDPDiscover_MX, gSSDPDiscover_ST );
		require_noerr( err, exit );
		
		n = sendto( sockV4, context->header.buf, context->header.len, 0, (const struct sockaddr *) &mcastAddr4,
			(socklen_t) sizeof( mcastAddr4 ) );
		err = map_socket_value_errno( sockV4, n == (ssize_t) context->header.len, n );
		if( err )
		{
			FPrintF( stderr, "*** Failed to send query on IPv4 socket with error %#m\n", err );
			ForgetSocket( &sockV4 );
		}
		else
		{
			if( gSSDPDiscover_Verbose )
			{
				gettimeofday( &now, NULL );
				FPrintF( stdout, "---\n" );
				FPrintF( stdout, "Send time:    %{du:time}\n",	&now );
				FPrintF( stdout, "Source Port:  %d\n",			SocketToPortNumber( sockV4 ) );
				FPrintF( stdout, "Destination:  %##a\n",		&mcastAddr4 );
				FPrintF( stdout, "Message size: %zu\n",			context->header.len );
				FPrintF( stdout, "HTTP header:\n%1{text}",		context->header.buf, context->header.len );
			}
			++sendCount;
		}
	}
	
	if( IsValidSocket( sockV6 ) )
	{
		struct sockaddr_in6		mcastAddr6;
		
		memset( &mcastAddr6, 0, sizeof( mcastAddr6 ) );
		SIN6_LEN_SET( &mcastAddr6 );
		mcastAddr6.sin6_family				= AF_INET6;
		mcastAddr6.sin6_port				= htons( kSSDPPort );
		mcastAddr6.sin6_addr.s6_addr[  0 ]	= 0xFF;	// SSDP IPv6 link-local multicast address FF02::C
		mcastAddr6.sin6_addr.s6_addr[  1 ]	= 0x02;
		mcastAddr6.sin6_addr.s6_addr[ 15 ]	= 0x0C;
		
		err = WriteSSDPSearchRequest( &context->header, &mcastAddr6, gSSDPDiscover_MX, gSSDPDiscover_ST );
		require_noerr( err, exit );
		
		n = sendto( sockV6, context->header.buf, context->header.len, 0, (const struct sockaddr *) &mcastAddr6,
			(socklen_t) sizeof( mcastAddr6 ) );
		err = map_socket_value_errno( sockV6, n == (ssize_t) context->header.len, n );
		if( err )
		{
			FPrintF( stderr, "*** Failed to send query on IPv6 socket with error %#m\n", err );
			ForgetSocket( &sockV6 );
		}
		else
		{
			if( gSSDPDiscover_Verbose )
			{
				gettimeofday( &now, NULL );
				FPrintF( stdout, "---\n" );
				FPrintF( stdout, "Send time:    %{du:time}\n",	&now );
				FPrintF( stdout, "Source Port:  %d\n",			SocketToPortNumber( sockV6 ) );
				FPrintF( stdout, "Destination:  %##a\n",		&mcastAddr6 );
				FPrintF( stdout, "Message size: %zu\n",			context->header.len );
				FPrintF( stdout, "HTTP header:\n%1{text}",		context->header.buf, context->header.len );
			}
			++sendCount;
		}
	}
	require_action_quiet( sendCount > 0, exit, err = kUnexpectedErr );
	
	// If there's no wait period after the send, then exit.
	
	if( context->receiveSecs == 0 ) goto exit;
	
	// Create dispatch read sources for socket(s).
	
	if( IsValidSocket( sockV4 ) )
	{
		SocketContext *		sockCtx;
		
		err = SocketContextCreate( sockV4, context, &sockCtx );
		require_noerr( err, exit );
		sockV4 = kInvalidSocketRef;
		
		err = DispatchReadSourceCreate( sockCtx->sock, NULL, SSDPDiscoverReadHandler, SocketContextCancelHandler, sockCtx,
			&context->readSourceV4 );
		if( err ) ForgetSocketContext( &sockCtx );
		require_noerr( err, exit );
		
		dispatch_resume( context->readSourceV4 );
	}
	
	if( IsValidSocket( sockV6 ) )
	{
		SocketContext *		sockCtx;
		
		err = SocketContextCreate( sockV6, context, &sockCtx );
		require_noerr( err, exit );
		sockV6 = kInvalidSocketRef;
		
		err = DispatchReadSourceCreate( sockCtx->sock, NULL, SSDPDiscoverReadHandler, SocketContextCancelHandler, sockCtx,
			&context->readSourceV6 );
		if( err ) ForgetSocketContext( &sockCtx );
		require_noerr( err, exit );
		
		dispatch_resume( context->readSourceV6 );
	}
	
	if( context->receiveSecs > 0 )
	{
		dispatch_after_f( dispatch_time_seconds( context->receiveSecs ), dispatch_get_main_queue(), kExitReason_Timeout,
			Exit );
	}
	dispatch_main();
	
exit:
	ForgetSocket( &sockV4 );
	ForgetSocket( &sockV6 );
	dispatch_source_forget( &signalSource );
	exit( err ? 1 : 0 );
}

static int	SocketToPortNumber( SocketRef inSock )
{
	OSStatus		err;
	sockaddr_ip		sip;
	socklen_t		len;
	
	len = (socklen_t) sizeof( sip );
	err = getsockname( inSock, &sip.sa, &len );
	err = map_socket_noerr_errno( inSock, err );
	check_noerr( err );
	return( err ? -1 : SockAddrGetPort( &sip ) );
}

static OSStatus	WriteSSDPSearchRequest( HTTPHeader *inHeader, const void *inHostSA, int inMX, const char *inST )
{
	OSStatus		err;
	
	err = HTTPHeader_InitRequest( inHeader, "M-SEARCH", "*", "HTTP/1.1" );
	require_noerr( err, exit );
	
	err = HTTPHeader_SetField( inHeader, "Host", "%##a", inHostSA );
	require_noerr( err, exit );
	
	err = HTTPHeader_SetField( inHeader, "ST", "%s", inST ? inST : "ssdp:all" );
	require_noerr( err, exit );
	
	err = HTTPHeader_SetField( inHeader, "Man", "\"ssdp:discover\"" );
	require_noerr( err, exit );
	
	err = HTTPHeader_SetField( inHeader, "MX", "%d", inMX );
	require_noerr( err, exit );
	
	err = HTTPHeader_Commit( inHeader );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	SSDPDiscoverPrintPrologue
//===========================================================================================================================

static void	SSDPDiscoverPrintPrologue( const SSDPDiscoverContext *inContext )
{
	const int				receiveSecs = inContext->receiveSecs;
	const char *			ifName;
	char					ifNameBuf[ IF_NAMESIZE + 1 ];
	NetTransportType		ifType;
	
	ifName = if_indextoname( inContext->ifindex, ifNameBuf );
	
	ifType = kNetTransportType_Undefined;
	if( ifName ) SocketGetInterfaceInfo( kInvalidSocketRef, ifName, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &ifType );
	
	FPrintF( stdout, "Interface:        %s/%d/%s\n",
		ifName ? ifName : "?", inContext->ifindex, NetTransportTypeToString( ifType ) );
	FPrintF( stdout, "IP protocols:     %?s%?s%?s\n",
		inContext->useIPv4, "IPv4", ( inContext->useIPv4 && inContext->useIPv6 ), ", ", inContext->useIPv6, "IPv6" );
	FPrintF( stdout, "Receive duration: " );
	if( receiveSecs >= 0 )	FPrintF( stdout, "%d second%?c\n", receiveSecs, receiveSecs != 1, 's' );
	else					FPrintF( stdout, "∞\n" );
	FPrintF( stdout, "Start time:       %{du:time}\n", NULL );
}

//===========================================================================================================================
//	SSDPDiscoverReadHandler
//===========================================================================================================================

static Boolean	_HTTPHeader_Validate( HTTPHeader *inHeader );

static void	SSDPDiscoverReadHandler( void *inContext )
{
	OSStatus						err;
	struct timeval					now;
	SocketContext * const			sockCtx	= (SocketContext *) inContext;
	SSDPDiscoverContext * const		context	= (SSDPDiscoverContext *) sockCtx->userContext;
	HTTPHeader * const				header	= &context->header;
	sockaddr_ip						fromAddr;
	size_t							msgLen;
	
	gettimeofday( &now, NULL );
	
	err = SocketRecvFrom( sockCtx->sock, header->buf, sizeof( header->buf ), &msgLen, &fromAddr, sizeof( fromAddr ),
		NULL, NULL, NULL, NULL );
	require_noerr( err, exit );
	
	FPrintF( stdout, "---\n" );
	FPrintF( stdout, "Receive time: %{du:time}\n",	&now );
	FPrintF( stdout, "Source:       %##a\n", 		&fromAddr );
	FPrintF( stdout, "Message size: %zu\n",			msgLen );
	header->len = msgLen;
	if( _HTTPHeader_Validate( header ) )
	{
		FPrintF( stdout, "HTTP header:\n%1{text}", header->buf, header->len );
		if( header->extraDataLen > 0 )
		{
			FPrintF( stdout, "HTTP body: %1.1H", header->extraDataPtr, (int) header->extraDataLen, INT_MAX );
		}
	}
	else
	{
		FPrintF( stdout, "Invalid HTTP message:\n%1.1H", header->buf, (int) msgLen, INT_MAX );
		goto exit;
	}
	
exit:
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	_HTTPHeader_Validate
//
//	Parses for the end of an HTTP header and updates the HTTPHeader structure so it's ready to parse. Returns true if valid.
//	This assumes the "buf" and "len" fields are set. The other fields are set by this function.
//
//	Note: This was copied from CoreUtils because the HTTPHeader_Validate function is currently not exported in the framework.
//===========================================================================================================================

static Boolean	_HTTPHeader_Validate( HTTPHeader *inHeader )
{
	const char *		src;
	const char *		end;
	
	// Check for interleaved binary data (4 byte header that begins with $). See RFC 2326 section 10.12.
	
	require( inHeader->len < sizeof( inHeader->buf ), exit );
	src = inHeader->buf;
	end = src + inHeader->len;
	if( ( ( end - src ) >= 4 ) && ( src[ 0 ] == '$' ) )
	{
		src += 4;
	}
	else
	{
		// Search for an empty line (HTTP-style header/body separator). CRLFCRLF, LFCRLF, or LFLF accepted.
		// $$$ TO DO: Start from the last search location to avoid re-searching the same data over and over.
		
		for( ;; )
		{
			while( ( src < end ) && ( src[ 0 ] != '\n' ) ) ++src;
			if( src >= end ) goto exit;
			++src;
			if( ( ( end - src ) >= 2 ) && ( src[ 0 ] == '\r' ) && ( src[ 1 ] == '\n' ) ) // CFLFCRLF or LFCRLF
			{
				src += 2;
				break;
			}
			else if( ( ( end - src ) >= 1 ) && ( src[ 0 ] == '\n' ) ) // LFLF
			{
				src += 1;
				break;
			}
		}
	}
	inHeader->extraDataPtr	= src;
	inHeader->extraDataLen	= (size_t)( end - src );
	inHeader->len			= (size_t)( src - inHeader->buf );
	return( true );
	
exit:
	return( false );
}

#if( TARGET_OS_DARWIN )
//===========================================================================================================================
//	ResQueryCmd
//===========================================================================================================================

// res_query() from libresolv is actually called res_9_query (see /usr/include/resolv.h).

SOFT_LINK_LIBRARY_EX( "/usr/lib", resolv );
SOFT_LINK_FUNCTION_EX( resolv, res_9_query,
	int,
	( const char *dname, int class, int type, u_char *answer, int anslen ),
	( dname, class, type, answer, anslen ) );

// res_query() from libinfo

SOFT_LINK_LIBRARY_EX( "/usr/lib", info );
SOFT_LINK_FUNCTION_EX( info, res_query,
	int,
	( const char *dname, int class, int type, u_char *answer, int anslen ),
	( dname, class, type, answer, anslen ) );

typedef int ( *res_query_f )( const char *dname, int class, int type, u_char *answer, int anslen );

static void	ResQueryCmd( void )
{
	OSStatus		err;
	res_query_f		res_query_ptr;
	int				n;
	uint16_t		type, class;
	uint8_t			answer[ 1024 ];
	
	// Get pointer to one of the res_query() functions.
	
	if( gResQuery_UseLibInfo )
	{
		if( !SOFT_LINK_HAS_FUNCTION( info, res_query ) )
		{
			FPrintF( stderr, "Failed to soft link res_query from libinfo.\n" );
			err = kNotFoundErr;
			goto exit;
		}
		res_query_ptr = soft_res_query;
	}
	else
	{
		if( !SOFT_LINK_HAS_FUNCTION( resolv, res_9_query ) )
		{
			FPrintF( stderr, "Failed to soft link res_query from libresolv.\n" );
			err = kNotFoundErr;
			goto exit;
		}
		res_query_ptr = soft_res_9_query;
	}
	
	// Get record type.
	
	err = RecordTypeFromArgString( gResQuery_Type, &type );
	require_noerr( err, exit );
	
	// Get record class.
	
	if( gResQuery_Class )
	{
		err = RecordClassFromArgString( gResQuery_Class, &class );
		require_noerr( err, exit );
	}
	else
	{
		class = kDNSServiceClass_IN;
	}
	
	// Print prologue.
	
	FPrintF( stdout, "Name:       %s\n",			gResQuery_Name );
	FPrintF( stdout, "Type:       %s (%u)\n",		RecordTypeToString( type ), type );
	FPrintF( stdout, "Class:      %s (%u)\n",		( class == kDNSServiceClass_IN ) ? "IN" : "???", class );
	FPrintF( stdout, "Start time: %{du:time}\n",	NULL );
	FPrintF( stdout, "---\n" );
	
	// Call res_query().
	
	n = res_query_ptr( gResQuery_Name, class, type, (u_char *) answer, (int) sizeof( answer ) );
	if( n < 0 )
	{
		FPrintF( stderr, "res_query() failed with error: %d (%s).\n", h_errno, hstrerror( h_errno ) );
		err = kUnknownErr;
		goto exit;
	}
	
	// Print result.
	
	FPrintF( stdout, "Message size: %d\n\n%{du:dnsmsg}", n, answer, (size_t) n );
	
exit:
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	ResolvDNSQueryCmd
//===========================================================================================================================

// dns_handle_t is defined as a pointer to a privately-defined struct in /usr/include/dns.h. It's defined as a void * here to
// avoid including the header file.

typedef void *		dns_handle_t;

SOFT_LINK_FUNCTION_EX( resolv, dns_open, dns_handle_t, ( const char *path ), ( path ) );
SOFT_LINK_FUNCTION_VOID_RETURN_EX( resolv, dns_free, ( dns_handle_t *dns ), ( dns ) );
SOFT_LINK_FUNCTION_EX( resolv, dns_query,
	int32_t, (
		dns_handle_t		dns,
		const char *		name,
		uint32_t			dnsclass,
		uint32_t			dnstype,
		char *				buf,
		uint32_t			len,
		struct sockaddr *	from,
		uint32_t *			fromlen ),
	( dns, name, dnsclass, dnstype, buf, len, from, fromlen ) );

static void	ResolvDNSQueryCmd( void )
{
	OSStatus			err;
	int					n;
	dns_handle_t		dns = NULL;
	uint16_t			type, class;
	sockaddr_ip			from;
	uint32_t			fromLen;
	uint8_t				answer[ 1024 ];
	
	// Make sure that the required symbols are available.
	
	if( !SOFT_LINK_HAS_FUNCTION( resolv, dns_open ) )
	{
		FPrintF( stderr, "Failed to soft link dns_open from libresolv.\n" );
		err = kNotFoundErr;
		goto exit;
	}
	
	if( !SOFT_LINK_HAS_FUNCTION( resolv, dns_free ) )
	{
		FPrintF( stderr, "Failed to soft link dns_free from libresolv.\n" );
		err = kNotFoundErr;
		goto exit;
	}
	
	if( !SOFT_LINK_HAS_FUNCTION( resolv, dns_query ) )
	{
		FPrintF( stderr, "Failed to soft link dns_query from libresolv.\n" );
		err = kNotFoundErr;
		goto exit;
	}
	
	// Get record type.
	
	err = RecordTypeFromArgString( gResolvDNSQuery_Type, &type );
	require_noerr( err, exit );
	
	// Get record class.
	
	if( gResolvDNSQuery_Class )
	{
		err = RecordClassFromArgString( gResolvDNSQuery_Class, &class );
		require_noerr( err, exit );
	}
	else
	{
		class = kDNSServiceClass_IN;
	}
	
	// Get dns handle.
	
	dns = soft_dns_open( gResolvDNSQuery_Path );
	if( !dns )
	{
		FPrintF( stderr, "dns_open( %s ) failed.\n", gResolvDNSQuery_Path );
		err = kUnknownErr;
		goto exit;
	}
	
	// Print prologue.
	
	FPrintF( stdout, "Name:       %s\n",			gResolvDNSQuery_Name );
	FPrintF( stdout, "Type:       %s (%u)\n",		RecordTypeToString( type ), type );
	FPrintF( stdout, "Class:      %s (%u)\n",		( class == kDNSServiceClass_IN ) ? "IN" : "???", class );
	FPrintF( stdout, "Path:       %s\n",			gResolvDNSQuery_Path ? gResolvDNSQuery_Name : "<NULL>" );
	FPrintF( stdout, "Start time: %{du:time}\n",	NULL );
	FPrintF( stdout, "---\n" );
	
	// Call dns_query().
	
	memset( &from, 0, sizeof( from ) );
	fromLen = (uint32_t) sizeof( from );
	n = soft_dns_query( dns, gResolvDNSQuery_Name, class, type, (char *) answer, (uint32_t) sizeof( answer ), &from.sa,
		&fromLen );
	if( n < 0 )
	{
		FPrintF( stderr, "dns_query() failed with error: %d (%s).\n", h_errno, hstrerror( h_errno ) );
		err = kUnknownErr;
		goto exit;
	}
	
	// Print result.
	
	FPrintF( stdout, "From:         %##a\n", &from );
	FPrintF( stdout, "Message size: %d\n\n%{du:dnsmsg}", n, answer, (size_t) n );
	
exit:
	if( dns ) soft_dns_free( dns );
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	CFHostCmd
//===========================================================================================================================

static void
	_CFHostResolveCallback(
		CFHostRef				inHost,
		CFHostInfoType			inInfoType,
		const CFStreamError *	inError,
		void *					inInfo );

static void	CFHostCmd( void )
{
	OSStatus				err;
	CFStringRef				name;
	Boolean					success;
	CFHostRef				host = NULL;
	CFHostClientContext		context;
	CFStreamError			streamErr;
	
	name = CFStringCreateWithCString( kCFAllocatorDefault, gCFHost_Name, kCFStringEncodingUTF8 );
	require_action( name, exit, err = kUnknownErr );
	
	host = CFHostCreateWithName( kCFAllocatorDefault, name );
	ForgetCF( &name );
	require_action( host, exit, err = kUnknownErr );
	
	memset( &context, 0, sizeof( context ) );
	success = CFHostSetClient( host, _CFHostResolveCallback, &context );
	require_action( success, exit, err = kUnknownErr );
	
	CFHostScheduleWithRunLoop( host, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode );
	
	// Print prologue.
	
	FPrintF( stdout, "Hostname:   %s\n",			gCFHost_Name );
	FPrintF( stdout, "Start time: %{du:time}\n",	NULL );
	FPrintF( stdout, "---\n" );
	
	success = CFHostStartInfoResolution( host, kCFHostAddresses, &streamErr );
	require_action( success, exit, err = kUnknownErr );
	err = kNoErr;
	
	CFRunLoopRun();
	
exit:
	CFReleaseNullSafe( host );
	if( err ) exit( 1 );
}

static void	_CFHostResolveCallback( CFHostRef inHost, CFHostInfoType inInfoType, const CFStreamError *inError, void *inInfo )
{
	OSStatus			err;
	struct timeval		now;
	
	gettimeofday( &now, NULL );
	
	Unused( inInfoType );
	Unused( inInfo );
	
	if( inError && ( inError->domain != 0 ) && ( inError->error ) )
	{
		err = inError->error;
		if( inError->domain == kCFStreamErrorDomainNetDB )
		{
			FPrintF( stderr, "Error %d: %s.\n", err, gai_strerror( err ) );
		}
		else
		{
			FPrintF( stderr, "Error %#m\n", err );
		}
	}
	else
	{
		CFArrayRef					addresses;
		CFIndex						count, i;
		CFDataRef					addrData;
		const struct sockaddr *		sockAddr;
		Boolean						wasResolved = false;
		
		addresses = CFHostGetAddressing( inHost, &wasResolved );
		check( wasResolved );
		
		if( addresses )
		{
			count = CFArrayGetCount( addresses );
			for( i = 0; i < count; ++i )
			{
				addrData = CFArrayGetCFDataAtIndex( addresses, i, &err );
				require_noerr( err, exit );
				
				sockAddr = (const struct sockaddr *) CFDataGetBytePtr( addrData );
				FPrintF( stdout, "%##a\n", sockAddr );
			}
		}
		err = kNoErr;
	}
	
	FPrintF( stdout, "---\n" );
	FPrintF( stdout, "End time:   %{du:time}\n", &now );
	
	if( gCFHost_WaitSecs > 0 ) sleep( (unsigned int) gCFHost_WaitSecs );
	
exit:
	exit( err ? 1 : 0 );
}

//===========================================================================================================================
//	DNSConfigAddCmd
//
//	Note: Based on ajn's supplemental test tool.
//===========================================================================================================================

static void	DNSConfigAddCmd( void )
{
	OSStatus					err;
	CFMutableDictionaryRef		dict	= NULL;
	CFMutableArrayRef			array	= NULL;
	size_t						i;
	SCDynamicStoreRef			store	= NULL;
	CFStringRef					key		= NULL;
	Boolean						success;
	
	// Create dictionary.
	
	dict = CFDictionaryCreateMutable( NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
	require_action( dict, exit, err = kNoMemoryErr );
	
	// Add DNS server IP addresses.
	
	array = CFArrayCreateMutable( NULL, (CFIndex) gDNSConfigAdd_IPAddrCount, &kCFTypeArrayCallBacks );
	require_action( array, exit, err = kNoMemoryErr );
	
	for( i = 0; i < gDNSConfigAdd_IPAddrCount; ++i )
	{
		CFStringRef		addrStr;
		
		addrStr = CFStringCreateWithCString( NULL, gDNSConfigAdd_IPAddrArray[ i ], kCFStringEncodingUTF8 );
		require_action( addrStr, exit, err = kUnknownErr );
		
		CFArrayAppendValue( array, addrStr );
		CFRelease( addrStr );
	}
	
	CFDictionarySetValue( dict, kSCPropNetDNSServerAddresses, array );
	ForgetCF( &array );
	
	// Add domains, if any.
	
	array = CFArrayCreateMutable( NULL, (CFIndex) Min( gDNSConfigAdd_DomainCount, 1 ), &kCFTypeArrayCallBacks );
	require_action( array, exit, err = kNoMemoryErr );
	
	if( gDNSConfigAdd_DomainCount > 0 )
	{
		for( i = 0; i < gDNSConfigAdd_DomainCount; ++i )
		{
			CFStringRef		domainStr;
			
			domainStr = CFStringCreateWithCString( NULL, gDNSConfigAdd_DomainArray[ i ], kCFStringEncodingUTF8 );
			require_action( domainStr, exit, err = kUnknownErr );
			
			CFArrayAppendValue( array, domainStr );
			CFRelease( domainStr );
		}
	}
	else
	{
		// There are no domains, but the domain array needs to be non-empty, so add a zero-length string to the array.
		
		CFArrayAppendValue( array, CFSTR( "" ) );
	}
	
	CFDictionarySetValue( dict, kSCPropNetDNSSupplementalMatchDomains, array );
	ForgetCF( &array );
	
	// Add interface, if any.
	
	if( gDNSConfigAdd_Interface )
	{
		err = CFDictionarySetCString( dict, kSCPropInterfaceName, gDNSConfigAdd_Interface, kSizeCString );
		require_noerr( err, exit );
		
		CFDictionarySetValue( dict, kSCPropNetDNSConfirmedServiceID, gDNSConfigAdd_ID );
	}
	
	// Set dictionary in dynamic store.
	
	store = SCDynamicStoreCreate( NULL, CFSTR( kDNSSDUtilIdentifier ), NULL, NULL );
	err = map_scerror( store );
	require_noerr( err, exit );
	
	key = SCDynamicStoreKeyCreateNetworkServiceEntity( NULL, kSCDynamicStoreDomainState, gDNSConfigAdd_ID, kSCEntNetDNS );
	require_action( key, exit, err = kUnknownErr );
	
	success = SCDynamicStoreSetValue( store, key, dict );
	require_action( success, exit, err = kUnknownErr );
	
exit:
	CFReleaseNullSafe( dict );
	CFReleaseNullSafe( array );
	CFReleaseNullSafe( store );
	CFReleaseNullSafe( key );
	gExitCode = err ? 1 : 0;
}

//===========================================================================================================================
//	DNSConfigRemoveCmd
//===========================================================================================================================

static void	DNSConfigRemoveCmd( void )
{
	OSStatus				err;
	SCDynamicStoreRef		store	= NULL;
	CFStringRef				key		= NULL;
	Boolean					success;
	
	store = SCDynamicStoreCreate( NULL, CFSTR( kDNSSDUtilIdentifier ), NULL, NULL );
	err = map_scerror( store );
	require_noerr( err, exit );
	
	key = SCDynamicStoreKeyCreateNetworkServiceEntity( NULL, kSCDynamicStoreDomainState, gDNSConfigRemove_ID, kSCEntNetDNS );
	require_action( key, exit, err = kUnknownErr );
	
	success = SCDynamicStoreRemoveValue( store, key );
	require_action( success, exit, err = kUnknownErr );
	
exit:
	CFReleaseNullSafe( store );
	CFReleaseNullSafe( key );
	gExitCode = err ? 1 : 0;
}

//===========================================================================================================================
//	XPCSendCmd
//===========================================================================================================================

static OSStatus	_XPCDictionaryCreateFromString( const char *inString, xpc_object_t *outDict );

static void	XPCSendCmd( void )
{
	OSStatus			err;
	xpc_object_t		msg, reply;
	
	err = _XPCDictionaryCreateFromString( gXPCSend_MessageStr, &msg );
	require_noerr_quiet( err, exit );
	
	FPrintF( stdout, "Service:    %s\n", gXPCSend_ServiceName );
	FPrintF( stdout, "Message:    %s\n", gXPCSend_MessageStr );
	FPrintF( stdout, "Start time: %{du:time}\n", NULL );
	FPrintF( stdout, "---\n" );
	FPrintF( stdout, "XPC Message:\n%{xpc}\n", msg );
	
	err = xpc_send_message_sync( gXPCSend_ServiceName, 0, 0, msg, &reply );
	xpc_forget( &msg );
	require_noerr_quiet( err, exit );
	
	FPrintF( stdout, "XPC Reply:\n%{xpc}\n", reply );
	FPrintF( stdout, "---\n" );
	FPrintF( stdout, "End time:   %{du:time}\n", NULL );
	xpc_forget( &reply );
	
exit:
	if( err ) ErrQuit( 1, "error: %#m\n", err );
}

//===========================================================================================================================
//	_XPCDictionaryCreateFromString
//===========================================================================================================================

#define kXPCObjectPrefix_Bool		"bool:"
#define kXPCObjectPrefix_Data		"data:"
#define kXPCObjectPrefix_Int64		"int:"
#define kXPCObjectPrefix_String		"string:"
#define kXPCObjectPrefix_UInt64		"uint:"
#define kXPCObjectPrefix_UUID		"uuid:"

typedef struct XPCListItem		XPCListItem;
struct XPCListItem
{
	XPCListItem *		next;
	xpc_object_t		obj;
	char *				key;
};

static OSStatus	_XPCListItemCreate( xpc_object_t inObject, const char *inKey, XPCListItem **outItem );
static void		_XPCListItemFree( XPCListItem *inItem );
static void		_XPCListFree( XPCListItem *inList );

static OSStatus	_XPCObjectFromString( const char *inString, xpc_object_t *outObject );

static OSStatus	_XPCDictionaryCreateFromString( const char *inString, xpc_object_t *outDict )
{
	OSStatus				err;
	xpc_object_t			container;
	const char *			ptr		= inString;
	const char * const		end		= inString + strlen( inString );
	XPCListItem *			list	= NULL;
	
	container = xpc_dictionary_create( NULL, NULL, 0 );
	require_action( container, exit, err = kNoMemoryErr );
	
	while( *ptr )
	{
		xpc_type_t				containerType;
		xpc_object_t			value;
		int						c;
		char					keyStr[ 256 ];
		char					valStr[ 256 ];
		
		// At this point, zero or more of the current container's elements have been parsed.
		// Skip the white space leading up to the container's next element, if any, or the container's end.
		
		while( isspace_safe( *ptr ) ) ++ptr;
		
		// Check if we're done with the current container.
		
		c = *ptr;
		if( c == '\0' ) break;
		
		containerType = xpc_get_type( container );
		if( ( ( containerType == XPC_TYPE_DICTIONARY ) && ( c == '}' ) ) ||
			( ( containerType == XPC_TYPE_ARRAY )      && ( c == ']' ) ) )
		{
			XPCListItem *		item;
			
			item = list;
			require_action_quiet( item, exit, err = kMalformedErr );
			
			++ptr;
			
			// Add the current container to its parent container.
			
			if( item->key )
			{
				xpc_dictionary_set_value( item->obj, item->key, container );
			}
			else
			{
				xpc_array_append_value( item->obj, container );
			}
			
			// Continue with the parent container.
			
			xpc_release( container );
			container = xpc_retain( item->obj );
			list = item->next;
			_XPCListItemFree( item );
			continue;
		}
		
		// If the current container is a dictionary, parse the key string.
		
		if( containerType == XPC_TYPE_DICTIONARY )
		{
			err = _ParseEscapedString( ptr, end, "={}[]" kWhiteSpaceCharSet, keyStr, sizeof( keyStr ), NULL, NULL, &ptr );
			require_noerr_quiet( err, exit );
			
			c = *ptr;
			require_action_quiet( c == '=', exit, err = kMalformedErr );
			++ptr;
		}
		
		// Check if the value is a dictionary ({...}) or an array ([...]).
		
		c = *ptr;
		if( ( c == '{' ) || ( c == '[' ) )
		{
			XPCListItem *		item;
			
			++ptr;
			
			// Save the current container.
			
			err = _XPCListItemCreate( container, ( containerType == XPC_TYPE_DICTIONARY ) ? keyStr : NULL, &item );
			require_noerr( err, exit );
			
			item->next = list;
			list = item;
			item = NULL;
			
			// Create and continue with the child container.
			
			xpc_release( container );
			if( c == '{' )
			{
				container = xpc_dictionary_create( NULL, NULL, 0 );
				require_action( container, exit, err = kNoMemoryErr );
			}
			else
			{
				container = xpc_array_create( NULL, 0 );
				require_action( container, exit, err = kNoMemoryErr );
			}
			continue;
		}
		
		// Parse the value string.
		
		err = _ParseEscapedString( ptr, end, "{}[]" kWhiteSpaceCharSet, valStr, sizeof( valStr ), NULL, NULL, &ptr );
		require_noerr_quiet( err, exit );
		
		err = _XPCObjectFromString( valStr, &value );
		require_noerr_quiet( err, exit );
		
		if( containerType == XPC_TYPE_DICTIONARY )
		{
			xpc_dictionary_set_value( container, keyStr, value );
		}
		else
		{
			xpc_array_append_value( container, value );
		}
		xpc_forget( &value );
	}
	require_action_quiet( !list, exit, err = kMalformedErr );
	
	check( container );
	check( xpc_get_type( container ) == XPC_TYPE_DICTIONARY );
	
	*outDict = container;
	container = NULL;
	err = kNoErr;
	
exit:
	xpc_release_null_safe( container );
	if( list ) _XPCListFree( list );
	return( err );
}

static OSStatus	_XPCObjectFromString( const char *inString, xpc_object_t *outObject )
{
	OSStatus			err;
	xpc_object_t		object;
	
	if( 0 ) {}
	
	// Bool
	
	else if( stricmp_prefix( inString, kXPCObjectPrefix_Bool ) == 0 )
	{
		const char * const		str = inString + sizeof_string( kXPCObjectPrefix_Bool );
		bool					value;
		
		if( IsTrueString(  str, kSizeCString ) )
		{
			value = true;
		}
		else if( IsFalseString( str, kSizeCString ) )
		{
			value = false;
		}
		else
		{
			err = kValueErr;
			goto exit;
		}
		
		object = xpc_bool_create( value );
		require_action( object, exit, err = kNoMemoryErr );
	}
	
	// Data
	
	else if( stricmp_prefix( inString, kXPCObjectPrefix_Data ) == 0 )
	{
		const char * const		str = inString + sizeof_string( kXPCObjectPrefix_Data );
		uint8_t *				dataPtr;
		size_t					dataLen;
		
		err = HexToDataCopy( str, kSizeCString, kHexToData_DefaultFlags, &dataPtr, &dataLen, NULL );
		require_noerr( err, exit );
		
		object = xpc_data_create( dataPtr, dataLen );
		free( dataPtr );
		require_action( object, exit, err = kNoMemoryErr );
	}
	
	// Int64
	
	else if( stricmp_prefix( inString, kXPCObjectPrefix_Int64 ) == 0 )
	{
		const char * const		str = inString + sizeof_string( kXPCObjectPrefix_Int64 );
		int64_t					i64;
		
		i64 = _StringToInt64( str, &err );
		require_noerr_quiet( err, exit );
		
		object = xpc_int64_create( i64 );
		require_action( object, exit, err = kNoMemoryErr );
	}
	
	// String
	
	else if( stricmp_prefix( inString, kXPCObjectPrefix_String ) == 0 )
	{
		const char * const		str = inString + sizeof_string( kXPCObjectPrefix_String );
		
		object = xpc_string_create( str );
		require_action( object, exit, err = kNoMemoryErr );
	}
	
	// UInt64
	
	else if( stricmp_prefix( inString, kXPCObjectPrefix_UInt64 ) == 0 )
	{
		const char * const		str = inString + sizeof_string( kXPCObjectPrefix_UInt64 );
		uint64_t				u64;
		
		u64 = _StringToUInt64( str, &err );
		require_noerr_quiet( err, exit );
		
		object = xpc_uint64_create( u64 );
		require_action( object, exit, err = kNoMemoryErr );
	}
	
	// UUID
	
	else if( stricmp_prefix( inString, kXPCObjectPrefix_UUID ) == 0 )
	{
		const char * const		str = inString + sizeof_string( kXPCObjectPrefix_UUID );
		uuid_t					uuid;
		
		err = uuid_parse( str, uuid );
		require_noerr_action_quiet( err, exit, err = kValueErr );
		
		object = xpc_uuid_create( uuid );
		require_action( object, exit, err = kNoMemoryErr );
	}
	
	// Unsupported prefix
	
	else
	{
		err = kValueErr;
		goto exit;
	}
	
	*outObject = object;
	err = kNoErr;
	
exit:
	return( err );
}

static OSStatus	_XPCListItemCreate( xpc_object_t inObject, const char *inKey, XPCListItem **outItem )
{
	OSStatus			err;
	XPCListItem *		item;
	
	item = (XPCListItem *) calloc( 1, sizeof( *item ) );
	require_action( item, exit, err = kNoMemoryErr );
	
	item->obj = xpc_retain( inObject );
	if( ( xpc_get_type( item->obj ) == XPC_TYPE_DICTIONARY ) && inKey )
	{
		item->key = strdup( inKey );
		require_action( item->key, exit, err = kNoMemoryErr );
	}
	
	*outItem = item;
	item = NULL;
	err = kNoErr;
	
exit:
	if( item ) _XPCListItemFree( item );
	return( err );
}

static void	_XPCListItemFree( XPCListItem *inItem )
{
	xpc_forget( &inItem->obj );
	ForgetMem( &inItem->key );
	free( inItem );
}

static void _XPCListFree( XPCListItem *inList )
{
	XPCListItem *		item;
	
	while( ( item = inList ) != NULL )
	{
		inList = item->next;
		_XPCListItemFree( item );
	}
}
#endif	// TARGET_OS_DARWIN

#if( MDNSRESPONDER_PROJECT )
//===========================================================================================================================
//	InterfaceMonitorCmd
//===========================================================================================================================

static void	_InterfaceMonitorPrint( mdns_interface_monitor_t inMonitor );
static void	_InterfaceMonitorSignalHandler( void *inContext );

static void	InterfaceMonitorCmd( void )
{
	OSStatus						err;
	mdns_interface_monitor_t		monitor;
	dispatch_source_t				signalSource = NULL;
	uint32_t						ifIndex;
	__block int						exitCode;
	
	err = InterfaceIndexFromArgString( gInterface, &ifIndex );
	require_noerr_quiet( err, exit );
	
	monitor = mdns_interface_monitor_create( ifIndex );
	require_action( monitor, exit, err = kNoResourcesErr );
	
	exitCode = 0;
	mdns_interface_monitor_set_queue( monitor, dispatch_get_main_queue() );
	mdns_interface_monitor_set_event_handler( monitor,
	^( mdns_event_t inEvent, OSStatus inError )
	{
		switch( inEvent )
		{
			case mdns_event_error:
				FPrintF( stderr, "error: Interface monitor failed: %#m\n", inError );
				mdns_interface_monitor_invalidate( monitor );
				exitCode = 1;
				break;
			
			case mdns_event_invalidated:
				FPrintF( stdout, "Interface monitor invalidated.\n" );
				mdns_release( monitor );
				exit( exitCode );
			
			default:
				FPrintF( stdout, "Unhandled event '%s' (%ld)\n", mdns_event_to_string( inEvent ), (long) inEvent );
				break;
		}
	} );
	mdns_interface_monitor_set_update_handler( monitor,
	^( __unused mdns_interface_flags_t inChangeFlags )
	{
		_InterfaceMonitorPrint( monitor );
	} );
	
	_InterfaceMonitorPrint( monitor );
	mdns_interface_monitor_activate( monitor );
	
	signal( SIGINT, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGINT, dispatch_get_main_queue(), _InterfaceMonitorSignalHandler, monitor,
		&signalSource );
	require_noerr( err, exit );
	dispatch_resume( signalSource );
	
	dispatch_main();
	
exit:
	if( err ) ErrQuit( 1, "error: %#m\n", err );
}

static void	_InterfaceMonitorPrint( mdns_interface_monitor_t inMonitor )
{
	FPrintF( stdout, "%{du:time} %@\n", NULL, inMonitor );
}

static void	_InterfaceMonitorSignalHandler( void *inContext )
{
	mdns_interface_monitor_invalidate( (mdns_interface_monitor_t) inContext );
}

//===========================================================================================================================
//	DNSProxyCmd
//===========================================================================================================================

static void	_DNSProxyCallback( DNSXConnRef inConnection, DNSXErrorType inError );
static void	_DNSProxyCmdSignalHandler( void *inContext );

static void	DNSProxyCmd( void )
{
	OSStatus				err;
	size_t					i;
	DNSXConnRef				connection;
	IfIndex					inputIfIndexes[ MaxInputIf ];
	dispatch_source_t		sigIntSource	= NULL;
	dispatch_source_t		sigTermSource	= NULL;
	uint32_t				outputIfIndex;
	char					ifName[ kInterfaceNameBufLen ];
	
	if( gDNSProxy_InputInterfaceCount > MaxInputIf )
	{
		FPrintF( stderr, "error: Invalid input interface count: %zu > %d (max).\n",
			gDNSProxy_InputInterfaceCount, MaxInputIf );
		err = kRangeErr;
		goto exit;
	}
	
	for( i = 0; i < gDNSProxy_InputInterfaceCount; ++i )
	{
		uint32_t		ifIndex;
		
		err = InterfaceIndexFromArgString( gDNSProxy_InputInterfaces[ i ], &ifIndex );
		require_noerr_quiet( err, exit );
		
		inputIfIndexes[ i ] = ifIndex;
	}
	while( i < MaxInputIf ) inputIfIndexes[ i++ ] = 0;	// Remaining interface indexes are required to be 0.
	
	if( gDNSProxy_OutputInterface )
	{
		err = InterfaceIndexFromArgString( gDNSProxy_OutputInterface, &outputIfIndex );
		require_noerr_quiet( err, exit );
	}
	else
	{
		outputIfIndex = kDNSIfindexAny;
	}
	
	FPrintF( stdout, "Input Interfaces:" );
	for( i = 0; i < gDNSProxy_InputInterfaceCount; ++i )
	{
		const uint32_t		ifIndex = (uint32_t) inputIfIndexes[ i ];
		
		FPrintF( stdout, "%s %u (%s)", ( i == 0 ) ? "" : ",", ifIndex, InterfaceIndexToName( ifIndex, ifName ) );
	}
	FPrintF( stdout, "\n" );
	FPrintF( stdout, "Output Interface: %u (%s)\n",		outputIfIndex, InterfaceIndexToName( outputIfIndex, ifName ) );
	FPrintF( stdout, "Start time:       %{du:time}\n",	NULL );
	FPrintF( stdout, "---\n" );
	
	connection = NULL;
	err = DNSXEnableProxy( &connection, kDNSProxyEnable, inputIfIndexes, outputIfIndex, dispatch_get_main_queue(),
		_DNSProxyCallback );
	require_noerr_quiet( err, exit );
	
	signal( SIGINT, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGINT, dispatch_get_main_queue(), _DNSProxyCmdSignalHandler, connection,
		&sigIntSource );
	require_noerr( err, exit );
	dispatch_activate( sigIntSource );
	
	signal( SIGTERM, SIG_IGN );
	err = DispatchSignalSourceCreate( SIGTERM, dispatch_get_main_queue(), _DNSProxyCmdSignalHandler, connection,
		&sigTermSource );
	require_noerr( err, exit );
	dispatch_activate( sigTermSource );
	
	dispatch_main();
	
exit:
	if( err ) ErrQuit( 1, "error: %#m\n", err );
}

static void	_DNSProxyCallback( DNSXConnRef inConnection, DNSXErrorType inError )
{
	Unused( inConnection );
	
	if( inError ) ErrQuit( 1, "error: DNS proxy failed: %#m\n", inError );
}

static void	_DNSProxyCmdSignalHandler( void *inContext )
{
	DNSXConnRef const		connection = (DNSXConnRef) inContext;
	struct timeval			now;
	
	gettimeofday( &now, NULL );
	
	DNSXRefDeAlloc( connection );
	
	FPrintF( stdout, "---\n" );
	FPrintF( stdout, "End time:         %{du:time}\n", &now );
	exit( 0 );
}

//===========================================================================================================================
//    XCTestCmd
//===========================================================================================================================

static void    XCTestCmd( void )
{
    int result = 0;
    setenv(DNSSDUTIL_XCTEST, DNSSDUTIL_XCTEST, 0);
    if(!TestUtilsRunXCTestNamed(gXCTest_Classname)) {
        result = 1;
    }
    unsetenv(DNSSDUTIL_XCTEST);
    exit( result );
}

#endif	// MDNSRESPONDER_PROJECT

//===========================================================================================================================
//	DaemonVersionCmd
//===========================================================================================================================

static void	DaemonVersionCmd( void )
{
	OSStatus		err;
	uint32_t		size, version;
	char			strBuf[ 16 ];
	
	size = (uint32_t) sizeof( version );
	err = DNSServiceGetProperty( kDNSServiceProperty_DaemonVersion, &version, &size );
	require_noerr( err, exit );
	
	FPrintF( stdout, "Daemon version: %s\n", SourceVersionToCString( version, strBuf ) );
	
exit:
	if( err ) exit( 1 );
}

//===========================================================================================================================
//	Exit
//===========================================================================================================================

static void	Exit( void *inContext )
{
	const char * const		reason = (const char *) inContext;
	
	FPrintF( stdout, "---\n" );
	FPrintF( stdout, "End time:   %{du:time}\n", NULL );
	if( reason ) FPrintF( stdout, "End reason: %s\n", reason );
	exit( gExitCode );
}

//===========================================================================================================================
//	_PrintFExtensionTimestampHandler
//===========================================================================================================================

static int
	_PrintFExtensionTimestampHandler(
		PrintFContext *	inContext,
		PrintFFormat *	inFormat,
		PrintFVAList *	inArgs,
		void *			inUserContext )
{
	struct timeval				now;
	const struct timeval *		tv;
	struct tm *					localTime;
	size_t						len;
	int							n;
	char						dateTimeStr[ 32 ];
	
	Unused( inUserContext );
	
	tv = va_arg( inArgs->args, const struct timeval * );
	require_action_quiet( !inFormat->suppress, exit, n = 0 );
	
	if( !tv )
	{
		gettimeofday( &now, NULL );
		tv = &now;
	}
	localTime = localtime( &tv->tv_sec );
	len = strftime( dateTimeStr, sizeof( dateTimeStr ), "%Y-%m-%d %H:%M:%S", localTime );
	if( len == 0 ) dateTimeStr[ 0 ] = '\0';
	
	n = PrintFCore( inContext, "%s.%06u", dateTimeStr, (unsigned int) tv->tv_usec );
	
exit:
	return( n );
}

//===========================================================================================================================
//	_PrintFExtensionDNSMessageHandler
//===========================================================================================================================

static int
	_PrintFExtensionDNSMessageHandler(
		PrintFContext *	inContext,
		PrintFFormat *	inFormat,
		PrintFVAList *	inArgs,
		void *			inUserContext )
{
	OSStatus			err;
	const void *		msgPtr;
	size_t				msgLen;
	char *				text;
	int					n;
	Boolean				isMDNS;
	Boolean				printRawRData;
	
	Unused( inUserContext );
	
	msgPtr = va_arg( inArgs->args, const void * );
	msgLen = va_arg( inArgs->args, size_t );
	require_action_quiet( !inFormat->suppress, exit, n = 0 );
	
	isMDNS			= ( inFormat->altForm   > 0 ) ? true : false;
	printRawRData	= ( inFormat->precision > 0 ) ? true : false;
	err = DNSMessageToText( msgPtr, msgLen, isMDNS, printRawRData, &text );
	if( !err )
	{
		n = PrintFCore( inContext, "%*{text}", inFormat->fieldWidth, text, kSizeCString );
		free( text );
	}
	else
	{
		n = PrintFCore( inContext, "%*.1H", inFormat->fieldWidth, msgPtr, (int) msgLen, (int) msgLen );
	}
	
exit:
	return( n );
}

//===========================================================================================================================
//	_PrintFExtensionCallbackFlagsHandler
//===========================================================================================================================

static int
	_PrintFExtensionCallbackFlagsHandler(
		PrintFContext *	inContext,
		PrintFFormat *	inFormat,
		PrintFVAList *	inArgs,
		void *			inUserContext )
{
	DNSServiceFlags		flags;
	int					n;
	
	Unused( inUserContext );
	
	flags = va_arg( inArgs->args, DNSServiceFlags );
	require_action_quiet( !inFormat->suppress, exit, n = 0 );
	
	n = PrintFCore( inContext, "%08X %s%c %c%c",
		flags, DNSServiceFlagsToAddRmvStr( flags ),
		( flags & kDNSServiceFlagsMoreComing )       ? '+' : ' ',
		( flags & kDNSServiceFlagAnsweredFromCache ) ? 'C' : ' ',
		( flags & kDNSServiceFlagsExpiredAnswer )    ? '*' : ' ' );
	
exit:
	return( n );
}

//===========================================================================================================================
//	_PrintFExtensionDNSRecordDataHandler
//===========================================================================================================================

static int
	_PrintFExtensionDNSRecordDataHandler(
		PrintFContext *	inContext,
		PrintFFormat *	inFormat,
		PrintFVAList *	inArgs,
		void *			inUserContext )
{
	const void *		rdataPtr;
	unsigned int		rdataLen, rdataType;
	int					n, fieldWidth;
	
	Unused( inUserContext );
	
	rdataType	= va_arg( inArgs->args, unsigned int );
	rdataPtr	= va_arg( inArgs->args, const void * );
	rdataLen	= va_arg( inArgs->args, unsigned int );
	require_action_quiet( !inFormat->suppress, exit, n = 0 );
	
	check( inFormat->fieldWidth < INT_MAX );
	fieldWidth = inFormat->leftJustify ? -( (int) inFormat->fieldWidth ) : ( (int) inFormat->fieldWidth );
	
	if( rdataLen > 0 )
	{
		char *		rdataStr = NULL;
		
		DNSRecordDataToString( rdataPtr, rdataLen, rdataType, NULL, 0, &rdataStr );
		if( rdataStr )
		{
			n = PrintFCore( inContext, "%*s", fieldWidth, rdataStr );
			free( rdataStr );
		}
		else
		{
			n = PrintFCore( inContext, "%*H", fieldWidth, rdataPtr, rdataLen, rdataLen );
		}
	}
	else
	{
		n = PrintFCore( inContext, "%*s", fieldWidth, "<< ZERO-LENGTH RDATA >>" );
	}
	
exit:
	return( n );
}

//===========================================================================================================================
//	GetDNSSDFlagsFromOpts
//===========================================================================================================================

static DNSServiceFlags	GetDNSSDFlagsFromOpts( void )
{
	DNSServiceFlags		flags;
	
	flags = (DNSServiceFlags) gDNSSDFlags;
	if( flags & kDNSServiceFlagsShareConnection )
	{
		FPrintF( stderr, "*** Warning: kDNSServiceFlagsShareConnection (0x%X) is explicitly set in flag parameters.\n",
			kDNSServiceFlagsShareConnection );
	}
	
	if( gDNSSDFlag_AllowExpiredAnswers )	flags |= kDNSServiceFlagsAllowExpiredAnswers;
	if( gDNSSDFlag_BrowseDomains )			flags |= kDNSServiceFlagsBrowseDomains;
	if( gDNSSDFlag_DenyCellular )			flags |= kDNSServiceFlagsDenyCellular;
	if( gDNSSDFlag_DenyConstrained )		flags |= kDNSServiceFlagsDenyConstrained;
	if( gDNSSDFlag_DenyExpensive )			flags |= kDNSServiceFlagsDenyExpensive;
	if( gDNSSDFlag_ForceMulticast )			flags |= kDNSServiceFlagsForceMulticast;
	if( gDNSSDFlag_IncludeAWDL )			flags |= kDNSServiceFlagsIncludeAWDL;
	if( gDNSSDFlag_KnownUnique )			flags |= kDNSServiceFlagsKnownUnique;
	if( gDNSSDFlag_NoAutoRename )			flags |= kDNSServiceFlagsNoAutoRename;
	if( gDNSSDFlag_PathEvaluationDone )		flags |= kDNSServiceFlagsPathEvaluationDone;
	if( gDNSSDFlag_RegistrationDomains )	flags |= kDNSServiceFlagsRegistrationDomains;
	if( gDNSSDFlag_ReturnIntermediates )	flags |= kDNSServiceFlagsReturnIntermediates;
	if( gDNSSDFlag_Shared )					flags |= kDNSServiceFlagsShared;
	if( gDNSSDFlag_SuppressUnusable )		flags |= kDNSServiceFlagsSuppressUnusable;
	if( gDNSSDFlag_Timeout )				flags |= kDNSServiceFlagsTimeout;
	if( gDNSSDFlag_UnicastResponse )		flags |= kDNSServiceFlagsUnicastResponse;
	if( gDNSSDFlag_Unique )					flags |= kDNSServiceFlagsUnique;
	if( gDNSSDFlag_WakeOnResolve )			flags |= kDNSServiceFlagsWakeOnResolve;
	
	return( flags );
}

//===========================================================================================================================
//	CreateConnectionFromArgString
//===========================================================================================================================

static OSStatus
	CreateConnectionFromArgString(
		const char *			inString,
		dispatch_queue_t		inQueue,
		DNSServiceRef *			outSDRef,
		ConnectionDesc *		outDesc )
{
	OSStatus			err;
	DNSServiceRef		sdRef = NULL;
	ConnectionType		type;
	int32_t				pid = -1;	// Initializing because the analyzer claims pid may be used uninitialized.
	uint8_t				uuid[ 16 ];
	
	if( strcasecmp( inString, kConnectionArg_Normal ) == 0 )
	{
		err = DNSServiceCreateConnection( &sdRef );
		require_noerr( err, exit );
		type = kConnectionType_Normal;
	}
	else if( stricmp_prefix( inString, kConnectionArgPrefix_PID ) == 0 )
	{
		const char * const		pidStr = inString + sizeof_string( kConnectionArgPrefix_PID );
		
		err = StringToInt32( pidStr, &pid );
		if( err )
		{
			FPrintF( stderr, "Invalid delegate connection PID value: %s\n", pidStr );
			err = kParamErr;
			goto exit;
		}
		
		memset( uuid, 0, sizeof( uuid ) );
		err = DNSServiceCreateDelegateConnection( &sdRef, pid, uuid );
		if( err )
		{
			FPrintF( stderr, "DNSServiceCreateDelegateConnection() returned %#m for PID %d\n", err, pid );
			goto exit;
		}
		type = kConnectionType_DelegatePID;
	}
	else if( stricmp_prefix( inString, kConnectionArgPrefix_UUID ) == 0 )
	{
		const char * const		uuidStr = inString + sizeof_string( kConnectionArgPrefix_UUID );
		
		check_compile_time_code( sizeof( uuid ) == sizeof( uuid_t ) );
		
		err = StringToUUID( uuidStr, kSizeCString, false, uuid );
		if( err )
		{
			FPrintF( stderr, "Invalid delegate connection UUID value: %s\n", uuidStr );
			err = kParamErr;
			goto exit;
		}
		
		err = DNSServiceCreateDelegateConnection( &sdRef, 0, uuid );
		if( err )
		{
			FPrintF( stderr, "DNSServiceCreateDelegateConnection() returned %#m for UUID %#U\n", err, uuid );
			goto exit;
		}
		type = kConnectionType_DelegateUUID;
	}
	else
	{
		FPrintF( stderr, "Unrecognized connection string \"%s\".\n", inString );
		err = kParamErr;
		goto exit;
	}
	
	err = DNSServiceSetDispatchQueue( sdRef, inQueue );
	require_noerr( err, exit );
	
	*outSDRef = sdRef;
	if( outDesc )
	{
		outDesc->type = type;
		if(      type == kConnectionType_DelegatePID )	outDesc->delegate.pid = pid;
		else if( type == kConnectionType_DelegateUUID )	memcpy( outDesc->delegate.uuid, uuid, 16 );
	}
	sdRef = NULL;
	
exit:
	if( sdRef ) DNSServiceRefDeallocate( sdRef );
	return( err );
}

//===========================================================================================================================
//	InterfaceIndexFromArgString
//===========================================================================================================================

static OSStatus	InterfaceIndexFromArgString( const char *inString, uint32_t *outIndex )
{
	OSStatus		err;
	uint32_t		ifIndex;
	
	if( inString )
	{
		ifIndex = if_nametoindex( inString );
		if( ifIndex == 0 )
		{
			err = StringToUInt32( inString, &ifIndex );
			if( err )
			{
				FPrintF( stderr, "error: Invalid interface value: %s\n", inString );
				err = kParamErr;
				goto exit;
			}
		}
	}
	else
	{
		ifIndex	= 0;
	}
	
	*outIndex = ifIndex;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	RecordDataFromArgString
//===========================================================================================================================

static OSStatus	RecordDataFromArgString( const char *inString, uint8_t **outDataPtr, size_t *outDataLen )
{
	OSStatus		err;
	uint8_t *		dataPtr = NULL;
	size_t			dataLen;
	
	if( 0 ) {}
	
	// Domain name
	
	else if( stricmp_prefix( inString, kRDataArgPrefix_Domain ) == 0 )
	{
		const char * const		str = inString + sizeof_string( kRDataArgPrefix_Domain );
		
		err = StringToDomainName( str, &dataPtr, &dataLen );
		require_noerr_quiet( err, exit );
	}
	
	// File path
	
	else if( stricmp_prefix( inString, kRDataArgPrefix_File ) == 0 )
	{
		const char * const		path = inString + sizeof_string( kRDataArgPrefix_File );
		
		err = CopyFileDataByPath( path, (char **) &dataPtr, &dataLen );
		require_noerr( err, exit );
		require_action( dataLen <= kDNSRecordDataLengthMax, exit, err = kSizeErr );
	}
	
	// Hexadecimal string
	
	else if( stricmp_prefix( inString, kRDataArgPrefix_HexString ) == 0 )
	{
		const char * const		str = inString + sizeof_string( kRDataArgPrefix_HexString );
		
		err = HexToDataCopy( str, kSizeCString, kHexToData_DefaultFlags, &dataPtr, &dataLen, NULL );
		require_noerr( err, exit );
		require_action( dataLen <= kDNSRecordDataLengthMax, exit, err = kSizeErr );
	}
	
	// IPv4 address string
	
	else if( stricmp_prefix( inString, kRDataArgPrefix_IPv4 ) == 0 )
	{
		const char * const		str = inString + sizeof_string( kRDataArgPrefix_IPv4 );
		
		err = StringToARecordData( str, &dataPtr, &dataLen );
		require_noerr_quiet( err, exit );
	}
	
	// IPv6 address string
	
	else if( stricmp_prefix( inString, kRDataArgPrefix_IPv6 ) == 0 )
	{
		const char * const		str = inString + sizeof_string( kRDataArgPrefix_IPv6 );
		
		err = StringToAAAARecordData( str, &dataPtr, &dataLen );
		require_noerr_quiet( err, exit );
	}
	
	// SRV record
	
	else if( stricmp_prefix( inString, kRDataArgPrefix_SRV ) == 0 )
	{
		const char * const		str = inString + sizeof_string( kRDataArgPrefix_SRV );
		
		err = CreateSRVRecordDataFromString( str, &dataPtr, &dataLen );
		require_noerr( err, exit );
	}
	
	// String with escaped hex and octal bytes
	
	else if( stricmp_prefix( inString, kRDataArgPrefix_String ) == 0 )
	{
		const char * const		str = inString + sizeof_string( kRDataArgPrefix_String );
		const char * const		end = str + strlen( str );
		size_t					copiedLen;
		size_t					totalLen;
		Boolean					success;
		
		if( str < end )
		{
			success = _ParseQuotedEscapedString( str, end, "", NULL, 0, NULL, &totalLen, NULL );
			require_action( success, exit, err = kParamErr );
			require_action( totalLen <= kDNSRecordDataLengthMax, exit, err = kSizeErr );
			
			dataLen = totalLen;
			dataPtr = (uint8_t *) malloc( dataLen );
			require_action( dataPtr, exit, err = kNoMemoryErr );
			
			success = _ParseQuotedEscapedString( str, end, "", (char *) dataPtr, dataLen, &copiedLen, NULL, NULL );
			require_action( success, exit, err = kParamErr );
			check( copiedLen == dataLen );
		}
		else
		{
			dataPtr = NULL;
			dataLen = 0;
		}
	}
	
	// TXT record
	
	else if( stricmp_prefix( inString, kRDataArgPrefix_TXT ) == 0 )
	{
		const char * const		str = inString + sizeof_string( kRDataArgPrefix_TXT );
		
		err = CreateTXTRecordDataFromString( str, ',', &dataPtr, &dataLen );
		require_noerr( err, exit );
	}
	
	// Unrecognized format
	
	else
	{
		FPrintF( stderr, "Unrecognized record data string \"%s\".\n", inString );
		err = kParamErr;
		goto exit;
	}
	
	err = kNoErr;
	*outDataLen = dataLen;
	*outDataPtr = dataPtr;
	dataPtr = NULL;
	
exit:
	FreeNullSafe( dataPtr );
	return( err );
}

//===========================================================================================================================
//	RecordTypeFromArgString
//===========================================================================================================================

typedef struct
{
	uint16_t			value;	// Record type's numeric value.
	const char *		name;	// Record type's name as a string (e.g., "A", "PTR", "SRV").
	
}	RecordType;

static const RecordType		kRecordTypes[] =
{
	// Common types.
	
	{ kDNSServiceType_A,			"A" },
	{ kDNSServiceType_AAAA,			"AAAA" },
	{ kDNSServiceType_PTR,			"PTR" },
	{ kDNSServiceType_SRV,			"SRV" },
	{ kDNSServiceType_TXT,			"TXT" },
	{ kDNSServiceType_CNAME,		"CNAME" },
	{ kDNSServiceType_SOA,			"SOA" },
	{ kDNSServiceType_NSEC,			"NSEC" },
	{ kDNSServiceType_NS,			"NS" },
	{ kDNSServiceType_MX,			"MX" },
	{ kDNSServiceType_ANY,			"ANY" },
	{ kDNSServiceType_OPT,			"OPT" },
	
	// Less common types.
	
	{ kDNSServiceType_MD,			"MD" },
	{ kDNSServiceType_NS,			"NS" },
	{ kDNSServiceType_MD,			"MD" },
	{ kDNSServiceType_MF,			"MF" },
	{ kDNSServiceType_MB,			"MB" },
	{ kDNSServiceType_MG,			"MG" },
	{ kDNSServiceType_MR,			"MR" },
	{ kDNSServiceType_NULL,			"NULL" },
	{ kDNSServiceType_WKS,			"WKS" },
	{ kDNSServiceType_HINFO,		"HINFO" },
	{ kDNSServiceType_MINFO,		"MINFO" },
	{ kDNSServiceType_RP,			"RP" },
	{ kDNSServiceType_AFSDB,		"AFSDB" },
	{ kDNSServiceType_X25,			"X25" },
	{ kDNSServiceType_ISDN,			"ISDN" },
	{ kDNSServiceType_RT,			"RT" },
	{ kDNSServiceType_NSAP,			"NSAP" },
	{ kDNSServiceType_NSAP_PTR,		"NSAP_PTR" },
	{ kDNSServiceType_SIG,			"SIG" },
	{ kDNSServiceType_KEY,			"KEY" },
	{ kDNSServiceType_PX,			"PX" },
	{ kDNSServiceType_GPOS,			"GPOS" },
	{ kDNSServiceType_LOC,			"LOC" },
	{ kDNSServiceType_NXT,			"NXT" },
	{ kDNSServiceType_EID,			"EID" },
	{ kDNSServiceType_NIMLOC,		"NIMLOC" },
	{ kDNSServiceType_ATMA,			"ATMA" },
	{ kDNSServiceType_NAPTR,		"NAPTR" },
	{ kDNSServiceType_KX,			"KX" },
	{ kDNSServiceType_CERT,			"CERT" },
	{ kDNSServiceType_A6,			"A6" },
	{ kDNSServiceType_DNAME,		"DNAME" },
	{ kDNSServiceType_SINK,			"SINK" },
	{ kDNSServiceType_APL,			"APL" },
	{ kDNSServiceType_DS,			"DS" },
	{ kDNSServiceType_SSHFP,		"SSHFP" },
	{ kDNSServiceType_IPSECKEY,		"IPSECKEY" },
	{ kDNSServiceType_RRSIG,		"RRSIG" },
	{ kDNSServiceType_DNSKEY,		"DNSKEY" },
	{ kDNSServiceType_DHCID,		"DHCID" },
	{ kDNSServiceType_NSEC3,		"NSEC3" },
	{ kDNSServiceType_NSEC3PARAM,	"NSEC3PARAM" },
	{ kDNSServiceType_HIP,			"HIP" },
	{ kDNSServiceType_SPF,			"SPF" },
	{ kDNSServiceType_UINFO,		"UINFO" },
	{ kDNSServiceType_UID,			"UID" },
	{ kDNSServiceType_GID,			"GID" },
	{ kDNSServiceType_UNSPEC,		"UNSPEC" },
	{ kDNSServiceType_TKEY,			"TKEY" },
	{ kDNSServiceType_TSIG,			"TSIG" },
	{ kDNSServiceType_IXFR,			"IXFR" },
	{ kDNSServiceType_AXFR,			"AXFR" },
	{ kDNSServiceType_MAILB,		"MAILB" },
	{ kDNSServiceType_MAILA,		"MAILA" }
};

static OSStatus	RecordTypeFromArgString( const char *inString, uint16_t *outValue )
{
	OSStatus						err;
	int32_t							i32;
	const RecordType *				type;
	const RecordType * const		end = kRecordTypes + countof( kRecordTypes );
	
	for( type = kRecordTypes; type < end; ++type )
	{
		if( strcasecmp( type->name, inString ) == 0 )
		{
			*outValue = type->value;
			return( kNoErr );
		}
	}
	
	err = StringToInt32( inString, &i32 );
	require_noerr_quiet( err, exit );
	require_action_quiet( ( i32 >= 0 ) && ( i32 <= UINT16_MAX ), exit, err = kParamErr );
	
	*outValue = (uint16_t) i32;
	
exit:
	return( err );
}

//===========================================================================================================================
//	RecordClassFromArgString
//===========================================================================================================================

static OSStatus	RecordClassFromArgString( const char *inString, uint16_t *outValue )
{
	OSStatus		err;
	int32_t			i32;
	
	if( strcasecmp( inString, "IN" ) == 0 )
	{
		*outValue = kDNSServiceClass_IN;
		err = kNoErr;
		goto exit;
	}
	
	err = StringToInt32( inString, &i32 );
	require_noerr_quiet( err, exit );
	require_action_quiet( ( i32 >= 0 ) && ( i32 <= UINT16_MAX ), exit, err = kParamErr );
	
	*outValue = (uint16_t) i32;
	
exit:
	return( err );
}

//===========================================================================================================================
//	InterfaceIndexToName
//===========================================================================================================================

static char * InterfaceIndexToName( uint32_t inIfIndex, char inNameBuf[ kInterfaceNameBufLen ] )
{
	switch( inIfIndex )
	{
		case kDNSServiceInterfaceIndexAny:
			strlcpy( inNameBuf, "Any", kInterfaceNameBufLen );
			break;
		
		case kDNSServiceInterfaceIndexLocalOnly:
			strlcpy( inNameBuf, "LocalOnly", kInterfaceNameBufLen );
			break;
		
		case kDNSServiceInterfaceIndexUnicast:
			strlcpy( inNameBuf, "Unicast", kInterfaceNameBufLen );
			break;
		
		case kDNSServiceInterfaceIndexP2P:
			strlcpy( inNameBuf, "P2P", kInterfaceNameBufLen );
			break;
		
	#if( defined( kDNSServiceInterfaceIndexBLE ) )
		case kDNSServiceInterfaceIndexBLE:
			strlcpy( inNameBuf, "BLE", kInterfaceNameBufLen );
			break;
	#endif
		
		default:
		{
			const char *		name;
			
			name = if_indextoname( inIfIndex, inNameBuf );
			if( !name ) strlcpy( inNameBuf, "NO NAME", kInterfaceNameBufLen );
			break;
		}
	}
	
	return( inNameBuf );
}

//===========================================================================================================================
//	RecordTypeToString
//===========================================================================================================================

static const char *	RecordTypeToString( unsigned int inValue )
{
	const RecordType *				type;
	const RecordType * const		end = kRecordTypes + countof( kRecordTypes );
	
	for( type = kRecordTypes; type < end; ++type )
	{
		if( type->value == inValue ) return( type->name );
	}
	return( "???" );
}

//===========================================================================================================================
//	DNSRecordDataToString
//===========================================================================================================================

static OSStatus
	DNSRecordDataToString(
		const void *	inRDataPtr,
		size_t			inRDataLen,
		unsigned int	inRDataType,
		const void *	inMsgPtr,
		size_t			inMsgLen,
		char **			outString )
{
	OSStatus					err;
	const uint8_t * const		rdataPtr = (uint8_t *) inRDataPtr;
	const uint8_t * const		rdataEnd = rdataPtr + inRDataLen;
	char *						rdataStr;
	const uint8_t *				ptr;
	int							n;
	char						domainNameStr[ kDNSServiceMaxDomainName ];
	
	rdataStr = NULL;
	
	// A Record
	
	if( inRDataType == kDNSServiceType_A )
	{
		require_action_quiet( inRDataLen == 4, exit, err = kMalformedErr );
		
		ASPrintF( &rdataStr, "%.4a", rdataPtr );
		require_action( rdataStr, exit, err = kNoMemoryErr );
	}
	
	// AAAA Record
	
	else if( inRDataType == kDNSServiceType_AAAA )
	{
		require_action_quiet( inRDataLen == 16, exit, err = kMalformedErr );
		
		ASPrintF( &rdataStr, "%.16a", rdataPtr );
		require_action( rdataStr, exit, err = kNoMemoryErr );
	}
	
	// PTR, CNAME, or NS Record
	
	else if( ( inRDataType == kDNSServiceType_PTR )   ||
			 ( inRDataType == kDNSServiceType_CNAME ) ||
			 ( inRDataType == kDNSServiceType_NS ) )
	{
		if( inMsgPtr )
		{
			err = DNSMessageExtractDomainNameString( inMsgPtr, inMsgLen, rdataPtr, domainNameStr, NULL );
			require_noerr( err, exit );
		}
		else
		{
			err = DomainNameToString( rdataPtr, rdataEnd, domainNameStr, NULL );
			require_noerr( err, exit );
		}
		
		rdataStr = strdup( domainNameStr );
		require_action( rdataStr, exit, err = kNoMemoryErr );
	}
	
	// SRV Record
	
	else if( inRDataType == kDNSServiceType_SRV )
	{
		const dns_fixed_fields_srv *		fields;
		const uint8_t *						target;
		unsigned int						priority, weight, port;
		
		require_action_quiet( inRDataLen > sizeof( dns_fixed_fields_srv ), exit, err = kMalformedErr );
		
		fields		= (const dns_fixed_fields_srv *) rdataPtr;
		priority	= dns_fixed_fields_srv_get_priority( fields );
		weight		= dns_fixed_fields_srv_get_weight( fields );
		port		= dns_fixed_fields_srv_get_port( fields );
		target		= (const uint8_t *) &fields[ 1 ];
		
		if( inMsgPtr )
		{
			err = DNSMessageExtractDomainNameString( inMsgPtr, inMsgLen, target, domainNameStr, NULL );
			require_noerr( err, exit );
		}
		else
		{
			err = DomainNameToString( target, rdataEnd, domainNameStr, NULL );
			require_noerr( err, exit );
		}
		
		ASPrintF( &rdataStr, "%u %u %u %s", priority, weight, port, domainNameStr );
		require_action( rdataStr, exit, err = kNoMemoryErr );
	}
	
	// TXT Record
	
	else if( inRDataType == kDNSServiceType_TXT )
	{
		require_action_quiet( inRDataLen > 0, exit, err = kMalformedErr );
		
		if( inRDataLen == 1 )
		{
			ASPrintF( &rdataStr, "%#H", rdataPtr, (int) inRDataLen, INT_MAX );
			require_action( rdataStr, exit, err = kNoMemoryErr );
		}
		else
		{
			ASPrintF( &rdataStr, "%#{txt}", rdataPtr, inRDataLen );
			require_action( rdataStr, exit, err = kNoMemoryErr );
		}
	}
	
	// SOA Record
	
	else if( inRDataType == kDNSServiceType_SOA )
	{
		const dns_fixed_fields_soa *		fields;
		uint32_t							serial, refresh, retry, expire, minimum;
		
		if( inMsgPtr )
		{
			err = DNSMessageExtractDomainNameString( inMsgPtr, inMsgLen, rdataPtr, domainNameStr, &ptr );
			require_noerr( err, exit );
			
			require_action_quiet( ptr < rdataEnd, exit, err = kMalformedErr );
			
			rdataStr = strdup( domainNameStr );
			require_action( rdataStr, exit, err = kNoMemoryErr );
			
			err = DNSMessageExtractDomainNameString( inMsgPtr, inMsgLen, ptr, domainNameStr, &ptr );
			require_noerr( err, exit );
		}
		else
		{
			err = DomainNameToString( rdataPtr, rdataEnd, domainNameStr, &ptr );
			require_noerr( err, exit );
			
			rdataStr = strdup( domainNameStr );
			require_action( rdataStr, exit, err = kNoMemoryErr );
			
			err = DomainNameToString( ptr, rdataEnd, domainNameStr, &ptr );
			require_noerr( err, exit );
		}
		
		require_action_quiet( ( rdataEnd - ptr ) == sizeof( dns_fixed_fields_soa ), exit, err = kMalformedErr );
		
		fields	= (const dns_fixed_fields_soa *) ptr;
		serial	= dns_fixed_fields_soa_get_serial( fields );
		refresh	= dns_fixed_fields_soa_get_refresh( fields );
		retry	= dns_fixed_fields_soa_get_retry( fields );
		expire	= dns_fixed_fields_soa_get_expire( fields );
		minimum	= dns_fixed_fields_soa_get_minimum( fields );
		
		n = AppendPrintF( &rdataStr, " %s %u %u %u %u %u\n", domainNameStr, serial, refresh, retry, expire, minimum );
		require_action( n > 0, exit, err = kUnknownErr );
	}
	
	// NSEC Record
	
	else if( inRDataType == kDNSServiceType_NSEC )
	{
		unsigned int		windowBlock, bitmapLen, i, recordType;
		const uint8_t *		bitmapPtr;
		
		if( inMsgPtr )
		{
			err = DNSMessageExtractDomainNameString( inMsgPtr, inMsgLen, rdataPtr, domainNameStr, &ptr );
			require_noerr( err, exit );
		}
		else
		{
			err = DomainNameToString( rdataPtr, rdataEnd, domainNameStr, &ptr );
			require_noerr( err, exit );
		}
		
		require_action_quiet( ptr < rdataEnd, exit, err = kMalformedErr );
		
		rdataStr = strdup( domainNameStr );
		require_action( rdataStr, exit, err = kNoMemoryErr );
		
		for( ; ptr < rdataEnd; ptr += ( 2 + bitmapLen ) )
		{
			require_action_quiet( ( ptr + 2 ) < rdataEnd, exit, err = kMalformedErr );
			
			windowBlock	=  ptr[ 0 ];
			bitmapLen	=  ptr[ 1 ];
			bitmapPtr	= &ptr[ 2 ];
			
			require_action_quiet( ( bitmapLen >= 1 ) && ( bitmapLen <= 32 ) , exit, err = kMalformedErr );
			require_action_quiet( ( bitmapPtr + bitmapLen ) <= rdataEnd, exit, err = kMalformedErr );
			
			for( i = 0; i < BitArray_MaxBits( bitmapLen ); ++i )
			{
				if( BitArray_GetBit( bitmapPtr, bitmapLen, i ) )
				{
					recordType = ( windowBlock * 256 ) + i;
					n = AppendPrintF( &rdataStr, " %s", RecordTypeToString( recordType ) );
					require_action( n > 0, exit, err = kUnknownErr );
				}
			}
		}
	}
	
	// MX Record
	
	else if( inRDataType == kDNSServiceType_MX )
	{
		uint16_t			preference;
		const uint8_t *		exchange;
		
		require_action_quiet( ( rdataPtr + 2 ) < rdataEnd, exit, err = kMalformedErr );
		
		preference	= ReadBig16( rdataPtr );
		exchange	= &rdataPtr[ 2 ];
		
		if( inMsgPtr )
		{
			err = DNSMessageExtractDomainNameString( inMsgPtr, inMsgLen, exchange, domainNameStr, NULL );
			require_noerr( err, exit );
		}
		else
		{
			err = DomainNameToString( exchange, rdataEnd, domainNameStr, NULL );
			require_noerr( err, exit );
		}
		
		n = ASPrintF( &rdataStr, "%u %s", preference, domainNameStr );
		require_action( n > 0, exit, err = kUnknownErr );
	}
	
	// Unhandled record type
	
	else
	{
		err = kNotHandledErr;
		goto exit;
	}
	
	check( rdataStr );
	*outString = rdataStr;
	rdataStr = NULL;
	err = kNoErr;
	
exit:
	FreeNullSafe( rdataStr );
	return( err );
}

//===========================================================================================================================
//	DNSMessageToText
//===========================================================================================================================

#define DNSFlagsOpCodeToString( X ) (					\
	( (X) == kDNSOpCode_Query )			? "Query"	:	\
	( (X) == kDNSOpCode_InverseQuery )	? "IQuery"	:	\
	( (X) == kDNSOpCode_Status )		? "Status"	:	\
	( (X) == kDNSOpCode_Notify )		? "Notify"	:	\
	( (X) == kDNSOpCode_Update )		? "Update"	:	\
										  "Unassigned" )

#define DNSFlagsRCodeToString( X ) (						\
	( (X) == kDNSRCode_NoError )		? "NoError"		:	\
	( (X) == kDNSRCode_FormatError )	? "FormErr"		:	\
	( (X) == kDNSRCode_ServerFailure )	? "ServFail"	:	\
	( (X) == kDNSRCode_NXDomain )		? "NXDomain"	:	\
	( (X) == kDNSRCode_NotImplemented )	? "NotImp"		:	\
	( (X) == kDNSRCode_Refused )		? "Refused"		:	\
										  "???" )

static OSStatus
	DNSMessageToText(
		const uint8_t *	inMsgPtr,
		size_t			inMsgLen,
		const Boolean	inMDNS,
		const Boolean	inPrintRaw,
		char **			outText )
{
	OSStatus				err;
	DataBuffer				dataBuf;
	size_t					len;
	const DNSHeader *		hdr;
	const uint8_t *			ptr;
	unsigned int			id, flags, opcode, rcode;
	unsigned int			questionCount, answerCount, authorityCount, additionalCount, i, totalRRCount;
	uint8_t					name[ kDomainNameLengthMax ];
	char					nameStr[ kDNSServiceMaxDomainName ];
	
	DataBuffer_Init( &dataBuf, NULL, 0, SIZE_MAX );
	#define _Append( ... )		do { err = DataBuffer_AppendF( &dataBuf, __VA_ARGS__ ); require_noerr( err, exit ); } while( 0 )
	
	require_action_quiet( inMsgLen >= kDNSHeaderLength, exit, err = kSizeErr );
	
	hdr				= (DNSHeader *) inMsgPtr;
	id				= DNSHeaderGetID( hdr );
	flags			= DNSHeaderGetFlags( hdr );
	questionCount	= DNSHeaderGetQuestionCount( hdr );
	answerCount		= DNSHeaderGetAnswerCount( hdr );
	authorityCount	= DNSHeaderGetAuthorityCount( hdr );
	additionalCount	= DNSHeaderGetAdditionalCount( hdr );
	opcode			= DNSFlagsGetOpCode( flags );
	rcode			= DNSFlagsGetRCode( flags );
	
	_Append( "ID:               0x%04X (%u)\n", id, id );
	_Append( "Flags:            0x%04X %c/%s %cAA%cTC%cRD%cRA%?s%?s %s\n",
		flags,
		( flags & kDNSHeaderFlag_Response )				? 'R' : 'Q', DNSFlagsOpCodeToString( opcode ),
		( flags & kDNSHeaderFlag_AuthAnswer )			? ' ' : '!',
		( flags & kDNSHeaderFlag_Truncation )			? ' ' : '!',
		( flags & kDNSHeaderFlag_RecursionDesired )		? ' ' : '!',
		( flags & kDNSHeaderFlag_RecursionAvailable )	? ' ' : '!',
		!inMDNS, ( flags & kDNSHeaderFlag_AuthenticData )		? " AD" : "!AD",
		!inMDNS, ( flags & kDNSHeaderFlag_CheckingDisabled )	? " CD" : "!CD",
		DNSFlagsRCodeToString( rcode ) );
	_Append( "Question count:   %u\n", questionCount );
	_Append( "Answer count:     %u\n", answerCount );
	_Append( "Authority count:  %u\n", authorityCount );
	_Append( "Additional count: %u\n", additionalCount );
	
	ptr = (const uint8_t *) &hdr[ 1 ];
	for( i = 0; i < questionCount; ++i )
	{
		uint16_t		qtype, qclass;
		Boolean			isQU;
		
		err = DNSMessageExtractQuestion( inMsgPtr, inMsgLen, ptr, name, &qtype, &qclass, &ptr );
		require_noerr( err, exit );
		
		err = DomainNameToString( name, NULL, nameStr, NULL );
		require_noerr( err, exit );
		
		isQU = ( inMDNS && ( qclass & kQClassUnicastResponseBit ) ) ? true : false;
		if( inMDNS ) qclass &= ~kQClassUnicastResponseBit;
		
		if( i == 0 ) _Append( "\nQUESTION SECTION\n" );
		
		_Append( "%-30s %2s %?2s%?2u %-5s\n",
			nameStr, inMDNS ? ( isQU ? "QU" : "QM" ) : "",
			( qclass == kDNSServiceClass_IN ), "IN", ( qclass != kDNSServiceClass_IN ), qclass, RecordTypeToString( qtype ) );
	}
	
	totalRRCount = answerCount + authorityCount + additionalCount;
	for( i = 0; i < totalRRCount; ++i )
	{
		uint16_t			type;
		uint16_t			class;
		uint32_t			ttl;
		const uint8_t *		rdataPtr;
		size_t				rdataLen;
		char *				rdataStr;
		Boolean				cacheFlush;
		
		err = DNSMessageExtractRecord( inMsgPtr, inMsgLen, ptr, name, &type, &class, &ttl, &rdataPtr, &rdataLen, &ptr );
		require_noerr( err, exit );
		
		err = DomainNameToString( name, NULL, nameStr, NULL );
		require_noerr( err, exit );
		
		cacheFlush = ( inMDNS && ( class & kRRClassCacheFlushBit ) ) ? true : false;
		if( inMDNS ) class &= ~kRRClassCacheFlushBit;
		
		rdataStr = NULL;
		if( !inPrintRaw ) DNSRecordDataToString( rdataPtr, rdataLen, type, inMsgPtr, inMsgLen, &rdataStr );
		if( !rdataStr )
		{
			ASPrintF( &rdataStr, "%#H", rdataPtr, (int) rdataLen, (int) rdataLen );
			require_action( rdataStr, exit, err = kNoMemoryErr );
		}
		
		if(      answerCount     && ( i ==   0                              ) ) _Append( "\nANSWER SECTION\n" );
		else if( authorityCount  && ( i ==   answerCount                    ) ) _Append( "\nAUTHORITY SECTION\n" );
		else if( additionalCount && ( i == ( answerCount + authorityCount ) ) ) _Append( "\nADDITIONAL SECTION\n" );
		
		_Append( "%-42s %6u %2s %?2s%?2u %-5s %s\n",
			nameStr, ttl, cacheFlush ? "CF" : "",
			( class == kDNSServiceClass_IN ), "IN", ( class != kDNSServiceClass_IN ), class,
			RecordTypeToString( type ), rdataStr );
		free( rdataStr );
	}
	_Append( "\n" );
	
	err = DataBuffer_Append( &dataBuf, "", 1 );
	require_noerr( err, exit );
	
	err = DataBuffer_Detach( &dataBuf, (uint8_t **) outText, &len );
	require_noerr( err, exit );
	
exit:
	DataBuffer_Free( &dataBuf );
	return( err );
}

//===========================================================================================================================
//	WriteDNSQueryMessage
//===========================================================================================================================

static OSStatus
	WriteDNSQueryMessage(
		uint8_t			inMsg[ kDNSQueryMessageMaxLen ],
		uint16_t		inMsgID,
		uint16_t		inFlags,
		const char *	inQName,
		uint16_t		inQType,
		uint16_t		inQClass,
		size_t *		outMsgLen )
{
	OSStatus		err;
	uint8_t			qname[ kDomainNameLengthMax ];
	
	err = DomainNameFromString( qname, inQName, NULL );
	require_noerr_quiet( err, exit );
	
	err = DNSMessageWriteQuery( inMsgID, inFlags, qname, inQType, inQClass, inMsg, outMsgLen );
	require_noerr_quiet( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	DispatchSignalSourceCreate
//===========================================================================================================================

static OSStatus
	DispatchSignalSourceCreate(
		int					inSignal,
		dispatch_queue_t	inQueue,
		DispatchHandler		inEventHandler,
		void *				inContext,
		dispatch_source_t *	outSource )
{
	OSStatus				err;
	dispatch_source_t		source;
	
	source = dispatch_source_create( DISPATCH_SOURCE_TYPE_SIGNAL, (uintptr_t) inSignal, 0, inQueue );
	require_action( source, exit, err = kUnknownErr );
	
	dispatch_set_context( source, inContext );
	dispatch_source_set_event_handler_f( source, inEventHandler );
	
	*outSource = source;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	DispatchSocketSourceCreate
//===========================================================================================================================

static OSStatus
	DispatchSocketSourceCreate(
		SocketRef				inSock,
		dispatch_source_type_t	inType,
		dispatch_queue_t		inQueue,
		DispatchHandler			inEventHandler,
		DispatchHandler			inCancelHandler,
		void *					inContext,
		dispatch_source_t *		outSource )
{
	OSStatus				err;
	dispatch_source_t		source;
	
	source = dispatch_source_create( inType, (uintptr_t) inSock, 0, inQueue ? inQueue : dispatch_get_main_queue() );
	require_action( source, exit, err = kUnknownErr );
	
	dispatch_set_context( source, inContext );
	dispatch_source_set_event_handler_f( source, inEventHandler );
	dispatch_source_set_cancel_handler_f( source, inCancelHandler );
	
	*outSource = source;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	DispatchTimerCreate
//===========================================================================================================================

static OSStatus
	DispatchTimerCreate(
		dispatch_time_t		inStart,
		uint64_t			inIntervalNs,
		uint64_t			inLeewayNs,
		dispatch_queue_t	inQueue,
		DispatchHandler		inEventHandler,
		DispatchHandler		inCancelHandler,
		void *				inContext,
		dispatch_source_t *	outTimer )
{
	OSStatus				err;
	dispatch_source_t		timer;
	
	timer = dispatch_source_create( DISPATCH_SOURCE_TYPE_TIMER, 0, 0, inQueue ? inQueue : dispatch_get_main_queue() );
	require_action( timer, exit, err = kUnknownErr );
	
	dispatch_source_set_timer( timer, inStart, inIntervalNs, inLeewayNs );
	dispatch_set_context( timer, inContext );
	dispatch_source_set_event_handler_f( timer, inEventHandler );
	dispatch_source_set_cancel_handler_f( timer, inCancelHandler );
	
	*outTimer = timer;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	DispatchProcessMonitorCreate
//===========================================================================================================================

static OSStatus
	DispatchProcessMonitorCreate(
		pid_t				inPID,
		unsigned long		inFlags,
		dispatch_queue_t	inQueue,
		DispatchHandler		inEventHandler,
		DispatchHandler		inCancelHandler,
		void *				inContext,
		dispatch_source_t *	outMonitor )
{
	OSStatus				err;
	dispatch_source_t		monitor;
	
	monitor = dispatch_source_create( DISPATCH_SOURCE_TYPE_PROC, (uintptr_t) inPID, inFlags,
		inQueue ? inQueue : dispatch_get_main_queue() );
	require_action( monitor, exit, err = kUnknownErr );
	
	dispatch_set_context( monitor, inContext );
	dispatch_source_set_event_handler_f( monitor, inEventHandler );
	dispatch_source_set_cancel_handler_f( monitor, inCancelHandler );
	
	*outMonitor = monitor;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	ServiceTypeDescription
//===========================================================================================================================

typedef struct
{
	const char *		name;			// Name of the service type in two-label "_service._proto" format.
	const char *		description;	// Description of the service type.
	
}	ServiceType;

// A Non-comprehensive table of DNS-SD service types

static const ServiceType		kServiceTypes[] =
{
	{ "_acp-sync._tcp",			"AirPort Base Station Sync" },
	{ "_adisk._tcp",			"Automatic Disk Discovery" },
	{ "_afpovertcp._tcp",		"Apple File Sharing" },
	{ "_airdrop._tcp",			"AirDrop" },
	{ "_airplay._tcp",			"AirPlay" },
	{ "_airport._tcp",			"AirPort Base Station" },
	{ "_daap._tcp",				"Digital Audio Access Protocol (iTunes)" },
	{ "_eppc._tcp",				"Remote AppleEvents" },
	{ "_ftp._tcp",				"File Transfer Protocol" },
	{ "_home-sharing._tcp",		"Home Sharing" },
	{ "_homekit._tcp",			"HomeKit" },
	{ "_http._tcp",				"World Wide Web HTML-over-HTTP" },
	{ "_https._tcp",			"HTTP over SSL/TLS" },
	{ "_ipp._tcp",				"Internet Printing Protocol" },
	{ "_ldap._tcp",				"Lightweight Directory Access Protocol" },
	{ "_mediaremotetv._tcp",	"Media Remote" },
	{ "_net-assistant._tcp",	"Apple Remote Desktop" },
	{ "_od-master._tcp",		"OpenDirectory Master" },
	{ "_nfs._tcp",				"Network File System" },
	{ "_presence._tcp",			"Peer-to-peer messaging / Link-Local Messaging" },
	{ "_pdl-datastream._tcp",	"Printer Page Description Language Data Stream" },
	{ "_raop._tcp",				"Remote Audio Output Protocol" },
	{ "_rfb._tcp",				"Remote Frame Buffer" },
	{ "_scanner._tcp",			"Bonjour Scanning" },
	{ "_smb._tcp",				"Server Message Block over TCP/IP" },
	{ "_sftp-ssh._tcp",			"Secure File Transfer Protocol over SSH" },
	{ "_sleep-proxy._udp",		"Sleep Proxy Server" },
	{ "_ssh._tcp",				"SSH Remote Login Protocol" },
	{ "_teleport._tcp",			"teleport" },
	{ "_tftp._tcp",				"Trivial File Transfer Protocol" },
	{ "_workstation._tcp",		"Workgroup Manager" },
	{ "_webdav._tcp",			"World Wide Web Distributed Authoring and Versioning (WebDAV)" },
	{ "_webdavs._tcp",			"WebDAV over SSL/TLS" }
};

static const char *	ServiceTypeDescription( const char *inName )
{
	const ServiceType *				serviceType;
	const ServiceType * const		end = kServiceTypes + countof( kServiceTypes );
	
	for( serviceType = kServiceTypes; serviceType < end; ++serviceType )
	{
		if( ( stricmp_prefix( inName, serviceType->name ) == 0 ) )
		{
			const char * const		ptr = &inName[ strlen( serviceType->name ) ];
			
			if( ( ptr[ 0 ] == '\0' ) || ( ( ptr[ 0 ] == '.' ) && ( ptr[ 1 ] == '\0' ) ) )
			{
				return( serviceType->description );
			}
		}
	}
	return( NULL );
}

//===========================================================================================================================
//	SocketContextCreate
//===========================================================================================================================

static OSStatus	SocketContextCreate( SocketRef inSock, void * inUserContext, SocketContext **outContext )
{
	OSStatus			err;
	SocketContext *		context;
	
	context = (SocketContext *) calloc( 1, sizeof( *context ) );
	require_action( context, exit, err = kNoMemoryErr );
	
	context->refCount		= 1;
	context->sock			= inSock;
	context->userContext	= inUserContext;
	
	*outContext = context;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	SocketContextRetain
//===========================================================================================================================

static SocketContext *	SocketContextRetain( SocketContext *inContext )
{
	++inContext->refCount;
	return( inContext );
}

//===========================================================================================================================
//	SocketContextRelease
//===========================================================================================================================

static void	SocketContextRelease( SocketContext *inContext )
{
	if( --inContext->refCount == 0 )
	{
		ForgetSocket( &inContext->sock );
		free( inContext );
	}
}

//===========================================================================================================================
//	SocketContextCancelHandler
//===========================================================================================================================

static void	SocketContextCancelHandler( void *inContext )
{
	SocketContextRelease( (SocketContext *) inContext );
}

//===========================================================================================================================
//	StringToInt32
//===========================================================================================================================

static OSStatus	StringToInt32( const char *inString, int32_t *outValue )
{
	OSStatus		err;
	long			value;
	char *			endPtr;
	
	value = strtol( inString, &endPtr, 0 );
	require_action_quiet( ( *endPtr == '\0' ) && ( endPtr != inString ), exit, err = kParamErr );
	require_action_quiet( ( value >= INT32_MIN ) && ( value <= INT32_MAX ), exit, err = kRangeErr );
	
	*outValue = (int32_t) value;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	StringToUInt32
//===========================================================================================================================

static OSStatus	StringToUInt32( const char *inString, uint32_t *outValue )
{
	OSStatus		err;
	uint32_t		value;
	char *			endPtr;
	
	value = (uint32_t) strtol( inString, &endPtr, 0 );
	require_action_quiet( ( *endPtr == '\0' ) && ( endPtr != inString ), exit, err = kParamErr );
	
	*outValue = value;
	err = kNoErr;
	
exit:
	return( err );
}

#if( TARGET_OS_DARWIN )
//===========================================================================================================================
//	_StringToInt64
//===========================================================================================================================

static int64_t	_StringToInt64( const char *inString, OSStatus *outError )
{
	OSStatus		err;
	long long		val;
	char *			end;
	int				errnoVal;
	
	set_errno_compat( 0 );
	val = strtoll( inString, &end, 0 );
	errnoVal = errno_compat();
	
	require_action_quiet( ( *end == '\0' ) && ( end != inString ), exit, err = kMalformedErr );
	require_action_quiet( ( ( val != LLONG_MIN ) && ( val != LLONG_MAX ) ) || ( errnoVal != ERANGE ), exit, err = kRangeErr );
	require_action_quiet( ( val >= INT64_MIN ) && ( val <= INT64_MAX ), exit, err = kRangeErr );
	err = kNoErr;
	
exit:
	if( outError ) *outError = err;
	return( (int64_t)val );
}

//===========================================================================================================================
//	_StringToUInt64
//===========================================================================================================================

static uint64_t	_StringToUInt64( const char *inString, OSStatus *outError )
{
	OSStatus				err;
	unsigned long long		val;
	char *					end;
	int						errnoVal;
	
	set_errno_compat( 0 );
	val = strtoull( inString, &end, 0 );
	errnoVal = errno_compat();
	
	require_action_quiet( ( *end == '\0' ) && ( end != inString ), exit, err = kMalformedErr );
	require_action_quiet( ( val != ULLONG_MAX ) || ( errnoVal != ERANGE ), exit, err = kRangeErr );
	require_action_quiet( val <= UINT64_MAX, exit, err = kRangeErr );
	err = kNoErr;
	
exit:
	if( outError ) *outError = err;
	return( (uint64_t)val );
}

//===========================================================================================================================
//	_StringToPID
//===========================================================================================================================

static pid_t	_StringToPID( const char *inString, OSStatus *outError )
{
	OSStatus		err;
	int64_t			val;
	
	val = _StringToInt64( inString, &err );
	require_noerr_quiet( err, exit );
	require_action_quiet( val == (pid_t) val, exit, err = kRangeErr );
	err = kNoErr;
	
exit:
	if( outError ) *outError = err;
	return( (pid_t) val );
}

//===========================================================================================================================
//	_ParseEscapedString
//	
//	Note: Similar to ParseEscapedString() from CoreUtils except that _ParseEscapedString() takes an optional C string
//	containing delimiter characters instead of being limited to one delimiter character. Also, when the function returns
//	due to a delimiter, the output pointer is set to the delimiter character instead of the character after the delimiter.
//===========================================================================================================================

static OSStatus
	_ParseEscapedString(
		const char *	inSrc,
		const char *	inEnd,
		const char *	inDelimiters,
		char *			inBufPtr,
		size_t			inBufLen,
		size_t *		outCopiedLen,
		size_t *		outActualLen,
		const char **	outPtr )
{
	OSStatus				err;
	const char *			ptr;
	char *					dst = inBufPtr;
	const char * const		lim = ( inBufLen > 0 ) ? &inBufPtr[ inBufLen - 1 ] : inBufPtr;
	size_t					len;
	
	len = 0;
	ptr = inSrc;
	if( !inDelimiters ) inDelimiters = "";
	while( ptr < inEnd )
	{
		int					c;
		const char *		del;
		
		c = *ptr;
		for( del = inDelimiters; ( *del != '\0' ) && ( c != *del ); ++del ) {}
		if( *del != '\0' ) break;
		++ptr;
		if( c == '\\' )
		{
			require_action_quiet( ptr < inEnd, exit, err = kUnderrunErr );
			c = *ptr++;
		}
		++len;
		if( dst < lim ) *dst++ = (char) c;
	}
	if( inBufLen > 0 ) *dst = '\0';
	
	if( outCopiedLen )	*outCopiedLen	= (size_t)( dst - inBufPtr );
	if( outActualLen )	*outActualLen	= len;
	if( outPtr )		*outPtr			= ptr;
	err = kNoErr;
	
exit:
	return( err );
}
#endif

//===========================================================================================================================
//	StringToARecordData
//===========================================================================================================================

static OSStatus	StringToARecordData( const char *inString, uint8_t **outPtr, size_t *outLen )
{
	OSStatus			err;
	uint32_t *			addrPtr;
	const size_t		addrLen = sizeof( *addrPtr );
	const char *		end;
	
	addrPtr = (uint32_t *) malloc( addrLen );
	require_action( addrPtr, exit, err = kNoMemoryErr );
	
	err = _StringToIPv4Address( inString, kStringToIPAddressFlagsNoPort | kStringToIPAddressFlagsNoPrefix, addrPtr,
		NULL, NULL, NULL, &end );
	if( !err && ( *end != '\0' ) ) err = kMalformedErr;
	require_noerr_quiet( err, exit );
	
	*addrPtr = HostToBig32( *addrPtr );
	
	*outPtr = (uint8_t *) addrPtr;
	addrPtr = NULL;
	*outLen = addrLen;
	
exit:
	FreeNullSafe( addrPtr );
	return( err );
}

//===========================================================================================================================
//	StringToAAAARecordData
//===========================================================================================================================

static OSStatus	StringToAAAARecordData( const char *inString, uint8_t **outPtr, size_t *outLen )
{
	OSStatus			err;
	uint8_t *			addrPtr;
	const size_t		addrLen = 16;
	const char *		end;
	
	addrPtr = (uint8_t *) malloc( addrLen );
	require_action( addrPtr, exit, err = kNoMemoryErr );
	
	err = _StringToIPv6Address( inString,
		kStringToIPAddressFlagsNoPort | kStringToIPAddressFlagsNoPrefix | kStringToIPAddressFlagsNoScope,
		addrPtr, NULL, NULL, NULL, &end );
	if( !err && ( *end != '\0' ) ) err = kMalformedErr;
	require_noerr_quiet( err, exit );
	
	*outPtr = addrPtr;
	addrPtr = NULL;
	*outLen = addrLen;
	
exit:
	FreeNullSafe( addrPtr );
	return( err );
}

//===========================================================================================================================
//	StringToDomainName
//===========================================================================================================================

static OSStatus	StringToDomainName( const char *inString, uint8_t **outPtr, size_t *outLen )
{
	OSStatus		err;
	uint8_t *		namePtr;
	size_t			nameLen;
	uint8_t *		end;
	uint8_t			nameBuf[ kDomainNameLengthMax ];
	
	err = DomainNameFromString( nameBuf, inString, &end );
	require_noerr_quiet( err, exit );
	
	nameLen = (size_t)( end - nameBuf );
	namePtr = _memdup( nameBuf, nameLen );
	require_action( namePtr, exit, err = kNoMemoryErr );
	
	*outPtr = namePtr;
	namePtr = NULL;
	if( outLen ) *outLen = nameLen;
	
exit:
	return( err );
}

#if( TARGET_OS_DARWIN )
//===========================================================================================================================
//	GetDefaultDNSServer
//===========================================================================================================================

static OSStatus	GetDefaultDNSServer( sockaddr_ip *outAddr )
{
	OSStatus				err;
	dns_config_t *			config;
	struct sockaddr *		addr;
	int32_t					i;
	
	config = dns_configuration_copy();
	require_action( config, exit, err = kUnknownErr );
	
	addr = NULL;
	for( i = 0; i < config->n_resolver; ++i )
	{
		const dns_resolver_t * const		resolver = config->resolver[ i ];
		
		if( !resolver->domain && ( resolver->n_nameserver > 0 ) )
		{
			addr = resolver->nameserver[ 0 ];
			break;
		}
 	}
	require_action_quiet( addr, exit, err = kNotFoundErr );
	
	SockAddrCopy( addr, outAddr );
	err = kNoErr;
	
exit:
	if( config ) dns_configuration_free( config );
	return( err );
}
#endif

//===========================================================================================================================
//	GetMDNSMulticastAddrV4
//===========================================================================================================================

static void	_MDNSMulticastAddrV4Init( void *inContext );

static const struct sockaddr *	GetMDNSMulticastAddrV4( void )
{
	static struct sockaddr_in		sMDNSMulticastAddrV4;
	static dispatch_once_t			sMDNSMulticastAddrV4InitOnce = 0;
	
	dispatch_once_f( &sMDNSMulticastAddrV4InitOnce, &sMDNSMulticastAddrV4, _MDNSMulticastAddrV4Init );
	return( (const struct sockaddr *) &sMDNSMulticastAddrV4 );
}

static void	_MDNSMulticastAddrV4Init( void *inContext )
{
	struct sockaddr_in * const		addr = (struct sockaddr_in *) inContext;
	
	memset( addr, 0, sizeof( *addr ) );
	SIN_LEN_SET( addr );
	addr->sin_family		= AF_INET;
	addr->sin_port			= htons( kMDNSPort );
	addr->sin_addr.s_addr	= htonl( 0xE00000FB );	// The mDNS IPv4 multicast address is 224.0.0.251
}

//===========================================================================================================================
//	GetMDNSMulticastAddrV6
//===========================================================================================================================

static void	_MDNSMulticastAddrV6Init( void *inContext );

static const struct sockaddr *	GetMDNSMulticastAddrV6( void )
{
	static struct sockaddr_in6		sMDNSMulticastAddrV6;
	static dispatch_once_t			sMDNSMulticastAddrV6InitOnce = 0;
	
	dispatch_once_f( &sMDNSMulticastAddrV6InitOnce, &sMDNSMulticastAddrV6, _MDNSMulticastAddrV6Init );
	return( (const struct sockaddr *) &sMDNSMulticastAddrV6 );
}

static void	_MDNSMulticastAddrV6Init( void *inContext )
{
	struct sockaddr_in6 * const		addr = (struct sockaddr_in6 *) inContext;
	
	memset( addr, 0, sizeof( *addr ) );
	SIN6_LEN_SET( addr );
	addr->sin6_family	= AF_INET6;
	addr->sin6_port		= htons( kMDNSPort );
	addr->sin6_addr.s6_addr[  0 ] = 0xFF;	// The mDNS IPv6 multicast address is FF02::FB.
	addr->sin6_addr.s6_addr[  1 ] = 0x02;
	addr->sin6_addr.s6_addr[ 15 ] = 0xFB;
}

//===========================================================================================================================
//	CreateMulticastSocket
//===========================================================================================================================

static OSStatus
	CreateMulticastSocket(
		const struct sockaddr *	inAddr,
		int						inPort,
		const char *			inIfName,
		uint32_t				inIfIndex,
		Boolean					inJoin,
		int *					outPort,
		SocketRef *				outSock )
{
	OSStatus		err;
	SocketRef		sock	= kInvalidSocketRef;
	const int		family	= inAddr->sa_family;
	int				port;
	
	require_action_quiet( ( family == AF_INET ) ||( family == AF_INET6 ), exit, err = kUnsupportedErr );
	
	err = ServerSocketOpen( family, SOCK_DGRAM, IPPROTO_UDP, inPort, &port, kSocketBufferSize_DontSet, &sock );
	require_noerr_quiet( err, exit );
	
	err = SocketSetMulticastInterface( sock, inIfName, inIfIndex );
	require_noerr_quiet( err, exit );
	
	if( family == AF_INET )
	{
		err = setsockopt( sock, IPPROTO_IP, IP_MULTICAST_LOOP, (char *) &(uint8_t){ 1 }, (socklen_t) sizeof( uint8_t ) );
		err = map_socket_noerr_errno( sock, err );
		require_noerr_quiet( err, exit );
	}
	else
	{
		err = setsockopt( sock, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, (char *) &(int){ 1 }, (socklen_t) sizeof( int ) );
		err = map_socket_noerr_errno( sock, err );
		require_noerr_quiet( err, exit );
	}
	
	if( inJoin )
	{
		err = SocketJoinMulticast( sock, inAddr, inIfName, inIfIndex );
		require_noerr_quiet( err, exit );
	}
	
	if( outPort ) *outPort = port;
	*outSock = sock;
	sock = kInvalidSocketRef;
	
exit:
	ForgetSocket( &sock );
	return( err );
}

//===========================================================================================================================
//	DecimalTextToUInt32
//===========================================================================================================================

static OSStatus	DecimalTextToUInt32( const char *inSrc, const char *inEnd, uint32_t *outValue, const char **outPtr )
{
	OSStatus			err;
	uint64_t			value;
	const char *		ptr = inSrc;
	
	require_action_quiet( ( ptr < inEnd ) && isdigit_safe( *ptr ), exit, err = kMalformedErr );
	
	value = (uint64_t)( *ptr++ - '0' );
	if( value == 0 )
	{
		if( ( ptr < inEnd ) && isdigit_safe( *ptr ) )
		{
			err = kMalformedErr;
			goto exit;
		}
	}
	else
	{
		while( ( ptr < inEnd ) && isdigit_safe( *ptr ) )
		{
			value = ( value * 10 ) + (uint64_t)( *ptr++ - '0' );
			require_action_quiet( value <= UINT32_MAX, exit, err = kRangeErr );
		}
	}
	
	*outValue = (uint32_t) value;
	if( outPtr ) *outPtr = ptr;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	CheckIntegerArgument
//===========================================================================================================================

static OSStatus	CheckIntegerArgument( int inArgValue, const char *inArgName, int inMin, int inMax )
{
	if( ( inArgValue >= inMin ) && ( inArgValue <= inMax ) ) return( kNoErr );
	
	FPrintF( stderr, "error: Invalid %s: %d. Valid range is [%d, %d].\n", inArgName, inArgValue, inMin, inMax );
	return( kRangeErr );
}

//===========================================================================================================================
//	CheckDoubleArgument
//===========================================================================================================================

static OSStatus	CheckDoubleArgument( double inArgValue, const char *inArgName, double inMin, double inMax )
{
	if( ( inArgValue >= inMin ) && ( inArgValue <= inMax ) ) return( kNoErr );
	
	FPrintF( stderr, "error: Invalid %s: %.1f. Valid range is [%.1f, %.1f].\n", inArgName, inArgValue, inMin, inMax );
	return( kRangeErr );
}

//===========================================================================================================================
//	CheckRootUser
//===========================================================================================================================

static OSStatus	CheckRootUser( void )
{
	if( geteuid() == 0 ) return( kNoErr );
	
	FPrintF( stderr, "error: This command must to be run as root.\n" );
	return( kPermissionErr );
}

//===========================================================================================================================
//	SpawnCommand
//
//	Note: Based on systemf() from CoreUtils framework.
//===========================================================================================================================

extern char **		environ;

static OSStatus	SpawnCommand( pid_t *outPID, const char *inFormat, ... )
{
	OSStatus		err;
	va_list			args;
	char *			command;
	char *			argv[ 4 ];
	pid_t			pid;
	
	command = NULL;
	va_start( args, inFormat );
	VASPrintF( &command, inFormat, args );
	va_end( args );
	require_action( command, exit, err = kUnknownErr );
	
	argv[ 0 ] = "/bin/sh";
	argv[ 1 ] = "-c";
	argv[ 2 ] = command;
	argv[ 3 ] = NULL;
	err = posix_spawn( &pid, argv[ 0 ], NULL, NULL, argv, environ );
	free( command );
	require_noerr_quiet( err, exit );
	
	if( outPID ) *outPID = pid;
	
exit:
	return( err );
}

//===========================================================================================================================
//	OutputFormatFromArgString
//===========================================================================================================================

static OSStatus	OutputFormatFromArgString( const char *inArgString, OutputFormatType *outFormat )
{
	OSStatus				err;
	OutputFormatType		format;
	
	format = (OutputFormatType) CLIArgToValue( "format", inArgString, &err,
		kOutputFormatStr_JSON,		kOutputFormatType_JSON,
		kOutputFormatStr_XML,		kOutputFormatType_XML,
		kOutputFormatStr_Binary,	kOutputFormatType_Binary,
		NULL );
	if( outFormat ) *outFormat = format;
	return( err );
}

//===========================================================================================================================
//	OutputPropertyList
//===========================================================================================================================

static OSStatus	OutputPropertyList( CFPropertyListRef inPList, OutputFormatType inType, const char *inOutputFilePath )
{
	OSStatus		err;
	CFDataRef		results = NULL;
	FILE *			file	= NULL;
	
	// Convert plist to a specific format.
	
	switch( inType )
	{
		case kOutputFormatType_JSON:
			results = CFCreateJSONData( inPList, kJSONFlags_None, NULL );
			require_action( results, exit, err = kUnknownErr );
			break;
		
		case kOutputFormatType_XML:
			results = CFPropertyListCreateData( NULL, inPList, kCFPropertyListXMLFormat_v1_0, 0, NULL );
			require_action( results, exit, err = kUnknownErr );
			break;
		
		case kOutputFormatType_Binary:
			results = CFPropertyListCreateData( NULL, inPList, kCFPropertyListBinaryFormat_v1_0, 0, NULL );
			require_action( results, exit, err = kUnknownErr );
			break;
		
		default:
			err = kTypeErr;
			goto exit;
	}
	
	// Write formatted results to file or stdout.
	
	if( inOutputFilePath )
	{
		file = fopen( inOutputFilePath, "wb" );
		err = map_global_value_errno( file, file );
		require_noerr( err, exit );
	}
	else
	{
		file = stdout;
	}
	
	err = WriteANSIFile( file, CFDataGetBytePtr( results ), (size_t) CFDataGetLength( results ) );
	require_noerr_quiet( err, exit );
	
	// Write a trailing newline for JSON-formatted results.
	
	if( inType == kOutputFormatType_JSON )
	{
		err = WriteANSIFile( file, "\n", 1 );
		require_noerr_quiet( err, exit );
	}
	
exit:
	if( file && ( file != stdout ) ) fclose( file );
	CFReleaseNullSafe( results );
	return( err );
}

//===========================================================================================================================
//	CreateSRVRecordDataFromString
//===========================================================================================================================

static OSStatus	CreateSRVRecordDataFromString( const char *inString, uint8_t **outPtr, size_t *outLen )
{
	OSStatus			err;
	DataBuffer			dataBuf;
	const char *		ptr;
	int					i;
	uint8_t *			end;
	uint8_t				target[ kDomainNameLengthMax ];
	
	DataBuffer_Init( &dataBuf, NULL, 0, ( 3 * 2 ) + kDomainNameLengthMax );
	
	// Parse and set the priority, weight, and port values (all three are unsigned 16-bit values).
	
	ptr = inString;
	for( i = 0; i < 3; ++i )
	{
		char *		next;
		long		value;
		uint8_t		buf[ 2 ];
		
		value = strtol( ptr, &next, 0 );
		require_action_quiet( ( next != ptr ) && ( *next == ',' ), exit, err = kMalformedErr );
		require_action_quiet( ( value >= 0 ) && ( value <= UINT16_MAX ), exit, err = kRangeErr );
		ptr = next + 1;
		
		WriteBig16( buf, value );
		
		err = DataBuffer_Append( &dataBuf, buf, sizeof( buf ) );
		require_noerr( err, exit );
	}
	
	// Set the target domain name.
	
	err = DomainNameFromString( target, ptr, &end );
    require_noerr_quiet( err, exit );
	
	err = DataBuffer_Append( &dataBuf, target, (size_t)( end - target ) );
	require_noerr( err, exit );
	
	err = DataBuffer_Detach( &dataBuf, outPtr, outLen );
	require_noerr( err, exit );
	
exit:
	DataBuffer_Free( &dataBuf );
	return( err );
}

//===========================================================================================================================
//	CreateTXTRecordDataFromString
//===========================================================================================================================

static OSStatus	CreateTXTRecordDataFromString(const char *inString, int inDelimiter, uint8_t **outPtr, size_t *outLen )
{
	OSStatus			err;
	DataBuffer			dataBuf;
	const char *		src;
	uint8_t				txtStr[ 256 ];	// Buffer for single TXT string: 1 length byte + up to 255 bytes of data.
	
	DataBuffer_Init( &dataBuf, NULL, 0, kDNSRecordDataLengthMax );
	
	src = inString;
	for( ;; )
	{
		uint8_t *					dst = &txtStr[ 1 ];
		const uint8_t * const		lim = &txtStr[ 256 ];
		int							c;
		
		while( *src && ( *src != inDelimiter ) )
		{
			if( ( c = *src++ ) == '\\' )
			{
				require_action_quiet( *src != '\0', exit, err = kUnderrunErr );
				c = *src++;
			}
			require_action_quiet( dst < lim, exit, err = kOverrunErr );
			*dst++ = (uint8_t) c;
		}
		txtStr[ 0 ] = (uint8_t)( dst - &txtStr[ 1 ] );
		err = DataBuffer_Append( &dataBuf, txtStr, 1 + txtStr[ 0 ] );
		require_noerr( err, exit );
		
		if( *src == '\0' ) break;
		++src;
	}
	
	err = DataBuffer_Detach( &dataBuf, outPtr, outLen );
	require_noerr( err, exit );
	
exit:
	DataBuffer_Free( &dataBuf );
	return( err );
}

//===========================================================================================================================
//	CreateNSECRecordData
//===========================================================================================================================

DECLARE_QSORT_NUMERIC_COMPARATOR( _QSortCmpUnsigned );
DEFINE_QSORT_NUMERIC_COMPARATOR( unsigned int, _QSortCmpUnsigned )

#define kNSECBitmapMaxLength		32	// 32 bytes (256 bits). See <https://tools.ietf.org/html/rfc4034#section-4.1.2>.

static OSStatus
	CreateNSECRecordData(
		const uint8_t *	inNextDomainName,
		uint8_t **		outPtr,
		size_t *		outLen,
		unsigned int	inTypeCount,
		... )
{
	OSStatus			err;
	va_list				args;
	DataBuffer			rdataDB;
	unsigned int *		array	= NULL;
	unsigned int		i, type, maxBit, currBlock, bitmapLen;
	uint8_t				fields[ 2 + kNSECBitmapMaxLength ];
	uint8_t * const		bitmap	= &fields[ 2 ];
	
	va_start( args, inTypeCount );
	DataBuffer_Init( &rdataDB, NULL, 0, kDNSRecordDataLengthMax );
	
	// Append Next Domain Name.
	
	err = DataBuffer_Append( &rdataDB, inNextDomainName, DomainNameLength( inNextDomainName ) );
	require_noerr( err, exit );
	
	// Append Type Bit Maps.
	
	maxBit = 0;
	memset( bitmap, 0, kNSECBitmapMaxLength );
	if( inTypeCount > 0 )
	{
		array = (unsigned int *) malloc( inTypeCount * sizeof_element( array ) );
		require_action( array, exit, err = kNoMemoryErr );
		
		for( i = 0; i < inTypeCount; ++i )
		{
			type = va_arg( args, unsigned int );
			require_action_quiet( type <= UINT16_MAX, exit, err = kRangeErr );
			array[ i ] = type;
		}
		qsort( array, inTypeCount, sizeof_element( array ), _QSortCmpUnsigned );
		
		currBlock = array[ 0 ] / 256;
		for( i = 0; i < inTypeCount; ++i )
		{
			const unsigned int		block	= array[ i ] / 256;
			const unsigned int		bit		= array[ i ] % 256;
			
			if( block != currBlock )
			{
				bitmapLen	= BitArray_MaxBytes( maxBit + 1 );
				fields[ 0 ] = (uint8_t) currBlock;
				fields[ 1 ] = (uint8_t) bitmapLen;
				
				err = DataBuffer_Append( &rdataDB, fields, 2 + bitmapLen );
				require_noerr( err, exit );
				
				maxBit		= 0;
				currBlock	= block;
				memset( bitmap, 0, bitmapLen );
			}
			BitArray_SetBit( bitmap, bit );
			if( bit > maxBit ) maxBit = bit;
		}
	}
	else
	{
		currBlock = 0;
	}
	
	bitmapLen	= BitArray_MaxBytes( maxBit + 1 );
	fields[ 0 ] = (uint8_t) currBlock;
	fields[ 1 ] = (uint8_t) bitmapLen;
	
	err = DataBuffer_Append( &rdataDB, fields, 2 + bitmapLen );
	require_noerr( err, exit );
	
	err = DataBuffer_Detach( &rdataDB, outPtr, outLen );
	require_noerr( err, exit );
	
exit:
	va_end( args );
	DataBuffer_Free( &rdataDB );
	FreeNullSafe( array );
	return( err );
}

//===========================================================================================================================
//	AppendSOARecord
//===========================================================================================================================

static OSStatus
	_AppendSOARecordData(
		DataBuffer *	inDB,
		const uint8_t *	inMName,
		const uint8_t *	inRName,
		uint32_t		inSerial,
		uint32_t		inRefresh,
		uint32_t		inRetry,
		uint32_t		inExpire,
		uint32_t		inMinimumTTL,
		size_t *		outLen );

static OSStatus
	AppendSOARecord(
		DataBuffer *	inDB,
		const uint8_t *	inNamePtr,
		size_t			inNameLen,
		uint16_t		inType,
		uint16_t		inClass,
		uint32_t		inTTL,
		const uint8_t *	inMName,
		const uint8_t *	inRName,
		uint32_t		inSerial,
		uint32_t		inRefresh,
		uint32_t		inRetry,
		uint32_t		inExpire,
		uint32_t		inMinimumTTL,
		size_t *		outLen )
{
	OSStatus		err;
	size_t			rdataLen;
	size_t			rdlengthOffset = 0;
	uint8_t *		rdlengthPtr;
	
	if( inDB )
	{
		err = _DataBuffer_AppendDNSRecord( inDB, inNamePtr, inNameLen, inType, inClass, inTTL, NULL, 0 );
		require_noerr( err, exit );
		
		rdlengthOffset = DataBuffer_GetLen( inDB ) - 2;
	}
	
	err = _AppendSOARecordData( inDB, inMName, inRName, inSerial, inRefresh, inRetry, inExpire, inMinimumTTL, &rdataLen );
	require_noerr( err, exit );
	
	if( inDB )
	{
		rdlengthPtr = DataBuffer_GetPtr( inDB ) + rdlengthOffset;
		WriteBig16( rdlengthPtr, rdataLen );
	}
	
	if( outLen ) *outLen = inNameLen + sizeof( dns_fixed_fields_record ) + rdataLen;
	err = kNoErr;
	
exit:
	return( err );
}

static OSStatus
	_AppendSOARecordData(
		DataBuffer *	inDB,
		const uint8_t *	inMName,
		const uint8_t *	inRName,
		uint32_t		inSerial,
		uint32_t		inRefresh,
		uint32_t		inRetry,
		uint32_t		inExpire,
		uint32_t		inMinimumTTL,
		size_t *		outLen )
{
	OSStatus					err;
	dns_fixed_fields_soa		fields;
	const size_t				mnameLen = DomainNameLength( inMName );
	const size_t				rnameLen = DomainNameLength( inRName );
	
	if( inDB )
	{
		err = DataBuffer_Append( inDB, inMName, mnameLen );
		require_noerr( err, exit );
		
		err = DataBuffer_Append( inDB, inRName, rnameLen );
		require_noerr( err, exit );
		
		dns_fixed_fields_soa_init( &fields, inSerial, inRefresh, inRetry, inExpire, inMinimumTTL );
		err = DataBuffer_Append( inDB, &fields, sizeof( fields ) );
		require_noerr( err, exit );
	}
	if( outLen ) *outLen = mnameLen + rnameLen + sizeof( fields );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	CreateSOARecordData
//===========================================================================================================================

static OSStatus
	CreateSOARecordData(
		const uint8_t *	inMName,
		const uint8_t *	inRName,
		uint32_t		inSerial,
		uint32_t		inRefresh,
		uint32_t		inRetry,
		uint32_t		inExpire,
		uint32_t		inMinimumTTL,
		uint8_t **		outPtr,
		size_t *		outLen )
{
	OSStatus		err;
	DataBuffer		rdataDB;
	
	DataBuffer_Init( &rdataDB, NULL, 0, kDNSRecordDataLengthMax );
	
	err = _AppendSOARecordData( &rdataDB, inMName, inRName, inSerial, inRefresh, inRetry, inExpire, inMinimumTTL, NULL );
	require_noerr( err, exit );
	
	err = DataBuffer_Detach( &rdataDB, outPtr, outLen );
	require_noerr( err, exit );
	
exit:
	DataBuffer_Free( &rdataDB );
	return( err );
}

//===========================================================================================================================
//	_DataBuffer_AppendDNSQuestion
//===========================================================================================================================

static OSStatus
	_DataBuffer_AppendDNSQuestion(
		DataBuffer *	inDB,
		const uint8_t *	inNamePtr,
		size_t			inNameLen,
		uint16_t		inType,
		uint16_t		inClass )
{
	OSStatus						err;
	dns_fixed_fields_question		fields;
	
	err = DataBuffer_Append( inDB, inNamePtr, inNameLen );
	require_noerr( err, exit );
	
	dns_fixed_fields_question_init( &fields, inType, inClass );
	err = DataBuffer_Append( inDB, &fields, sizeof( fields ) );
	require_noerr( err, exit );
	
exit:
	return( err );
}

//===========================================================================================================================
//	_DataBuffer_AppendDNSRecord
//===========================================================================================================================

static OSStatus
	_DataBuffer_AppendDNSRecord(
		DataBuffer *	inDB,
		const uint8_t *	inNamePtr,
		size_t			inNameLen,
		uint16_t		inType,
		uint16_t		inClass,
		uint32_t		inTTL,
		const uint8_t *	inRDataPtr,
		size_t			inRDataLen )
{
	OSStatus					err;
	dns_fixed_fields_record		fields;
	
	require_action_quiet( inRDataLen < kDNSRecordDataLengthMax, exit, err = kSizeErr );
	
	err = DataBuffer_Append( inDB, inNamePtr, inNameLen );
	require_noerr( err, exit );
	
	dns_fixed_fields_record_init( &fields, inType, inClass, inTTL, (uint16_t) inRDataLen );
	err = DataBuffer_Append( inDB, &fields, sizeof( fields ) );
	require_noerr( err, exit );
	
	if( inRDataPtr )
	{
		err = DataBuffer_Append( inDB, inRDataPtr, inRDataLen );
		require_noerr( err, exit );
	}
	
exit:
	return( err );
}

//===========================================================================================================================
//	_NanoTime64ToTimestamp
//===========================================================================================================================

static char *	_NanoTime64ToTimestamp( NanoTime64 inTime, char *inBuf, size_t inMaxLen )
{
	struct  timeval		tv;
	
	NanoTimeToTimeVal( inTime, &tv );
	return( MakeFractionalDateString( &tv, inBuf, inMaxLen ) );
}

//===========================================================================================================================
//	_MDNSInterfaceListCreate
//===========================================================================================================================

static Boolean	_MDNSInterfaceIsBlacklisted( SocketRef inInfoSock, const char *inIfName );

static OSStatus	_MDNSInterfaceListCreate( MDNSInterfaceSubset inSubset, size_t inItemSize, MDNSInterfaceItem **outList )
{
	OSStatus					err;
	struct ifaddrs *			ifaList;
	const struct ifaddrs *		ifa;
	MDNSInterfaceItem *			interfaceList;
	MDNSInterfaceItem **		ptr;
	SocketRef					infoSock;
	
	ifaList			= NULL;
	interfaceList	= NULL;
	infoSock		= kInvalidSocketRef;
	if( inItemSize == 0 ) inItemSize = sizeof( MDNSInterfaceItem );
	require_action_quiet( inItemSize >= sizeof( MDNSInterfaceItem ), exit, err = kSizeErr );
	
	infoSock = socket( AF_INET, SOCK_DGRAM, 0 );
	err = map_socket_creation_errno( infoSock );
	require_noerr( err, exit );
	
	err = getifaddrs( &ifaList );
	err = map_global_noerr_errno( err );
	require_noerr( err, exit );
	
	ptr = &interfaceList;
	for( ifa = ifaList; ifa; ifa = ifa->ifa_next )
	{
		MDNSInterfaceItem *		item;
		int						family;
		const unsigned int		flagsMask	= IFF_UP | IFF_MULTICAST | IFF_POINTOPOINT;
		const unsigned int		flagsNeeded	= IFF_UP | IFF_MULTICAST;
		
		if( ( ifa->ifa_flags & flagsMask ) != flagsNeeded )		continue;
		if( !ifa->ifa_addr || !ifa->ifa_name )					continue;
		family = ifa->ifa_addr->sa_family;
		if( ( family != AF_INET ) && ( family != AF_INET6 ) )	continue;
		
		for( item = interfaceList; item && ( strcmp( item->ifName, ifa->ifa_name ) != 0 ); item = item->next ) {}
		if( !item )
		{
			NetTransportType		type;
			uint32_t				ifIndex;
			const char * const		ifName = ifa->ifa_name;
			
			if( _MDNSInterfaceIsBlacklisted( infoSock, ifName ) ) continue;
			err = SocketGetInterfaceInfo( infoSock, ifName, NULL, &ifIndex, NULL, NULL, NULL, NULL, NULL, &type );
			require_noerr( err, exit );
			
			if( ifIndex == 0 ) continue;
			if( type == kNetTransportType_AWDL )
			{
				if( inSubset == kMDNSInterfaceSubset_NonAWDL ) continue;
			}
			else
			{
				if( inSubset == kMDNSInterfaceSubset_AWDL ) continue;
			}
			item = (MDNSInterfaceItem *) calloc( 1, inItemSize );
			require_action( item, exit, err = kNoMemoryErr );
			
			*ptr =  item;
			 ptr = &item->next;
			
			item->ifName = strdup( ifName );
			require_action( item->ifName, exit, err = kNoMemoryErr );
			
			item->ifIndex = ifIndex;
			if(      type == kNetTransportType_AWDL ) item->isAWDL = true;
			else if( type == kNetTransportType_WiFi ) item->isWiFi = true;
		}
		if( family == AF_INET )	item->hasIPv4 = true;
		else					item->hasIPv6 = true;
	}
	require_action_quiet( interfaceList, exit, err = kNotFoundErr );
	
	if( outList )
	{
		*outList = interfaceList;
		interfaceList = NULL;
	}
	
exit:
	if( ifaList ) freeifaddrs( ifaList );
	_MDNSInterfaceListFree( interfaceList );
	ForgetSocket( &infoSock );
	return( err );
}

static Boolean	_MDNSInterfaceIsBlacklisted( SocketRef inInfoSock, const char *inIfName )
{
	OSStatus						err;
	int								i;
	static const char * const		kMDNSInterfacePrefixBlacklist[] = { "llw" };
	struct ifreq					ifr;
	
	// Check if the interface name's prefix matches the prefix blacklist.
	
	for( i = 0; i < (int) countof( kMDNSInterfacePrefixBlacklist ); ++i )
	{
		const char * const		prefix	= kMDNSInterfacePrefixBlacklist[ i ];
		
        if( strcmp_prefix( inIfName, prefix ) == 0 )
		{
			const char *		ptr = &inIfName[ strlen( prefix ) ];
			
			while( isdigit_safe( *ptr ) ) ++ptr;
			if( *ptr == '\0' ) return( true );
		}
	}
	
	// Check if the interface is used for inter-(co)processor networking.
	
	memset( &ifr, 0, sizeof( ifr ) );
	strlcpy( ifr.ifr_name, inIfName, sizeof( ifr.ifr_name ) );
	err = ioctl( inInfoSock, SIOCGIFFUNCTIONALTYPE, &ifr );
	err = map_global_value_errno( err != -1, err );
	if( !err && ( ifr.ifr_functional_type == IFRTYPE_FUNCTIONAL_INTCOPROC ) ) return( true );
	
	return( false );
}

//===========================================================================================================================
//	_MDNSInterfaceListFree
//===========================================================================================================================

static void	_MDNSInterfaceListFree( MDNSInterfaceItem *inList )
{
	MDNSInterfaceItem *		item;
	
	while( ( item = inList ) != NULL )
	{
		inList = item->next;
		FreeNullSafe( item->ifName );
		free( item );
	}
}

//===========================================================================================================================
//	_MDNSInterfaceGetAny
//===========================================================================================================================

static OSStatus	_MDNSInterfaceGetAny( MDNSInterfaceSubset inSubset, char inNameBuf[ IF_NAMESIZE + 1 ], uint32_t *outIndex )
{
	OSStatus						err;
	MDNSInterfaceItem *				list;
	const MDNSInterfaceItem *		item;
	
	list = NULL;
	err = _MDNSInterfaceListCreate( inSubset, 0, &list );
	require_noerr_quiet( err, exit );
	require_action_quiet( list, exit, err = kNotFoundErr );
	
	for( item = list; item; item = item->next )
	{
		if( item->hasIPv4 && item->hasIPv6 ) break;
	}
	if( !item ) item = list;
	if( inNameBuf )	strlcpy( inNameBuf, item->ifName, IF_NAMESIZE + 1 );
	if( outIndex ) *outIndex = item->ifIndex;
	
exit:
	_MDNSInterfaceListFree( list );
	return( err );
}

//===========================================================================================================================
//	_SetComputerName
//===========================================================================================================================

static OSStatus	_SetComputerName( CFStringRef inComputerName, CFStringEncoding inEncoding )
{
	OSStatus				err;
	SCPreferencesRef		prefs;
	Boolean					ok;
	
	prefs = SCPreferencesCreateWithAuthorization( NULL, CFSTR( kDNSSDUtilIdentifier ), NULL, NULL );
	err = map_scerror( prefs );
	require_noerr_quiet( err, exit );
	
	ok = SCPreferencesSetComputerName( prefs, inComputerName, inEncoding );
	err = map_scerror( ok );
	require_noerr_quiet( err, exit );
	
	ok = SCPreferencesCommitChanges( prefs );
	err = map_scerror( ok );
	require_noerr_quiet( err, exit );
	
	ok = SCPreferencesApplyChanges( prefs );
	err = map_scerror( ok );
	require_noerr_quiet( err, exit );
	
exit:
	CFReleaseNullSafe( prefs );
	return( err );
}

//===========================================================================================================================
//	_SetComputerNameWithUTF8CString
//===========================================================================================================================

static OSStatus	_SetComputerNameWithUTF8CString( const char *inComputerName )
{
	OSStatus		err;
	CFStringRef		computerName;
	
	computerName = CFStringCreateWithCString( NULL, inComputerName, kCFStringEncodingUTF8 );
	require_action( computerName, exit, err = kNoMemoryErr );
	
	err = _SetComputerName( computerName, kCFStringEncodingUTF8 );
	require_noerr_quiet( err, exit );
	
exit:
	CFReleaseNullSafe( computerName );
	return( err );
}

//===========================================================================================================================
//	_SetLocalHostName
//===========================================================================================================================

static OSStatus	_SetLocalHostName( CFStringRef inLocalHostName )
{
	OSStatus				err;
	SCPreferencesRef		prefs;
	Boolean					ok;
	
	prefs = SCPreferencesCreateWithAuthorization( NULL, CFSTR( kDNSSDUtilIdentifier ), NULL, NULL );
	err = map_scerror( prefs );
	require_noerr_quiet( err, exit );
	
	ok = SCPreferencesSetLocalHostName( prefs, inLocalHostName );
	err = map_scerror( ok );
	require_noerr_quiet( err, exit );
	
	ok = SCPreferencesCommitChanges( prefs );
	err = map_scerror( ok );
	require_noerr_quiet( err, exit );
	
	ok = SCPreferencesApplyChanges( prefs );
	err = map_scerror( ok );
	require_noerr_quiet( err, exit );
	
exit:
	CFReleaseNullSafe( prefs );
	return( err );
}

//===========================================================================================================================
//	_SetLocalHostNameWithUTF8CString
//===========================================================================================================================

static OSStatus	_SetLocalHostNameWithUTF8CString( const char *inLocalHostName )
{
	OSStatus		err;
	CFStringRef		localHostName;
	
	localHostName = CFStringCreateWithCString( NULL, inLocalHostName, kCFStringEncodingUTF8 );
	require_action( localHostName, exit, err = kNoMemoryErr );
	
	err = _SetLocalHostName( localHostName );
	require_noerr_quiet( err, exit );
	
exit:
	CFReleaseNullSafe( localHostName );
	return( err );
}

//===========================================================================================================================
//	MDNSColliderCreate
//===========================================================================================================================

typedef enum
{
	kMDNSColliderOpCode_Invalid			= 0,
	kMDNSColliderOpCode_Send			= 1,
	kMDNSColliderOpCode_Wait			= 2,
	kMDNSColliderOpCode_SetProbeActions	= 3,
	kMDNSColliderOpCode_LoopPush		= 4,
	kMDNSColliderOpCode_LoopPop			= 5,
	kMDNSColliderOpCode_Exit			= 6
	
}	MDNSColliderOpCode;

typedef struct
{
	MDNSColliderOpCode		opcode;
	uint32_t				operand;
	
}	MDNSCInstruction;

#define kMaxLoopDepth		16

struct MDNSColliderPrivate
{
	CFRuntimeBase					base;							// CF object base.
	dispatch_queue_t				queue;							// Queue for collider's events.
	dispatch_source_t				readSourceV4;					// Read dispatch source for IPv4 socket.
	dispatch_source_t				readSourceV6;					// Read dispatch source for IPv6 socket.
	SocketRef						sockV4;							// IPv4 UDP socket for mDNS.
	SocketRef						sockV6;							// IPv6 UDP socket for mDNS.
	uint8_t *						target;							// Record name being targeted. (malloced)
	uint8_t *						responsePtr;					// Response message pointer. (malloced)
	size_t							responseLen;					// Response message length.
	uint8_t *						probePtr;						// Probe query message pointer. (malloced)
	size_t							probeLen;						// Probe query message length.
	unsigned int					probeCount;						// Count of probe queries received for collider's record.
	uint32_t						probeActionMap;					// Bitmap of actions to take for 
	MDNSCInstruction *				program;						// Program to execute.
	uint32_t						pc;								// Program's program counter.
	uint32_t						loopCounts[ kMaxLoopDepth ];	// Stack of loop counters.
	uint32_t						loopDepth;						// Current loop depth.
	dispatch_source_t				waitTimer;						// Timer for program's wait commands.
	uint32_t						interfaceIndex;					// Interface over which to send and receive mDNS msgs.
	MDNSColliderStopHandler_f		stopHandler;					// User's stop handler.
	void *							stopContext;					// User's stop handler context.
	MDNSColliderProtocols			protocols;						// Protocols to use, i.e., IPv4, IPv6.
	Boolean							stopped;						// True if the collider has been stopped.
	uint8_t							msgBuf[ kMDNSMessageSizeMax ];	// mDNS message buffer.
};

static void		_MDNSColliderStop( MDNSColliderRef inCollider, OSStatus inError );
static void		_MDNSColliderReadHandler( void *inContext );
static void		_MDNSColliderExecuteProgram( void *inContext );
static OSStatus	_MDNSColliderSendResponse( MDNSColliderRef inCollider, SocketRef inSock, const struct sockaddr *inDest );
static OSStatus	_MDNSColliderSendProbe( MDNSColliderRef inCollider, SocketRef inSock, const struct sockaddr *inDest );

CF_CLASS_DEFINE( MDNSCollider );

ulog_define_ex( kDNSSDUtilIdentifier, MDNSCollider, kLogLevelInfo, kLogFlags_None, "MDNSCollider", NULL );
#define mc_ulog( LEVEL, ... )		ulog( &log_category_from_name( MDNSCollider ), (LEVEL), __VA_ARGS__ )

static OSStatus	MDNSColliderCreate( dispatch_queue_t inQueue, MDNSColliderRef *outCollider )
{
	OSStatus			err;
	MDNSColliderRef		obj = NULL;
	
	CF_OBJECT_CREATE( MDNSCollider, obj, err, exit );
	
	ReplaceDispatchQueue( &obj->queue, inQueue );
	obj->sockV4 = kInvalidSocketRef;
	obj->sockV6 = kInvalidSocketRef;
	
	*outCollider = obj;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_MDNSColliderFinalize
//===========================================================================================================================

static void	_MDNSColliderFinalize( CFTypeRef inObj )
{
	MDNSColliderRef const		me = (MDNSColliderRef) inObj;
	
	check( !me->waitTimer );
	check( !me->readSourceV4 );
	check( !me->readSourceV6 );
	check( !IsValidSocket( me->sockV4 ) );
	check( !IsValidSocket( me->sockV6 ) );
	ForgetMem( &me->target );
	ForgetMem( &me->responsePtr );
	ForgetMem( &me->probePtr );
	ForgetMem( &me->program );
	dispatch_forget( &me->queue );
}

//===========================================================================================================================
//	MDNSColliderStart
//===========================================================================================================================

static void	_MDNSColliderStart( void *inContext );

static OSStatus	MDNSColliderStart( MDNSColliderRef me )
{
	OSStatus		err;
	
	require_action_quiet( me->target,         exit, err = kNotPreparedErr );
	require_action_quiet( me->responsePtr,    exit, err = kNotPreparedErr );
	require_action_quiet( me->probePtr,       exit, err = kNotPreparedErr );
	require_action_quiet( me->program,        exit, err = kNotPreparedErr );
	require_action_quiet( me->interfaceIndex, exit, err = kNotPreparedErr );
	require_action_quiet( me->protocols,      exit, err = kNotPreparedErr );
	
	CFRetain( me );
	dispatch_async_f( me->queue, me, _MDNSColliderStart );
	err = kNoErr;
	
exit:
	return( err );
}

static void	_MDNSColliderStart( void *inContext )
{
	OSStatus					err;
	MDNSColliderRef const		me		= (MDNSColliderRef) inContext;
	SocketRef					sock	= kInvalidSocketRef;
	SocketContext *				sockCtx	= NULL;
	
	if( me->protocols & kMDNSColliderProtocol_IPv4 )
	{
		err = CreateMulticastSocket( GetMDNSMulticastAddrV4(), kMDNSPort, NULL, me->interfaceIndex, true, NULL, &sock );
		require_noerr( err, exit );
		
		err = SocketContextCreate( sock, me, &sockCtx );
		require_noerr( err, exit );
		sock = kInvalidSocketRef;
		
		err = DispatchReadSourceCreate( sockCtx->sock, me->queue, _MDNSColliderReadHandler, SocketContextCancelHandler,
			sockCtx, &me->readSourceV4 );
		require_noerr( err, exit );
		me->sockV4 = sockCtx->sock;
		sockCtx = NULL;
		
		dispatch_resume( me->readSourceV4 );
	}
	
	if( me->protocols & kMDNSColliderProtocol_IPv6 )
	{
		err = CreateMulticastSocket( GetMDNSMulticastAddrV6(), kMDNSPort, NULL, me->interfaceIndex, true, NULL, &sock );
		require_noerr( err, exit );
		
		err = SocketContextCreate( sock, me, &sockCtx );
		require_noerr( err, exit );
		sock = kInvalidSocketRef;
		
		err = DispatchReadSourceCreate( sockCtx->sock, me->queue, _MDNSColliderReadHandler, SocketContextCancelHandler,
			sockCtx, &me->readSourceV6 );
		require_noerr( err, exit );
		me->sockV6 = sockCtx->sock;
		sockCtx = NULL;
		
		dispatch_resume( me->readSourceV6 );
	}
	
	_MDNSColliderExecuteProgram( me );
	err = kNoErr;
	
exit:
	ForgetSocket( &sock );
	ForgetSocketContext( &sockCtx );
	if( err ) _MDNSColliderStop( me, err );
}

//===========================================================================================================================
//	MDNSColliderStop
//===========================================================================================================================

static void	_MDNSColliderUserStop( void *inContext );

static void	MDNSColliderStop( MDNSColliderRef me )
{
	CFRetain( me );
	dispatch_async_f( me->queue, me, _MDNSColliderUserStop );
}

static void	_MDNSColliderUserStop( void *inContext )
{
	MDNSColliderRef const		me = (MDNSColliderRef) inContext;
	
	_MDNSColliderStop( me, kCanceledErr );
	CFRelease( me );
}

//===========================================================================================================================
//	MDNSColliderSetProtocols
//===========================================================================================================================

static void	MDNSColliderSetProtocols( MDNSColliderRef me, MDNSColliderProtocols inProtocols )
{
	me->protocols = inProtocols;
}

//===========================================================================================================================
//	MDNSColliderSetInterfaceIndex
//===========================================================================================================================

static void	MDNSColliderSetInterfaceIndex( MDNSColliderRef me, uint32_t inInterfaceIndex )
{
	me->interfaceIndex = inInterfaceIndex;
}

//===========================================================================================================================
//	MDNSColliderSetProgram
//===========================================================================================================================

#define kMDNSColliderProgCmd_Done		"done"
#define kMDNSColliderProgCmd_Loop		"loop"
#define kMDNSColliderProgCmd_Send		"send"
#define kMDNSColliderProgCmd_Probes		"probes"
#define kMDNSColliderProgCmd_Wait		"wait"

typedef uint32_t		MDNSColliderProbeAction;

#define kMDNSColliderProbeAction_None					0
#define kMDNSColliderProbeAction_Respond				1
#define kMDNSColliderProbeAction_RespondUnicast			2
#define kMDNSColliderProbeAction_RespondMulticast		3
#define kMDNSColliderProbeAction_Probe					4
#define kMDNSColliderProbeAction_MaxValue				kMDNSColliderProbeAction_Probe

#define kMDNSColliderProbeActionBits_Count			3
#define kMDNSColliderProbeActionBits_Mask			( ( 1U << kMDNSColliderProbeActionBits_Count ) - 1 )
#define kMDNSColliderProbeActionMaxProbeCount		( 32 / kMDNSColliderProbeActionBits_Count )

check_compile_time( kMDNSColliderProbeAction_MaxValue <= kMDNSColliderProbeActionBits_Mask );

static OSStatus	_MDNSColliderParseProbeActionString( const char *inString, size_t inLen, uint32_t *outBitmap );

static OSStatus	MDNSColliderSetProgram( MDNSColliderRef me, const char *inProgramStr )
{
	OSStatus				err;
	uint32_t				insCount;
	unsigned int			loopDepth;
	const char *			cmd;
	const char *			end;
	const char *			next;
	MDNSCInstruction *		program = NULL;
	uint32_t				loopStart[ kMaxLoopDepth ];
	
	insCount = 0;
	for( cmd = inProgramStr; *cmd; cmd = next )
	{
		for( end = cmd; *end && ( *end != ';' ); ++end ) {}
		require_action_quiet( end != cmd, exit, err = kMalformedErr );
		next = ( *end == ';' ) ? ( end + 1 ) : end;
		++insCount;
	}
	
	program = (MDNSCInstruction *) calloc( insCount + 1, sizeof( *program ) );
	require_action( program, exit, err = kNoMemoryErr );
	
	insCount	= 0;
	loopDepth	= 0;
	for( cmd = inProgramStr; *cmd; cmd = next )
	{
		size_t							cmdLen;
		const char *					ptr;
		const char *					arg;
		size_t							argLen;
		uint32_t						value;
		MDNSCInstruction * const		ins = &program[ insCount ];
		
		while( isspace_safe( *cmd ) ) ++cmd;
		for( end = cmd; *end && ( *end != ';' ); ++end ) {}
		next = ( *end == ';' ) ? ( end + 1 ) : end;
		
		for( ptr = cmd; ( ptr < end ) && !isspace_safe( *ptr ); ++ptr ) {}
		cmdLen = (size_t)( ptr - cmd );
		
		// Done statement
		
		if( strnicmpx( cmd, cmdLen, kMDNSColliderProgCmd_Done ) == 0 )
		{
			while( ( ptr < end ) && isspace_safe( *ptr ) ) ++ptr;
			require_action_quiet( ptr == end, exit, err = kMalformedErr );
			
			require_action_quiet( loopDepth > 0, exit, err = kMalformedErr );
			
			ins->opcode		= kMDNSColliderOpCode_LoopPop;
			ins->operand	= loopStart[ --loopDepth ];
		}
		
		// Loop command
		
		else if( strnicmpx( cmd, cmdLen, kMDNSColliderProgCmd_Loop ) == 0 )
		{
			for( arg = ptr; ( arg < end ) && isspace_safe( *arg ); ++arg ) {}
			err = DecimalTextToUInt32( arg, end, &value, &ptr );
			require_noerr_quiet( err, exit );
			require_action_quiet( value > 0, exit, err = kValueErr );
			
			while( ( ptr < end ) && isspace_safe( *ptr ) ) ++ptr;
			require_action_quiet( ptr == end, exit, err = kMalformedErr );
			
			ins->opcode 	= kMDNSColliderOpCode_LoopPush;
			ins->operand	= value;
			
			require_action_quiet( loopDepth < kMaxLoopDepth, exit, err = kNoSpaceErr );
			loopStart[ loopDepth++ ] = insCount + 1;
		}
		
		// Probes command
		
		else if( strnicmpx( cmd, cmdLen, kMDNSColliderProgCmd_Probes ) == 0 )
		{
			for( arg = ptr; ( arg < end ) &&  isspace_safe( *arg ); ++arg ) {}
			for( ptr = arg; ( ptr < end ) && !isspace_safe( *ptr ); ++ptr ) {}
			argLen = (size_t)( ptr - arg );
			if( argLen > 0 )
			{
				err = _MDNSColliderParseProbeActionString( arg, argLen, &value );
				require_noerr_quiet( err, exit );
			}
			else
			{
				value = 0;
			}
			
			while( ( ptr < end ) && isspace_safe( *ptr ) ) ++ptr;
			require_action_quiet( ptr == end, exit, err = kMalformedErr );
			
			ins->opcode 	= kMDNSColliderOpCode_SetProbeActions;
			ins->operand	= value;
		}
		
		// Send command
		
		else if( strnicmpx( cmd, cmdLen, kMDNSColliderProgCmd_Send ) == 0 )
		{
			while( ( ptr < end ) && isspace_safe( *ptr ) ) ++ptr;
			require_action_quiet( ptr == end, exit, err = kMalformedErr );
			
			ins->opcode = kMDNSColliderOpCode_Send;
		}
		
		// Wait command
		
		else if( strnicmpx( cmd, cmdLen, kMDNSColliderProgCmd_Wait ) == 0 )
		{
			for( arg = ptr; ( arg < end ) && isspace_safe( *arg ); ++arg ) {}
			err = DecimalTextToUInt32( arg, end, &value, &ptr );
			require_noerr_quiet( err, exit );
			
			while( ( ptr < end ) && isspace_safe( *ptr ) ) ++ptr;
			require_action_quiet( ptr == end, exit, err = kMalformedErr );
			
			ins->opcode		= kMDNSColliderOpCode_Wait;
			ins->operand	= value;
		}
		
		// Unrecognized command
		
		else
		{
			err = kCommandErr;
			goto exit;
		}
		++insCount;
	}
	require_action_quiet( loopDepth == 0, exit, err = kMalformedErr );
	
	program[ insCount ].opcode = kMDNSColliderOpCode_Exit;
	
	FreeNullSafe( me->program );
	me->program = program;
	program = NULL;
	err = kNoErr;
	
exit:
	FreeNullSafe( program );
	return( err );
}

static OSStatus	_MDNSColliderParseProbeActionString( const char *inString, size_t inLen, uint32_t *outBitmap )
{
	OSStatus				err;
	const char *			ptr;
	const char * const		end = &inString[ inLen ];
	uint32_t				bitmap;
	int						index;
	
	bitmap	= 0;
	index	= 0;
	ptr		= inString;
	while( ptr < end )
	{
		int							c, count;
		MDNSColliderProbeAction		action;
		
		c = *ptr++;
		if( isdigit_safe( c ) )
		{
			count = 0;
			do
			{
				count = ( count * 10 ) + ( c - '0' );
				require_action_quiet( count <= ( kMDNSColliderProbeActionMaxProbeCount - index ), exit, err = kCountErr );
				require_action_quiet( ptr < end, exit, err = kUnderrunErr );
				c = *ptr++;
				
			}	while( isdigit_safe( c ) );
			require_action_quiet( count > 0, exit, err = kCountErr );
		}
		else
		{
			require_action_quiet( index < kMDNSColliderProbeActionMaxProbeCount, exit, err = kMalformedErr );
			count = 1;
		}
		
		switch( c )
		{
			case 'n':	action = kMDNSColliderProbeAction_None;				break;
			case 'r':	action = kMDNSColliderProbeAction_Respond;			break;
			case 'u':	action = kMDNSColliderProbeAction_RespondUnicast;	break;
			case 'm':	action = kMDNSColliderProbeAction_RespondMulticast;	break;
			case 'p':	action = kMDNSColliderProbeAction_Probe;			break;
			default:	err = kMalformedErr;								goto exit;
		}
		if( ptr < end )
		{
			c = *ptr++;
			require_action_quiet( ( c == '-' ) && ( ptr < end ), exit, err = kMalformedErr );
		}
		while( count-- > 0 )
		{
			bitmap |= ( action << ( index * kMDNSColliderProbeActionBits_Count ) );
			++index;
		}
	}
	
	*outBitmap = bitmap;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	MDNSColliderSetStopHandler
//===========================================================================================================================

static void	MDNSColliderSetStopHandler( MDNSColliderRef me, MDNSColliderStopHandler_f inStopHandler, void *inStopContext )
{
	me->stopHandler = inStopHandler;
	me->stopContext = inStopContext;
}

//===========================================================================================================================
//	MDNSColliderSetRecord
//===========================================================================================================================

#define kMDNSColliderDummyStr			"\x16" "mdnscollider-sent-this" "\x05" "local"
#define kMDNSColliderDummyName			( (const uint8_t *) kMDNSColliderDummyStr )
#define kMDNSColliderDummyNameLen		sizeof( kMDNSColliderDummyStr )

static OSStatus
	MDNSColliderSetRecord(
		MDNSColliderRef	me,
		const uint8_t *	inName,
		uint16_t		inType,
		const void *	inRDataPtr,
		size_t			inRDataLen )
{
	OSStatus		err;
	DataBuffer		msgDB;
	DNSHeader		header;
	uint8_t *		targetPtr	= NULL;
	size_t			targetLen;
	uint8_t *		responsePtr	= NULL;
	size_t			responseLen;
	uint8_t *		probePtr	= NULL;
	size_t			probeLen;
	
	DataBuffer_Init( &msgDB, NULL, 0, kMDNSMessageSizeMax );
	
	err = DomainNameDup( inName, &targetPtr, &targetLen );
	require_noerr_quiet( err, exit );
	
	// Create response message.
	
	memset( &header, 0, sizeof( header ) );
	DNSHeaderSetFlags( &header, kDNSHeaderFlag_Response | kDNSHeaderFlag_AuthAnswer );
	DNSHeaderSetAnswerCount( &header, 1 );
	
	err = DataBuffer_Append( &msgDB, &header, sizeof( header ) );
	require_noerr( err, exit );
	
	err = _DataBuffer_AppendDNSRecord( &msgDB, targetPtr, targetLen, inType, kDNSServiceClass_IN | kRRClassCacheFlushBit,
		1976, inRDataPtr, inRDataLen );
	require_noerr( err, exit );
	
	err = DataBuffer_Detach( &msgDB, &responsePtr, &responseLen );
	require_noerr( err, exit );
	
	// Create probe message.
	
	memset( &header, 0, sizeof( header ) );
	DNSHeaderSetQuestionCount( &header, 2 );
	DNSHeaderSetAuthorityCount( &header, 1 );
	
	err = DataBuffer_Append( &msgDB, &header, sizeof( header ) );
	require_noerr( err, exit );
	
	err = _DataBuffer_AppendDNSQuestion( &msgDB, targetPtr, targetLen, kDNSServiceType_ANY, kDNSServiceClass_IN );
	require_noerr( err, exit );
	
	err = _DataBuffer_AppendDNSQuestion( &msgDB, kMDNSColliderDummyName, kMDNSColliderDummyNameLen,
		kDNSServiceType_NULL, kDNSServiceClass_IN );
	require_noerr( err, exit );
	
	err = _DataBuffer_AppendDNSRecord( &msgDB, targetPtr, targetLen, inType, kDNSServiceClass_IN,
		1976, inRDataPtr, inRDataLen );
	require_noerr( err, exit );
	
	err = DataBuffer_Detach( &msgDB, &probePtr, &probeLen );
	require_noerr( err, exit );
	
	FreeNullSafe( me->target );
	me->target = targetPtr;
	targetPtr = NULL;
	
	FreeNullSafe( me->responsePtr );
	me->responsePtr = responsePtr;
	me->responseLen = responseLen;
	responsePtr = NULL;
	
	FreeNullSafe( me->probePtr );
	me->probePtr = probePtr;
	me->probeLen = probeLen;
	probePtr = NULL;
	
exit:
	DataBuffer_Free( &msgDB );
	FreeNullSafe( targetPtr );
	FreeNullSafe( responsePtr );
	FreeNullSafe( probePtr );
	return( err );
}

//===========================================================================================================================
//	_MDNSColliderStop
//===========================================================================================================================

static void	_MDNSColliderStop( MDNSColliderRef me, OSStatus inError )
{
	dispatch_source_forget( &me->waitTimer );
	dispatch_source_forget( &me->readSourceV4 );
	dispatch_source_forget( &me->readSourceV6 );
	me->sockV4 = kInvalidSocketRef;
	me->sockV6 = kInvalidSocketRef;
	
	if( !me->stopped )
	{
		me->stopped = true;
		if( me->stopHandler ) me->stopHandler( me->stopContext, inError );
		CFRelease( me );
	}
}

//===========================================================================================================================
//	_MDNSColliderReadHandler
//===========================================================================================================================

static MDNSColliderProbeAction	_MDNSColliderGetProbeAction( uint32_t inBitmap, unsigned int inProbeNumber );
static const char *				_MDNSColliderProbeActionToString( MDNSColliderProbeAction inAction );

static void	_MDNSColliderReadHandler( void *inContext )
{
	OSStatus					err;
	struct timeval				now;
	SocketContext * const		sockCtx	= (SocketContext *) inContext;
	MDNSColliderRef const		me		= (MDNSColliderRef) sockCtx->userContext;
	size_t						msgLen;
	sockaddr_ip					sender;
	const DNSHeader *			hdr;
	const uint8_t *				ptr;
	const struct sockaddr *		dest;
	int							probeFound, probeIsQU;
	unsigned int				qCount, i;
	MDNSColliderProbeAction		action;
	
	gettimeofday( &now, NULL );
	
	err = SocketRecvFrom( sockCtx->sock, me->msgBuf, sizeof( me->msgBuf ), &msgLen, &sender, sizeof( sender ),
		NULL, NULL, NULL, NULL );
	require_noerr( err, exit );
	
	require_quiet( msgLen >= kDNSHeaderLength, exit );
	hdr = (const DNSHeader *) me->msgBuf;
	
	probeFound	= false;
	probeIsQU	= false;
	qCount = DNSHeaderGetQuestionCount( hdr );
	ptr = (const uint8_t *) &hdr[ 1 ];
	for( i = 0; i < qCount; ++i )
	{
		uint16_t		qtype, qclass;
		uint8_t			qname[ kDomainNameLengthMax ];
		
		err = DNSMessageExtractQuestion( me->msgBuf, msgLen, ptr, qname, &qtype, &qclass, &ptr );
		require_noerr_quiet( err, exit );
		
		if( ( qtype == kDNSServiceType_NULL ) && ( qclass == kDNSServiceClass_IN ) &&
			DomainNameEqual( qname, kMDNSColliderDummyName ) )
		{
			probeFound = false;
			break;
		}
		
		if( qtype != kDNSServiceType_ANY ) continue;
		if( ( qclass & ~kQClassUnicastResponseBit ) != kDNSServiceClass_IN ) continue;
		if( !DomainNameEqual( qname, me->target ) ) continue;
		
		if( !probeFound )
		{
			probeFound	= true;
			probeIsQU	= ( qclass & kQClassUnicastResponseBit ) ? true : false;
		}
	}
	require_quiet( probeFound, exit );
	
	++me->probeCount;
	action = _MDNSColliderGetProbeAction( me->probeActionMap, me->probeCount );
	
	mc_ulog( kLogLevelInfo, "Received probe from %##a at %{du:time} (action: %s):\n\n%#1{du:dnsmsg}",
		&sender, &now, _MDNSColliderProbeActionToString( action ), me->msgBuf, msgLen );
	
	if( ( action == kMDNSColliderProbeAction_Respond )			||
		( action == kMDNSColliderProbeAction_RespondUnicast )	||
		( action == kMDNSColliderProbeAction_RespondMulticast ) )
	{
		if( ( ( action == kMDNSColliderProbeAction_Respond ) && probeIsQU ) ||
			(   action == kMDNSColliderProbeAction_RespondUnicast ) )
		{
			dest = &sender.sa;
		}
		else if( ( ( action == kMDNSColliderProbeAction_Respond ) && !probeIsQU ) ||
				 (   action == kMDNSColliderProbeAction_RespondMulticast ) )
		{
			dest = ( sender.sa.sa_family == AF_INET ) ? GetMDNSMulticastAddrV4() : GetMDNSMulticastAddrV6();
		}
		
		err = _MDNSColliderSendResponse( me, sockCtx->sock, dest );
		require_noerr( err, exit );
	}
	else if( action == kMDNSColliderProbeAction_Probe )
	{
		dest = ( sender.sa.sa_family == AF_INET ) ? GetMDNSMulticastAddrV4() : GetMDNSMulticastAddrV6();
		
		err = _MDNSColliderSendProbe( me, sockCtx->sock, dest );
		require_noerr( err, exit );
	}
	
exit:
	return;
}

static MDNSColliderProbeAction	_MDNSColliderGetProbeAction( uint32_t inBitmap, unsigned int inProbeNumber )
{
	MDNSColliderProbeAction		action;
	
	if( ( inProbeNumber >= 1 ) && ( inProbeNumber <= kMDNSColliderProbeActionMaxProbeCount ) )
	{
		action = ( inBitmap >> ( ( inProbeNumber - 1 ) * kMDNSColliderProbeActionBits_Count ) ) &
			kMDNSColliderProbeActionBits_Mask;
	}
	else
	{
		action = kMDNSColliderProbeAction_None;
	}
	return( action );
}

static const char *	_MDNSColliderProbeActionToString( MDNSColliderProbeAction inAction )
{
	switch( inAction )
	{
		case kMDNSColliderProbeAction_None:				return( "None" );
		case kMDNSColliderProbeAction_Respond:			return( "Respond" );
		case kMDNSColliderProbeAction_RespondUnicast:	return( "Respond (unicast)" );
		case kMDNSColliderProbeAction_RespondMulticast:	return( "Respond (multicast)" );
		case kMDNSColliderProbeAction_Probe:			return( "Probe" );
		default:										return( "???" );
	}
}

//===========================================================================================================================
//	_MDNSColliderExecuteProgram
//===========================================================================================================================

static void	_MDNSColliderExecuteProgram( void *inContext )
{
	OSStatus					err;
	MDNSColliderRef const		me = (MDNSColliderRef) inContext;
	int							stop;
	
	dispatch_forget( &me->waitTimer );
	
	stop = false;
	for( ;; )
	{
		const MDNSCInstruction * const		ins = &me->program[ me->pc++ ];
		uint32_t							waitMs;
		
		switch( ins->opcode )
		{
			case kMDNSColliderOpCode_Send:
				if( IsValidSocket( me->sockV4 ) )
				{
					err = _MDNSColliderSendResponse( me, me->sockV4, GetMDNSMulticastAddrV4() );
					require_noerr( err, exit );
				}
				if( IsValidSocket( me->sockV6 ) )
				{
					err = _MDNSColliderSendResponse( me, me->sockV6, GetMDNSMulticastAddrV6() );
					require_noerr( err, exit );
				}
				break;
			
			case kMDNSColliderOpCode_Wait:
				waitMs = ins->operand;
				if( waitMs > 0 )
				{
					err = DispatchTimerOneShotCreate( dispatch_time_milliseconds( waitMs ), 1, me->queue,
						_MDNSColliderExecuteProgram, me, &me->waitTimer );
					require_noerr( err, exit );
					dispatch_resume( me->waitTimer );
					goto exit;
				}
				break;
			
			case kMDNSColliderOpCode_SetProbeActions:
				me->probeCount		= 0;
				me->probeActionMap	= ins->operand;
				break;
			
			case kMDNSColliderOpCode_LoopPush:
				check( me->loopDepth < kMaxLoopDepth );
				me->loopCounts[ me->loopDepth++ ] = ins->operand;
				break;
			
			case kMDNSColliderOpCode_LoopPop:
				check( me->loopDepth > 0 );
				if( --me->loopCounts[ me->loopDepth - 1 ] > 0 )
				{
					me->pc = ins->operand;
				}
				else
				{
					--me->loopDepth;
				}
				break;
			
			case kMDNSColliderOpCode_Exit:
				stop = true;
				err	= kNoErr;
				goto exit;
			
			default:
				dlogassert( "Unhandled opcode %u\n", ins->opcode );
				err = kCommandErr;
				goto exit;
		}
	}
	
exit:
	if( err || stop ) _MDNSColliderStop( me, err );
}

//===========================================================================================================================
//	_MDNSColliderSendResponse
//===========================================================================================================================

static OSStatus	_MDNSColliderSendResponse( MDNSColliderRef me, SocketRef inSock, const struct sockaddr *inDest )
{
	OSStatus		err;
	ssize_t			n;
	
	n = sendto( inSock, (char *) me->responsePtr, me->responseLen, 0, inDest, SockAddrGetSize( inDest ) );
	err = map_socket_value_errno( inSock, n == (ssize_t) me->responseLen, n );
	return( err );
}

//===========================================================================================================================
//	_MDNSColliderSendProbe
//===========================================================================================================================

static OSStatus	_MDNSColliderSendProbe( MDNSColliderRef me, SocketRef inSock, const struct sockaddr *inDest )
{
	OSStatus		err;
	ssize_t			n;
	
	n = sendto( inSock, (char *) me->probePtr, me->probeLen, 0, inDest, SockAddrGetSize( inDest ) );
	err = map_socket_value_errno( inSock, n == (ssize_t) me->probeLen, n );
	return( err );
}

//===========================================================================================================================
//	ServiceBrowserCreate
//===========================================================================================================================

typedef struct SBDomain					SBDomain;
typedef struct SBServiceType			SBServiceType;
typedef struct SBServiceBrowse			SBServiceBrowse;
typedef struct SBServiceInstance		SBServiceInstance;
typedef struct SBIPAddress				SBIPAddress;

struct ServiceBrowserPrivate
{
	CFRuntimeBase					base;				// CF object base.
	dispatch_queue_t				queue;				// Queue for service browser's events.
	DNSServiceRef					connection;			// Shared connection for DNS-SD ops.
	DNSServiceRef					domainsQuery;		// Query for recommended browsing domains.
	char *							domain;				// If non-null, then browsing is limited to this domain.
	StringListItem *				serviceTypeList;	// If non-null, then browsing is limited to these service types.
	ServiceBrowserCallback_f		userCallback;		// User's callback. Called when browsing stops.
	void *							userContext;		// User's callback context.
	SBDomain *						domainList;			// List of domains and their browse results.
	dispatch_source_t				stopTimer;			// Timer to stop browsing after browseTimeSecs.
	uint32_t						ifIndex;			// If non-zero, then browsing is limited to this interface.
	unsigned int					browseTimeSecs;		// Amount of time to spend browsing in seconds.
	Boolean							includeAWDL;		// True if the IncludeAWDL flag should be used for DNS-SD ops that
														// use the "any" interface.
};

struct SBDomain
{
	SBDomain *				next;			// Next domain object in list.
	ServiceBrowserRef		browser;		// Pointer to parent service browser.
	char *					name;			// Name of the domain.
	DNSServiceRef			servicesQuery;	// Query for services (_services._dns-sd._udp.<domain> PTR record) in domain.
	SBServiceType *			typeList;		// List of service types to browse for in this domain.
};

struct SBServiceType
{
	SBServiceType *			next;		// Next service type object in list.
	char *					name;		// Name of the service type.
	SBServiceBrowse *		browseList;	// List of browses for this service type.
};

struct SBServiceBrowse
{
	SBServiceBrowse *		next;			// Next browse object in list.
	ServiceBrowserRef		browser;		// Pointer to parent service browser.
	DNSServiceRef			browse;			// Reference to DNSServiceBrowse op.
	SBServiceInstance *		instanceList;	// List of service instances that were discovered by this browse.
	uint64_t				startTicks;		// Value of UpTicks() when the browse op began.
	uint32_t				ifIndex;		// If non-zero, then the browse is limited to this interface.
};

struct SBServiceInstance
{
	SBServiceInstance *		next;				// Next service instance object in list.
	ServiceBrowserRef		browser;			// Pointer to parent service browser.
	char *					name;				// Name of the service instance.
	char *					fqdn;				// Fully qualified domain name of service instance (for logging/debugging).
	uint32_t				ifIndex;			// Index of interface over which this service instance was discovered.
	uint64_t				discoverTimeUs;		// Time it took to discover this service instance in microseconds.
	DNSServiceRef			resolve;			// Reference to DNSServiceResolve op for this service instance.
	uint64_t				resolveStartTicks;	// Value of UpTicks() when the DNSServiceResolve op began.
	uint64_t				resolveTimeUs;		// Time it took to resolve this service instance.
	char *					hostname;			// Service instance's hostname. Result of DNSServiceResolve.
	uint16_t				port;				// Service instance's port number. Result of DNSServiceResolve.
	uint8_t *				txtPtr;				// Service instance's TXT record data. Result of DNSServiceResolve.
	size_t					txtLen;				// Length of service instance's TXT record data.
	DNSServiceRef			getAddrInfo;		// Reference to DNSServiceGetAddrInfo op for service instance's hostname.
	uint64_t				gaiStartTicks;		// Value of UpTicks() when the DNSServiceGetAddrInfo op began.
	SBIPAddress *			ipaddrList;			// List of IP addresses that the hostname resolved to.
};

struct SBIPAddress
{
	SBIPAddress *		next;			// Next IP address object in list.
	sockaddr_ip			sip;			// IPv4 or IPv6 address.
	uint64_t			resolveTimeUs;	// Time it took to resolve this IP address in microseconds.
};

typedef struct
{
	SBRDomain *		domainList;	// List of domains in which services were found.
	int32_t			refCount;	// This object's reference count.
	
}	ServiceBrowserResultsPrivate;

static void		_ServiceBrowserStop( ServiceBrowserRef me, OSStatus inError );
static OSStatus	_ServiceBrowserAddDomain( ServiceBrowserRef inBrowser, const char *inDomain );
static OSStatus	_ServiceBrowserRemoveDomain( ServiceBrowserRef inBrowser, const char *inName );
static void		_ServiceBrowserTimerHandler( void *inContext );
static void DNSSD_API
	_ServiceBrowserDomainsQueryCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		uint16_t				inType,
		uint16_t				inClass,
		uint16_t				inRDataLen,
		const void *			inRDataPtr,
		uint32_t				inTTL,
		void *					inContext );
static void DNSSD_API
	_ServiceBrowserServicesQueryCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		uint16_t				inType,
		uint16_t				inClass,
		uint16_t				inRDataLen,
		const void *			inRDataPtr,
		uint32_t				inTTL,
		void *					inContext );
static void DNSSD_API
	_ServiceBrowserBrowseCallback(
		DNSServiceRef		inSDRef,
		DNSServiceFlags		inFlags,
		uint32_t			inInterfaceIndex,
		DNSServiceErrorType	inError,
		const char *		inName,
		const char *		inRegType,
		const char *		inDomain,
		void *				inContext );
static void DNSSD_API
	_ServiceBrowserResolveCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		const char *			inHostname,
		uint16_t				inPort,
		uint16_t				inTXTLen,
		const unsigned char *	inTXTPtr,
		void *					inContext );
static void DNSSD_API
	_ServiceBrowserGAICallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inHostname,
		const struct sockaddr *	inSockAddr,
		uint32_t				inTTL,
		void *					inContext );
static OSStatus
	_ServiceBrowserAddServiceType(
		ServiceBrowserRef	inBrowser,
		SBDomain *			inDomain,
		const char *		inName,
		uint32_t			inIfIndex );
static OSStatus
	_ServiceBrowserRemoveServiceType(
		ServiceBrowserRef	inBrowser,
		SBDomain *			inDomain,
		const char *		inName,
		uint32_t			inIfIndex );
static OSStatus
	_ServiceBrowserAddServiceInstance(
		ServiceBrowserRef	inBrowser,
		SBServiceBrowse *	inBrowse,
		uint32_t			inIfIndex,
		const char *		inName,
		const char *		inRegType,
		const char *		inDomain,
		uint64_t			inDiscoverTimeUs );
static OSStatus
	_ServiceBrowserRemoveServiceInstance(
		ServiceBrowserRef	inBrowser,
		SBServiceBrowse *	inBrowse,
		const char *		inName,
		uint32_t			inIfIndex );
static OSStatus
	_ServiceBrowserAddIPAddress(
		ServiceBrowserRef		inBrowser,
		SBServiceInstance *		inInstance,
		const struct sockaddr *	inSockAddr,
		uint64_t				inResolveTimeUs );
static OSStatus
	_ServiceBrowserRemoveIPAddress(
		ServiceBrowserRef		inBrowser,
		SBServiceInstance *		inInstance,
		const struct sockaddr *	inSockAddr );
static OSStatus	_ServiceBrowserCreateResults( ServiceBrowserRef me, ServiceBrowserResults **outResults );
static OSStatus	_SBDomainCreate( const char *inName, ServiceBrowserRef inBrowser, SBDomain **outDomain );
static void		_SBDomainFree( SBDomain *inDomain );
static OSStatus	_SBServiceTypeCreate( const char *inName, SBServiceType **outType );
static void		_SBServiceTypeFree( SBServiceType *inType );
static OSStatus	_SBServiceBrowseCreate( uint32_t inIfIndex, ServiceBrowserRef inBrowser, SBServiceBrowse **outBrowse );
static void		_SBServiceBrowseFree( SBServiceBrowse *inBrowse );
static OSStatus
	_SBServiceInstanceCreate(
		const char *			inName,
		const char *			inType,
		const char *			inDomain,
		uint32_t				inIfIndex,
		uint64_t				inDiscoverTimeUs,
		ServiceBrowserRef		inBrowser,
		SBServiceInstance **	outInstance );
static void		_SBServiceInstanceFree( SBServiceInstance *inInstance );
static OSStatus
	_SBIPAddressCreate(
		const struct sockaddr *	inSockAddr,
		uint64_t				inResolveTimeUs,
		SBIPAddress **			outIPAddress );
static void		_SBIPAddressFree( SBIPAddress *inIPAddress );
static void		_SBIPAddressFreeList( SBIPAddress *inList );
static OSStatus	_SBRDomainCreate( const char *inName, SBRDomain **outDomain );
static void		_SBRDomainFree( SBRDomain *inDomain );
static OSStatus	_SBRServiceTypeCreate( const char *inName, SBRServiceType **outType );
static void		_SBRServiceTypeFree( SBRServiceType *inType );
static OSStatus
	_SBRServiceInstanceCreate(
		const char *			inName,
		uint32_t				inInterfaceIndex,
		const char *			inHostname,
		uint16_t				inPort,
		const uint8_t *			inTXTPtr,
		size_t					inTXTLen,
		uint64_t				inDiscoverTimeUs,
		uint64_t				inResolveTimeUs,
		SBRServiceInstance **	outInstance );
static void		_SBRServiceInstanceFree( SBRServiceInstance *inInstance );
static OSStatus
	_SBRIPAddressCreate(
		const struct sockaddr *	inSockAddr,
		uint64_t				inResolveTimeUs,
		SBRIPAddress **			outIPAddress );
static void		_SBRIPAddressFree( SBRIPAddress *inIPAddress );

#define ForgetSBIPAddressList( X )		ForgetCustom( X, _SBIPAddressFreeList )

CF_CLASS_DEFINE( ServiceBrowser );

ulog_define_ex( kDNSSDUtilIdentifier, ServiceBrowser, kLogLevelTrace, kLogFlags_None, "ServiceBrowser", NULL );
#define sb_ulog( LEVEL, ... )		ulog( &log_category_from_name( ServiceBrowser ), (LEVEL), __VA_ARGS__ )

static OSStatus
	ServiceBrowserCreate(
		dispatch_queue_t	inQueue,
		uint32_t			inInterfaceIndex,
		const char *		inDomain,
		unsigned int		inBrowseTimeSecs,
		Boolean				inIncludeAWDL,
		ServiceBrowserRef *	outBrowser )
{
	OSStatus				err;
	ServiceBrowserRef		obj;
	
	CF_OBJECT_CREATE( ServiceBrowser, obj, err, exit );
	
	ReplaceDispatchQueue( &obj->queue, inQueue );
	obj->ifIndex		= inInterfaceIndex;
	if( inDomain )
	{
		obj->domain = strdup( inDomain );
		require_action( obj->domain, exit, err = kNoMemoryErr );
	}
	obj->browseTimeSecs	= inBrowseTimeSecs;
	obj->includeAWDL	= inIncludeAWDL;
	
	*outBrowser = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	CFReleaseNullSafe( obj );
	return( err );
}

//===========================================================================================================================
//	_ServiceBrowserFinalize
//===========================================================================================================================

static void	_ServiceBrowserFinalize( CFTypeRef inObj )
{
	ServiceBrowserRef const		me = (ServiceBrowserRef) inObj;
	StringListItem *			serviceType;
	
	dispatch_forget( &me->queue );
	check( !me->connection );
	check( !me->domainsQuery );
	ForgetMem( &me->domain );
	while( ( serviceType = me->serviceTypeList ) != NULL )
	{
		me->serviceTypeList = serviceType->next;
		ForgetMem( &serviceType->str );
		free( serviceType );
	}
	check( !me->domainList );
	check( !me->stopTimer );
}

//===========================================================================================================================
//	ServiceBrowserStart
//===========================================================================================================================

static void	_ServiceBrowserStart( void *inContext );

static void	ServiceBrowserStart( ServiceBrowserRef me )
{
	CFRetain( me );
	dispatch_async_f( me->queue, me, _ServiceBrowserStart );
}

static void	_ServiceBrowserStart( void *inContext )
{
	OSStatus					err;
	ServiceBrowserRef const		me = (ServiceBrowserRef) inContext;
	
	err = DNSServiceCreateConnection( &me->connection );
	require_noerr( err, exit );
	
	err = DNSServiceSetDispatchQueue( me->connection, me->queue );
	require_noerr( err, exit );
	
	if( me->domain )
	{
		err = _ServiceBrowserAddDomain( me, me->domain );
		require_noerr( err, exit );
	}
	else
	{
		DNSServiceRef			sdRef;
		const char * const		recordName	= "b._dns-sd._udp.local.";
		const uint32_t			ifIndex		= kDNSServiceInterfaceIndexLocalOnly;
		
		// Perform PTR meta-query for "b._dns-sd._udp.local." to enumerate recommended browsing domains.
		// See <https://tools.ietf.org/html/rfc6763#section-11>.
		
		sb_ulog( kLogLevelTrace, "Starting PTR QueryRecord on interface %d for %s", (int32_t) ifIndex, recordName );
		
		sdRef = me->connection;
		err = DNSServiceQueryRecord( &sdRef, kDNSServiceFlagsShareConnection, ifIndex, recordName,
			kDNSServiceType_PTR, kDNSServiceClass_IN, _ServiceBrowserDomainsQueryCallback, me );
		require_noerr( err, exit );
		
		me->domainsQuery = sdRef;
	}
	
	err = DispatchTimerCreate( dispatch_time_seconds( me->browseTimeSecs ), DISPATCH_TIME_FOREVER,
		100 * kNanosecondsPerMillisecond, me->queue, _ServiceBrowserTimerHandler, NULL, me, &me->stopTimer );
	require_noerr( err, exit );
	dispatch_resume( me->stopTimer );
	
exit:
	if( err ) _ServiceBrowserStop( me, err );
}

//===========================================================================================================================
//	ServiceBrowserAddServiceType
//===========================================================================================================================

static OSStatus	ServiceBrowserAddServiceType( ServiceBrowserRef me, const char *inServiceType )
{
	OSStatus				err;
	StringListItem *		item;
	StringListItem **		itemPtr;
	StringListItem *		newItem = NULL;
	
	for( itemPtr = &me->serviceTypeList; ( item = *itemPtr ) != NULL; itemPtr = &item->next )
	{
		if( strcmp( item->str, inServiceType ) == 0 ) break;
	}
	if( !item )
	{
		newItem = (StringListItem *) calloc( 1, sizeof( *newItem ) );
		require_action( newItem, exit, err = kNoMemoryErr );
		
		newItem->str = strdup( inServiceType );
		require_action( newItem->str, exit, err = kNoMemoryErr );
		
		*itemPtr = newItem;
		newItem = NULL;
	}
	err = kNoErr;
	
exit:
	FreeNullSafe( newItem );
	return( err );
}

//===========================================================================================================================
//	ServiceBrowserSetCallback
//===========================================================================================================================

static void	ServiceBrowserSetCallback( ServiceBrowserRef me, ServiceBrowserCallback_f inCallback, void *inContext )
{
	me->userCallback	= inCallback;
	me->userContext		= inContext;
}

//===========================================================================================================================
//	ServiceBrowserResultsRetain
//===========================================================================================================================

static void	ServiceBrowserResultsRetain( ServiceBrowserResults *inResults )
{
	ServiceBrowserResultsPrivate * const		results = (ServiceBrowserResultsPrivate *) inResults;
	
	atomic_add_32( &results->refCount, 1 );
}

//===========================================================================================================================
//	ServiceBrowserResultsRelease
//===========================================================================================================================

static void	ServiceBrowserResultsRelease( ServiceBrowserResults *inResults )
{
	ServiceBrowserResultsPrivate * const		results = (ServiceBrowserResultsPrivate *) inResults;
	SBRDomain *									domain;
	
	if( atomic_add_and_fetch_32( &results->refCount, -1 ) == 0 )
	{
		while( ( domain = inResults->domainList ) != NULL )
		{
			inResults->domainList = domain->next;
			_SBRDomainFree( domain );
		}
		free( inResults );
	}
}

//===========================================================================================================================
//	_ServiceBrowserStop
//===========================================================================================================================

static void	_ServiceBrowserStop( ServiceBrowserRef me, OSStatus inError )
{
	OSStatus				err;
	SBDomain *				d;
	SBServiceType *			t;
	SBServiceBrowse *		b;
	SBServiceInstance *		i;
	
	dispatch_source_forget( &me->stopTimer );
	DNSServiceForget( &me->domainsQuery );
	for( d = me->domainList; d; d = d->next )
	{
		DNSServiceForget( &d->servicesQuery );
		for( t = d->typeList; t; t = t->next )
		{
			for( b = t->browseList; b; b = b->next )
			{
				DNSServiceForget( &b->browse );
				for( i = b->instanceList; i; i = i->next )
				{
					DNSServiceForget( &i->resolve );
					DNSServiceForget( &i->getAddrInfo );
				}
			}
		}
	}
	DNSServiceForget( &me->connection );
	
	if( me->userCallback )
	{
		ServiceBrowserResults *		results = NULL;
		
		err = _ServiceBrowserCreateResults( me, &results );
		if( !err ) err = inError;
		
		me->userCallback( results, err, me->userContext );
		me->userCallback	= NULL;
		me->userContext		= NULL;
		if( results ) ServiceBrowserResultsRelease( results );
	}
	
	while( ( d = me->domainList ) != NULL )
	{
		me->domainList = d->next;
		_SBDomainFree( d );
	}
	CFRelease( me );
}

//===========================================================================================================================
//	_ServiceBrowserAddDomain
//===========================================================================================================================

static OSStatus	_ServiceBrowserAddDomain( ServiceBrowserRef me, const char *inDomain )
{
	OSStatus		err;
	SBDomain *		domain;
	SBDomain **		domainPtr;
	SBDomain *		newDomain = NULL;
	
	for( domainPtr = &me->domainList; ( domain = *domainPtr ) != NULL; domainPtr = &domain->next )
	{
		if( strcasecmp( domain->name, inDomain ) == 0 ) break;
	}
	require_action_quiet( !domain, exit, err = kDuplicateErr );
	
	err = _SBDomainCreate( inDomain, me, &newDomain );
	require_noerr_quiet( err, exit );
	
	if( me->serviceTypeList )
	{
		const StringListItem *		item;
		
		for( item = me->serviceTypeList; item; item = item->next )
		{
			err = _ServiceBrowserAddServiceType( me, newDomain, item->str, me->ifIndex );
			if( err == kDuplicateErr ) err = kNoErr;
			require_noerr( err, exit );
		}
	}
	else
	{
		char *				recordName;
		DNSServiceRef		sdRef;
		DNSServiceFlags		flags;
		
		// Perform PTR meta-query for _services._dns-sd._udp.<domain> to enumerate service types in domain.
		// See <https://tools.ietf.org/html/rfc6763#section-9>.
		
		ASPrintF( &recordName, "_services._dns-sd._udp.%s", newDomain->name );
		require_action( recordName, exit, err = kNoMemoryErr );
		
		flags = kDNSServiceFlagsShareConnection;
		if( ( me->ifIndex == kDNSServiceInterfaceIndexAny ) && me->includeAWDL ) flags |= kDNSServiceFlagsIncludeAWDL;
		
		sb_ulog( kLogLevelTrace, "Starting PTR QueryRecord on interface %d for %s", (int32_t) me->ifIndex, recordName );
		
		sdRef = newDomain->browser->connection;
		err = DNSServiceQueryRecord( &sdRef, flags, me->ifIndex, recordName, kDNSServiceType_PTR, kDNSServiceClass_IN,
			_ServiceBrowserServicesQueryCallback, newDomain );
		free( recordName );
		require_noerr( err, exit );
		
		newDomain->servicesQuery = sdRef;
	}
	
	*domainPtr	= newDomain;
	newDomain	= NULL;
	err = kNoErr;
	
exit:
	if( newDomain ) _SBDomainFree( newDomain );
	return( err );
}

//===========================================================================================================================
//	_ServiceBrowserRemoveDomain
//===========================================================================================================================

static OSStatus	_ServiceBrowserRemoveDomain( ServiceBrowserRef me, const char *inName )
{
	OSStatus		err;
	SBDomain *		domain;
	SBDomain **		domainPtr;
	
	for( domainPtr = &me->domainList; ( domain = *domainPtr ) != NULL; domainPtr = &domain->next )
	{
		if( strcasecmp( domain->name, inName ) == 0 ) break;
	}
	
	if( domain )
	{
		*domainPtr = domain->next;
		_SBDomainFree( domain );
		err = kNoErr;
	}
	else
	{
		err = kNotFoundErr;
	}
	
	return( err );
}

//===========================================================================================================================
//	_ServiceBrowserTimerHandler
//===========================================================================================================================

static void	_ServiceBrowserTimerHandler( void *inContext )
{
	ServiceBrowserRef const		me = (ServiceBrowserRef) inContext;
	
	_ServiceBrowserStop( me, kNoErr );
}

//===========================================================================================================================
//	_ServiceBrowserDomainsQueryCallback
//===========================================================================================================================

static void DNSSD_API
	_ServiceBrowserDomainsQueryCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		uint16_t				inType,
		uint16_t				inClass,
		uint16_t				inRDataLen,
		const void *			inRDataPtr,
		uint32_t				inTTL,
		void *					inContext )
{
	ServiceBrowserRef const		me = (ServiceBrowserRef) inContext;
	OSStatus					err;
	char						domainStr[ kDNSServiceMaxDomainName ];
	
	Unused( inSDRef );
	Unused( inClass );
	Unused( inTTL );
	
	sb_ulog( kLogLevelTrace, "QueryRecord result: %s on interface %d for %s -> %{du:rdata}%?{end} (error: %#m)",
		DNSServiceFlagsToAddRmvStr( inFlags ), (int32_t) inInterfaceIndex, inFullName, inType, inRDataPtr, inRDataLen,
		!inError, inError );
	
	require_noerr( inError, exit );
	
	err = DomainNameToString( inRDataPtr, ( (const uint8_t *) inRDataPtr ) + inRDataLen, domainStr, NULL );
	require_noerr( err, exit );
	
	if( inFlags & kDNSServiceFlagsAdd )
	{
		err = _ServiceBrowserAddDomain( me, domainStr );
		if( err == kDuplicateErr ) err = kNoErr;
		require_noerr( err, exit );
	}
	else
	{
		err = _ServiceBrowserRemoveDomain( me, domainStr );
		if( err == kNotFoundErr ) err = kNoErr;
		require_noerr( err, exit );
	}
	
exit:
	return;
}

//===========================================================================================================================
//	_ServiceBrowserServicesQueryCallback
//===========================================================================================================================

static void DNSSD_API
	_ServiceBrowserServicesQueryCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		uint16_t				inType,
		uint16_t				inClass,
		uint16_t				inRDataLen,
		const void *			inRDataPtr,
		uint32_t				inTTL,
		void *					inContext )
{
	OSStatus					err;
	SBDomain * const			domain	= (SBDomain *) inContext;
	ServiceBrowserRef const		me		= domain->browser;
	const uint8_t *				src;
	const uint8_t *				end;
	uint8_t *					dst;
	int							i;
	uint8_t						serviceType[ 2 * ( 1 + kDomainLabelLengthMax ) + 1 ];
	char						serviceTypeStr[ kDNSServiceMaxDomainName ];
	
	Unused( inSDRef );
	Unused( inTTL );
	Unused( inClass );
	
	sb_ulog( kLogLevelTrace, "QueryRecord result: %s on interface %d for %s -> %{du:rdata}%?{end} (error: %#m)",
		DNSServiceFlagsToAddRmvStr( inFlags ), (int32_t) inInterfaceIndex, inFullName, inType, inRDataPtr, inRDataLen,
		!inError, inError );
	
	require_noerr( inError, exit );
	
	check( inType  == kDNSServiceType_PTR );
	check( inClass == kDNSServiceClass_IN );
	
	// The first two labels of the domain name in the RDATA describe a service type.
	// See <https://tools.ietf.org/html/rfc6763#section-9>.
	
	src = (const uint8_t *) inRDataPtr;
	end = src + inRDataLen;
	dst = serviceType;
	for( i = 0; i < 2; ++i )
	{
		size_t		labelLen;
		
		require_action_quiet( ( end - src ) > 0, exit, err = kUnderrunErr );
		
		labelLen = *src;
		require_action_quiet( ( labelLen > 0 ) && ( labelLen <= kDomainLabelLengthMax ), exit, err = kMalformedErr );
		require_action_quiet( ( (size_t)( end - src ) ) >= ( 1 + labelLen ), exit, err = kUnderrunErr );
		
		memcpy( dst, src, 1 + labelLen );
		src += 1 + labelLen;
		dst += 1 + labelLen;
	}
	*dst = 0;
	
	err = DomainNameToString( serviceType, NULL, serviceTypeStr, NULL );
	require_noerr( err, exit );
	
	if( inFlags & kDNSServiceFlagsAdd )
	{
		err = _ServiceBrowserAddServiceType( me, domain, serviceTypeStr, inInterfaceIndex );
		if( err == kDuplicateErr ) err = kNoErr;
		require_noerr( err, exit );
	}
	else
	{
		err = _ServiceBrowserRemoveServiceType( me, domain, serviceTypeStr, inInterfaceIndex );
		if( err == kNotFoundErr ) err = kNoErr;
		require_noerr( err, exit );
	}
	
exit:
	return;
}

//===========================================================================================================================
//	_ServiceBrowserBrowseCallback
//===========================================================================================================================

static void DNSSD_API
	_ServiceBrowserBrowseCallback(
		DNSServiceRef		inSDRef,
		DNSServiceFlags		inFlags,
		uint32_t			inInterfaceIndex,
		DNSServiceErrorType	inError,
		const char *		inName,
		const char *		inRegType,
		const char *		inDomain,
		void *				inContext )
{
	OSStatus					err;
	const uint64_t				nowTicks	= UpTicks();
	SBServiceBrowse * const		browse		= (SBServiceBrowse *) inContext;
	ServiceBrowserRef const		me			= (ServiceBrowserRef) browse->browser;
	
	Unused( inSDRef );
	
	sb_ulog( kLogLevelTrace, "Browse result: %s on interface %d for %s.%s%s%?{end} (error: %#m)",
		DNSServiceFlagsToAddRmvStr( inFlags ), (int32_t) inInterfaceIndex, inName, inRegType, inDomain, !inError, inError );
	
	require_noerr( inError, exit );
	
	if( inFlags & kDNSServiceFlagsAdd )
	{
		err = _ServiceBrowserAddServiceInstance( me, browse, inInterfaceIndex, inName, inRegType, inDomain,
			UpTicksToMicroseconds( nowTicks - browse->startTicks ) );
		if( err == kDuplicateErr ) err = kNoErr;
		require_noerr( err, exit );
	}
	else
	{
		err = _ServiceBrowserRemoveServiceInstance( me, browse, inName, inInterfaceIndex );
		if( err == kNotFoundErr ) err = kNoErr;
		require_noerr( err, exit );
	}
	
exit:
	return;
}

//===========================================================================================================================
//	_ServiceBrowserResolveCallback
//===========================================================================================================================

static void DNSSD_API
	_ServiceBrowserResolveCallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inFullName,
		const char *			inHostname,
		uint16_t				inPort,
		uint16_t				inTXTLen,
		const unsigned char *	inTXTPtr,
		void *					inContext )
{
	OSStatus						err;
	const uint64_t					nowTicks	= UpTicks();
	SBServiceInstance * const		instance	= (SBServiceInstance *) inContext;
	ServiceBrowserRef const			me			= (ServiceBrowserRef) instance->browser;
	
	Unused( inSDRef );
	Unused( inFlags );
	
	sb_ulog( kLogLevelTrace, "Resolve result on interface %d for %s -> %s:%u %#{txt}%?{end} (error %#m)",
		(int32_t) inInterfaceIndex, inFullName, inHostname, inPort, inTXTPtr, (size_t) inTXTLen, !inError, inError );
	
	require_noerr( inError, exit );
	
	if( !MemEqual( instance->txtPtr, instance->txtLen, inTXTPtr, inTXTLen ) )
	{
		FreeNullSafe( instance->txtPtr );
		instance->txtPtr = _memdup( inTXTPtr, inTXTLen );
		require_action( instance->txtPtr, exit, err = kNoMemoryErr );
		
		instance->txtLen = inTXTLen;
	}
	
	instance->port = ntohs( inPort );
	
	if( !instance->hostname || ( strcasecmp( instance->hostname, inHostname ) != 0 ) )
	{
		DNSServiceRef		sdRef;
		
		if( !instance->hostname ) instance->resolveTimeUs = UpTicksToMicroseconds( nowTicks - instance->resolveStartTicks );
		
		err = ReplaceString( &instance->hostname, NULL, inHostname, kSizeCString );
		require_noerr( err, exit );
		
		DNSServiceForget( &instance->getAddrInfo );
		ForgetSBIPAddressList( &instance->ipaddrList );
		
		sb_ulog( kLogLevelTrace, "Starting GetAddrInfo on interface %d for %s",
			(int32_t) instance->ifIndex, instance->hostname );
		
		sdRef = me->connection;
		instance->gaiStartTicks = UpTicks();
		err = DNSServiceGetAddrInfo( &sdRef, kDNSServiceFlagsShareConnection, instance->ifIndex,
			kDNSServiceProtocol_IPv4 | kDNSServiceProtocol_IPv6, instance->hostname, _ServiceBrowserGAICallback, instance );
		require_noerr( err, exit );
		
		instance->getAddrInfo = sdRef;
	}
	
exit:
	return;
}

//===========================================================================================================================
//	_ServiceBrowserGAICallback
//===========================================================================================================================

static void DNSSD_API
	_ServiceBrowserGAICallback(
		DNSServiceRef			inSDRef,
		DNSServiceFlags			inFlags,
		uint32_t				inInterfaceIndex,
		DNSServiceErrorType		inError,
		const char *			inHostname,
		const struct sockaddr *	inSockAddr,
		uint32_t				inTTL,
		void *					inContext )
{
	OSStatus						err;
	const uint64_t					nowTicks	= UpTicks();
	SBServiceInstance * const		instance	= (SBServiceInstance *) inContext;
	ServiceBrowserRef const			me			= (ServiceBrowserRef) instance->browser;
	
	Unused( inSDRef );
	Unused( inTTL );
	
	sb_ulog( kLogLevelTrace, "GetAddrInfo result: %s on interface %d for (%s ->) %s -> %##a%?{end} (error: %#m)",
		DNSServiceFlagsToAddRmvStr( inFlags ), (int32_t) inInterfaceIndex, instance->fqdn, inHostname, inSockAddr,
		!inError, inError );
	
	require_noerr( inError, exit );
	
	if( ( inSockAddr->sa_family != AF_INET ) && ( inSockAddr->sa_family != AF_INET6 ) )
	{
		dlogassert( "Unexpected address family: %d", inSockAddr->sa_family );
		goto exit;
	}
	
	if( inFlags & kDNSServiceFlagsAdd )
	{
		err = _ServiceBrowserAddIPAddress( me, instance, inSockAddr,
			UpTicksToMicroseconds( nowTicks - instance->gaiStartTicks ) );
		if( err == kDuplicateErr ) err = kNoErr;
		require_noerr( err, exit );
	}
	else
	{
		err = _ServiceBrowserRemoveIPAddress( me, instance, inSockAddr );
		if( err == kNotFoundErr ) err = kNoErr;
		require_noerr( err, exit );
	}
	
exit:
	return;
}

//===========================================================================================================================
//	_ServiceBrowserAddServiceType
//===========================================================================================================================

static OSStatus
	_ServiceBrowserAddServiceType(
		ServiceBrowserRef	me,
		SBDomain *			inDomain,
		const char *		inName,
		uint32_t			inIfIndex )
{
	OSStatus				err;
	SBServiceType *			type;
	SBServiceType **		typePtr;
	SBServiceType *			newType		= NULL;
	SBServiceBrowse *		browse;
	SBServiceBrowse **		browsePtr;
	SBServiceBrowse *		newBrowse	= NULL;
	DNSServiceRef			sdRef;
	DNSServiceFlags			flags;
	
	for( typePtr = &inDomain->typeList; ( type = *typePtr ) != NULL; typePtr = &type->next )
	{
		if( strcasecmp( type->name, inName ) == 0 ) break;
	}
	if( !type )
	{
		err = _SBServiceTypeCreate( inName, &newType );
		require_noerr_quiet( err, exit );
		
		type = newType;
	}
	
	for( browsePtr = &type->browseList; ( browse = *browsePtr ) != NULL; browsePtr = &browse->next )
	{
		if( browse->ifIndex == inIfIndex ) break;
	}
	require_action_quiet( !browse, exit, err = kDuplicateErr );
	
	err = _SBServiceBrowseCreate( inIfIndex, me, &newBrowse );
	require_noerr_quiet( err, exit );
	
	flags = kDNSServiceFlagsShareConnection;
	if( ( newBrowse->ifIndex == kDNSServiceInterfaceIndexAny ) && me->includeAWDL ) flags |= kDNSServiceFlagsIncludeAWDL;
	
	sb_ulog( kLogLevelTrace, "Starting Browse on interface %d for %s%s",
		(int32_t) newBrowse->ifIndex, type->name, inDomain->name );
	
	sdRef = me->connection;
	newBrowse->startTicks = UpTicks();
	err = DNSServiceBrowse( &sdRef, flags, newBrowse->ifIndex, type->name, inDomain->name, _ServiceBrowserBrowseCallback,
		newBrowse );
	require_noerr( err, exit );
	
	newBrowse->browse = sdRef;
	*browsePtr	= newBrowse;
	newBrowse	= NULL;
	
	if( newType )
	{
		*typePtr	= newType;
		newType		= NULL;
	}
	
exit:
	if( newBrowse )	_SBServiceBrowseFree( newBrowse );
	if( newType )	_SBServiceTypeFree( newType );
	return( err );
}

//===========================================================================================================================
//	_ServiceBrowserRemoveServiceType
//===========================================================================================================================

static OSStatus
	_ServiceBrowserRemoveServiceType(
		ServiceBrowserRef	me,
		SBDomain *			inDomain,
		const char *		inName,
		uint32_t			inIfIndex )
{
	OSStatus				err;
	SBServiceType *			type;
	SBServiceType **		typePtr;
	SBServiceBrowse *		browse;
	SBServiceBrowse **		browsePtr;
	
	Unused( me );
	
	for( typePtr = &inDomain->typeList; ( type = *typePtr ) != NULL; typePtr = &type->next )
	{
		if( strcasecmp( type->name, inName ) == 0 ) break;
	}
	require_action_quiet( type, exit, err = kNotFoundErr );
	
	for( browsePtr = &type->browseList; ( browse = *browsePtr ) != NULL; browsePtr = &browse->next )
	{
		if( browse->ifIndex == inIfIndex ) break;
	}
	require_action_quiet( browse, exit, err = kNotFoundErr );
	
	*browsePtr = browse->next;
	_SBServiceBrowseFree( browse );
	if( !type->browseList )
	{
		*typePtr = type->next;
		_SBServiceTypeFree( type );
	}
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_ServiceBrowserAddServiceInstance
//===========================================================================================================================

static OSStatus
	_ServiceBrowserAddServiceInstance(
		ServiceBrowserRef	me,
		SBServiceBrowse *	inBrowse,
		uint32_t			inIfIndex,
		const char *		inName,
		const char *		inRegType,
		const char *		inDomain,
		uint64_t			inDiscoverTimeUs )
{
	OSStatus					err;
	DNSServiceRef				sdRef;
	SBServiceInstance *			instance;
	SBServiceInstance **		instancePtr;
	SBServiceInstance *			newInstance	= NULL;
	
	for( instancePtr = &inBrowse->instanceList; ( instance = *instancePtr ) != NULL; instancePtr = &instance->next )
	{
		if( ( instance->ifIndex == inIfIndex ) && ( strcasecmp( instance->name, inName ) == 0 ) ) break;
	}
	require_action_quiet( !instance, exit, err = kDuplicateErr );
	
	err = _SBServiceInstanceCreate( inName, inRegType, inDomain, inIfIndex, inDiscoverTimeUs, me, &newInstance );
	require_noerr_quiet( err, exit );
	
	sb_ulog( kLogLevelTrace, "Starting Resolve on interface %d for %s.%s%s",
		(int32_t) newInstance->ifIndex, inName, inRegType, inDomain );
	
	sdRef = me->connection;
	newInstance->resolveStartTicks = UpTicks();
	err = DNSServiceResolve( &sdRef, kDNSServiceFlagsShareConnection, newInstance->ifIndex, inName, inRegType, inDomain,
		_ServiceBrowserResolveCallback, newInstance );
	require_noerr( err, exit );
	
	newInstance->resolve = sdRef;
	*instancePtr	= newInstance;
	newInstance		= NULL;
	
exit:
	if( newInstance ) _SBServiceInstanceFree( newInstance );
	return( err );
}

//===========================================================================================================================
//	_ServiceBrowserRemoveServiceInstance
//===========================================================================================================================

static OSStatus
	_ServiceBrowserRemoveServiceInstance(
		ServiceBrowserRef	me,
		SBServiceBrowse *	inBrowse,
		const char *		inName,
		uint32_t			inIfIndex )
{
	OSStatus					err;
	SBServiceInstance *			instance;
	SBServiceInstance **		ptr;
	
	Unused( me );
	
	for( ptr = &inBrowse->instanceList; ( instance = *ptr ) != NULL; ptr = &instance->next )
	{
		if( ( instance->ifIndex == inIfIndex ) && ( strcasecmp( instance->name, inName ) == 0 ) ) break;
	}
	require_action_quiet( instance, exit, err = kNotFoundErr );
	
	*ptr = instance->next;
	_SBServiceInstanceFree( instance );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_ServiceBrowserAddIPAddress
//===========================================================================================================================

static OSStatus
	_ServiceBrowserAddIPAddress(
		ServiceBrowserRef		me,
		SBServiceInstance *		inInstance,
		const struct sockaddr *	inSockAddr,
		uint64_t				inResolveTimeUs )
{
	OSStatus			err;
	SBIPAddress *		ipaddr;
	SBIPAddress **		ipaddrPtr;
	SBIPAddress *		newIPAddr = NULL;
	
	Unused( me );
	
	if( ( inSockAddr->sa_family != AF_INET ) && ( inSockAddr->sa_family != AF_INET6 ) )
	{
		dlogassert( "Unexpected address family: %d", inSockAddr->sa_family );
		err = kTypeErr;
		goto exit;
	}
	
	for( ipaddrPtr = &inInstance->ipaddrList; ( ipaddr = *ipaddrPtr ) != NULL; ipaddrPtr = &ipaddr->next )
	{
		if( SockAddrCompareAddr( &ipaddr->sip, inSockAddr ) == 0 ) break;
	}
	require_action_quiet( !ipaddr, exit, err = kDuplicateErr );
	
	err = _SBIPAddressCreate( inSockAddr, inResolveTimeUs, &newIPAddr );
	require_noerr_quiet( err, exit );
	
	*ipaddrPtr = newIPAddr;
	newIPAddr = NULL;
	err = kNoErr;
	
exit:
	if( newIPAddr ) _SBIPAddressFree( newIPAddr );
	return( err );
}

//===========================================================================================================================
//	_ServiceBrowserRemoveIPAddress
//===========================================================================================================================

static OSStatus
	_ServiceBrowserRemoveIPAddress(
		ServiceBrowserRef		me,
		SBServiceInstance *		inInstance,
		const struct sockaddr *	inSockAddr )
{
	OSStatus			err;
	SBIPAddress *		ipaddr;
	SBIPAddress **		ipaddrPtr;
	
	Unused( me );
	
	for( ipaddrPtr = &inInstance->ipaddrList; ( ipaddr = *ipaddrPtr ) != NULL; ipaddrPtr = &ipaddr->next )
	{
		if( SockAddrCompareAddr( &ipaddr->sip.sa, inSockAddr ) == 0 ) break;
	}
	require_action_quiet( ipaddr, exit, err = kNotFoundErr );
	
	*ipaddrPtr = ipaddr->next;
	_SBIPAddressFree( ipaddr );
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_ServiceBrowserCreateResults
//===========================================================================================================================

static OSStatus	_ServiceBrowserCreateResults( ServiceBrowserRef me, ServiceBrowserResults **outResults )
{
	OSStatus							err;
	SBDomain *							d;
	SBServiceType *						t;
	SBServiceBrowse *					b;
	SBServiceInstance *					i;
	SBIPAddress *						a;
	ServiceBrowserResultsPrivate *		results;
	SBRDomain **						domainPtr;
	
	results = (ServiceBrowserResultsPrivate *) calloc( 1, sizeof( *results ) );
	require_action( results, exit, err = kNoMemoryErr );
	
	results->refCount = 1;
	
	domainPtr = &results->domainList;
	for( d = me->domainList; d; d = d->next )
	{
		SBRDomain *				domain;
		SBRServiceType **		typePtr;
		
		err = _SBRDomainCreate( d->name, &domain );
		require_noerr_quiet( err, exit );
		*domainPtr = domain;
		 domainPtr = &domain->next;
		
		typePtr = &domain->typeList;
		for( t = d->typeList; t; t = t->next )
		{
			SBRServiceType *			type;
			SBRServiceInstance **		instancePtr;
			
			err = _SBRServiceTypeCreate( t->name, &type );
			require_noerr_quiet( err, exit );
			*typePtr = type;
			 typePtr = &type->next;
			
			instancePtr = &type->instanceList;
			for( b = t->browseList; b; b = b->next )
			{
				for( i = b->instanceList; i; i = i->next )
				{
					SBRServiceInstance *		instance;
					SBRIPAddress **				ipaddrPtr;
					
					err = _SBRServiceInstanceCreate( i->name, i->ifIndex, i->hostname, i->port, i->txtPtr, i->txtLen,
						i->discoverTimeUs, i->resolveTimeUs, &instance );
					require_noerr_quiet( err, exit );
					*instancePtr = instance;
					 instancePtr = &instance->next;
					
					ipaddrPtr = &instance->ipaddrList;
					for( a = i->ipaddrList; a; a = a->next )
					{
						SBRIPAddress *		ipaddr;
						
						err = _SBRIPAddressCreate( &a->sip.sa, a->resolveTimeUs, &ipaddr );
						require_noerr_quiet( err, exit );
						
						*ipaddrPtr = ipaddr;
						 ipaddrPtr = &ipaddr->next;
					}
				}
			}
		}
	}
	
	*outResults = (ServiceBrowserResults *) results;
	results = NULL;
	err = kNoErr;
	
exit:
	if( results ) ServiceBrowserResultsRelease( (ServiceBrowserResults *) results );
	return( err );
}

//===========================================================================================================================
//	_SBDomainCreate
//===========================================================================================================================

static OSStatus	_SBDomainCreate( const char *inName, ServiceBrowserRef inBrowser, SBDomain **outDomain )
{
	OSStatus		err;
	SBDomain *		obj;
	
	obj = (SBDomain *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	obj->name = strdup( inName );
	require_action( obj->name, exit, err = kNoMemoryErr );
	
	obj->browser = inBrowser;
	
	*outDomain = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	if( obj ) _SBDomainFree( obj );
	return( err );
}

//===========================================================================================================================
//	_SBDomainFree
//===========================================================================================================================

static void	_SBDomainFree( SBDomain *inDomain )
{
	SBServiceType *		type;
	
	ForgetMem( &inDomain->name );
	DNSServiceForget( &inDomain->servicesQuery );
	while( ( type = inDomain->typeList ) != NULL )
	{
		inDomain->typeList = type->next;
		_SBServiceTypeFree( type );
	}
	free( inDomain );
}

//===========================================================================================================================
//	_SBServiceTypeCreate
//===========================================================================================================================

static OSStatus	_SBServiceTypeCreate( const char *inName, SBServiceType **outType )
{
	OSStatus			err;
	SBServiceType *		obj;
	
	obj = (SBServiceType *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	obj->name = strdup( inName );
	require_action( obj->name, exit, err = kNoMemoryErr );
	
	*outType = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	if( obj ) _SBServiceTypeFree( obj );
	return( err );
}

//===========================================================================================================================
//	_SBServiceTypeFree
//===========================================================================================================================

static void	_SBServiceTypeFree( SBServiceType *inType )
{
	SBServiceBrowse *		browse;
	
	ForgetMem( &inType->name );
	while( ( browse = inType->browseList ) != NULL )
	{
		inType->browseList = browse->next;
		_SBServiceBrowseFree( browse );
	}
	free( inType );
}

//===========================================================================================================================
//	_SBServiceBrowseCreate
//===========================================================================================================================

static OSStatus	_SBServiceBrowseCreate( uint32_t inIfIndex, ServiceBrowserRef inBrowser, SBServiceBrowse **outBrowse )
{
	OSStatus				err;
	SBServiceBrowse *		obj;
	
	obj = (SBServiceBrowse *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	obj->ifIndex = inIfIndex;
	obj->browser = inBrowser;
	*outBrowse = obj;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_SBServiceBrowseFree
//===========================================================================================================================

static void	_SBServiceBrowseFree( SBServiceBrowse *inBrowse )
{
	SBServiceInstance *		instance;
	
	DNSServiceForget( &inBrowse->browse );
	while( ( instance = inBrowse->instanceList ) != NULL )
	{
		inBrowse->instanceList = instance->next;
		_SBServiceInstanceFree( instance );
	}
	free( inBrowse );
}

//===========================================================================================================================
//	_SBServiceInstanceCreate
//===========================================================================================================================

static OSStatus
	_SBServiceInstanceCreate(
		const char *			inName,
		const char *			inType,
		const char *			inDomain,
		uint32_t				inIfIndex,
		uint64_t				inDiscoverTimeUs,
		ServiceBrowserRef		inBrowser,
		SBServiceInstance **	outInstance )
{
	OSStatus				err;
	SBServiceInstance *		obj;
	
	obj = (SBServiceInstance *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	obj->name = strdup( inName );
	require_action( obj->name, exit, err = kNoMemoryErr );
	
	ASPrintF( &obj->fqdn, "%s.%s%s", obj->name, inType, inDomain );
	require_action( obj->fqdn, exit, err = kNoMemoryErr );
	
	obj->ifIndex		= inIfIndex;
	obj->discoverTimeUs	= inDiscoverTimeUs;
	obj->browser		= inBrowser;
	
	*outInstance = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	if( obj ) _SBServiceInstanceFree( obj );
	return( err );
}

//===========================================================================================================================
//	_SBServiceInstanceFree
//===========================================================================================================================

static void	_SBServiceInstanceFree( SBServiceInstance *inInstance )
{
	ForgetMem( &inInstance->name );
	ForgetMem( &inInstance->fqdn );
	DNSServiceForget( &inInstance->resolve );
	ForgetMem( &inInstance->hostname );
	ForgetMem( &inInstance->txtPtr );
	DNSServiceForget( &inInstance->getAddrInfo );
	ForgetSBIPAddressList( &inInstance->ipaddrList );
	free( inInstance );
}

//===========================================================================================================================
//	_SBIPAddressCreate
//===========================================================================================================================

static OSStatus	_SBIPAddressCreate( const struct sockaddr *inSockAddr, uint64_t inResolveTimeUs, SBIPAddress **outIPAddress )
{
	OSStatus			err;
	SBIPAddress *		obj;
	
	obj = (SBIPAddress *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	SockAddrCopy( inSockAddr, &obj->sip );
	obj->resolveTimeUs = inResolveTimeUs;
	
	*outIPAddress = obj;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_SBIPAddressFree
//===========================================================================================================================

static void _SBIPAddressFree( SBIPAddress *inIPAddress )
{
	free( inIPAddress );
}

//===========================================================================================================================
//	_SBIPAddressFreeList
//===========================================================================================================================

static void	_SBIPAddressFreeList( SBIPAddress *inList )
{
	SBIPAddress *		ipaddr;
	
	while( ( ipaddr = inList ) != NULL )
	{
		inList = ipaddr->next;
		_SBIPAddressFree( ipaddr );
	}
}

//===========================================================================================================================
//	_SBRDomainCreate
//===========================================================================================================================

static OSStatus	_SBRDomainCreate( const char *inName, SBRDomain **outDomain )
{
	OSStatus		err;
	SBRDomain *		obj;
	
	obj = (SBRDomain *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	obj->name = strdup( inName );
	require_action( obj->name, exit, err = kNoMemoryErr );
	
	*outDomain = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	if( obj ) _SBRDomainFree( obj );
	return( err );
}

//===========================================================================================================================
//	_SBRDomainFree
//===========================================================================================================================

static void	_SBRDomainFree( SBRDomain *inDomain )
{
	SBRServiceType *		type;
	
	ForgetMem( &inDomain->name );
	while( ( type = inDomain->typeList ) != NULL )
	{
		inDomain->typeList = type->next;
		_SBRServiceTypeFree( type );
	}
	free( inDomain );
}

//===========================================================================================================================
//	_SBRServiceTypeCreate
//===========================================================================================================================

static OSStatus	_SBRServiceTypeCreate( const char *inName, SBRServiceType **outType )
{
	OSStatus				err;
	SBRServiceType *		obj;
	
	obj = (SBRServiceType *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	obj->name = strdup( inName );
	require_action( obj->name, exit, err = kNoMemoryErr );
	
	*outType = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	if( obj ) _SBRServiceTypeFree( obj );
	return( err );
}

//===========================================================================================================================
//	_SBRServiceTypeFree
//===========================================================================================================================

static void	_SBRServiceTypeFree( SBRServiceType *inType )
{
	SBRServiceInstance *		instance;
	
	ForgetMem( &inType->name );
	while( ( instance = inType->instanceList ) != NULL )
	{
		inType->instanceList = instance->next;
		_SBRServiceInstanceFree( instance );
	}
	free( inType );
}

//===========================================================================================================================
//	_SBRServiceInstanceCreate
//===========================================================================================================================

static OSStatus
	_SBRServiceInstanceCreate(
		const char *			inName,
		uint32_t				inInterfaceIndex,
		const char *			inHostname,
		uint16_t				inPort,
		const uint8_t *			inTXTPtr,
		size_t					inTXTLen,
		uint64_t				inDiscoverTimeUs,
		uint64_t				inResolveTimeUs,
		SBRServiceInstance **	outInstance )
{
	OSStatus					err;
	SBRServiceInstance *		obj;
	
	obj = (SBRServiceInstance *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	obj->name = strdup( inName );
	require_action( obj->name, exit, err = kNoMemoryErr );
	
	if( inHostname )
	{
		obj->hostname = strdup( inHostname );
		require_action( obj->hostname, exit, err = kNoMemoryErr );
	}
	if( inTXTLen > 0 )
	{
		obj->txtPtr = (uint8_t *) _memdup( inTXTPtr, inTXTLen );
		require_action( obj->txtPtr, exit, err = kNoMemoryErr );
		obj->txtLen = inTXTLen;
	}
	obj->discoverTimeUs	= inDiscoverTimeUs;
	obj->resolveTimeUs	= inResolveTimeUs;
	obj->ifIndex		= inInterfaceIndex;
	obj->port			= inPort;
	
	*outInstance = obj;
	obj = NULL;
	err = kNoErr;
	
exit:
	if( obj ) _SBRServiceInstanceFree( obj );
	return( err );
}

//===========================================================================================================================
//	_SBRServiceInstanceFree
//===========================================================================================================================

static void	_SBRServiceInstanceFree( SBRServiceInstance *inInstance )
{
	SBRIPAddress *		ipaddr;
	
	ForgetMem( &inInstance->name );
	ForgetMem( &inInstance->hostname );
	ForgetMem( &inInstance->txtPtr );
	while( ( ipaddr = inInstance->ipaddrList ) != NULL )
	{
		inInstance->ipaddrList = ipaddr->next;
		_SBRIPAddressFree( ipaddr );
	}
	free( inInstance );
}

//===========================================================================================================================
//	_SBRIPAddressCreate
//===========================================================================================================================

static OSStatus
	_SBRIPAddressCreate(
		const struct sockaddr *	inSockAddr,
		uint64_t				inResolveTimeUs,
		SBRIPAddress **			outIPAddress )
{
	OSStatus			err;
	SBRIPAddress *		obj;
	
	obj = (SBRIPAddress *) calloc( 1, sizeof( *obj ) );
	require_action( obj, exit, err = kNoMemoryErr );
	
	SockAddrCopy( inSockAddr, &obj->sip );
	obj->resolveTimeUs = inResolveTimeUs;
	
	*outIPAddress = obj;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_SBRIPAddressFree
//===========================================================================================================================

static void	_SBRIPAddressFree( SBRIPAddress *inIPAddress )
{
	free( inIPAddress );
}

//===========================================================================================================================
//	_SocketWriteAll
//
//	Note: This was copied from CoreUtils because the SocketWriteAll function is currently not exported in the framework.
//===========================================================================================================================

static OSStatus	_SocketWriteAll( SocketRef inSock, const void *inData, size_t inSize, int32_t inTimeoutSecs )
{
	OSStatus			err;
	const uint8_t *		src;
	const uint8_t *		end;
	fd_set				writeSet;
	struct timeval		timeout;
	ssize_t				n;
	
	FD_ZERO( &writeSet );
	src = (const uint8_t *) inData;
	end = src + inSize;
	while( src < end )
	{
		FD_SET( inSock, &writeSet );
		timeout.tv_sec 	= inTimeoutSecs;
		timeout.tv_usec = 0;
		n = select( (int)( inSock + 1 ), NULL, &writeSet, NULL, &timeout );
		if( n == 0 ) { err = kTimeoutErr; goto exit; }
		err = map_socket_value_errno( inSock, n > 0, n );
		require_noerr( err, exit );
		
		n = send( inSock, (char *) src, (size_t)( end - src ), 0 );
		err = map_socket_value_errno( inSock, n >= 0, n );
		if( err == EINTR ) continue;
		require_noerr( err, exit );
		
		src += n;
	}
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_ParseIPv4Address
//
//	Warning: "inBuffer" may be modified even in error cases.
//
//	Note: This was copied from CoreUtils because the StringToIPv4Address function is currently not exported in the framework.
//===========================================================================================================================

static OSStatus	_ParseIPv4Address( const char *inStr, uint8_t inBuffer[ 4 ], const char **outStr )
{
	OSStatus		err;
	uint8_t *		dst;
	int				segments;
	int				sawDigit;
	int				c;
	int				v;
	
	check( inBuffer );
	check( outStr );
	
	dst		 = inBuffer;
	*dst	 = 0;
	sawDigit = 0;
	segments = 0;
	for( ; ( c = *inStr ) != '\0'; ++inStr )
	{
		if( isdigit_safe( c ) )
		{
			v = ( *dst * 10 ) + ( c - '0' );
			require_action_quiet( v <= 255, exit, err = kRangeErr );
			*dst = (uint8_t) v;
			if( !sawDigit )
			{
				++segments;
				require_action_quiet( segments <= 4, exit, err = kOverrunErr );
				sawDigit = 1;
			}
		}
		else if( ( c == '.' ) && sawDigit )
		{
			require_action_quiet( segments < 4, exit, err = kMalformedErr );
			*++dst = 0;
			sawDigit = 0;
		}
		else
		{
			break;
		}
	}
	require_action_quiet( segments == 4, exit, err = kUnderrunErr );
	
	*outStr = inStr;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_StringToIPv4Address
//
//	Note: This was copied from CoreUtils because the StringToIPv4Address function is currently not exported in the framework.
//===========================================================================================================================

static OSStatus
	_StringToIPv4Address( 
		const char *			inStr, 
		StringToIPAddressFlags	inFlags, 
		uint32_t *				outIP, 
		int *					outPort, 
		uint32_t *				outSubnet, 
		uint32_t *				outRouter, 
		const char **			outStr )
{
	OSStatus			err;
	uint8_t				buf[ 4 ];
	int					c;
	uint32_t			ip;
	int					hasPort;
	int					port;
	int					hasPrefix;
	int					prefix;
	uint32_t			subnetMask;
	uint32_t			router;
	
	require_action( inStr, exit, err = kParamErr );
	
	// Parse the address-only part of the address (e.g. "1.2.3.4").
	
	err = _ParseIPv4Address( inStr, buf, &inStr );
	require_noerr_quiet( err, exit );
	ip = (uint32_t)( ( buf[ 0 ] << 24 ) | ( buf[ 1 ] << 16 ) | ( buf[ 2 ] << 8 ) | buf[ 3 ] );
	c = *inStr;
	
	// Parse the port (if any).
	
	hasPort = 0;
	port    = 0;
	if( c == ':' )
	{
		require_action_quiet( !( inFlags & kStringToIPAddressFlagsNoPort ), exit, err = kUnexpectedErr );
		while( ( ( c = *( ++inStr ) ) != '\0' ) && ( ( c >= '0' ) && ( c <= '9' ) ) ) port = ( port * 10 ) + ( c - '0' );
		require_action_quiet( port <= 65535, exit, err = kRangeErr );
		hasPort = 1;
	}
	
	// Parse the prefix length (if any).
	
	hasPrefix  = 0;
	prefix     = 0;
	subnetMask = 0;
	router     = 0;
	if( c == '/' )
	{
		require_action_quiet( !( inFlags & kStringToIPAddressFlagsNoPrefix ), exit, err = kUnexpectedErr );
		while( ( ( c = *( ++inStr ) ) != '\0' ) && ( ( c >= '0' ) && ( c <= '9' ) ) ) prefix = ( prefix * 10 ) + ( c - '0' );
		require_action_quiet( ( prefix >= 0 ) && ( prefix <= 32 ), exit, err = kRangeErr );
		hasPrefix = 1;
		
		subnetMask = ( prefix > 0 ) ? ( UINT32_C( 0xFFFFFFFF ) << ( 32 - prefix ) ) : 0;
		router	   = ( ip & subnetMask ) | 1;
	}
	
	// Return the results. Only fill in port/prefix/router results if the info was found to allow for defaults.
	
	if( outIP )					 *outIP		= ip;
	if( outPort   && hasPort )	 *outPort	= port;
	if( outSubnet && hasPrefix ) *outSubnet	= subnetMask;
	if( outRouter && hasPrefix ) *outRouter	= router;
	if( outStr )				 *outStr	= inStr;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_ParseIPv6Address
//
//	Note: Parsed according to the rules specified in RFC 3513.
//	Warning: "inBuffer" may be modified even in error cases.
//
//	Note: This was copied from CoreUtils because the StringToIPv6Address function is currently not exported in the framework.
//===========================================================================================================================

static OSStatus	_ParseIPv6Address( const char *inStr, int inAllowV4Mapped, uint8_t inBuffer[ 16 ], const char **outStr )
{
													// Table to map uppercase hex characters - '0' to their numeric values.
													// 0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?  @  A   B   C   D   E   F
	static const uint8_t		kASCIItoHexTable[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15 };
	OSStatus					err;
	const char *				ptr;
	uint8_t *					dst;
	uint8_t *					lim;
	uint8_t *					colonPtr;
	int							c;
	int							sawDigit;
	unsigned int				v;
	int							i;
	int							n;
	
	// Pre-zero the address to simplify handling of compressed addresses (e.g. "::1").
	
	for( i = 0; i < 16; ++i ) inBuffer[ i ] = 0;
	
	// Special case leading :: (e.g. "::1") to simplify processing later.
	
	if( *inStr == ':' )
	{
		++inStr;
		require_action_quiet( *inStr == ':', exit, err = kMalformedErr );
	}
	
	// Parse the address.
	
	ptr		 = inStr;
	dst		 = inBuffer;
	lim		 = dst + 16;
	colonPtr = NULL;
	sawDigit = 0;
	v		 = 0;
	while( ( ( c = *inStr++ ) != '\0' ) && ( c != '%' ) && ( c != '/' ) && ( c != ']' ) )
	{
		if(   ( c >= 'a' ) && ( c <= 'f' ) ) c -= ( 'a' - 'A' );
		if( ( ( c >= '0' ) && ( c <= '9' ) ) || ( ( c >= 'A' ) && ( c <= 'F' ) ) )
		{
			c -= '0';
			check( c < (int) countof( kASCIItoHexTable ) );
			v = ( v << 4 ) | kASCIItoHexTable[ c ];
			require_action_quiet( v <= 0xFFFF, exit, err = kRangeErr );
			sawDigit = 1;
			continue;
		}
		if( c == ':' )
		{
			ptr = inStr;
			if( !sawDigit )
			{
				require_action_quiet( !colonPtr, exit, err = kMalformedErr );
				colonPtr = dst;
				continue;
			}
			require_action_quiet( *inStr != '\0', exit, err = kUnderrunErr );
			require_action_quiet( ( dst + 2 ) <= lim, exit, err = kOverrunErr );
			*dst++ = (uint8_t)( ( v >> 8 ) & 0xFF );
			*dst++ = (uint8_t)(   v        & 0xFF );
			sawDigit = 0;
			v = 0;
			continue;
		}
		
		// Handle IPv4-mapped/compatible addresses (e.g. ::FFFF:1.2.3.4).
		
		if( inAllowV4Mapped && ( c == '.' ) && ( ( dst + 4 ) <= lim ) )
		{
			err = _ParseIPv4Address( ptr, dst, &inStr );
			require_noerr_quiet( err, exit );
			dst += 4;
			sawDigit = 0;
			++inStr; // Increment because the code below expects the end to be at "inStr - 1".
		}
		break;
	}
	if( sawDigit )
	{
		require_action_quiet( ( dst + 2 ) <= lim, exit, err = kOverrunErr );
		*dst++ = (uint8_t)( ( v >> 8 ) & 0xFF );
		*dst++ = (uint8_t)(   v        & 0xFF );
	}
	check( dst <= lim );
	if( colonPtr )
	{
		require_action_quiet( dst < lim, exit, err = kOverrunErr );
		n = (int)( dst - colonPtr );
		for( i = 1; i <= n; ++i )
		{
			lim[ -i ] = colonPtr[ n - i ];
			colonPtr[ n - i ] = 0;
		}
		dst = lim;
	}
	require_action_quiet( dst == lim, exit, err = kUnderrunErr );
	
	*outStr = inStr - 1;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_ParseIPv6Scope
//
//	Note: This was copied from CoreUtils because the StringToIPv6Address function is currently not exported in the framework.
//===========================================================================================================================

static OSStatus	_ParseIPv6Scope( const char *inStr, uint32_t *outScope, const char **outStr )
{
#if( TARGET_OS_POSIX )
	OSStatus			err;
	char				scopeStr[ 64 ];
	char *				dst;
	char *				lim;
	int					c;
	uint32_t			scope;
	const char *		ptr;
	
	// Copy into a local NULL-terminated string since that is what if_nametoindex expects.
	
	dst = scopeStr;
	lim = dst + ( countof( scopeStr ) - 1 );
	while( ( ( c = *inStr ) != '\0' ) && ( c != ':' ) && ( c != '/' ) && ( c != ']' ) && ( dst < lim ) )
	{
		*dst++ = *inStr++;
	}
	*dst = '\0';
	check( dst <= lim );
	
	// First try to map as a name and if that fails, treat it as a numeric scope.
	
	scope = if_nametoindex( scopeStr );
	if( scope == 0 )
	{
		for( ptr = scopeStr; ( ( c = *ptr ) >= '0' ) && ( c <= '9' ); ++ptr )
		{
			scope = ( scope * 10 ) + ( ( (uint8_t) c ) - '0' );
		}
		require_action_quiet( c == '\0', exit, err = kMalformedErr );
		require_action_quiet( ( ptr != scopeStr ) && ( ( (int)( ptr - scopeStr ) ) <= 10 ), exit, err = kMalformedErr );
	}
	
	*outScope = scope;
	*outStr   = inStr;
	err = kNoErr;
	
exit:
	return( err );
#else
	OSStatus			err;
	uint32_t			scope;
	const char *		start;
	int					c;
	
	scope = 0;
	for( start = inStr; ( ( c = *inStr ) >= '0' ) && ( c <= '9' ); ++inStr )
	{
		scope = ( scope * 10 ) + ( c - '0' );
	}
	require_action_quiet( ( inStr != start ) && ( ( (int)( inStr - start ) ) <= 10 ), exit, err = kMalformedErr );
	
	*outScope = scope;
	*outStr   = inStr;
	err = kNoErr;
	
exit:
	return( err );
#endif
}

//===========================================================================================================================
//	_StringToIPv6Address
//
//	Note: This was copied from CoreUtils because the StringToIPv6Address function is currently not exported in the framework.
//===========================================================================================================================

static OSStatus
	_StringToIPv6Address( 
		const char *			inStr, 
		StringToIPAddressFlags	inFlags, 
		uint8_t					outIPv6[ 16 ], 
		uint32_t *				outScope, 
		int *					outPort, 
		int *					outPrefix, 
		const char **			outStr )
{
	OSStatus		err;
	uint8_t			ipv6[ 16 ];
	int				c;
	int				hasScope;
	uint32_t		scope;
	int				hasPort;
	int				port;
	int				hasPrefix;
	int				prefix;
	int				hasBracket;
	int				i;
	
	require_action( inStr, exit, err = kParamErr );
	
	if( *inStr == '[' ) ++inStr; // Skip a leading bracket for []-wrapped addresses (e.g. "[::1]:80").
	
	// Parse the address-only part of the address (e.g. "1::1").
	
	err = _ParseIPv6Address( inStr, !( inFlags & kStringToIPAddressFlagsNoIPv4Mapped ), ipv6, &inStr );
	require_noerr_quiet( err, exit );
	c = *inStr;
	
	// Parse the scope, port, or prefix length.
	
	hasScope	= 0;
	scope		= 0;
	hasPort		= 0;
	port		= 0;
	hasPrefix	= 0;
	prefix		= 0;
	hasBracket	= 0;
	for( ;; )
	{
		if( c == '%' )		// Scope (e.g. "%en0" or "%5")
		{
			require_action_quiet( !hasScope, exit, err = kMalformedErr );
			require_action_quiet( !( inFlags & kStringToIPAddressFlagsNoScope ), exit, err = kUnexpectedErr );
			++inStr;
			err = _ParseIPv6Scope( inStr, &scope, &inStr );
			require_noerr_quiet( err, exit );
			hasScope = 1;
			c = *inStr;
		}
		else if( c == ':' )	// Port (e.g. ":80")
		{
			require_action_quiet( !hasPort, exit, err = kMalformedErr );
			require_action_quiet( !( inFlags & kStringToIPAddressFlagsNoPort ), exit, err = kUnexpectedErr );
			while( ( ( c = *( ++inStr ) ) != '\0' ) && ( ( c >= '0' ) && ( c <= '9' ) ) ) port = ( port * 10 ) + ( c - '0' );
			require_action_quiet( port <= 65535, exit, err = kRangeErr );
			hasPort = 1;
		}
		else if( c == '/' )	// Prefix Length (e.g. "/64")
		{
			require_action_quiet( !hasPrefix, exit, err = kMalformedErr );
			require_action_quiet( !( inFlags & kStringToIPAddressFlagsNoPrefix ), exit, err = kUnexpectedErr );
			while( ( ( c = *( ++inStr ) ) != '\0' ) && ( ( c >= '0' ) && ( c <= '9' ) ) ) prefix = ( prefix * 10 ) + ( c - '0' );
			require_action_quiet( ( prefix >= 0 ) && ( prefix <= 128 ), exit, err = kRangeErr );
			hasPrefix = 1;
		}
		else if( c == ']' )
		{
			require_action_quiet( !hasBracket, exit, err = kMalformedErr );
			hasBracket = 1;
			c = *( ++inStr );
		}
		else
		{
			break;
		}
	}
	
	// Return the results. Only fill in scope/port/prefix results if the info was found to allow for defaults.
	
	if( outIPv6 )				 for( i = 0; i < 16; ++i ) outIPv6[ i ] = ipv6[ i ];
	if( outScope  && hasScope )  *outScope	= scope;
	if( outPort   && hasPort )   *outPort	= port;
	if( outPrefix && hasPrefix ) *outPrefix	= prefix;
	if( outStr )				 *outStr	= inStr;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================
//	_StringArray_Free
//
//	Note: This was copied from CoreUtils because the StringArray_Free function is currently not exported in the framework.
//===========================================================================================================================

static void	_StringArray_Free( char **inArray, size_t inCount )
{
	size_t		i;
	
	for( i = 0; i < inCount; ++i )
	{
		free( inArray[ i ] );
	}
	if( inCount > 0 ) free( inArray );
}

//===========================================================================================================================
//	_ParseQuotedEscapedString
//
//	Note: This was copied from CoreUtils because it's currently not exported in the framework.
//===========================================================================================================================

static Boolean
	_ParseQuotedEscapedString( 
		const char *	inSrc, 
		const char *	inEnd, 
		const char *	inDelimiters, 
		char *			inBuf, 
		size_t			inMaxLen, 
		size_t *		outCopiedLen, 
		size_t *		outTotalLen, 
		const char **	outSrc )
{
	const unsigned char *		src;
	const unsigned char *		end;
	unsigned char *				dst;
	unsigned char *				lim;
	unsigned char				c;
	unsigned char				c2;
	size_t						totalLen;
	Boolean						singleQuote;
	Boolean						doubleQuote;
	
	if( inEnd == NULL ) inEnd = inSrc + strlen( inSrc );
	src = (const unsigned char *) inSrc;
	end = (const unsigned char *) inEnd;
	dst = (unsigned char *) inBuf;
	lim = dst + inMaxLen;
	while( ( src < end ) && isspace_safe( *src ) ) ++src; // Skip leading spaces.
	if( src >= end ) return( false );
	
	// Parse each argument from the string.
	//
	// See <http://resources.mpi-inf.mpg.de/departments/rg1/teaching/unixffb-ss98/quoting-guide.html> for details.
	
	totalLen = 0;
	singleQuote = false;
	doubleQuote = false;
	while( src < end )
	{
		c = *src++;
		if( singleQuote )
		{
			// Single quotes protect everything (even backslashes, newlines, etc.) except single quotes.
			
			if( c == '\'' )
			{
				singleQuote = false;
				continue;
			}
		}
		else if( doubleQuote )
		{
			// Double quotes protect everything except double quotes and backslashes. A backslash can be 
			// used to protect " or \ within double quotes. A backslash-newline pair disappears completely.
			// A backslash followed by x or X and 2 hex digits (e.g. "\x1f") is stored as that hex byte.
			// A backslash followed by 3 octal digits (e.g. "\377") is stored as that octal byte.
			// A backslash that does not precede ", \, x, X, or a newline is taken literally.
			
			if( c == '"' )
			{
				doubleQuote = false;
				continue;
			}
			else if( c == '\\' )
			{
				if( src < end )
				{
					c2 = *src;
					if( ( c2 == '"' ) || ( c2 == '\\' ) )
					{
						++src;
						c = c2;
					}
					else if( c2 == '\n' )
					{
						++src;
						continue;
					}
					else if( ( c2 == 'x' ) || ( c2 == 'X' ) )
					{
						++src;
						c = c2;
						if( ( ( end - src ) >= 2 ) && IsHexPair( src ) )
						{
							c = HexPairToByte( src );
							src += 2;
						}
					}
					else if( isoctal_safe( c2 ) )
					{
						if( ( ( end - src ) >= 3 ) && IsOctalTriple( src ) )
						{
							c = OctalTripleToByte( src );
							src += 3;
						}
					}
				}
			}
		}
		else if( strchr( inDelimiters, c ) )
		{
			break;
		}
		else if( c == '\\' )
		{
			// A backslash protects the next character, except a newline, x, X and 2 hex bytes or 3 octal bytes. 
			// A backslash followed by a newline disappears completely.
			// A backslash followed by x or X and 2 hex digits (e.g. "\x1f") is stored as that hex byte.
			// A backslash followed by 3 octal digits (e.g. "\377") is stored as that octal byte.
			
			if( src < end )
			{
				c = *src;
				if( c == '\n' )
				{
					++src;
					continue;
				}
				else if( ( c == 'x' ) || ( c == 'X' ) )
				{
					++src;
					if( ( ( end - src ) >= 2 ) && IsHexPair( src ) )
					{
						c = HexPairToByte( src );
						src += 2;
					}
				}
				else if( isoctal_safe( c ) )
				{
					if( ( ( end - src ) >= 3 ) && IsOctalTriple( src ) )
					{
						c = OctalTripleToByte( src );
						src += 3;
					}
					else
					{
						++src;
					}
				}
				else
				{
					++src;
				}
			}
		}
		else if( c == '\'' )
		{
			singleQuote = true;
			continue;
		}
		else if( c == '"' )
		{
			doubleQuote = true;
			continue;
		}
		
		if( dst < lim )
		{
			if( inBuf ) *dst = c;
			++dst;
		}
		++totalLen;
	}
	
	if( outCopiedLen )	*outCopiedLen	= (size_t)( dst - ( (unsigned char *) inBuf ) );
	if( outTotalLen )	*outTotalLen	= totalLen;
	if( outSrc )		*outSrc			= (const char *) src;
	return( true );
}

//===========================================================================================================================
//	_ServerSocketOpenEx2
//
//	Note: Based on ServerSocketOpenEx() from CoreUtils. Added parameter to not use SO_REUSEPORT.
//===========================================================================================================================

static OSStatus
	_ServerSocketOpenEx2( 
		int				inFamily, 
		int				inType, 
		int				inProtocol, 
		const void *	inAddr, 
		int				inPort, 
		int *			outPort, 
		int				inRcvBufSize, 
		Boolean			inNoPortReuse,
		SocketRef *		outSock )
{
	OSStatus		err;
	int				port;
	SocketRef		sock;
	int				name;
	int				option;
	sockaddr_ip		sip;
	socklen_t		len;
	
	port = ( inPort < 0 ) ? -inPort : inPort; // Negated port number means "try this port, but allow dynamic".
	
	sock = socket( inFamily, inType, inProtocol );
	err = map_socket_creation_errno( sock );
	require_noerr_quiet( err, exit );
	
#if( defined( SO_NOSIGPIPE ) )
	setsockopt( sock, SOL_SOCKET, SO_NOSIGPIPE, &(int){ 1 }, (socklen_t) sizeof( int ) );
#endif
	
	err = SocketMakeNonBlocking( sock );
	require_noerr( err, exit );
	
	// Set receive buffer size. This has to be done on the listening socket *before* listen is called because
	// accept does not return until after the window scale option is exchanged during the 3-way handshake. 
	// Since accept returns a new socket, the only way to use a larger window scale option is to set the buffer
	// size on the listening socket since SO_RCVBUF is inherited by the accepted socket. See UNPv1e3 Section 7.5.
	
	err = SocketSetBufferSize( sock, SO_RCVBUF, inRcvBufSize );
	check_noerr( err );
	
	// Allow port or address reuse because we may bind separate IPv4 and IPv6 sockets to the same port.
	
	if( ( inType != SOCK_DGRAM ) || !inNoPortReuse )
	{
		option = 1;
		name = ( inType == SOCK_DGRAM ) ? SO_REUSEPORT : SO_REUSEADDR;
		err = setsockopt( sock, SOL_SOCKET, name, (char *) &option, (socklen_t) sizeof( option ) );
		err = map_socket_noerr_errno( sock, err );
		require_noerr( err, exit );
	}
	
	if( inFamily == AF_INET )
	{
		// Bind to the port. If it fails, retry with a dynamic port.
		
		memset( &sip.v4, 0, sizeof( sip.v4 ) );
		SIN_LEN_SET( &sip.v4 );
		sip.v4.sin_family		= AF_INET;
		sip.v4.sin_port			= htons( (uint16_t) port );
		sip.v4.sin_addr.s_addr	= inAddr ? *( (const uint32_t *) inAddr ) : htonl( INADDR_ANY );
		err = bind( sock, &sip.sa, (socklen_t) sizeof( sip.v4 ) );
		err = map_socket_noerr_errno( sock, err );
		if( err && ( inPort < 0 ) )
		{
			sip.v4.sin_port = 0;
			err = bind( sock, &sip.sa, (socklen_t) sizeof( sip.v4 ) );
			err = map_socket_noerr_errno( sock, err );
		}
		require_noerr( err, exit );
	}
#if( defined( AF_INET6 ) )
	else if( inFamily == AF_INET6 )
	{
		// Restrict this socket to IPv6 only because we're going to use a separate socket for IPv4.
		
		option = 1;
		err = setsockopt( sock, IPPROTO_IPV6, IPV6_V6ONLY, (char *) &option, (socklen_t) sizeof( option ) );
		err = map_socket_noerr_errno( sock, err );
		require_noerr( err, exit );
		
		// Bind to the port. If it fails, retry with a dynamic port.
		
		memset( &sip.v6, 0, sizeof( sip.v6 ) );
		SIN6_LEN_SET( &sip.v6 );
		sip.v6.sin6_family	= AF_INET6;
		sip.v6.sin6_port	= htons( (uint16_t) port );
		sip.v6.sin6_addr	= inAddr ? *( (const struct in6_addr *) inAddr ) : in6addr_any;	
		err = bind( sock, &sip.sa, (socklen_t) sizeof( sip.v6 ) );
		err = map_socket_noerr_errno( sock, err );
		if( err && ( inPort < 0 ) )
		{
			sip.v6.sin6_port = 0;
			err = bind( sock, &sip.sa, (socklen_t) sizeof( sip.v6 ) );
			err = map_socket_noerr_errno( sock, err );
		}
		require_noerr( err, exit );
	}
#endif
	else
	{
		dlogassert( "Unsupported family: %d", inFamily );
		err = kUnsupportedErr;
		goto exit;
	}
	
	if( inType == SOCK_STREAM )
	{
		err = listen( sock, SOMAXCONN );
		err = map_socket_noerr_errno( sock, err );
		if( err )
		{
			err = listen( sock, 5 );
			err = map_socket_noerr_errno( sock, err );
			require_noerr( err, exit );
		}
	}
	
	if( outPort )
	{
		len = (socklen_t) sizeof( sip );
		err = getsockname( sock, &sip.sa, &len );
		err = map_socket_noerr_errno( sock, err );
		require_noerr( err, exit );
		
		*outPort = SockAddrGetPort( &sip );
	}
	*outSock = sock;
	sock = kInvalidSocketRef;
	
exit:
	ForgetSocket( &sock );
	return( err );
}

//===========================================================================================================================
//	_memdup
//
//	Note: This was copied from CoreUtils because it's currently not exported in the framework.
//===========================================================================================================================

static void *	_memdup( const void *inPtr, size_t inLen )
{
	void *		mem;
	
	mem = malloc( ( inLen > 0 ) ? inLen : 1 ); // If inLen is 0, use 1 since malloc( 0 ) is not well defined.
	require( mem, exit );
	if( inLen > 0 ) memcpy( mem, inPtr, inLen );
	
exit:
	return( mem );
}

//===========================================================================================================================
//	_memicmp
//
//	Note: This was copied from CoreUtils because it's currently not exported in the framework.
//===========================================================================================================================

static int	_memicmp( const void *inP1, const void *inP2, size_t inLen )
{
	const unsigned char *		p1;
	const unsigned char *		e1;
	const unsigned char *		p2;
	int							c1;
	int							c2;
	
	p1 = (const unsigned char *) inP1;
	e1 = p1 + inLen;
	p2 = (const unsigned char *) inP2;
	while( p1 < e1 )
	{
		c1 = *p1++;
		c2 = *p2++;
		c1 = tolower( c1 );
		c2 = tolower( c2 );
		if( c1 < c2 ) return( -1 );
		if( c1 > c2 ) return(  1 );
	}
	return( 0 );
}

//===========================================================================================================================
//	_FNV1
//
//	Note: This was copied from CoreUtils because it's currently not exported in the framework.
//===========================================================================================================================

static uint32_t	_FNV1( const void *inData, size_t inSize )
{
	const uint8_t *				src = (const uint8_t *) inData;
	const uint8_t * const		end = src + inSize;
	uint32_t					hash;
	
	hash = 0x811c9dc5U;
	while( src != end )
	{
		hash *= 0x01000193;
		hash ^= *src++;
	}
	return( hash );
}
