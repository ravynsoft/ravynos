	.text
	.global _start
_start:

.ifdef __medany__
	.option pic
.ifdef __undefweak__
	# Refer to undefined weak symbol by GOT_PCREL.
	la	t0, symbolW
	.option nopic
.else
	# Refer to global data symbol by GOT_PCREL.
	la	t0, symbolG
	.option nopic
	# Refer to local data symbol by PCREL.
	lla	t0, symbolL
	# Refer to non-pic data global symbol by PCREL.
	la	t0, symbolG
.endif
.endif

.ifdef __medlow__
.ifdef __undefweak__
	# Refer to undefined weak symbol by absolutely access.
	lui	t0, %hi(symbolW)
	addi	t0, t0, %lo(symbolW)
.else
	# Refer to local data symbol by absolutely access.
	lui	t0, %hi(symbolL)
	addi	t0, t0, %lo(symbolL)
	# Refer to global data symbol by absolutely access.
	lui	t0, %hi(symbolG)
	addi	t0, t0, %lo(symbolG)
.endif
.endif
	.size   _start, .-_start

	.data
	.global symbolG
symbolL:
	.dword	0x1111222233334444
symbolG:
	.dword	0x5555666677778888

.ifdef __undefweak__
	.weak	symbolW
.endif
