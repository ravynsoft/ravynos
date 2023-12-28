	.symver x,expfn@TST2	; .symver required to make @ part of name.
	.global _start
	.type	_start,@function
_start:
	.dword	0,0,0,0
	.reloc	0,R_CRIS_32_PCREL,expfn
	.reloc	4,R_CRIS_32_PCREL,expfn
	.reloc	8,R_CRIS_32_PCREL,x
	.reloc	12,R_CRIS_32_PCREL,x
.Lfe3:
	.size	_start,.Lfe3-_start

	.data
	.global tab1
	.type	tab1,@object
tab1:
	.dword	0,0,0,0,0,0,0,0
	.reloc	0,R_CRIS_32_PCREL,expfn
	.reloc	4,R_CRIS_32_PCREL,expfn
	.reloc	8,R_CRIS_32_PCREL,expfn
	.reloc	12,R_CRIS_32_PCREL,expfn
	.reloc	16,R_CRIS_32_PCREL,x
	.reloc	20,R_CRIS_32_PCREL,x
	.reloc	24,R_CRIS_32_PCREL,x
	.reloc	28,R_CRIS_32_PCREL,x
	.size	tab1,.-tab1

	.section .data2,"aw",@progbits
	.global tab2
	.type	tab2,@object
tab2:
	.dword	0,0,0,0,0,0,0,0
	.dword	0,0,0,0,0,0,0,0
	.reloc	0,R_CRIS_32_PCREL,expfn
	.reloc	4,R_CRIS_32_PCREL,expfn
	.reloc	8,R_CRIS_32_PCREL,expfn
	.reloc	12,R_CRIS_32_PCREL,expfn
	.reloc	16,R_CRIS_32_PCREL,expfn
	.reloc	20,R_CRIS_32_PCREL,expfn
	.reloc	24,R_CRIS_32_PCREL,expfn
	.reloc	28,R_CRIS_32_PCREL,expfn
	.reloc	32,R_CRIS_32_PCREL,x
	.reloc	36,R_CRIS_32_PCREL,x
	.reloc	40,R_CRIS_32_PCREL,x
	.reloc	44,R_CRIS_32_PCREL,x
	.reloc	48,R_CRIS_32_PCREL,x
	.reloc	52,R_CRIS_32_PCREL,x
	.reloc	56,R_CRIS_32_PCREL,x
	.reloc	60,R_CRIS_32_PCREL,x
	.size	tab1,.-tab2
