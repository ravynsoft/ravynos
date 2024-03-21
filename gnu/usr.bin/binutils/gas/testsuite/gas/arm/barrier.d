#objdump: -dr --prefix-addresses --show-raw-insn
#name: Barrier Instruction Operands
#as: -mcpu=cortex-a8

# Test Barrier Instruction Operands

.*: *file format .*arm.*

Disassembly of section .text:
00000000 <[^>]*> f57ff05f 	dmb	(sy|#15)
00000004 <[^>]*> f57ff05e 	dmb	(st|#14)
00000008 <[^>]*> f57ff05b 	dmb	(sh|ish|#11)
0000000c <[^>]*> f57ff05b 	dmb	(sh|ish|#11)
00000010 <[^>]*> f57ff05a 	dmb	(ishst|shst|#10)
00000014 <[^>]*> f57ff05a 	dmb	(ishst|shst|#10)
00000018 <[^>]*> f57ff057 	dmb	(un|nsh|#7)
0000001c <[^>]*> f57ff057 	dmb	(un|nsh|#7)
00000020 <[^>]*> f57ff056 	dmb	(unst|nshst|#6)
00000024 <[^>]*> f57ff056 	dmb	(unst|nshst|#6)
00000028 <[^>]*> f57ff053 	dmb	(osh|#3)
0000002c <[^>]*> f57ff052 	dmb	(oshst|#2)
00000030 <[^>]*> f57ff04f 	dsb	(sy|#15)
00000034 <[^>]*> f57ff04e 	dsb	(st|#14)
00000038 <[^>]*> f57ff04b 	dsb	(sh|ish|#11)
0000003c <[^>]*> f57ff04b 	dsb	(sh|ish|#11)
00000040 <[^>]*> f57ff04a 	dsb	(ishst|ish|#10)
00000044 <[^>]*> f57ff04a 	dsb	(ishst|ish|#10)
00000048 <[^>]*> f57ff047 	dsb	(un|nsh|#7)
0000004c <[^>]*> f57ff047 	dsb	(un|nsh|#7)
00000050 <[^>]*> f57ff046 	dsb	(nshst|unst|#6)
00000054 <[^>]*> f57ff046 	dsb	(nshst|unst|#6)
00000058 <[^>]*> f57ff043 	dsb	(osh|#3)
0000005c <[^>]*> f57ff06f 	isb	(sy|#15)
00000060 <[^>]*> f57ff06f 	isb	(sy|#15)
00000064 <[^>]*> f57ff05f 	dmb	(sy|#15)
00000068 <[^>]*> f57ff05e 	dmb	(st|#14)
0000006c <[^>]*> f57ff05b 	dmb	(sh|ish|#11)
00000070 <[^>]*> f57ff05b 	dmb	(sh|ish|#11)
00000074 <[^>]*> f57ff05a 	dmb	(ishst|shst|#10)
00000078 <[^>]*> f57ff05a 	dmb	(ishst|shst|#10)
0000007c <[^>]*> f57ff057 	dmb	(un|nsh|#7)
00000080 <[^>]*> f57ff057 	dmb	(un|nsh|#7)
00000084 <[^>]*> f57ff056 	dmb	(unst|nshst|#6)
00000088 <[^>]*> f57ff056 	dmb	(unst|nshst|#6)
0000008c <[^>]*> f57ff053 	dmb	(osh|#3)
00000090 <[^>]*> f57ff052 	dmb	(oshst|#2)
00000094 <[^>]*> f57ff04f 	dsb	(sy|#15)
00000098 <[^>]*> f57ff04e 	dsb	(st|#14)
0000009c <[^>]*> f57ff04b 	dsb	(sh|ish|#11)
000000a0 <[^>]*> f57ff04b 	dsb	(sh|ish|#11)
000000a4 <[^>]*> f57ff04a 	dsb	(ishst|ish|#10)
000000a8 <[^>]*> f57ff04a 	dsb	(ishst|ish|#10)
000000ac <[^>]*> f57ff047 	dsb	(un|nsh|#7)
000000b0 <[^>]*> f57ff047 	dsb	(un|nsh|#7)
000000b4 <[^>]*> f57ff046 	dsb	(nshst|unst|#6)
000000b8 <[^>]*> f57ff046 	dsb	(nshst|unst|#6)
000000bc <[^>]*> f57ff043 	dsb	(osh|#3)
000000c0 <[^>]*> f57ff06f 	isb	(sy|#15)
000000c4 <[^>]*> f57ff040 	ssbb
000000c8 <[^>]*> f57ff04f 	dsb	(sy|#15)
000000cc <[^>]*> f57ff050 	dmb	#0
000000d0 <[^>]*> f57ff05f 	dmb	(sy|#15)
000000d4 <[^>]*> f57ff060 	isb	#0
000000d8 <[^>]*> f57ff06e 	isb	#14
000000dc <[^>]*> f57ff06b 	isb	#11
000000e0 <[^>]*> f57ff06a 	isb	#10
000000e4 <[^>]*> f57ff067 	isb	#7
000000e8 <[^>]*> f57ff066 	isb	#6
000000ec <[^>]*> f57ff063 	isb	#3
000000f0 <[^>]*> f57ff062 	isb	#2
000000f4 <[^>]*> f57ff06f 	isb	(sy|#15)
