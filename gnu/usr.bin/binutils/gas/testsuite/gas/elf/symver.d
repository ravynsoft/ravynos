#objdump: --syms
#name: ELF symbol versioning
#
# The #... and #pass are there to match extra symbols inserted by
# some toolchains, eg the mips-elf port will add .reginfo and .ptrd
# and the arm-elf toolchain will add $d.

.*:     file format .*

SYMBOL TABLE:
#...
0+000 l.*O.*\.data.*0+004 x
#...
0+000 l.*O.*\.data.*0+004 x@VERS\.0
#pass
