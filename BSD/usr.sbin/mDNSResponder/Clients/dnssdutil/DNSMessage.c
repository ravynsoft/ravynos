/*
	Copyright (c) 2016-2019 Apple Inc. All rights reserved.
*/

#include "DNSMessage.h"

//===========================================================================================================================

#define IsCompressionByte( X )		( ( ( X ) & 0xC0 ) == 0xC0 )

OSStatus
	DNSMessageExtractDomainName(
		const uint8_t *		inMsgPtr,
		size_t				inMsgLen,
		const uint8_t *		inPtr,
		uint8_t				outName[ kDomainNameLengthMax ],
		const uint8_t **	outPtr )
{
	OSStatus					err;
	const uint8_t *				label;
	uint8_t						labelLen;
	const uint8_t *				nextLabel;
	const uint8_t * const		msgEnd	= inMsgPtr + inMsgLen;
	uint8_t *					dst		= outName;
	const uint8_t * const		dstLim	= outName ? ( outName + kDomainNameLengthMax ) : NULL;
	const uint8_t *				nameEnd	= NULL;
	
	require_action_quiet( ( inPtr >= inMsgPtr ) && ( inPtr < msgEnd ), exit, err = kRangeErr );
	
	for( label = inPtr; ( labelLen = label[ 0 ] ) != 0; label = nextLabel )
	{
		if( labelLen <= kDomainLabelLengthMax )
		{
			nextLabel = label + 1 + labelLen;
			require_action_quiet( nextLabel < msgEnd, exit, err = kUnderrunErr );
			if( dst )
			{
				require_action_quiet( ( dstLim - dst ) > ( 1 + labelLen ), exit, err = kOverrunErr );
				memcpy( dst, label, 1 + labelLen );
				dst += ( 1 + labelLen );
			}
		}
		else if( IsCompressionByte( labelLen ) )
		{
			uint16_t		offset;
			
			require_action_quiet( ( msgEnd - label ) >= 2, exit, err = kUnderrunErr );
			if( !nameEnd )
			{
				nameEnd = label + 2;
				if( !dst ) break;
			}
			offset = (uint16_t)( ( ( label[ 0 ] & 0x3F ) << 8 ) | label[ 1 ] );
			nextLabel = inMsgPtr + offset;
			require_action_quiet( nextLabel < msgEnd, exit, err = kUnderrunErr );
			require_action_quiet( !IsCompressionByte( nextLabel[ 0 ] ), exit, err = kMalformedErr );
		}
		else
		{
			err = kMalformedErr;
			goto exit;
		}
	}
	
	if( dst ) *dst = 0;
	if( !nameEnd ) nameEnd = label + 1;
	
	if( outPtr ) *outPtr = nameEnd;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================

OSStatus
	DNSMessageExtractDomainNameString(
		const void *		inMsgPtr,
		size_t				inMsgLen,
		const void *		inPtr,
		char				inBuf[ kDNSServiceMaxDomainName ],
		const uint8_t **	outPtr )
{
	OSStatus			err;
	const uint8_t *		nextPtr;
	uint8_t				domainName[ kDomainNameLengthMax ];
	
	err = DNSMessageExtractDomainName( inMsgPtr, inMsgLen, inPtr, domainName, &nextPtr );
	require_noerr_quiet( err, exit );
	
	err = DomainNameToString( domainName, NULL, inBuf, NULL );
	require_noerr_quiet( err, exit );
	
	if( outPtr ) *outPtr = nextPtr;
	
exit:
	return( err );
}

//===========================================================================================================================

OSStatus
	DNSMessageExtractQuestion(
		const uint8_t *		inMsgPtr,
		size_t				inMsgLen,
		const uint8_t *		inPtr,
		uint8_t				outName[ kDomainNameLengthMax ],
		uint16_t *			outType,
		uint16_t *			outClass,
		const uint8_t **	outPtr )
{
	OSStatus								err;
	const uint8_t * const					msgEnd = &inMsgPtr[ inMsgLen ];
	const uint8_t *							ptr;
	const dns_fixed_fields_question *		fields;
	
	err = DNSMessageExtractDomainName( inMsgPtr, inMsgLen, inPtr, outName, &ptr );
	require_noerr_quiet( err, exit );
	require_action_quiet( (size_t)( msgEnd - ptr ) >= sizeof( dns_fixed_fields_question ), exit, err = kUnderrunErr );
	
	fields = (const dns_fixed_fields_question *) ptr;
	if( outType )  *outType  = dns_fixed_fields_question_get_type( fields );
	if( outClass ) *outClass = dns_fixed_fields_question_get_class( fields );
	if( outPtr )   *outPtr   = (const uint8_t *) &fields[ 1 ];
	
exit:
	return( err );
}

//===========================================================================================================================

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
		const uint8_t **	outPtr )
{
	OSStatus							err;
	const uint8_t * const				msgEnd = inMsgPtr + inMsgLen;
	const uint8_t *						ptr;
	const dns_fixed_fields_record *		fields;
	const uint8_t *						rdata;
	size_t								rdLength;
	
	err = DNSMessageExtractDomainName( inMsgPtr, inMsgLen, inPtr, outName, &ptr );
	require_noerr_quiet( err, exit );
	require_action_quiet( (size_t)( msgEnd - ptr ) >= sizeof( *fields ), exit, err = kUnderrunErr );
	
	fields	= (const dns_fixed_fields_record *) ptr;
	rdata	= ptr + sizeof( *fields );
	
	rdLength = dns_fixed_fields_record_get_rdlength( fields );
	require_action_quiet( (size_t)( msgEnd - rdata ) >= rdLength , exit, err = kUnderrunErr );
	
	if( outType )		*outType		= dns_fixed_fields_record_get_type( fields );
	if( outClass )		*outClass		= dns_fixed_fields_record_get_class( fields );
	if( outTTL )		*outTTL			= dns_fixed_fields_record_get_ttl( fields );
	if( outRDataPtr )	*outRDataPtr	= rdata;
	if( outRDataLen )	*outRDataLen	= rdLength;
	if( outPtr )		*outPtr			= &rdata[ rdLength ];
	
exit:
	return( err );
}

