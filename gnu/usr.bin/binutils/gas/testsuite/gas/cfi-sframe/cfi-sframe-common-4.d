#as: --gsframe
#objdump: --sframe=.sframe
#name: SFrame cfi_def_cfa_offset test
#...
Contents of the SFrame section .sframe:

  Header :

    Version: SFRAME_VERSION_2
    Flags: NONE
    Num FDEs: 1
    Num FREs: 3

  Function Index :
    func idx \[0\]: pc = 0x0, size = 12 bytes
    STARTPC + CFA + FP + RA +
#...
    0+0004 +sp\+16 +u +u +
    0+0008 +sp\+32 +u +u +

#pass
