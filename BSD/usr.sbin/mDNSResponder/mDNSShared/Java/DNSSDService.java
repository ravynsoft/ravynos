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

/**	A tracking object for a service created by {@link DNSSD}. */

public interface	DNSSDService
{
	/**
	Halt the active operation and free resources associated with the DNSSDService.<P>

	Any services or records registered with this DNSSDService will be deregistered. Any
	Browse, Resolve, or Query operations associated with this reference will be terminated.<P>

	Note: if the service was initialized with DNSSD.register(), and an extra resource record was
	added to the service via {@link DNSSDRegistration#addRecord}, the DNSRecord so created 
	is invalidated when this method is called - the DNSRecord may not be used afterward.
	*/
	void		stop();
} 

