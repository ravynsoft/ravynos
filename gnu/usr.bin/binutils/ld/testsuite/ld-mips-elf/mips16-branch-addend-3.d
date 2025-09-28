#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 link branch addend 3
#source: mips16-branch.s
#source: ../../../gas/testsuite/gas/mips/mips16-branch-addend-3.s
#ld: -Ttext 0x1c000000 -e bar

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
[0-9a-f]+ <[^>]*> f100 1008 	b	1c002234 <foo\+0x214>
[0-9a-f]+ <[^>]*> f100 6006 	bteqz	1c002234 <foo\+0x214>
[0-9a-f]+ <[^>]*> f100 6104 	btnez	1c002234 <foo\+0x214>
[0-9a-f]+ <[^>]*> f100 2202 	beqz	v0,1c002234 <foo\+0x214>
[0-9a-f]+ <[^>]*> f100 2a00 	bnez	v0,1c002234 <foo\+0x214>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
