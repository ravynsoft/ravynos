/*
	Copyright (c) 2016-2019 Apple Inc. All rights reserved.
*/

#ifndef	__DNSMessage_h
#define	__DNSMessage_h

#include <CoreUtils/CoreUtils.h>

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------------------------------------------------------
/*!	@group		DNS domain name size limits
	
	@discussion See <https://tools.ietf.org/html/rfc1035#section-2.3.4>.
*/
#define kDomainLabelLengthMax		63
#define kDomainNameLengthMax		256	// For compatibility with mDNS. See <https://tools.ietf.org/html/rfc6762#appendix-C>.

//---------------------------------------------------------------------------------------------------------------------------
/*!	@group		DNS message header
*/
typedef struct
{
	uint8_t		id[ 2 ];
	uint8_t		flags[ 2 ];
	uint8_t		questionCount[ 2 ];
	uint8_t		answerCount[ 2 ];
	uint8_t		authorityCount[ 2 ];
	uint8_t		additionalCount[ 2 ];
	
}	DNSHeader;

#define kDNSHeaderLength		12
check_compile_time( sizeof( DNSHeader ) == kDNSHeaderLength );

#define DNSHeaderGetID( HDR )					ReadBig16( ( HDR )->id )
#define DNSHeaderGetFlags( HDR )				ReadBig16( ( HDR )->flags )
#define DNSHeaderGetQuestionCount( HDR )		ReadBig16( ( HDR )->questionCount )
#define DNSHeaderGetAnswerCount( HDR )			ReadBig16( ( HDR )->answerCount )
#define DNSHeaderGetAuthorityCount( HDR )		ReadBig16( ( HDR )->authorityCount )
#define DNSHeaderGetAdditionalCount( HDR )		ReadBig16( ( HDR )->additionalCount )

#define DNSHeaderSetID( HDR, X )					WriteBig16( ( HDR )->id, (X) )
#define DNSHeaderSetFlags( HDR, X )					WriteBig16( ( HDR )->flags, (X) )
#define DNSHeaderSetQuestionCount( HDR, X )			WriteBig16( ( HDR )->questionCount, (X) )
#define DNSHeaderSetAnswerCount( HDR, X )			WriteBig16( ( HDR )->answerCount, (X) )
#define DNSHeaderSetAuthorityCount( HDR, X )		WriteBig16( ( HDR )->authorityCount, (X) )
#define DNSHeaderSetAdditionalCount( HDR, X )		WriteBig16( ( HDR )->additionalCount, (X) )

// Single-bit DNS header fields

#define kDNSHeaderFlag_Response					( 1U << 15 )	// QR (bit 15), Query (0)/Response (1)
#define kDNSHeaderFlag_AuthAnswer				( 1U << 10 )	// AA (bit 10), Authoritative Answer
#define kDNSHeaderFlag_Truncation				( 1U <<  9 )	// TC (bit  9), TrunCation
#define kDNSHeaderFlag_RecursionDesired			( 1U <<  8 )	// RD (bit  8), Recursion Desired
#define kDNSHeaderFlag_RecursionAvailable		( 1U <<  7 )	// RA (bit  7), Recursion Available
#define kDNSHeaderFlag_Z						( 1U <<  6 )	//  Z (bit  6), Reserved (must be zero)
#define kDNSHeaderFlag_AuthenticData			( 1U <<  5 )	// AD (bit  5), Authentic Data (RFC 2535, Section 6)
#define kDNSHeaderFlag_CheckingDisabled			( 1U <<  4 )	// CD (bit  4), Checking Disabled (RFC 2535, Section 6)

// OPCODE (bits 14-11), Operation Code

#define DNSFlagsGetOpCode( FLAGS )		( ( (FLAGS) >> 11 ) & 0x0FU )
#define DNSFlagsSetOpCode( FLAGS, OPCODE ) \
	do { (FLAGS) = ( (FLAGS) & ~0x7800U ) | ( ( (OPCODE) & 0x0FU ) << 11 ); } while( 0 )

