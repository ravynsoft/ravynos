#objdump: -dr
# as: -mccs -mcpu=cortex-r4 -mthumb

.*:     file format .*arm.*


Disassembly of section \.text:

00000000 <_test_func>:
   0:	e92d5fff 	push	{r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}
   4:	e59fc018 	ldr	ip, \[pc, #24\]	@ 24 <sym1>
   8:	e59c0000 	ldr	r0, \[ip\]
   c:	e3100008 	tst	r0, #8
  10:	1a000000 	bne	18 <aLabel>
  14:	e59c0000 	ldr	r0, \[ip\]

00000018 <aLabel>:
  18:	eb...... 	bl	. <ext_sy.*>
			18: .*	ext_sy.*
  1c:	e8bd5fff 	pop	{r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, sl, fp, ip, lr}
  20:	e25ef008 	subs	pc, lr, #8

00000024 <sym1>:
  24:	fffff520 	.*
#...
