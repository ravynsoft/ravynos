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

	This file declares and implements DNSSD, the central Java factory class
	for doing DNS Service Discovery. It includes the mostly-abstract public
	interface, as well as the Apple* implementation subclasses.
 */


package	com.apple.dnssd;


/**
	DNSSD provides access to DNS Service Discovery features of ZeroConf networking.<P>

	It is a factory class that is used to invoke registration and discovery-related
	operations. Most operations are non-blocking; clients are called back through an interface
	with the result of an operation. Callbacks are made from a separate worker thread.<P>

	For example, in this program<P>
	<PRE><CODE>
    class   MyClient implements BrowseListener {
        void    lookForWebServers() {
            myBrowser = DNSSD.browse("_http._tcp", this);
        }
    
        public void serviceFound(DNSSDService browser, int flags, int ifIndex,
                            String serviceName, String regType, String domain) {}
        ...
    }</CODE></PRE>
	<CODE>MyClient.serviceFound()</CODE> would be called for every HTTP server discovered in the
	default browse domain(s).
*/

abstract public class	DNSSD
{
	/**	Flag indicates to a {@link BrowseListener} that another result is
		queued.  Applications should not update their UI to display browse
		results if the MORE_COMING flag is set; they will be called at least once
		more with the flag clear.
	*/
	public static final int		MORE_COMING = ( 1 << 0 );

	/** If flag is set in a {@link DomainListener} callback, indicates that the result is the default domain. */
	public static final int		DEFAULT = ( 1 << 2 );

    /**	If flag is set, a name conflict will trigger an exception when registering non-shared records.<P>
    	A name must be explicitly specified when registering a service if this bit is set
    	(i.e. the default name may not not be used).
     */
	public static final int		NO_AUTO_RENAME = ( 1 << 3 );

	/**	If flag is set, allow multiple records with this name on the network (e.g. PTR records)
		when registering individual records on a {@link DNSSDRegistration}.
	*/
	public static final int		SHARED = ( 1 << 4 );

	/**	If flag is set, records with this name must be unique on the network (e.g. SRV records). */
	public static final int		UNIQUE = ( 1 << 5 );

	/** Set flag when calling enumerateDomains() to restrict results to domains recommended for browsing. */
	public static final int		BROWSE_DOMAINS = ( 1 << 6 );
	/** Set flag when calling enumerateDomains() to restrict results to domains recommended for registration. */
	public static final int		REGISTRATION_DOMAINS = ( 1 << 7 );

	/** Maximum length, in bytes, of a domain name represented as an escaped C-String. */
    public static final int     MAX_DOMAIN_NAME = 1009;

	/** Pass for ifIndex to specify all available interfaces. */
    public static final int     ALL_INTERFACES = 0;

	/** Pass for ifIndex to specify the localhost interface. */
    public static final int     LOCALHOST_ONLY = -1;

	/** Browse for instances of a service.<P>

		Note: browsing consumes network bandwidth. Call {@link DNSSDService#stop} when you have finished browsing.<P>

		@param	flags
					Currently ignored, reserved for future use.
		<P>
		@param	ifIndex
					If non-zero, specifies the interface on which to browse for services
					(the index for a given interface is determined via the if_nametoindex()
					family of calls.)  Most applications will pass 0 to browse on all available
					interfaces.  Pass -1 to only browse for services provided on the local host.
		<P>
		@param	regType
					The registration type being browsed for followed by the protocol, separated by a
					dot (e.g. "_ftp._tcp"). The transport protocol must be "_tcp" or "_udp".
		<P>
		@param	domain
					If non-null, specifies the domain on which to browse for services.
					Most applications will not specify a domain, instead browsing on the
					default domain(s).
		<P>
		@param	listener
					This object will get called when instances of the service are discovered (or disappear).
		<P>
		@return		A {@link DNSSDService} that represents the active browse operation.

		@throws SecurityException If a security manager is present and denies <tt>RuntimePermission("getDNSSDInstance")</tt>.
		@see    RuntimePermission
	*/
	public static DNSSDService	browse( int flags, int ifIndex, String regType, String domain, BrowseListener listener)
	throws DNSSDException
	{ return getInstance()._makeBrowser( flags, ifIndex, regType, domain, listener); }

