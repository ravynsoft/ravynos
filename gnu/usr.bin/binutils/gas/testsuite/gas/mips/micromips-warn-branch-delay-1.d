#objdump: -dr --prefix-addresses --show-raw-insn
#name: microMIPS fixed-size branch delay slots 1
#as: -32 -mmicromips
#source: micromips-warn-branch-delay-1.s

.*: +file format .*mips.*

Disassembly of section \.text:
([0-9a-f]+) <[^>]*> 4220 fffe 	bltzals	zero,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4220 fffe 	bltzals	zero,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4220 fffe 	bltzals	zero,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 45e2      	jalrs	v0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4220 fffe 	bltzals	zero,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4020 fffe 	bltzal	zero,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4020 fffe 	bltzal	zero,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4020 fffe 	bltzal	zero,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 03e2 4f3c 	jalrs	v0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4020 fffe 	bltzal	zero,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0000 0000 	nop
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
