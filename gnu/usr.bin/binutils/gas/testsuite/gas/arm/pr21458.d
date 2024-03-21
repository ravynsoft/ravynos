#as: -mthumb-interwork
#objdump: -d --prefix-addresses --show-raw-insn
#name: ADR(L) for Thumb functions
#skip: *-*-pe *-wince-* *-*-vxworks

# Test that using ADR(L) on thumb function symbols sets the T bit when -mthumb-interwork is active.

.*: +file format .*arm.*

Disassembly of section .text:
0+00000 <.*> 4770[ 	]+bx[ 	]+lr
0+00002 <.*> 46c0[ 	]+nop[ 	]+@ \(mov r8, r8\)
0+00004 <.*> e12fff1e[ 	]+bx[ 	]+lr
0+00008 <.*> f2af 000b[ 	]+subw[ 	]+r0, pc, #11
0+0000c <.*> 4780[ 	]+blx[ 	]+r0
0+0000e <.*> f2af 020c[ 	]+subw[ 	]+r2, pc, #12
0+00012 <.*> 4790[ 	]+blx[ 	]+r2
0+00014 <.*> e24f401b[ 	]+sub[ 	]+r4, pc, #27
0+00018 <.*> e1a00000[ 	]+nop[ 	]+@ \(mov r0, r0\)
0+0001c <.*> e12fff34[ 	]+blx[ 	]+r4
0+00020 <.*> e24f6024[ 	]+sub[ 	]+r6, pc, #36[ 	]+@ 0x24
0+00024 <.*> e1a00000[ 	]+nop[ 	]+@ \(mov r0, r0\)
0+00028 <.*> e12fff36[ 	]+blx[ 	]+r6
0+0002c <.*> e24f8033[ 	]+sub[ 	]+r8, pc, #51[ 	]+@ 0x33
0+00030 <.*> e12fff38[ 	]+blx[ 	]+r8
0+00034 <.*> e24fa038[ 	]+sub[ 	]+sl, pc, #56[ 	]+@ 0x38
0+00038 <.*> e12fff3a[ 	]+blx[ 	]+sl
0+0003c <.*> 324fc043[ 	]+subcc[ 	]+ip, pc, #67[ 	]+@ 0x43