//===========================================================================================================================

OSStatus	DNSMessageGetAnswerSection( const uint8_t *inMsgPtr, size_t inMsgLen, const uint8_t **outPtr )
{
	OSStatus				err;
	unsigned int			questionCount, i;
	const DNSHeader *		hdr;
	const uint8_t *			ptr;
	
	require_action_quiet( inMsgLen >= kDNSHeaderLength, exit, err = kSizeErr );
	
	hdr = (DNSHeader *) inMsgPtr;
	questionCount = DNSHeaderGetQuestionCount( hdr );
	
	ptr = (const uint8_t *) &hdr[ 1 ];
	for( i = 0; i < questionCount; ++i )
	{
		err = DNSMessageExtractQuestion( inMsgPtr, inMsgLen, ptr, NULL, NULL, NULL, &ptr );
		require_noerr_quiet( err, exit );
	}
	
	if( outPtr ) *outPtr = ptr;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================

OSStatus
	DNSMessageWriteQuery(
		uint16_t		inMsgID,
		uint16_t		inFlags,
		const uint8_t *	inQName,
		uint16_t		inQType,
		uint16_t		inQClass,
		uint8_t			outMsg[ STATIC_PARAM kDNSQueryMessageMaxLen ],
		size_t *		outLen )
{
	OSStatus				err;
	DNSHeader * const		hdr = (DNSHeader *) outMsg;
	uint8_t *				ptr;
	size_t					qnameLen;
	size_t					msgLen;
	
	memset( hdr, 0, sizeof( *hdr ) );
	DNSHeaderSetID( hdr, inMsgID );
	DNSHeaderSetFlags( hdr, inFlags );
	DNSHeaderSetQuestionCount( hdr, 1 );
	
	qnameLen = DomainNameLength( inQName );
	require_action_quiet( qnameLen <= kDomainNameLengthMax, exit, err = kSizeErr );
	
	ptr = (uint8_t *) &hdr[ 1 ];
	memcpy( ptr, inQName, qnameLen );
	ptr += qnameLen;
	
	dns_fixed_fields_question_init( (dns_fixed_fields_question *) ptr, inQType, inQClass );
	ptr += sizeof( dns_fixed_fields_question );
	
	msgLen = (size_t)( ptr - outMsg );
	
	if( outLen ) *outLen = msgLen;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================

OSStatus
	DomainNameAppendString(
		uint8_t			inDomainName[ STATIC_PARAM kDomainNameLengthMax ],
		const char *	inString,
		uint8_t **		outEnd )
{
	OSStatus					err;
	const char *				src;
	uint8_t *					root;
	const uint8_t * const		nameLim = inDomainName + kDomainNameLengthMax;
	
	for( root = inDomainName; ( root < nameLim ) && *root; root += ( 1 + *root ) ) {}
	require_action_quiet( root < nameLim, exit, err = kMalformedErr );
	
	// If the string is a single dot, denoting the root domain, then there are no non-empty labels.
	
	src = inString;
	if( ( src[ 0 ] == '.' ) && ( src[ 1 ] == '\0' ) ) ++src;
	while( *src )
	{
		uint8_t * const				label		= root;
		const uint8_t * const		labelLim	= Min( &label[ 1 + kDomainLabelLengthMax ], nameLim - 1 );
		uint8_t *					dst;
		int							c;
		size_t						labelLen;
		
		dst = &label[ 1 ];
		while( *src && ( ( c = *src++ ) != '.' ) )
		{
			if( c == '\\' )
			{
				require_action_quiet( *src != '\0', exit, err = kUnderrunErr );
				c = *src++;
				if( isdigit_safe( c ) && isdigit_safe( src[ 0 ] ) && isdigit_safe( src[ 1 ] ) )
				{
					const int		decimal = ( ( c - '0' ) * 100 ) + ( ( src[ 0 ] - '0' ) * 10 ) + ( src[ 1 ] - '0' );
					
					if( decimal <= 255 )
					{
						c = decimal;
						src += 2;
					}
				}
			}
			require_action_quiet( dst < labelLim, exit, err = kOverrunErr );
			*dst++ = (uint8_t) c;
		}
		
		labelLen = (size_t)( dst - &label[ 1 ] );
		require_action_quiet( labelLen > 0, exit, err = kMalformedErr );
		
		label[ 0 ] = (uint8_t) labelLen;
		root = dst;
		*root = 0;
	}
	
	if( outEnd ) *outEnd = root + 1;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================

OSStatus	DomainNameDupEx( const uint8_t *inName, Boolean inLower, uint8_t **outNamePtr, size_t *outNameLen )
{
	OSStatus			err;
	uint8_t *			namePtr;
	const size_t		nameLen = DomainNameLength( inName );
	
	namePtr = (uint8_t *) malloc( nameLen );
	require_action_quiet( namePtr, exit, err = kNoMemoryErr );
	
	if( inLower )
	{
		const uint8_t *		src;
		uint8_t *			dst;
		unsigned int		len;
		
		src = inName;
		dst = namePtr;
		while( ( len = *src ) != 0 )
		{
			*dst++ = *src++;
			while( len-- > 0 )
			{
				const int c = *src++;
				*dst++ = (uint8_t) tolower_safe( c );
			}
		}
		*dst = 0;
	}
	else
	{
		memcpy( namePtr, inName, nameLen );
	}
	
	*outNamePtr = namePtr;
	if( outNameLen ) *outNameLen = nameLen;
	err = kNoErr;
	
exit:
	return( err );
}

//===========================================================================================================================

Boolean	DomainNameEqual( const uint8_t *inName1, const uint8_t *inName2 )
{
	const uint8_t *		p1 = inName1;
	const uint8_t *		p2 = inName2;
	if( p1 == p2 ) return( true );
	for( ;; )
	{
		int			len1 = *p1++;
		const int	len2 = *p2++;
		if( len1 != len2 )	return( false );
		if( len1 == 0 )		return( true );
		while( len1-- > 0 )
		{
			const int		c1 = *p1++;
			const int		c2 = *p2++;
			if( tolower_safe( c1 ) != tolower_safe( c2 ) ) return( false );
		}
	}
}

//===========================================================================================================================

OSStatus
	DomainNameFromString(
		uint8_t			outName[ STATIC_PARAM kDomainNameLengthMax ],
		const char *	inString,
		uint8_t **		outEnd )
{
	outName[ 0 ] = 0;
	return( DomainNameAppendString( outName, inString, outEnd ) );
}

//===========================================================================================================================

OSStatus
	DomainNameToString(
		const uint8_t *		inName,
		const uint8_t *		inLimit,
		char				outString[ STATIC_PARAM kDNSServiceMaxDomainName ],
		const uint8_t **	outPtr )
{
	OSStatus			err;
	const uint8_t *		label;
	uint8_t				labelLen;
	const uint8_t *		nextLabel;
	char *				dst;
	const uint8_t *		src;
	
	require_action_quiet( !inLimit || ( inName < inLimit ), exit, err = kUnderrunErr );
	
	// Convert each label up until the root label, i.e., the zero-length label.
	
	dst = outString;
	for( label = inName; ( labelLen = label[ 0 ] ) != 0; label = nextLabel )
	{
		require_action_quiet( labelLen <= kDomainLabelLengthMax, exit, err = kMalformedErr );
		
		nextLabel = &label[ 1 + labelLen ];
		require_action_quiet( ( nextLabel - inName ) < kDomainNameLengthMax, exit, err = kMalformedErr );
		require_action_quiet( !inLimit || ( nextLabel < inLimit ), exit, err = kUnderrunErr );
		
		for( src = &label[ 1 ]; src < nextLabel; ++src )
		{
			// Only allow 7-bit ASCII characters.
			if( ( *src >= 32 ) && ( *src <= 126 ) )
			{
				if( ( *src == '.' ) || ( *src == '\\' ) ||  ( *src == ' ' ) ) *dst++ = '\\';
				*dst++ = (char) *src;
			}
			else
			{
				*dst++ = '\\';
				*dst++ = '0' + (   *src / 100 );
				*dst++ = '0' + ( ( *src /  10 ) % 10 );
				*dst++ = '0' + (   *src         % 10 );
			}
		}
		*dst++ = '.';
	}
	
	// At this point, label points to the root label.
	// If the root label was the only label, then write a dot for it.
	
	if( label == inName ) *dst++ = '.';
	*dst = '\0';
	if( outPtr ) *outPtr = label + 1;
	err = kNoErr;
	
exit:
	return( err );
}
