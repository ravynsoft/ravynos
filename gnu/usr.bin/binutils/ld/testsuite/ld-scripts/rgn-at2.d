# name: rgn-at2
# source: rgn-at.s
# ld: -T rgn-at2.t
# objdump: -w -h

.*:     file format .*

Sections:
Idx +Name +Size +VMA +LMA +File off +Algn +Flags
  0 .text +0+[0-9a-f][0-9a-f] +0+0010000 +0+0020000 +.*
  1 .data +0+[0-9a-f][0-9a-f] +0+0030000 +0+0030000 +.*
  2 .bss +0+[0-9a-f][0-9a-f] +0+00300[0-9a-f]+ +0+00300[0-9a-f]+ +.*
