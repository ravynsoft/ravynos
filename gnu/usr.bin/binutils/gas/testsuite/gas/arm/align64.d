# name: 64 Bytes alignment test
# objdump: -dr --prefix-addresses --show-raw-insn
#skip: *-*-pe *-*-wince

.*: +file format .*arm.*

Disassembly of section .text:
00000000 <foo> f04f 0001.*mov.w.*r0, #1
00000004 <foo\+0x4> 46c0.*nop.*
00000006 <foo\+0x6> 46c0.*nop.*
00000008 <foo\+0x8> 46c0.*nop.*
0000000a <foo\+0xa> 46c0.*nop.*
0000000c <foo\+0xc> 46c0.*nop.*
0000000e <foo\+0xe> 46c0.*nop.*
00000010 <foo\+0x10> 46c0.*nop.*
00000012 <foo\+0x12> 46c0.*nop.*
00000014 <foo\+0x14> 46c0.*nop.*
00000016 <foo\+0x16> 46c0.*nop.*
00000018 <foo\+0x18> 46c0.*nop.*
0000001a <foo\+0x1a> 46c0.*nop.*
0000001c <foo\+0x1c> 46c0.*nop.*
0000001e <foo\+0x1e> 46c0.*nop.*
00000020 <foo\+0x20> 46c0.*nop.*
00000022 <foo\+0x22> 46c0.*nop.*
00000024 <foo\+0x24> 46c0.*nop.*
00000026 <foo\+0x26> 46c0.*nop.*
00000028 <foo\+0x28> 46c0.*nop.*
0000002a <foo\+0x2a> 46c0.*nop.*
0000002c <foo\+0x2c> 46c0.*nop.*
0000002e <foo\+0x2e> 46c0.*nop.*
00000030 <foo\+0x30> 46c0.*nop.*
00000032 <foo\+0x32> 46c0.*nop.*
00000034 <foo\+0x34> 46c0.*nop.*
00000036 <foo\+0x36> 46c0.*nop.*
00000038 <foo\+0x38> 46c0.*nop.*
0000003a <foo\+0x3a> 46c0.*nop.*
0000003c <foo\+0x3c> 46c0.*nop.*
0000003e <foo\+0x3e> 46c0.*nop.*
00000040 <foo\+0x40> f04f 0002.*mov.w.*r0, #2
00000044 <foo2> e3a00003.*mov.*r0, #3
00000048 <foo2\+0x4> e1a00000.*nop.*
0000004c <foo2\+0x8> e1a00000.*nop.*
00000050 <foo2\+0xc> e1a00000.*nop.*
00000054 <foo2\+0x10> e1a00000.*nop.*
00000058 <foo2\+0x14> e1a00000.*nop.*
0000005c <foo2\+0x18> e1a00000.*nop.*
00000060 <foo2\+0x1c> e1a00000.*nop.*
00000064 <foo2\+0x20> e1a00000.*nop.*
00000068 <foo2\+0x24> e1a00000.*nop.*
0000006c <foo2\+0x28> e1a00000.*nop.*
00000070 <foo2\+0x2c> e1a00000.*nop.*
00000074 <foo2\+0x30> e1a00000.*nop.*
00000078 <foo2\+0x34> e1a00000.*nop.*
0000007c <foo2\+0x38> e1a00000.*nop.*
00000080 <foo2\+0x3c> e3a00004.*mov.*r0, #4
