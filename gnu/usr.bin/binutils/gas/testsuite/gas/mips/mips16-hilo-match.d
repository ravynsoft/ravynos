#objdump: -r 
#as: -mabi=32 -march=mips1
#name: MIPS16 mips16-hilo-match

.*: +file format .*mips.*

RELOCATION RECORDS FOR \[\.text\]:
OFFSET +TYPE +VALUE
00000010 R_MIPS_HI16       var4
00000018 R_MIPS_LO16       var4
00000008 R_MIPS_HI16       __var1
00000020 R_MIPS_LO16       __var1
0000001c R_MIPS_HI16       .sbss
0000002c R_MIPS_LO16       .sbss
00000030 R_MIPS_26         func4
00000028 R_MIPS_HI16       __var3
00000034 R_MIPS_LO16       __var3
00000038 R_MIPS_LO16       hilo_match
0000003c R_MIPS_HI16       .bss
00000044 R_MIPS_LO16       .bss
00000040 R_MIPS_HI16       .bss
00000048 R_MIPS_LO16       .bss
0000004c R_MIPS_26         func3
00000024 R_MIPS_HI16       hilo_match
00000050 R_MIPS_LO16       hilo_match
0000005e R_MIPS16_HI16     .data
00000068 R_MIPS16_LO16     .data
0000006c R_MIPS16_26       func1
00000072 R_MIPS16_26       func2
00000078 R_MIPS16_HI16     hilo_match
00000080 R_MIPS16_LO16     hilo_match
00000086 R_MIPS16_LO16     hilo_match
0000008a R_MIPS16_26       func1


RELOCATION RECORDS FOR \[\.pdr\]:
OFFSET +TYPE +VALUE
00000000 R_MIPS_32         _pinit
00000020 R_MIPS_32         pdelt