	/** Browse for instances of a service. Use default flags, ifIndex and domain.<P>

		@param	regType
					The registration type being browsed for followed by the protocol, separated by a
					dot (e.g. "_ftp._tcp"). The transport protocol must be "_tcp" or "_udp".
		<P>
		@param	listener
					This object will get called when instances of the service are discovered (or disappear).
		<P>
		@return		A {@link DNSSDService} that represents the active browse operation.

		@throws SecurityException If a security manager is present and denies <tt>RuntimePermission("getDNSSDInstance")</tt>.
		@see    RuntimePermission
	*/
	public static DNSSDService	browse( String regType, BrowseListener listener)
	throws DNSSDException
	{ return browse( 0, 0, regType, "", listener); }

	/** Resolve a service name discovered via browse() to a target host name, port number, and txt record.<P>
		
		Note: Applications should NOT use resolve() solely for txt record monitoring - use
		queryRecord() instead, as it is more efficient for this task.<P>
		
		Note: When the desired results have been returned, the client MUST terminate the resolve by
		calling {@link DNSSDService#stop}.<P>

		Note: resolve() behaves correctly for typical services that have a single SRV record and
 		a single TXT record (the TXT record may be empty.)  To resolve non-standard services with
 		multiple SRV or TXT records, use queryRecord().<P>

		@param	flags
					Currently ignored, reserved for future use.
		<P>
		@param	ifIndex
					The interface on which to resolve the service.  The client should
					pass the interface on which the serviceName was discovered (i.e.
					the ifIndex passed to the serviceFound() callback)
					or 0 to resolve the named service on all available interfaces.
		<P>
		@param	serviceName
					The servicename to be resolved.
		<P>
		@param	regType
					The registration type being resolved followed by the protocol, separated by a
					dot (e.g. "_ftp._tcp").  The transport protocol must be "_tcp" or "_udp".
		<P>
		@param	domain
					The domain on which the service is registered, i.e. the domain passed
					to the serviceFound() callback.
		<P>
		@param	listener
					This object will get called when the service is resolved.
		<P>
		@return		A {@link DNSSDService} that represents the active resolve operation.

		@throws SecurityException If a security manager is present and denies <tt>RuntimePermission("getDNSSDInstance")</tt>.
		@see    RuntimePermission
	*/
	public static DNSSDService	resolve( int flags, int ifIndex, String serviceName, String regType,
										String domain, ResolveListener listener)
	throws DNSSDException
	{ return getInstance()._resolve( flags, ifIndex, serviceName, regType, domain, listener); }

	/** Register a service, to be discovered via browse() and resolve() calls.<P>
		@param	flags
					Possible values are: NO_AUTO_RENAME.
		<P>
		@param	ifIndex
					If non-zero, specifies the interface on which to register the service
					(the index for a given interface is determined via the if_nametoindex()
					family of calls.)  Most applications will pass 0 to register on all
					available interfaces.  Pass -1 to register a service only on the local
					machine (service will not be visible to remote hosts).
		<P>
		@param	serviceName
					If non-null, specifies the service name to be registered.
					Applications need not specify a name, in which case the
					computer name is used (this name is communicated to the client via
					the serviceRegistered() callback).
		<P>
		@param	regType
					The registration type being registered followed by the protocol, separated by a
					dot (e.g. "_ftp._tcp").  The transport protocol must be "_tcp" or "_udp".
		<P>
		@param	domain
					If non-null, specifies the domain on which to advertise the service.
					Most applications will not specify a domain, instead automatically
					registering in the default domain(s).
		<P>
		@param	host
					If non-null, specifies the SRV target host name.  Most applications
					will not specify a host, instead automatically using the machine's
					default host name(s).  Note that specifying a non-null host does NOT
					create an address record for that host - the application is responsible
					for ensuring that the appropriate address record exists, or creating it
					via {@link DNSSDRegistration#addRecord}.
		<P>
		@param	port
					The port on which the service accepts connections.  Pass 0 for a
					"placeholder" service (i.e. a service that will not be discovered by
					browsing, but will cause a name conflict if another client tries to
					register that same name.)  Most clients will not use placeholder services.
		<P>
		@param	txtRecord
					The txt record rdata.  May be null.  Note that a non-null txtRecord
					MUST be a properly formatted DNS TXT record, i.e. &lt;length byte&gt; &lt;data&gt;
					&lt;length byte&gt; &lt;data&gt; ...
		<P>
		@param	listener
					This object will get called when the service is registered.
		<P>
		@return		A {@link DNSSDRegistration} that controls the active registration.

		@throws SecurityException If a security manager is present and denies <tt>RuntimePermission("getDNSSDInstance")</tt>.
		@see    RuntimePermission
	*/
	public static DNSSDRegistration	register( int flags, int ifIndex, String serviceName, String regType,
									String domain, String host, int port, TXTRecord txtRecord, RegisterListener listener)
	throws DNSSDException
	{ return getInstance()._register( flags, ifIndex, serviceName, regType, domain, host, port, txtRecord, listener); }

