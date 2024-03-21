#as: -KPIC -32 -relax-branch --defsym atk0=1
#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS relax with .set at
#warning_output: relax.l
#source: relax.s

# Test relaxation with .set at (microMIPS).

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45da      	jalr	k0
[0-9a-f]+ <[^>]*> 0000 0000 	nop
([0-9a-f]+) <[^>]*> b462 fffe 	bne	v0,v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
([0-9a-f]+) <[^>]*> 94a4 fffe 	beq	a0,a1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
([0-9a-f]+) <[^>]*> 40c2 fffe 	bgtz	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
([0-9a-f]+) <[^>]*> 4083 fffe 	blez	v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
([0-9a-f]+) <[^>]*> 4044 fffe 	bgez	a0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
([0-9a-f]+) <[^>]*> 4005 fffe 	bltz	a1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
([0-9a-f]+) <[^>]*> 43a0 fffe 	bc1t	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
([0-9a-f]+) <[^>]*> 4380 fffe 	bc1f	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45da      	jalr	k0
[0-9a-f]+ <[^>]*> 0000 0000 	nop
([0-9a-f]+) <[^>]*> 4003 fffe 	bltz	v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45da      	jalr	k0
[0-9a-f]+ <[^>]*> 0000 0000 	nop
([0-9a-f]+) <[^>]*> b462 fffe 	bne	v0,v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 459a      	jr	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 94a4 fffe 	beq	a0,a1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 459a      	jr	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 40c2 fffe 	bgtz	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 459a      	jr	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4083 fffe 	blez	v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 459a      	jr	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4044 fffe 	bgez	a0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 459a      	jr	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4005 fffe 	bltz	a1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 459a      	jr	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 43a0 fffe 	bc1t	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 459a      	jr	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4380 fffe 	bc1f	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 459a      	jr	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45fa      	jalrs	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4003 fffe 	bltz	v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0002 	lw	k0,2\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0175 	addiu	k0,k0,373
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45fa      	jalrs	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45da      	jalr	k0
[0-9a-f]+ <[^>]*> 0000 0000 	nop
([0-9a-f]+) <[^>]*> b462 fffe 	bne	v0,v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
([0-9a-f]+) <[^>]*> 94a4 fffe 	beq	a0,a1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
([0-9a-f]+) <[^>]*> 40c2 fffe 	bgtz	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
([0-9a-f]+) <[^>]*> 4083 fffe 	blez	v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
([0-9a-f]+) <[^>]*> 4044 fffe 	bgez	a0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
([0-9a-f]+) <[^>]*> 4005 fffe 	bltz	a1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
([0-9a-f]+) <[^>]*> 43a0 fffe 	bc1t	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
([0-9a-f]+) <[^>]*> 4380 fffe 	bc1f	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45ba      	jrc	k0
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45da      	jalr	k0
[0-9a-f]+ <[^>]*> 0000 0000 	nop
([0-9a-f]+) <[^>]*> 4003 fffe 	bltz	v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45da      	jalr	k0
[0-9a-f]+ <[^>]*> 0000 0000 	nop
([0-9a-f]+) <[^>]*> b462 fffe 	bne	v0,v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 459a      	jr	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 94a4 fffe 	beq	a0,a1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 459a      	jr	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 40c2 fffe 	bgtz	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 459a      	jr	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4083 fffe 	blez	v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 459a      	jr	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4044 fffe 	bgez	a0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 459a      	jr	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4005 fffe 	bltz	a1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 459a      	jr	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 43a0 fffe 	bc1t	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 459a      	jr	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4380 fffe 	bc1f	\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 459a      	jr	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4042 fffe 	bgez	v0,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45fa      	jalrs	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
([0-9a-f]+) <[^>]*> 4003 fffe 	bltz	v1,\1 <.*>
[ 	]*[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
[0-9a-f]+ <[^>]*> 0c00      	nop
[0-9a-f]+ <[^>]*> ff5c 0000 	lw	k0,0\(gp\)
[ 	]*[0-9a-f]+: R_MICROMIPS_GOT16	\.text
[0-9a-f]+ <[^>]*> 335a 0001 	addiu	k0,k0,1
[ 	]*[0-9a-f]+: R_MICROMIPS_LO16	\.text
[0-9a-f]+ <[^>]*> 45fa      	jalrs	k0
[0-9a-f]+ <[^>]*> 0c00      	nop
	\.\.\.
