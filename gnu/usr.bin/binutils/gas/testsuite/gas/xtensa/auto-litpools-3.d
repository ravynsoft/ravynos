#as: --auto-litpools
#objdump: -dr
#name: auto-litpools-3 (don't move arbitrary data referenced by l32r)

.*: +file format .*xtensa.*
#...
Disassembly of section .text:
#...
 *0:.*l32r.*
.*0:.*\.data
#...
