#as: -mx86-used-note=no --generate-missing-build-notes=no
#name: x86-64 size 5
#readelf: -r


Relocation section '.rela.text' at offset .* contains 3 entries:
  Offset          Info           Type           Sym. Value    Sym. Name \+ Addend
0+2  000100000021 R_X86_64_SIZE64   0000000000000000 xxx \+ 0
0+c  000100000021 R_X86_64_SIZE64   0000000000000000 xxx - 8
0+16  000100000021 R_X86_64_SIZE64   0000000000000000 xxx \+ 8

Relocation section '.rela.data' at offset .* contains 3 entries:
  Offset          Info           Type           Sym. Value    Sym. Name \+ Addend
0+50  000100000021 R_X86_64_SIZE64   0000000000000000 xxx - 1
0+58  000300000021 R_X86_64_SIZE64   0000000000000000 yyy \+ c8
0+60  000200000021 R_X86_64_SIZE64   0000000000000020 zzz \+ 0
#pass
