.text

	.ascii ">>>>"
pre04:	.ascii "<<<<"
	.ascii ">>>>>"
pre0d:	.ascii "<<<"
	.ascii ">>>>>>"
pre16:	.ascii "<<"
	.ascii ">>>>>>>"
pre1f:	.ascii "<"

.data

	.ascii ">>>>"
sam04:	.ascii "<<<<"
	.ascii ">>>>>"
sam0d:	.ascii "<<<"
	.ascii ">>>>>>"
sam16:	.ascii "<<"
	.ascii ">>>>>>>"
sam1f:	.ascii "<"

	.ascii ">>>>"
	.secidx pre04
	.byte 0x11
	.secidx pre0d
	.byte 0x11
	.secidx pre16
	.byte 0x11
	.secidx pre1f
	.byte 0x11
	.ascii "<<<<<<<<"

	.ascii ">>>>"
	.secidx sam04
	.byte 0x11
	.secidx sam0d
	.byte 0x11
	.secidx sam16
	.byte 0x11
	.secidx sam1f
	.byte 0x11
	.ascii "<<<<<<<<"

	.ascii ">>>>"
	.secidx nex04
	.byte 0x11
	.secidx nex0d
	.byte 0x11
	.secidx nex16
	.byte 0x11
	.secidx nex1f
	.byte 0x11
	.ascii "<<<<<<<<"

	.ascii ">>>>"
	.secidx ext1
	.byte 0x11
	.secidx ext2
	.byte 0x11
	.secidx ext3
	.byte 0x11
	.ascii "<<<<<<<<"

.section .rdata

	.ascii ">>>>"
nex04:	.ascii "<<<<"
	.ascii ">>>>>"
nex0d:	.ascii "<<<"
	.ascii ">>>>>>"
nex16:	.ascii "<<"
	.ascii ">>>>>>>"
nex1f:	.ascii "<"
	.ascii ">>>>"

	.p2align 4,0
