# Check 64bit AVX512{VNNI,VL} instructions

	.allow_index_reg
	.text
_start:
	vpdpwssd	%xmm20, %xmm22, %xmm26	 # AVX512{VNNI,VL}
	vpdpwssd	%xmm20, %xmm22, %xmm26{%k3}	 # AVX512{VNNI,VL}
	vpdpwssd	%xmm20, %xmm22, %xmm26{%k3}{z}	 # AVX512{VNNI,VL}
	vpdpwssd	0x123(%rax,%r14,8), %xmm22, %xmm26	 # AVX512{VNNI,VL}
	vpdpwssd	2032(%rdx), %xmm22, %xmm26	 # AVX512{VNNI,VL} Disp8
	vpdpwssd	508(%rdx){1to4}, %xmm22, %xmm26	 # AVX512{VNNI,VL} Disp8
	vpdpwssd	%ymm18, %ymm20, %ymm20	 # AVX512{VNNI,VL}
	vpdpwssd	%ymm18, %ymm20, %ymm20{%k5}	 # AVX512{VNNI,VL}
	vpdpwssd	%ymm18, %ymm20, %ymm20{%k5}{z}	 # AVX512{VNNI,VL}
	vpdpwssd	0x123(%rax,%r14,8), %ymm20, %ymm20	 # AVX512{VNNI,VL}
	vpdpwssd	4064(%rdx), %ymm20, %ymm20	 # AVX512{VNNI,VL} Disp8
	vpdpwssd	508(%rdx){1to8}, %ymm20, %ymm20	 # AVX512{VNNI,VL} Disp8

	vpdpwssds	%xmm23, %xmm19, %xmm22	 # AVX512{VNNI,VL}
	vpdpwssds	%xmm23, %xmm19, %xmm22{%k7}	 # AVX512{VNNI,VL}
	vpdpwssds	%xmm23, %xmm19, %xmm22{%k7}{z}	 # AVX512{VNNI,VL}
	vpdpwssds	0x123(%rax,%r14,8), %xmm19, %xmm22	 # AVX512{VNNI,VL}
	vpdpwssds	2032(%rdx), %xmm19, %xmm22	 # AVX512{VNNI,VL} Disp8
	vpdpwssds	508(%rdx){1to4}, %xmm19, %xmm22	 # AVX512{VNNI,VL} Disp8
	vpdpwssds	%ymm28, %ymm23, %ymm23	 # AVX512{VNNI,VL}
	vpdpwssds	%ymm28, %ymm23, %ymm23{%k3}	 # AVX512{VNNI,VL}
	vpdpwssds	%ymm28, %ymm23, %ymm23{%k3}{z}	 # AVX512{VNNI,VL}
	vpdpwssds	0x123(%rax,%r14,8), %ymm23, %ymm23	 # AVX512{VNNI,VL}
	vpdpwssds	4064(%rdx), %ymm23, %ymm23	 # AVX512{VNNI,VL} Disp8
	vpdpwssds	508(%rdx){1to8}, %ymm23, %ymm23	 # AVX512{VNNI,VL} Disp8

	vpdpbusd	%xmm28, %xmm29, %xmm18	 # AVX512{VNNI,VL}
	vpdpbusd	%xmm28, %xmm29, %xmm18{%k3}	 # AVX512{VNNI,VL}
	vpdpbusd	%xmm28, %xmm29, %xmm18{%k3}{z}	 # AVX512{VNNI,VL}
	vpdpbusd	0x123(%rax,%r14,8), %xmm29, %xmm18	 # AVX512{VNNI,VL}
	vpdpbusd	2032(%rdx), %xmm29, %xmm18	 # AVX512{VNNI,VL} Disp8
	vpdpbusd	508(%rdx){1to4}, %xmm29, %xmm18	 # AVX512{VNNI,VL} Disp8
	vpdpbusd	%ymm17, %ymm18, %ymm20	 # AVX512{VNNI,VL}
	vpdpbusd	%ymm17, %ymm18, %ymm20{%k2}	 # AVX512{VNNI,VL}
	vpdpbusd	%ymm17, %ymm18, %ymm20{%k2}{z}	 # AVX512{VNNI,VL}
	vpdpbusd	0x123(%rax,%r14,8), %ymm18, %ymm20	 # AVX512{VNNI,VL}
	vpdpbusd	4064(%rdx), %ymm18, %ymm20	 # AVX512{VNNI,VL} Disp8
	vpdpbusd	508(%rdx){1to8}, %ymm18, %ymm20	 # AVX512{VNNI,VL} Disp8

	vpdpbusds	%xmm27, %xmm26, %xmm24	 # AVX512{VNNI,VL}
	vpdpbusds	%xmm27, %xmm26, %xmm24{%k4}	 # AVX512{VNNI,VL}
	vpdpbusds	%xmm27, %xmm26, %xmm24{%k4}{z}	 # AVX512{VNNI,VL}
	vpdpbusds	0x123(%rax,%r14,8), %xmm26, %xmm24	 # AVX512{VNNI,VL}
	vpdpbusds	2032(%rdx), %xmm26, %xmm24	 # AVX512{VNNI,VL} Disp8
	vpdpbusds	508(%rdx){1to4}, %xmm26, %xmm24	 # AVX512{VNNI,VL} Disp8
	vpdpbusds	%ymm25, %ymm29, %ymm30	 # AVX512{VNNI,VL}
	vpdpbusds	%ymm25, %ymm29, %ymm30{%k1}	 # AVX512{VNNI,VL}
	vpdpbusds	%ymm25, %ymm29, %ymm30{%k1}{z}	 # AVX512{VNNI,VL}
	vpdpbusds	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{VNNI,VL}
	vpdpbusds	4064(%rdx), %ymm29, %ymm30	 # AVX512{VNNI,VL} Disp8
	vpdpbusds	508(%rdx){1to8}, %ymm29, %ymm30	 # AVX512{VNNI,VL} Disp8

	.intel_syntax noprefix
	vpdpwssd	xmm21, xmm20, xmm23	 # AVX512{VNNI,VL}
	vpdpwssd	xmm21{k6}, xmm20, xmm23	 # AVX512{VNNI,VL}
	vpdpwssd	xmm21{k6}{z}, xmm20, xmm23	 # AVX512{VNNI,VL}
	vpdpwssd	xmm21, xmm20, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VNNI,VL}
	vpdpwssd	xmm21, xmm20, XMMWORD PTR [rdx+2032]	 # AVX512{VNNI,VL} Disp8
	vpdpwssd	xmm21, xmm20, [rdx+508]{1to4}	 # AVX512{VNNI,VL} Disp8
	vpdpwssd	ymm25, ymm27, ymm17	 # AVX512{VNNI,VL}
	vpdpwssd	ymm25{k6}, ymm27, ymm17	 # AVX512{VNNI,VL}
	vpdpwssd	ymm25{k6}{z}, ymm27, ymm17	 # AVX512{VNNI,VL}
	vpdpwssd	ymm25, ymm27, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VNNI,VL}
	vpdpwssd	ymm25, ymm27, YMMWORD PTR [rdx+4064]	 # AVX512{VNNI,VL} Disp8
	vpdpwssd	ymm25, ymm27, [rdx+508]{1to8}	 # AVX512{VNNI,VL} Disp8

	vpdpwssds	xmm30, xmm25, xmm21	 # AVX512{VNNI,VL}
	vpdpwssds	xmm30{k6}, xmm25, xmm21	 # AVX512{VNNI,VL}
	vpdpwssds	xmm30{k6}{z}, xmm25, xmm21	 # AVX512{VNNI,VL}
	vpdpwssds	xmm30, xmm25, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VNNI,VL}
	vpdpwssds	xmm30, xmm25, XMMWORD PTR [rdx+2032]	 # AVX512{VNNI,VL} Disp8
	vpdpwssds	xmm30, xmm25, [rdx+508]{1to4}	 # AVX512{VNNI,VL} Disp8
	vpdpwssds	ymm28, ymm27, ymm27	 # AVX512{VNNI,VL}
	vpdpwssds	ymm28{k7}, ymm27, ymm27	 # AVX512{VNNI,VL}
	vpdpwssds	ymm28{k7}{z}, ymm27, ymm27	 # AVX512{VNNI,VL}
	vpdpwssds	ymm28, ymm27, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VNNI,VL}
	vpdpwssds	ymm28, ymm27, YMMWORD PTR [rdx+4064]	 # AVX512{VNNI,VL} Disp8
	vpdpwssds	ymm28, ymm27, [rdx+508]{1to8}	 # AVX512{VNNI,VL} Disp8

	vpdpbusd	xmm26, xmm18, xmm19	 # AVX512{VNNI,VL}
	vpdpbusd	xmm26{k6}, xmm18, xmm19	 # AVX512{VNNI,VL}
	vpdpbusd	xmm26{k6}{z}, xmm18, xmm19	 # AVX512{VNNI,VL}
	vpdpbusd	xmm26, xmm18, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VNNI,VL}
	vpdpbusd	xmm26, xmm18, XMMWORD PTR [rdx+2032]	 # AVX512{VNNI,VL} Disp8
	vpdpbusd	xmm26, xmm18, [rdx+508]{1to4}	 # AVX512{VNNI,VL} Disp8
	vpdpbusd	ymm21, ymm17, ymm27	 # AVX512{VNNI,VL}
	vpdpbusd	ymm21{k2}, ymm17, ymm27	 # AVX512{VNNI,VL}
	vpdpbusd	ymm21{k2}{z}, ymm17, ymm27	 # AVX512{VNNI,VL}
	vpdpbusd	ymm21, ymm17, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VNNI,VL}
	vpdpbusd	ymm21, ymm17, YMMWORD PTR [rdx+4064]	 # AVX512{VNNI,VL} Disp8
	vpdpbusd	ymm21, ymm17, [rdx+508]{1to8}	 # AVX512{VNNI,VL} Disp8

	vpdpbusds	xmm28, xmm26, xmm24	 # AVX512{VNNI,VL}
	vpdpbusds	xmm28{k1}, xmm26, xmm24	 # AVX512{VNNI,VL}
	vpdpbusds	xmm28{k1}{z}, xmm26, xmm24	 # AVX512{VNNI,VL}
	vpdpbusds	xmm28, xmm26, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VNNI,VL}
	vpdpbusds	xmm28, xmm26, XMMWORD PTR [rdx+2032]	 # AVX512{VNNI,VL} Disp8
	vpdpbusds	xmm28, xmm26, [rdx+508]{1to4}	 # AVX512{VNNI,VL} Disp8
	vpdpbusds	ymm23, ymm18, ymm27	 # AVX512{VNNI,VL}
	vpdpbusds	ymm23{k6}, ymm18, ymm27	 # AVX512{VNNI,VL}
	vpdpbusds	ymm23{k6}{z}, ymm18, ymm27	 # AVX512{VNNI,VL}
	vpdpbusds	ymm23, ymm18, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{VNNI,VL}
	vpdpbusds	ymm23, ymm18, YMMWORD PTR [rdx+4064]	 # AVX512{VNNI,VL} Disp8
	vpdpbusds	ymm23, ymm18, [rdx+508]{1to8}	 # AVX512{VNNI,VL} Disp8
