#as: -march=armv8-a+sve
#objdump: -dr

[^:]+:     file format .*


[^:]+:

[^:]+:
[^:]+:	2518e400 	pfalse	p0\.b
[^:]+:	2518e400 	pfalse	p0\.b
[^:]+:	2518e405 	pfalse	p5\.b
[^:]+:	2518e40f 	pfalse	p15\.b
[^:]+:	25804000 	mov	p0\.b, p0\.b
[^:]+:	258f7de0 	mov	p0\.b, p15\.b
[^:]+:	2580400f 	mov	p15\.b, p0\.b
[^:]+:	258c7183 	mov	p3\.b, p12\.b
[^:]+:	85800000 	ldr	p0, \[x0\]
[^:]+:	8580000f 	ldr	p15, \[x0\]
[^:]+:	858003cf 	ldr	p15, \[x30\]
[^:]+:	858003e0 	ldr	p0, \[sp\]
[^:]+:	85800000 	ldr	p0, \[x0\]
[^:]+:	85a00000 	ldr	p0, \[x0, #-256, mul vl\]
[^:]+:	859f1c00 	ldr	p0, \[x0, #255, mul vl\]
[^:]+:	859a0dcb 	ldr	p11, \[x14, #211, mul vl\]
[^:]+:	e5800000 	str	p0, \[x0\]
[^:]+:	e580000f 	str	p15, \[x0\]
[^:]+:	e58003cf 	str	p15, \[x30\]
[^:]+:	e58003e0 	str	p0, \[sp\]
[^:]+:	e5800000 	str	p0, \[x0\]
[^:]+:	e5a00000 	str	p0, \[x0, #-256, mul vl\]
[^:]+:	e59f1c00 	str	p0, \[x0, #255, mul vl\]
[^:]+:	e5b90385 	str	p5, \[x28, #-56, mul vl\]
