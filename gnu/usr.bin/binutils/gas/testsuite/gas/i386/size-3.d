#as: -mx86-used-note=no --generate-missing-build-notes=no
#name: i386 size 3
#readelf: -r


Relocation section '.rel.text' at offset .* contains 6 entries:
 Offset     Info    Type            Sym.Value  Sym. Name
0+1  00000126 R_386_SIZE32      00000000   xxx
0+6  00000126 R_386_SIZE32      00000000   xxx
0+b  00000126 R_386_SIZE32      00000000   xxx
0+10  00000226 R_386_SIZE32      00000000   yyy
0+15  00000226 R_386_SIZE32      00000000   yyy
0+1a  00000226 R_386_SIZE32      00000000   yyy

Relocation section '.rel.tdata' at offset .* contains 2 entries:
 Offset     Info    Type            Sym.Value  Sym. Name
0+50  00000126 R_386_SIZE32      00000000   xxx
0+54  00000226 R_386_SIZE32      00000000   yyy
#pass
