#as: --gsframe
#source: sframe-foo.s
#source: sframe-bar.s
#objdump: --sframe=.sframe
#ld: -shared
#name: SFrame for plt0 and pltN

.*: +file format .*

Contents of the SFrame section .sframe:
  Header :

    Version: SFRAME_VERSION_2
    Flags: SFRAME_F_FDE_SORTED
#...

  Function Index :

    func idx \[0\]: pc = 0x1000, size = 16 bytes
    STARTPC +CFA +FP +RA +
    0+1000 +sp\+16 +u +u +
    0+1006 +sp\+24 +u +u +

    func idx \[1\]: pc = 0x1010, size = 16 bytes
    STARTPC\[m\] +CFA +FP +RA +
    0+0000 +sp\+8 +u +u +
    0+000b +sp\+16 +u +u +

#...
