#objdump: -h
#source: empty.s
# we should only see a text section by default.
.*: +file format mach-o.*
#...
Idx Name.*
  0 .text.*
.*
#failif
  1 .data.*
.*
  2 .bss.*
.*

