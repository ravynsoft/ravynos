#as: -mx86-used-note=no --generate-missing-build-notes=no
#objdump: -sr
#name: x86-64 (ILP32) quad

.*: +file format .*

RELOCATION RECORDS FOR \[.data\]:
OFFSET +TYPE +VALUE
0+ R_X86_64_64 +foo
0+10 R_X86_64_64 +bar
0+20 R_X86_64_64 +foo
0+30 R_X86_64_64 +bar


Contents of section .data:
 0000 00000000 00000000 efcdab90 78674512  ............xgE.
 0010 00000000 00000000 ffffffff ffffffff  ................
 0020 00000000 00000000 efcdab90 78674512  ............xgE.
 0030 00000000 00000000 ffffffff ffffffff  ................
