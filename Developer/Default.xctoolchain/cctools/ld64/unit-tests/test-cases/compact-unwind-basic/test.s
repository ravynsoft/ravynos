#if __LP64__
	#define pointer quad
#else
	#define pointer long
#endif



	.text
	
_basic:
	nop
	nop
Lbasicend:


_multi:
	nop
	nop
Lmulti1:
	nop
Lmulti1a:
	nop
	nop
Lmulti2:
	nop
Lmultiend:
	
	
_person:
	nop
	nop
Lpersonend:
	
	
_person_lsda:
	nop
	nop
Lpersonlsdaend:
	
	
	.section __TEXT,__gcc_except_tab
_lsda1:
	.long 1
	.long 2
	
	
	.section __LD,__compact_unwind,regular,debug

	.pointer	_basic
	.set L1,Lbasicend-_basic
	.long		L1
	.long		0
	.pointer	0
	.pointer	0

	.pointer	_multi
	.set L2,Lmulti1-_multi
	.long		L2
	.long		1
	.pointer	0
	.pointer	0

	.pointer	Lmulti1
	.set L3,Lmulti2-Lmulti1
	.long		L3
	.long		2
	.pointer	0
	.pointer	0

	.pointer	Lmulti2
	.set L4,Lmultiend-Lmulti2
	.long		L4
	.long		3
	.pointer	0
	.pointer	0


	.pointer	_person
	.set L5,Lpersonend-_person
	.long		L5
	.long		0
	.pointer	_gxx_personality_v0
	.pointer	0


	.pointer	_person_lsda
	.set L6,Lpersonlsdaend-_person_lsda
	.long		L6
	.long		0
	.pointer	_gxx_personality_v0
	.pointer	_lsda1
	
	
	.subsections_via_symbols
	
	
	
	
