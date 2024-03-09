#
#   Borland C++ - (C) Copyright 1991, 1994 by Borland International
#   Makefile for building Turbo Vision 2.0
#

#
# Flags for makefile:
#
#     -DOVERLAY      Build an overlayable version of TV.LIB.
#
#     -DDOS32        Build the 32-bit version - TV32.LIB.
#
#     -DDEBUG        Turbo Vision library with full debug info.
#
#     -DNOTASM       If you didn't purchase Turbo Assembler 4.0.
#
#     -DALIGNMENT=2  Word (=2) or dword (=4) aligned library.
#     -DALIGNMENT=4
#
#     -DEXCEPTIONS   Compile with exception handling code in the
#                    library.  Note that Turbo Vision does NOT throw
#                    any exceptions.
#
#   If you installed TVISION somewhere other than directly under
#   BORLANDC or TC, you must invoke MAKE with two defines:
#
#     -DBCROOT=... specifies the path where you installed the compiler
#
#     -DTVDIR=...  specifies the path where you installed TVISION
#
#   Examples:
#
#     for a standard installation:
#
#       make
#
#     for an installation in \COMPILER and TVISION in \AF:
#
#       make -DBCROOT=\COMPILER -DTVDIR=\AF
#
#   Any of these defines can be omitted if the corresponding name
#   is defined in the environment:
#
#       set BCROOT=\COMPILER
#       set TVDIR=\AF
#       make -DOVERLAY
#
#
#
#                       !!!!!!!!CAUTION!!!!!!!!
#
#   If you did not purchase Turbo Assembler 4.0, DO NOT DELETE the
#   .OBJ files in the \BC4\LIB\COMPAT directory.  They are built from
#   .ASM files, and you will not be able to rebuild them.
#
#   If you accidentally delete any of these .OBJ files you will have
#   to reinstall them from a backup or from your original diskettes.
#
.autodepend
.swap

!if !$d(BCROOT)
BCROOT = $(MAKEDIR)\..
!endif

!if !$d(TVDIR)
TVDIR = $(BCROOT)
!endif

!if !$d(SRCDIR)
SRCDIR = $(TVDIR)\SOURCE\TVISION
!endif

!if $d(DOS32)

!if $d(DEBUG)
LIBNAME = TV32D.LIB
OBJDIR  = $(SRCDIR)\OBJS32D
!else
LIBNAME = TV32.LIB
OBJDIR  = $(SRCDIR)\OBJS32
!endif

CFGNAME = TV32.CFG
INCNAME = TVWRIT32.INC
GENEXE  = GENINC32.EXE
GENOBJ  = GENINC32.OBJ
GENSRC  = GENINC.CPP
TARGETFLAG = -WX

CC      = $(BCROOT)\BIN\BCC32
ASM     = TASM32
LIB     = $(BCROOT)\BIN\TLIB
LINK    = $(BCROOT)\BIN\TLINK32

COMPATDIR = $(BCROOT)\LIB\COMPAT\TV32

!else   # DOS32

COMPATDIR = $(BCROOT)\LIB\COMPAT\TV

!if $d(DEBUG)
LIBNAME = TVD.LIB
OBJDIR  = $(SRCDIR)\OBJS16D
!else
LIBNAME = TV.LIB
OBJDIR  = $(SRCDIR)\OBJS16
!endif

MODEL   = l
CFGNAME = TV.CFG
INCNAME = TVWRITE.INC
GENEXE  = GENINC.EXE
GENOBJ  = GENINC.OBJ
GENSRC  = GENINC.CPP
TARGETFLAG =

CC      = $(BCROOT)\BIN\BCC
ASM     = TASM
LIB     = $(BCROOT)\BIN\TLIB
LINK    = $(BCROOT)\BIN\TLINK

!endif  # DOS32

