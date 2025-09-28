# warning_output: mops_invalid_2.l
# objdump: -dr -M notes
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*


Disassembly of section \.text:

0+ <\.text>:
[^:]*:	1901d440 	cpyfpwtn	\[x0\]!, \[x1\]!, x2!
[^:]*:	1981d440 	cpyfewtn	\[x0\]!, \[x1\]!, x2!  // note: expected `cpyfmwtn' after previous `cpyfpwtn'
[^:]*:	1941d440 	cpyfmwtn	\[x0\]!, \[x1\]!, x2!  // note: this `cpyfmwtn' should have an immediately preceding `cpyfpwtn'
[^:]*:	1901d440 	cpyfpwtn	\[x0\]!, \[x1\]!, x2!
[^:]*:	1941f440 	cpyfmtn	\[x0\]!, \[x1\]!, x2!  // note: expected `cpyfmwtn' after previous `cpyfpwtn'
[^:]*:	1981f440 	cpyfetn	\[x0\]!, \[x1\]!, x2!
[^:]*:	1d010440 	cpyp	\[x0\]!, \[x1\]!, x2!
[^:]*:	19c24420 	setm	\[x0\]!, x1!, x2  // note: expected `cpym' after previous `cpyp'
[^:]*:	19c28420 	sete	\[x0\]!, x1!, x2
[^:]*:	19011440 	cpyfpwt	\[x0\]!, \[x1\]!, x2!
[^:]*:	19411443 	cpyfmwt	\[x3\]!, \[x1\]!, x2!  // note: destination register differs from preceding instruction at operand 1
[^:]*:	19811444 	cpyfewt	\[x4\]!, \[x1\]!, x2!  // note: destination register differs from preceding instruction at operand 1
[^:]*:	19011440 	cpyfpwt	\[x0\]!, \[x1\]!, x2!
[^:]*:	19431440 	cpyfmwt	\[x0\]!, \[x3\]!, x2!  // note: source register differs from preceding instruction at operand 2
[^:]*:	19841440 	cpyfewt	\[x0\]!, \[x4\]!, x2!  // note: source register differs from preceding instruction at operand 2
[^:]*:	19011440 	cpyfpwt	\[x0\]!, \[x1\]!, x2!
[^:]*:	19411460 	cpyfmwt	\[x0\]!, \[x1\]!, x3!  // note: size register differs from preceding instruction at operand 3
[^:]*:	19811480 	cpyfewt	\[x0\]!, \[x1\]!, x4!  // note: size register differs from preceding instruction at operand 3
[^:]*:	1901d440 	cpyfpwtn	\[x0\]!, \[x1\]!, x2!
[^:]*:	8b020020 	add	x0, x1, x2  // note: expected `cpyfmwtn' after previous `cpyfpwtn'
[^:]*:	1901e440 	cpyfprtn	\[x0\]!, \[x1\]!, x2!
[^:]*:	1941e440 	cpyfmrtn	\[x0\]!, \[x1\]!, x2!

Disassembly of section \.text2:

0+ <\.text2>:
[^:]*:	1901d440 	cpyfpwtn	\[x0\]!, \[x1\]!, x2!  // note: instruction opens new dependency sequence without ending previous one

Disassembly of section \.text3:

0+ <\.text3>:
[^:]*:	1941d440 	cpyfmwtn	\[x0\]!, \[x1\]!, x2!  // note: this `cpyfmwtn' should have an immediately preceding `cpyfpwtn'

Disassembly of section \.text4:

0+ <\.text4>:
[^:]*:	1981d440 	cpyfewtn	\[x0\]!, \[x1\]!, x2!  // note: this `cpyfewtn' should have an immediately preceding `cpyfmwtn'
[^:]*:	19014440 	cpyfpwn	\[x0\]!, \[x1\]!, x2!

Disassembly of section \.text5:

0+ <\.text5>:
[^:]*:	91000020 	add	x0, x1, #0x0  // note: expected `cpyfmwn' after previous `cpyfpwn'
[^:]*:	19c20420 	setp	\[x0\]!, x1!, x2
[^:]*:	19c28420 	sete	\[x0\]!, x1!, x2  // note: expected `setm' after previous `setp'
[^:]*:	19c24420 	setm	\[x0\]!, x1!, x2  // note: this `setm' should have an immediately preceding `setp'
[^:]*:	19c20420 	setp	\[x0\]!, x1!, x2
[^:]*:	19c24423 	setm	\[x3\]!, x1!, x2  // note: destination register differs from preceding instruction at operand 1
[^:]*:	19c28424 	sete	\[x4\]!, x1!, x2  // note: destination register differs from preceding instruction at operand 1
[^:]*:	19c20420 	setp	\[x0\]!, x1!, x2
[^:]*:	19c24460 	setm	\[x0\]!, x3!, x2  // note: size register differs from preceding instruction at operand 2
[^:]*:	19c28480 	sete	\[x0\]!, x4!, x2  // note: size register differs from preceding instruction at operand 2
[^:]*:	19c20420 	setp	\[x0\]!, x1!, x2
[^:]*:	19c44420 	setm	\[x0\]!, x1!, x4
[^:]*:	19c38420 	sete	\[x0\]!, x1!, x3
[^:]*:	0420bc20 	movprfx	z0, z1
[^:]*:	19c24420 	setm	\[x0\]!, x1!, x2  // note: SVE instruction expected after `movprfx'
[^:]*:	19c20420 	setp	\[x0\]!, x1!, x2
[^:]*:	0420bc20 	movprfx	z0, z1  // note: instruction opens new dependency sequence without ending previous one
[^:]*:	65808080 	fadd	z0\.s, p0/m, z0\.s, z4\.s
[^:]*:	19c20420 	setp	\[x0\]!, x1!, x2
[^:]*:	0420bc20 	movprfx	z0, z1  // note: instruction opens new dependency sequence without ending previous one
[^:]*:	65808082 	fadd	z2\.s, p0/m, z2\.s, z4\.s  // note: output register of preceding `movprfx' not used in current instruction at operand 1
