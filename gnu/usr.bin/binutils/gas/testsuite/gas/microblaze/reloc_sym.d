
.*: +file format .*

Disassembly of section .text:

10000054 <__def_start>:
10000054:	3021fff8 	addik	r1, r1, -8
10000058:	fa610004 	swi	r19, r1, 4
1000005c:	12610000 	addk	r19, r1, r0
10000060:	10330000 	addk	r1, r19, r0
10000064:	ea610004 	lwi	r19, r1, 4
10000068:	30210008 	addik	r1, r1, 8
1000006c:	b60f0008 	rtsd	r15, 8
10000070:	80000000 	or	r0, r0, r0

10000074 <main>:
10000074:	3021ffe0 	addik	r1, r1, -32
10000078:	f9e10000 	swi	r15, r1, 0
1000007c:	fa61001c 	swi	r19, r1, 28
10000080:	12610000 	addk	r19, r1, r0
10000084:	b000efff 	imm	-4097
10000088:	b9f4ff7c 	brlid	r15, -132	// 4 <test_start>
1000008c:	80000000 	or	r0, r0, r0
10000090:	b9f4ffc4 	brlid	r15, -60	// 10000054 <__def_start>
10000094:	80000000 	or	r0, r0, r0
10000098:	10600000 	addk	r3, r0, r0
1000009c:	e9e10000 	lwi	r15, r1, 0
100000a0:	10330000 	addk	r1, r19, r0
100000a4:	ea61001c 	lwi	r19, r1, 28
100000a8:	30210020 	addik	r1, r1, 32
100000ac:	b60f0008 	rtsd	r15, 8
100000b0:	80000000 	or	r0, r0, r0

Disassembly of section .testsection:

00000004 <test_start>:
   4:	3021fff8 	addik	r1, r1, -8
   8:	fa610004 	swi	r19, r1, 4
   c:	12610000 	addk	r19, r1, r0
  10:	10330000 	addk	r1, r19, r0
  14:	ea610004 	lwi	r19, r1, 4
  18:	30210008 	addik	r1, r1, 8
  1c:	b60f0008 	rtsd	r15, 8
  20:	80000000 	or	r0, r0, r0