!if "$(BCROOT)"=="$(TVDIR)"
INCLUDE  = $(BCROOT)\INCLUDE
LIBDIR   = $(BCROOT)\LIB
!else
INCLUDE  = $(TVDIR)\INCLUDE;$(BCROOT)\INCLUDE
LIBDIR   = $(TVDIR)\LIB;$(BCROOT)\LIB
!endif

TVLIBDIR = $(TVDIR)\LIB

!if $d(DEBUG)
LIBFLAGS   = /E /P128
CDEBUGFLAG = -v
ADEBUGFLAG = /zi
!else
LIBFLAGS   = /E /0
CDEBUGFLAG =
ADEBUGFLAG =
!endif

!if $d(OVERLAY)
!if !$d(DOS32)
CCOVYFLAGS = -Y -Vs
AOVYFLAGS  = /o
OBJDIR     = $(SRCDIR)\OBJSOVY
LIBNAME    = TVO.LIB
!else
!error Cannot use overlays with 32-bit version.
!endif
!endif

.path.obj = $(OBJDIR)

!if $d(ALIGNMENT)
ALIGNFLAG = -a$(ALIGNMENT)
!else
ALIGNFLAG =
!endif

!if $d(EXCEPTIONS)
EXCEPTFLAG = -x
!else
EXCEPTFLAG = -x-
!endif

!if $d(DOS32)
AFLAGS32 = /D__FLAT__
CFLAGS32 =
!else
AFLAGS32 =
CFLAGS32 = -m$(MODEL)
!endif

CFLAGS = $(CDEBUGFLAG) $(EXCEPTFLAG) $(ALIGNFLAG) $(CCOVYFLAGS) -P\
         $(CFLAGS32) -I$(INCLUDE) -n$(OBJDIR) -w -w-sig
AFLAGS = /ml /m2 $(ADEBUGFLAG) $(AOVYFLAGS) $(AFLAGS32)

!if $d(NO_STREAMABLE)
CFLAGS = $(CFLAGS) -DNO_STREAMABLE
!endif


AOBJS = pfxEDITS.OBJ    pfxFRAMELIN.OBJ pfxHARDWARE.OBJ pfxSWAPST.OBJ   \
        pfxSYSINT.OBJ   pfxTGRMV.OBJ    pfxTTPRVLNS.OBJ pfxTVCURSOR.OBJ \
        pfxTVEXPOSD.OBJ pfxTVWRITE.OBJ