	/** Register a service, to be discovered via browse() and resolve() calls. Use default flags, ifIndex, domain, host and txtRecord.<P>
		@param	serviceName
					If non-null, specifies the service name to be registered.
					Applications need not specify a name, in which case the
					computer name is used (this name is communicated to the client via
					the serviceRegistered() callback).
		<P>
		@param	regType
					The registration type being registered followed by the protocol, separated by a
					dot (e.g. "_ftp._tcp").  The transport protocol must be "_tcp" or "_udp".
		<P>
		@param	port
					The port on which the service accepts connections.  Pass 0 for a
					"placeholder" service (i.e. a service that will not be discovered by
					browsing, but will cause a name conflict if another client tries to
					register that same name.)  Most clients will not use placeholder services.
		<P>
		@param	listener
					This object will get called when the service is registered.
		<P>
		@return		A {@link DNSSDRegistration} that controls the active registration.

		@throws SecurityException If a security manager is present and denies <tt>RuntimePermission("getDNSSDInstance")</tt>.
		@see    RuntimePermission
	*/
	public static DNSSDRegistration	register( String serviceName, String regType, int port, RegisterListener listener)
	throws DNSSDException
	{ return register( 0, 0, serviceName, regType, null, null, port, null, listener); }

	/** Create a {@link DNSSDRecordRegistrar} allowing efficient registration of
		multiple individual records.<P>
		<P>
		@return		A {@link DNSSDRecordRegistrar} that can be used to register records.

		@throws SecurityException If a security manager is present and denies <tt>RuntimePermission("getDNSSDInstance")</tt>.
		@see    RuntimePermission
	*/
	public static DNSSDRecordRegistrar	createRecordRegistrar( RegisterRecordListener listener)
	throws DNSSDException
	{ return getInstance()._createRecordRegistrar( listener); }

	/** Query for an arbitrary DNS record.<P>
		@param	flags
					Possible values are: MORE_COMING.
		<P>
		@param	ifIndex
					If non-zero, specifies the interface on which to issue the query
					(the index for a given interface is determined via the if_nametoindex()
					family of calls.)  Passing 0 causes the name to be queried for on all
					interfaces.  Passing -1 causes the name to be queried for only on the
					local host.
		<P>
		@param	serviceName
					The full domain name of the resource record to be queried for.
		<P>
		@param	rrtype
					The numerical type of the resource record to be queried for (e.g. PTR, SRV, etc)
					as defined in nameser.h.
		<P>
		@param	rrclass
					The class of the resource record, as defined in nameser.h
					(usually 1 for the Internet class).
		<P>
		@param	listener
					This object will get called when the query completes.
		<P>
		@return		A {@link DNSSDService} that controls the active query.

		@throws SecurityException If a security manager is present and denies <tt>RuntimePermission("getDNSSDInstance")</tt>.
		@see    RuntimePermission
	*/
	public static DNSSDService	queryRecord( int flags, int ifIndex, String serviceName, int rrtype,
										int rrclass, QueryListener listener)
	throws DNSSDException
	{ return getInstance()._queryRecord( flags, ifIndex, serviceName, rrtype, rrclass, listener); }