#define kDNSOpCode_Query			0	// QUERY (standard query)
#define kDNSOpCode_InverseQuery		1	// IQUERY (inverse query)
#define kDNSOpCode_Status			2	// STATUS
#define kDNSOpCode_Notify			4	// NOTIFY
#define kDNSOpCode_Update			5	// UPDATE

// RCODE (bits 3-0), Response Code

#define DNSFlagsGetRCode( FLAGS )		( (FLAGS) & 0x0FU )
#define DNSFlagsSetRCode( FLAGS, RCODE ) \
	do { (FLAGS) = ( (FLAGS) & ~0x000FU ) | ( (RCODE) & 0x0FU ); } while( 0 )

#define kDNSRCode_NoError				0
#define kDNSRCode_FormatError			1
#define kDNSRCode_ServerFailure			2
#define kDNSRCode_NXDomain				3
#define kDNSRCode_NotImplemented		4
#define kDNSRCode_Refused				5

//---------------------------------------------------------------------------------------------------------------------------
/*!	@group		Misc. DNS message data structures
*/

#define dns_fixed_fields_define_accessors( TYPE, FIELD, BIT_SIZE )	\
	STATIC_INLINE uint ## BIT_SIZE ##_t								\
		dns_fixed_fields_ ## TYPE ## _get_ ## FIELD (				\
			const dns_fixed_fields_ ## TYPE *	inFields )			\
	{																\
		return ReadBig ## BIT_SIZE ( inFields->FIELD );				\
	}																\
																	\
	STATIC_INLINE void												\
		dns_fixed_fields_ ## TYPE ## _set_ ## FIELD (				\
			dns_fixed_fields_ ## TYPE *	inFields,					\
			uint ## BIT_SIZE ## _t		inValue )					\
	{																\
		WriteBig ## BIT_SIZE ( inFields->FIELD, inValue );			\
	}																\
	check_compile_time( ( sizeof_field( dns_fixed_fields_ ## TYPE, FIELD ) * 8 ) == (BIT_SIZE) )

// DNS question fixed-length fields (see <https://tools.ietf.org/html/rfc1035#section-4.1.2>)

typedef struct
{
	uint8_t		type[ 2 ];
	uint8_t		class[ 2 ];
	
}	dns_fixed_fields_question;

check_compile_time( sizeof( dns_fixed_fields_question ) == 4 );

dns_fixed_fields_define_accessors( question, type,  16 );
dns_fixed_fields_define_accessors( question, class, 16 );

STATIC_INLINE void
	dns_fixed_fields_question_init(
		dns_fixed_fields_question *	inFields,
		uint16_t					inQType,
		uint16_t					inQClass )
{
	dns_fixed_fields_question_set_type( inFields, inQType );
	dns_fixed_fields_question_set_class( inFields, inQClass );
}

// DNS resource record fixed-length fields (see <https://tools.ietf.org/html/rfc1035#section-4.1.3>)

typedef struct
{
	uint8_t		type[ 2 ];
	uint8_t		class[ 2 ];
	uint8_t		ttl[ 4 ];
	uint8_t		rdlength[ 2 ];
	
}	dns_fixed_fields_record;

check_compile_time( sizeof( dns_fixed_fields_record ) == 10 );

dns_fixed_fields_define_accessors( record, type,     16 );
dns_fixed_fields_define_accessors( record, class,    16 );
dns_fixed_fields_define_accessors( record, ttl,      32 );
dns_fixed_fields_define_accessors( record, rdlength, 16 );

STATIC_INLINE void
	dns_fixed_fields_record_init(
		dns_fixed_fields_record *	inFields,
		uint16_t					inType,
		uint16_t					inClass,
		uint32_t					inTTL,
		uint16_t					inRDLength )
{
	dns_fixed_fields_record_set_type( inFields, inType );
	dns_fixed_fields_record_set_class( inFields, inClass );
	dns_fixed_fields_record_set_ttl( inFields, inTTL );
	dns_fixed_fields_record_set_rdlength( inFields, inRDLength );
}

// DNS SRV record data fixed-length fields (see <https://tools.ietf.org/html/rfc2782>)

typedef struct
{
	uint8_t		priority[ 2 ];
	uint8_t		weight[ 2 ];
	uint8_t		port[ 2 ];
	
}	dns_fixed_fields_srv;

check_compile_time( sizeof( dns_fixed_fields_srv ) == 6 );

dns_fixed_fields_define_accessors( srv, priority, 16 );
dns_fixed_fields_define_accessors( srv, weight,   16 );
dns_fixed_fields_define_accessors( srv, port,     16 );

STATIC_INLINE void
	dns_fixed_fields_srv_init(
		dns_fixed_fields_srv *	inFields,
		uint16_t				inPriority,
		uint16_t				inWeight,
		uint16_t				inPort )
{
	dns_fixed_fields_srv_set_priority( inFields, inPriority );
	dns_fixed_fields_srv_set_weight( inFields, inWeight );
	dns_fixed_fields_srv_set_port( inFields, inPort );
}

// DNS SOA record data fixed-length fields (see <https://tools.ietf.org/html/rfc1035#section-3.3.13>)

typedef struct
{
	uint8_t		serial[ 4 ];
	uint8_t		refresh[ 4 ];
	uint8_t		retry[ 4 ];
	uint8_t		expire[ 4 ];
	uint8_t		minimum[ 4 ];
	
}	dns_fixed_fields_soa;

check_compile_time( sizeof( dns_fixed_fields_soa ) == 20 );

dns_fixed_fields_define_accessors( soa, serial,  32 );
dns_fixed_fields_define_accessors( soa, refresh, 32 );
dns_fixed_fields_define_accessors( soa, retry,   32 );
dns_fixed_fields_define_accessors( soa, expire,  32 );
dns_fixed_fields_define_accessors( soa, minimum, 32 );

STATIC_INLINE void
	dns_fixed_fields_soa_init(
		dns_fixed_fields_soa *	inFields,
		uint32_t				inSerial,
		uint32_t				inRefresh,
		uint32_t				inRetry,
		uint32_t				inExpire,
		uint32_t				inMinimum )
{
	dns_fixed_fields_soa_set_serial( inFields, inSerial );
	dns_fixed_fields_soa_set_refresh( inFields, inRefresh );
	dns_fixed_fields_soa_set_retry( inFields, inRetry );
	dns_fixed_fields_soa_set_expire( inFields, inExpire );
	dns_fixed_fields_soa_set_minimum( inFields, inMinimum );
}

//---------------------------------------------------------------------------------------------------------------------------
/*!	@brief		Extracts a domain name from a DNS message.
	
	@param		inMsgPtr		Pointer to the beginning of the DNS message containing the domain name.
	@param		inMsgLen		Length of the DNS message containing the domain name.
	@param		inPtr			Pointer to the domain name field.
	@param		outName			Buffer to write extracted domain name. (Optional)
	@param		outPtr			Gets set to point to the end of the domain name field. (Optional)
*/
OSStatus
	DNSMessageExtractDomainName(
		const uint8_t *		inMsgPtr,
		size_t				inMsgLen,
		const uint8_t *		inPtr,
		uint8_t				outName[ kDomainNameLengthMax ],
		const uint8_t **	outPtr );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@brief		Extracts a domain name from a DNS message as a C string.
	
	@param		inMsgPtr		Pointer to the beginning of the DNS message containing the domain name.
	@param		inMsgLen		Length of the DNS message containing the domain name.
	@param		inPtr			Pointer to the domain name field.
	@param		outName			Buffer to write extracted domain name. (Optional)
	@param		outPtr			Gets set to point to the end of the domain name field. (Optional)
*/
OSStatus
	DNSMessageExtractDomainNameString(
		const void *		inMsgPtr,
		size_t				inMsgLen,
		const void *		inPtr,
		char				outName[ kDNSServiceMaxDomainName ],
		const uint8_t **	outPtr );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@brief		Extracts a question from a DNS message.
	
	@param		inMsgPtr		Pointer to the beginning of the DNS message containing a question.
	@param		inMsgLen		Length of the DNS message containing the question.
	@param		inPtr			Pointer to the question.
	@param		outName			Buffer to write the question's QNAME. (Optional)
	@param		outType			Gets set to question's QTYPE value. (Optional)
	@param		outClass		Gets set to question's QCLASS value. (Optional)
	@param		outPtr			Gets set to point to the end of the question. (Optional)
*/
OSStatus
	DNSMessageExtractQuestion(
		const uint8_t *		inMsgPtr,
		size_t				inMsgLen,
		const uint8_t *		inPtr,
		uint8_t				outName[ kDomainNameLengthMax ],
		uint16_t *			outType,
		uint16_t *			outClass,
		const uint8_t **	outPtr );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@brief		Extracts a resource record from a DNS message.
	
	@param		inMsgPtr		Pointer to the beginning of the DNS message containing the resource record.
	@param		inMsgLen		Length of the DNS message containing the resource record.
	@param		inPtr			Pointer to the resource record.
	@param		outName			Buffer to write the resource record's NAME. (Optional)
	@param		outType			Gets set to resource record's TYPE value. (Optional)
	@param		outClass		Gets set to resource record's CLASS value. (Optional)
	@param		outTTL			Gets set to resource record's TTL value. (Optional)
	@param		outRDataPtr		Gets set to point to the resource record's RDATA. (Optional)
	@param		outRDataLen		Gets set to the resource record's RDLENGTH. (Optional)
	@param		outPtr			Gets set to point to the end of the resource record. (Optional)
*/
OSStatus
	DNSMessageExtractRecord(
		const uint8_t *		inMsgPtr,
		size_t				inMsgLen,
		const uint8_t *		inPtr,
		uint8_t				outName[ kDomainNameLengthMax ],
		uint16_t *			outType,
		uint16_t *			outClass,
		uint32_t *			outTTL,
		const uint8_t **	outRDataPtr,
		size_t *			outRDataLen,
		const uint8_t **	outPtr );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@brief		Returns pointer to the start of the answer section, i.e., the end of the question section, of a DNS message.
	
	@param		inMsgPtr		Pointer to the beginning of the DNS message.
	@param		inMsgLen		Length of the DNS message.
	@param		outPtr			Gets set to point to the start of the answer section. (Optional)
*/
OSStatus	DNSMessageGetAnswerSection( const uint8_t *inMsgPtr, size_t inMsgLen, const uint8_t **outPtr );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@brief		Writes a DNS message compression label pointer.
	
	@param		inLabelPtr		Pointer to the two bytes to which to write the label pointer.
	@param		inOffset		The label pointer's offset value. This offset is relative to the start of the DNS message.
	
	@discussion	See <https://tools.ietf.org/html/rfc1035#section-4.1.4>.
*/
STATIC_INLINE void	DNSMessageWriteLabelPointer( uint8_t inLabelPtr[ STATIC_PARAM 2 ], size_t inOffset )
{
	inLabelPtr[ 0 ] = (uint8_t)( ( ( inOffset >> 8 ) & 0x3F ) | 0xC0 );
	inLabelPtr[ 1 ] = (uint8_t)(     inOffset        & 0xFF          );
}

#define kDNSCompressionOffsetMax		0x3FFF

//---------------------------------------------------------------------------------------------------------------------------
/*!	@brief		Writes a single-question DNS query message.
	
	@param		inMsgID			The query message's ID.
	@param		inFlags			The query message's flags.
	@param		inQName			The question's QNAME in label format.
	@param		inQType			The question's QTYPE.
	@param		inQClass		The question's QCLASS.
	@param		outMsg			Buffer to write DNS query message.
	@param		outLen			Gets set to the length of the DNS query message.
	
	@discussion	The duplicate domain name must be freed with free() when no longer needed.
*/
#define kDNSQueryMessageMaxLen		( kDNSHeaderLength + kDomainNameLengthMax + sizeof( dns_fixed_fields_question ) )

OSStatus
	DNSMessageWriteQuery(
		uint16_t		inMsgID,
		uint16_t		inFlags,
		const uint8_t *	inQName,
		uint16_t		inQType,
		uint16_t		inQClass,
		uint8_t			outMsg[ STATIC_PARAM kDNSQueryMessageMaxLen ],
		size_t *		outLen );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@brief		Appends a C string representing a textual sequence of labels to a domain name.
	
	@param		inName			Pointer to the domain name.
	@param		inString		Pointer to textual sequence of labels as a C string. (Optional)
	@param		outEnd			Gets set to point to the new end of the domain name if the append succeeded. (Optional)
*/
OSStatus
	DomainNameAppendString(
		uint8_t			inName[ STATIC_PARAM kDomainNameLengthMax ],
		const char *	inString,
		uint8_t **		outEnd );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@brief		Creates a duplicate domain name.
	
	@param		inName			The domain name to duplicate.
	@param		inLower			If true, uppercase letters in the duplicate are converted to lowercase.
	@param		outNamePtr		Gets set to point to a dynamically allocated duplicate.
	@param		outNameLen		Gets set to the length of the duplicate.
	
	@discussion	The duplicate domain name must be freed with free() when no longer needed.
*/
OSStatus	DomainNameDupEx( const uint8_t *inName, Boolean inLower, uint8_t **outNamePtr, size_t *outNameLen );

#define DomainNameDup( IN_NAME, OUT_NAME, OUT_LEN )				DomainNameDupEx( IN_NAME, false, OUT_NAME, OUT_LEN )
#define DomainNameDupLower( IN_NAME, OUT_NAME, OUT_LEN )		DomainNameDupEx( IN_NAME, true, OUT_NAME, OUT_LEN )

//---------------------------------------------------------------------------------------------------------------------------
/*!	@brief		Compares two domain names in label format for case-insensitive equality.
	
	@param		inName1		Pointer to the first domain name.
	@param		inName2		Pointer to the second domain name.
	
	@result		If the domain names are equal, returns true, otherwise, returns false.
*/
Boolean	DomainNameEqual( const uint8_t *inName1, const uint8_t *inName2 );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@brief		Converts a domain name's textual representation to a domain name in label format.
	
	@param		outName			Buffer to write the domain name in label format.
	@param		inString		Textual representation of a domain name as a C string.
	@param		outEnd			Gets set to point to the new end of the domain name if the append succeeded. (Optional)
	
	@discussion	The duplicate domain name must be freed with free() when no longer needed.
*/
OSStatus
	DomainNameFromString(
		uint8_t			outName[ STATIC_PARAM kDomainNameLengthMax ],
		const char *	inString,
		uint8_t **		outEnd );

//---------------------------------------------------------------------------------------------------------------------------
/*!	@brief		Gets the next label in a domain name label sequence.
	
	@param		inLabel		Pointer to the current label.
	
	@result		If the current label is a root label, returns NULL. Otherwise, returns the next label.
*/
STATIC_INLINE const uint8_t *	DomainNameGetNextLabel( const uint8_t *inLabel )
{
	const int len = *inLabel;
	return ( ( len == 0 ) ? NULL : &inLabel[ 1 + len ] );
}

//---------------------------------------------------------------------------------------------------------------------------
/*!	@brief		Computes the length of a domain name in label format.
	
	@param		inName		The domain name.
*/
STATIC_INLINE size_t	DomainNameLength( const uint8_t *inName )
{
	const uint8_t *		label;
	int					len;
	
	for( label = inName; ( len = *label ) != 0; label = &label[ 1 + len ] ) {}
	return( (size_t)( label - inName ) + 1 );
}

//---------------------------------------------------------------------------------------------------------------------------
/*!	@brief		Converts a domain name in label format to its textual representation as a C string.
	
	@param		inName		Pointer to the domain name.
	@param		inLimit		Pointer to not exceed while parsing a potentially truncated domain name. (Optional)
	@param		outString	Buffer to write the C string.
	@param		outPtr		Gets set to point to the end of the domain name. (Optional)
*/
OSStatus
	DomainNameToString(
		const uint8_t *		inName,
		const uint8_t *		inLimit,
		char				outString[ STATIC_PARAM kDNSServiceMaxDomainName ],
		const uint8_t **	outPtr );

#ifdef __cplusplus
}
#endif

#endif // __DNSMessage_h
