#objdump: -dr --prefix-addresses --show-raw-insn -M reg-names=numeric
#name: MIPS mips5 instructions

# Check MIPS V instruction assembly

.*: +file format .*mips.*

Disassembly of section \.text:
0+0000 <[^>]*> 46c09428 	cvt\.s\.pl	\$f16,\$f18
0+0004 <[^>]*> 46c0a4a0 	cvt\.s\.pu	\$f18,\$f20
	\.\.\.
