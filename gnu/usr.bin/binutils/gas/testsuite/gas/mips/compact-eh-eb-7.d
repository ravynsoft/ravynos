#objdump: -sr
#name: Compact EH EB #7 with personality id and fallback FDE
#source: compact-eh-7.s
#as: -EB -mno-pdr
#notarget: mips64*-*-openbsd* mips64*-*-*-gnuabi64

.*:     file format.*


RELOCATION RECORDS FOR \[.eh_frame\]:
OFFSET +TYPE +VALUE
0000001c R_MIPS_PC32       .text.*


RELOCATION RECORDS FOR \[.eh_frame_entry\]:
OFFSET +TYPE +VALUE
00000000 R_MIPS_PC32       .text.*
00000004 R_MIPS_PC32       .eh_frame.*


Contents of section .text:
 0000 00000000 00000000.*
Contents of section .reginfo:
 0000 00000000 00000000 00000000 00000000  .*
 0010 00000000 00000000                    .*
Contents of section .MIPS.abiflags:
 .*
 .*
Contents of section .eh_frame:
 0000 00000010 00000000 017a5200 017c1f01  .*
 0010 1b0d1d00 00000014 00000018 00000000  .*
 0020 00000008 00441308 440e0000           .*
Contents of section .eh_frame_entry:
 0000 00000001 00000015                    .*
Contents of section .gnu.attributes:
 0000 41000000 0f676e75 00010000 00070401  .*
