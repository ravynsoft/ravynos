# name: jmpi2 - csky
#as: -mcpu=ck610
#objdump: -D

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]:\s*0000\s*bkpt\s*
\s*[0-9a-f]:\s*70[0-9a-f]*\s*jmpi\s*0x0\s*\/\/\s*from\s*address\s*pool\s*at\s*0x[0-9a-f]*
\s*[0-9a-f]:\s*f7ff\s*br\s*0x[0-9a-f]*\s*\/\/\s*[0-9a-f]*\s*\<LABEL\+0x[0-9a-f]*\>
\s*[0-9a-f]:\s*f7ff\s*br\s*0x[0-9a-f]*\s*\/\/\s*[0-9a-f]*\s*\<LABEL\+0x[0-9a-f]*\>
\s*[0-9a-f]:\s*00000000\s*\.long\s*0x00000000
#...
