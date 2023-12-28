# name: csky - ck803r2
#as: -mcpu=ck803r2
#objdump: -D

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]*:\s*e8200002\s*bnezad\s*r0, 0x4.*
#...
\s*[0-9a-f]*:\s*6c03\s*mov\s*r0,\s*r0
\s*[0-9a-f]*:\s*e820fffd\s*bnezad\s*r0,\s*0.*
#...
