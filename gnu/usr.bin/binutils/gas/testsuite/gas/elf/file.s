	# delta (m68k sub-target)
	.file "~tilde"

	# ia64
	.file "hash#"

	# m68k
	.ifdef m86k
	.opt nocase
	.endif
	.file "lower"
	.file "UPPER"

	# mmix
	.file ":colon"
	.ifdef mmix
	.prefix prefix
	.endif
	.file "/dir/file.s"

	# ppc/xcoff
	.file "[brackets]"
	.file "{braces}"

	# thumb (arm sub-target)
	.file "slash/data"

	# xtensa (through --rename-section file.s=file.c)
	.file "file.s"
