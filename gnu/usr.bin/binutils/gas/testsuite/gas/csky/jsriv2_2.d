# name: jsriv2_2 - csky
#as: -mcpu=ck810
#objdump: -D

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]:\s*0000\s*bkpt
\s*[0-9a-f]:\s*e3ffffff\s*bsr\s*0x0\s*\/\/\s*0\s*<LABEL>
\s*[0-9a-f]:\s*c4004820\s*lsli\s*r0,\s*\s*r0,\s*0
\s*[0-9a-f]:\s*0000\s*.short\s*0x0000
\s*[0-9a-f]:\s*00000000\s*.long\s*0x00000000
#...
