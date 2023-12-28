	.section .text

        .globl sub3
sub3:
        se_blr

        .globl sub4
sub4:
        se_blr

        .globl sub5
sub5:
        se_blr

	.section .sdata
	.globl low_sdarel
low_sdarel:
	.long	2

	.globl high_adjust_sdarel
high_adjust_sdarel:
	.long	0xff

	.section .sdata2
	.globl	high_sdarel 
high_sdarel:
	.long	0xf


	.data
	.globl low
low:
	.long 5

	.globl high
high:
	.long 0x10

	.globl high_adjust
high_adjust:
	.long 0xffff
