#as: -mx86-used-note=no --generate-missing-build-notes=no
#name: i386 size 1
#readelf: -r


Relocation section '.rel.text' at offset .* contains 9 entries:
 Offset     Info    Type            Sym.Value  Sym. Name
0+1  00000126 R_386_SIZE32      00000000   xxx
0+6  00000126 R_386_SIZE32      00000000   xxx
0+b  00000126 R_386_SIZE32      00000000   xxx
0+10  00000226 R_386_SIZE32      00000000   yyy
0+15  00000226 R_386_SIZE32      00000000   yyy
0+1a  00000226 R_386_SIZE32      00000000   yyy
0+1f  00000326 R_386_SIZE32      00000020   zzz
0+24  00000326 R_386_SIZE32      00000020   zzz
0+29  00000326 R_386_SIZE32      00000020   zzz

Relocation section '.rel.data' at offset .* contains 3 entries:
 Offset     Info    Type            Sym.Value  Sym. Name
0+50  00000126 R_386_SIZE32      00000000   xxx
0+54  00000226 R_386_SIZE32      00000000   yyy
0+58  00000326 R_386_SIZE32      00000020   zzz
#pass
