#objdump: -dr --prefix-addresses --show-raw-insn
#name: MIPS jal-svr4pic
#as: -32 -KPIC
#source: jal-svr4pic.s

# Test the jal macro with -KPIC.

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 3c1c0000 	lui	gp,0x0
[ 	]*[0-9a-f]+: R_MIPS_HI16	_gp_disp
[0-9a-f]+ <[^>]*> 279c0000 	addiu	gp,gp,0
[ 	]*[0-9a-f]+: R_MIPS_LO16	_gp_disp
[0-9a-f]+ <[^>]*> 0399e021 	addu	gp,gp,t9
[0-9a-f]+ <[^>]*> afbc0000 	sw	gp,0\(sp\)
[0-9a-f]+ <[^>]*> 0320f809 	jalr	t9
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 03202009 	jalr	a0,t9
[0-9a-f]+ <[^>]*> 8fbc0000 	lw	gp,0\(sp\)
[0-9a-f]+ <[^>]*> 8fbc0000 	lw	gp,0\(sp\)
[0-9a-f]+ <[^>]*> 8f990000 	lw	t9,0\(gp\)
[ 	]*[0-9a-f]+: R_MIPS_GOT16	.text
[0-9a-f]+ <[^>]*> 27390000 	addiu	t9,t9,0
[ 	]*[0-9a-f]+: R_MIPS_LO16	.text
[0-9a-f]+ <[^>]*> 0320f809 	jalr	t9
[ 	]*[0-9a-f]+: R_MIPS_JALR	text_label
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 8fbc0000 	lw	gp,0\(sp\)
[0-9a-f]+ <[^>]*> 8f990000 	lw	t9,0\(gp\)
[ 	]*[0-9a-f]+: R_MIPS_CALL16	weak_text_label
[0-9a-f]+ <[^>]*> 0320f809 	jalr	t9
[ 	]*[0-9a-f]+: R_MIPS_JALR	weak_text_label
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 8fbc0000 	lw	gp,0\(sp\)
[0-9a-f]+ <[^>]*> 8f990000 	lw	t9,0\(gp\)
[ 	]*[0-9a-f]+: R_MIPS_CALL16	external_text_label
[0-9a-f]+ <[^>]*> 0320f809 	jalr	t9
[ 	]*[0-9a-f]+: R_MIPS_JALR	external_text_label
[0-9a-f]+ <[^>]*> 00000000 	nop
[0-9a-f]+ <[^>]*> 1000ffff 	b	0+0054 <text_label\+0x54>
[	]*54: .*R_MIPS_PC16	text_label
[0-9a-f]+ <[^>]*> 8fbc0000 	lw	gp,0\(sp\)
	\.\.\.
