#
# OS/2 GNU Makefile for building gettext with GNU Make and GNU C compiler
#
# OS/2 still supports the regular configure/make building mechanism, but its
# way more clumsy, complicated and error prone. It is highly recommended to
# use this makefile instead, because :
# - this makefile builds an optimized static and dynamic version of the
#   library
# - it is able to build both optimized and debug versions of the library
#   without any reconfiguring
# - this makefile builds an backward compatible DLL. When building with
#   configure you will get a DLL which is compatible only with the 0.10.40
#   and later DLLs of gettext; this makefile builds a DLL which is binary
#   compatible with gettext 0.10.35 (exports by ordinal matter).
# - it is able to generate a complete OS/2 binary distribution (make distr)
# - besides its simply alot faster than configure generated makefiles
#
# The makefile is designed to be more or less gettext version independent,
# so it is likely to work with future versions of gettext as well.
#
# If you get unresolved dependencies (e.g. "don't know how to make somefile.h
# which is required for zzz.o) remove (or rebuild, if you have the makedep tool)
# the depend.mak file which contains all the dependencies.
#

# Use CMD.EXE as shell since its way faster
SHELL = $(COMSPEC)
# An Unix-like shell (needed for running config.charset)
UNIXSHELL = sh.exe

# Debug mode (1) or optimize mode (0)
DEBUG = 0

# The version of INTL.DLL (the name suffix)
INTLDLLVER =

# Pack the DLL and executables with lxlite
LXLITE = 1

# Output directory
OUT = out/$(OUT.SUFFIX)/
# Root package directory
ROOT = ../
# The base directory for distribution archive (emx/ or usr/)
INST = emx/

# Tools
CC = gcc -c
CFLAGS = -Wall -Zmt $(INCLUDE) $(DEFS)
INCLUDE = -I. -I$(ROOT) -I$(ROOT)intl -I$(ROOT)src -I$(ROOT)lib
DEFS = -DHAVE_CONFIG_H -DLIBDIR=\"/usr/lib\" \
  -DLOCALEDIR=\"/usr/share/locale\" -DLOCALE_ALIAS_PATH=\"/usr/share/locale\" \
  -DGETTEXTDATADIR=\"/usr/share/gettext\" \
  -DPROJECTSDIR=\"/usr/share/gettext/projects\" \
  -DGETTEXTJAR=\"/usr/share/gettext/gettext.jar\"


LD = gcc
LDFLAGS = -Zmt -Zcrtdll
LDFLAGS.SHARED = -Zmt -Zcrtdll -Zdll
LIBS = -liconv -liberty -lgcc

AR = ar
ARFLAGS = crs

MKDIR = mkdir.exe -p
COPY = cp -p

LINKINTL = $(OUT)intl.a

ifeq ($(DEBUG),0)
  CFLAGS += -s -O2
  LDFLAGS += -s -Zexe
  LDFLAGS.SHARED += -s
  OUT.SUFFIX = release
else
  CFLAGS += -g
  LDFLAGS += -g -Zexe
  LDFLAGS.SHARED += -g
  OUT.SUFFIX = debug
  LXLITE := 0
  LINKINTL = $(OUT)intl_s.a
endif

# Languages with encodings unsupported by OS/2 API (BIG5)
BAD.LINGUAS=zh

# The list of languages to be included in binary distribution
LINGUAS = $(filter-out $(BAD.LINGUAS),$(shell sed -e "/^\#/d" $(ROOT)po/LINGUAS))

# Fetch version number from configure.in
VERSION = $(shell sed ../configure.in -ne "/AM_INIT_AUTOMAKE/{" -e "s/.*(gettext, *\\(.*\\))/\\1/" -e "p" -e "}")

# Fetch the list of source files for libintl from intl/Makefile.in
INTL.SOURCES = $(addprefix $(ROOT)intl/,\
  $(subst $$lo,c,\
  $(subst @INTLOBJS@,intl-compat.c,\
  $(subst OBJECTS = ,,\
  $(subst \,,\
  $(shell sed $(ROOT)intl/Makefile.in -ne "/^OBJECTS =/,/[^\]$$/p"))))))
INTL.OBJECTS = $(addprefix $(OUT),$(subst $(ROOT),,$(INTL.SOURCES:.c=.o)))

PROGRAMS = $(addsuffix .exe,$(subst bin_PROGRAMS = ,,\
  $(subst \,,\
  $(shell sed $(ROOT)src/Makefile.am -ne "/^bin_PROGRAMS =/,/[^\]$$/p"))))
