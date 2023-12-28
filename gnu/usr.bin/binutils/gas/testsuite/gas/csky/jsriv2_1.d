# name: jsriv2_1 - csky
#as: -mcpu=ck807
#objdump: -D

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]:\s*0000\s*bkpt
\s*[0-9a-f]:\s*eae00002\s*jsri\s*0x0\s*\/\/\s*from\s*address\s*pool\s*at\s*0x[0-9a-f]*
\s*[0-9a-f]:\s*0000\s*.short\s*0x0000
\s*[0-9a-f]:\s*00000000\s*.long\s*0x00000000
#...
