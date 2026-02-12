What is mDNSResponder?
----------------------

The mDNSResponder project is a component of Bonjour,
Apple's ease-of-use IP networking initiative:
<http://developer.apple.com/bonjour/>

Apple's Bonjour software derives from the ongoing standardization
work of the IETF Zero Configuration Networking Working Group:
<http://zeroconf.org/>

The Zeroconf Working Group has identified three requirements for Zero
Configuration Networking:
1. An IP address (even when there is no DHCP server to assign one)
2. Name-to-address translation (even when there is no DNS server)
3. Discovery of Services on the network (again, without infrastucture)

Requirement 1 is met by self-assigned link-local addresses, as
described in "Dynamic Configuration of IPv4 Link-Local Addresses"
<http://files.zeroconf.org/draft-ietf-zeroconf-ipv4-linklocal.txt>

Requirement 2 is met by sending DNS-like queries via Multicast (mDNS).

Requirement 3 is met by DNS Service Dicsovery (DNS-SD).

Self-assigned link-local address capability has been available since
1998, when it first appeared in Windows '98 and in Mac OS 8.5.
Implementations for other platforms also exist.

The mDNSResponder project allows us to meet requirements 2 and 3.
It provides the ability for the user to identify hosts using names
instead of dotted-decimal IP addresses, even if the user doesn't have a
conventional DNS server set up. It also provides the ability for the
user to discover what services are being advertised on the network,
without having to know about them in advance, or configure the machines.

The name "mDNS" was chosen because this protocol is designed to be,
as much as possible, similar to conventional DNS. The main difference is
that queries are sent via multicast to all local hosts, instead of via
unicast to a specific known server. Every host on the local link runs an
mDNSResponder which is constantly listening for those multicast queries,
and if the mDNSResponder receives a query for which it knows the answer,
then it responds. The mDNS protocol uses the same packet format as
unicast DNS, and the same name structure, and the same DNS record types.
The main difference is that queries are sent to a different UDP port
(5353 instead of 53) and they are sent via multicast to address
224.0.0.251. Another important difference is that all "mDNS" names
end in ".local." When a user types "yourcomputer.local." into their Web
browser, the presence of ".local." on the end of the name tells the host
OS that the name should be looked up using local multicast instead of by
sending that name to the worldwide DNS service for resolution. This
helps reduce potential user confusion about whether a particular name
is globally unique (e.g. "www.apple.com.") or whether that name has only
local significance (e.g. "yourcomputer.local.").


About the mDNSResponder Code
----------------------------

Because Apple benefits more from widespread adoption of Bonjour than
it would benefit from keeping Bonjour proprietary, Apple is making
this code open so that other developers can use it too.

Because Apple recognises that networks are hetrogenous environments
where devices run many different kinds of OS, this code has been made
as portable as possible.

A typical mDNS program contains three components:

    +------------------+
    |   Application    |
    +------------------+
    |    mDNS Core     |
    +------------------+
    | Platform Support |
    +------------------+

The "mDNS Core" layer is absolutely identical for all applications and
all Operating Systems.

The "Platform Support" layer provides the necessary supporting routines
that are specific to each platform -- what routine do you call to send
a UDP packet, what routine do you call to join multicast group, etc.

The "Application" layer does whatever that particular application wants
to do. It calls routines provided by the "mDNS Core" layer to perform
the functions it needs --
 * advertise services,
 * browse for named instances of a particular type of service
 * resolve a named instance to a specific IP address and port number,
 * etc.
The "mDNS Core" layer in turn calls through to the "Platform Support"
layer to send and receive the multicast UDP packets to do the actual work.

Apple currently provides "Platform Support" layers for Mac OS 9, Mac OS X,
Microsoft Windows, and for POSIX platforms like Linux, Solaris, FreeBSD,
etc.

Note: Developers writing applications for OS X do not need to incorporate
this code into their applications, since OS X provides a system service to
handle this for them. If every application developer were to link-in the
mDNSResponder code into their application, then we would end up with a
situation like the picture below:

  +------------------+    +------------------+    +------------------+
  |   Application 1  |    |   Application 2  |    |   Application 3  |
  +------------------+    +------------------+    +------------------+
  |    mDNS Core     |    |    mDNS Core     |    |    mDNS Core     |
  +------------------+    +------------------+    +------------------+
  | Platform Support |    | Platform Support |    | Platform Support |
  +------------------+    +------------------+    +------------------+

This would not be very efficient. Each separate application would be sending
their own separate multicast UDP packets and maintaining their own list of
answers. Because of this, OS X provides a common system service which client
software should access through the "/usr/include/dns_sd.h" APIs.

The situation on OS X looks more like the picture below:

                                   -------------------
                                  /                   \
  +---------+    +------------------+    +---------+   \  +---------+
  |  App 1  |<-->|    daemon.c      |<-->|  App 2  |    ->|  App 3  |
  +---------+    +------------------+    +---------+      +---------+
                 |    mDNS Core     |
                 +------------------+
                 | Platform Support |
                 +------------------+

Applications on OS X make calls to the single mDNSResponder daemon
which implements the mDNS and DNS-SD protocols. 

Vendors of products such as printers, which are closed environments not
expecting to be running third-party application software, can reasonably
implement a single monolithic mDNSResponder to advertise all the
services of that device. Vendors of open systems which run third-party
application software should implement a system service such as the one
provided by the OS X mDNSResponder daemon, and application software on
that platform should, where possible, make use of that system service
instead of embedding their own mDNSResponder.

See ReadMe.txt in the mDNSPosix directory for specific details of
building an mDNSResponder on a POSIX Operating System.


Compiling on Older C Compilers
------------------------------

We go to some lengths to make the code portable, but //-style comments
are one of the modern conveniences we can't live without.

If your C compiler doesn't understand these comments, you can transform
them into classical K&R /* style */ comments with a quick GREP
search-and-replace pattern.

In BBEdit on the Mac:
1. Open the "Find" dialog window and make sure "Use Grep" is selected
2. Search For  : ([^:])//(.*)
3. Replace With: \1/*\2 */
4. Drag your mDNSResponder source code folder to the Multi-File search pane
5. Click "Replace All"

For the more command-line oriented, cd into your mDNSResponder source code
directory and execute the following command (all one line):

find mDNSResponder \( -name \*.c\* -or -name \*.h \) -exec sed -i .orig -e 's,^//\(.*\),/*\1 */,' -e '/\/\*/\!s,\([^:]\)//\(.*\),\1/*\2 */,' {} \;
