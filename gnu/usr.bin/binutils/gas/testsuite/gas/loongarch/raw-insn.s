target:
	.word 0
	# Given how the LoongArch encoding space is apparently centrally-
	# managed and sequentially allocated in chunks of prefixes, it is
	# highly unlikely this would become a valid LoongArch instruction in
	# the foreseeable future.
	.word 0xfeedf00d
