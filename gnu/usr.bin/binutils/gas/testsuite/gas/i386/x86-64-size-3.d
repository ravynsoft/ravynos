#as: -mx86-used-note=no --generate-missing-build-notes=no
#name: x86-64 size 3
#source: size-3.s
#readelf: -r


Relocation section '.rela.text' at offset .* contains 6 entries:
  Offset          Info           Type           Sym. Value    Sym. Name \+ Addend
0+1  000100000020 R_X86_64_SIZE32   0000000000000000 xxx \+ 0
0+6  000100000020 R_X86_64_SIZE32   0000000000000000 xxx - 8
0+b  000100000020 R_X86_64_SIZE32   0000000000000000 xxx \+ 8
0+10  000200000020 R_X86_64_SIZE32   0000000000000000 yyy \+ 0
0+15  000200000020 R_X86_64_SIZE32   0000000000000000 yyy - 10
0+1a  000200000020 R_X86_64_SIZE32   0000000000000000 yyy \+ 10

Relocation section '.rela.tdata' at offset .* contains 2 entries:
  Offset          Info           Type           Sym. Value    Sym. Name \+ Addend
0+50  000100000020 R_X86_64_SIZE32   0000000000000000 xxx - 1
0+54  000200000020 R_X86_64_SIZE32   0000000000000000 yyy \+ 2
#pass
