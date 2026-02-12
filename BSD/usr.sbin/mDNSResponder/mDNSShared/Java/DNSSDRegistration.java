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


/**	A tracking object for a registration created by {@link DNSSD#register}. */

public interface	DNSSDRegistration extends DNSSDService
{
	/** Get a reference to the primary TXT record of a registered service.<P> 
		The record can be updated by sending it an update() message.<P>

		<P>
		@return		A {@link DNSRecord}. 
					If {@link DNSSDRegistration#stop} is called, the DNSRecord is also 
					invalidated and may not be used further.
	*/
	DNSRecord		getTXTRecord()
	throws DNSSDException;

	/** Add a record to a registered service.<P> 
		The name of the record will be the same as the registered service's name.<P>
		The record can be updated or deregistered by sending it an update() or remove() message.<P>

		@param	flags
					Currently unused, reserved for future use.
		<P>
		@param	rrType
					The type of the record (e.g. TXT, SRV, etc), as defined in nameser.h.
		<P>
		@param	rData
					The raw rdata to be contained in the added resource record.
		<P>
		@param	ttl
					The time to live of the resource record, in seconds.
		<P>
		@return		A {@link DNSRecord} that may be passed to updateRecord() or removeRecord(). 
					If {@link DNSSDRegistration#stop} is called, the DNSRecord is also 
					invalidated and may not be used further.
	*/
	DNSRecord		addRecord( int flags, int rrType, byte[] rData, int ttl)
	throws DNSSDException;
} 

