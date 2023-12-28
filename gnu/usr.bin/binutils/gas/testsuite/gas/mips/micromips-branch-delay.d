#objdump: -dr --prefix-addresses --show-raw-insn -mmips:micromips
#name: microMIPS branch delay
#as: -32 -march=mips64 -mmicromips
#source: micromips-branch-delay.s
#warning_output: micromips-branch-delay.l

# Test microMIPS branch delay slots.

.*: +file format .*mips.*

Disassembly of section \.text:
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 3040 ffff 	li	v0,-1
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 3040 7fff 	li	v0,32767
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 5040 ffff 	li	v0,0xffff
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 41a2 0001 	lui	v0,0x1
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> ed7f      	li	v0,-1
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 3040 7fff 	li	v0,32767
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 5040 ffff 	li	v0,0xffff
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 41a2 0001 	lui	v0,0x1
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 3040 ffff 	li	v0,-1
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 3040 7fff 	li	v0,32767
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 5040 ffff 	li	v0,0xffff
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 41a2 0001 	lui	v0,0x1
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d ffff 	addiu	v0,sp,-1
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 0008 	addiu	v0,sp,8
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 0100 	addiu	v0,sp,256
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 7fff 	addiu	v0,sp,32767
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d ffff 	addiu	v0,sp,-1
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 6d05      	addiu	v0,sp,8
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 0100 	addiu	v0,sp,256
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 7fff 	addiu	v0,sp,32767
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d ffff 	addiu	v0,sp,-1
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 0008 	addiu	v0,sp,8
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 0100 	addiu	v0,sp,256
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 7fff 	addiu	v0,sp,32767
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd ffff 	addiu	sp,sp,-1
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd 0008 	addiu	sp,sp,8
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd 0100 	addiu	sp,sp,256
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd 7fff 	addiu	sp,sp,32767
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 4fbe      	addiu	sp,sp,-1
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 4c05      	addiu	sp,sp,8
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 4c81      	addiu	sp,sp,256
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd 7fff 	addiu	sp,sp,32767
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd ffff 	addiu	sp,sp,-1
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd 0008 	addiu	sp,sp,8
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd 0100 	addiu	sp,sp,256
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd 7fff 	addiu	sp,sp,32767
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d ffff 	addiu	v0,sp,-1
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 0008 	addiu	v0,sp,8
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 0100 	addiu	v0,sp,256
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 7fff 	addiu	v0,sp,32767
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 41a1 0001 	lui	at,0x1
([0-9a-f]+) <[^>]*> 003d 1150 	addu	v0,sp,at
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d ffff 	addiu	v0,sp,-1
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 0008 	addiu	v0,sp,8
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 0100 	addiu	v0,sp,256
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 7fff 	addiu	v0,sp,32767
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 41a1 0001 	lui	at,0x1
([0-9a-f]+) <[^>]*> 003d 1150 	addu	v0,sp,at
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d ffff 	addiu	v0,sp,-1
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 0008 	addiu	v0,sp,8
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 0100 	addiu	v0,sp,256
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 305d 7fff 	addiu	v0,sp,32767
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 41a1 0001 	lui	at,0x1
([0-9a-f]+) <[^>]*> 003d 1150 	addu	v0,sp,at
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd ffff 	addiu	sp,sp,-1
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd 0008 	addiu	sp,sp,8
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd 0100 	addiu	sp,sp,256
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd 7fff 	addiu	sp,sp,32767
([0-9a-f]+) <[^>]*> 4022 fffe 	bltzal	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 41a1 0001 	lui	at,0x1
([0-9a-f]+) <[^>]*> 003d e950 	addu	sp,sp,at
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd ffff 	addiu	sp,sp,-1
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd 0008 	addiu	sp,sp,8
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd 0100 	addiu	sp,sp,256
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd 7fff 	addiu	sp,sp,32767
([0-9a-f]+) <[^>]*> 4222 fffe 	bltzals	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 41a1 0001 	lui	at,0x1
([0-9a-f]+) <[^>]*> 003d e950 	addu	sp,sp,at
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd ffff 	addiu	sp,sp,-1
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd 0008 	addiu	sp,sp,8
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd 0100 	addiu	sp,sp,256
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 33bd 7fff 	addiu	sp,sp,32767
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4060 fffe 	bal	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
([0-9a-f]+) <[^>]*> 41a1 0001 	lui	at,0x1
([0-9a-f]+) <[^>]*> 003d e950 	addu	sp,sp,at
([0-9a-f]+) <[^>]*> 0c00      	nop
	\.\.\.
