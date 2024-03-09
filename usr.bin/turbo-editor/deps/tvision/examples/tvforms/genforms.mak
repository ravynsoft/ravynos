#
#   Turbo Vision Forms Demo
#
#   This makefile generates two forms data files, PHONENUM.F16 and
#   PARTS.F16 (or PHONENUM.F32 and PARTS.F32 if compiled in 32 bit
#   mode). These can be loaded and edited using TVFORMS.CPP.
#
#   Define DOS32 to build the forms for the 32 bit demo.
#
#   Define TVDEBUG to use the debugging version of the Turbo Vision
#   library.
#

.autodepend

!if !$d(BCROOT)
BCROOT=$(MAKEDIR)\..
!endif

!if !$d(TVDIR)
TVDIR=$(BCROOT)
!endif

!if "$(BCROOT)"=="$(TVDIR)"    # shorten the include/lib paths if TV
LIBPATH = $(BCROOT)\LIB;       # is in the same place as the compiler
INCPATH = $(BCROOT)\INCLUDE;
!else
LIBPATH = $(TVDIR)\LIB;$(BCROOT)\LIB
INCPATH = $(TVDIR)\INCLUDE;$(BCROOT)\INCLUDE
!endif

!ifdef TVDEBUG
TVSUFFIX = d
!endif

!if $d(DOS32)
CC      = bcc32
TVLIB   = tv32$(TVSUFFIX).lib
CCFLAGS = -WX -I$(INCPATH) -L$(LIBPATH)
!else
CC      = bcc
TVLIB   = tv"(TVSUFFIX).lib
CCFLAGS = -ml -I$(INCPATH) -L$(LIBPATH)
!endif

SRC     = genform.cpp
OBJS    = forms.obj datacoll.obj fields.obj listdlg.obj

.cpp.obj:
   $(CC) -c $(CCFLAGS) {$< }

all: phonenum.tvf parts.tvf

phonenum.tvf: $(SRC) $(OBJS)
   $(CC) -DPHONENUM $(CCFLAGS) @&&|
$(SRC) $(OBJS) $(TVLIB)
|
   genform

parts.tvf: $(SRC) $(OBJS)
   $(CC) -DPARTS $(CCFLAGS) @&&|
$(SRC) $(OBJS) $(TVLIB)
|
   genform