PROGRAMS.EXE = $(addprefix $(OUT),$(PROGRAMS))
SRC.SOURCES = $(filter-out $(addprefix $(ROOT)src/,$(PROGRAMS:.exe=.c)),\
  $(wildcard $(ROOT)src/*.c))
SRC.OBJECTS = $(addprefix $(OUT),$(subst $(ROOT),,$(SRC.SOURCES:.c=.o)))
LIB.SOURCES = $(addprefix $(ROOT)lib/, error.c stpcpy.c stpncpy.c mkdtemp.c getline.c \
  $(subst libgettextlib_la_SOURCES = ,,\
  $(subst \,,\
  $(patsubst %.h,,\
  $(patsubst getopt%.c,,\
  $(shell sed $(ROOT)lib/Makefile.am -ne "/^libgettextlib_la_SOURCES =/,/[^\]$$/p"))))))
LIB.OBJECTS = $(addprefix $(OUT),$(subst $(ROOT),,$(LIB.SOURCES:.c=.o)))

OUTDIRS = $(OUT) $(sort $(dir $(INTL.OBJECTS) $(SRC.OBJECTS) $(LIB.OBJECTS) \
  $(LIBUNINAME.OBJECTS) $(INSTALL.FILES)))

INSTALL.FILES = $(addprefix $(INST)bin/,$(PROGRAMS)) \
  $(INST)lib/intl.a $(INST)lib/intl_s.a $(INST)include/libintl.h \
  $(INST)dll/intl.dll $(INST)share/locale/charset.alias \
  $(INST)share/locale/locale.alias $(INST)doc/gettext-$(VERSION)/README.OS2 \
  $(INST)doc/gettext-$(VERSION)/COPYING $(INST)doc/gettext-$(VERSION)/README \
  $(addsuffix /LC_MESSAGES/gettext.mo,$(addprefix $(INST)share/locale/,$(LINGUAS))) \
  $(INSTALL.DIFF)

.SUFFIXES:
.SUFFIXES: .o .a .def .exe .dll .po .mo
.PRECIOUS: $(OUT)%.o $(OUT)%.a

.PHONY: all depend clean distr rmzip

$(OUT)%.o: $(ROOT)%.c
	$(CC) $(CFLAGS) -o $@ $<

# To avoid playing with object file lists for every program we will build
# instead a library containing all the object files from src directory, and
# then link the library against the main program module, so that linker can
# pull all the required functions from there
$(OUT)%.exe: $(OUT)src/%.o $(OUT)util.a $(OUT)uniname.a $(LINKINTL)
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
ifeq ($(LXLITE),1)
	lxlite /b- $@
endif

all: config.h $(OUTDIRS) $(ROOT)src/po-gram-gen2.h $(ROOT)lib/stdbool.h \
  $(ROOT)intl/libintl.h $(OUT)intl.a $(OUT)intl_s.a $(OUT)intl$(INTLDLLVER).dll \
  $(PROGRAMS.EXE)

define MAKEDEP
	echo $(OUT)PATH = FILENAME
	makedep $(INCLUDE) $(DEFS) -p $$(OUT)PATH -r -a FILENAME -c -f depend.mak

endef

depend: $(INTL.SOURCES) $(SRC.SOURCES)
# Remove all dependencies
	rm -f depend.mak
# Build dependencies, by one file (to avoid 1024 chars command line length limit)
	@$(foreach fn,$^,$(subst PATH,$(subst ../,,$(dir $(fn))),$(subst FILENAME,$(fn),$(MAKEDEP))))

clean:
	rm -rf out emx config.h

distr: all rmzip gettext-os2-$(VERSION)-bin.zip

rmzip:
	rm -f gettext-os2-$(VERSION)-bin.zip

# How to configure without configure...
config.h: ../config.h.in ./configure.awk
	gawk -f configure.awk -v PACKAGE=gettext -v VERSION=$(VERSION) $< >config.h

$(OUTDIRS):
	$(MKDIR) $(@:/=)

$(OUT)intl_s.a: $(INTL.OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(OUT)intl.def: $(INTL.OBJECTS)
	@echo LIBRARY INTL$(INTLDLLVER) INITINSTANCE TERMINSTANCE>$@
	@echo DESCRIPTION "GNU gettext internationalization library version $(VERSION)">>$@
	@echo DATA MULTIPLE NONSHARED>>$@
	@echo EXPORTS>>$@
	type backward.def>>$@
	emxexp -u $^ >>$@

$(OUT)intl$(INTLDLLVER).dll: $(INTL.OBJECTS) $(OUT)intl.def
	@echo *********************************************************
	@echo *** YOU CAN SAFELY IGNORE WARNINGS FROM EMXBIND BELOW ***
	@echo *********************************************************
	$(LD) $(LDFLAGS.SHARED) -o $@ $^ $(LIBS)
ifeq ($(LXLITE),1)
	lxlite $@
endif

# How to build an import library from a .DEF file
$(OUT)%.a: $(OUT)%.def
	emximp -o $@ $<

$(OUT)util.a: $(SRC.OBJECTS) $(LIB.OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(OUT)uniname.a: $(LIBUNINAME.OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

$(ROOT)src/po-gram-gen2.h: $(ROOT)src/po-gram-gen.h
	sed -e "s/[yY][yY]/po_gram_/g" $< > $@

$(ROOT)intl/libintl.h: $(ROOT)intl/libgnuintl.h 
	$(COPY) $< $@

$(ROOT)lib/stdbool.h: $(ROOT)lib/stdbool.h.in
	$(COPY) $< $@

gettext-os2-$(VERSION)-bin.zip: $(INSTALL.FILES)
	@rm -f $@
	zip -9XD $@ $^

# The following rules are for `make distr' target only

$(INST)share/locale/charset.alias: $(ROOT)lib/config.charset
	$(UNIXSHELL) $< i386-pc-os2-emx >$@
$(INST)share/locale/locale.alias: $(ROOT)intl/locale.alias
	$(COPY) $< $@
$(INST)bin/% $(INST)lib/% $(INST)dll/%: $(OUT)%
	$(COPY) $< $@
$(INST)include/%: $(ROOT)intl/%
	$(COPY) $< $@
$(INST)doc/gettext-$(VERSION)/%: $(ROOT)%
	$(COPY) $< $@
$(INST)doc/gettext-$(VERSION)/% $(INST)include/%: %
	$(COPY) $< $@
$(INST)share/locale/%/LC_MESSAGES/gettext.mo: $(ROOT)po/%.po
	$(MKDIR) $(dir $@)
	$(COMSPEC) /c "$(subst /,\\,set BEGINLIBPATH=$(OUT:/=) && \
	  $(OUT)msgfmt.exe) --statistics --verbose -o $@ $<"

-include depend.mak
