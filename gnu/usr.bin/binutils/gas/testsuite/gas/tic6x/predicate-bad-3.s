# Test instructions that cannot be predicated.
.text
.globl f
f:
	[A1] nop
	[!B1] nop 2
	[a2] addab .D2 b14,32,b29
	[b0] addah .D1X b14,32,a5
	[!b0] addaw .D2 b14,32,b7
	[a1] callp .S1 f,a3
	[b1] addsub .L1 a1,a2,a5:a4
	[b2] addsub2 .L2 b1,b2,b5:b4
	[!a2] cmpy .M1 a1,a2,a5:a4
	[!b2] cmpyr .M1 a1,a2,a5
	[!a1] cmpyr1 .M1 a1,a2,a5
	[!b1] ddotp4 .M2 b0,b1,b3:b2
	[!a0] ddotph2 .M2 b1:b0,b2,b5:b4
	[!a0] ddotph2r .M2 b1:b0,b2,b5
	[!a0] ddotpl2 .M2 b1:b0,b2,b5:b4
	[!a0] ddotpl2r .M2 b1:b0,b2,b5
	[!b0] dint
	[a0] dpack2 .L1 a0,a1,a3:a2
	[b0] dpackx2 .L1 a0,a1,a3:a2
	[b1] gmpy .M1 a1,a2,a3
	[a1] idle
	[b2] mpy2ir .M1 a1,a2,a5:a4
	[a0] rint
	[b0] rpack2 .S1 a0,a1,a2
	[!b1] saddsub .L1 a0,a0,a1:a0
	[!b2] saddsub2 .L1 a0,a0,a1:a0
	[a0] shfl3 .L1 a0,a0,a1:a0
	[b1] smpy32 .M1 a0,a0,a0
	[a1] swe
	[!a2] swenr
	[b0] xormpy .M1 a0,a1,a2
	[a1] spmask
	[b1] spmaskr
	sploop 1
	[a0] spkernel
	[b0] spkernelr
