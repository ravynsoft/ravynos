A Brief History of Sudo
=======================

## The Early Years

Sudo was first conceived and implemented by Bob Coggeshall and Cliff Spencer
around 1980 at the Department of Computer Science at SUNY/Buffalo. It ran on
a VAX-11/750 running 4.1BSD. An updated version, credited to Phil Betchel,
Cliff Spencer, Gretchen Phillips, John LoVerso, and Don Gworek, was posted to
the net.sources Usenet newsgroup in December of 1985.

## Sudo at CU-Boulder

In the Summer of 1986, Garth Snyder released an enhanced version of sudo.
For the next 5 years, sudo was fed and watered by a handful of folks at
CU-Boulder, including Bob Coggeshall, Bob Manchek, and Trent Hein.

## Root Group Sudo

In 1991, Dave Hieb and Jeff Nieusma wrote a new version of sudo with an
enhanced sudoers format under contract to a consulting firm called "The Root
Group". This version was later released under the GNU public license.

## CU Sudo

In 1994, after maintaining sudo informally within CU-Boulder for some time,
Todd C. Miller made a public release of "CU sudo" (version 1.3) with bug
fixes and support for more operating systems. The "CU" was added to
differentiate it from the "official" version from "The Root Group".

In 1995, a new parser for the sudoers file was contributed by Chris Jepeway.
The new parser was a proper grammar (unlike the old one) and could work with
both sudo and visudo (previously they had slightly different parsers).

In 1996, Todd, who had been maintaining sudo for several years in his spare
time, moved distribution of sudo from a CU-Boulder ftp site to his domain,
courtesan.com.

## Just Plain Sudo

In 1999, the "CU" prefix was dropped from the name since there had been no
formal release of sudo from "The Root Group" since 1991 (the original
authors now work elsewhere). As of version 1.6, Sudo no longer contains any
of the original "Root Group" code and is available under an ISC-style
license.

In 2001, the sudo web site, ftp site, and mailing lists were moved from
courtesan.com to the sudo.ws domain (sudo.org was already taken).

## LDAP Integration

In 2003, Nationwide Mutual Insurance Company contributed code written by
Aaron Spangler to store the sudoers data in LDAP. These changes were
incorporated into Sudo 1.6.8.

## New Parser

In 2005, Todd rewrote the sudoers parser to better support the features that
had been added in the past ten years. This new parser removes some
limitations of the previous one, removes ordering constraints and adds
support for including multiple sudoers files.

## Quest Sponsorship

In 2010, Quest Software began sponsoring Sudo development by hiring
Todd to work on Sudo as part of his full-time job.  This enabled
the addition of I/O logging, the plugin API, the log server,
additional regression and fuzz tests, support for binary packages
and more regular releases.

## Present Day

Sudo, in its current form, is maintained by:

    Todd C. Miller <Todd.Miller@sudo.ws>

Todd continues to enhance sudo and fix bugs.
