# These instructions are new in POWER10, but enabled for POWER8 and
# later cpus.  On POWER8 and POWER9, these instructions behave as
# nop's.
	.text
_start:
	hashst   20,-8(1)
	hashst   21,-16(1)
	hashst   22,-256(1)
	hashst   23,-512(1)
	hashchk  20,-8(1)
	hashchk  21,-16(1)
	hashchk  22,-256(1)
	hashchk  23,-512(1)
	hashstp  20,-8(1)
	hashstp  21,-16(1)
	hashstp  22,-256(1)
	hashstp  23,-512(1)
	hashchkp 20,-8(1)
	hashchkp 21,-16(1)
	hashchkp 22,-256(1)
	hashchkp 23,-512(1)
