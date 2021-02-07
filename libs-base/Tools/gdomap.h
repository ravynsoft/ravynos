/* Include for communications with GNUstep Distributed Objects name server.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: October 1996

   This file is part of the GNUstep Project.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   You should have received a copy of the GNU General Public
   License along with this program; see the file COPYING.
   If not, write to the Free Software Foundation,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

   */

/*
 *	About the GNU Distributed Objects name-server
 *
 *	The name server is intended to work with both the UDP and the TCP
 *	protocols.  It is intended that the TCP interface be used by
 *	GNUstep programs, while the UDP interface is intended primarily
 *	for communication between name servers on different machines.
 *
 *	The communications protocol is identical for both TCP and UDP and
 *	consists of a simple request-response sequence.
 *
 *	Each request is a single message consisting of -
 *		a single byte request type,
 *		a single byte giving name length,
 *		a single byte specifying the type of port being registered
 *		or looked up, or a nul byte for probe operations.
 *		a single nul byte.
 *		a four byte port number in network byte order must be
 *		present for register operations, otherwise this is zero.
 *		a service name of 0 to GDO_NAME_MAX_LEN bytes (or two IP
 *		addresses in network byte order and an optional list of
 *		additional addresseso for probe operations)
 *		0 to GDO_NAME_MAX_LEN nul bytes padding the service name to its
 *		full size.
 *		a terminating nul byte.
 *		The total is always sent in a packet with everything after the
 *		service name (except the final byte) cleared to nul bytes.
 *
 *	Each response consists of at least 4 bytes and depends on the
 *	corresponding request type and where it came from as follows -
 *
 *	Request type	Effect
 *
 *	GDO_LOOKUP	Looks up the server name and returns its port number.
 *			Response is the port number in network byte order,
 *			or zero if the named server was not registered.
 *
 *	GDO_REGISTER	Registers the given server name with a port number.
 *			This service is only available to processes on the
 *			same host as the name server.
 *			Response is the port number in network byte order,
 *			or zero if the named server was already registered.
 *
 *	GDO_UNREG	Un-register the server name and return old port number.
 *			If the server name is of length zero, and the port is
 *			non-zero then all names for the port are unregistered.
 *			This service is only available to a process on the
 *			same host as this name server.
 *			Response is the old port number in network byte order,
 *			or zero if the name could not be un-registered.
 *			If multiple names were unregistered the response is
 *			the port for those names.
 *
 *	GDO_SERVERS	Return a list of the known servers on the local net.
 *			Response is an unsigned long (in network byte order)
 *			saying how many servers the name server knows about,
 *			followed by a list of their IP addresses in network
 *			byte order.
 *			NB. This response may not be possible over UDP as the
 *			response length may exceed the maximum UDP packet size.
 *
 *	GDO_NAMES	Return a list of registered names known to the server.
 *			Response is an unsigned long (in network byte order)
 *			saying how many bytes of data are to follow,
 *			followed by a list of the names each preceded by the
 *			name length (a single byte) and port type (a byte).
 *			NB. This response may not be possible over UDP as the
 *			response length may exceed the maximum UDP packet size.
 *
 *	The following are used for communications between name servers -
 *
 *	GDO_PROBE	Requests a response
 *			Passes two IP addresses in the name field - first the
 *			address of the sender, next that of the recipient.
 *			The packet may (optionally) include a variable number
 *			of addresses (as specified by the name length minus the
 *			size of the two addresses), each of which is an internet
 *			address on which the sender is also listening.
 *			For a request from a name server via UDP there is no
 *			response, but a GDO_REPLY request is sent.
 *			For a request from a non-name-server, or a TCP
 *			connect, the response is the port number of this
 *			server in network byte order.
 *
 *	GDO_PREPLY	Replies to a GDO_PROBE via UDP from a name server.
 *			The format of the message is as for GDO_PROBE.
 *			No response is sent.
 *
 *
 *	HOW IT WORKS AND WHY (implementation notes)
 *
 *	1.  The fixed size of a request packet was chosen for maximum
 *	    ease and speed of implementation of a non-blocking name server.
 *	    The server knows how much it needs to read and can therefore
 *	    usually do a read as a single operation since it doesn't have
 *	    to read a little, figure out request length, allocate a buffer,
 *	    and read the rest.
 *
 *	    The server name length (bytes) is specified - no assumptions
 *	    should be made about whether the name contains nul characters
 *	    or indeed about the name at all.  This is future-proofing.
 *
 *	2.  Why UDP as well as TCP?
 *	    The OpenStep specification says that a connection may be
 *	    established to any host on the local network which supplys a
 *	    named service if the host name is specified as '*'
 *
 *	    This means that the application must poll to see if it can
 *	    find a server with the name it wants.  The polling could take
 *	    a huge amount of time!
 *
 *	    To make this all easier - the server is capable of supplying
 *	    a list of those hosts on the local network which it knows to
 *	    have (or have had) a name server running on them.
 *
 *	    The application then need only poll those name servers to find
 *	    the service it wants.
 *
 *	    However - to give the application a list of hosts, the name
 *	    server must have got the information from somewhere.
 *	    To gather the information the server has to poll the machines
 *	    on the net which would take ages using TCP since attempts to
 *	    talk to machines which are down or do not exist will take a
 *	    while to time out.
 *
 *	    To make things speedy, the server sends out GDO_PROBE requests
 *	    on UDP to all the machines on the net when it starts up.
 *	    Each machine which has a name server notes that the new name
 *	    server has started up and sends back a GDOPREPLY packet so
 *	    that the new name server will know about it.
 *
 *	    Things are never perfect though - if a name server dies, the
 *	    other name servers won't know, and will continute to tell
 *	    applications that it is there.
 *
 *	3.  Port type codes - these are used to say what the port is for so
 *	    that clients can look up only the names that are relevant to them.
 *	    This is to permit the name server to be used for multiple
 *	    communications protocols (at the moment, tcp or udp) and for
 *	    different systems (distributed objects or others).
 *	    This guarantees that if one app is using DO over UDP, its services
 *	    will not be found by an app which is using DO over TCP.
 */

