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


/**	
	Reference to a record returned by {@link DNSSDRegistration#addRecord}.<P> 

	Note: client is responsible for serializing access to these objects if 
	they are shared between concurrent threads.
*/

public interface	DNSRecord
{
	/** Update a registered resource record.<P> 
		The record must either be the primary txt record of a service registered via DNSSD.register(), 
		or a record added to a registered service via addRecord().<P>

		@param	flags
					Currently unused, reserved for future use.
		<P>
		@param	rData
					The new rdata to be contained in the updated resource record.
		<P>
		@param	ttl
					The time to live of the updated resource record, in seconds.
	*/
	void			update( int flags, byte[] rData, int ttl)
	throws DNSSDException;

	/** Remove a registered resource record.<P> 
	*/
	void			remove()
	throws DNSSDException;
}

