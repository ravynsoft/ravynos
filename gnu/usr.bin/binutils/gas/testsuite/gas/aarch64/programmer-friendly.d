#as: -mabi=lp64
#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0000000000000000 <.*>:
   0:	4f03e420 	movi	v0\.16b, #0x61
   4:	98000241 	ldrsw	x1, 4c <\.text\+0x4c>
   8:	98000007 	ldrsw	x7, 0 <\.text>
			8: R_AARCH64_LD_PREL_LO19	\.data\+0x4
   c:	fa42a02a 	ccmp	x1, x2, #0xa, ge	// ge = tcont
  10:	53001eaf 	uxtb	w15, w21
  14:	53003f67 	uxth	w7, w27
  18:	2a1f03e8 	mov	w8, wzr
  1c:	ab2013e0 	adds	x0, sp, w0, uxtb #4
  20:	ab2033e0 	adds	x0, sp, w0, uxth #4
  24:	ab2053e0 	adds	x0, sp, w0, uxtw #4
  28:	ab2083e0 	adds	x0, sp, w0, sxtb
  2c:	ab20a7e0 	adds	x0, sp, w0, sxth #1
  30:	ab20cbe0 	adds	x0, sp, w0, sxtw #2
  34:	9c000160 	ldr	q0, 60 <\.text\+0x60>
  38:	5c0000c0 	ldr	d0, 50 <\.text\+0x50>
  3c:	580000a0 	ldr	x0, 50 <\.text\+0x50>
  40:	1c000060 	ldr	s0, 4c <\.text\+0x4c>
  44:	18000040 	ldr	w0, 4c <\.text\+0x4c>
  48:	58000080 	ldr	x0, 58 <\.text\+0x58>
  4c:	deadbeef 	\.word	0xdeadbeef
  50:	.* 	\.word	0x.*
  54:	.* 	\.word	0x.*
	...
			58: R_AARCH64_ABS64	\.data
  60:	.*	\.word	0x.*
  64:	.* 	\.word	0x.*
  68:	.* 	\.word	0x.*
  6c:	.* 	\.word	0x.*
