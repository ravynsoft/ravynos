# Check SHA instructions

	.allow_index_reg
	.text
_start:
	sha1rnds4 $9, %xmm2, %xmm1
	sha1rnds4 $7, (%rax), %xmm2
	sha1rnds4 $5, 0x12(%rax), %xmm3
	sha1rnds4 $1, (%rax,%rbx,2), %xmm4
	sha1nexte %xmm2, %xmm7
	sha1nexte (%rax), %xmm8
	sha1nexte 0x12(%rax), %xmm9
	sha1nexte (%rax,%rbx,2), %xmm10
	sha1msg1 %xmm2, %xmm7
	sha1msg1 (%rax), %xmm8
	sha1msg1 0x12(%rax), %xmm9
	sha1msg1 (%rax,%rbx,2), %xmm10
	sha1msg2 %xmm2, %xmm7
	sha1msg2 (%rax), %xmm8
	sha1msg2 0x12(%rax), %xmm9
	sha1msg2 (%rax,%rbx,2), %xmm10
	sha256rnds2 %xmm2, %xmm1
	sha256rnds2 (%rax), %xmm1
	sha256rnds2 0x12(%rax), %xmm1
	sha256rnds2 (%rax,%rbx,2), %xmm1
	sha256rnds2 %xmm0, %xmm2, %xmm1
	sha256rnds2 %xmm0, (%rax), %xmm1
	sha256rnds2 %xmm0, 0x12(%rax), %xmm1
	sha256rnds2 %xmm0, (%rax,%rbx,2), %xmm1
	sha256msg1 %xmm2, %xmm1
	sha256msg1 (%rax), %xmm1
	sha256msg1 0x12(%rax), %xmm1
	sha256msg1 (%rax,%rbx,2), %xmm1
	sha256msg2 %xmm2, %xmm1
	sha256msg2 (%rax), %xmm1
	sha256msg2 0x12(%rax), %xmm1
	sha256msg2 (%rax,%rbx,2), %xmm1

	.intel_syntax noprefix

	sha1rnds4 xmm1, xmm2, 9
	sha1rnds4 xmm2, XMMWORD PTR [rax], 7
	sha1rnds4 xmm3, XMMWORD PTR [rax+0x12], 5
	sha1rnds4 xmm4, XMMWORD PTR [rax+rbx*2], 1
	sha1nexte xmm1, xmm2
	sha1nexte xmm2, XMMWORD PTR [rax]
	sha1nexte xmm3, XMMWORD PTR [rax+0x12]
	sha1nexte xmm4, XMMWORD PTR [rax+rbx*2]
	sha1msg1 xmm1, xmm2
	sha1msg1 xmm2, XMMWORD PTR [rax]
	sha1msg1 xmm3, XMMWORD PTR [rax+0x12]
	sha1msg1 xmm4, XMMWORD PTR [rax+rbx*2]
	sha1msg2 xmm1, xmm2
	sha1msg2 xmm2, XMMWORD PTR [rax]
	sha1msg2 xmm3, XMMWORD PTR [rax+0x12]
	sha1msg2 xmm4, XMMWORD PTR [rax+rbx*2]
	sha256rnds2 xmm1, xmm2
	sha256rnds2 xmm2, XMMWORD PTR [rax]
	sha256rnds2 xmm3, XMMWORD PTR [rax+0x12]
	sha256rnds2 xmm4, XMMWORD PTR [rax+rbx*2]
	sha256rnds2 xmm1, xmm2, xmm0
	sha256rnds2 xmm2, XMMWORD PTR [rax], xmm0
	sha256rnds2 xmm3, XMMWORD PTR [rax+0x12], xmm0
	sha256rnds2 xmm4, XMMWORD PTR [rax+rbx*2], xmm0
	sha256msg1 xmm1, xmm2
	sha256msg1 xmm2, XMMWORD PTR [rax]
	sha256msg1 xmm3, XMMWORD PTR [rax+0x12]
	sha256msg1 xmm4, XMMWORD PTR [rax+rbx*2]
	sha256msg2 xmm1, xmm2
	sha256msg2 xmm2, XMMWORD PTR [rax]
	sha256msg2 xmm3, XMMWORD PTR [rax+0x12]
	sha256msg2 xmm4, XMMWORD PTR [rax+rbx*2]

