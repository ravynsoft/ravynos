#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS branches to self
#as: -32
#source: branch-self.s

# Test various ways to request a branch to self (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
([0-9a-f]+) <[^>]*> e930      	sw	v0,0\(v1\)
([0-9a-f]+) <[^>]*> cfff      	b	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> e930      	sw	v0,0\(v1\)
([0-9a-f]+) <[^>]*> cfff      	b	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> e930      	sw	v0,0\(v1\)
([0-9a-f]+) <[^>]*> cfff      	b	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> e930      	sw	v0,0\(v1\)
([0-9a-f]+) <[^>]*> cfff      	b	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> e930      	sw	v0,0\(v1\)
([0-9a-f]+) <[^>]*> cfff      	b	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> e930      	sw	v0,0\(v1\)
([0-9a-f]+) <[^>]*> cfff      	b	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC10_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
	\.\.\.
	\.\.\.
