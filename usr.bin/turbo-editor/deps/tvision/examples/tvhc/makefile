#
#  Turbo Vision  -  Version 2.0
#
#  TVHC Makefile
#
#  IMPORTANT!!:  The help compiler produces different output for 32 bit
#  help files, than for 16 bit (and real mode) help files.  If your
#  application is running in 32 bits, you must compile the help compiler
#  in 32 bits before building your help files.
#
#  Define DOS32 if you want to build the 32 bit help compiler.
#

.AUTODEPEND

SOURCE=tvhc

!if !$d( SOURCE )
!error You must define SOURCE= to the file you wish to compile.
!endif

!if $d( OVERLAY ) && ( $d( DOS16 ) || $d( DOS32 ) )
!error Overlays cannot be used in protected mode.
!endif

!if !$d(BCROOT)
BCROOT = $(MAKEDIR)\..
!endif

!if !$d(TVDIR)                 # default is for Turbo Vision to be
TVDIR = $(BCROOT)              # installed in same subdir as compiler
!endif

!if "$(BCROOT)"=="$(TVDIR)"    # shorten the include/lib paths if TV
LIBPATH = $(BCROOT)\LIB;       # is in the same place as the compiler
INCPATH = $(BCROOT)\INCLUDE;
!else
LIBPATH = $(TVDIR)\LIB;$(BCROOT)\LIB
INCPATH = $(TVDIR)\INCLUDE;$(BCROOT)\INCLUDE
!endif

!ifdef DEBUG                   # set appropriate flags for debugging
CCDEBUGFLAG = -v
LINKDEBUGFLAG = /v
DEBUGSUFFIX = d
!endif

!ifdef TVDEBUG                 # this suffix is added to Turbo Vision
TVSUFFIX = d                   # library name, ie TV.LIB becomes TVD.LIB
!endif

!ifdef OVERLAY                 # set overlay flags
CCOVYFLAGS  = -Y -Vs
LINKOVY_ON  = /o+
LINKOVY_OFF = /o-
!endif

!if $d(BGI)                    # set BGI library
BGILIB = graphics.lib
!if $d(DOS16)
BGILIB = bgi16.lib
!endif
!if $d(DOS32)
BGILIB = bgi32.lib
!endif
!endif

#
# default options produce a real mode application
#

CFGFILE    = $(SOURCE).CFG
CC         = $(BCROOT)\BIN\bcc +$(CFGFILE)
TLINK      = $(BCROOT)\BIN\tlink
STARTUP    = c0l.obj
USERLIBS   = tv$(TVSUFFIX).lib
STDLIBS    = $(BGILIB) emu.lib mathl.lib cl.lib
CCFLAGS    = -ml $(CCDEBUGFLAG) $(CCOVYFLAGS)
LINKFLAGS  = $(LINKDEBUGFLAG)
OBJDIR     = objs16$(DEBUGSUFFIX)

#
# overlayed application options
#

!ifdef OVERLAY
USERLIBS   = tvo$(TVSUFFIX).lib
STDLIBS    = tvno$(TVSUFFIX).lib overlay.lib emu.lib mathl.lib cl.lib
!endif

#
# 16 bit protected mode application options
#

!ifdef DOS16
CFGFILE    = $(SOURCE).C16
CC         = $(BCROOT)\BIN\bcc +$(CFGFILE)
TLINK      = $(BCROOT)\BIN\tlink
STARTUP    = c0x.obj
USERLIBS   = tv$(TVSUFFIX).lib
STDLIBS    = $(BGILIB) dpmi16.lib emux.lib mathwl.lib cwl.lib
CCFLAGS    = -WX -ml $(CCDEBUGFLAG)
LINKFLAGS  = /Txe $(LINKDEBUGFLAG)
OBJDIR     = objs16$(DEBUGSUFFIX)
!endif

#
# 32 bit protected mode application options
#

!ifdef DOS32
CFGFILE    = $(SOURCE).C32
CC         = $(BCROOT)\BIN\bcc32 +$(CFGFILE)
TLINK      = $(BCROOT)\BIN\tlink32
STARTUP    = c0x32.obj
USERLIBS   = tv32$(TVSUFFIX).lib
STDLIBS    = $(BGILIB) dpmi32.lib cw32.lib
CCFLAGS    = -WX $(CCDEBUGFLAG)
LINKFLAGS  = /Tpe/ax $(LINKDEBUGFLAG)
OBJDIR     = objs32$(DEBUGSUFFIX)
!endif

#
# implicit rule takes care of compiling all C++ files
#

.cpp.obj:
  $(CC) $(CCFLAGS) -c -n$(OBJDIR) {$< }


#
# link the exe
#

$(SOURCE).exe: dirs $(CFGFILE) $(OBJDIR)\$(SOURCE).obj
  $(TLINK) @&&|
$(LINKFLAGS)/L$(LIBPATH) +      # linker options
$(STARTUP) +                    # startup code
$(OBJDIR)\$(SOURCE).obj         # user object files
$(SOURCE)                       # exe name
$(SOURCE)                       # mapfile
$(LINKOVY_ON)$(USERLIBS) +      # overlayable libraries
$(LINKOVY_OFF)$(STDLIBS)        # non-overlayable libraries
|

#
# the following rule ensures that the appropriate OBJS
# directory exists.
#

dirs:
  IF NOT EXIST $(OBJDIR) MD $(OBJDIR)

#
# source file depends on the cfg file, which is created
# whenever the makefile is modified
#

$(OBJDIR)\$(SOURCE).obj: $(CFGFILE) $(SOURCE).cpp

#
# create a new configuration file whenever the makefile
# has been modified
#

$(CFGFILE): makefile
  copy &&|
-w
-w-sig
-x-
-I$(INCPATH)
-L$(LIBPATH)
| $(CFGFILE)
