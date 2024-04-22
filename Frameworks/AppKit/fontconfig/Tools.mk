# -*- encoding: utf-8 -*-
#
# Copyright © 2003  Keith Packard
# Copyright © 2013  Google, Inc.
#
# Permission to use, copy, modify, distribute, and sell this software and its
# documentation for any purpose is hereby granted without fee, provided that
# the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation, and that the name of the author(s) not be used in
# advertising or publicity pertaining to distribution of the software without
# specific, written prior permission.  The authors make no
# representations about the suitability of this software for any purpose.  It
# is provided "as is" without express or implied warranty.
#
# THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
# EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
# DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
# TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
#
# Google Author(s): Behdad Esfahbod

DIR=fc-$(TAG)
OUT=fc$(TAG)
TMPL=$(OUT).tmpl.h
TARG=$(OUT).h
TOOL=$(srcdir)/$(DIR).py

noinst_SCRIPTS = $(TOOL)
EXTRA_DIST = $(TARG) $(TMPL) $(DIST)

$(TARG): $(TMPL) $(TOOL) $(DEPS)
	$(AM_V_GEN) \
	$(RM) $(TARG) && \
	$(PYTHON) $(TOOL) $(ARGS) --template $< --output $(TARG).tmp && \
	mv $(TARG).tmp $(TARG) || ( $(RM) $(TARG).tmp && false )
noinst_HEADERS=$(TARG)

ALIAS_FILES = fcalias.h fcaliastail.h

BUILT_SOURCES = $(ALIAS_FILES)

$(ALIAS_FILES):
	$(AM_V_GEN) touch $@

CLEANFILES = $(ALIAS_FILES)

MAINTAINERCLEANFILES = $(TARG)