TOBJS = pfxCOLORSEL.OBJ pfxDRIVERS.OBJ  pfxDRIVERS2.OBJ pfxEDITSTAT.OBJ \
        pfxGRP.OBJ      pfxHARDWRVR.OBJ pfxHELP.OBJ     pfxHELPBASE.OBJ \
        pfxHISTLIST.OBJ pfxMAPCOLOR.OBJ pfxMENU.OBJ     pfxMISC.OBJ     \
        pfxMSGBOX.OBJ   pfxNEW.OBJ      pfxNEWSTR.OBJ   pfxPALETTE.OBJ  \
        pfxSTDDLG.OBJ   pfxSYSERR.OBJ   pfxTAPPLICA.OBJ pfxTBKGRND.OBJ  \
        pfxTBUTTON.OBJ  pfxTCHDRDLG.OBJ pfxTCHECKBO.OBJ pfxTCLUSTER.OBJ \
        pfxTCMDSET.OBJ  pfxTCOLLECT.OBJ pfxTDESKTOP.OBJ pfxTDIALOG.OBJ  \
        pfxTDIRCOLL.OBJ pfxTDIRLIST.OBJ pfxTEDITOR1.OBJ pfxTEDITOR2.OBJ \
        pfxTEDITWND.OBJ pfxTEVENT.OBJ   pfxTEXTVIEW.OBJ pfxTFILDLG.OBJ  \
        pfxTFILECOL.OBJ pfxTFILEDTR.OBJ pfxTFILLIST.OBJ pfxTFRAME.OBJ   \
        pfxTGROUP.OBJ   pfxTHISTORY.OBJ pfxTHISTWIN.OBJ pfxTHSTVIEW.OBJ \
        pfxTINDICTR.OBJ pfxTINPUTLI.OBJ pfxTLABEL.OBJ   pfxTLISTBOX.OBJ \
        pfxTLSTVIEW.OBJ pfxTMEMO.OBJ    pfxTMENUBAR.OBJ pfxTMENUBOX.OBJ \
        pfxTMNUVIEW.OBJ pfxTMOUSE.OBJ   pfxTMULCHKB.OBJ pfxTOBJECT.OBJ  \
        pfxTOBJSTRM.OBJ pfxTOUTLINE.OBJ pfxTPARAMTE.OBJ pfxTSURFACE.OBJ \
        pfxTPROGRAM.OBJ pfxTRADIOBU.OBJ pfxTRESCOLL.OBJ pfxTRESFILE.OBJ \
        pfxTSCREEN.OBJ  pfxTSCRLBAR.OBJ pfxTSCROLLE.OBJ pfxTSORTCOL.OBJ \
        pfxTSTATICT.OBJ pfxTSTATUSL.OBJ pfxTSTRCOLL.OBJ pfxTSTRLIST.OBJ \
        pfxTVALIDAT.OBJ pfxTVIEW.OBJ    pfxTVTEXT1.OBJ  pfxTVTEXT2.OBJ  \
        pfxTWINDOW.OBJ  pfxTMENUPOP.OBJ pfxPOPUPMNU.OBJ pfxTSTRVIEW.OBJ \
        pfxPRNTCNST.OBJ pfxTKEY.OBJ     pfxTCLIPBRD.OBJ pfxTTIMERQU.OBJ \
        pfxSNPRINTF.OBJ pfxFMTSTR.OBJ

NOBJS = pfxNMBKGRND.OBJ pfxNMBUTTON.OBJ pfxNMCHDRDL.OBJ pfxNMCHKBOX.OBJ \
        pfxNMCLRSEL.OBJ pfxNMCLUSTR.OBJ pfxNMCOLLCT.OBJ pfxNMDIALOG.OBJ \
        pfxNMDIRBOX.OBJ pfxNMDIRCOL.OBJ pfxNMDSKTOP.OBJ pfxNMEDITOR.OBJ \
        pfxNMFILCOL.OBJ pfxNMFILDLG.OBJ pfxNMFILLST.OBJ pfxNMFRAME.OBJ  \
        pfxNMGROUP.OBJ  pfxNMHIST.OBJ   pfxNMINPTLN.OBJ pfxNMLABEL.OBJ  \
        pfxNMLSTBOX.OBJ pfxNMLSTVWR.OBJ pfxNMMNUBAR.OBJ pfxNMMNUBOX.OBJ \
        pfxNMMNUVW.OBJ  pfxNMMULCHK.OBJ pfxNMOUTLIN.OBJ pfxNMPRMTXT.OBJ \
        pfxNMRBTNS.OBJ  pfxNMRESCOL.OBJ pfxNMSCOLL.OBJ  pfxNMSCRBAR.OBJ \
        pfxNMSCROLL.OBJ pfxNMSTCTXT.OBJ pfxNMSTDDLG.OBJ pfxNMSTLINE.OBJ \
        pfxNMSTRCOL.OBJ pfxNMSTRLST.OBJ pfxNMVALIDA.OBJ pfxNMVIEW.OBJ   \
        pfxNMWINDOW.OBJ

