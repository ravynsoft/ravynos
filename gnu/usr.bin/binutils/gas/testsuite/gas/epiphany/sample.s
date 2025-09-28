 .data
foodata: .hword 42
 .text
footext:
	.text

	.macro test nm:req, args:vararg
\nm:	\nm \args
	.global \nm
	.endm

;;; Basic Instruction Tests
1:				; All branches
        test beq,1b
        test bne,1b
        test bgtu,1b
	test bgteu,1b
	test blteu,1b
        test bltu,1b
	test bgt,1b
	test bgte,1b
	test blt,1b
	test blte,1b

	test bbeq,1b
	test bbne,1b
	test bblt,1b
	test b,1b
	test bl,1b

;;; jumps
	test jr,r1
	jr	r31

	test jalr,r1
	jalr r31


	.macro test3i nm:req
	test \nm,r1,r2,r3
	\nm	r32,r33,r34
	\nm	r1,r2,#3
	\nm	r11,r2,#16
	.endm
	test3i add
	test3i sub
	test3i asr
	test3i lsr
	test3i lsl

	.macro test3 nm:req
	test \nm,r1,r2,r3
	\nm	r11,r12,r13
	.endm

	test3 orr
	test3 and
	test3 eor

	.macro testmem  nm:req
        \nm	r0,[r1,#3]
	\nm	r10,[r1,#255]
	\nm	r0,[r1,r2]
	\nm	r0,[r1,r11]
	\nm	r0,[r3],r2
	\nm	r10,[r12],r13
	.endm

	testmem ldrb
	testmem ldrh
	testmem ldr
	testmem	ldrd


	testmem strb
	testmem strh
	testmem str
	testmem	strd

	test mov,r6,#255
	mov	r31,#65535
	mov	r0,#4098

	.macro testmov cond:req
	mov\cond r1,r2
	mov\cond r11,r12
	.endm

	testmov eq
	testmov	ne
	testmov	gtu
	testmov gteu
	testmov lteu
	testmov ltu
	testmov	gt
	testmov gte
	testmov	lt
	testmov	lte
	testmov beq
	testmov bne
	testmov blt
	testmov blte
	mov	r1,r2
	mov	r11,r12

	test	nop
	test	idle
	test	bkpt

	test3	fadd
	test3	fsub
	test3	fmul
	test3	fmadd
	test3	fmsub

	movts	config,r1
	movts	status,r31

	movfs	r1,imask
	movfs	r31,pc

	test trap,#0		; write syscall for simulator.
        rti     		; dummy instruction
