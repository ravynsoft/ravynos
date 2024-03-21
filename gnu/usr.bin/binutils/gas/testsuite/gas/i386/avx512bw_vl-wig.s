# Check 32bit AVX512{BW,VL} WIG instructions

	.allow_index_reg
	.text
_start:
	vpabsb	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpabsb	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpabsb	(%ecx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpabsb	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BW,VL}
	vpabsb	2032(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpabsb	2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpabsb	-2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpabsb	-2064(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpabsb	%ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpabsb	%ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpabsb	(%ecx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpabsb	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BW,VL}
	vpabsb	4064(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpabsb	4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpabsb	-4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpabsb	-4128(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpabsw	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpabsw	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpabsw	(%ecx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpabsw	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BW,VL}
	vpabsw	2032(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpabsw	2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpabsw	-2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpabsw	-2064(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpabsw	%ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpabsw	%ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpabsw	(%ecx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpabsw	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BW,VL}
	vpabsw	4064(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpabsw	4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpabsw	-4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpabsw	-4128(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpacksswb	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpacksswb	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpacksswb	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpacksswb	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpacksswb	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpacksswb	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpacksswb	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpacksswb	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpacksswb	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpacksswb	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpacksswb	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpacksswb	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpacksswb	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpacksswb	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpacksswb	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpacksswb	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackuswb	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackuswb	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpackuswb	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackuswb	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackuswb	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackuswb	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackuswb	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackuswb	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackuswb	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackuswb	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpackuswb	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackuswb	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackuswb	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackuswb	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackuswb	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackuswb	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddb	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddb	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpaddb	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddb	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddb	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddb	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddb	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddb	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddb	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddb	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpaddb	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddb	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddb	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddb	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddb	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddb	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddsb	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddsb	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpaddsb	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddsb	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddsb	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddsb	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddsb	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddsb	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddsb	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddsb	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpaddsb	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddsb	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddsb	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddsb	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddsb	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddsb	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddsw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddsw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpaddsw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddsw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddsw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddsw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddsw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddsw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddsw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddsw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpaddsw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddsw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddsw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddsw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddsw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddsw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddusb	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddusb	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpaddusb	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddusb	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddusb	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddusb	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddusb	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddusb	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddusb	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddusb	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpaddusb	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddusb	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddusb	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddusb	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddusb	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddusb	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddusw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddusw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpaddusw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddusw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddusw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddusw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddusw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddusw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddusw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddusw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpaddusw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddusw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddusw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddusw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddusw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddusw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpaddw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpaddw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpaddw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpaddw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpaddw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpalignr	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpalignr	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpalignr	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpalignr	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpalignr	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpalignr	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpalignr	$123, 2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpalignr	$123, -2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpalignr	$123, -2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpalignr	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpalignr	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpalignr	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpalignr	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpalignr	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpalignr	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpalignr	$123, 4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpalignr	$123, -4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpalignr	$123, -4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpavgb	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpavgb	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpavgb	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpavgb	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpavgb	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpavgb	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpavgb	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpavgb	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpavgb	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpavgb	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpavgb	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpavgb	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpavgb	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpavgb	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpavgb	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpavgb	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpavgw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpavgw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpavgw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpavgw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpavgw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpavgw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpavgw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpavgw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpavgw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpavgw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpavgw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpavgw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpavgw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpavgw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpavgw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpavgw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpcmpeqb	%xmm5, %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqb	(%ecx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqb	-123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqb	2032(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpeqb	2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqb	-2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpeqb	-2064(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqb	%ymm5, %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqb	(%ecx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqb	-123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqb	4064(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpeqb	4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqb	-4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpeqb	-4128(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqw	%xmm5, %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqw	(%ecx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqw	-123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqw	2032(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpeqw	2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqw	-2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpeqw	-2064(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqw	%ymm5, %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqw	(%ecx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqw	-123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqw	4064(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpeqw	4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpeqw	-4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpeqw	-4128(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtb	%xmm5, %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtb	(%ecx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtb	-123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtb	2032(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpgtb	2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtb	-2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpgtb	-2064(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtb	%ymm5, %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtb	(%ecx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtb	-123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtb	4064(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpgtb	4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtb	-4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpgtb	-4128(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtw	%xmm5, %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtw	(%ecx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtw	-123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtw	2032(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpgtw	2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtw	-2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpgtw	-2064(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtw	%ymm5, %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtw	(%ecx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtw	-123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtw	4064(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpgtw	4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpgtw	-4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpgtw	-4128(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpmaddubsw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaddubsw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmaddubsw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaddubsw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaddubsw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaddubsw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaddubsw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaddubsw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaddubsw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaddubsw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpmaddubsw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaddubsw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaddubsw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaddubsw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaddubsw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaddubsw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaddwd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaddwd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmaddwd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaddwd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaddwd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaddwd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaddwd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaddwd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaddwd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaddwd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpmaddwd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaddwd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaddwd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaddwd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaddwd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaddwd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxsb	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxsb	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmaxsb	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxsb	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxsb	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaxsb	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxsb	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaxsb	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxsb	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxsb	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpmaxsb	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxsb	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxsb	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaxsb	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxsb	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaxsb	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxsw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxsw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmaxsw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxsw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxsw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaxsw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxsw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaxsw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxsw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxsw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpmaxsw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxsw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxsw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaxsw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxsw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaxsw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxub	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxub	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmaxub	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxub	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxub	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaxub	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxub	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaxub	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxub	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxub	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpmaxub	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxub	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxub	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaxub	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxub	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaxub	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxuw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxuw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmaxuw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxuw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxuw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaxuw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxuw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaxuw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmaxuw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxuw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpmaxuw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxuw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxuw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaxuw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmaxuw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmaxuw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminsb	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminsb	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpminsb	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminsb	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminsb	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpminsb	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminsb	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpminsb	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminsb	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminsb	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpminsb	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminsb	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminsb	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpminsb	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminsb	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpminsb	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminsw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminsw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpminsw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminsw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminsw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpminsw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminsw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpminsw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminsw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminsw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpminsw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminsw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminsw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpminsw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminsw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpminsw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminub	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminub	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpminub	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminub	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminub	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpminub	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminub	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpminub	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminub	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminub	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpminub	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminub	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminub	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpminub	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminub	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpminub	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminuw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminuw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpminuw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminuw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminuw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpminuw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminuw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpminuw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpminuw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminuw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpminuw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminuw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminuw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpminuw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpminuw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpminuw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmovsxbw	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmovsxbw	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmovsxbw	(%ecx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpmovsxbw	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BW,VL}
	vpmovsxbw	1016(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmovsxbw	1024(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpmovsxbw	-1024(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmovsxbw	-1032(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpmovsxbw	%xmm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmovsxbw	%xmm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpmovsxbw	(%ecx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpmovsxbw	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BW,VL}
	vpmovsxbw	2032(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmovsxbw	2048(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpmovsxbw	-2048(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmovsxbw	-2064(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpmovzxbw	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmovzxbw	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmovzxbw	(%ecx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpmovzxbw	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BW,VL}
	vpmovzxbw	1016(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmovzxbw	1024(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpmovzxbw	-1024(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmovzxbw	-1032(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpmovzxbw	%xmm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmovzxbw	%xmm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpmovzxbw	(%ecx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpmovzxbw	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BW,VL}
	vpmovzxbw	2032(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmovzxbw	2048(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpmovzxbw	-2048(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmovzxbw	-2064(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpmulhrsw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmulhrsw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmulhrsw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmulhrsw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmulhrsw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmulhrsw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmulhrsw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmulhrsw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmulhrsw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmulhrsw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpmulhrsw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmulhrsw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmulhrsw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmulhrsw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmulhrsw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmulhrsw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmulhuw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmulhuw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmulhuw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmulhuw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmulhuw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmulhuw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmulhuw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmulhuw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmulhuw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmulhuw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpmulhuw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmulhuw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmulhuw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmulhuw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmulhuw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmulhuw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmulhw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmulhw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmulhw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmulhw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmulhw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmulhw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmulhw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmulhw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmulhw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmulhw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpmulhw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmulhw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmulhw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmulhw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmulhw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmulhw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmullw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmullw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmullw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmullw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmullw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmullw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmullw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmullw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmullw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmullw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpmullw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmullw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmullw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmullw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpmullw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpmullw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpshufb	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpshufb	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpshufb	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpshufb	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpshufb	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpshufb	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpshufb	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpshufb	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpshufb	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpshufb	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpshufb	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpshufb	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpshufb	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpshufb	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpshufb	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpshufb	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpshufhw	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpshufhw	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpshufhw	$123, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpshufhw	$123, (%ecx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpshufhw	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BW,VL}
	vpshufhw	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpshufhw	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpshufhw	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpshufhw	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpshufhw	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpshufhw	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpshufhw	$123, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpshufhw	$123, (%ecx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpshufhw	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BW,VL}
	vpshufhw	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpshufhw	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpshufhw	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpshufhw	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpshuflw	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpshuflw	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpshuflw	$123, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpshuflw	$123, (%ecx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpshuflw	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BW,VL}
	vpshuflw	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpshuflw	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpshuflw	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpshuflw	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpshuflw	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpshuflw	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpshuflw	$123, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpshuflw	$123, (%ecx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpshuflw	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BW,VL}
	vpshuflw	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpshuflw	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpshuflw	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpshuflw	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpsllw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsllw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpsllw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsllw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsllw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsllw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsllw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsllw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsllw	%xmm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsllw	%xmm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpsllw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsllw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsllw	2032(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsllw	2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsllw	-2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsllw	-2064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsraw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsraw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpsraw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsraw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsraw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsraw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsraw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsraw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsraw	%xmm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsraw	%xmm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpsraw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsraw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsraw	2032(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsraw	2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsraw	-2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsraw	-2064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpsrlw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsrlw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsrlw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	%xmm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	%xmm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpsrlw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	2032(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsrlw	2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	-2048(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsrlw	-2064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpsrlw	$123, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	$123, (%ecx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsrlw	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsrlw	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpsrlw	$123, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	$123, (%ecx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsrlw	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpsrlw	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsrlw	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpsraw	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsraw	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpsraw	$123, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsraw	$123, (%ecx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpsraw	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BW,VL}
	vpsraw	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsraw	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpsraw	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsraw	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpsraw	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsraw	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpsraw	$123, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsraw	$123, (%ecx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpsraw	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BW,VL}
	vpsraw	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsraw	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpsraw	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsraw	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubb	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubb	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpsubb	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubb	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubb	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubb	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubb	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubb	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubb	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubb	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpsubb	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubb	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubb	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubb	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubb	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubb	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubsb	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubsb	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpsubsb	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubsb	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubsb	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubsb	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubsb	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubsb	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubsb	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubsb	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpsubsb	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubsb	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubsb	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubsb	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubsb	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubsb	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubsw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubsw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpsubsw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubsw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubsw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubsw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubsw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubsw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubsw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubsw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpsubsw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubsw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubsw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubsw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubsw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubsw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubusb	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubusb	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpsubusb	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubusb	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubusb	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubusb	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubusb	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubusb	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubusb	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubusb	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpsubusb	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubusb	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubusb	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubusb	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubusb	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubusb	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubusw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubusw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpsubusw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubusw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubusw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubusw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubusw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubusw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubusw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubusw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpsubusw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubusw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubusw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubusw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubusw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubusw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpsubw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsubw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpsubw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsubw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsubw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpckhbw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpckhbw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpunpckhbw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpckhbw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpckhbw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpunpckhbw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpckhbw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpunpckhbw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpckhbw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpckhbw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpunpckhbw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpckhbw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpckhbw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpunpckhbw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpckhbw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpunpckhbw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpckhwd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpckhwd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpunpckhwd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpckhwd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpckhwd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpunpckhwd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpckhwd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpunpckhwd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpckhwd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpckhwd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpunpckhwd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpckhwd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpckhwd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpunpckhwd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpckhwd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpunpckhwd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpcklbw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpcklbw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpunpcklbw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpcklbw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpcklbw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpunpcklbw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpcklbw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpunpcklbw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpcklbw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpcklbw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpunpcklbw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpcklbw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpcklbw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpunpcklbw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpcklbw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpunpcklbw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpcklwd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpcklwd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpunpcklwd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpcklwd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpcklwd	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpunpcklwd	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpcklwd	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpunpcklwd	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpunpcklwd	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpcklwd	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpunpcklwd	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpcklwd	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpcklwd	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpunpcklwd	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpunpcklwd	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpunpcklwd	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsllw	$0xab, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsllw	$0xab, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpsllw	$123, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsllw	$123, (%ecx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpsllw	$123, -123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BW,VL}
	vpsllw	$123, 2032(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsllw	$123, 2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpsllw	$123, -2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsllw	$123, -2064(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpsllw	$0xab, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsllw	$0xab, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpsllw	$123, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsllw	$123, (%ecx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpsllw	$123, -123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BW,VL}
	vpsllw	$123, 4064(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsllw	$123, 4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpsllw	$123, -4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsllw	$123, -4128(%edx), %ymm6{%k7}	 # AVX512{BW,VL}

	.intel_syntax noprefix
	vpabsb	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vpabsb	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vpabsb	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpabsb	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpabsb	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpabsb	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpabsb	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpabsb	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpabsb	ymm6{k7}, ymm5	 # AVX512{BW,VL}
	vpabsb	ymm6{k7}{z}, ymm5	 # AVX512{BW,VL}
	vpabsb	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpabsb	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpabsb	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpabsb	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpabsb	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpabsb	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpabsw	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vpabsw	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vpabsw	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpabsw	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpabsw	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpabsw	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpabsw	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpabsw	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpabsw	ymm6{k7}, ymm5	 # AVX512{BW,VL}
	vpabsw	ymm6{k7}{z}, ymm5	 # AVX512{BW,VL}
	vpabsw	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpabsw	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpabsw	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpabsw	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpabsw	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpabsw	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpacksswb	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpacksswb	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpacksswb	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpacksswb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpacksswb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpacksswb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpacksswb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpacksswb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpacksswb	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpacksswb	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpacksswb	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpacksswb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpacksswb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpacksswb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpacksswb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpacksswb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpackuswb	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpackuswb	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpackuswb	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpackuswb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpackuswb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpackuswb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpackuswb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpackuswb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpackuswb	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpackuswb	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpackuswb	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpackuswb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpackuswb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpackuswb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpackuswb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpackuswb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpaddb	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpaddb	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpaddb	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpaddb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpaddb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpaddb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpaddb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpaddb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpaddb	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpaddb	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpaddb	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpaddb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpaddb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpaddb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpaddb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpaddb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpaddsb	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpaddsb	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpaddsb	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpaddsb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpaddsb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpaddsb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpaddsb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpaddsb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpaddsb	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpaddsb	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpaddsb	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpaddsb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpaddsb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpaddsb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpaddsb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpaddsb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpaddsw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpaddsw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpaddsw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpaddsw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpaddsw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpaddsw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpaddsw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpaddsw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpaddsw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpaddsw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpaddsw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpaddsw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpaddsw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpaddsw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpaddsw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpaddsw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpaddusb	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpaddusb	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpaddusb	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpaddusb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpaddusb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpaddusb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpaddusb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpaddusb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpaddusb	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpaddusb	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpaddusb	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpaddusb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpaddusb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpaddusb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpaddusb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpaddusb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpaddusw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpaddusw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpaddusw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpaddusw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpaddusw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpaddusw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpaddusw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpaddusw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpaddusw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpaddusw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpaddusw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpaddusw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpaddusw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpaddusw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpaddusw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpaddusw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpaddw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpaddw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpaddw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpaddw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpaddw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpaddw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpaddw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpaddw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpaddw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpaddw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpaddw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpaddw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpaddw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpaddw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpaddw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpaddw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpalignr	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{BW,VL}
	vpalignr	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{BW,VL}
	vpalignr	xmm6{k7}, xmm5, xmm4, 123	 # AVX512{BW,VL}
	vpalignr	xmm6{k7}, xmm5, XMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpalignr	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpalignr	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{BW,VL} Disp8
	vpalignr	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048], 123	 # AVX512{BW,VL}
	vpalignr	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048], 123	 # AVX512{BW,VL} Disp8
	vpalignr	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064], 123	 # AVX512{BW,VL}
	vpalignr	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{BW,VL}
	vpalignr	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{BW,VL}
	vpalignr	ymm6{k7}, ymm5, ymm4, 123	 # AVX512{BW,VL}
	vpalignr	ymm6{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpalignr	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpalignr	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{BW,VL} Disp8
	vpalignr	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{BW,VL}
	vpalignr	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{BW,VL} Disp8
	vpalignr	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{BW,VL}
	vpavgb	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpavgb	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpavgb	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpavgb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpavgb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpavgb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpavgb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpavgb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpavgb	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpavgb	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpavgb	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpavgb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpavgb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpavgb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpavgb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpavgb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpavgw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpavgw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpavgw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpavgw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpavgw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpavgw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpavgw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpavgw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpavgw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpavgw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpavgw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpavgw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpavgw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpavgw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpavgw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpavgw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpcmpeqb	k5{k7}, xmm6, xmm5	 # AVX512{BW,VL}
	vpcmpeqb	k5{k7}, xmm6, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpcmpeqb	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpcmpeqb	k5{k7}, xmm6, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpcmpeqb	k5{k7}, xmm6, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpcmpeqb	k5{k7}, xmm6, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpcmpeqb	k5{k7}, xmm6, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpcmpeqb	k5{k7}, ymm6, ymm5	 # AVX512{BW,VL}
	vpcmpeqb	k5{k7}, ymm6, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpcmpeqb	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpcmpeqb	k5{k7}, ymm6, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpcmpeqb	k5{k7}, ymm6, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpcmpeqb	k5{k7}, ymm6, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpcmpeqb	k5{k7}, ymm6, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpcmpeqw	k5{k7}, xmm6, xmm5	 # AVX512{BW,VL}
	vpcmpeqw	k5{k7}, xmm6, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpcmpeqw	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpcmpeqw	k5{k7}, xmm6, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpcmpeqw	k5{k7}, xmm6, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpcmpeqw	k5{k7}, xmm6, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpcmpeqw	k5{k7}, xmm6, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpcmpeqw	k5{k7}, ymm6, ymm5	 # AVX512{BW,VL}
	vpcmpeqw	k5{k7}, ymm6, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpcmpeqw	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpcmpeqw	k5{k7}, ymm6, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpcmpeqw	k5{k7}, ymm6, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpcmpeqw	k5{k7}, ymm6, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpcmpeqw	k5{k7}, ymm6, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpcmpgtb	k5{k7}, xmm6, xmm5	 # AVX512{BW,VL}
	vpcmpgtb	k5{k7}, xmm6, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpcmpgtb	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpcmpgtb	k5{k7}, xmm6, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpcmpgtb	k5{k7}, xmm6, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpcmpgtb	k5{k7}, xmm6, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpcmpgtb	k5{k7}, xmm6, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpcmpgtb	k5{k7}, ymm6, ymm5	 # AVX512{BW,VL}
	vpcmpgtb	k5{k7}, ymm6, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpcmpgtb	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpcmpgtb	k5{k7}, ymm6, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpcmpgtb	k5{k7}, ymm6, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpcmpgtb	k5{k7}, ymm6, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpcmpgtb	k5{k7}, ymm6, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpcmpgtw	k5{k7}, xmm6, xmm5	 # AVX512{BW,VL}
	vpcmpgtw	k5{k7}, xmm6, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpcmpgtw	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpcmpgtw	k5{k7}, xmm6, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpcmpgtw	k5{k7}, xmm6, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpcmpgtw	k5{k7}, xmm6, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpcmpgtw	k5{k7}, xmm6, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpcmpgtw	k5{k7}, ymm6, ymm5	 # AVX512{BW,VL}
	vpcmpgtw	k5{k7}, ymm6, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpcmpgtw	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpcmpgtw	k5{k7}, ymm6, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpcmpgtw	k5{k7}, ymm6, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpcmpgtw	k5{k7}, ymm6, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpcmpgtw	k5{k7}, ymm6, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpmaddubsw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmaddubsw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmaddubsw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmaddubsw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmaddubsw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpmaddubsw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpmaddubsw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpmaddubsw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpmaddubsw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmaddubsw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmaddubsw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmaddubsw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmaddubsw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpmaddubsw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpmaddubsw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpmaddubsw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpmaddwd	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmaddwd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmaddwd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmaddwd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmaddwd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpmaddwd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpmaddwd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpmaddwd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpmaddwd	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmaddwd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmaddwd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmaddwd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmaddwd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpmaddwd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpmaddwd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpmaddwd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpmaxsb	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmaxsb	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmaxsb	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmaxsb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmaxsb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpmaxsb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpmaxsb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpmaxsb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpmaxsb	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmaxsb	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmaxsb	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmaxsb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmaxsb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpmaxsb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpmaxsb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpmaxsb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpmaxsw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmaxsw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmaxsw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmaxsw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmaxsw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpmaxsw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpmaxsw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpmaxsw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpmaxsw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmaxsw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmaxsw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmaxsw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmaxsw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpmaxsw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpmaxsw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpmaxsw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpmaxub	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmaxub	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmaxub	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmaxub	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmaxub	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpmaxub	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpmaxub	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpmaxub	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpmaxub	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmaxub	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmaxub	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmaxub	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmaxub	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpmaxub	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpmaxub	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpmaxub	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpmaxuw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmaxuw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmaxuw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmaxuw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmaxuw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpmaxuw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpmaxuw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpmaxuw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpmaxuw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmaxuw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmaxuw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmaxuw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmaxuw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpmaxuw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpmaxuw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpmaxuw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpminsb	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpminsb	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpminsb	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpminsb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpminsb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpminsb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpminsb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpminsb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpminsb	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpminsb	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpminsb	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpminsb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpminsb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpminsb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpminsb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpminsb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpminsw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpminsw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpminsw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpminsw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpminsw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpminsw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpminsw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpminsw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpminsw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpminsw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpminsw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpminsw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpminsw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpminsw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpminsw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpminsw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpminub	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpminub	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpminub	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpminub	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpminub	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpminub	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpminub	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpminub	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpminub	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpminub	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpminub	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpminub	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpminub	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpminub	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpminub	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpminub	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpminuw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpminuw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpminuw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpminuw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpminuw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpminuw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpminuw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpminuw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpminuw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpminuw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpminuw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpminuw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpminuw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpminuw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpminuw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpminuw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpmovsxbw	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vpmovsxbw	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vpmovsxbw	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmovsxbw	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmovsxbw	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{BW,VL} Disp8
	vpmovsxbw	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{BW,VL}
	vpmovsxbw	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{BW,VL} Disp8
	vpmovsxbw	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{BW,VL}
	vpmovsxbw	ymm6{k7}, xmm5	 # AVX512{BW,VL}
	vpmovsxbw	ymm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vpmovsxbw	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmovsxbw	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmovsxbw	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpmovsxbw	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpmovsxbw	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpmovsxbw	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpmovzxbw	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vpmovzxbw	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vpmovzxbw	xmm6{k7}, QWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmovzxbw	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmovzxbw	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512{BW,VL} Disp8
	vpmovzxbw	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512{BW,VL}
	vpmovzxbw	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512{BW,VL} Disp8
	vpmovzxbw	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512{BW,VL}
	vpmovzxbw	ymm6{k7}, xmm5	 # AVX512{BW,VL}
	vpmovzxbw	ymm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vpmovzxbw	ymm6{k7}, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmovzxbw	ymm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmovzxbw	ymm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpmovzxbw	ymm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpmovzxbw	ymm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpmovzxbw	ymm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpmulhrsw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmulhrsw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmulhrsw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmulhrsw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmulhrsw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpmulhrsw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpmulhrsw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpmulhrsw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpmulhrsw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmulhrsw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmulhrsw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmulhrsw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmulhrsw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpmulhrsw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpmulhrsw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpmulhrsw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpmulhuw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmulhuw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmulhuw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmulhuw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmulhuw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpmulhuw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpmulhuw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpmulhuw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpmulhuw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmulhuw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmulhuw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmulhuw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmulhuw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpmulhuw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpmulhuw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpmulhuw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpmulhw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmulhw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmulhw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmulhw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmulhw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpmulhw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpmulhw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpmulhw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpmulhw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmulhw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmulhw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmulhw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmulhw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpmulhw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpmulhw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpmulhw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpmullw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmullw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpmullw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmullw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmullw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpmullw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpmullw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpmullw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpmullw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmullw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpmullw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpmullw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpmullw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpmullw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpmullw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpmullw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpshufb	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpshufb	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpshufb	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpshufb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpshufb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpshufb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpshufb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpshufb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpshufb	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpshufb	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpshufb	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpshufb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpshufb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpshufb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpshufb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpshufb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpshufhw	xmm6{k7}, xmm5, 0xab	 # AVX512{BW,VL}
	vpshufhw	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{BW,VL}
	vpshufhw	xmm6{k7}, xmm5, 123	 # AVX512{BW,VL}
	vpshufhw	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpshufhw	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpshufhw	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{BW,VL} Disp8
	vpshufhw	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{BW,VL}
	vpshufhw	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{BW,VL} Disp8
	vpshufhw	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{BW,VL}
	vpshufhw	ymm6{k7}, ymm5, 0xab	 # AVX512{BW,VL}
	vpshufhw	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{BW,VL}
	vpshufhw	ymm6{k7}, ymm5, 123	 # AVX512{BW,VL}
	vpshufhw	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpshufhw	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpshufhw	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{BW,VL} Disp8
	vpshufhw	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{BW,VL}
	vpshufhw	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{BW,VL} Disp8
	vpshufhw	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{BW,VL}
	vpshuflw	xmm6{k7}, xmm5, 0xab	 # AVX512{BW,VL}
	vpshuflw	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{BW,VL}
	vpshuflw	xmm6{k7}, xmm5, 123	 # AVX512{BW,VL}
	vpshuflw	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpshuflw	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpshuflw	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{BW,VL} Disp8
	vpshuflw	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{BW,VL}
	vpshuflw	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{BW,VL} Disp8
	vpshuflw	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{BW,VL}
	vpshuflw	ymm6{k7}, ymm5, 0xab	 # AVX512{BW,VL}
	vpshuflw	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{BW,VL}
	vpshuflw	ymm6{k7}, ymm5, 123	 # AVX512{BW,VL}
	vpshuflw	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpshuflw	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpshuflw	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{BW,VL} Disp8
	vpshuflw	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{BW,VL}
	vpshuflw	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{BW,VL} Disp8
	vpshuflw	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{BW,VL}
	vpsllw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsllw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsllw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsllw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsllw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpsllw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpsllw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpsllw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpsllw	ymm6{k7}, ymm5, xmm4	 # AVX512{BW,VL}
	vpsllw	ymm6{k7}{z}, ymm5, xmm4	 # AVX512{BW,VL}
	vpsllw	ymm6{k7}, ymm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsllw	ymm6{k7}, ymm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsllw	ymm6{k7}, ymm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpsllw	ymm6{k7}, ymm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpsllw	ymm6{k7}, ymm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpsllw	ymm6{k7}, ymm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpsraw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsraw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsraw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsraw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsraw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpsraw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpsraw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpsraw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpsraw	ymm6{k7}, ymm5, xmm4	 # AVX512{BW,VL}
	vpsraw	ymm6{k7}{z}, ymm5, xmm4	 # AVX512{BW,VL}
	vpsraw	ymm6{k7}, ymm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsraw	ymm6{k7}, ymm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsraw	ymm6{k7}, ymm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpsraw	ymm6{k7}, ymm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpsraw	ymm6{k7}, ymm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpsraw	ymm6{k7}, ymm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpsrlw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsrlw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsrlw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsrlw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsrlw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpsrlw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpsrlw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpsrlw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpsrlw	ymm6{k7}, ymm5, xmm4	 # AVX512{BW,VL}
	vpsrlw	ymm6{k7}{z}, ymm5, xmm4	 # AVX512{BW,VL}
	vpsrlw	ymm6{k7}, ymm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsrlw	ymm6{k7}, ymm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsrlw	ymm6{k7}, ymm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpsrlw	ymm6{k7}, ymm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpsrlw	ymm6{k7}, ymm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpsrlw	ymm6{k7}, ymm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpsrlw	xmm6{k7}, xmm5, 0xab	 # AVX512{BW,VL}
	vpsrlw	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{BW,VL}
	vpsrlw	xmm6{k7}, xmm5, 123	 # AVX512{BW,VL}
	vpsrlw	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpsrlw	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpsrlw	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{BW,VL} Disp8
	vpsrlw	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{BW,VL}
	vpsrlw	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{BW,VL} Disp8
	vpsrlw	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{BW,VL}
	vpsrlw	ymm6{k7}, ymm5, 0xab	 # AVX512{BW,VL}
	vpsrlw	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{BW,VL}
	vpsrlw	ymm6{k7}, ymm5, 123	 # AVX512{BW,VL}
	vpsrlw	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpsrlw	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpsrlw	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{BW,VL} Disp8
	vpsrlw	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{BW,VL}
	vpsrlw	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{BW,VL} Disp8
	vpsrlw	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{BW,VL}
	vpsraw	xmm6{k7}, xmm5, 0xab	 # AVX512{BW,VL}
	vpsraw	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{BW,VL}
	vpsraw	xmm6{k7}, xmm5, 123	 # AVX512{BW,VL}
	vpsraw	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpsraw	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpsraw	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{BW,VL} Disp8
	vpsraw	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{BW,VL}
	vpsraw	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{BW,VL} Disp8
	vpsraw	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{BW,VL}
	vpsraw	ymm6{k7}, ymm5, 0xab	 # AVX512{BW,VL}
	vpsraw	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{BW,VL}
	vpsraw	ymm6{k7}, ymm5, 123	 # AVX512{BW,VL}
	vpsraw	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpsraw	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpsraw	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{BW,VL} Disp8
	vpsraw	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{BW,VL}
	vpsraw	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{BW,VL} Disp8
	vpsraw	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{BW,VL}
	vpsubb	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsubb	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsubb	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsubb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsubb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpsubb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpsubb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpsubb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpsubb	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsubb	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsubb	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsubb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsubb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpsubb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpsubb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpsubb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpsubsb	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsubsb	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsubsb	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsubsb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsubsb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpsubsb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpsubsb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpsubsb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpsubsb	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsubsb	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsubsb	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsubsb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsubsb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpsubsb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpsubsb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpsubsb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpsubsw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsubsw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsubsw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsubsw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsubsw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpsubsw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpsubsw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpsubsw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpsubsw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsubsw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsubsw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsubsw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsubsw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpsubsw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpsubsw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpsubsw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpsubusb	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsubusb	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsubusb	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsubusb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsubusb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpsubusb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpsubusb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpsubusb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpsubusb	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsubusb	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsubusb	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsubusb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsubusb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpsubusb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpsubusb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpsubusb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpsubusw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsubusw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsubusw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsubusw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsubusw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpsubusw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpsubusw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpsubusw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpsubusw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsubusw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsubusw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsubusw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsubusw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpsubusw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpsubusw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpsubusw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpsubw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsubw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsubw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsubw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsubw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpsubw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpsubw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpsubw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpsubw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsubw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsubw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsubw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsubw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpsubw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpsubw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpsubw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpunpckhbw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpunpckhbw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpunpckhbw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpunpckhbw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpunpckhbw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpunpckhbw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpunpckhbw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpunpckhbw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpunpckhbw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpunpckhbw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpunpckhbw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpunpckhbw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpunpckhbw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpunpckhbw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpunpckhbw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpunpckhbw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpunpckhwd	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpunpckhwd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpunpckhwd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpunpckhwd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpunpckhwd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpunpckhwd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpunpckhwd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpunpckhwd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpunpckhwd	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpunpckhwd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpunpckhwd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpunpckhwd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpunpckhwd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpunpckhwd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpunpckhwd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpunpckhwd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpunpcklbw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpunpcklbw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpunpcklbw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpunpcklbw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpunpcklbw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpunpcklbw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpunpcklbw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpunpcklbw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpunpcklbw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpunpcklbw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpunpcklbw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpunpcklbw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpunpcklbw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpunpcklbw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpunpcklbw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpunpcklbw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpunpcklwd	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpunpcklwd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpunpcklwd	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpunpcklwd	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpunpcklwd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpunpcklwd	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpunpcklwd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpunpcklwd	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpunpcklwd	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpunpcklwd	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpunpcklwd	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpunpcklwd	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpunpcklwd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpunpcklwd	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpunpcklwd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpunpcklwd	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpsllw	xmm6{k7}, xmm5, 0xab	 # AVX512{BW,VL}
	vpsllw	xmm6{k7}{z}, xmm5, 0xab	 # AVX512{BW,VL}
	vpsllw	xmm6{k7}, xmm5, 123	 # AVX512{BW,VL}
	vpsllw	xmm6{k7}, XMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpsllw	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpsllw	xmm6{k7}, XMMWORD PTR [edx+2032], 123	 # AVX512{BW,VL} Disp8
	vpsllw	xmm6{k7}, XMMWORD PTR [edx+2048], 123	 # AVX512{BW,VL}
	vpsllw	xmm6{k7}, XMMWORD PTR [edx-2048], 123	 # AVX512{BW,VL} Disp8
	vpsllw	xmm6{k7}, XMMWORD PTR [edx-2064], 123	 # AVX512{BW,VL}
	vpsllw	ymm6{k7}, ymm5, 0xab	 # AVX512{BW,VL}
	vpsllw	ymm6{k7}{z}, ymm5, 0xab	 # AVX512{BW,VL}
	vpsllw	ymm6{k7}, ymm5, 123	 # AVX512{BW,VL}
	vpsllw	ymm6{k7}, YMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpsllw	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpsllw	ymm6{k7}, YMMWORD PTR [edx+4064], 123	 # AVX512{BW,VL} Disp8
	vpsllw	ymm6{k7}, YMMWORD PTR [edx+4096], 123	 # AVX512{BW,VL}
	vpsllw	ymm6{k7}, YMMWORD PTR [edx-4096], 123	 # AVX512{BW,VL} Disp8
	vpsllw	ymm6{k7}, YMMWORD PTR [edx-4128], 123	 # AVX512{BW,VL}
