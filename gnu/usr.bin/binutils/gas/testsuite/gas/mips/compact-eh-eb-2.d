#objdump: -sr
#name: Compact EH EB #2 with personality routine and FDE data
#source: compact-eh-2.s
#as: -EB -mno-pdr

.*:     file format.*


RELOCATION RECORDS FOR \[.data.DW.ref.__gnu_compact_pr2\]:
OFFSET +TYPE +VALUE
0+000000 R_MIPS_32         __gnu_compact_pr2
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*


RELOCATION RECORDS FOR \[.gnu_extab\]:
OFFSET +TYPE +VALUE
0+000001 R_MIPS_PC32       DW.ref.__gnu_compact_pr2
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*


RELOCATION RECORDS FOR \[.eh_frame_entry\]:
OFFSET +TYPE +VALUE
0+000000 R_MIPS_PC32       .text.*
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*
0+000004 R_MIPS_PC32       .gnu_extab
#?.*R_MIPS_NONE.*
#?.*R_MIPS_NONE.*


Contents of section .group:
 0000 00000001 00000007 00000008          .*
Contents of section .text:
 0000 00000000.*
Contents of section (.reginfo|.MIPS.options):
#...
Contents of section .MIPS.abiflags:
 .*
 .*
Contents of section .data.DW.ref.__gnu_compact_pr2:
 0000 00000000                             .*
Contents of section .gnu_extab:
 0000 00000000 0004405c                    .*
Contents of section .eh_frame_entry:
 0000 00000001 00000000                    .*
Contents of section .gnu.attributes:
 0000 41000000 0f676e75 00010000 00070401  .*
