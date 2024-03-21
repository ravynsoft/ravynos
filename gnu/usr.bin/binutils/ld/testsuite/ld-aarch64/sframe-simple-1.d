#as: --gsframe
#source: sframe-foo.s
#source: sframe-bar.s
#objdump: --sframe=.sframe
#ld: -shared
#name: SFrame simple link

.*:     file format .*

Contents of the SFrame section .sframe:
  Header :

    Version: SFRAME_VERSION_2
    Flags: SFRAME_F_FDE_SORTED
    Num FDEs: 2
    Num FREs: 2

  Function Index :

#...
    STARTPC +CFA +FP +RA +
    0+[0-9a-f]+ +sp\+0 +u +u +

#...
    STARTPC +CFA +FP +RA +
    0+[0-9a-f]+ +sp\+0 +u +u +
