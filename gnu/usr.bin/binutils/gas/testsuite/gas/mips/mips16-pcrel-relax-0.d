#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16 PC-relative relaxation 0
#as: -32

# Check that PC-relative relaxation does not go into oscillation
# where the address referred depends on the size of the instruction;
# cf. RELAX_MIPS16_MARK_LONG_BRANCH.

.*: +file format .*mips.*

Disassembly of section \.text:
	\.\.\.
[0-9a-f]+ <[^>]*> f000 0a08 	la	v0,00001008 <foo\+0x8>
[0-9a-f]+ <[^>]*> e820      	jr	ra
[0-9a-f]+ <[^>]*> 6500      	nop
	\.\.\.