#define	GDOMAP_PORT	(538)	/* The well-known port for name server.	*/

/*
 *	Request type codes
 */
#define	GDO_REGISTER	'R'
#define	GDO_LOOKUP	'L'
#define	GDO_UNREG	'U'
#define	GDO_SERVERS	'S'
#define	GDO_PROBE	'P'
#define	GDO_PREPLY	'p'
#define	GDO_NAMES	'N'

/*
 *	Port type codes
 */
#define	GDO_NET_MASK	0x70	/* Network protocol of port.		*/
#define	GDO_NET_TCP	0x10
#define	GDO_NET_UDP	0x20
#define	GDO_SVC_MASK	0x0f	/* High level protocol of port.		*/
#define	GDO_SVC_GDO	0x01
#define	GDO_SVC_FOREIGN	0x02

/* tcp/ip distributed object server.	*/
#define	GDO_TCP_GDO	(GDO_NET_TCP|GDO_SVC_GDO)

/* udp/ip distributed object server.	*/
#define	GDO_UDP_GDO	(GDO_NET_UDP|GDO_SVC_GDO)

/* tcp/ip simple socket connection.	*/
#define	GDO_TCP_FOREIGN	(GDO_NET_TCP|GDO_SVC_FOREIGN)

/* udp/ip simple socket connection.	*/
#define	GDO_UDP_FOREIGN	(GDO_NET_UDP|GDO_SVC_FOREIGN)


#define	GDO_NAME_MAX_LEN	255	/* Max length registered name.	*/

/*
 *	Structure to hold a request.
 */
typedef	struct	{
    unsigned char	rtype;		/* Type of request being made.	*/
    unsigned char	nsize;		/* Length of the name to use.	*/
    unsigned char	ptype;		/* Type of port registered.	*/
    unsigned char	dummy;
    unsigned int	port;
    unsigned char	name[GDO_NAME_MAX_LEN+1];
} gdo_req;

#define	GDO_REQ_SIZE	sizeof(gdo_req)	/* Size of a request packet.	*/

/*
 *	If you have a fascist sysadmin who will not let you run gdomap
 *	as root and will not even let you modify /etc/services to point
 *	gdomap to another port, you can uncomment the next #define to
 *	run gdomap on port 6006 (or modify this to a port of your choice).
 *	Whatever port you choose, you should make sure that no other
 *	processes running on your network use that number!
 *
 *	When you have done this you must recompile gdomap.c and
 *	NSPortNameServer.m and re-install the base library with
 *	the new NSPortNameServer.o
 *
 *	NB. Doing this will render your system unable to communicate with
 * 	other systems which have not performed the same remapping.  You
 *	should not do it unless you have no choice.
 */
/* #define	GDOMAP_PORT_OVERRIDE	6006 */

