# name: csky - 802j
#as: -mcpu=ck802j -W
#objdump: -D

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]*:\s*1463\s*ipop
\s*[0-9a-f]*:\s*1462\s*ipush
\s*[0-9a-f]*:\s*1460\s*nie
\s*[0-9a-f]*:\s*1461\s*nir
\s*[0-9a-f]*:\s*3ae0\s*jmpix\s*r2,\s*16.*
#...
