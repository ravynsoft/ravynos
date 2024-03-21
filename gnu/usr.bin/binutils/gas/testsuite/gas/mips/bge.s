# Source file used to test the bge macro.
	
text_label:	
	bge	$4,$5,text_label
	bge	$4,$0,text_label
	bge	$0,$5,text_label
	bge	$4,0,text_label
	bge	$4,1,text_label
	bge	$4,2,text_label
	bge	$4,0x8000,text_label
	bge	$4,-0x8000,text_label
	bge	$4,0x10000,text_label
	bge	$4,0x1a5a5,text_label

# bgt is handled like bge, except when both arguments are registers.
# Just sanity check it otherwise.
	bgt	$4,$5,text_label
	bgt	$4,$0,text_label
	bgt	$0,$5,text_label
	bgt	$4,0,text_label

# Branch to an external label.
	bge	$4,$5,external_label
	bgt	$4,$5,external_label

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