	/** Asynchronously enumerate domains available for browsing and registration.<P>
	
		Currently, the only domain returned is "local.", but other domains will be returned in future.<P>
		
		The enumeration MUST be cancelled by calling {@link DNSSDService#stop} when no more domains
		are to be found.<P>
		@param	flags
					Possible values are: BROWSE_DOMAINS, REGISTRATION_DOMAINS.
		<P>
		@param	ifIndex
					If non-zero, specifies the interface on which to look for domains.
					(the index for a given interface is determined via the if_nametoindex()
					family of calls.)  Most applications will pass 0 to enumerate domains on
					all interfaces.
		<P>
		@param	listener
					This object will get called when domains are found.
		<P>
		@return		A {@link DNSSDService} that controls the active enumeration.

		@throws SecurityException If a security manager is present and denies <tt>RuntimePermission("getDNSSDInstance")</tt>.
		@see    RuntimePermission
	*/
	public static DNSSDService	enumerateDomains( int flags, int ifIndex, DomainListener listener)
	throws DNSSDException
	{ return getInstance()._enumerateDomains( flags, ifIndex, listener); }

	/**	Concatenate a three-part domain name (as provided to the listeners) into a
		properly-escaped full domain name. Note that strings passed to listeners are
		ALREADY ESCAPED where necessary.<P>
		@param	serviceName
					The service name - any dots or slashes must NOT be escaped.
					May be null (to construct a PTR record name, e.g. "_ftp._tcp.apple.com").
		<P>
		@param	regType
					The registration type followed by the protocol, separated by a dot (e.g. "_ftp._tcp").
		<P>
		@param	domain
					The domain name, e.g. "apple.com".  Any literal dots or backslashes must be escaped.
		<P>
		@return		The full domain name.

		@throws SecurityException If a security manager is present and denies <tt>RuntimePermission("getDNSSDInstance")</tt>.
		@see    RuntimePermission
	*/
	public static String		constructFullName( String serviceName, String regType, String domain)
	throws DNSSDException
	{ return getInstance()._constructFullName( serviceName, regType, domain); }

	/** Instruct the daemon to verify the validity of a resource record that appears to
		be out of date. (e.g. because tcp connection to a service's target failed.) <P>
		
		Causes the record to be flushed from the daemon's cache (as well as all other
		daemons' caches on the network) if the record is determined to be invalid.<P>
		@param	flags
					Currently unused, reserved for future use.
		<P>
		@param	ifIndex
					If non-zero, specifies the interface on which to reconfirm the record
					(the index for a given interface is determined via the if_nametoindex()
					family of calls.)  Passing 0 causes the name to be reconfirmed on all
					interfaces.  Passing -1 causes the name to be reconfirmed only on the
					local host.
		<P>
		@param	fullName
					The resource record's full domain name.
		<P>
		@param	rrtype
					The resource record's type (e.g. PTR, SRV, etc) as defined in nameser.h.
		<P>
		@param	rrclass
					The class of the resource record, as defined in nameser.h (usually 1).
		<P>
		@param	rdata
					The raw rdata of the resource record.

		@throws SecurityException If a security manager is present and denies <tt>RuntimePermission("getDNSSDInstance")</tt>.
		@see    RuntimePermission
	*/
	public static void		reconfirmRecord( int flags, int ifIndex, String fullName, int rrtype,
										int rrclass, byte[] rdata)
	{ getInstance()._reconfirmRecord( flags, ifIndex, fullName, rrtype, rrclass, rdata); }

	/** Return the canonical name of a particular interface index.<P>
		@param	ifIndex
					A valid interface index. Must not be ALL_INTERFACES.
		<P>
		@return		The name of the interface, which should match java.net.NetworkInterface.getName().

		@throws SecurityException If a security manager is present and denies <tt>RuntimePermission("getDNSSDInstance")</tt>.
		@see    RuntimePermission
	*/
	public static String	getNameForIfIndex( int ifIndex)
	{ return getInstance()._getNameForIfIndex( ifIndex); }

