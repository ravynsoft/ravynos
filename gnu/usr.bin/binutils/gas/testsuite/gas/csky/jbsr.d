# name: jbsr - csky
#as: -mcpu=ck610
#objdump: -D

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]*:\s*ffff\s*bsr\s*0x0.*
\s*[0-9a-f]*:\s*f7ff\s*br\s*0x2.*
\s*[0-9a-f]*:\s*f7ff\s*br\s*0x4.*
\s*[0-9a-f]*:\s*1200\s*\.short\s*0x1200.*
\s*[0-9a-f]*:\s*00000000\s*\.long\s*0x00000000.*
#...
