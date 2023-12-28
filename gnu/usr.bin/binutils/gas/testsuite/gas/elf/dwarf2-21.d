#name: DWARF2 21
#as: -gdwarf-2
#readelf: -wL -W
# Note that non-zero view numbers are allowed here.  This doesn't really
# make much sense, but DW_LNS_fixed_advance_pc is defined to not set the
# view back to zero in contrast with all other changes in PC.  A number
# of targets always use DW_LNS_fixed_advance_pc in their gas-generated
# line info.

Contents of the \.debug_line section:

CU: (.*/elf/dwarf2-21|tmpdir/asm)\.s:
File name +Line number +Starting address +View +Stmt
(dwarf2-21|asm)\.s +2 +0 +x
(dwarf2-21|asm)\.s +4 +0x10(| +1) +x
(dwarf2-21|asm)\.s +8 +0x20(| +2) +x
(dwarf2-21|asm)\.s +6 +0x30(| +3) +x
(dwarf2-21|asm)\.s +- +0x40
