# Note that the macros are not used for the install target any more with the
# change to the install(1) program.  Edit the install commands.

#
# TAPI supports installapi per default. Allow B&I to change the default and
# print a status message to the log file.
#
SUPPORTS_TEXT_BASED_API ?= YES
$(info SUPPORTS_TEXT_BASED_API=$(SUPPORTS_TEXT_BASED_API))

ifneq "" "$(wildcard /bin/mkdirs)"
  MKDIRS = /bin/mkdirs
else
  MKDIRS = /bin/mkdir -p
endif

FILES = mach-o/ldsyms.h mach-o/reloc.h \
	mach-o/nlist.h mach-o/stab.h mach-o/loader.h mach-o/fat.h \
	mach-o/dyld_debug.h mach-o/arch.h mach-o/gmon.h mach-o/ranlib.h \
	mach-o/swap.h mach-o/getsect.h mach-o/i386/swap.h \
	mach-o/ppc/swap.h mach-o/ppc/reloc.h mach-o/x86_64/reloc.h \
	mach-o/dyld.h mach-o/dyld_gdb.h mach-o/arm/reloc.h mach-o/arm64/reloc.h

LOCFILES = mach-o/rld_state.h mach-o/rld.h mach-o/sarld.h mach-o/kld.h \
	   mach-o/redo_prebinding.h \
	   mach-o/i860/swap.h mach-o/i860/reloc.h \
	   mach-o/hppa/swap.h mach-o/hppa/reloc.h \
	   mach-o/m88k/swap.h mach-o/m88k/reloc.h \
	   mach-o/m68k/swap.h \
	   mach-o/sparc/swap.h mach-o/sparc/reloc.h \
	   cbt/libsyminfo.h mach-o/dyld_priv.h

# Note that OTHER_SRCS do NOT get installed
GAS_OTHER_SRCS = stuff/bytesex.h stuff/bool.h stuff/rnd.h stuff/errors.h \
		 stuff/openstep_mach.h opcode/arm.h
OTHER_SRCS = notes gnu/symseg.h \
	     stuff/allocate.h stuff/arch.h stuff/execute.h \
	     stuff/ofile.h stuff/lto.h stuff/llvm.h \
	     stuff/hash_string.h stuff/breakout.h stuff/best_arch.h \
	     stuff/hppa.h stuff/reloc.h stuff/vm_flush_cache.h \
	     stuff/print.h stuff/version_number.h stuff/crc32.h sys/gmon.h \
	     stuff/seg_addr_table.h stuff/dylib_table.h \
	     stuff/SymLoc.h stuff/dylib_roots.h stuff/guess_short_name.h \
	     stuff/macosx_deployment_target.h stuff/symbol_list.h \
	     stuff/symbol.h stuff/unix_standard_mode.h \
	     stuff/write64.h \
	     coff/base_relocs.h coff/bytesex.h coff/ms_dos_stub.h \
	     coff/filehdr.h coff/aouthdr.h coff/scnhdr.h coff/syment.h \
	     coff/debug_directory.h elf/dwarf2.h llvm-c/Disassembler.h \
	     xar/xar.h $(GAS_OTHER_SRCS)

ENCUMBERED_SRCS = gnu/a.out.h gnu/exec.h

# Note that MISSING_SRCS are those may not be on all build machines
GAS_MISSING_SRCS = mach/m68k/thread_status.h \
		   mach/i860/thread_status.h \
		   mach/m88k/thread_status.h \
		   architecture/m88k/fp_regs.h \
		   architecture/m88k/reg_help.h \
		   architecture/nrw/reg_help.h \
		   architecture/nrw/macro_help.h \
		   mach/hppa/thread_status.h \
		   mach/sparc/thread_status.h \
		   architecture/sparc/reg.h \
		   mach/arm/thread_status.h \
		   mach/arm/thread_state.h \
		   mach/arm/_structs.h \
		   mach/ppc/thread_status.h \
		   mach/ppc/_structs.h

