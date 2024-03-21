#objdump: -s
#name: elf section6
# The h8300 port issues a warning message for
# new sections created without atrributes.
#xfail: h8300-*

.*: +file format .*

# The MIPS includes a 'section .reginfo' and such here.
#...
Contents of section sec1:
 0+000 01 ?02 ?05.*
Contents of section sec2:
 0+000 04 ?03.*
# Arm includes a .ARM.attributes section here
#...
