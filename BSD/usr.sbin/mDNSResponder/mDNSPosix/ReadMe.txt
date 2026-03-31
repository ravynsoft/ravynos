ReadMe About mDNSPosix
----------------------

mDNSPosix is a port of Apple's Multicast DNS and DNS Service Discovery
code to Posix platforms.

Multicast DNS and DNS Service Discovery are technologies that allow you
to register IP-based services and browse the network for those services.
For more information about mDNS, see the mDNS web site.

  <http://www.multicastdns.org/>

Multicast DNS is part of a family of technologies resulting from the
efforts of the IETF Zeroconf working group.  For information about
other Zeroconf technologies, see the Zeroconf web site.

  <http://www.zeroconf.org/>

Apple uses the trade mark "Bonjour" to describe our implementation of
Zeroconf technologies.  This sample is designed to show how easy it is
to make a device "Bonjour compatible".

The "Bonjour" trade mark can also be licensed at no charge for
inclusion on your own products, packaging, manuals, promotional
materials, or web site. For details and licensing terms, see

  <http://developer.apple.com/bonjour/>

The code in this sample was compiled and tested on Mac OS X (10.1.x,
10.2, 10.3), Solaris (SunOS 5.8), Linux (Redhat 2.4.9-21, Fedora Core 1), 
and OpenBSD (2.9). YMMV.


Packing List
------------

The sample uses the following directories:

o mDNSCore -- A directory containing the core mDNS code.  This code
  is written in pure ANSI C and has proved to be very portable.
  Every platform needs this core protocol engine code.

o mDNSShared -- A directory containing useful code that's not core to
  the main protocol engine itself, but nonetheless useful, and used by
  more than one (but not necessarily all) platforms.

o mDNSPosix -- The files that are specific to Posix platforms: Linux,
  Solaris, FreeBSD, NetBSD, OpenBSD, etc. This code will also work on
  OS X, though that's not its primary purpose.

o Clients -- Example client code showing how to use the API to the
  services provided by the daemon.


Building the Code
-----------------

The sample does not use autoconf technology, primarily because I didn't
want to delay shipping while I learnt how to use it.  Thus the code
builds using a very simple make file.  To build the sample you should
cd to the mDNSPosix directory and type "make os=myos", e.g.

    make os=panther

For Linux you would change that to:

    make os=linux

There are definitions for each of the platforms I ported to.  If you're
porting to any other platform please add appropriate definitions for it
and send us the diffs so they can be incorporated into the main
distribution.


Using the Sample
----------------
When you compile, you will get:

o Main products for general-purpose use (e.g. on a desktop computer):
  - mdnsd
  - libmdns
  - nss_mdns (See nss_ReadMe.txt for important information about nss_mdns)

o Standalone products for dedicated devices (printer, network camera, etc.)
  - mDNSClientPosix
  - mDNSResponderPosix
  - mDNSProxyResponderPosix

o Testing and Debugging tools
  - dns-sd command-line tool (from the "Clients" folder)
  - mDNSNetMonitor
  - mDNSIdentify

As root type "make install" to install eight things:
o mdnsd                   (usually in /usr/sbin)
o libmdns                 (usually in /usr/lib)
o dns_sd.h                (usually in /usr/include)
o startup scripts         (e.g. in /etc/rc.d)
o manual pages            (usually in /usr/share/man)
o dns-sd tool             (usually in /usr/bin)
o nss_mdns                (usually in /lib)
o nss configuration files (usually in /etc)

The "make install" concludes by executing the startup script
(usually "/etc/init.d/mdns start") to start the daemon running.
You shouldn't need to reboot unless you really want to.

Once the daemon is running, you can use the dns-sd test tool
to exercise all the major functionality of the daemon. Running
"dns-sd" with no arguments gives a summary of the available options.
This test tool is also described in detail, with several examples,
in Chapter 6 of the O'Reilly "Zero Configuration Networking" book.


How It Works
------------
                                                   +--------------------+
                                                   | Client Application |
   +----------------+                              +--------------------+
   |  uds_daemon.c  | <--- Unix Domain Socket ---> |      libmdns       |
   +----------------+                              +--------------------+
   |    mDNSCore    |
   +----------------+
   |  mDNSPosix.c   |
   +----------------+

mdnsd is divided into three sections.

o mDNSCore is the main protocol engine
o mDNSPosix.c provides the glue it needs to run on a Posix OS
o uds_daemon.c exports a Unix Domain Socket interface to
  the services provided by mDNSCore

Client applications link with the libmdns, which implements the functions
defined in the dns_sd.h header file, and implements the IPC protocol
used to communicate over the Unix Domain Socket interface to the daemon.

Note that, strictly speaking, nss_mdns could be just another client of
mdnsd, linking with libmdns just like any other client. However, because
of its central role in the normal operation of multicast DNS, it is built
and installed along with the other essential system support components.


Clients for Embedded Systems
----------------------------

For small devices with very constrained resources, with a single address
space and (typically) no virtual memory, the uds_daemon.c/UDS/libmdns
layer may be eliminated, and the Client Application may live directly
on top of mDNSCore:

    +--------------------+
    | Client Application |
    +--------------------+
    |      mDNSCore      |
    +--------------------+
    |    mDNSPosix.c     |
    +--------------------+

Programming to this model is more work, so using the daemon and its
library is recommended if your platform is capable of that.