MISSING_SRCS = mach/machine.h mach/slot_name.h \
	       architecture/i386/fpu.h architecture/i386/frame.h \
	       i386/eflags.h mach/i386/_structs.h mach/i386/fp_reg.h \
	       mach/i386/thread_state.h mach/i386/thread_status.h \
	       standalone/libsa.h \
	       $(GAS_MISSING_SRCS)

nextstep_INCDIR = /NextDeveloper/Headers
nextstep_LOCINCDIR = /LocalDeveloper/Headers

teflon_INCDIR = /System/Library/Frameworks/System.framework/Versions/B/Headers
teflon_LOCINCDIR = /System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders

ifeq "macos" "$(RC_OS)"
 macos_INCDIR := $(shell if [ "$(RC_RELEASE)" = "Beaker" ] ||    \
			    [ "$(RC_RELEASE)" = "Bunsen" ] ||    \
			    [ "$(RC_RELEASE)" = "Gonzo"  ] ||    \
			    [ "$(RC_RELEASE)" = "Kodiak" ]; then \
	 echo /System/Library/Frameworks/System.framework/Versions/B/Headers; \
	 else echo /usr/include; \
	 fi; )
 macos_LOCINCDIR := $(shell if [ "$(RC_RELEASE)" = "Beaker" ] ||    \
			       [ "$(RC_RELEASE)" = "Bunsen" ] ||    \
			       [ "$(RC_RELEASE)" = "Gonzo"  ] ||    \
			       [ "$(RC_RELEASE)" = "Kodiak" ]; then \
 echo /System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders; \
 else echo /usr/local/include; \
 fi; )
else
  macos_INCDIR = /System/Library/Frameworks/System.framework/Versions/B/Headers
  macos_LOCINCDIR = /System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders
endif

SRC_FILES = Makefile $(FILES) $(LOCFILES) $(OTHER_SRCS) $(MISSING_SRCS)
GAS_SRC_FILES = Makefile $(GAS_OTHER_SRCS) $(GAS_MISSING_SRCS)
ENCUMBERED_SRC_FILES = $(SRC_FILES) $(ENCUMBERED_SRCS)

installsrc:
	$(MKDIRS) $(SRCROOT)
	chmod 755 $(SRCROOT)
	gnutar cf - $(ENCUMBERED_SRC_FILES) | (cd $(SRCROOT); gnutar xf -)

installGASsrc:
	$(MKDIRS) $(SRCROOT)
	chmod 755 $(SRCROOT)
	gnutar cf - $(GAS_SRC_FILES) | (cd $(SRCROOT); gnutar xf -)

# This is a bit of a hack as 'tapi installapi' should take the RC_CFLAGS or
# RC_ARCHS as there is no way to make up the -target options from the
# RC_* buildit options.
ifeq "$(RC_MACOS)" "YES"
  TAPI_TARGET_ARCHS := -target x86_64-apple-macos10.14 \
    -target i386-apple-macos10.14 \
    -target-variant x86_64-apple-ios12.0-macabi
else
  # keep -arch and add version-min for non-OSX toolchains
  TAPI_TARGET_ARCHS := $(patsubst %,-arch %,$(RC_ARCHS))
  TAPI_TARGET_ARCHS += $(RC_MIN_VERSION_OPTION)
endif

TAPI_COMMON_OPTS = $(TAPI_TARGET_ARCHS) \
  -isysroot $(SDKROOT) -I$(SRCROOT)/include \
  -extra-public-header $(SRCROOT)/include/mach/slot_name.h \
  -exclude-private-header $(DSTROOT)/usr/local/include/cbt/libsyminfo.h \
  -exclude-public-header $(DSTROOT)/usr/include/mach-o/ldsyms.h \
  -exclude-public-header $(DSTROOT)/usr/include/mach-o/nlist.h \
  -dynamiclib -install_name /usr/lib/system/libmacho.dylib \
  -compatibility_version 1 \
  -current_version $(RC_ProjectSourceVersion) \
  -o $(OBJROOT)/libmacho.tbd

