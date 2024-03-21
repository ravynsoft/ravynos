	.text

	.set noreorder
	.set noat

	.ent text_label
	.global text_label
text_label:
	# Floating point transfer to VU
	lqc2	$0,0($0)
	lqc2	$1, 0x7fff($1)
	lqc2	$8, -0x8000($8)
	lqc2	$31, -1($31)
	.set at
	lqc2	$0, 0x8000($2)
	lqc2	$8, -0x8001($31)
	lqc2	$31, 0xF1234567($4)
	.set noat

	# Floating point transfer from VU
	sqc2	$0,0($0)
	sqc2	$1, 0x7fff($1)
	sqc2	$8, -0x8000($8)
	sqc2	$31, -1($31)
	.set at
	sqc2	$0, 0x8000($2)
	sqc2	$8, -0x8001($31)
	sqc2	$31, 0xF1234567($4)
	.set noat

	# Integer transfer from VU
	cfc2	$0,$0
	cfc2	$0,$31
	cfc2.i	$0,$0
	cfc2.i	$0,$31
	cfc2.ni	$0,$0
	cfc2.ni	$0,$31

	# Integer transfer to VU
	ctc2	$0,$0
	ctc2	$0,$31
	ctc2.i	$0,$0
	ctc2.i	$0,$31
	ctc2.ni	$0,$0
	ctc2.ni	$0,$31

	# Floating point transfer from VU
	qmfc2	$0,$0
	qmfc2	$0,$31
	qmfc2.i	$0,$0
	qmfc2.i	$0,$31
	qmfc2.ni	$0,$0
	qmfc2.ni	$0,$31

	# Floating point transfer to VU
	qmtc2	$0,$0
	qmtc2	$0,$31
	qmtc2.i	$0,$0
	qmtc2.i	$0,$31
	qmtc2.ni	$0,$0
	qmtc2.ni	$0,$31

	# COP2 conditional branch instructions
branch_label:
	bc2f    branch_label
	nop
	bc2fl   branch_label
	nop
	bc2t    branch_label
	nop
	bc2tl   branch_label
	nop

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space  8
	.end text_label
