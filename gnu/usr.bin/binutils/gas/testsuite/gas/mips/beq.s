# Source file used to test the beq macro.
	.text
text_label:	
	beq	$4,$5,text_label
	beq	$4,0,text_label
	beq	$4,1,text_label
	beq	$4,0x8000,text_label
	beq	$4,-0x8000,text_label
	beq	$4,0x10000,text_label
	beq	$4,0x1a5a5,text_label

# bne is handled by the same code as beq.  Just sanity check.
	bne	$4,0,text_label

	.ifndef r6
# Test that branches which overflow are converted to jumps.
	.space	0x20000
	b	text_label
	bal	text_label
	.endif

# Branch to an external label.
	b	external_label
	bal	external_label

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
