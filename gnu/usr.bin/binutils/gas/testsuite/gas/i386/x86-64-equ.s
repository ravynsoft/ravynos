 .text
_start:

 .set	ACC, %xmm17
	vaddss   %xmm0,%xmm1,ACC

 .intel_syntax noprefix

 .set	ACC, xmm17
	vaddss   xmm0,xmm1,ACC