DYLIBDIR = /usr/lib/system

TAPI_VERIFY_OPTS := $(TAPI_COMMON_OPTS) \
  --verify-mode=Pedantic \
  --verify-against=$(DSTROOT)$(DYLIBDIR)/libmacho.dylib \
  -umbrella System
# -fapplication-extension

installapi: install
	$(MKDIRS) $(DSTROOT)$(DYLIBDIR)
	$(MKDIRS) $(OBJROOT)
	@echo
	@echo ++++++++++++++++++++++
	@echo + Running InstallAPI +
	@echo ++++++++++++++++++++++
	@echo

	@if [ "$(SUPPORTS_TEXT_BASED_API)" != "YES" ]; then \
	    echo -n "installapi for target 'tapi' was requested, " \
	    echo "but SUPPORTS_TEXT_BASED_API has been disabled."; \
	exit 1; \
	fi

	xcrun --sdk $(SDKROOT) tapi installapi $(TAPI_COMMON_OPTS) $(DSTROOT)
     
	install -d -m 0755 $(DSTROOT)$(DYLIBDIR)
	install -c -m 0444 $(OBJROOT)/libmacho.tbd \
		$(DSTROOT)$(DYLIBDIR)/libmacho.tbd

# This is again a bit of a hack, as libmacho.dylib would need be built first
# for this to work which is not even done for installhdrs.  But it shows the
# command with the needed $(TAPI_VERIFY_OPTS) that could work.
installapi-verify:
	$(MKDIRS) $(DSTROOT)$(DYLIBDIR)
	$(MKDIRS) $(OBJROOT)
	@echo
	@echo +++++++++++++++++++++++++++++++++
	@echo + Running InstallAPI and Verify +
	@echo +++++++++++++++++++++++++++++++++
	@echo

	@if [ "$(SUPPORTS_TEXT_BASED_API)" != "YES" ]; then \
	    echo -n "installapi for target 'tapi' was requested, " \
	    echo "but SUPPORTS_TEXT_BASED_API has been disabled."; \
	exit 1; \
	fi

	xcrun --sdk $(SDKROOT) tapi installapi $(TAPI_VERIFY_OPTS) $(DSTROOT)

install: dirs $(RC_OS)_install

teflon_install macos_install: common_install

nextstep_install: common_install
	cd mach-o; \
	    install -c -m 444 ${IFLAGS} gmon.h \
	    ${DSTROOT}${$(RC_OS)_INCDIR}/mach-o

