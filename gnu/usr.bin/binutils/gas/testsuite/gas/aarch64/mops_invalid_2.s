	.arch	armv8.8-a+sve

	cpyfpwtn [x0]!, [x1]!, x2!
	cpyfewtn [x0]!, [x1]!, x2!
	cpyfmwtn [x0]!, [x1]!, x2!

	cpyfpwtn [x0]!, [x1]!, x2!
	cpyfmtn [x0]!, [x1]!, x2!
	cpyfetn [x0]!, [x1]!, x2!

	cpyp [x0]!, [x1]!, x2!
	setm [x0]!, x1!, x2
	sete [x0]!, x1!, x2

	cpyfpwt [x0]!, [x1]!, x2!
	cpyfmwt [x3]!, [x1]!, x2!
	cpyfewt [x4]!, [x1]!, x2!

	cpyfpwt [x0]!, [x1]!, x2!
	cpyfmwt [x0]!, [x3]!, x2!
	cpyfewt [x0]!, [x4]!, x2!

	cpyfpwt [x0]!, [x1]!, x2!
	cpyfmwt [x0]!, [x1]!, x3!
	cpyfewt [x0]!, [x1]!, x4!

	cpyfpwtn [x0]!, [x1]!, x2!
	add x0, x1, x2

	cpyfprtn [x0]!, [x1]!, x2!
	cpyfmrtn [x0]!, [x1]!, x2!

	.section .text2, "ax", @progbits

	cpyfpwtn [x0]!, [x1]!, x2!

	.section .text3, "ax", @progbits

	cpyfmwtn [x0]!, [x1]!, x2!

	.section .text4, "ax", @progbits

	cpyfewtn [x0]!, [x1]!, x2!
	cpyfpwn [x0]!, [x1]!, x2!

	.section .text5, "ax", @progbits

	add x0, x1, #0

	setp [x0]!, x1!, x2
	sete [x0]!, x1!, x2
	setm [x0]!, x1!, x2

	setp [x0]!, x1!, x2
	setm [x3]!, x1!, x2
	sete [x4]!, x1!, x2

	setp [x0]!, x1!, x2
	setm [x0]!, x3!, x2
	sete [x0]!, x4!, x2

	setp [x0]!, x1!, x2
	setm [x0]!, x1!, x4 // OK
	sete [x0]!, x1!, x3 // OK

	movprfx z0, z1
	setm [x0]!, x1!, x2

	setp [x0]!, x1!, x2
	movprfx z0, z1
	fadd z0.s, p0/m, z0.s, z4.s

	setp [x0]!, x1!, x2
	movprfx z0, z1
	fadd z2.s, p0/m, z2.s, z4.s
