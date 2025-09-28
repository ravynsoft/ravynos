#as: -O0
#objdump: --sframe=.sframe
#name: SFrame generation on x86_64
#...
Contents of the SFrame section .sframe:

  Header :

    Version: SFRAME_VERSION_2
    Flags: NONE
    Num FDEs: 1
    Num FREs: 4

  Function Index :

    func idx \[0\]: pc = 0x0, size = 25 bytes
    STARTPC +CFA +FP +RA +
    0+0000 +sp\+8 +u +u +
    0+0001 +sp\+16 +c\-16 +u +
    0+0004 +fp\+16 +c\-16 +u +
    0+0018 +sp\+8 +c\-16 +u +
#pass
