#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS16e2 MT ASE instructions
#as: -32 -mmt

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> f0c0 3010 	ehb
[0-9a-f]+ <[^>]*> f026 6701 	dmt
[0-9a-f]+ <[^>]*> f026 6701 	dmt
[0-9a-f]+ <[^>]*> f022 6741 	dmt	v0
[0-9a-f]+ <[^>]*> f027 6701 	emt
[0-9a-f]+ <[^>]*> f027 6701 	emt
[0-9a-f]+ <[^>]*> f023 6741 	emt	v0
[0-9a-f]+ <[^>]*> f026 6700 	dvpe
[0-9a-f]+ <[^>]*> f026 6700 	dvpe
[0-9a-f]+ <[^>]*> f022 6740 	dvpe	v0
[0-9a-f]+ <[^>]*> f027 6700 	evpe
[0-9a-f]+ <[^>]*> f027 6700 	evpe
[0-9a-f]+ <[^>]*> f023 6740 	evpe	v0
	\.\.\.
