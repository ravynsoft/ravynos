#as:
#objdump: --sframe=.sframe
#name: SFrame generation on aarch64
#...
Contents of the SFrame section .sframe:
  Header :

    Version: SFRAME_VERSION_2
    Flags: NONE
    Num FDEs: 1
    Num FREs: 3

  Function Index :

    func idx \[0\]: pc = 0x0, size = 80 bytes
    STARTPC +CFA +FP +RA +
    0+0000 +sp\+0 +u +u +
    0+0004 +sp\+144 +u +u +
    0+004c +sp\+0 +u +u +
#pass
