# Source file to test offsets used with the CACHE and PREF instruction.

# By default test CACHE.

# If defined, test PREF.
	.ifdef	tpref
	.macro	cache ops:vararg
	pref	\ops
	.endm
	.endif

	.set	noreorder
	.set	noat

	.text
text_label:

	.ifdef r6
	cache	5, 255($2)
	cache	5, -256($3)
	.else
	cache	5, 2047($2)
	cache	5, -2048($3)

	# 12 bits accepted for microMIPS code.
	.ifdef	micromips
	.set	at
	.endif
	cache	5, 2048($4)
	cache	5, -2049($5)
	cache	5, 32767($6)
	cache	5, -32768($7)

	# 16 bits accepted for standard MIPS code.
	.ifndef	micromips
	.set	at
	.endif
	cache	5, 32768($8)
	cache	5, -32769($9)
	cache	5, 36864($10)
	cache	5, -36865($11)
	.endif

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8
