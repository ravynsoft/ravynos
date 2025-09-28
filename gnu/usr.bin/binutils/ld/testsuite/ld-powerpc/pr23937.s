	.abiversion 2
	.text
	.p2align 4,,15
	.type	implementation, @function
implementation:
.LCF0:
	addis 2,12,.TOC.-.LCF0@ha
	addi 2,2,.TOC.-.LCF0@l
	.localentry	implementation,.-implementation
	mflr 0
	addis 3,2,.LC0@toc@ha
	addi 3,3,.LC0@toc@l
	std 0,16(1)
	stdu 1,-32(1)
	bl puts
	nop
	addi 1,1,32
	li 3,0
	ld 0,16(1)
	mtlr 0
	blr
	.size	implementation,.-implementation

	.p2align 4,,15
	.type	resolver, @function
resolver:
.LCF1:
	addis 2,12,.TOC.-.LCF1@ha
	addi 2,2,.TOC.-.LCF1@l
	.localentry	resolver,.-resolver
	addis 3,2,implementation@toc@ha
	addi 3,3,implementation@toc@l
	blr
	.size	resolver,.-resolver

	.type	magic, @gnu_indirect_function
	.set	magic,resolver

	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl main
	.type	main, @function
main:
.LCF2:
	addis 2,12,.TOC.-.LCF2@ha
	addi 2,2,.TOC.-.LCF2@l
	.localentry	main,.-main
	mflr 0
	std 0,16(1)
	stdu 1,-32(1)
	bl magic
	nop
	addi 1,1,32
	cntlzw 3,3
	ld 0,16(1)
	srwi 3,3,5
	mtlr 0
	xori 3,3,0x1
	blr
	.size	main,.-main

	.section	.rodata.str1.8,"aMS",@progbits,1
	.p2align 3
.LC0:
	.string	"'ere I am JH"
