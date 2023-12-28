# name: jbt - csky
#as: -mcpu=ck610
#objdump: -D

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]*:\s*e7ff\s*bt\s*0x0.*
\s*[0-9a-f]*:\s*e804\s*bf\s*0xc.*
\s*[0-9a-f]*:\s*7001\s*jmpi\s*0x0.*
\s*[0-9a-f]*:\s*0000\s*bkpt\s*
\s*[0-9a-f]*:\s*0000\s*bkpt\s*
\s*[0-9a-f]*:\s*0000\s*bkpt\s*
\s*[0-9a-f]*:\s*1200\s*mov\s*r0,\s*r0
#...
