# name: Compatibility with Renesas's own assembler
# objdump: -D --prefix-addresses --show-raw-insn 

.*: +file format elf32-rx-.*


Disassembly of section P:
.*
.*
0+0108 <mem\+0x8> 66 20[ 	]+mov.l[ 	]+#2, r0
0+010a <mem\+0xa> 66 10[ 	]+mov.l[ 	]+#1, r0
0+010c <mem\+0xc> 66 00[ 	]+mov.l[ 	]+#0, r0
0+010e <mem\+0xe> 05 .. .. ..[ 	]+bsr.a[ 	]+[0-9a-f]+ <mem.0x[0-9a-f]+>
0+0112 <mem\+0x12> 05 .. .. ..[ 	  ]+bsr.a[ 	]+[0-9a-f]+ <mem.0x[0-9a-f]+>
0+0116 <mem\+0x16> 62 65[ 	]+add[ 	]+#6, r5
0+0118 <mem\+0x18> 72 74 0b 2e[ 	]+add[ 	]+#0x2e0b, r7, r4
0+011c <mem\+0x1c> ff 2e 00[ 	]+add[ 	]+r0, r0, r14
.*

Disassembly of section D_1:
0+0000 <dmem> 01.*
0+0001 <dmem\+0x1> 00.*
0+0002 <dmem\+0x2> 00.*
0+0003 <dmem\+0x3> 64 61.*
0+0005 <dmem\+0x5> 74 00 00 00 00 00.*
.*
0+004f <dmem\+0x4f> 01.*
0+0050 <dmem\+0x50> 64 61.*
0+0052 <dmem\+0x52> 74 61.*
0+0054 <dmem\+0x54> 00.*
0+0055 <dmem\+0x55> 00.*
0+0056 <dmem\+0x56> 00.*
0+0057 <dmem\+0x57> fa 43 b6 f3 9d 3f 00 00.*
0+005f <dmem\+0x5f> fa 43 01 00 00 00 74 77.*
0+0067 <dmem\+0x67> 6f 07.*
0+0069 <dmem\+0x69> 00.*
0+006a <dmem\+0x6a> 00.*
0+006b <dmem\+0x6b> 00.*
0+006c <dmem\+0x6c> 03.*
0+006d <dmem\+0x6d> 00.*
.*
