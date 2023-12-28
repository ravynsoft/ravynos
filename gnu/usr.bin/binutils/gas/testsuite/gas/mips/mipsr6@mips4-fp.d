#objdump: -dr --prefix-addresses
#name: MIPS mips4 fp

# Test mips4 fp instructions.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> recip.d	\$f4,\$f6
[0-9a-f]+ <[^>]*> recip.s	\$f4,\$f6
[0-9a-f]+ <[^>]*> rsqrt.d	\$f4,\$f6
[0-9a-f]+ <[^>]*> rsqrt.s	\$f4,\$f6
	\.\.\.