common_install:
	cd mach-o; \
	    install -c -m 444 ${IFLAGS} arch.h ldsyms.h reloc.h \
	    stab.h loader.h fat.h swap.h getsect.h nlist.h \
	    ranlib.h ${DSTROOT}${$(RC_OS)_INCDIR}/mach-o
	if [ $(OLD_DYLD_STUFF) ];				\
	then							\
	    cd mach-o; install -c -m 444 ${IFLAGS} dyld.h dyld_debug.h	\
		${DSTROOT}${$(RC_OS)_INCDIR}/mach-o ;		\
	fi
	cd mach-o/i386; \
	    install -c -m 444 ${IFLAGS} swap.h \
	    ${DSTROOT}${$(RC_OS)_INCDIR}/mach-o/i386
	cd mach-o/ppc; \
	    install -c -m 444 ${IFLAGS} reloc.h swap.h \
	    ${DSTROOT}${$(RC_OS)_INCDIR}/mach-o/ppc
	cd mach-o/x86_64; \
	    install -c -m 444 ${IFLAGS} reloc.h \
	    ${DSTROOT}${$(RC_OS)_INCDIR}/mach-o/x86_64
	cd mach-o/arm; \
	    install -c -m 444 ${IFLAGS} reloc.h \
	    ${DSTROOT}${$(RC_OS)_INCDIR}/mach-o/arm
	cd mach-o/arm64; \
	    install -c -m 444 ${IFLAGS} reloc.h \
	    ${DSTROOT}${$(RC_OS)_INCDIR}/mach-o/arm64
	cd mach-o/m68k; \
	    install -c -m 444 ${IFLAGS} swap.h \
	    ${DSTROOT}${$(RC_OS)_LOCINCDIR}/mach-o/m68k
	cd mach-o/sparc; \
	    install -c -m 444 ${IFLAGS} reloc.h swap.h \
	    ${DSTROOT}${$(RC_OS)_LOCINCDIR}/mach-o/sparc
	cd mach-o/hppa; \
	    install -c -m 444 ${IFLAGS} reloc.h swap.h \
	    ${DSTROOT}${$(RC_OS)_LOCINCDIR}/mach-o/hppa
	cd mach-o/i860; \
	    install -c -m 444 ${IFLAGS} reloc.h swap.h \
	    ${DSTROOT}${$(RC_OS)_LOCINCDIR}/mach-o/i860
	cd mach-o/m88k; \
	    install -c -m 444 ${IFLAGS} reloc.h swap.h \
	    ${DSTROOT}${$(RC_OS)_LOCINCDIR}/mach-o/m88k
	if [ $(OLD_RLD_STUFF) ];					  \
	then								  \
	    cd mach-o; install -c -m 444 ${IFLAGS} rld.h rld_state.h 	  \
	        ${DSTROOT}${$(RC_OS)_LOCINCDIR}/mach-o ;		  \
	fi
	if [ $(OLD_DYLD_STUFF) ];					  \
	then								  \
	    cd mach-o; install -c -m 444 ${IFLAGS} dyld_gdb.h dyld_priv.h \
		${DSTROOT}${$(RC_OS)_LOCINCDIR}/mach-o ;		  \
	fi
	if [ $(OLD_RLD_STUFF) ];					  \
	then								  \
	    cd mach-o;							  \
	      install -c -m 444 ${IFLAGS} sarld.h kld.h redo_prebinding.h \
	      ${DSTROOT}${$(RC_OS)_LOCINCDIR}/mach-o ;			  \
	fi
	cd stuff; \
	    install -c -m 444 ${IFLAGS} bool.h \
	    ${DSTROOT}${$(RC_OS)_LOCINCDIR}/dyld
	cd cbt; \
	    install -c -m 444 ${IFLAGS} libsyminfo.h \
	    ${DSTROOT}${$(RC_OS)_LOCINCDIR}/cbt

dirs:
	$(MKDIRS) ${DSTROOT}${$(RC_OS)_INCDIR}/mach-o/i386
	$(MKDIRS) ${DSTROOT}${$(RC_OS)_INCDIR}/mach-o/ppc
	$(MKDIRS) ${DSTROOT}${$(RC_OS)_INCDIR}/mach-o/x86_64
	$(MKDIRS) ${DSTROOT}${$(RC_OS)_INCDIR}/mach-o/arm
	$(MKDIRS) ${DSTROOT}${$(RC_OS)_INCDIR}/mach-o/arm64
	$(MKDIRS) ${DSTROOT}${$(RC_OS)_LOCINCDIR}/mach-o/m68k
	$(MKDIRS) ${DSTROOT}${$(RC_OS)_LOCINCDIR}/mach-o/sparc
	$(MKDIRS) ${DSTROOT}${$(RC_OS)_LOCINCDIR}/mach-o/hppa
	$(MKDIRS) ${DSTROOT}${$(RC_OS)_LOCINCDIR}/mach-o/i860
	$(MKDIRS) ${DSTROOT}${$(RC_OS)_LOCINCDIR}/mach-o/m88k
	$(MKDIRS) ${DSTROOT}${$(RC_OS)_LOCINCDIR}/dyld
	$(MKDIRS) ${DSTROOT}${$(RC_OS)_LOCINCDIR}/cbt

depend:

clean shlib_clean:

all:
