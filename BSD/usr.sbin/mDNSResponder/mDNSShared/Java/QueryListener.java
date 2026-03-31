/* -*- Mode: Java; tab-width: 4 -*-
 *
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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


package	com.apple.dnssd;


/**	A listener that receives results from {@link DNSSD#queryRecord}. */

public interface QueryListener extends BaseListener
{
	/** Called when a record query has been completed. Inspect flags 
	    parameter to determine nature of query event.<P> 

		@param	query
					The active query object.
		<P>
		@param	flags
					If kDNSServiceFlagsAdd bit is set, this is a newly discovered answer; 
					otherwise this is a previously discovered answer which has expired.
					Other possible values are DNSSD.MORE_COMING.
		<P>
		@param	ifIndex
					The interface on which the query was resolved. (The index for a given 
					interface is determined via the if_nametoindex() family of calls.) 
		<P>
		@param	fullName
					The resource record's full domain name.
		<P>
		@param	rrtype
					The resource record's type (e.g. PTR, SRV, etc) as defined by RFC 1035 and its updates.
		<P>
		@param	rrclass
					The class of the resource record, as defined by RFC 1035 and its updates.
		<P>
		@param	rdata
					The raw rdata of the resource record.
		<P>
		@param	ttl
					The resource record's time to live, in seconds.
	*/
	void	queryAnswered( DNSSDService query, int flags, int ifIndex, String fullName, 
								int rrtype, int rrclass, byte[] rdata, int ttl);
}

