// illegal-2.s Test file for AArch64 instructions that should be rejected
// by the assembler.  This test is a complement to the illegal.s test.
// md_apply_fix will not run if there is any error occurred in an earlier
// stage, which means errors should be reported by md_apply_fix will not
// be issued.  This test hosts instructions that will only incur error
// report from md_apply_fix.


.text
	mov	x0, #deliberately_undefined_symbol

	// immediate out of range
	add     wsp, w0, #0xfff0, LSL #12
	add     wsp, w0, #0xfff0, LSL #0
	add     wsp, w0, u16, LSL #12
	add     wsp, w0, u16, LSL #0

	// immediate cannot be moved by a single instruction
	mov	wzr, #0x0f0f0f0f
	mov	wsp, #0x33030000

.set u16, 0xfff0

	ldr	x0, [x0, #257]
