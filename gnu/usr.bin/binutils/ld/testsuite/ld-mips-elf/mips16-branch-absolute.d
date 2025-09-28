#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 link branch to absolute expression
#source: ../../../gas/testsuite/gas/mips/mips16-branch-absolute.s
#ld: -Ttext 0 -e foo

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f100 1018 	b	0+001234 <foo\+0x234>
[0-9a-f]+ <[^>]*> f100 6016 	bteqz	0+001234 <foo\+0x234>
[0-9a-f]+ <[^>]*> f100 6114 	btnez	0+001234 <foo\+0x234>
[0-9a-f]+ <[^>]*> f100 2212 	beqz	v0,0+001234 <foo\+0x234>
[0-9a-f]+ <[^>]*> f100 2a10 	bnez	v0,0+001234 <foo\+0x234>
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
