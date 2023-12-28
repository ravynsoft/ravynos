// Test file for AArch64 GAS -- instructions with relocation operators.

func:
	// BFD_RELOC_AARCH64_MOVW_G0
	// immediate
	movz	x0,#:abs_g0:u12

	// BFD_RELOC_AARCH64_MOVW_G0_S
	// immediate
	movz	x0,#:abs_g0_s:s12
	
	// BFD_RELOC_AARCH64_MOVW_G1
	// immediate
	movz	x1,#:abs_g1:u32
	movk	x1,#:abs_g0_nc:u32
	
	// BFD_RELOC_AARCH64_MOVW_G1_S
	// immediate
	movz	x1,#:abs_g1_s:s12
	movk	x1,#:abs_g0_nc:s12
	
	// BFD_RELOC_AARCH64_MOVW_G2
	// immediate
	movz	x1,#:abs_g2:u48
	movk	x1,#:abs_g1_nc:u48
	movk	x1,#:abs_g0_nc:u48
	
	// local data (section relative)
	movz	x1,#:abs_g2:ldata
	movk	x1,#:abs_g1_nc:ldata
	movk	x1,#:abs_g0_nc:ldata
	
	// external data
	movz	x1,#:abs_g2:xdata
	movk	x1,#:abs_g1_nc:xdata
	movk	x1,#:abs_g0_nc:xdata
	
	// BFD_RELOC_AARCH64_MOVW_G2_S
	// immediate
	movz	x1,#:abs_g2_s:s12
	movk	x1,#:abs_g1_nc:s12
	movk	x1,#:abs_g0_nc:s12

	// BFD_RELOC_AARCH64_MOVW_G3
	// immediate
	movz	x1,#:abs_g3:s12
	movk	x1,#:abs_g2_nc:s12
	movk	x1,#:abs_g1_nc:s12
	movk	x1,#:abs_g0_nc:s12
	
	movz	x1,#:abs_g3:u64
	movk	x1,#:abs_g2_nc:u64
	movk	x1,#:abs_g1_nc:u64
	movk	x1,#:abs_g0_nc:u64
	
	// BFD_RELOC_AARCH64_LD_LO19_PCREL
	ldr	x0,llit
	ldr	x1,ldata
	ldr	x2,xdata+12

	// BFD_RELOC_AARCH64_ADR_LO21_PCREL
	//  AARCH64 ADR instruction, holding a simple 21 bit pc-relative byte offset.
	adr	x0,llit
	adr	x1,ldata
	adr	x2,ldata+4088
	adr	x3,xlit
	adr	x4,xdata+16
	adr	x5,xdata+4088
	
	// BFD_RELOC_AARCH64_ADR_HI21_PCREL
	adrp	x0,llit
	adrp	x1,ldata
	adrp	x2,ldata+4088
	adrp	x3,xlit
	adrp	x4,xdata+16
	adrp	x5,xdata+4088
	
	// BFD_RELOC_AARCH64_ADR_HI21_PCREL
	adrp	x0,:pg_hi21:llit
	adrp	x1,:pg_hi21:ldata
	adrp	x2,:pg_hi21:ldata+4088
	adrp	x3,:pg_hi21:xlit
	adrp	x4,:pg_hi21:xdata+16
	adrp	x5,:pg_hi21:xdata+4088

	// BFD_RELOC_AARCH64_ADD_LO12
	add	x0,x0,#:lo12:llit
	add	x1,x1,#:lo12:ldata
	add	x2,x2,#:lo12:ldata+4088
	add	x3,x3,#:lo12:xlit
	add	x4,x4,#:lo12:xdata+16
	add	x5,x5,#:lo12:xdata+4088
	add	x6,x6,u12
	
	// BFD_RELOC_AARCH64_LDST8_LO12
	ldrb	w0, [x0, #:lo12:llit]
	ldrb	w1, [x1, #:lo12:ldata]
	ldrb	w2, [x2, #:lo12:ldata+4088]
	ldrb	w3, [x3, #:lo12:xlit]
	ldrb	w4, [x4, #:lo12:xdata+16]
	ldrb	w5, [x5, #:lo12:xdata+4088]
	ldrb	w6, [x6, u12]

	// BFD_RELOC_AARCH64_TSTBR14
	tbz	x0,#0,lab
	tbz	x1,#63,xlab
	tbnz	x2,#8,lab
	tbnz	x2,#47,xlab
	
	// BFD_RELOC_AARCH64_BRANCH19
	b.eq	lab
	b.eq	xlab

	// BFD_RELOC_AARCH64_COMPARE19
	cbz	x0,lab
	cbnz	x30,xlab

	// BFD_RELOC_AARCH64_JUMP26
	b	lab
	b	xlab

	// BFD_RELOC_AARCH64_CALL26
	bl	lab
	bl	xlab
	
	// BFD_RELOC_AARCH64_MOVW_IMM
	movz	x0, #0x1234, lsl #48
	movk	x0, #0x5678, lsl #32
	movk	x0, #0x9abc, lsl #16
	movk	x0, #0xdef0, lsl #0

	movz	x0, (u64>>48)&0xffff, lsl #48
	movk	x0, (u64>>32)&0xffff, lsl #32
	movk	x0, (u64>>16)&0xffff, lsl #16
	movk	x0, (u64>>0)&0xffff, lsl #0
	
	// BFD_RELOC_AARCH64_BIT_IMM
	orr	x0,x0,bit1
	and	x0,x0,bit2
	and	w0,w0,bit2
	
	// BFD_RELOC_AARCH64_ADD_U12
	add	x0,x0,s12
	add	x0,x0,u12
	sub	x0,x0,s12
	sub	x0,x0,u12

	// BFD_RELOC_AARCH64_EXC_U16
	svc	u16

	// BFD_RELOC_AARCH64_LDST_I9
	//  Signed 9-bit byte offset for load/store single item with writeback options.
	//  Used internally by the AARCH64 assembler and not (currently)
	//  written to any object files.
	ldr	x0,[x1],#s9
	ldr	x0,[x1,#s9]!

	// No writeback, but a negative offset should cause this
	// to be converted to a LDST_I9 relocation
	ldr	x0,[x1,#s9]

	// BFD_RELOC_AARCH64_LDST_U12
	//  Unsigned 12-bit byte offset for load/store single item without options.
	//  Used internally by the AARCH64 assembler and not (currently)
	//  written to any object files.
	ldr	x0,[x1,#(u12*8)]

	// BFD_RELOC_AARCH64_LDST16_LO12
	ldrh	w0, [x0, #:lo12:llit]
	// BFD_RELOC_AARCH64_LDST32_LO12
	ldr	w1, [x1, #:lo12:ldata]
	// BFD_RELOC_AARCH64_LDST64_LO12
	ldr	x2, [x2, #:lo12:ldata+4088]
	// BFD_RELOC_AARCH64_LDST128_LO12
	ldr	q3, [x3, #:lo12:xlit]

	// BFD_RELOC_AARCH64_LDST64_LO12
	prfm	pstl1keep, [x7, #:lo12:ldata+4100]

	// BFD_RELOC_AARCH64_GOT_LD_PREL19
	ldr	x0, :got:cdata
	ldrb	w1, [x0]
	
	ret
	
	// BFD_RELOC_AARCH64_LD64_GOTPAGE_LO15
	ldr	x28, [x13, #:gotpage_lo15:dummy]
	// BFD_RELOC_AARCH64_LD64_GOTOFF_LO15
	ldr	x0, [x0, #:gotoff_lo15:dummy]

llit:	.word	0xdeadf00d
	
lab:	
		
	.data
	.align 8
	
dummy:	.xword  0
	
ldata:	.xword	0x1122334455667788
	.space	8184
	
.set u8, 248
.set s9, -256
.set s12, -2048	
.set u12, 4095
.set u16, 65535
.set u32, 0x12345678
.set u48, 0xaabbccddeeff
.set u64, 0xfedcba9876543210
.set bit1,0xf000000000000000
.set bit2,~0xf

.comm	cdata,1,8
