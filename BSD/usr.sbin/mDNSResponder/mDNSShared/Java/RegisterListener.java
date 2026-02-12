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


/**	A listener that receives results from {@link DNSSD#register}. */

public interface RegisterListener extends BaseListener
{
	/** Called when a registration has been completed.<P> 

		@param	registration
					The active registration.
		<P>
		@param	flags
					Currently unused, reserved for future use.
		<P>
		@param	serviceName
					The service name registered (if the application did not specify a name in 
					DNSSD.register(), this indicates what name was automatically chosen).
		<P>
		@param	regType
					The type of service registered, as it was passed to DNSSD.register().
		<P>
		@param	domain
					The domain on which the service was registered. If the application did not
					specify a domain in DNSSD.register(), this is the default domain
					on which the service was registered.
	*/
	void	serviceRegistered( DNSSDRegistration registration, int flags, String serviceName, 
								String regType, String domain);
}

