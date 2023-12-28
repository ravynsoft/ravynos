#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative relaxation 3
#as: -32 --defsym align=1
#source: mips16-pcrel-relax-2.s

# Check that PC-relative relaxation chooses the short encoding
# where the address referred is fixed by an alignment directive
# cf. RELAX_MIPS16_MARK_LONG_BRANCH.

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> 0a02      	la	v0,00001008 <foo\+0x8>
[0-9a-f]+ <[^>]*> e820      	jr	ra
[0-9a-f]+ <[^>]*> 6500      	nop
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
