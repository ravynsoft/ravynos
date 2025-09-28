# name: bsr1 - csky
#as: -mcpu=ck610
#objdump: -D

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*[0-9a-f]:\s*ffff\s*bsr\s*0x0\s*\/\/\s*0\s*\<lable\>
\s*[0-9a-f]:\s*f7fe\s*br\s*0x0\s*\/\/\s*0\s*\<lable*\>
\s*[0-9a-f]:\s*e7fd\s*bt\s*0x0\s*\/\/\s*0\s*\<lable*\>
\s*[0-9a-f]:\s*effc\s*bf\s*0x0\s*\/\/\s*0\s*\<lable*\>
#...
