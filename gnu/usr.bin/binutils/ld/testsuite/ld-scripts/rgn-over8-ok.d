# name: rgn-over8
# source: rgn-over8.s
# ld: -T rgn-over8.t
# objdump: -w -h
# xfail: rx-*-*
#   FAILS on the RX because the linker has to set LMA == VMA for the
#   Renesas loader.

.*:     file format .*

Sections:
Idx +Name +Size +VMA +LMA +File off +Algn +Flags
  0 .text         0+0000400  0+0000000  0+0000000  [0-9a-f]+  2\*\*[0-9]+  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .data         0+0000400  0+0001000  0+0000400  [0-9a-f]+  2\*\*[0-9]+  CONTENTS, ALLOC, LOAD, DATA
  2 .bss          0+0000400  0+0001400  0+0000800  [0-9a-f]+  2\*\*[0-9]+  ALLOC
