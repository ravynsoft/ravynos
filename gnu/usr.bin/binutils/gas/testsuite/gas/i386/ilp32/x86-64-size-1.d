#as: -mx86-used-note=no --generate-missing-build-notes=no
#name: x32 size 1
#source: ../size-1.s
#readelf: -r


Relocation section '.rela.text' at offset .* contains 9 entries:
 Offset     Info    Type            Sym.Value  Sym. Name \+ Addend
0+1  00000120 R_X86_64_SIZE32   00000000   xxx \+ 0
0+6  00000120 R_X86_64_SIZE32   00000000   xxx - 8
0+b  00000120 R_X86_64_SIZE32   00000000   xxx \+ 8
0+10  00000220 R_X86_64_SIZE32   00000000   yyy \+ 0
0+15  00000220 R_X86_64_SIZE32   00000000   yyy - 10
0+1a  00000220 R_X86_64_SIZE32   00000000   yyy \+ 10
0+1f  00000320 R_X86_64_SIZE32   00000020   zzz \+ 0
0+24  00000320 R_X86_64_SIZE32   00000020   zzz - 20
0+29  00000320 R_X86_64_SIZE32   00000020   zzz \+ 20

Relocation section '.rela.data' at offset .* contains 3 entries:
 Offset     Info    Type            Sym.Value  Sym. Name \+ Addend
0+50  00000120 R_X86_64_SIZE32   00000000   xxx - 1
0+54  00000220 R_X86_64_SIZE32   00000000   yyy \+ 2
0+58  00000320 R_X86_64_SIZE32   00000020   zzz \+ 0
#pass
