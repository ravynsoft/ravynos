# -*- tab-width: 4 -*-
#
# Copyright (c) 2002-2019 Apple Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions are met:
#
# 1.  Redistributions of source code must retain the above copyright notice, 
#     this list of conditions and the following disclaimer. 
# 2.  Redistributions in binary form must reproduce the above copyright notice, 
#     this list of conditions and the following disclaimer in the documentation 
#     and/or other materials provided with the distribution. 
# 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of its 
#     contributors may be used to endorse or promote products derived from this 
#     software without specific prior written permission. 
#
# THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY 
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
# DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY 
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# IMPORTANT NOTE: This is a Makefile for *GNU make*
# On some systems, a different program may be the default "make" command.
# If "make os=xxx" gives lots of errors like "Missing dependency operator",
# then try typing "gmake os=xxx" instead.
#
# This Makefile builds an mDNSResponder daemon and a libdns_sd.so shared library
# for Linux. It also builds several example programs for embedded systems.
#
# Make with no arguments to build all production targets.
# 'make DEBUG=1' to build debugging targets.
# 'make clean' or 'make clean DEBUG=1' to delete prod/debug objects & targets
# 'sudo make install [DEBUG=1]' to install mdnsd daemon and libdns_sd.
#
# Notes:
# $@ means "The file name of the target of the rule"
# $< means "The name of the first prerequisite"
# $* means "The stem with which an implicit rule matches"
# $+ means "The names of all the prerequisites, with spaces between them, exactly as given"
# For more magic automatic variables, see
# <http://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html>

#############################################################################

LIBVERS = 1

COREDIR = ../mDNSCore
SHAREDDIR ?= ../mDNSShared
DSODIR ?= ../DSO
JDK = /usr/jdk

SYSTEM := $(shell uname -s)
ifeq ($(SYSTEM), Darwin)
  os=x
else ifeq ($(SYSTEM), Linux)
  os=linux
endif

CC = cc
BISON = bison
FLEX = flex
ST = strip
LD = ld
SOOPTS = -shared
CP = cp
RM = rm
LN = ln -s -f
CFLAGS_COMMON = -I$(COREDIR) -I$(SHAREDDIR) -I$(DSODIR) -I$(PROXYDIR) -I$(OBJDIR) -fwrapv -W -Wall -DPID_FILE=\"/var/run/mdnsd.pid\" -DMDNS_UDS_SERVERPATH=\"/var/run/mdnsd\"
CFLAGS_PTHREAD =
LINKOPTS =
LINKOPTS_PTHREAD = -lpthread
LDSUFFIX = so
JAVACFLAGS_OS = -fPIC -shared -ldns_sd

ifeq "$(OPEN_SOURCE)" "1"
CFLAGS_OPEN_SOURCE=-D__OPEN_SOURCE__
else
CFLAGS_OPEN_SOURCE=
endif

# Set up diverging paths for debug vs. prod builds
ifeq "$(DEBUG)" "1"
CFLAGS_DEBUGGING = -g -DMDNS_DEBUGMSGS=2
OBJDIR = objects/debug
BUILDDIR = build/debug
STRIP = echo
else
ifeq "$(DEBUGSYMS)" "1"
CFLAGS_DEBUGGING = -g -DMDNS_DEBUGMSGS=0
OBJDIR = objects/prod
BUILDDIR = build/prod
STRIP = echo
else
# We use -Os for two reasons:
# 1. We want to make small binaries, suitable for putting into hardware devices
# 2. Some of the code analysis warnings only work when some form of optimization is enabled
CFLAGS_DEBUGGING = -g -DMDNS_DEBUGMSGS=0
OBJDIR ?= objects/prod
BUILDDIR ?= build/prod
STRIP = $(ST) -S
endif
endif

# Configure per-OS peculiarities
ifeq ($(os),solaris)
CFLAGS_DEBUGGING = -O0 -DMDNS_DEBUGMSGS=0
CFLAGS_OS = -DNOT_HAVE_DAEMON -DNOT_HAVE_SA_LEN -DNOT_HAVE_SOCKLEN_T -DNOT_HAVE_IF_NAMETOINDEX \
	 -DLOG_PERROR=0 -D_XPG4_2 -D__EXTENSIONS__ -DHAVE_BROKEN_RECVIF_NAME -DTARGET_OS_SOLARIS
CC = gcc
LD = gcc
SOOPTS = -shared
LINKOPTS = -lsocket -lnsl -lresolv
JAVACFLAGS_OS += -I$(JDK)/include/solaris
ifneq ($(DEBUG),1)
STRIP = $(ST)
endif
else

