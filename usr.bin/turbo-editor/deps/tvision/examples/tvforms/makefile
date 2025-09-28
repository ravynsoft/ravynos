#
#  Turbo Vision  -  Version 2.0
#
#  TVFORMS Makefile
#
#  One of the following symbols may be defined to build an app in a
#  target other than real mode (the default).
#
#     OVERLAY -  build an overlayed DOS app
#
#     DOS16  -  build a 16 bit DPMI app
#
#     DOS32  -  build a 32 bit DPMI app
#
#
#  The following additional symbols may also be defined:
#
#     DEBUG   -  include debugging information in the app
#
#     TVDEBUG -  use the debugging version of the Turbo Vision
#                library.  This assumes that you have built
#                the debugging version of the Turbo Vision library
#                for whichever target you are compiling to.
#

.AUTODEPEND


SOURCE=tvforms

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

#
# default options produce a real mode application
#

CFGFILE    = $(SOURCE).CFG
CC         = bcc +$(CFGFILE)
TLINK      = tlink
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
CC         = bcc +$(CFGFILE)
TLINK      = tlink
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
CC         = bcc32 +$(CFGFILE)
TLINK      = tlink32
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

OBJS = $(OBJDIR)\tvforms.obj $(OBJDIR)\listdlg.obj $(OBJDIR)\forms.obj \
       $(OBJDIR)\fields.obj $(OBJDIR)\datacoll.obj

#
# link the exe
#

$(SOURCE).exe: dirs $(CFGFILE) $(OBJS)
  $(TLINK) @&&|
$(LINKFLAGS)/L$(LIBPATH) +      # linker options
$(STARTUP) +                    # startup code
$(OBJDIR)\tvforms.obj +         # user object files
$(OBJDIR)\listdlg.obj +
$(OBJDIR)\forms.obj +
$(OBJDIR)\fields.obj +
$(OBJDIR)\datacoll.obj
$(SOURCE)                       # exe name
$(SOURCE)                       # mapfile
$(LINKOVY_ON) $(USERLIBS) +     # overlayable libraries
$(LINKOVY_OFF) $(STDLIBS)       # non-overlayable libraries
|

dirs:
  IF NOT EXIST $(OBJDIR) MD $(OBJDIR)

#
# source file depends on the cfg file, which is created
# whenever the makefile is modified
#

$(OBJDIR)\tvforms.obj: $(CFGFILE) tvforms.cpp

$(OBJDIR)\listdlg.obj: $(CFGFILE) listdlg.cpp

$(OBJDIR)\forms.obj: $(CFGFILE) forms.cpp

$(OBJDIR)\fields.obj: $(CFGFILE) fields.cpp

$(OBJDIR)\datacoll.obj: $(CFGFILE) datacoll.cpp

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
