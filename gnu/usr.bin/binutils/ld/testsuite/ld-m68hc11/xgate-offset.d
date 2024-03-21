#as: -mm9s12xg --xgate-ramoffset
#source: xgate-offset.s
#ld: --relax -mm68hc12elf -defsym var=0x20fe
#objdump: -d --prefix-addresses -r -mm9s12xg

tmpdir/dump:     file format elf32-m68hc12


Disassembly of section .text:
00008000 <_start> ldl R1, #0xfe
00008002 <_start\+0x2> ldh R1, #0xe0
00008004 <_start\+0x4> ldl R2, #0x04
00008006 <_start\+0x6> ldh R2, #0xe2
