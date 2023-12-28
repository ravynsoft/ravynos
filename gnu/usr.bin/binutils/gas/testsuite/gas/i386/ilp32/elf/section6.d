#source: ../../../elf/section6.s
#objdump: -s
#name: elf section6

.*: +file format .*

# The MIPS includes a 'section .reginfo' and such here.
#...
Contents of section sec1:
 0+000 01 ?02 ?05.*
Contents of section sec2:
 0+000 04 ?03.*
# Arm includes a .ARM.attributes section here
#...
