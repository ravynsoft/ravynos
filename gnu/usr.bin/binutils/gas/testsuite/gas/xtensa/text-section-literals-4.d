#as: --text-section-literals --longcalls
#objdump: -d

.*file format .*xtensa.*
#...
00000004 <foo>:
.*4:.*l32r.*a0, 0 .*
.*7:.*callx0.*a0
#...