SOBJS = pfxSBKGRND.OBJ  pfxSBUTTON.OBJ  pfxSCHDRDLG.OBJ pfxSCHECKBO.OBJ \
        pfxSCLRSEL.OBJ  pfxSCLUSTER.OBJ pfxSDESKTOP.OBJ pfxSDIALOG.OBJ  \
        pfxSDIRCOLL.OBJ pfxSDIRLIST.OBJ pfxSEDITORS.OBJ pfxSFILCOLL.OBJ \
        pfxSFILDLG.OBJ  pfxSFILELST.OBJ pfxSFINFPNE.OBJ pfxSFINPUTL.OBJ \
        pfxSFRAME.OBJ   pfxSGROUP.OBJ   pfxSHISTORY.OBJ pfxSINPUTLI.OBJ \
        pfxSLABEL.OBJ   pfxSLISTBOX.OBJ pfxSLSTVIEW.OBJ pfxSMENUBAR.OBJ \
        pfxSMENUBOX.OBJ pfxSMNUVIEW.OBJ pfxSMULCHKB.OBJ pfxSOUTLINE.OBJ \
        pfxSPARAMTE.OBJ pfxSRADIOBU.OBJ pfxSRESCOLL.OBJ pfxSSCRLBAR.OBJ \
        pfxSSCROLLE.OBJ pfxSSTATICT.OBJ pfxSSTATUSL.OBJ pfxSSTRCOLL.OBJ \
        pfxSSTRLST.OBJ  pfxSTRMSTAT.OBJ pfxSVALID.OBJ   pfxSVIEW.OBJ    \
        pfxSWINDOW.OBJ

OBJS  = $(AOBJS) $(TOBJS) $(NOBJS) $(SOBJS)


.cpp.obj:
     $(CC) -c +$(CFGNAME) {$. }

.asm.obj:
!if !$d(NOTASM)
     $(ASM) $(AFLAGS) $&.ASM $(OBJDIR)\$&.OBJ
!else
     COPY $(COMPATDIR)\$&.obj $(OBJDIR)
!endif

all: $(CFGNAME) dirs $(TVLIBDIR)\$(LIBNAME)
    DEL $(CFGNAME)

dirs:
    IF NOT EXIST $(OBJDIR) MD $(OBJDIR)

$(CFGNAME):
    ECHO $(TARGETFLAG) $(CFLAGS) > $(CFGNAME)

*.cpp: $(CFGNAME)

!if !$d(NOTASM)
*.asm: $(INCNAME)

$(INCNAME): $(GENEXE)
     $(GENEXE) > $(INCNAME)

$(GENEXE): $(GENOBJ)
     $(CC) +$(CFGNAME) -f- $(TARGETFLAG) -e$(GENEXE) -L$(LIBDIR) -n. $(OBJDIR)\$(GENOBJ)

$(GENOBJ): $(GENSRC)
     $(CC) +$(CFGNAME) -f- $(TARGETFLAG) -c -o$(OBJDIR)\$(GENOBJ) $(GENSRC)

!endif

OBJPFX=+$(OBJDIR)\\

$(TVLIBDIR)\$(LIBNAME): $(OBJS:pfx=)
     @if not exist $(TVLIBDIR) md $(TVLIBDIR)
     @if exist $(TVLIBDIR)\$(LIBNAME) del $(TVLIBDIR)\$(LIBNAME)
     $(LIB) $(LIBFLAGS) $(TVLIBDIR)\$(LIBNAME) @&&|
     $(OBJS:pfx=$(OBJPFX))
|
!if $d(OVERLAY)
     @if exist $(TVLIBDIR)\tvno.lib del $(TVLIBDIR)\tvno.lib
     $(LIB) $(LIBFLAGS) $(TVLIBDIR)\$(LIBNAME) @&&|
     -*syserr -*tscreen -*drivers -*drivers2 -*swapst -*tevent &
     -*sysint -*hardware
|
     $(LIB) $(LIBFLAGS) $(TVLIBDIR)\tvno.lib @&&|
     +syserr +tscreen +drivers +drivers2 +swapst +tevent +sysint +hardware
|
!endif

clean:
     del $(OBJDIR)\*.obj

# End of Turbo Vision 2.0 Makefile!
