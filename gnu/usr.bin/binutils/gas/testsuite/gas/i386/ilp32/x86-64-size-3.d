#as: -mx86-used-note=no --generate-missing-build-notes=no
#name: x32 size 3
#source: ../size-3.s
#readelf: -r


Relocation section '.rela.text' at offset .* contains 6 entries:
 Offset     Info    Type            Sym.Value  Sym. Name \+ Addend
0+1  00000120 R_X86_64_SIZE32   00000000   xxx \+ 0
0+6  00000120 R_X86_64_SIZE32   00000000   xxx - 8
0+b  00000120 R_X86_64_SIZE32   00000000   xxx \+ 8
0+10  00000220 R_X86_64_SIZE32   00000000   yyy \+ 0
0+15  00000220 R_X86_64_SIZE32   00000000   yyy - 10
0+1a  00000220 R_X86_64_SIZE32   00000000   yyy \+ 10

Relocation section '.rela.tdata' at offset .* contains 2 entries:
 Offset     Info    Type            Sym.Value  Sym. Name \+ Addend
0+50  00000120 R_X86_64_SIZE32   00000000   xxx - 1
0+54  00000220 R_X86_64_SIZE32   00000000   yyy \+ 2
#pass
