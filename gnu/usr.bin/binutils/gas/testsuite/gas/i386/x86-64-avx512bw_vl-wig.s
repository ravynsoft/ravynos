# Check 64bit AVX512{BW,VL} WIG instructions

	.allow_index_reg
	.text
_start:
	vpabsb	%xmm29, %xmm30	 # AVX512{BW,VL}
	vpabsb	%xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpabsb	%xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpabsb	(%rcx), %xmm30	 # AVX512{BW,VL}
	vpabsb	0x123(%rax,%r14,8), %xmm30	 # AVX512{BW,VL}
	vpabsb	2032(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpabsb	2048(%rdx), %xmm30	 # AVX512{BW,VL}
	vpabsb	-2048(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpabsb	-2064(%rdx), %xmm30	 # AVX512{BW,VL}
	vpabsb	%ymm29, %ymm30	 # AVX512{BW,VL}
	vpabsb	%ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpabsb	%ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpabsb	(%rcx), %ymm30	 # AVX512{BW,VL}
	vpabsb	0x123(%rax,%r14,8), %ymm30	 # AVX512{BW,VL}
	vpabsb	4064(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpabsb	4096(%rdx), %ymm30	 # AVX512{BW,VL}
	vpabsb	-4096(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpabsb	-4128(%rdx), %ymm30	 # AVX512{BW,VL}
	vpabsw	%xmm29, %xmm30	 # AVX512{BW,VL}
	vpabsw	%xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpabsw	%xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpabsw	(%rcx), %xmm30	 # AVX512{BW,VL}
	vpabsw	0x123(%rax,%r14,8), %xmm30	 # AVX512{BW,VL}
	vpabsw	2032(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpabsw	2048(%rdx), %xmm30	 # AVX512{BW,VL}
	vpabsw	-2048(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpabsw	-2064(%rdx), %xmm30	 # AVX512{BW,VL}
	vpabsw	%ymm29, %ymm30	 # AVX512{BW,VL}
	vpabsw	%ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpabsw	%ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpabsw	(%rcx), %ymm30	 # AVX512{BW,VL}
	vpabsw	0x123(%rax,%r14,8), %ymm30	 # AVX512{BW,VL}
	vpabsw	4064(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpabsw	4096(%rdx), %ymm30	 # AVX512{BW,VL}
	vpabsw	-4096(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpabsw	-4128(%rdx), %ymm30	 # AVX512{BW,VL}
	vpacksswb	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpacksswb	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpacksswb	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpacksswb	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpacksswb	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpacksswb	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpacksswb	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpacksswb	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpacksswb	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpacksswb	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpacksswb	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpacksswb	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpacksswb	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpacksswb	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpacksswb	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpacksswb	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpacksswb	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpacksswb	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpackuswb	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpackuswb	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpackuswb	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpackuswb	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpackuswb	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpackuswb	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpackuswb	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpackuswb	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpackuswb	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpackuswb	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpackuswb	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpackuswb	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpackuswb	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpackuswb	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpackuswb	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpackuswb	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpackuswb	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpackuswb	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddb	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddb	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpaddb	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpaddb	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddb	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddb	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpaddb	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddb	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpaddb	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddb	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddb	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpaddb	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpaddb	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddb	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddb	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpaddb	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddb	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpaddb	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddsb	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddsb	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpaddsb	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpaddsb	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddsb	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddsb	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpaddsb	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddsb	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpaddsb	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddsb	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddsb	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpaddsb	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpaddsb	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddsb	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddsb	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpaddsb	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddsb	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpaddsb	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddsw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddsw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpaddsw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpaddsw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddsw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddsw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpaddsw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddsw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpaddsw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddsw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddsw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpaddsw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpaddsw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddsw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddsw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpaddsw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddsw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpaddsw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddusb	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddusb	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpaddusb	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpaddusb	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddusb	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddusb	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpaddusb	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddusb	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpaddusb	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddusb	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddusb	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpaddusb	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpaddusb	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddusb	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddusb	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpaddusb	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddusb	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpaddusb	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddusw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddusw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpaddusw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpaddusw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddusw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddusw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpaddusw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddusw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpaddusw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddusw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddusw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpaddusw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpaddusw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddusw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddusw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpaddusw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddusw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpaddusw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpaddw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpaddw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpaddw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpaddw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpaddw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpaddw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpaddw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpaddw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpaddw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpaddw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpalignr	$0xab, %xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpalignr	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpalignr	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpalignr	$123, %xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpalignr	$123, (%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpalignr	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpalignr	$123, 2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpalignr	$123, 2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpalignr	$123, -2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpalignr	$123, -2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpalignr	$0xab, %ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpalignr	$0xab, %ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpalignr	$0xab, %ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpalignr	$123, %ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpalignr	$123, (%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpalignr	$123, 0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpalignr	$123, 4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpalignr	$123, 4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpalignr	$123, -4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpalignr	$123, -4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpavgb	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpavgb	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpavgb	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpavgb	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpavgb	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpavgb	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpavgb	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpavgb	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpavgb	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpavgb	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpavgb	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpavgb	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpavgb	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpavgb	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpavgb	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpavgb	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpavgb	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpavgb	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpavgw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpavgw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpavgw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpavgw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpavgw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpavgw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpavgw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpavgw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpavgw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpavgw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpavgw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpavgw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpavgw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpavgw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpavgw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpavgw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpavgw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpavgw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpcmpeqb	%xmm29, %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpeqb	%xmm29, %xmm30, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqb	(%rcx), %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpeqb	0x123(%rax,%r14,8), %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpeqb	2032(%rdx), %xmm30, %k5	 # AVX512{BW,VL} Disp8
	vpcmpeqb	2048(%rdx), %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpeqb	-2048(%rdx), %xmm30, %k5	 # AVX512{BW,VL} Disp8
	vpcmpeqb	-2064(%rdx), %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpeqb	%ymm29, %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpeqb	%ymm29, %ymm30, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqb	(%rcx), %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpeqb	0x123(%rax,%r14,8), %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpeqb	4064(%rdx), %ymm30, %k5	 # AVX512{BW,VL} Disp8
	vpcmpeqb	4096(%rdx), %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpeqb	-4096(%rdx), %ymm30, %k5	 # AVX512{BW,VL} Disp8
	vpcmpeqb	-4128(%rdx), %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpeqw	%xmm29, %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpeqw	%xmm29, %xmm30, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqw	(%rcx), %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpeqw	0x123(%rax,%r14,8), %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpeqw	2032(%rdx), %xmm30, %k5	 # AVX512{BW,VL} Disp8
	vpcmpeqw	2048(%rdx), %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpeqw	-2048(%rdx), %xmm30, %k5	 # AVX512{BW,VL} Disp8
	vpcmpeqw	-2064(%rdx), %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpeqw	%ymm29, %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpeqw	%ymm29, %ymm30, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqw	(%rcx), %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpeqw	0x123(%rax,%r14,8), %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpeqw	4064(%rdx), %ymm30, %k5	 # AVX512{BW,VL} Disp8
	vpcmpeqw	4096(%rdx), %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpeqw	-4096(%rdx), %ymm30, %k5	 # AVX512{BW,VL} Disp8
	vpcmpeqw	-4128(%rdx), %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpgtb	%xmm29, %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpgtb	%xmm29, %xmm30, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtb	(%rcx), %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpgtb	0x123(%rax,%r14,8), %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpgtb	2032(%rdx), %xmm30, %k5	 # AVX512{BW,VL} Disp8
	vpcmpgtb	2048(%rdx), %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpgtb	-2048(%rdx), %xmm30, %k5	 # AVX512{BW,VL} Disp8
	vpcmpgtb	-2064(%rdx), %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpgtb	%ymm29, %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpgtb	%ymm29, %ymm30, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtb	(%rcx), %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpgtb	0x123(%rax,%r14,8), %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpgtb	4064(%rdx), %ymm30, %k5	 # AVX512{BW,VL} Disp8
	vpcmpgtb	4096(%rdx), %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpgtb	-4096(%rdx), %ymm30, %k5	 # AVX512{BW,VL} Disp8
	vpcmpgtb	-4128(%rdx), %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpgtw	%xmm29, %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpgtw	%xmm29, %xmm30, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtw	(%rcx), %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpgtw	0x123(%rax,%r14,8), %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpgtw	2032(%rdx), %xmm30, %k5	 # AVX512{BW,VL} Disp8
	vpcmpgtw	2048(%rdx), %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpgtw	-2048(%rdx), %xmm30, %k5	 # AVX512{BW,VL} Disp8
	vpcmpgtw	-2064(%rdx), %xmm30, %k5	 # AVX512{BW,VL}
	vpcmpgtw	%ymm29, %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpgtw	%ymm29, %ymm30, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtw	(%rcx), %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpgtw	0x123(%rax,%r14,8), %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpgtw	4064(%rdx), %ymm30, %k5	 # AVX512{BW,VL} Disp8
	vpcmpgtw	4096(%rdx), %ymm30, %k5	 # AVX512{BW,VL}
	vpcmpgtw	-4096(%rdx), %ymm30, %k5	 # AVX512{BW,VL} Disp8
	vpcmpgtw	-4128(%rdx), %ymm30, %k5	 # AVX512{BW,VL}
	vpmaddubsw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaddubsw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpmaddubsw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpmaddubsw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaddubsw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaddubsw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmaddubsw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaddubsw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmaddubsw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaddubsw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaddubsw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpmaddubsw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpmaddubsw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaddubsw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaddubsw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmaddubsw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaddubsw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmaddubsw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaddwd	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaddwd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpmaddwd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpmaddwd	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaddwd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaddwd	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmaddwd	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaddwd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmaddwd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaddwd	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaddwd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpmaddwd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpmaddwd	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaddwd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaddwd	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmaddwd	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaddwd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmaddwd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxsb	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxsb	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpmaxsb	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpmaxsb	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxsb	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxsb	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmaxsb	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxsb	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmaxsb	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxsb	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxsb	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpmaxsb	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpmaxsb	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxsb	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxsb	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmaxsb	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxsb	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmaxsb	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxsw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxsw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpmaxsw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpmaxsw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxsw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxsw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmaxsw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxsw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmaxsw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxsw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxsw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpmaxsw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpmaxsw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxsw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxsw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmaxsw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxsw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmaxsw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxub	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxub	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpmaxub	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpmaxub	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxub	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxub	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmaxub	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxub	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmaxub	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxub	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxub	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpmaxub	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpmaxub	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxub	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxub	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmaxub	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxub	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmaxub	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxuw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxuw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpmaxuw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpmaxuw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxuw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxuw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmaxuw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxuw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmaxuw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmaxuw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxuw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpmaxuw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpmaxuw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxuw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxuw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmaxuw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmaxuw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmaxuw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminsb	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminsb	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpminsb	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpminsb	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminsb	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminsb	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpminsb	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminsb	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpminsb	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminsb	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminsb	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpminsb	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpminsb	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminsb	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminsb	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpminsb	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminsb	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpminsb	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminsw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminsw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpminsw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpminsw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminsw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminsw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpminsw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminsw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpminsw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminsw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminsw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpminsw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpminsw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminsw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminsw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpminsw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminsw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpminsw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminub	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminub	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpminub	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpminub	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminub	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminub	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpminub	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminub	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpminub	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminub	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminub	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpminub	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpminub	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminub	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminub	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpminub	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminub	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpminub	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminuw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminuw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpminuw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpminuw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminuw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminuw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpminuw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminuw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpminuw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpminuw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminuw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpminuw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpminuw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminuw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminuw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpminuw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpminuw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpminuw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmovsxbw	%xmm29, %xmm30	 # AVX512{BW,VL}
	vpmovsxbw	%xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpmovsxbw	%xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpmovsxbw	(%rcx), %xmm30	 # AVX512{BW,VL}
	vpmovsxbw	0x123(%rax,%r14,8), %xmm30	 # AVX512{BW,VL}
	vpmovsxbw	1016(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpmovsxbw	1024(%rdx), %xmm30	 # AVX512{BW,VL}
	vpmovsxbw	-1024(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpmovsxbw	-1032(%rdx), %xmm30	 # AVX512{BW,VL}
	vpmovsxbw	%xmm29, %ymm30	 # AVX512{BW,VL}
	vpmovsxbw	%xmm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpmovsxbw	%xmm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpmovsxbw	(%rcx), %ymm30	 # AVX512{BW,VL}
	vpmovsxbw	0x123(%rax,%r14,8), %ymm30	 # AVX512{BW,VL}
	vpmovsxbw	2032(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpmovsxbw	2048(%rdx), %ymm30	 # AVX512{BW,VL}
	vpmovsxbw	-2048(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpmovsxbw	-2064(%rdx), %ymm30	 # AVX512{BW,VL}
	vpmovzxbw	%xmm29, %xmm30	 # AVX512{BW,VL}
	vpmovzxbw	%xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpmovzxbw	%xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpmovzxbw	(%rcx), %xmm30	 # AVX512{BW,VL}
	vpmovzxbw	0x123(%rax,%r14,8), %xmm30	 # AVX512{BW,VL}
	vpmovzxbw	1016(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpmovzxbw	1024(%rdx), %xmm30	 # AVX512{BW,VL}
	vpmovzxbw	-1024(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpmovzxbw	-1032(%rdx), %xmm30	 # AVX512{BW,VL}
	vpmovzxbw	%xmm29, %ymm30	 # AVX512{BW,VL}
	vpmovzxbw	%xmm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpmovzxbw	%xmm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpmovzxbw	(%rcx), %ymm30	 # AVX512{BW,VL}
	vpmovzxbw	0x123(%rax,%r14,8), %ymm30	 # AVX512{BW,VL}
	vpmovzxbw	2032(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpmovzxbw	2048(%rdx), %ymm30	 # AVX512{BW,VL}
	vpmovzxbw	-2048(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpmovzxbw	-2064(%rdx), %ymm30	 # AVX512{BW,VL}
	vpmulhrsw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmulhrsw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpmulhrsw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpmulhrsw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmulhrsw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmulhrsw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmulhrsw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmulhrsw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmulhrsw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmulhrsw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmulhrsw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpmulhrsw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpmulhrsw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmulhrsw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmulhrsw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmulhrsw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmulhrsw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmulhrsw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmulhuw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmulhuw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpmulhuw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpmulhuw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmulhuw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmulhuw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmulhuw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmulhuw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmulhuw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmulhuw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmulhuw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpmulhuw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpmulhuw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmulhuw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmulhuw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmulhuw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmulhuw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmulhuw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmulhw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmulhw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpmulhw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpmulhw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmulhw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmulhw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmulhw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmulhw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmulhw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmulhw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmulhw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpmulhw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpmulhw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmulhw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmulhw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmulhw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmulhw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmulhw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmullw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmullw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpmullw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpmullw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmullw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmullw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmullw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmullw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpmullw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpmullw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmullw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpmullw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpmullw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmullw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmullw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmullw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpmullw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpmullw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsadbw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsadbw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsadbw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsadbw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsadbw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsadbw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsadbw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsadbw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsadbw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsadbw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsadbw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsadbw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsadbw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsadbw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpshufb	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpshufb	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpshufb	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpshufb	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpshufb	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpshufb	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpshufb	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpshufb	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpshufb	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpshufb	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpshufb	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpshufb	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpshufb	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpshufb	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpshufb	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpshufb	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpshufb	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpshufb	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpshufhw	$0xab, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpshufhw	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpshufhw	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpshufhw	$123, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpshufhw	$123, (%rcx), %xmm30	 # AVX512{BW,VL}
	vpshufhw	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{BW,VL}
	vpshufhw	$123, 2032(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpshufhw	$123, 2048(%rdx), %xmm30	 # AVX512{BW,VL}
	vpshufhw	$123, -2048(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpshufhw	$123, -2064(%rdx), %xmm30	 # AVX512{BW,VL}
	vpshufhw	$0xab, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpshufhw	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpshufhw	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpshufhw	$123, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpshufhw	$123, (%rcx), %ymm30	 # AVX512{BW,VL}
	vpshufhw	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{BW,VL}
	vpshufhw	$123, 4064(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpshufhw	$123, 4096(%rdx), %ymm30	 # AVX512{BW,VL}
	vpshufhw	$123, -4096(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpshufhw	$123, -4128(%rdx), %ymm30	 # AVX512{BW,VL}
	vpshuflw	$0xab, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpshuflw	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpshuflw	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpshuflw	$123, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpshuflw	$123, (%rcx), %xmm30	 # AVX512{BW,VL}
	vpshuflw	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{BW,VL}
	vpshuflw	$123, 2032(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpshuflw	$123, 2048(%rdx), %xmm30	 # AVX512{BW,VL}
	vpshuflw	$123, -2048(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpshuflw	$123, -2064(%rdx), %xmm30	 # AVX512{BW,VL}
	vpshuflw	$0xab, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpshuflw	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpshuflw	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpshuflw	$123, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpshuflw	$123, (%rcx), %ymm30	 # AVX512{BW,VL}
	vpshuflw	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{BW,VL}
	vpshuflw	$123, 4064(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpshuflw	$123, 4096(%rdx), %ymm30	 # AVX512{BW,VL}
	vpshuflw	$123, -4096(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpshuflw	$123, -4128(%rdx), %ymm30	 # AVX512{BW,VL}
	vpsllw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsllw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpsllw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpsllw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsllw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsllw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsllw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsllw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsllw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsllw	%xmm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsllw	%xmm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpsllw	%xmm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpsllw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsllw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsllw	2032(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsllw	2048(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsllw	-2048(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsllw	-2064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsraw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsraw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpsraw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpsraw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsraw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsraw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsraw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsraw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsraw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsraw	%xmm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsraw	%xmm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpsraw	%xmm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpsraw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsraw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsraw	2032(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsraw	2048(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsraw	-2048(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsraw	-2064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsrlw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsrlw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpsrlw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpsrlw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsrlw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsrlw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsrlw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsrlw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsrlw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsrlw	%xmm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsrlw	%xmm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpsrlw	%xmm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpsrlw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsrlw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsrlw	2032(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsrlw	2048(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsrlw	-2048(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsrlw	-2064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsrldq	$0xab, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsrldq	$123, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsrldq	$123, (%rcx), %xmm30	 # AVX512{BW,VL}
	vpsrldq	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{BW,VL}
	vpsrldq	$123, 2032(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpsrldq	$123, 2048(%rdx), %xmm30	 # AVX512{BW,VL}
	vpsrldq	$123, -2048(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpsrldq	$123, -2064(%rdx), %xmm30	 # AVX512{BW,VL}
	vpsrldq	$0xab, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsrldq	$123, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsrldq	$123, (%rcx), %ymm30	 # AVX512{BW,VL}
	vpsrldq	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{BW,VL}
	vpsrldq	$123, 4064(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpsrldq	$123, 4096(%rdx), %ymm30	 # AVX512{BW,VL}
	vpsrldq	$123, -4096(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpsrldq	$123, -4128(%rdx), %ymm30	 # AVX512{BW,VL}
	vpsrlw	$0xab, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsrlw	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpsrlw	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpsrlw	$123, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsrlw	$123, (%rcx), %xmm30	 # AVX512{BW,VL}
	vpsrlw	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{BW,VL}
	vpsrlw	$123, 2032(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpsrlw	$123, 2048(%rdx), %xmm30	 # AVX512{BW,VL}
	vpsrlw	$123, -2048(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpsrlw	$123, -2064(%rdx), %xmm30	 # AVX512{BW,VL}
	vpsrlw	$0xab, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsrlw	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpsrlw	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpsrlw	$123, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsrlw	$123, (%rcx), %ymm30	 # AVX512{BW,VL}
	vpsrlw	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{BW,VL}
	vpsrlw	$123, 4064(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpsrlw	$123, 4096(%rdx), %ymm30	 # AVX512{BW,VL}
	vpsrlw	$123, -4096(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpsrlw	$123, -4128(%rdx), %ymm30	 # AVX512{BW,VL}
	vpsraw	$0xab, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsraw	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpsraw	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpsraw	$123, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsraw	$123, (%rcx), %xmm30	 # AVX512{BW,VL}
	vpsraw	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{BW,VL}
	vpsraw	$123, 2032(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpsraw	$123, 2048(%rdx), %xmm30	 # AVX512{BW,VL}
	vpsraw	$123, -2048(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpsraw	$123, -2064(%rdx), %xmm30	 # AVX512{BW,VL}
	vpsraw	$0xab, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsraw	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpsraw	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpsraw	$123, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsraw	$123, (%rcx), %ymm30	 # AVX512{BW,VL}
	vpsraw	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{BW,VL}
	vpsraw	$123, 4064(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpsraw	$123, 4096(%rdx), %ymm30	 # AVX512{BW,VL}
	vpsraw	$123, -4096(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpsraw	$123, -4128(%rdx), %ymm30	 # AVX512{BW,VL}
	vpsubb	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubb	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpsubb	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpsubb	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubb	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubb	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsubb	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubb	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsubb	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubb	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubb	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpsubb	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpsubb	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubb	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubb	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsubb	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubb	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsubb	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubsb	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubsb	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpsubsb	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpsubsb	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubsb	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubsb	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsubsb	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubsb	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsubsb	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubsb	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubsb	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpsubsb	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpsubsb	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubsb	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubsb	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsubsb	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubsb	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsubsb	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubsw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubsw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpsubsw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpsubsw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubsw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubsw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsubsw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubsw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsubsw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubsw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubsw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpsubsw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpsubsw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubsw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubsw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsubsw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubsw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsubsw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubusb	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubusb	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpsubusb	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpsubusb	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubusb	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubusb	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsubusb	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubusb	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsubusb	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubusb	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubusb	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpsubusb	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpsubusb	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubusb	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubusb	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsubusb	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubusb	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsubusb	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubusw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubusw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpsubusw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpsubusw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubusw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubusw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsubusw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubusw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsubusw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubusw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubusw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpsubusw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpsubusw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubusw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubusw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsubusw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubusw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsubusw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpsubw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpsubw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsubw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpsubw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsubw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpsubw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpsubw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsubw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsubw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpsubw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpckhbw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpckhbw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpunpckhbw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpunpckhbw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpckhbw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpckhbw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpunpckhbw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpckhbw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpunpckhbw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpckhbw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpckhbw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpunpckhbw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpunpckhbw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpckhbw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpckhbw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpunpckhbw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpckhbw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpunpckhbw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpckhwd	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpckhwd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpunpckhwd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpunpckhwd	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpckhwd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpckhwd	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpunpckhwd	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpckhwd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpunpckhwd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpckhwd	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpckhwd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpunpckhwd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpunpckhwd	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpckhwd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpckhwd	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpunpckhwd	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpckhwd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpunpckhwd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpcklbw	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpcklbw	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpunpcklbw	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpunpcklbw	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpcklbw	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpcklbw	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpunpcklbw	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpcklbw	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpunpcklbw	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpcklbw	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpcklbw	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpunpcklbw	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpunpcklbw	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpcklbw	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpcklbw	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpunpcklbw	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpcklbw	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpunpcklbw	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpcklwd	%xmm28, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpcklwd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpunpcklwd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpunpcklwd	(%rcx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpcklwd	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpcklwd	2032(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpunpcklwd	2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpcklwd	-2048(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL} Disp8
	vpunpcklwd	-2064(%rdx), %xmm29, %xmm30	 # AVX512{BW,VL}
	vpunpcklwd	%ymm28, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpcklwd	%ymm28, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpunpcklwd	%ymm28, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpunpcklwd	(%rcx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpcklwd	0x123(%rax,%r14,8), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpcklwd	4064(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpunpcklwd	4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpunpcklwd	-4096(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL} Disp8
	vpunpcklwd	-4128(%rdx), %ymm29, %ymm30	 # AVX512{BW,VL}
	vpslldq	$0xab, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpslldq	$123, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpslldq	$123, (%rcx), %xmm30	 # AVX512{BW,VL}
	vpslldq	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{BW,VL}
	vpslldq	$123, 2032(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpslldq	$123, 2048(%rdx), %xmm30	 # AVX512{BW,VL}
	vpslldq	$123, -2048(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpslldq	$123, -2064(%rdx), %xmm30	 # AVX512{BW,VL}
	vpslldq	$0xab, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpslldq	$123, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpslldq	$123, (%rcx), %ymm30	 # AVX512{BW,VL}
	vpslldq	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{BW,VL}
	vpslldq	$123, 4064(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpslldq	$123, 4096(%rdx), %ymm30	 # AVX512{BW,VL}
	vpslldq	$123, -4096(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpslldq	$123, -4128(%rdx), %ymm30	 # AVX512{BW,VL}
	vpsllw	$0xab, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsllw	$0xab, %xmm29, %xmm30{%k7}	 # AVX512{BW,VL}
	vpsllw	$0xab, %xmm29, %xmm30{%k7}{z}	 # AVX512{BW,VL}
	vpsllw	$123, %xmm29, %xmm30	 # AVX512{BW,VL}
	vpsllw	$123, (%rcx), %xmm30	 # AVX512{BW,VL}
	vpsllw	$123, 0x123(%rax,%r14,8), %xmm30	 # AVX512{BW,VL}
	vpsllw	$123, 2032(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpsllw	$123, 2048(%rdx), %xmm30	 # AVX512{BW,VL}
	vpsllw	$123, -2048(%rdx), %xmm30	 # AVX512{BW,VL} Disp8
	vpsllw	$123, -2064(%rdx), %xmm30	 # AVX512{BW,VL}
	vpsllw	$0xab, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsllw	$0xab, %ymm29, %ymm30{%k7}	 # AVX512{BW,VL}
	vpsllw	$0xab, %ymm29, %ymm30{%k7}{z}	 # AVX512{BW,VL}
	vpsllw	$123, %ymm29, %ymm30	 # AVX512{BW,VL}
	vpsllw	$123, (%rcx), %ymm30	 # AVX512{BW,VL}
	vpsllw	$123, 0x123(%rax,%r14,8), %ymm30	 # AVX512{BW,VL}
	vpsllw	$123, 4064(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpsllw	$123, 4096(%rdx), %ymm30	 # AVX512{BW,VL}
	vpsllw	$123, -4096(%rdx), %ymm30	 # AVX512{BW,VL} Disp8
	vpsllw	$123, -4128(%rdx), %ymm30	 # AVX512{BW,VL}

	.intel_syntax noprefix
	vpabsb	xmm30, xmm29	 # AVX512{BW,VL}
	vpabsb	xmm30{k7}, xmm29	 # AVX512{BW,VL}
	vpabsb	xmm30{k7}{z}, xmm29	 # AVX512{BW,VL}
	vpabsb	xmm30, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpabsb	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpabsb	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpabsb	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpabsb	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpabsb	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpabsb	ymm30, ymm29	 # AVX512{BW,VL}
	vpabsb	ymm30{k7}, ymm29	 # AVX512{BW,VL}
	vpabsb	ymm30{k7}{z}, ymm29	 # AVX512{BW,VL}
	vpabsb	ymm30, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpabsb	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpabsb	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpabsb	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpabsb	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpabsb	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpabsw	xmm30, xmm29	 # AVX512{BW,VL}
	vpabsw	xmm30{k7}, xmm29	 # AVX512{BW,VL}
	vpabsw	xmm30{k7}{z}, xmm29	 # AVX512{BW,VL}
	vpabsw	xmm30, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpabsw	xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpabsw	xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpabsw	xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpabsw	xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpabsw	xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpabsw	ymm30, ymm29	 # AVX512{BW,VL}
	vpabsw	ymm30{k7}, ymm29	 # AVX512{BW,VL}
	vpabsw	ymm30{k7}{z}, ymm29	 # AVX512{BW,VL}
	vpabsw	ymm30, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpabsw	ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpabsw	ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpabsw	ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpabsw	ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpabsw	ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpacksswb	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpacksswb	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpacksswb	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpacksswb	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpacksswb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpacksswb	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpacksswb	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpacksswb	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpacksswb	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpacksswb	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpacksswb	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpacksswb	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpacksswb	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpacksswb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpacksswb	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpacksswb	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpacksswb	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpacksswb	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpackuswb	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpackuswb	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpackuswb	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpackuswb	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpackuswb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpackuswb	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpackuswb	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpackuswb	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpackuswb	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpackuswb	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpackuswb	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpackuswb	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpackuswb	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpackuswb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpackuswb	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpackuswb	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpackuswb	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpackuswb	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpaddb	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddb	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddb	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddb	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpaddb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpaddb	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpaddb	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpaddb	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpaddb	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpaddb	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddb	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddb	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddb	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpaddb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpaddb	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpaddb	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpaddb	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpaddb	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpaddsb	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddsb	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddsb	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddsb	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpaddsb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpaddsb	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpaddsb	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpaddsb	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpaddsb	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpaddsb	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddsb	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddsb	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddsb	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpaddsb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpaddsb	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpaddsb	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpaddsb	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpaddsb	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpaddsw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddsw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddsw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddsw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpaddsw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpaddsw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpaddsw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpaddsw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpaddsw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpaddsw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddsw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddsw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddsw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpaddsw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpaddsw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpaddsw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpaddsw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpaddsw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpaddusb	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddusb	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddusb	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddusb	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpaddusb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpaddusb	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpaddusb	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpaddusb	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpaddusb	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpaddusb	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddusb	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddusb	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddusb	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpaddusb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpaddusb	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpaddusb	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpaddusb	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpaddusb	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpaddusw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddusw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddusw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddusw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpaddusw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpaddusw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpaddusw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpaddusw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpaddusw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpaddusw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddusw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddusw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddusw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpaddusw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpaddusw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpaddusw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpaddusw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpaddusw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpaddw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpaddw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpaddw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpaddw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpaddw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpaddw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpaddw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpaddw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpaddw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpaddw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpaddw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpaddw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpaddw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpaddw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpalignr	xmm30, xmm29, xmm28, 0xab	 # AVX512{BW,VL}
	vpalignr	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512{BW,VL}
	vpalignr	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512{BW,VL}
	vpalignr	xmm30, xmm29, xmm28, 123	 # AVX512{BW,VL}
	vpalignr	xmm30, xmm29, XMMWORD PTR [rcx], 123	 # AVX512{BW,VL}
	vpalignr	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{BW,VL}
	vpalignr	xmm30, xmm29, XMMWORD PTR [rdx+2032], 123	 # AVX512{BW,VL} Disp8
	vpalignr	xmm30, xmm29, XMMWORD PTR [rdx+2048], 123	 # AVX512{BW,VL}
	vpalignr	xmm30, xmm29, XMMWORD PTR [rdx-2048], 123	 # AVX512{BW,VL} Disp8
	vpalignr	xmm30, xmm29, XMMWORD PTR [rdx-2064], 123	 # AVX512{BW,VL}
	vpalignr	ymm30, ymm29, ymm28, 0xab	 # AVX512{BW,VL}
	vpalignr	ymm30{k7}, ymm29, ymm28, 0xab	 # AVX512{BW,VL}
	vpalignr	ymm30{k7}{z}, ymm29, ymm28, 0xab	 # AVX512{BW,VL}
	vpalignr	ymm30, ymm29, ymm28, 123	 # AVX512{BW,VL}
	vpalignr	ymm30, ymm29, YMMWORD PTR [rcx], 123	 # AVX512{BW,VL}
	vpalignr	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{BW,VL}
	vpalignr	ymm30, ymm29, YMMWORD PTR [rdx+4064], 123	 # AVX512{BW,VL} Disp8
	vpalignr	ymm30, ymm29, YMMWORD PTR [rdx+4096], 123	 # AVX512{BW,VL}
	vpalignr	ymm30, ymm29, YMMWORD PTR [rdx-4096], 123	 # AVX512{BW,VL} Disp8
	vpalignr	ymm30, ymm29, YMMWORD PTR [rdx-4128], 123	 # AVX512{BW,VL}
	vpavgb	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpavgb	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpavgb	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpavgb	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpavgb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpavgb	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpavgb	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpavgb	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpavgb	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpavgb	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpavgb	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpavgb	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpavgb	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpavgb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpavgb	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpavgb	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpavgb	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpavgb	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpavgw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpavgw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpavgw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpavgw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpavgw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpavgw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpavgw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpavgw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpavgw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpavgw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpavgw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpavgw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpavgw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpavgw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpavgw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpavgw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpavgw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpavgw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpcmpeqb	k5, xmm30, xmm29	 # AVX512{BW,VL}
	vpcmpeqb	k5{k7}, xmm30, xmm29	 # AVX512{BW,VL}
	vpcmpeqb	k5, xmm30, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpcmpeqb	k5, xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpcmpeqb	k5, xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpcmpeqb	k5, xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpcmpeqb	k5, xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpcmpeqb	k5, xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpcmpeqb	k5, ymm30, ymm29	 # AVX512{BW,VL}
	vpcmpeqb	k5{k7}, ymm30, ymm29	 # AVX512{BW,VL}
	vpcmpeqb	k5, ymm30, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpcmpeqb	k5, ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpcmpeqb	k5, ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpcmpeqb	k5, ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpcmpeqb	k5, ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpcmpeqb	k5, ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpcmpeqw	k5, xmm30, xmm29	 # AVX512{BW,VL}
	vpcmpeqw	k5{k7}, xmm30, xmm29	 # AVX512{BW,VL}
	vpcmpeqw	k5, xmm30, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpcmpeqw	k5, xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpcmpeqw	k5, xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpcmpeqw	k5, xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpcmpeqw	k5, xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpcmpeqw	k5, xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpcmpeqw	k5, ymm30, ymm29	 # AVX512{BW,VL}
	vpcmpeqw	k5{k7}, ymm30, ymm29	 # AVX512{BW,VL}
	vpcmpeqw	k5, ymm30, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpcmpeqw	k5, ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpcmpeqw	k5, ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpcmpeqw	k5, ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpcmpeqw	k5, ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpcmpeqw	k5, ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpcmpgtb	k5, xmm30, xmm29	 # AVX512{BW,VL}
	vpcmpgtb	k5{k7}, xmm30, xmm29	 # AVX512{BW,VL}
	vpcmpgtb	k5, xmm30, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpcmpgtb	k5, xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpcmpgtb	k5, xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpcmpgtb	k5, xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpcmpgtb	k5, xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpcmpgtb	k5, xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpcmpgtb	k5, ymm30, ymm29	 # AVX512{BW,VL}
	vpcmpgtb	k5{k7}, ymm30, ymm29	 # AVX512{BW,VL}
	vpcmpgtb	k5, ymm30, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpcmpgtb	k5, ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpcmpgtb	k5, ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpcmpgtb	k5, ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpcmpgtb	k5, ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpcmpgtb	k5, ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpcmpgtw	k5, xmm30, xmm29	 # AVX512{BW,VL}
	vpcmpgtw	k5{k7}, xmm30, xmm29	 # AVX512{BW,VL}
	vpcmpgtw	k5, xmm30, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpcmpgtw	k5, xmm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpcmpgtw	k5, xmm30, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpcmpgtw	k5, xmm30, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpcmpgtw	k5, xmm30, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpcmpgtw	k5, xmm30, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpcmpgtw	k5, ymm30, ymm29	 # AVX512{BW,VL}
	vpcmpgtw	k5{k7}, ymm30, ymm29	 # AVX512{BW,VL}
	vpcmpgtw	k5, ymm30, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpcmpgtw	k5, ymm30, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpcmpgtw	k5, ymm30, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpcmpgtw	k5, ymm30, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpcmpgtw	k5, ymm30, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpcmpgtw	k5, ymm30, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpmaddubsw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaddubsw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaddubsw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaddubsw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmaddubsw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmaddubsw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpmaddubsw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpmaddubsw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpmaddubsw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpmaddubsw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaddubsw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaddubsw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaddubsw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmaddubsw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmaddubsw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpmaddubsw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpmaddubsw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpmaddubsw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpmaddwd	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaddwd	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaddwd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaddwd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmaddwd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmaddwd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpmaddwd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpmaddwd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpmaddwd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpmaddwd	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaddwd	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaddwd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaddwd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmaddwd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmaddwd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpmaddwd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpmaddwd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpmaddwd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpmaxsb	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaxsb	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaxsb	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaxsb	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmaxsb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmaxsb	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpmaxsb	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpmaxsb	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpmaxsb	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpmaxsb	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaxsb	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaxsb	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaxsb	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmaxsb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmaxsb	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpmaxsb	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpmaxsb	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpmaxsb	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpmaxsw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaxsw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaxsw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaxsw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmaxsw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmaxsw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpmaxsw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpmaxsw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpmaxsw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpmaxsw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaxsw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaxsw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaxsw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmaxsw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmaxsw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpmaxsw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpmaxsw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpmaxsw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpmaxub	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaxub	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaxub	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaxub	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmaxub	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmaxub	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpmaxub	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpmaxub	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpmaxub	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpmaxub	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaxub	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaxub	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaxub	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmaxub	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmaxub	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpmaxub	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpmaxub	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpmaxub	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpmaxuw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaxuw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaxuw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmaxuw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmaxuw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmaxuw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpmaxuw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpmaxuw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpmaxuw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpmaxuw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaxuw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaxuw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmaxuw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmaxuw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmaxuw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpmaxuw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpmaxuw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpmaxuw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpminsb	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpminsb	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpminsb	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpminsb	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpminsb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpminsb	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpminsb	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpminsb	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpminsb	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpminsb	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpminsb	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpminsb	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpminsb	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpminsb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpminsb	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpminsb	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpminsb	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpminsb	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpminsw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpminsw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpminsw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpminsw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpminsw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpminsw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpminsw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpminsw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpminsw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpminsw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpminsw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpminsw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpminsw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpminsw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpminsw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpminsw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpminsw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpminsw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpminub	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpminub	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpminub	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpminub	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpminub	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpminub	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpminub	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpminub	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpminub	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpminub	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpminub	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpminub	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpminub	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpminub	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpminub	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpminub	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpminub	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpminub	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpminuw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpminuw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpminuw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpminuw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpminuw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpminuw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpminuw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpminuw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpminuw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpminuw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpminuw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpminuw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpminuw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpminuw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpminuw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpminuw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpminuw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpminuw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpmovsxbw	xmm30, xmm29	 # AVX512{BW,VL}
	vpmovsxbw	xmm30{k7}, xmm29	 # AVX512{BW,VL}
	vpmovsxbw	xmm30{k7}{z}, xmm29	 # AVX512{BW,VL}
	vpmovsxbw	xmm30, QWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmovsxbw	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmovsxbw	xmm30, QWORD PTR [rdx+1016]	 # AVX512{BW,VL} Disp8
	vpmovsxbw	xmm30, QWORD PTR [rdx+1024]	 # AVX512{BW,VL}
	vpmovsxbw	xmm30, QWORD PTR [rdx-1024]	 # AVX512{BW,VL} Disp8
	vpmovsxbw	xmm30, QWORD PTR [rdx-1032]	 # AVX512{BW,VL}
	vpmovsxbw	ymm30, xmm29	 # AVX512{BW,VL}
	vpmovsxbw	ymm30{k7}, xmm29	 # AVX512{BW,VL}
	vpmovsxbw	ymm30{k7}{z}, xmm29	 # AVX512{BW,VL}
	vpmovsxbw	ymm30, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmovsxbw	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmovsxbw	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpmovsxbw	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpmovsxbw	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpmovsxbw	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpmovzxbw	xmm30, xmm29	 # AVX512{BW,VL}
	vpmovzxbw	xmm30{k7}, xmm29	 # AVX512{BW,VL}
	vpmovzxbw	xmm30{k7}{z}, xmm29	 # AVX512{BW,VL}
	vpmovzxbw	xmm30, QWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmovzxbw	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmovzxbw	xmm30, QWORD PTR [rdx+1016]	 # AVX512{BW,VL} Disp8
	vpmovzxbw	xmm30, QWORD PTR [rdx+1024]	 # AVX512{BW,VL}
	vpmovzxbw	xmm30, QWORD PTR [rdx-1024]	 # AVX512{BW,VL} Disp8
	vpmovzxbw	xmm30, QWORD PTR [rdx-1032]	 # AVX512{BW,VL}
	vpmovzxbw	ymm30, xmm29	 # AVX512{BW,VL}
	vpmovzxbw	ymm30{k7}, xmm29	 # AVX512{BW,VL}
	vpmovzxbw	ymm30{k7}{z}, xmm29	 # AVX512{BW,VL}
	vpmovzxbw	ymm30, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmovzxbw	ymm30, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmovzxbw	ymm30, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpmovzxbw	ymm30, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpmovzxbw	ymm30, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpmovzxbw	ymm30, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpmulhrsw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpmulhrsw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmulhrsw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmulhrsw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmulhrsw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmulhrsw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpmulhrsw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpmulhrsw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpmulhrsw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpmulhrsw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpmulhrsw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmulhrsw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmulhrsw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmulhrsw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmulhrsw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpmulhrsw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpmulhrsw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpmulhrsw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpmulhuw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpmulhuw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmulhuw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmulhuw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmulhuw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmulhuw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpmulhuw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpmulhuw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpmulhuw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpmulhuw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpmulhuw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmulhuw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmulhuw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmulhuw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmulhuw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpmulhuw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpmulhuw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpmulhuw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpmulhw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpmulhw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmulhw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmulhw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmulhw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmulhw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpmulhw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpmulhw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpmulhw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpmulhw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpmulhw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmulhw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmulhw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmulhw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmulhw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpmulhw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpmulhw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpmulhw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpmullw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpmullw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmullw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpmullw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmullw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmullw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpmullw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpmullw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpmullw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpmullw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpmullw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmullw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpmullw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpmullw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpmullw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpmullw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpmullw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpmullw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpsadbw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpsadbw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsadbw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsadbw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpsadbw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpsadbw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpsadbw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpsadbw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpsadbw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsadbw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsadbw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpsadbw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpsadbw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpsadbw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpshufb	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpshufb	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpshufb	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpshufb	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpshufb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpshufb	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpshufb	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpshufb	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpshufb	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpshufb	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpshufb	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpshufb	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpshufb	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpshufb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpshufb	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpshufb	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpshufb	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpshufb	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpshufhw	xmm30, xmm29, 0xab	 # AVX512{BW,VL}
	vpshufhw	xmm30{k7}, xmm29, 0xab	 # AVX512{BW,VL}
	vpshufhw	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{BW,VL}
	vpshufhw	xmm30, xmm29, 123	 # AVX512{BW,VL}
	vpshufhw	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{BW,VL}
	vpshufhw	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{BW,VL}
	vpshufhw	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{BW,VL} Disp8
	vpshufhw	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{BW,VL}
	vpshufhw	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{BW,VL} Disp8
	vpshufhw	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{BW,VL}
	vpshufhw	ymm30, ymm29, 0xab	 # AVX512{BW,VL}
	vpshufhw	ymm30{k7}, ymm29, 0xab	 # AVX512{BW,VL}
	vpshufhw	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{BW,VL}
	vpshufhw	ymm30, ymm29, 123	 # AVX512{BW,VL}
	vpshufhw	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{BW,VL}
	vpshufhw	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{BW,VL}
	vpshufhw	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{BW,VL} Disp8
	vpshufhw	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{BW,VL}
	vpshufhw	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{BW,VL} Disp8
	vpshufhw	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{BW,VL}
	vpshuflw	xmm30, xmm29, 0xab	 # AVX512{BW,VL}
	vpshuflw	xmm30{k7}, xmm29, 0xab	 # AVX512{BW,VL}
	vpshuflw	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{BW,VL}
	vpshuflw	xmm30, xmm29, 123	 # AVX512{BW,VL}
	vpshuflw	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{BW,VL}
	vpshuflw	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{BW,VL}
	vpshuflw	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{BW,VL} Disp8
	vpshuflw	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{BW,VL}
	vpshuflw	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{BW,VL} Disp8
	vpshuflw	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{BW,VL}
	vpshuflw	ymm30, ymm29, 0xab	 # AVX512{BW,VL}
	vpshuflw	ymm30{k7}, ymm29, 0xab	 # AVX512{BW,VL}
	vpshuflw	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{BW,VL}
	vpshuflw	ymm30, ymm29, 123	 # AVX512{BW,VL}
	vpshuflw	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{BW,VL}
	vpshuflw	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{BW,VL}
	vpshuflw	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{BW,VL} Disp8
	vpshuflw	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{BW,VL}
	vpshuflw	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{BW,VL} Disp8
	vpshuflw	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{BW,VL}
	vpsllw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpsllw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsllw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsllw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsllw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsllw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpsllw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpsllw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpsllw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpsllw	ymm30, ymm29, xmm28	 # AVX512{BW,VL}
	vpsllw	ymm30{k7}, ymm29, xmm28	 # AVX512{BW,VL}
	vpsllw	ymm30{k7}{z}, ymm29, xmm28	 # AVX512{BW,VL}
	vpsllw	ymm30, ymm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsllw	ymm30, ymm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsllw	ymm30, ymm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpsllw	ymm30, ymm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpsllw	ymm30, ymm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpsllw	ymm30, ymm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpsraw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpsraw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsraw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsraw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsraw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsraw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpsraw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpsraw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpsraw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpsraw	ymm30, ymm29, xmm28	 # AVX512{BW,VL}
	vpsraw	ymm30{k7}, ymm29, xmm28	 # AVX512{BW,VL}
	vpsraw	ymm30{k7}{z}, ymm29, xmm28	 # AVX512{BW,VL}
	vpsraw	ymm30, ymm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsraw	ymm30, ymm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsraw	ymm30, ymm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpsraw	ymm30, ymm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpsraw	ymm30, ymm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpsraw	ymm30, ymm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpsrlw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpsrlw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsrlw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsrlw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsrlw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsrlw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpsrlw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpsrlw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpsrlw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpsrlw	ymm30, ymm29, xmm28	 # AVX512{BW,VL}
	vpsrlw	ymm30{k7}, ymm29, xmm28	 # AVX512{BW,VL}
	vpsrlw	ymm30{k7}{z}, ymm29, xmm28	 # AVX512{BW,VL}
	vpsrlw	ymm30, ymm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsrlw	ymm30, ymm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsrlw	ymm30, ymm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpsrlw	ymm30, ymm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpsrlw	ymm30, ymm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpsrlw	ymm30, ymm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpsrldq	xmm30, xmm29, 0xab	 # AVX512{BW,VL}
	vpsrldq	xmm30, xmm29, 123	 # AVX512{BW,VL}
	vpsrldq	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{BW,VL}
	vpsrldq	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{BW,VL}
	vpsrldq	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{BW,VL} Disp8
	vpsrldq	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{BW,VL}
	vpsrldq	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{BW,VL} Disp8
	vpsrldq	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{BW,VL}
	vpsrldq	ymm30, ymm29, 0xab	 # AVX512{BW,VL}
	vpsrldq	ymm30, ymm29, 123	 # AVX512{BW,VL}
	vpsrldq	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{BW,VL}
	vpsrldq	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{BW,VL}
	vpsrldq	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{BW,VL} Disp8
	vpsrldq	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{BW,VL}
	vpsrldq	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{BW,VL} Disp8
	vpsrldq	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{BW,VL}
	vpsrlw	xmm30, xmm29, 0xab	 # AVX512{BW,VL}
	vpsrlw	xmm30{k7}, xmm29, 0xab	 # AVX512{BW,VL}
	vpsrlw	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{BW,VL}
	vpsrlw	xmm30, xmm29, 123	 # AVX512{BW,VL}
	vpsrlw	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{BW,VL}
	vpsrlw	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{BW,VL}
	vpsrlw	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{BW,VL} Disp8
	vpsrlw	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{BW,VL}
	vpsrlw	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{BW,VL} Disp8
	vpsrlw	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{BW,VL}
	vpsrlw	ymm30, ymm29, 0xab	 # AVX512{BW,VL}
	vpsrlw	ymm30{k7}, ymm29, 0xab	 # AVX512{BW,VL}
	vpsrlw	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{BW,VL}
	vpsrlw	ymm30, ymm29, 123	 # AVX512{BW,VL}
	vpsrlw	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{BW,VL}
	vpsrlw	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{BW,VL}
	vpsrlw	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{BW,VL} Disp8
	vpsrlw	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{BW,VL}
	vpsrlw	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{BW,VL} Disp8
	vpsrlw	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{BW,VL}
	vpsraw	xmm30, xmm29, 0xab	 # AVX512{BW,VL}
	vpsraw	xmm30{k7}, xmm29, 0xab	 # AVX512{BW,VL}
	vpsraw	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{BW,VL}
	vpsraw	xmm30, xmm29, 123	 # AVX512{BW,VL}
	vpsraw	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{BW,VL}
	vpsraw	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{BW,VL}
	vpsraw	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{BW,VL} Disp8
	vpsraw	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{BW,VL}
	vpsraw	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{BW,VL} Disp8
	vpsraw	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{BW,VL}
	vpsraw	ymm30, ymm29, 0xab	 # AVX512{BW,VL}
	vpsraw	ymm30{k7}, ymm29, 0xab	 # AVX512{BW,VL}
	vpsraw	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{BW,VL}
	vpsraw	ymm30, ymm29, 123	 # AVX512{BW,VL}
	vpsraw	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{BW,VL}
	vpsraw	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{BW,VL}
	vpsraw	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{BW,VL} Disp8
	vpsraw	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{BW,VL}
	vpsraw	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{BW,VL} Disp8
	vpsraw	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{BW,VL}
	vpsubb	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubb	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubb	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubb	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsubb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsubb	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpsubb	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpsubb	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpsubb	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpsubb	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubb	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubb	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubb	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsubb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsubb	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpsubb	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpsubb	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpsubb	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpsubsb	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubsb	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubsb	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubsb	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsubsb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsubsb	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpsubsb	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpsubsb	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpsubsb	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpsubsb	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubsb	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubsb	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubsb	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsubsb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsubsb	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpsubsb	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpsubsb	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpsubsb	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpsubsw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubsw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubsw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubsw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsubsw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsubsw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpsubsw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpsubsw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpsubsw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpsubsw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubsw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubsw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubsw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsubsw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsubsw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpsubsw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpsubsw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpsubsw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpsubusb	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubusb	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubusb	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubusb	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsubusb	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsubusb	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpsubusb	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpsubusb	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpsubusb	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpsubusb	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubusb	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubusb	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubusb	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsubusb	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsubusb	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpsubusb	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpsubusb	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpsubusb	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpsubusw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubusw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubusw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubusw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsubusw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsubusw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpsubusw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpsubusw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpsubusw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpsubusw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubusw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubusw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubusw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsubusw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsubusw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpsubusw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpsubusw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpsubusw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpsubw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpsubw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsubw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsubw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpsubw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpsubw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpsubw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpsubw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpsubw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpsubw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpsubw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpsubw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpsubw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpsubw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpunpckhbw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpunpckhbw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpunpckhbw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpunpckhbw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpunpckhbw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpunpckhbw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpunpckhbw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpunpckhbw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpunpckhbw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpunpckhbw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpunpckhbw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpunpckhbw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpunpckhbw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpunpckhbw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpunpckhbw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpunpckhbw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpunpckhbw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpunpckhbw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpunpckhwd	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpunpckhwd	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpunpckhwd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpunpckhwd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpunpckhwd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpunpckhwd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpunpckhwd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpunpckhwd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpunpckhwd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpunpckhwd	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpunpckhwd	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpunpckhwd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpunpckhwd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpunpckhwd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpunpckhwd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpunpckhwd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpunpckhwd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpunpckhwd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpunpcklbw	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpunpcklbw	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpunpcklbw	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpunpcklbw	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpunpcklbw	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpunpcklbw	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpunpcklbw	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpunpcklbw	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpunpcklbw	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpunpcklbw	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpunpcklbw	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpunpcklbw	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpunpcklbw	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpunpcklbw	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpunpcklbw	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpunpcklbw	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpunpcklbw	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpunpcklbw	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpunpcklwd	xmm30, xmm29, xmm28	 # AVX512{BW,VL}
	vpunpcklwd	xmm30{k7}, xmm29, xmm28	 # AVX512{BW,VL}
	vpunpcklwd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512{BW,VL}
	vpunpcklwd	xmm30, xmm29, XMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpunpcklwd	xmm30, xmm29, XMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpunpcklwd	xmm30, xmm29, XMMWORD PTR [rdx+2032]	 # AVX512{BW,VL} Disp8
	vpunpcklwd	xmm30, xmm29, XMMWORD PTR [rdx+2048]	 # AVX512{BW,VL}
	vpunpcklwd	xmm30, xmm29, XMMWORD PTR [rdx-2048]	 # AVX512{BW,VL} Disp8
	vpunpcklwd	xmm30, xmm29, XMMWORD PTR [rdx-2064]	 # AVX512{BW,VL}
	vpunpcklwd	ymm30, ymm29, ymm28	 # AVX512{BW,VL}
	vpunpcklwd	ymm30{k7}, ymm29, ymm28	 # AVX512{BW,VL}
	vpunpcklwd	ymm30{k7}{z}, ymm29, ymm28	 # AVX512{BW,VL}
	vpunpcklwd	ymm30, ymm29, YMMWORD PTR [rcx]	 # AVX512{BW,VL}
	vpunpcklwd	ymm30, ymm29, YMMWORD PTR [rax+r14*8+0x1234]	 # AVX512{BW,VL}
	vpunpcklwd	ymm30, ymm29, YMMWORD PTR [rdx+4064]	 # AVX512{BW,VL} Disp8
	vpunpcklwd	ymm30, ymm29, YMMWORD PTR [rdx+4096]	 # AVX512{BW,VL}
	vpunpcklwd	ymm30, ymm29, YMMWORD PTR [rdx-4096]	 # AVX512{BW,VL} Disp8
	vpunpcklwd	ymm30, ymm29, YMMWORD PTR [rdx-4128]	 # AVX512{BW,VL}
	vpslldq	xmm30, xmm29, 0xab	 # AVX512{BW,VL}
	vpslldq	xmm30, xmm29, 123	 # AVX512{BW,VL}
	vpslldq	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{BW,VL}
	vpslldq	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{BW,VL}
	vpslldq	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{BW,VL} Disp8
	vpslldq	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{BW,VL}
	vpslldq	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{BW,VL} Disp8
	vpslldq	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{BW,VL}
	vpslldq	ymm30, ymm29, 0xab	 # AVX512{BW,VL}
	vpslldq	ymm30, ymm29, 123	 # AVX512{BW,VL}
	vpslldq	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{BW,VL}
	vpslldq	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{BW,VL}
	vpslldq	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{BW,VL} Disp8
	vpslldq	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{BW,VL}
	vpslldq	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{BW,VL} Disp8
	vpslldq	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{BW,VL}
	vpsllw	xmm30, xmm29, 0xab	 # AVX512{BW,VL}
	vpsllw	xmm30{k7}, xmm29, 0xab	 # AVX512{BW,VL}
	vpsllw	xmm30{k7}{z}, xmm29, 0xab	 # AVX512{BW,VL}
	vpsllw	xmm30, xmm29, 123	 # AVX512{BW,VL}
	vpsllw	xmm30, XMMWORD PTR [rcx], 123	 # AVX512{BW,VL}
	vpsllw	xmm30, XMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{BW,VL}
	vpsllw	xmm30, XMMWORD PTR [rdx+2032], 123	 # AVX512{BW,VL} Disp8
	vpsllw	xmm30, XMMWORD PTR [rdx+2048], 123	 # AVX512{BW,VL}
	vpsllw	xmm30, XMMWORD PTR [rdx-2048], 123	 # AVX512{BW,VL} Disp8
	vpsllw	xmm30, XMMWORD PTR [rdx-2064], 123	 # AVX512{BW,VL}
	vpsllw	ymm30, ymm29, 0xab	 # AVX512{BW,VL}
	vpsllw	ymm30{k7}, ymm29, 0xab	 # AVX512{BW,VL}
	vpsllw	ymm30{k7}{z}, ymm29, 0xab	 # AVX512{BW,VL}
	vpsllw	ymm30, ymm29, 123	 # AVX512{BW,VL}
	vpsllw	ymm30, YMMWORD PTR [rcx], 123	 # AVX512{BW,VL}
	vpsllw	ymm30, YMMWORD PTR [rax+r14*8+0x1234], 123	 # AVX512{BW,VL}
	vpsllw	ymm30, YMMWORD PTR [rdx+4064], 123	 # AVX512{BW,VL} Disp8
	vpsllw	ymm30, YMMWORD PTR [rdx+4096], 123	 # AVX512{BW,VL}
	vpsllw	ymm30, YMMWORD PTR [rdx-4096], 123	 # AVX512{BW,VL} Disp8
	vpsllw	ymm30, YMMWORD PTR [rdx-4128], 123	 # AVX512{BW,VL}
