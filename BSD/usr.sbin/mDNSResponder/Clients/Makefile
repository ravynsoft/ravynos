# -*- tab-width: 4 -*-
#
# Copyright (c) 2002-2004, 2015 Apple Computer, Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Notes:
# $@ means "The file name of the target of the rule"
# $< means "The name of the first prerequisite"
# $+ means "The names of all the prerequisites, with spaces between them, exactly as given"
# For more magic automatic variables, see
# <http://www.gnu.org/software/make/manual/html_chapter/make_10.html#SEC111>
#
# This makefile uses $(CC) for compilation and linking, which is
# an automatic implicit gcc variable that defaults to "cc"
# <https://www.gnu.org/software/make/manual/html_node/Implicit-Variables.html>

#############################################################################

# On OS X the dns_sd library functions are included in libSystem, which is implicitly linked with every executable
# If /usr/lib/libSystem.dylib exists, then we're on OS X, so we don't need also to link the "dns_sd" shared library
ifeq "$(DEBUG)" "1"
DEBUGFLAGS = -g
BUILDDIR = build/debug
else
DEBUGFLAGS = -Os
BUILDDIR = build/prod
endif

ifneq "$(wildcard /usr/lib/libSystem.dylib)" ""
TARGETS = build/dns-sd build/dns-sd64
LIBS =
else
TARGETS = build/dns-sd
LIBS = -L../mDNSPosix/$(BUILDDIR)/ -ldns_sd
endif

all: $(TARGETS)

clean:
	rm -rf build

build:
	mkdir build

build/dns-sd: build dns-sd.c ClientCommon.c
	$(CC) $(SUPMAKE_CFLAGS) $(filter %.c %.o, $+) $(LIBS) -I../mDNSShared -Wall -o $@

build/dns-sd64: build dns-sd.c ClientCommon.c
	$(CC) $(SUPMAKE_CFLAGS) $(filter %.c %.o, $+) $(LIBS) -I../mDNSShared -Wall -o $@ -m64

# Note, we can make a 'fat' version of dns-sd using 'lipo', as shown below, but we
# don't, because we don't want or need a 'fat' version of dns-sd, because it will
# never need to access more than 4GB of data. We build the 64-bit version purely so
# we have a test tool for making sure that the APIs work properly from 64-bit clients.
# lipo -create dns-sd dns-sd64 -output dns-sd-fat