	/** Return the index of a named interface.<P>
		@param	ifName
					A valid interface name. An example is java.net.NetworkInterface.getName().
		<P>
		@return		The interface index.

		@throws SecurityException If a security manager is present and denies <tt>RuntimePermission("getDNSSDInstance")</tt>.
		@see    RuntimePermission
	*/
	public static int		getIfIndexForName( String ifName)
	{ return getInstance()._getIfIndexForName( ifName); }

	protected						DNSSD() {}	// prevent direct instantiation

	/** Return the single instance of DNSSD. */
	static protected final DNSSD	getInstance()
	{
		SecurityManager sm = System.getSecurityManager();
        if (sm != null)
            sm.checkPermission( new RuntimePermission( "getDNSSDInstance"));
		 return fInstance;
	}

	abstract protected DNSSDService	_makeBrowser( int flags, int ifIndex, String regType, String domain, BrowseListener listener)
	throws DNSSDException;

	abstract protected DNSSDService	_resolve( int flags, int ifIndex, String serviceName, String regType,
										String domain, ResolveListener listener)
	throws DNSSDException;

	abstract protected DNSSDRegistration	_register( int flags, int ifIndex, String serviceName, String regType,
									String domain, String host, int port, TXTRecord txtRecord, RegisterListener listener)
	throws DNSSDException;

	abstract protected DNSSDRecordRegistrar	_createRecordRegistrar( RegisterRecordListener listener)
	throws DNSSDException;

	abstract protected DNSSDService	_queryRecord( int flags, int ifIndex, String serviceName, int rrtype,
										int rrclass, QueryListener listener)
	throws DNSSDException;

	abstract protected DNSSDService	_enumerateDomains( int flags, int ifIndex, DomainListener listener)
	throws DNSSDException;

	abstract protected String		_constructFullName( String serviceName, String regType, String domain)
	throws DNSSDException;

	abstract protected void			_reconfirmRecord( int flags, int ifIndex, String fullName, int rrtype,
										int rrclass, byte[] rdata);

	abstract protected String		_getNameForIfIndex( int ifIndex);

	abstract protected int			_getIfIndexForName( String ifName);

	protected static DNSSD			fInstance;

	static
	{
		try
		{
			String name = System.getProperty( "com.apple.dnssd.DNSSD" );
			if (name == null)
				name = "com.apple.dnssd.AppleDNSSD";	// Fall back to Apple-provided class.
			fInstance = (DNSSD) Class.forName(name).newInstance();
		}
		catch( Exception e )
		{
			throw new InternalError( "cannot instantiate DNSSD" + e );
		}
	}
}


// Concrete implementation of DNSSDException
class	AppleDNSSDException extends DNSSDException
{
	public						AppleDNSSDException( int errorCode) { fErrorCode = errorCode; }

	public int					getErrorCode() { return fErrorCode; }

	public String				getMessage()
	{
		final String	kMessages[] = {		// should probably be put into a resource or something
			"UNKNOWN",
			"NO_SUCH_NAME",
			"NO_MEMORY",
			"BAD_PARAM",
			"BAD_REFERENCE",
			"BAD_STATE",
			"BAD_FLAGS",
			"UNSUPPORTED",
			"NOT_INITIALIZED",
			"NO_CACHE",
			"ALREADY_REGISTERED",
			"NAME_CONFLICT",
			"INVALID",
			"FIREWALL",
			"INCOMPATIBLE",
			"BAD_INTERFACE_INDEX",
			"REFUSED",
			"NOSUCHRECORD",
			"NOAUTH",
			"NOSUCHKEY",
			"NATTRAVERSAL",
			"DOUBLENAT",
			"BADTIME",
			"BADSIG",
			"BADKEY",
			"TRANSIENT",
			"SERVICENOTRUNNING",
			"NATPORTMAPPINGUNSUPPORTED",
			"NATPORTMAPPINGDISABLED"
		};
	
		if (fErrorCode <= UNKNOWN && fErrorCode > ( UNKNOWN - kMessages.length))
		{
			return "DNS-SD Error " + String.valueOf( fErrorCode) + ": " + kMessages[ UNKNOWN - fErrorCode];
		}
		else
			return super.getMessage() + "(" + String.valueOf( fErrorCode) + ")";
	}

