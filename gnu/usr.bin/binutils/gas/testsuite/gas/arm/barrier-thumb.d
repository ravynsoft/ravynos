#objdump: -dr --prefix-addresses --show-raw-insn
#name: Barrier Instruction Operands (Thumb)
#source: barrier.s
#as: -mcpu=cortex-a8 -mthumb
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince

# Test Barrier Instruction Operands

.*: *file format .*arm.*

Disassembly of section .text:
00000000 <[^>]*> f3bf 8f5f 	dmb	(sy|#15)
00000004 <[^>]*> f3bf 8f5e 	dmb	(st|#14)
00000008 <[^>]*> f3bf 8f5b 	dmb	(sh|ish|#11)
0000000c <[^>]*> f3bf 8f5b 	dmb	(sh|ish|#11)
00000010 <[^>]*> f3bf 8f5a 	dmb	(ishst|shst|#10)
00000014 <[^>]*> f3bf 8f5a 	dmb	(ishst|shst|#10)
00000018 <[^>]*> f3bf 8f57 	dmb	(un|nsh|#7)
0000001c <[^>]*> f3bf 8f57 	dmb	(un|nsh|#7)
00000020 <[^>]*> f3bf 8f56 	dmb	(unst|nshst|#6)
00000024 <[^>]*> f3bf 8f56 	dmb	(unst|nshst|#6)
00000028 <[^>]*> f3bf 8f53 	dmb	(osh|#3)
0000002c <[^>]*> f3bf 8f52 	dmb	(oshst|#2)
00000030 <[^>]*> f3bf 8f4f 	dsb	(sy|#15)
00000034 <[^>]*> f3bf 8f4e 	dsb	(st|#14)
00000038 <[^>]*> f3bf 8f4b 	dsb	(sh|ish|#11)
0000003c <[^>]*> f3bf 8f4b 	dsb	(sh|ish|#11)
00000040 <[^>]*> f3bf 8f4a 	dsb	(ishst|ish|#10)
00000044 <[^>]*> f3bf 8f4a 	dsb	(ishst|ish|#10)
00000048 <[^>]*> f3bf 8f47 	dsb	(un|nsh|#7)
0000004c <[^>]*> f3bf 8f47 	dsb	(un|nsh|#7)
00000050 <[^>]*> f3bf 8f46 	dsb	(nshst|unst|#6)
00000054 <[^>]*> f3bf 8f46 	dsb	(nshst|unst|#6)
00000058 <[^>]*> f3bf 8f43 	dsb	(osh|#3)
0000005c <[^>]*> f3bf 8f6f 	isb	(sy|#15)
00000060 <[^>]*> f3bf 8f6f 	isb	(sy|#15)
00000064 <[^>]*> f3bf 8f5f 	dmb	(sy|#15)
00000068 <[^>]*> f3bf 8f5e 	dmb	(st|#14)
0000006c <[^>]*> f3bf 8f5b 	dmb	(sh|ish|#11)
00000070 <[^>]*> f3bf 8f5b 	dmb	(sh|ish|#11)
00000074 <[^>]*> f3bf 8f5a 	dmb	(ishst|shst|#10)
00000078 <[^>]*> f3bf 8f5a 	dmb	(ishst|shst|#10)
0000007c <[^>]*> f3bf 8f57 	dmb	(un|nsh|#7)
00000080 <[^>]*> f3bf 8f57 	dmb	(un|nsh|#7)
00000084 <[^>]*> f3bf 8f56 	dmb	(unst|nshst|#6)
00000088 <[^>]*> f3bf 8f56 	dmb	(unst|nshst|#6)
0000008c <[^>]*> f3bf 8f53 	dmb	(osh|#3)
00000090 <[^>]*> f3bf 8f52 	dmb	(oshst|#2)
00000094 <[^>]*> f3bf 8f4f 	dsb	(sy|#15)
00000098 <[^>]*> f3bf 8f4e 	dsb	(st|#14)
0000009c <[^>]*> f3bf 8f4b 	dsb	(sh|ish|#11)
000000a0 <[^>]*> f3bf 8f4b 	dsb	(sh|ish|#11)
000000a4 <[^>]*> f3bf 8f4a 	dsb	(ishst|ish|#10)
000000a8 <[^>]*> f3bf 8f4a 	dsb	(ishst|ish|#10)
000000ac <[^>]*> f3bf 8f47 	dsb	(un|nsh|#7)
000000b0 <[^>]*> f3bf 8f47 	dsb	(un|nsh|#7)
000000b4 <[^>]*> f3bf 8f46 	dsb	(nshst|unst|#6)
000000b8 <[^>]*> f3bf 8f46 	dsb	(nshst|unst|#6)
000000bc <[^>]*> f3bf 8f43 	dsb	(osh|#3)
000000c0 <[^>]*> f3bf 8f6f 	isb	(sy|#15)
000000c4 <[^>]*> f3bf 8f40 	ssbb
000000c8 <[^>]*> f3bf 8f4f 	dsb	(sy|#15)
000000cc <[^>]*> f3bf 8f50 	dmb	#0
000000d0 <[^>]*> f3bf 8f5f 	dmb	(sy|#15)
000000d4 <[^>]*> f3bf 8f60 	isb	#0
000000d8 <[^>]*> f3bf 8f6e 	isb	#14
000000dc <[^>]*> f3bf 8f6b 	isb	#11
000000e0 <[^>]*> f3bf 8f6a 	isb	#10
000000e4 <[^>]*> f3bf 8f67 	isb	#7
000000e8 <[^>]*> f3bf 8f66 	isb	#6
000000ec <[^>]*> f3bf 8f63 	isb	#3
000000f0 <[^>]*> f3bf 8f62 	isb	#2
000000f4 <[^>]*> f3bf 8f6f 	isb	(sy|#15)
