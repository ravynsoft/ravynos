# Check SHA instructions

	.allow_index_reg
	.text
_start:
	sha1rnds4 $9, %xmm2, %xmm1
	sha1rnds4 $7, (%eax), %xmm2
	sha1rnds4 $5, 0x12(%eax), %xmm3
	sha1rnds4 $1, (%eax,%ebx,2), %xmm4
	sha1nexte %xmm2, %xmm1
	sha1nexte (%eax), %xmm1
	sha1nexte 0x12(%eax), %xmm1
	sha1nexte (%eax,%ebx,2), %xmm1
	sha1msg1 %xmm2, %xmm1
	sha1msg1 (%eax), %xmm1
	sha1msg1 0x12(%eax), %xmm1
	sha1msg1 (%eax,%ebx,2), %xmm1
	sha1msg2 %xmm2, %xmm1
	sha1msg2 (%eax), %xmm1
	sha1msg2 0x12(%eax), %xmm1
	sha1msg2 (%eax,%ebx,2), %xmm1
	sha256rnds2 %xmm2, %xmm1
	sha256rnds2 (%eax), %xmm1
	sha256rnds2 0x12(%eax), %xmm1
	sha256rnds2 (%eax,%ebx,2), %xmm1
	sha256rnds2 %xmm0, %xmm2, %xmm1
	sha256rnds2 %xmm0, (%eax), %xmm1
	sha256rnds2 %xmm0, 0x12(%eax), %xmm1
	sha256rnds2 %xmm0, (%eax,%ebx,2), %xmm1
	sha256msg1 %xmm2, %xmm1
	sha256msg1 (%eax), %xmm1
	sha256msg1 0x12(%eax), %xmm1
	sha256msg1 (%eax,%ebx,2), %xmm1
	sha256msg2 %xmm2, %xmm1
	sha256msg2 (%eax), %xmm1
	sha256msg2 0x12(%eax), %xmm1
	sha256msg2 (%eax,%ebx,2), %xmm1

	.intel_syntax noprefix

	sha1rnds4 xmm1, xmm2, 9
	sha1rnds4 xmm2, XMMWORD PTR [eax], 7
	sha1rnds4 xmm3, XMMWORD PTR [eax+0x12], 5
	sha1rnds4 xmm4, XMMWORD PTR [eax+ebx*2], 1
	sha1nexte xmm1, xmm2
	sha1nexte xmm2, XMMWORD PTR [eax]
	sha1nexte xmm3, XMMWORD PTR [eax+0x12]
	sha1nexte xmm4, XMMWORD PTR [eax+ebx*2]
	sha1msg1 xmm1, xmm2
	sha1msg1 xmm2, XMMWORD PTR [eax]
	sha1msg1 xmm3, XMMWORD PTR [eax+0x12]
	sha1msg1 xmm4, XMMWORD PTR [eax+ebx*2]
	sha1msg2 xmm1, xmm2
	sha1msg2 xmm2, XMMWORD PTR [eax]
	sha1msg2 xmm3, XMMWORD PTR [eax+0x12]
	sha1msg2 xmm4, XMMWORD PTR [eax+ebx*2]
	sha256rnds2 xmm1, xmm2
	sha256rnds2 xmm2, XMMWORD PTR [eax]
	sha256rnds2 xmm3, XMMWORD PTR [eax+0x12]
	sha256rnds2 xmm4, XMMWORD PTR [eax+ebx*2]
	sha256rnds2 xmm1, xmm2, xmm0
	sha256rnds2 xmm2, XMMWORD PTR [eax], xmm0
	sha256rnds2 xmm3, XMMWORD PTR [eax+0x12], xmm0
	sha256rnds2 xmm4, XMMWORD PTR [eax+ebx*2], xmm0
	sha256msg1 xmm1, xmm2
	sha256msg1 xmm2, XMMWORD PTR [eax]
	sha256msg1 xmm3, XMMWORD PTR [eax+0x12]
	sha256msg1 xmm4, XMMWORD PTR [eax+ebx*2]
	sha256msg2 xmm1, xmm2
	sha256msg2 xmm2, XMMWORD PTR [eax]
	sha256msg2 xmm3, XMMWORD PTR [eax+0x12]
	sha256msg2 xmm4, XMMWORD PTR [eax+ebx*2]
