/* -*- Mode: Java; tab-width: 4 -*-
 *
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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


/**	An object for registering records, created by {@link DNSSD#createRecordRegistrar}. */

public interface	DNSSDRecordRegistrar extends DNSSDService
{
	/** Register an independent {@link DNSRecord}.<P> 
		@param	flags
					Possible values are SHARED or UNIQUE (see flag type definitions for details).
		<P>
		@param	ifIndex
					If non-zero, specifies the interface on which to register the record
					(the index for a given interface is determined via the if_nametoindex()
					family of calls.)  Passing 0 causes the record to be registered on all interfaces.
		<P>
		@param	fullname
					The full domain name of the resource record.
		<P>
		@param	rrtype
					The numerical type of the resource record to be queried for (e.g. PTR, SRV, etc)
					as defined in nameser.h.
		<P>
		@param	rrclass
					The class of the resource record, as defined in nameser.h 
					(usually 1 for the Internet class).
		<P>
		@param	rData
					The new rdata  as it is to appear in the DNS record.
		<P>
		@param	ttl
					The time to live of the resource record, in seconds. Pass 0 to use a default value.
		<P>
		@param	listener
					This object will get called when the service is registered.
		<P>
		@return		A {@link DNSSDService} that can be used to abort the record registration.

		@throws SecurityException If a security manager is present and denies <tt>RuntimePermission("getDNSSDInstance")</tt>.
		@see    RuntimePermission
	*/
	public DNSRecord	registerRecord( int flags, int ifIndex, String fullname, int rrtype, 
									int rrclass, byte[] rdata, int ttl)
	throws DNSSDException;
} 

