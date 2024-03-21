// programmer-friendly.s Test file for AArch64 instructions variants that are
// not part of the architectural assembly syntax but are supported for the
// ease of assembly level programming.

.text
	// The preferred architectural syntax does not accept the shifter
	// LSL or any other shift operator, when the destination register
	// has the shape of 16B or 8B.
	movi	v0.16b, 97, lsl 0	// N.B.: this is now part of the architecture specification.

	// LDR Wt, label | =value
	// As a convenience assemblers will typically permit the notation
	// "=value" in conjunction with the pc-relative literal load
	// instructions to automatically place an immediate value or
	// symbolic address in a nearby literal pool and generate a hidden
	// label which references it.
	ldrsw	x1, =0xdeadbeef
	ldrsw	x7, u16_lable + 4

	// CCMN Xn, Xm, #uimm4, cond
	// As a convenience, GAS accepts a string representation for #uimm4,
	// e.g. NzCv for #0xa (0b1010).
	ccmp	x1, x2, NzCv, GE

.data
u16_lable:
	.word	0xdeadbeef
	.word	0xcafebabe

.text
	// UXT[BHW] Wd, Wn
	// Unsigned Extend Byte|Halfword|Word: UXT[BH] is architectural alias
	// for UBFM Wd,Wn,#0,#7|15, while UXTW is pseudo instruction which is
	// encoded using ORR Wd, WZR, Wn (MOV Wd,Wn).
	// A programmer-friendly assembler should accept a destination Xd in
	// place of Wd, however that is not the preferred form for disassembly.
	uxtb	x15, w21
	uxth	x7, w27
	uxtw	x8, wzr


	// ADDS <Xd>, <Xn|SP>, <R><m>{, UXTB {#<amount>}}
	// In the 64-bit form, the final register operand is written as Wm
	// for all but the (possibly omitted) UXTX/LSL and SXTX
	// operators.
	// As a programmer-friendly assembler, we allow e.g.
	// ADDS <Xd>, <Xn|SP>, <Xm>{, UXTB {#<amount>}} by changing it to
	// ADDS <Xd>, <Xn|SP>, <Wm>{, UXTB {#<amount>}}.
	adds	x0, sp, x0, uxtb #4
	adds	x0, sp, x0, uxth #4
	adds	x0, sp, x0, uxtw #4

	adds	x0, sp, x0, sxtb #0
	adds	x0, sp, x0, sxth #1
	adds	x0, sp, x0, sxtw #2

	// More tests on
	// LDR Wt, label | =value
	// Find more comment above.
	ldr	q0, =0xdeadcafebeefbabe0123456789abcedf
	ldr	d0, =0xfebeefbabe012345
	ldr	x0, =0xfebeefbabe012345
	ldr	s0, =0xdeadbeef
	ldr	w0, =0xdeadbeef
	ldr	x0, =u16_lable