	protected int			fErrorCode;
}

// The concrete, default implementation.
class	AppleDNSSD extends DNSSD
{
	static
	{
		System.loadLibrary( "jdns_sd");
	
		int		libInitResult = InitLibrary( 2);	// Current version number (must be sync'd with jnilib version)

		if (libInitResult != DNSSDException.NO_ERROR)
			throw new InternalError( "cannot instantiate DNSSD: " + new AppleDNSSDException( libInitResult).getMessage());
	}

	static public boolean	hasAutoCallbacks;	// Set by InitLibrary() to value of AUTO_CALLBACKS

	protected DNSSDService	_makeBrowser( int flags, int ifIndex, String regType, String domain, BrowseListener client)
	throws DNSSDException
	{
		return new AppleBrowser( flags, ifIndex, regType, domain, client);
	}

	protected DNSSDService	_resolve( int flags, int ifIndex, String serviceName, String regType,
										String domain, ResolveListener client)
	throws DNSSDException
	{
		return new AppleResolver( flags, ifIndex, serviceName, regType, domain, client);
	}

	protected DNSSDRegistration	_register( int flags, int ifIndex, String serviceName, String regType,
									String domain, String host, int port, TXTRecord txtRecord, RegisterListener client)
	throws DNSSDException
	{
		return new AppleRegistration( flags, ifIndex, serviceName, regType, domain, host, port,
										( txtRecord != null) ? txtRecord.getRawBytes() : null, client);
	}

	protected DNSSDRecordRegistrar	_createRecordRegistrar( RegisterRecordListener listener)
	throws DNSSDException
	{
		return new AppleRecordRegistrar( listener);
	}

	protected DNSSDService		_queryRecord( int flags, int ifIndex, String serviceName, int rrtype,
										int rrclass, QueryListener client)
	throws DNSSDException
	{
		return new AppleQuery( flags, ifIndex, serviceName, rrtype, rrclass, client);
	}

	protected DNSSDService		_enumerateDomains( int flags, int ifIndex, DomainListener listener)
	throws DNSSDException
	{
		return new AppleDomainEnum( flags, ifIndex, listener);
	}

	protected String			_constructFullName( String serviceName, String regType, String domain)
	throws DNSSDException
	{
		String[]	responseHolder = new String[1];	// lame maneuver to get around Java's lack of reference parameters

		int rc = ConstructName( serviceName, regType, domain, responseHolder);
		if (rc != 0)
			throw new AppleDNSSDException( rc);

		return responseHolder[0];
	}

	protected void				_reconfirmRecord( int flags, int ifIndex, String fullName, int rrtype,
										int rrclass, byte[] rdata)
	{
		ReconfirmRecord( flags, ifIndex, fullName, rrtype, rrclass, rdata);
	}

	protected String			_getNameForIfIndex( int ifIndex)
	{
		return GetNameForIfIndex( ifIndex);
	}

	protected int				_getIfIndexForName( String ifName)
	{
		return GetIfIndexForName( ifName);
	}


	protected native int	ConstructName( String serviceName, String regType, String domain, String[] pOut);

	protected native void	ReconfirmRecord( int flags, int ifIndex, String fullName, int rrtype,
										int rrclass, byte[] rdata);

	protected native String	GetNameForIfIndex( int ifIndex);

	protected native int	GetIfIndexForName( String ifName);

	protected static native int	InitLibrary( int callerVersion);
}

class	AppleService implements DNSSDService, Runnable
{
	public					AppleService(BaseListener listener)	{ fNativeContext = 0; fListener = listener; }

	public void				stop() { this.HaltOperation(); }

	/* Block until data arrives, or one second passes. Returns 1 if data present, 0 otherwise. */
	protected native int	BlockForData();

