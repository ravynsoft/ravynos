#objdump: -d --prefix-addresses --show-raw-insn
#name: MIPS undefined reference with --warn-unresolved-symbols
#source: undefined.s
#ld: -e foo --warn-unresolved-symbols
#warning: \A[^\n]*\.o: in function `foo':\n\(\.text\+0x0\): warning: undefined reference to `bar'\Z

.*:     file format .*

Disassembly of section \.text:

# Loaded value must not be 0.
[0-9a-f]+ <[^>]*> 2402.... 	li	v0,[-1-9][0-9]*
	\.\.\.
