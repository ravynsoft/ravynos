#name: ARMv8-A RAS
#as: -march=armv8-a+ras
#source: armv8_2-a.s
#objdump: -dr
#skip: *-*-pe *-wince-*


.*: +file format .*arm.*

Disassembly of section .text:

[0-9a-f]+ <.*>:
   [0-9a-f]+:	e320f010 	esb

[0-9a-f]+ <.*>:
   [0-9a-f]+:	f3af 8010 	esb

[0-9a-f]+ <.*>:
   [0-9a-f]+:	ee100f11 	mrc	15, 0, r0, cr0, cr1, \{0\}
   [0-9a-f]+:	ee100fd2 	mrc	15, 0, r0, cr0, cr2, \{6\}
  [0-9a-f]+:	ee150f13 	mrc	15, 0, r0, cr5, cr3, \{0\}
  [0-9a-f]+:	ee150f33 	mrc	15, 0, r0, cr5, cr3, \{1\}
  [0-9a-f]+:	ee051f33 	mcr	15, 0, r1, cr5, cr3, \{1\}
  [0-9a-f]+:	ee150f14 	mrc	15, 0, r0, cr5, cr4, \{0\}
  [0-9a-f]+:	ee150f34 	mrc	15, 0, r0, cr5, cr4, \{1\}
  [0-9a-f]+:	ee051f34 	mcr	15, 0, r1, cr5, cr4, \{1\}
  [0-9a-f]+:	ee150f54 	mrc	15, 0, r0, cr5, cr4, \{2\}
  [0-9a-f]+:	ee051f54 	mcr	15, 0, r1, cr5, cr4, \{2\}
  [0-9a-f]+:	ee150f74 	mrc	15, 0, r0, cr5, cr4, \{3\}
  [0-9a-f]+:	ee051f74 	mcr	15, 0, r1, cr5, cr4, \{3\}
  [0-9a-f]+:	ee150f94 	mrc	15, 0, r0, cr5, cr4, \{4\}
  [0-9a-f]+:	ee150fb4 	mrc	15, 0, r0, cr5, cr4, \{5\}
  [0-9a-f]+:	ee051fb4 	mcr	15, 0, r1, cr5, cr4, \{5\}
  [0-9a-f]+:	ee150ff4 	mrc	15, 0, r0, cr5, cr4, \{7\}
  [0-9a-f]+:	ee051ff4 	mcr	15, 0, r1, cr5, cr4, \{7\}
  [0-9a-f]+:	ee150f15 	mrc	15, 0, r0, cr5, cr5, \{0\}
  [0-9a-f]+:	ee051f15 	mcr	15, 0, r1, cr5, cr5, \{0\}
  [0-9a-f]+:	ee150f35 	mrc	15, 0, r0, cr5, cr5, \{1\}
  [0-9a-f]+:	ee051f35 	mcr	15, 0, r1, cr5, cr5, \{1\}
  [0-9a-f]+:	ee150f95 	mrc	15, 0, r0, cr5, cr5, \{4\}
  [0-9a-f]+:	ee051f95 	mcr	15, 0, r1, cr5, cr5, \{4\}
  [0-9a-f]+:	ee150fb5 	mrc	15, 0, r0, cr5, cr5, \{5\}
  [0-9a-f]+:	ee051fb5 	mcr	15, 0, r1, cr5, cr5, \{5\}
  [0-9a-f]+:	ee1c0f31 	mrc	15, 0, r0, cr12, cr1, \{1\}
  [0-9a-f]+:	ee0c1f31 	mcr	15, 0, r1, cr12, cr1, \{1\}
  [0-9a-f]+:	ee910f91 	mrc	15, 4, r0, cr1, cr1, \{4\}
  [0-9a-f]+:	ee811f91 	mcr	15, 4, r1, cr1, cr1, \{4\}
  [0-9a-f]+:	ee950f72 	mrc	15, 4, r0, cr5, cr2, \{3\}
  [0-9a-f]+:	ee851f72 	mcr	15, 4, r1, cr5, cr2, \{3\}
  [0-9a-f]+:	ee910f31 	mrc	15, 4, r0, cr1, cr1, \{1\}
  [0-9a-f]+:	ee811f31 	mcr	15, 4, r1, cr1, cr1, \{1\}
  [0-9a-f]+:	ee9c0f31 	mrc	15, 4, r0, cr12, cr1, \{1\}
  [0-9a-f]+:	ee8c1f31 	mcr	15, 4, r1, cr12, cr1, \{1\}
  [0-9a-f]+:	eed10f11 	mrc	15, 6, r0, cr1, cr1, \{0\}
  [0-9a-f]+:	eec11f11 	mcr	15, 6, r1, cr1, cr1, \{0\}
