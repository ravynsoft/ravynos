	.text
_start:	nop

	# dummy empty apuinfo
	# some other tools emit these
	.section ".PPC.EMB.apuinfo"
	.long 8
	.long 0
	.long 2
	.asciz "APUinfo"
