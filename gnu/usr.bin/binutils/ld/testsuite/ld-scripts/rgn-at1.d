# name: rgn-at1
# source: rgn-at.s
# ld: -T rgn-at1.t
# objdump: -w -h
# xfail: rx-*-*
#   FAILS on the RX because the linker has to set LMA == VMA for the
#   Renesas loader.

.*:     file format .*

Sections:
Idx +Name +Size +VMA +LMA +File off +Algn +Flags
  0 .text +0+[0-9a-f][0-9a-f] +0+0010000 +0+0020000 +.*
  1 .data +0+[0-9a-f][0-9a-f] +0+00100[0-9a-f]+ +0+00200[0-9a-f]+ +.*
  2 .bss +0+[0-9a-f][0-9a-f] +0+00100[0-9a-f]+ +0+00200[0-9a-f]+ +.*
