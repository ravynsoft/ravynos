#objdump: -s
#name: elf section7
# .pushsection always creates the named section, but the
# test harness translates ".text" into "P" for the RX...
#xfail: rx-*

.*: +file format .*

# The MIPS includes a 'section .reginfo' and such here.
#...
Contents of section .bar:
 0000 00000000 00000000 0000 .*
Contents of section .bar1:
 0000 0102 .*
Contents of section .bar2:
 0000 0102 .*
Contents of section .bar3:
 0000 0103 .*
Contents of section .bar4:
 0000 04 .*
Contents of section .text:
 0000 feff .*
# Arm includes a .ARM.attributes section here
#...
