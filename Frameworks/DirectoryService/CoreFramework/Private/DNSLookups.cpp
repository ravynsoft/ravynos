/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*!
 * @header DNSLookups
 */

#include "DNSLookups.h"

CFMutableArrayRef ParseServiceResults( dns_reply_t *answer )
{
    CFMutableArrayRef	cfArray = 0;

	if (answer != NULL)
	{
		dns_SRV_record_t	*srv;
		int index = 0;

		if ( (answer->header != NULL) && (answer->header->ancount > 0) )
		{
			cfArray = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
			// the answer is not a null terminated list, it has a count...
			while( index < answer->header->ancount )
			{
				if ( answer->answer[index] != NULL)
				{
					srv = answer->answer[index]->data.SRV;
					if ( (srv != NULL) && (srv->target != NULL) )
					{
						CFNumberRef				cfNumber	= 0;
						CFMutableDictionaryRef  cfDict		= CFDictionaryCreateMutable( kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
						CFStringRef				cfStringRef = 0;
						
						cfStringRef = CFStringCreateWithCString( kCFAllocatorDefault, srv->target, kCFStringEncodingUTF8 );
						CFDictionarySetValue( cfDict, CFSTR("Host"), cfStringRef );
						CFRelease(cfStringRef);
						cfNumber = CFNumberCreate(kCFAllocatorDefault,kCFNumberShortType,&(srv->port));
						CFDictionarySetValue( cfDict, CFSTR("Port"), cfNumber );
						CFRelease(cfNumber);
						cfNumber = CFNumberCreate(kCFAllocatorDefault,kCFNumberShortType,&(srv->weight));
						CFDictionarySetValue( cfDict, CFSTR("Weight"), cfNumber );
						CFRelease(cfNumber);
						cfNumber = CFNumberCreate(kCFAllocatorDefault,kCFNumberShortType,&(srv->priority));
						CFDictionarySetValue( cfDict, CFSTR("Priority"), cfNumber );
						CFRelease(cfNumber);
						
						CFArrayAppendValue(cfArray, cfDict);
						CFRelease(cfDict);
					}
				}
				
				index++;
			}
		}
		dns_free_reply( answer );
	}
    return(cfArray);
}

dns_reply_t *doDNSLookup( const char *inType, const char *inQuery )
{
	uint16_t		type		= 0;
	uint16_t		classtype   = 0;
	dns_handle_t	dns			= dns_open(NULL);
	dns_reply_t    *outReply	= NULL;

	if( dns ) {
		dns_type_number( inType, &type );
		dns_class_number( "IN", &classtype );

		// let's use a 256 k buffer to be safe for large networks
		//    40-100 bytes per lookup = 2560-6553 servers
		// it gets disposed of when done 
		dns_set_buffer_size( dns, 256*1024 ); 
		outReply = dns_lookup( dns, inQuery, classtype, type );
		dns_free( dns );
	}

	return outReply;
}

CFMutableArrayRef getDNSServiceRecs( const char *type, const char *domain )
{
    char			   *service;
	dns_reply_t		   *answer;
	CFMutableArrayRef   finalResults	= 0;
	
	// _ldap._tcp.ldap.domain.com. SRV 10 5 389. ldap.domain.com
	// the four fields are priority, weight, port, hostname follow the naming
	//can just use _ldap._tcp alone as the name will get implicitly added
	if ( type != NULL)
	{
		service = (char *)calloc(1, 256);
		if( domain != NULL )
		{
			snprintf( service, 256, "_%s._tcp.%s.", type, domain );
		}
		else
		{
			snprintf( service, 256, "_%s._tcp", type );
		}
		answer = doDNSLookup( "SRV", service );
		free(service);
		service = NULL;
		
		finalResults = ParseServiceResults( answer );	// let's parse the results
	}

	return(finalResults);
}//getDNSServiceRecs
