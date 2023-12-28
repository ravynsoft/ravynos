# as: --generate-missing-build-notes=yes
# objdump: -r

# Check that the relocations are for increasing addresses...

#...
RELOCATION RECORDS FOR \[.gnu.build.attributes\]:
OFFSET +TYPE +VALUE
0+014 .*[ 	]+.*
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+0(18|1c) .*[ 	]+.*
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+0(30|38) .*[ 	]+.*
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+0(34|40) .*[ 	]+.*
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
#pass
