# Don't accept a register for 'u' operands.
	auipc	x8,x9
	lui	x10,x11
# Don't accept a symbol without %hi() for 'u' operands.
	auipc	x12,symbol
	lui	x13,symbol
# Don't accept reloc functions without parentheses.
	auipc	x14,%hi symbol
	lui	x15,%hi symbol
