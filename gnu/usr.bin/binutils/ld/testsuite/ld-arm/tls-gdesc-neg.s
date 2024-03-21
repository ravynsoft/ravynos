	.text
	.arm
	.globl foo
	.type foo, %function
foo:
	ldr	r0, 1f
	b	2f
1:
	@ Negative addend for R_ARM_TLS_GOTDESC.
	.word	tlsdata(tlsdesc) + (. - 2f + 0)
2:
	blx	tlsdata(tlscall)

	.thumb
	.globl bar
	.type bar, %function
bar:
	ldr	r0, 1f
	b	2f
1:
	@ Negative addend for R_ARM_TLS_GOTDESC.
	.word	tlsdata(tlsdesc) + (. - 2f + 1)
2:
	blx	tlsdata(tlscall)

	.section .tdata,"awT"
	.global tlsdata
tlsdata:
	.space	4
