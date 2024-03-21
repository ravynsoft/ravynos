# source file to test illegal ext, dext, dextm, dextu instructions

	.text
text_label:
	ext	$2, $3, 1, 0
	dext	$2, $3, 1, 0
	dextm	$2, $3, 31, 2
	dextm	$2, $3, 1, 32
	dextu	$2, $3, 33, 0
