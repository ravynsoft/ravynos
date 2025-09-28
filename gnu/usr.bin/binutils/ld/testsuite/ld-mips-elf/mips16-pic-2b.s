	.abicalls
	.set	noreorder
	.include "mips16-pic-1.inc"

	.set	mips16
	.ent	bar
bar:
	pic_prologue mips16
	pic_call     unused4,mips16
	pic_call     used7,mips16
	pic_epilogue
	.end	bar

	.data
	.word	used3
	.word	used5
