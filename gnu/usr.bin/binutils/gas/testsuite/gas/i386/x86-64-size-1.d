#as: -mx86-used-note=no --generate-missing-build-notes=no
#name: x86-64 size 1
#source: size-1.s
#readelf: -r


Relocation section '.rela.text' at offset .* contains 9 entries:
  Offset          Info           Type           Sym. Value    Sym. Name \+ Addend
0+1  000100000020 R_X86_64_SIZE32   0000000000000000 xxx \+ 0
0+6  000100000020 R_X86_64_SIZE32   0000000000000000 xxx - 8
0+b  000100000020 R_X86_64_SIZE32   0000000000000000 xxx \+ 8
0+10  000200000020 R_X86_64_SIZE32   0000000000000000 yyy \+ 0
0+15  000200000020 R_X86_64_SIZE32   0000000000000000 yyy - 10
0+1a  000200000020 R_X86_64_SIZE32   0000000000000000 yyy \+ 10
0+1f  000300000020 R_X86_64_SIZE32   0000000000000020 zzz \+ 0
0+24  000300000020 R_X86_64_SIZE32   0000000000000020 zzz - 20
0+29  000300000020 R_X86_64_SIZE32   0000000000000020 zzz \+ 20

Relocation section '.rela.data' at offset .* contains 3 entries:
  Offset          Info           Type           Sym. Value    Sym. Name \+ Addend
0+50  000100000020 R_X86_64_SIZE32   0000000000000000 xxx - 1
0+54  000200000020 R_X86_64_SIZE32   0000000000000000 yyy \+ 2
0+58  000300000020 R_X86_64_SIZE32   0000000000000020 zzz \+ 0
#pass
