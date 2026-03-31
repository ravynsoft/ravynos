# Readme for libnss_mdns

Andrew White <Andrew.White@nicta.com.au>
June 2004

Before using this software, see "Licensing" at bottom of this file.


# Introduction

This code implements a module for the Name Service Switch to perform
hostname lookups using the Darwin mDNSResponder / mdnsd.  This code has
been tested on Debian and Redhat Linux.  It may work on other platforms. 
It *will not* work on Darwin or Mac OS X - the necessary functionality is
already built into the operation system.


# Building and Installing:

See "ReadMe.txt" for instructions on building and installing.

When you run "make install" as described in that file:
o libnss_mdns-0.2.so and libnss_mdns.so.2 are installed in /lib,
o manual pages libnss_mdns(8) and nss_mdns.conf(5) are installed,
o nss_mdns.conf is installed in /etc, and
o /etc/nsswitch.conf is modified to add "mdns" on the "hosts:" line

This will cause dns lookups to be passed via mdnsd before trying the dns.


# Testing

For most purposes, 'ping myhostname.local' will tell you if mdns is
working.  If MDNS_VERBOSE was set in nss_mdns.c during compilation then
lots of chatty debug messages will be dumped to LOG_DEBUG in syslog. 
Otherwise, nss_mdns will only log if something isn't behaving quite right.


# Implementation details

libnss_mdns provides alternative back-end implementations of the libc
functions gethostbyname, gethostbyname2 and gethostbyaddr, using the Name
Service Switch mechanism.  More information on writing nsswitch modules
can be found via 'info libc "Name Service Switch"', if installed.


# Licensing

This software is licensed under the NICTA Public Software License, version
1.0, printed below:

NICTA Public Software Licence
Version 1.0

Copyright © 2004 National ICT Australia Ltd

All rights reserved.

By this licence, National ICT Australia Ltd (NICTA) grants permission,
free of charge, to any person who obtains a copy of this software
and any associated documentation files ("the Software") to use and
deal with the Software in source code and binary forms without
restriction, with or without modification, and to permit persons
to whom the Software is furnished to do so, provided that the
following conditions are met:

- Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimers.
- Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimers in
  the documentation and/or other materials provided with the
  distribution.
- The name of NICTA may not be used to endorse or promote products
  derived from this Software without specific prior written permission.

EXCEPT AS EXPRESSLY STATED IN THIS LICENCE AND TO THE FULL EXTENT
PERMITTED BY APPLICABLE LAW, THE SOFTWARE IS PROVIDED "AS-IS" AND
NICTA MAKES NO REPRESENTATIONS, WARRANTIES OR CONDITIONS OF ANY
KIND, EXPRESS OR IMPLIED, INCLUDING, WITHOUT LIMITATION, ANY
REPRESENTATIONS, WARRANTIES OR CONDITIONS REGARDING THE CONTENTS
OR ACCURACY OF THE SOFTWARE, OR OF TITLE, MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE, NONINFRINGEMENT, THE ABSENCE OF LATENT
OR OTHER DEFECTS, OR THE PRESENCE OR ABSENCE OF ERRORS, WHETHER OR
NOT DISCOVERABLE.

TO THE FULL EXTENT PERMITTED BY APPLICABLE LAW, IN NO EVENT WILL
NICTA BE LIABLE ON ANY LEGAL THEORY (INCLUDING, WITHOUT LIMITATION,
NEGLIGENCE) FOR ANY LOSS OR DAMAGE WHATSOEVER, INCLUDING (WITHOUT
LIMITATION) LOSS OF PRODUCTION OR OPERATION TIME, LOSS, DAMAGE OR
CORRUPTION OF DATA OR RECORDS; OR LOSS OF ANTICIPATED SAVINGS,
OPPORTUNITY, REVENUE, PROFIT OR GOODWILL, OR OTHER ECONOMIC LOSS;
OR ANY SPECIAL, INCIDENTAL, INDIRECT, CONSEQUENTIAL, PUNITIVE OR
EXEMPLARY DAMAGES ARISING OUT OF OR IN CONNECTION WITH THIS LICENCE,
THE SOFTWARE OR THE USE OF THE SOFTWARE, EVEN IF NICTA HAS BEEN
ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

If applicable legislation implies warranties or conditions, or
imposes obligations or liability on NICTA in respect of the Software
that cannot be wholly or partly excluded, restricted or modified,
NICTA's liability is limited, to the full extent permitted by the
applicable legislation, at its option, to:

a. in the case of goods, any one or more of the following:
  i.   the replacement of the goods or the supply of equivalent goods;
  ii.  the repair of the goods;
  iii. the payment of the cost of replacing the goods or of acquiring
       equivalent goods;
  iv.  the payment of the cost of having the goods repaired; or
b. in the case of services:
  i.   the supplying of the services again; or 
  ii.  the payment of the cost of having the services supplied
       again.


# Links:

NICTA
	http://www.nicta.com.au/
Darwin
	http://developer.apple.com/darwin/
DNS service discovery and link-local
	http://http://zeroconf.org/
	http://http://multicastdns.org/
	http://http://dns-sd.org/
	http://http://dotlocal.org/