# any target that contains the string "linux"
ifeq ($(findstring linux,$(os)),linux)
CFLAGS_OS = -D_GNU_SOURCE -DHAVE_IPV6 -DNOT_HAVE_SA_LEN -DUSES_NETLINK -DHAVE_LINUX -DTARGET_OS_LINUX -ftabstop=4
LD = $(CC)
SOOPTS = -shared
FLEXFLAGS_OS = -l
JAVACFLAGS_OS += -I$(JDK)/include/linux

# uClibc does not support Name Service Switch
ifneq ($(os),linux-uclibc)
OPTIONALTARG = nss_mdns
OPTINSTALL   = InstalledNSS
endif
else

ifeq ($(os),netbsd)
CFLAGS_OS =
LDCONFIG = ldconfig
else

ifeq ($(os),freebsd)
# If not already defined, set LOCALBASE to /usr/local
LOCALBASE?=/usr/local
INSTBASE=$(LOCALBASE)
CFLAGS_OS = -DHAVE_IPV6
# FreeBSD 4 requires threaded code to be compiled and linked using the "-pthread" option,
# and requires that the "-lpthread" link option NOT be used
# This appies only to FreeBSD -- "man cc" on FreeBSD says:
#   FreeBSD SPECIFIC OPTIONS
#     -pthread
#       Link a user-threaded process against libc_r instead of libc.
CFLAGS_PTHREAD   = -pthread -D_THREAD_SAFE
LINKOPTS_PTHREAD = -pthread
JAVACFLAGS_OS += -I$(JDK)/include/freebsd
LDCONFIG = ldconfig
else

ifeq ($(os),openbsd)
CFLAGS_OS = -DHAVE_BROKEN_RECVDSTADDR
LDCONFIG = ldconfig
else

ifeq ($(os),x)
# We have to define __MAC_OS_X_VERSION_MIN_REQUIRED=__MAC_OS_X_VERSION_10_4 or on Leopard
# we get build failures: ‘daemon’ is deprecated (declared at /usr/include/stdlib.h:283)
CFLAGS_OS = -DHAVE_IPV6 -no-cpp-precomp -Werror -Wdeclaration-after-statement \
	-D__MAC_OS_X_VERSION_MIN_REQUIRED=__MAC_OS_X_VERSION_10_4 \
	-DHAVE_STRLCPY=1 -DTARGET_OS_MAC \
	-D__APPLE_USE_RFC_2292 #-Wunreachable-code
CC = gcc
LD = $(CC)
SOOPTS = -dynamiclib
LINKOPTS = -lSystem
LDSUFFIX = dylib
JDK = /System/Library/Frameworks/JavaVM.framework/Home
JAVACFLAGS_OS = -dynamiclib -I/System/Library/Frameworks/JavaVM.framework/Headers -framework JavaVM
OPTIONALTARG = dnsextd
else

$(error ERROR: Must specify target OS on command-line, e.g. "make os=x [target]".\
Supported operating systems include: x, linux, linux-uclibc, netbsd, freebsd, openbsd, solaris)
endif
endif
endif
endif
endif
endif

NSSLIBNAME  := libnss_mdns
NSSVERSION  := 0.2
NSSLIBFILE  := $(NSSLIBNAME)-$(NSSVERSION).so
NSSLINKNAME := $(NSSLIBNAME).so.2
NSSINSTPATH := /lib

# If not otherwise defined, we install into /usr/lib and /usr/include
# and our startup script is called mdns (e.g. /etc/init.d/mdns)
ETCBASE?=/etc
INSTBASE?=/usr
STARTUPSCRIPTNAME?=mdns

ifeq ($(HAVE_IPV6),1)
CFLAGS_OS += -DHAVE_IPV6=1
else
ifeq ($(HAVE_IPV6),0)
CFLAGS_OS += -DHAVE_IPV6=0
endif
endif

# If directory /usr/share/man exists, then we install man pages into that, else /usr/man
ifeq ($(wildcard /usr/share/man), /usr/share/man)
MANPATH := /usr/share/man
else
MANPATH := /usr/man
endif

# If directories /etc/init.d/rc*.d exist, then we install into that (Suse)
ifeq ($(wildcard /etc/init.d/rc2.d/), /etc/init.d/rc2.d/)
STARTUPSCRIPTDIR = /etc/init.d
RUNLEVELSCRIPTSDIR = /etc/init.d
else
