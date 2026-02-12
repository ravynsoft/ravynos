This directory contains a variety of clients that use the
"/usr/include/dns_sd.h" APIs.

Some of the clients are command-line oriented; some are graphical.

Some of the clients, like the "dns-sd" command-line tool, can be built
for a variety of platforms. Some of the clients only build for one
platform, like "DNS Service Browser" and "DNS Service Registration",
which use Objective C and Cocoa and only run on OS X 10.3 or later.

Some of the clients can be built more than one way. For example, the
"dns-sd" command-line tool can be built for OS X using both the XCode
IDE, or using a simple command-line "make" command.

What all clients have in common is that they all demonstrate how you
can use the "/usr/include/dns_sd.h" APIs.

The table below gives a summary of which clients build for which platforms.

Client                    Type          OS X   OS X   Posix
                                        XCode  Make   Make

dns-sd                    Command-line   X      X      X
DNS Service Browser       Graphical      X
DNS Service Registration  Graphical      X
