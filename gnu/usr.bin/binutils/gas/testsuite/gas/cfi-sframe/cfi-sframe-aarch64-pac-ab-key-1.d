#as: --gsframe
#objdump: --sframe=.sframe
#name: SFrame cfi_b_key_frame and cfi_negate_ra_state composite test
#...
Contents of the SFrame section .sframe:

  Header :

    Version: SFRAME_VERSION_2
    Flags: NONE
    Num FDEs: 2
    Num FREs: 6

  Function Index :
    func idx \[0\]: pc = 0x0, size = 12 bytes
    STARTPC + CFA + FP + RA +
    0+0000 +sp\+0 +u +u +
    0+0004 +sp\+0 +u +u\[s\] +
    0+0008 +sp\+16 +c-16 +c-8\[s\] +

    func idx \[1\]: pc = 0x0, size = 20 bytes, pauth = B key
    STARTPC + CFA + FP +  RA +
    0+0000 +sp\+0 +u +u +
    0+0004 +sp\+0 +u +u\[s\] +
    0+0008 +sp\+16 +c-16 +c-8\[s\] +

#pass
