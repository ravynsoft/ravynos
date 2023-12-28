# Source file used to test operations on odd numbered floating point
# registers.

text_label:
	lwxc1	$f1,$4($5)
	swxc1	$f3,$4($5)
