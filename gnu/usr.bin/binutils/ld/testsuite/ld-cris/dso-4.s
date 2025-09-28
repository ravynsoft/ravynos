	.text
	.global	export_2
	.type	export_2,@function
export_2:
	.hidden dsofn
	move.d dsofn:GOTOFF,$r4
