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


/**	A listener that receives results from {@link DNSSD#resolve}. */

public interface ResolveListener extends BaseListener
{
	/** Called when a service has been resolved.<P> 

		@param	resolver
					The active resolver object.
		<P>
		@param	flags
					Currently unused, reserved for future use.
		<P>
		@param	fullName
					The full service domain name, in the form &lt;servicename&gt;.&lt;protocol&gt;.&lt;domain&gt;.
					(Any literal dots (".") are escaped with a backslash ("\."), and literal
					backslashes are escaped with a second backslash ("\\"), e.g. a web server
					named "Dr. Pepper" would have the fullname  "Dr\.\032Pepper._http._tcp.local.").
					This is the appropriate format to pass to standard system DNS APIs such as 
					res_query(), or to the special-purpose functions included in this API that
					take fullname parameters.
		<P>
		@param	hostName
					The target hostname of the machine providing the service.  This name can 
					be passed to functions like queryRecord() to look up the host's IP address.
		<P>
		@param	port
					The port number on which connections are accepted for this service.
		<P>
		@param	txtRecord
					The service's primary txt record.
	*/
	void	serviceResolved( DNSSDService resolver, int flags, int ifIndex, String fullName, 
								String hostName, int port, TXTRecord txtRecord);
}

