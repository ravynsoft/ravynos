#name: s390x opcode
#objdump: -dr

.*: +file format .*

Disassembly of section .text:

.* <foo>:
.*:	e6 f1 40 c0 d6 74 [	 ]*vschp	%v15,%v17,%v20,13,12
.*:	e6 f1 40 d0 26 74 [	 ]*vschsp	%v15,%v17,%v20,13
.*:	e6 f1 40 d0 36 74 [	 ]*vschdp	%v15,%v17,%v20,13
.*:	e6 f1 40 d0 46 74 [	 ]*vschxp	%v15,%v17,%v20,13
.*:	e6 f1 40 00 06 7c [	 ]*vscshp	%v15,%v17,%v20
.*:	e6 f1 40 d0 06 7d [	 ]*vcsph	%v15,%v17,%v20,13
.*:	e6 f1 00 d0 04 51 [	 ]*vclzdp	%v15,%v17,13
.*:	e6 f1 40 cf d6 70 [	 ]*vpkzr	%v15,%v17,%v20,253,12
.*:	e6 f1 40 cf d6 72 [	 ]*vsrpr	%v15,%v17,%v20,253,12
.*:	e6 f1 00 d0 04 54 [	 ]*vupkzh	%v15,%v17,13
.*:	e6 f1 00 d0 04 5c [	 ]*vupkzl	%v15,%v17,13
.*:	b9 3b 00 00 [	 ]*nnpa
.*:	e6 f1 00 0c d4 56 [	 ]*vclfnh	%v15,%v17,13,12
.*:	e6 f1 00 0c d4 5e [	 ]*vclfnl	%v15,%v17,13,12
.*:	e6 f1 40 0c d6 75 [	 ]*vcrnf	%v15,%v17,%v20,13,12
.*:	e6 f1 00 0c d4 5d [	 ]*vcfn	%v15,%v17,13,12
.*:	e6 f1 00 0c d4 55 [	 ]*vcnf	%v15,%v17,13,12
.*:	b9 8b 90 6b [	 ]*rdp	%r6,%r9,%r11
.*:	b9 8b 9d 6b [	 ]*rdp	%r6,%r9,%r11,13
.*:	eb 00 68 f0 fd 71 [	 ]*lpswey	-10000\(%r6\)
.*:	b2 00 6f a0 [	 ]*lbear	4000\(%r6\)
.*:	b2 01 6f a0 [	 ]*stbear	4000\(%r6\)
.*:	b2 8f 5f ff [	 ]*qpaci	4095\(%r5\)
.*:	07 07 [	 ]*nopr	%r7
