#as: -32 -mips1
#objdump: -dr

.*


Disassembly of section \.text:

00000000 <\.text>:
   0:	3c1b0000 	lui	k1,0x0
			0: R_MIPS_HI16	kernelsp
   4:	8f7b0000 	lw	k1,0\(k1\)
			4: R_MIPS_LO16	kernelsp
   8:	401c7000 	mfc0	gp,c0_epc
   c:	279c0004 	addiu	gp,gp,4
