# Source file used to test the branch-likely macros.

	.text
text_label:
# Sanity check beql and bnel
	beql	$4,0,text_label
	bnel	$4,0,text_label
	beql	$4,0,external_label
	bnel	$4,0,external_label

# Sanity test bgel and bgtl
	bgel	$4,$5,text_label
	bgtl	$4,$5,text_label
	bgel	$4,$5,external_label
	bgtl	$4,$5,external_label

# Sanity test bgeul and bgtul
	bgeul	$4,$5,text_label
	bgtul	$4,$5,text_label
	bgeul	$4,$5,external_label
	bgtul	$4,$5,external_label

# Sanity test bltl and blel
	bltl	$4,$5,text_label
	blel	$4,$5,text_label
	bltl	$4,$5,external_label
	blel	$4,$5,external_label

# Sanity test bltul and bleul
	bltul	$4,$5,text_label
	bleul	$4,$5,text_label
	bltul	$4,$5,external_label
	bleul	$4,$5,external_label

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space	8
