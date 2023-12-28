# Source file used to test the jal macro.
	.globl	text_label	.text
text_label:	
	jal	$25
	jal	$4,$25
	jal	text_label
	jal	external_text_label
	
# Test j as well	
	j	text_label
	j	external_text_label

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
