#name: Emit relocs 1
#source: emit-relocs-1a.s
#source: emit-relocs-1b.s
#ld: -q -T emit-relocs-1.ld
#objdump: -sr

.*:     file format .*

RELOCATION RECORDS FOR \[\.data\]:
OFFSET +TYPE +VALUE
00000000 R_CKCORE_ADDR32   \.data
00000004 R_CKCORE_ADDR32   \.data\+0x00001000
00000008 R_CKCORE_ADDR32   \.merge1\+0x00000002
0000000c R_CKCORE_ADDR32   \.merge2
00000010 R_CKCORE_ADDR32   \.merge3
00000014 R_CKCORE_ADDR32   \.merge3\+0x00000004
00000020 R_CKCORE_ADDR32   \.data\+0x00000020
00000024 R_CKCORE_ADDR32   \.data\+0x00001020
00000028 R_CKCORE_ADDR32   \.merge1
0000002c R_CKCORE_ADDR32   \.merge2\+0x00000002
00000030 R_CKCORE_ADDR32   \.merge3\+0x00000008
00000034 R_CKCORE_ADDR32   \.merge3\+0x00000004


Contents of section \.text:
 80000 036c                                 .*
Contents of section \.merge1:
 80400 666c7574 74657200                    flutter.*
Contents of section \.merge2:
 80800 74617374 696e6700                    tasting.*
Contents of section \.merge3:
 80c00 00010000 00020000 00030000           .*
Contents of section \.data:
 81000 00100800 00200800 02040800 00080800  .*
 81010 000c0800 040c0800 00000000 00000000  .*
 81020 20100800 20200800 00040800 02080800  .*
 81030 080c0800 040c0800 .*
