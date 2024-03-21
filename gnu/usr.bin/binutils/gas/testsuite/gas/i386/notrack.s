# Check 32bit NOTRACK prefix

	.allow_index_reg
	.text
_start:
	notrack call *%eax
	notrack call *%ax
	notrack jmp *%eax
	notrack jmp *%ax

	notrack call *(%eax)
	notrack callw *(%eax)
	notrack jmp *(%eax)
	notrack jmpw *(%eax)

	notrack bnd call *%eax
	notrack bnd call *%ax
	notrack bnd jmp *%eax
	notrack bnd jmp *%ax

	notrack bnd call *(%eax)
	notrack bnd callw *(%eax)
	notrack bnd jmp *(%eax)
	notrack bnd jmpw *(%eax)

	bnd notrack call *%eax
	bnd notrack call *%ax
	bnd notrack call *(%eax)
	bnd notrack callw *(%eax)

	.intel_syntax noprefix
	notrack call eax
	notrack call ax
	notrack jmp eax
	notrack jmp ax

	notrack call DWORD PTR [eax]
	notrack call WORD PTR [eax]
	notrack jmp DWORD PTR [eax]
	notrack jmp WORD PTR [eax]

	notrack bnd call eax
	notrack bnd call ax
	notrack bnd jmp eax
	notrack bnd jmp ax

	notrack bnd call DWORD PTR [eax]
	notrack bnd call WORD PTR [eax]
	notrack bnd jmp DWORD PTR [eax]
	notrack bnd jmp WORD PTR [eax]

	bnd notrack call eax
	bnd notrack call ax
	bnd notrack call DWORD PTR [eax]
	bnd notrack call WORD PTR [eax]

	.att_syntax prefix
	# bnd notrack call *%eax
	bnd
	notrack call *%eax

	# notrack callw *%ax
	.byte 0x66
	notrack call *%eax
