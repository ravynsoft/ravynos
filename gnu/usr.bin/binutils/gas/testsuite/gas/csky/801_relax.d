#name: csky - 801_relax
#as: -march=ck801
#objdump: -d

.*: +file format .*csky.*

Disassembly of section \.text:
#...
\s*2:\s*0803\s*bt\s*0x8\s*.*
\s*4:\s*e800025c\s*br\s*0x4bc\s*.*
#...
\s*4bc:\s*0c03\s*bf\s*0x4c2\s*.*
\s*4be:\s*e800fda2\s*br\s*0x2\s*.*
#...
