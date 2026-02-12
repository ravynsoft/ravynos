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


/**	A listener that receives results from {@link DNSSD#browse}. */

public interface BrowseListener extends BaseListener
{
	/** Called to report discovered services.<P> 

		@param	browser
					The active browse service.
		<P>
		@param	flags
					Possible values are DNSSD.MORE_COMING.
		<P>
		@param	ifIndex
					The interface on which the service is advertised. This index should be passed 
					to {@link DNSSD#resolve} when resolving the service.
		<P>
		@param	serviceName
					The service name discovered.
		<P>
		@param	regType
					The registration type, as passed in to DNSSD.browse().
		<P>
		@param	domain
					The domain in which the service was discovered.
	*/
	void	serviceFound( DNSSDService browser, int flags, int ifIndex, 
							String serviceName, String regType, String domain);

	/** Called to report services which have been deregistered.<P> 

		@param	browser
					The active browse service.
		<P>
		@param	flags
					Possible values are DNSSD.MORE_COMING.
		<P>
		@param	ifIndex
					The interface on which the service is advertised.
		<P>
		@param	serviceName
					The service name which has deregistered.
		<P>
		@param	regType
					The registration type, as passed in to DNSSD.browse().
		<P>
		@param	domain
					The domain in which the service was discovered.
	*/
	void	serviceLost( DNSSDService browser, int flags, int ifIndex,
							String serviceName, String regType, String domain);
}