	/* Call ProcessResults when data appears on socket descriptor. */
	protected native int	ProcessResults();

	protected synchronized native void HaltOperation();

	protected void			ThrowOnErr( int rc) throws DNSSDException
	{
		if (rc != 0)
			throw new AppleDNSSDException( rc);
	}

	protected long	/* warning */	fNativeContext;		// Private storage for native side

	public void		run()
	{
		while ( true )
		{
			// Note: We want to allow our DNS-SD operation to be stopped from other threads, so we have to
			// block waiting for data *outside* the synchronized section. Because we're doing this unsynchronized
			// we have to write some careful code. Suppose our DNS-SD operation is stopped from some other thread,
			// and then immediately afterwards that thread (or some third, unrelated thread) starts a new DNS-SD
			// operation. The Unix kernel always allocates the lowest available file descriptor to a new socket,
			// so the same file descriptor is highly likely to be reused for the new operation, and if our old
			// stale ServiceThread accidentally consumes bytes off that new socket we'll get really messed up.
			// To guard against that, before calling ProcessResults we check to ensure that our
			// fNativeContext has not been deleted, which is a telltale sign that our operation was stopped.
			// After calling ProcessResults we check again, because it's extremely common for callback
			// functions to stop their own operation and start others. For example, a resolveListener callback
			// may well stop the resolve and then start a QueryRecord call to monitor the TXT record.
			//
			// The remaining risk is that between our checking fNativeContext and calling ProcessResults(),
			// some other thread could stop the operation and start a new one using same file descriptor, and
			// we wouldn't know. To prevent this, the AppleService object's HaltOperation() routine is declared
			// synchronized and we perform our checks synchronized on the AppleService object, which ensures
			// that HaltOperation() can't execute while we're doing it. Because Java locks are re-entrant this
			// locking DOESN'T prevent the callback routine from stopping its own operation, but DOES prevent
			// any other thread from stopping it until after the callback has completed and returned to us here.

			int result = BlockForData();
			synchronized (this)
			{
				if (fNativeContext == 0) break;	// Some other thread stopped our DNSSD operation; time to terminate this thread
				if (result == 0) continue;		// If BlockForData() said there was no data, go back and block again
				result = ProcessResults();
				if (fNativeContext == 0) break;	// Event listener stopped its own DNSSD operation; terminate this thread
				if (result != 0) { fListener.operationFailed(this, result); break; }	// If error, notify listener
			}
		}
	}

	protected BaseListener fListener;
}


class	AppleBrowser extends AppleService
{
	public			AppleBrowser( int flags, int ifIndex, String regType, String domain, BrowseListener client)
	throws DNSSDException
	{
		super(client);
		this.ThrowOnErr( this.CreateBrowser( flags, ifIndex, regType, domain));
		if (!AppleDNSSD.hasAutoCallbacks)
			new Thread(this).start();
	}

	// Sets fNativeContext. Returns non-zero on error.
	protected native int	CreateBrowser( int flags, int ifIndex, String regType, String domain);
}

class	AppleResolver extends AppleService
{
	public			AppleResolver( int flags, int ifIndex, String serviceName, String regType,
									String domain, ResolveListener client)
	throws DNSSDException
	{
		super(client);
		this.ThrowOnErr( this.CreateResolver( flags, ifIndex, serviceName, regType, domain));
		if (!AppleDNSSD.hasAutoCallbacks)
			new Thread(this).start();
	}

	// Sets fNativeContext. Returns non-zero on error.
	protected native int	CreateResolver( int flags, int ifIndex, String serviceName, String regType,
											String domain);
}

// An AppleDNSRecord is a simple wrapper around a dns_sd DNSRecord.
class	AppleDNSRecord implements DNSRecord
{
	public			AppleDNSRecord( AppleService owner)
	{
		fOwner = owner;
		fRecord = 0; 		// record always starts out empty
	}

	public void			update( int flags, byte[] rData, int ttl)
	throws DNSSDException
	{
		this.ThrowOnErr( this.Update( flags, rData, ttl));
	}

