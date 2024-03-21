# Source file used to test the bltu macro.
	
text_label:	
	bltu	$4,$5,text_label
	bltu	$0,$5,text_label
	# A second argument of 0 or $0 is always false
	bltu	$4,1,text_label
	bltu	$4,2,text_label
	bltu	$4,0x8000,text_label
	bltu	$4,-0x8000,text_label
	bltu	$4,0x10000,text_label
	bltu	$4,0x1a5a5,text_label

# bleu is handled like bltu, except when both arguments are registers.
# Just sanity check it otherwise.
	bleu	$4,$5,text_label
	bleu	$4,$0,text_label
	bleu	$4,0,text_label

# Branch to an external label.
	bltu	$4,$5,external_label
	bleu	$4,$5,external_label

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
