#as: -KPIC -32 -relax-branch
#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS relax
#warning_output: relax.l
#source: relax.s

# Test relaxation (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45c1      	jalr	at
[0-9a-f]+ <[^>]*> 0000 0000 	nop
([0-9a-f]+) <[^>]*> b462 fffe 	bne	v0,v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
([0-9a-f]+) <[^>]*> 94a4 fffe 	beq	a0,a1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
([0-9a-f]+) <[^>]*> 40c2 fffe 	bgtz	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
([0-9a-f]+) <[^>]*> 4083 fffe 	blez	v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
([0-9a-f]+) <[^>]*> 4044 fffe 	bgez	a0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
([0-9a-f]+) <[^>]*> 4005 fffe 	bltz	a1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
([0-9a-f]+) <[^>]*> 43a0 fffe 	bc1t	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
([0-9a-f]+) <[^>]*> 4380 fffe 	bc1f	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45c1      	jalr	at
[0-9a-f]+ <[^>]*> 0000 0000 	nop
([0-9a-f]+) <[^>]*> 4003 fffe 	bltz	v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45c1      	jalr	at
[0-9a-f]+ <[^>]*> 0000 0000 	nop
([0-9a-f]+) <[^>]*> b462 fffe 	bne	v0,v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 4581      	jr	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 94a4 fffe 	beq	a0,a1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 4581      	jr	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 40c2 fffe 	bgtz	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 4581      	jr	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4083 fffe 	blez	v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 4581      	jr	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4044 fffe 	bgez	a0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 4581      	jr	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4005 fffe 	bltz	a1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 4581      	jr	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 43a0 fffe 	bc1t	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 4581      	jr	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4380 fffe 	bc1f	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 4581      	jr	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45e1      	jalrs	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4003 fffe 	bltz	v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0002 	lw	at,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0175 	addiu	at,at,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45e1      	jalrs	at
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45c1      	jalr	at
[0-9a-f]+ <[^>]*> 0000 0000 	nop
([0-9a-f]+) <[^>]*> b462 fffe 	bne	v0,v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
([0-9a-f]+) <[^>]*> 94a4 fffe 	beq	a0,a1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
([0-9a-f]+) <[^>]*> 40c2 fffe 	bgtz	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
([0-9a-f]+) <[^>]*> 4083 fffe 	blez	v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
([0-9a-f]+) <[^>]*> 4044 fffe 	bgez	a0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
([0-9a-f]+) <[^>]*> 4005 fffe 	bltz	a1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
([0-9a-f]+) <[^>]*> 43a0 fffe 	bc1t	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
([0-9a-f]+) <[^>]*> 4380 fffe 	bc1f	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45a1      	jrc	at
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45c1      	jalr	at
[0-9a-f]+ <[^>]*> 0000 0000 	nop
([0-9a-f]+) <[^>]*> 4003 fffe 	bltz	v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45c1      	jalr	at
[0-9a-f]+ <[^>]*> 0000 0000 	nop
([0-9a-f]+) <[^>]*> b462 fffe 	bne	v0,v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 4581      	jr	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 94a4 fffe 	beq	a0,a1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 4581      	jr	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 40c2 fffe 	bgtz	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 4581      	jr	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4083 fffe 	blez	v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 4581      	jr	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4044 fffe 	bgez	a0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 4581      	jr	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4005 fffe 	bltz	a1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 4581      	jr	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 43a0 fffe 	bc1t	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 4581      	jr	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4380 fffe 	bc1f	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 4581      	jr	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45e1      	jalrs	at
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4003 fffe 	bltz	v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> fc3c 0000 	lw	at,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 3021 0001 	addiu	at,at,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45e1      	jalrs	at
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