The runtime behaviour when using the embedded model is as follows:

1. The application calls mDNS_Init, which in turns calls the platform
   (mDNSPlatformInit).

2. mDNSPlatformInit gets a list of interfaces (get_ifi_info) and registers
   each one with the core (mDNS_RegisterInterface).  For each interface
   it also creates a multicast socket (SetupSocket).

3. The application then calls select() repeatedly to handle file descriptor
   events. Before calling select() each time, the application calls
   mDNSPosixGetFDSet() to give mDNSPosix.c a chance to add its own file
   descriptors to the set, and then after select() returns, it calls
   mDNSPosixProcessFDSet() to give mDNSPosix.c a chance to receive and
   process any packets that may have arrived.

4. When the core needs to send a UDP packet it calls
   mDNSPlatformSendUDP.  That routines finds the interface that
   corresponds to the source address requested by the core, and
   sends the datagram using the UDP socket created for the
   interface.  If the socket is flow send-side controlled it just
   drops the packet.

5. When SocketDataReady runs it uses a complex routine,
   "recvfrom_flags", to actually receive the packet.  This is required
   because the core needs information about the packet that is
   only available via the "recvmsg" call, and that call is complex
   to implement in a portable way.  I got my implementation of
   "recvfrom_flags" from Stevens' "UNIX Network Programming", but
   I had to modify it further to work with Linux.

One thing to note is that the Posix platform code is very deliberately
not multi-threaded.  I do everything from a main loop that calls
"select()".  This is good because it avoids all the problems that often
accompany multi-threaded code. If you decide to use threads in your
platform, you will have to implement the mDNSPlatformLock() and
mDNSPlatformUnlock() calls which are currently no-ops in mDNSPosix.c.


Once you've built the embedded samples you can test them by first
running the client, as shown below.

  quinn% build/mDNSClientPosix
  Hit ^C when you're bored waiting for responses.

By default the client starts a search for AppleShare servers and then
sits and waits, printing a message when services appear and disappear.

To continue with the test you should start the responder in another
shell window.

  quinn% build/mDNSResponderPosix -n Foo

This will start the responder and tell it to advertise a AppleShare
service "Foo".  In the client window you will see the client print out
the following as the service shows up on the network.

  quinn% build/mDNSClientPosix
  Hit ^C when you're bored waiting for responses.
  *** Found name = 'Foo', type = '_afpovertcp._tcp.', domain = 'local.'

Back in the responder window you can quit the responder cleanly using
SIGINT (typically ^C).

  quinn% build/mDNSResponderPosix -n Foo
  ^C
  quinn%

As the responder quits it will multicast that the "Foo" service is
disappearing and the client will see that notification and print a
message to that effect (shown below).  Finally, when you're done with
the client you can use SIGINT to quit it.

  quinn% build/mDNSClientPosix
  Hit ^C when you're bored waiting for responses.
  *** Found name = 'Foo', type = '_afpovertcp._tcp.', domain = 'local.'
  *** Lost  name = 'Foo', type = '_afpovertcp._tcp.', domain = 'local.'
  ^C
  quinn%

If things don't work, try starting each program in verbose mode (using
the "-v 1" option, or very verbose mode with "-v 2") to see if there's
an obvious cause.

That's it for the core functionality.  Each program supports a variety
of other options.  For example, you can advertise and browse for a
different service type using the "-t type" option.  Use the "-?" option
on each program for more user-level information.


Caveats
-------
Currently the program uses a simple make file.

The Multicast DNS protocol can also operate locally over the loopback
interface, but this exposed some problems with the underlying network
stack in early versions of Mac OS X and may expose problems with other
network stacks too.

o On Mac OS X 10.1.x the code failed to start on the loopback interface
  because the IP_ADD_MEMBERSHIP option returns ENOBUFS.

o On Mac OS X 10.2 the loopback-only case failed because
  "sendto" calls fails with error EHOSTUNREACH. (3016042)

Consequently, the code will attempt service discovery on the loopback
interface only if no other interfaces are available.

I haven't been able to test the loopback-only case on other platforms
because I don't have access to the physical machine.


Licencing
---------
This source code is Open Source; for details see the "LICENSE" file.

Credits and Version History
---------------------------
If you find any problems with this sample, mail <dts@apple.com> and I
will try to fix them up.

1.0a1 (Jul 2002) was a prerelease version that was distributed
internally at Apple.

1.0a2 (Jul 2002) was a prerelease version that was distributed
internally at Apple.

1.0a3 (Aug 2002) was the first shipping version.  The core mDNS code is
the code from Mac OS 10.2 (Jaguar) GM.

Share and Enjoy

Apple Developer Technical Support
Networking, Communications, Hardware

6 Aug 2002

Impact: A local network user may cause a denial of the Bonjour service
Description: An error handling issue exists in the Bonjour Namespace
Provider. A local network user may send a maliciously crafted multicast
DNS packet leading to an unexpected termination of the Bonjour service.
This update addresses the issue by performing additional validation of
multicast DNS packets. This issue does not affect systems running Mac OS
X or Windows.
CVE-ID
CVE-2011-0220 : JaeSeung Song of the Department of Computing at Imperial
College London

To Do List
----------
• port to a System V that's not Solaris
• use sig_atomic_t for signal to main thread flags
