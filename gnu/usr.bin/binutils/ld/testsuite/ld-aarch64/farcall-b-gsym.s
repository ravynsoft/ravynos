	.global _start
	.global bar_gsym

# We will place the section .text at 0x1000.

	.text

_start:
# for long jump (JUMP26) to global symbol, we shouldn't insert veneer
# as the veneer will clobber IP0/IP1 which is caller saved, gcc only
# reserve them for function call relocation (CALL26).
	b bar_gsym
	# ((1 << 25) - 1) << 2
	.skip 134217724, 0
bar_gsym:
	nop
	ret