	public void			remove()
	throws DNSSDException
	{
		this.ThrowOnErr( this.Remove());
	}

	protected long			fRecord;		// Really a DNSRecord; sizeof(long) == sizeof(void*) ?
	protected AppleService	fOwner;

	protected void			ThrowOnErr( int rc) throws DNSSDException
	{
		if (rc != 0)
			throw new AppleDNSSDException( rc);
	}

	protected native int	Update( int flags, byte[] rData, int ttl);

	protected native int	Remove();
}

class	AppleRegistration extends AppleService implements DNSSDRegistration
{
	public			AppleRegistration( int flags, int ifIndex, String serviceName, String regType, String domain,
								String host, int port, byte[] txtRecord, RegisterListener client)
	throws DNSSDException
	{
		super(client);
		this.ThrowOnErr( this.BeginRegister( ifIndex, flags, serviceName, regType, domain, host, port, txtRecord));
		if (!AppleDNSSD.hasAutoCallbacks)
			new Thread(this).start();
	}

	public DNSRecord	addRecord( int flags, int rrType, byte[] rData, int ttl)
	throws DNSSDException
	{
		AppleDNSRecord	newRecord = new AppleDNSRecord( this);

		this.ThrowOnErr( this.AddRecord( flags, rrType, rData, ttl, newRecord));
		return newRecord;
	}

	public DNSRecord	getTXTRecord()
	throws DNSSDException
	{
		return new AppleDNSRecord( this);	// A record with ref 0 is understood to be primary TXT record
	}

	// Sets fNativeContext. Returns non-zero on error.
	protected native int	BeginRegister( int ifIndex, int flags, String serviceName, String regType,
											String domain, String host, int port, byte[] txtRecord);

	// Sets fNativeContext. Returns non-zero on error.
	protected native int	AddRecord( int flags, int rrType, byte[] rData, int ttl, AppleDNSRecord destObj);
}

class	AppleRecordRegistrar extends AppleService implements DNSSDRecordRegistrar
{
	public			AppleRecordRegistrar( RegisterRecordListener listener)
	throws DNSSDException
	{
		super(listener);
		this.ThrowOnErr( this.CreateConnection());
		if (!AppleDNSSD.hasAutoCallbacks)
			new Thread(this).start();
	}

	public DNSRecord	registerRecord( int flags, int ifIndex, String fullname, int rrtype,
									int rrclass, byte[] rdata, int ttl)
	throws DNSSDException
	{
		AppleDNSRecord	newRecord = new AppleDNSRecord( this);

		this.ThrowOnErr( this.RegisterRecord( flags, ifIndex, fullname, rrtype, rrclass, rdata, ttl, newRecord));
		return newRecord;
	}

	// Sets fNativeContext. Returns non-zero on error.
	protected native int	CreateConnection();

	// Sets fNativeContext. Returns non-zero on error.
	protected native int	RegisterRecord( int flags, int ifIndex, String fullname, int rrtype,
										int rrclass, byte[] rdata, int ttl, AppleDNSRecord destObj);
}

class	AppleQuery extends AppleService
{
	public			AppleQuery( int flags, int ifIndex, String serviceName, int rrtype,
										int rrclass, QueryListener client)
	throws DNSSDException
	{
		super(client);
		this.ThrowOnErr( this.CreateQuery( flags, ifIndex, serviceName, rrtype, rrclass));
		if (!AppleDNSSD.hasAutoCallbacks)
			new Thread(this).start();
	}

	// Sets fNativeContext. Returns non-zero on error.
	protected native int	CreateQuery( int flags, int ifIndex, String serviceName, int rrtype, int rrclass);
}

class	AppleDomainEnum extends AppleService
{
	public			AppleDomainEnum( int flags, int ifIndex, DomainListener client)
	throws DNSSDException
	{
		super(client);
		this.ThrowOnErr( this.BeginEnum( flags, ifIndex));
		if (!AppleDNSSD.hasAutoCallbacks)
			new Thread(this).start();
	}

	// Sets fNativeContext. Returns non-zero on error.
	protected native int	BeginEnum( int flags, int ifIndex);
}


