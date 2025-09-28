#objdump: -dr --prefix-addresses --show-raw-insn
#name: ARM WinCE basic tests
#as: -mcpu=arm7m -EL -mccs
#source: wince.s
#noskip: *-wince-*

# Some WinCE specific tests.

.*: +file format .*arm.*

Disassembly of section .text:
0+000 <global_data> 00000007 	andeq	r0, r0, r7
			0: ARM_32	global_data
0+004 <global_sym> e1a00000 	nop			@ \(mov r0, r0\)
0+008 <global_sym\+0x4> e1a00000 	nop			@ \(mov r0, r0\)
0+00c <global_sym\+0x8> e1a00000 	nop			@ \(mov r0, r0\)
0+010 <global_sym\+0xc> eafffffb 	b	f+ff8 <global_sym\+0xf+ff4>
			10: ARM_26D	global_sym-0x4
0+014 <global_sym\+0x10> ebfffffa 	bl	f+ff4 <global_sym\+0xf+ff0>
			14: ARM_26D	global_sym-0x4
0+018 <global_sym\+0x14> 0afffff9 	beq	f+ff0 <global_sym\+0xf+fec>
			18: ARM_26D	global_sym-0x4
0+01c <global_sym\+0x18> eafffff8 	b	0+004 <global_sym>
0+020 <global_sym\+0x1c> ebfffff7 	bl	0+004 <global_sym>
0+024 <global_sym\+0x20> 0afffff6 	beq	0+004 <global_sym>
0+028 <global_sym\+0x24> eafffff5 	b	0+004 <global_sym>
0+02c <global_sym\+0x28> ebfffff4 	bl	0+004 <global_sym>
0+030 <global_sym\+0x2c> e51f0034 	ldr	r0, \[pc, #-52\]	@ 0+004 <global_sym>
0+034 <global_sym\+0x30> e51f0038 	ldr	r0, \[pc, #-56\]	@ 0+004 <global_sym>
0+038 <global_sym\+0x34> e51f003c 	ldr	r0, \[pc, #-60\]	@ 0+004 <global_sym>
