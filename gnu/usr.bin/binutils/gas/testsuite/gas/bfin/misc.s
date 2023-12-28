	.text
	.global _misc
_misc:
	/* Check "0 & 0xffff" is parsed correctly.  */
	r0.l = 0 & 0xffff;

