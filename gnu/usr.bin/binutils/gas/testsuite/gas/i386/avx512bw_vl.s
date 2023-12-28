# Check 32bit AVX512{BW,VL} instructions

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
	vpackssdw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackssdw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpackssdw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackssdw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackssdw	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackssdw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackssdw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackssdw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackssdw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackssdw	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackssdw	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackssdw	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackssdw	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackssdw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackssdw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpackssdw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackssdw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackssdw	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackssdw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackssdw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackssdw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackssdw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackssdw	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackssdw	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackssdw	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackssdw	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
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
	vpackusdw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackusdw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpackusdw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackusdw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackusdw	(%eax){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackusdw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackusdw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackusdw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackusdw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackusdw	508(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackusdw	512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackusdw	-512(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackusdw	-516(%edx){1to4}, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpackusdw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackusdw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpackusdw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackusdw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackusdw	(%eax){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackusdw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackusdw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackusdw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackusdw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackusdw	508(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackusdw	512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpackusdw	-512(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpackusdw	-516(%edx){1to8}, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
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
	vpblendmb	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpblendmb	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpblendmb	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpblendmb	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpblendmb	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpblendmb	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpblendmb	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpblendmb	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpblendmb	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpblendmb	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpblendmb	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpblendmb	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpblendmb	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpblendmb	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpblendmb	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpblendmb	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastb	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastb	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpbroadcastb	(%ecx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastb	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastb	127(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpbroadcastb	128(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastb	-128(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpbroadcastb	-129(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastb	%xmm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastb	%xmm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpbroadcastb	(%ecx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastb	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastb	127(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpbroadcastb	128(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastb	-128(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpbroadcastb	-129(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastb	%eax, %xmm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastb	%eax, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpbroadcastb	%ebp, %xmm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastb	%eax, %ymm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastb	%eax, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpbroadcastb	%ebp, %ymm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastw	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastw	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpbroadcastw	(%ecx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastw	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastw	254(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpbroadcastw	256(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastw	-256(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpbroadcastw	-258(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastw	%xmm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastw	%xmm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpbroadcastw	(%ecx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastw	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastw	254(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpbroadcastw	256(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastw	-256(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpbroadcastw	-258(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastw	%eax, %xmm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastw	%eax, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpbroadcastw	%ebp, %xmm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastw	%eax, %ymm6{%k7}	 # AVX512{BW,VL}
	vpbroadcastw	%eax, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpbroadcastw	%ebp, %ymm6{%k7}	 # AVX512{BW,VL}
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
	vpblendmw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpblendmw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpblendmw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpblendmw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpblendmw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpblendmw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpblendmw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpblendmw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpblendmw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpblendmw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpblendmw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpblendmw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpblendmw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpblendmw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpblendmw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpblendmw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
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
	vpsrlvw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsrlvw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpsrlvw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsrlvw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsrlvw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsrlvw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsrlvw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsrlvw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsrlvw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsrlvw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpsrlvw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsrlvw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsrlvw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsrlvw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsrlvw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsrlvw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsravw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsravw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpsravw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsravw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsravw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsravw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsravw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsravw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsravw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsravw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpsravw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsravw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsravw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsravw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsravw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsravw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
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
	vpmovwb	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmovwb	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmovwb	%ymm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmovwb	%ymm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmovswb	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmovswb	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmovswb	%ymm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmovswb	%ymm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmovuswb	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmovuswb	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpmovuswb	%ymm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpmovuswb	%ymm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vdbpsadbw	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vdbpsadbw	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vdbpsadbw	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vdbpsadbw	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vdbpsadbw	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vdbpsadbw	$123, 2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vdbpsadbw	$123, 2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vdbpsadbw	$123, -2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vdbpsadbw	$123, -2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vdbpsadbw	$0xab, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vdbpsadbw	$0xab, %ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vdbpsadbw	$123, %ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vdbpsadbw	$123, (%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vdbpsadbw	$123, -123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vdbpsadbw	$123, 4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vdbpsadbw	$123, 4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vdbpsadbw	$123, -4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vdbpsadbw	$123, -4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpermw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpermw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpermw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpermw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpermw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpermw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpermw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpermw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpermw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpermw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpermw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpermw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpermw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpermw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpermw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpermw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpermt2w	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpermt2w	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpermt2w	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpermt2w	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpermt2w	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpermt2w	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpermt2w	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpermt2w	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpermt2w	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpermt2w	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpermt2w	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpermt2w	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpermt2w	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpermt2w	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpermt2w	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpermt2w	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
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
	vpsllvw	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsllvw	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpsllvw	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsllvw	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsllvw	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsllvw	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsllvw	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsllvw	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpsllvw	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsllvw	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpsllvw	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsllvw	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsllvw	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsllvw	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpsllvw	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpsllvw	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu8	(%ecx), %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8	2032(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vmovdqu8	2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8	-2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vmovdqu8	-2064(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8	%ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8	%ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu8	(%ecx), %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8	4064(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vmovdqu8	4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu8	-4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vmovdqu8	-4128(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16	%xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16	%xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu16	(%ecx), %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16	2032(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vmovdqu16	2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16	-2048(%edx), %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vmovdqu16	-2064(%edx), %xmm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16	%ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16	%ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vmovdqu16	(%ecx), %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16	-123456(%esp,%esi,8), %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16	4064(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vmovdqu16	4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vmovdqu16	-4096(%edx), %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vmovdqu16	-4128(%edx), %ymm6{%k7}	 # AVX512{BW,VL}
	vpmovwb	%xmm6, (%ecx){%k7}	 # AVX512{BW,VL}
	vpmovwb	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{BW,VL}
	vpmovwb	%xmm6, 1016(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vpmovwb	%xmm6, 1024(%edx){%k7}	 # AVX512{BW,VL}
	vpmovwb	%xmm6, -1024(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vpmovwb	%xmm6, -1032(%edx){%k7}	 # AVX512{BW,VL}
	vpmovwb	%ymm6, (%ecx){%k7}	 # AVX512{BW,VL}
	vpmovwb	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{BW,VL}
	vpmovwb	%ymm6, 2032(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vpmovwb	%ymm6, 2048(%edx){%k7}	 # AVX512{BW,VL}
	vpmovwb	%ymm6, -2048(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vpmovwb	%ymm6, -2064(%edx){%k7}	 # AVX512{BW,VL}
	vpmovswb	%xmm6, (%ecx){%k7}	 # AVX512{BW,VL}
	vpmovswb	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{BW,VL}
	vpmovswb	%xmm6, 1016(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vpmovswb	%xmm6, 1024(%edx){%k7}	 # AVX512{BW,VL}
	vpmovswb	%xmm6, -1024(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vpmovswb	%xmm6, -1032(%edx){%k7}	 # AVX512{BW,VL}
	vpmovswb	%ymm6, (%ecx){%k7}	 # AVX512{BW,VL}
	vpmovswb	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{BW,VL}
	vpmovswb	%ymm6, 2032(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vpmovswb	%ymm6, 2048(%edx){%k7}	 # AVX512{BW,VL}
	vpmovswb	%ymm6, -2048(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vpmovswb	%ymm6, -2064(%edx){%k7}	 # AVX512{BW,VL}
	vpmovuswb	%xmm6, (%ecx){%k7}	 # AVX512{BW,VL}
	vpmovuswb	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{BW,VL}
	vpmovuswb	%xmm6, 1016(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vpmovuswb	%xmm6, 1024(%edx){%k7}	 # AVX512{BW,VL}
	vpmovuswb	%xmm6, -1024(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vpmovuswb	%xmm6, -1032(%edx){%k7}	 # AVX512{BW,VL}
	vpmovuswb	%ymm6, (%ecx){%k7}	 # AVX512{BW,VL}
	vpmovuswb	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{BW,VL}
	vpmovuswb	%ymm6, 2032(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vpmovuswb	%ymm6, 2048(%edx){%k7}	 # AVX512{BW,VL}
	vpmovuswb	%ymm6, -2048(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vpmovuswb	%ymm6, -2064(%edx){%k7}	 # AVX512{BW,VL}
	vmovdqu8	%xmm6, (%ecx){%k7}	 # AVX512{BW,VL}
	vmovdqu8	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{BW,VL}
	vmovdqu8	%xmm6, 2032(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vmovdqu8	%xmm6, 2048(%edx){%k7}	 # AVX512{BW,VL}
	vmovdqu8	%xmm6, -2048(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vmovdqu8	%xmm6, -2064(%edx){%k7}	 # AVX512{BW,VL}
	vmovdqu8	%ymm6, (%ecx){%k7}	 # AVX512{BW,VL}
	vmovdqu8	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{BW,VL}
	vmovdqu8	%ymm6, 4064(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vmovdqu8	%ymm6, 4096(%edx){%k7}	 # AVX512{BW,VL}
	vmovdqu8	%ymm6, -4096(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vmovdqu8	%ymm6, -4128(%edx){%k7}	 # AVX512{BW,VL}
	vmovdqu16	%xmm6, (%ecx){%k7}	 # AVX512{BW,VL}
	vmovdqu16	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512{BW,VL}
	vmovdqu16	%xmm6, 2032(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vmovdqu16	%xmm6, 2048(%edx){%k7}	 # AVX512{BW,VL}
	vmovdqu16	%xmm6, -2048(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vmovdqu16	%xmm6, -2064(%edx){%k7}	 # AVX512{BW,VL}
	vmovdqu16	%ymm6, (%ecx){%k7}	 # AVX512{BW,VL}
	vmovdqu16	%ymm6, -123456(%esp,%esi,8){%k7}	 # AVX512{BW,VL}
	vmovdqu16	%ymm6, 4064(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vmovdqu16	%ymm6, 4096(%edx){%k7}	 # AVX512{BW,VL}
	vmovdqu16	%ymm6, -4096(%edx){%k7}	 # AVX512{BW,VL} Disp8
	vmovdqu16	%ymm6, -4128(%edx){%k7}	 # AVX512{BW,VL}
	vpermi2w	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpermi2w	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512{BW,VL}
	vpermi2w	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpermi2w	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpermi2w	2032(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpermi2w	2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpermi2w	-2048(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL} Disp8
	vpermi2w	-2064(%edx), %xmm5, %xmm6{%k7}	 # AVX512{BW,VL}
	vpermi2w	%ymm4, %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpermi2w	%ymm4, %ymm5, %ymm6{%k7}{z}	 # AVX512{BW,VL}
	vpermi2w	(%ecx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpermi2w	-123456(%esp,%esi,8), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpermi2w	4064(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpermi2w	4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vpermi2w	-4096(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL} Disp8
	vpermi2w	-4128(%edx), %ymm5, %ymm6{%k7}	 # AVX512{BW,VL}
	vptestmb	%xmm5, %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmb	(%ecx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmb	-123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmb	2032(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vptestmb	2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmb	-2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vptestmb	-2064(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmb	%ymm5, %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmb	(%ecx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmb	-123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmb	4064(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vptestmb	4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmb	-4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vptestmb	-4128(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmw	%xmm5, %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmw	(%ecx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmw	-123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmw	2032(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vptestmw	2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmw	-2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vptestmw	-2064(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmw	%ymm5, %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmw	(%ecx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmw	-123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmw	4064(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vptestmw	4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vptestmw	-4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vptestmw	-4128(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpmovb2m	%xmm6, %k5	 # AVX512{BW,VL}
	vpmovb2m	%ymm6, %k5	 # AVX512{BW,VL}
	vpmovw2m	%xmm6, %k5	 # AVX512{BW,VL}
	vpmovw2m	%ymm6, %k5	 # AVX512{BW,VL}
	vpmovm2b	%k5, %xmm6	 # AVX512{BW,VL}
	vpmovm2b	%k5, %ymm6	 # AVX512{BW,VL}
	vpmovm2w	%k5, %xmm6	 # AVX512{BW,VL}
	vpmovm2w	%k5, %ymm6	 # AVX512{BW,VL}
	vptestnmb	%xmm4, %xmm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmb	(%ecx), %xmm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmb	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmb	2032(%edx), %xmm5, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vptestnmb	2048(%edx), %xmm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmb	-2048(%edx), %xmm5, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vptestnmb	-2064(%edx), %xmm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmb	%ymm4, %ymm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmb	(%ecx), %ymm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmb	-123456(%esp,%esi,8), %ymm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmb	4064(%edx), %ymm5, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vptestnmb	4096(%edx), %ymm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmb	-4096(%edx), %ymm5, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vptestnmb	-4128(%edx), %ymm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmw	%xmm4, %xmm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmw	(%ecx), %xmm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmw	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmw	2032(%edx), %xmm5, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vptestnmw	2048(%edx), %xmm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmw	-2048(%edx), %xmm5, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vptestnmw	-2064(%edx), %xmm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmw	%ymm4, %ymm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmw	(%ecx), %ymm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmw	-123456(%esp,%esi,8), %ymm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmw	4064(%edx), %ymm5, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vptestnmw	4096(%edx), %ymm5, %k5{%k7}	 # AVX512{BW,VL}
	vptestnmw	-4096(%edx), %ymm5, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vptestnmw	-4128(%edx), %ymm5, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpb	$0xab, %xmm5, %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpb	$123, %xmm5, %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpb	$123, (%ecx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpb	$123, -123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpb	$123, 2032(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpb	$123, 2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpb	$123, -2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpb	$123, -2064(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpb	$0xab, %ymm5, %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpb	$123, %ymm5, %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpb	$123, (%ecx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpb	$123, -123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpb	$123, 4064(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpb	$123, 4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpb	$123, -4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpb	$123, -4128(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpb	$0, %xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpb	$0, %ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpleb	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpleb	0x7f0(%eax), %xmm6, %k5	 # AVX512{BW,VL} Disp8
	vpcmpleb	0x800(%eax), %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpleb	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpleb	0xfe0(%eax), %ymm6, %k5	 # AVX512{BW,VL} Disp8
	vpcmpleb	0x1000(%eax), %ymm6, %k5 # AVX512{BW,VL}
	vpcmpltb	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpltb	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpneqb	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpneqb	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpnleb	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpnleb	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpnltb	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpnltb	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpw	$0xab, %xmm5, %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpw	$123, %xmm5, %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpw	$123, (%ecx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpw	$123, -123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpw	$123, 2032(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpw	$123, 2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpw	$123, -2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpw	$123, -2064(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpw	$0xab, %ymm5, %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpw	$123, %ymm5, %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpw	$123, (%ecx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpw	$123, -123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpw	$123, 4064(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpw	$123, 4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpw	$123, -4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpw	$123, -4128(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpw	$0, %xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpw	$0, %ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmplew	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmplew	0x7f0(%eax), %xmm6, %k5	 # AVX512{BW,VL} Disp8
	vpcmplew	0x800(%eax), %xmm6, %k5	 # AVX512{BW,VL}
	vpcmplew	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmplew	0xfe0(%eax), %ymm6, %k5	 # AVX512{BW,VL} Disp8
	vpcmplew	0x1000(%eax), %ymm6, %k5 # AVX512{BW,VL}
	vpcmpltw	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpltw	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpneqw	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpneqw	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpnlew	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpnlew	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpnltw	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpnltw	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpub	$0xab, %xmm5, %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpub	$123, %xmm5, %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpub	$123, (%ecx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpub	$123, -123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpub	$123, 2032(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpub	$123, 2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpub	$123, -2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpub	$123, -2064(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpub	$0xab, %ymm5, %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpub	$123, %ymm5, %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpub	$123, (%ecx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpub	$123, -123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpub	$123, 4064(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpub	$123, 4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpub	$123, -4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpub	$123, -4128(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpub	$0, %xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpub	$0, %ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpleub	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpleub	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpltub	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpltub	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpnequb	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpnequb	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpnleub	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpnleub	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpnltub	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpnltub	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpuw	$0xab, %xmm5, %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpuw	$123, %xmm5, %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpuw	$123, (%ecx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpuw	$123, -123456(%esp,%esi,8), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpuw	$123, 2032(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpuw	$123, 2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpuw	$123, -2048(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpuw	$123, -2064(%edx), %xmm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpuw	$0xab, %ymm5, %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpuw	$123, %ymm5, %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpuw	$123, (%ecx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpuw	$123, -123456(%esp,%esi,8), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpuw	$123, 4064(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpuw	$123, 4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpuw	$123, -4096(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL} Disp8
	vpcmpuw	$123, -4128(%edx), %ymm6, %k5{%k7}	 # AVX512{BW,VL}
	vpcmpuw	$0, %xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpuw	$0, %ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpleuw	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpleuw	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpltuw	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpltuw	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpnequw	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpnequw	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpnleuw	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpnleuw	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}
	vpcmpnltuw	%xmm5, %xmm6, %k5	 # AVX512{BW,VL}
	vpcmpnltuw	%ymm5, %ymm6, %k5	 # AVX512{BW,VL}

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
	vpackssdw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpackssdw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpackssdw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpackssdw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpackssdw	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{BW,VL}
	vpackssdw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpackssdw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpackssdw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpackssdw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpackssdw	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{BW,VL} Disp8
	vpackssdw	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{BW,VL}
	vpackssdw	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{BW,VL} Disp8
	vpackssdw	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{BW,VL}
	vpackssdw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpackssdw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpackssdw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpackssdw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpackssdw	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{BW,VL}
	vpackssdw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpackssdw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpackssdw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpackssdw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpackssdw	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{BW,VL} Disp8
	vpackssdw	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{BW,VL}
	vpackssdw	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{BW,VL} Disp8
	vpackssdw	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{BW,VL}
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
	vpackusdw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpackusdw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpackusdw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpackusdw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpackusdw	xmm6{k7}, xmm5, [eax]{1to4}	 # AVX512{BW,VL}
	vpackusdw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpackusdw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpackusdw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpackusdw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpackusdw	xmm6{k7}, xmm5, [edx+508]{1to4}	 # AVX512{BW,VL} Disp8
	vpackusdw	xmm6{k7}, xmm5, [edx+512]{1to4}	 # AVX512{BW,VL}
	vpackusdw	xmm6{k7}, xmm5, [edx-512]{1to4}	 # AVX512{BW,VL} Disp8
	vpackusdw	xmm6{k7}, xmm5, [edx-516]{1to4}	 # AVX512{BW,VL}
	vpackusdw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpackusdw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpackusdw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpackusdw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpackusdw	ymm6{k7}, ymm5, [eax]{1to8}	 # AVX512{BW,VL}
	vpackusdw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpackusdw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpackusdw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpackusdw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpackusdw	ymm6{k7}, ymm5, [edx+508]{1to8}	 # AVX512{BW,VL} Disp8
	vpackusdw	ymm6{k7}, ymm5, [edx+512]{1to8}	 # AVX512{BW,VL}
	vpackusdw	ymm6{k7}, ymm5, [edx-512]{1to8}	 # AVX512{BW,VL} Disp8
	vpackusdw	ymm6{k7}, ymm5, [edx-516]{1to8}	 # AVX512{BW,VL}
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
	vpblendmb	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpblendmb	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpblendmb	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpblendmb	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpblendmb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpblendmb	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpblendmb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpblendmb	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpblendmb	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpblendmb	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpblendmb	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpblendmb	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpblendmb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpblendmb	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpblendmb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpblendmb	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpbroadcastb	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vpbroadcastb	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vpbroadcastb	xmm6{k7}, BYTE PTR [ecx]	 # AVX512{BW,VL}
	vpbroadcastb	xmm6{k7}, BYTE PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpbroadcastb	xmm6{k7}, BYTE PTR [edx+127]	 # AVX512{BW,VL} Disp8
	vpbroadcastb	xmm6{k7}, BYTE PTR [edx+128]	 # AVX512{BW,VL}
	vpbroadcastb	xmm6{k7}, BYTE PTR [edx-128]	 # AVX512{BW,VL} Disp8
	vpbroadcastb	xmm6{k7}, BYTE PTR [edx-129]	 # AVX512{BW,VL}
	vpbroadcastb	ymm6{k7}, xmm5	 # AVX512{BW,VL}
	vpbroadcastb	ymm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vpbroadcastb	ymm6{k7}, BYTE PTR [ecx]	 # AVX512{BW,VL}
	vpbroadcastb	ymm6{k7}, BYTE PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpbroadcastb	ymm6{k7}, BYTE PTR [edx+127]	 # AVX512{BW,VL} Disp8
	vpbroadcastb	ymm6{k7}, BYTE PTR [edx+128]	 # AVX512{BW,VL}
	vpbroadcastb	ymm6{k7}, BYTE PTR [edx-128]	 # AVX512{BW,VL} Disp8
	vpbroadcastb	ymm6{k7}, BYTE PTR [edx-129]	 # AVX512{BW,VL}
	vpbroadcastb	xmm6{k7}, eax	 # AVX512{BW,VL}
	vpbroadcastb	xmm6{k7}{z}, eax	 # AVX512{BW,VL}
	vpbroadcastb	xmm6{k7}, ebp	 # AVX512{BW,VL}
	vpbroadcastb	ymm6{k7}, eax	 # AVX512{BW,VL}
	vpbroadcastb	ymm6{k7}{z}, eax	 # AVX512{BW,VL}
	vpbroadcastb	ymm6{k7}, ebp	 # AVX512{BW,VL}
	vpbroadcastw	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vpbroadcastw	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vpbroadcastw	xmm6{k7}, WORD PTR [ecx]	 # AVX512{BW,VL}
	vpbroadcastw	xmm6{k7}, WORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpbroadcastw	xmm6{k7}, WORD PTR [edx+254]	 # AVX512{BW,VL} Disp8
	vpbroadcastw	xmm6{k7}, WORD PTR [edx+256]	 # AVX512{BW,VL}
	vpbroadcastw	xmm6{k7}, WORD PTR [edx-256]	 # AVX512{BW,VL} Disp8
	vpbroadcastw	xmm6{k7}, WORD PTR [edx-258]	 # AVX512{BW,VL}
	vpbroadcastw	ymm6{k7}, xmm5	 # AVX512{BW,VL}
	vpbroadcastw	ymm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vpbroadcastw	ymm6{k7}, WORD PTR [ecx]	 # AVX512{BW,VL}
	vpbroadcastw	ymm6{k7}, WORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpbroadcastw	ymm6{k7}, WORD PTR [edx+254]	 # AVX512{BW,VL} Disp8
	vpbroadcastw	ymm6{k7}, WORD PTR [edx+256]	 # AVX512{BW,VL}
	vpbroadcastw	ymm6{k7}, WORD PTR [edx-256]	 # AVX512{BW,VL} Disp8
	vpbroadcastw	ymm6{k7}, WORD PTR [edx-258]	 # AVX512{BW,VL}
	vpbroadcastw	xmm6{k7}, eax	 # AVX512{BW,VL}
	vpbroadcastw	xmm6{k7}{z}, eax	 # AVX512{BW,VL}
	vpbroadcastw	xmm6{k7}, ebp	 # AVX512{BW,VL}
	vpbroadcastw	ymm6{k7}, eax	 # AVX512{BW,VL}
	vpbroadcastw	ymm6{k7}{z}, eax	 # AVX512{BW,VL}
	vpbroadcastw	ymm6{k7}, ebp	 # AVX512{BW,VL}
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
	vpblendmw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpblendmw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpblendmw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpblendmw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpblendmw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpblendmw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpblendmw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpblendmw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpblendmw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpblendmw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpblendmw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpblendmw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpblendmw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpblendmw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpblendmw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpblendmw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
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
	vpsrlvw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsrlvw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsrlvw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsrlvw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsrlvw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpsrlvw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpsrlvw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpsrlvw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpsrlvw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsrlvw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsrlvw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsrlvw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsrlvw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpsrlvw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpsrlvw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpsrlvw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpsravw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsravw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsravw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsravw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsravw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpsravw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpsravw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpsravw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpsravw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsravw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsravw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsravw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsravw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpsravw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpsravw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpsravw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
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
	vpmovwb	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vpmovwb	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vpmovwb	xmm6{k7}, ymm5	 # AVX512{BW,VL}
	vpmovwb	xmm6{k7}{z}, ymm5	 # AVX512{BW,VL}
	vpmovswb	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vpmovswb	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vpmovswb	xmm6{k7}, ymm5	 # AVX512{BW,VL}
	vpmovswb	xmm6{k7}{z}, ymm5	 # AVX512{BW,VL}
	vpmovuswb	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vpmovuswb	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vpmovuswb	xmm6{k7}, ymm5	 # AVX512{BW,VL}
	vpmovuswb	xmm6{k7}{z}, ymm5	 # AVX512{BW,VL}
	vdbpsadbw	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512{BW,VL}
	vdbpsadbw	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512{BW,VL}
	vdbpsadbw	xmm6{k7}, xmm5, xmm4, 123	 # AVX512{BW,VL}
	vdbpsadbw	xmm6{k7}, xmm5, XMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vdbpsadbw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vdbpsadbw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032], 123	 # AVX512{BW,VL} Disp8
	vdbpsadbw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048], 123	 # AVX512{BW,VL}
	vdbpsadbw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048], 123	 # AVX512{BW,VL} Disp8
	vdbpsadbw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064], 123	 # AVX512{BW,VL}
	vdbpsadbw	ymm6{k7}, ymm5, ymm4, 0xab	 # AVX512{BW,VL}
	vdbpsadbw	ymm6{k7}{z}, ymm5, ymm4, 0xab	 # AVX512{BW,VL}
	vdbpsadbw	ymm6{k7}, ymm5, ymm4, 123	 # AVX512{BW,VL}
	vdbpsadbw	ymm6{k7}, ymm5, YMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vdbpsadbw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vdbpsadbw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064], 123	 # AVX512{BW,VL} Disp8
	vdbpsadbw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096], 123	 # AVX512{BW,VL}
	vdbpsadbw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096], 123	 # AVX512{BW,VL} Disp8
	vdbpsadbw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128], 123	 # AVX512{BW,VL}
	vpermw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpermw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpermw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpermw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpermw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpermw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpermw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpermw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpermw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpermw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpermw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpermw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpermw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpermw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpermw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpermw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpermt2w	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpermt2w	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpermt2w	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpermt2w	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpermt2w	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpermt2w	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpermt2w	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpermt2w	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpermt2w	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpermt2w	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpermt2w	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpermt2w	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpermt2w	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpermt2w	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpermt2w	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpermt2w	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
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
	vpsllvw	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsllvw	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpsllvw	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsllvw	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsllvw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpsllvw	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpsllvw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpsllvw	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpsllvw	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsllvw	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpsllvw	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpsllvw	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpsllvw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpsllvw	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpsllvw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpsllvw	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vmovdqu8	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vmovdqu8	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vmovdqu8	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vmovdqu8	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vmovdqu8	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vmovdqu8	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vmovdqu8	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vmovdqu8	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vmovdqu8	ymm6{k7}, ymm5	 # AVX512{BW,VL}
	vmovdqu8	ymm6{k7}{z}, ymm5	 # AVX512{BW,VL}
	vmovdqu8	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vmovdqu8	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vmovdqu8	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vmovdqu8	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vmovdqu8	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vmovdqu8	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vmovdqu16	xmm6{k7}, xmm5	 # AVX512{BW,VL}
	vmovdqu16	xmm6{k7}{z}, xmm5	 # AVX512{BW,VL}
	vmovdqu16	xmm6{k7}, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vmovdqu16	xmm6{k7}, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vmovdqu16	xmm6{k7}, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vmovdqu16	xmm6{k7}, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vmovdqu16	xmm6{k7}, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vmovdqu16	xmm6{k7}, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vmovdqu16	ymm6{k7}, ymm5	 # AVX512{BW,VL}
	vmovdqu16	ymm6{k7}{z}, ymm5	 # AVX512{BW,VL}
	vmovdqu16	ymm6{k7}, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vmovdqu16	ymm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vmovdqu16	ymm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vmovdqu16	ymm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vmovdqu16	ymm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vmovdqu16	ymm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpmovwb	QWORD PTR [ecx]{k7}, xmm6	 # AVX512{BW,VL}
	vpmovwb	QWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{BW,VL}
	vpmovwb	QWORD PTR [edx+1016]{k7}, xmm6	 # AVX512{BW,VL} Disp8
	vpmovwb	QWORD PTR [edx+1024]{k7}, xmm6	 # AVX512{BW,VL}
	vpmovwb	QWORD PTR [edx-1024]{k7}, xmm6	 # AVX512{BW,VL} Disp8
	vpmovwb	QWORD PTR [edx-1032]{k7}, xmm6	 # AVX512{BW,VL}
	vpmovwb	XMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{BW,VL}
	vpmovwb	XMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{BW,VL}
	vpmovwb	XMMWORD PTR [edx+2032]{k7}, ymm6	 # AVX512{BW,VL} Disp8
	vpmovwb	XMMWORD PTR [edx+2048]{k7}, ymm6	 # AVX512{BW,VL}
	vpmovwb	XMMWORD PTR [edx-2048]{k7}, ymm6	 # AVX512{BW,VL} Disp8
	vpmovwb	XMMWORD PTR [edx-2064]{k7}, ymm6	 # AVX512{BW,VL}
	vpmovswb	QWORD PTR [ecx]{k7}, xmm6	 # AVX512{BW,VL}
	vpmovswb	QWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{BW,VL}
	vpmovswb	QWORD PTR [edx+1016]{k7}, xmm6	 # AVX512{BW,VL} Disp8
	vpmovswb	QWORD PTR [edx+1024]{k7}, xmm6	 # AVX512{BW,VL}
	vpmovswb	QWORD PTR [edx-1024]{k7}, xmm6	 # AVX512{BW,VL} Disp8
	vpmovswb	QWORD PTR [edx-1032]{k7}, xmm6	 # AVX512{BW,VL}
	vpmovswb	XMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{BW,VL}
	vpmovswb	XMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{BW,VL}
	vpmovswb	XMMWORD PTR [edx+2032]{k7}, ymm6	 # AVX512{BW,VL} Disp8
	vpmovswb	XMMWORD PTR [edx+2048]{k7}, ymm6	 # AVX512{BW,VL}
	vpmovswb	XMMWORD PTR [edx-2048]{k7}, ymm6	 # AVX512{BW,VL} Disp8
	vpmovswb	XMMWORD PTR [edx-2064]{k7}, ymm6	 # AVX512{BW,VL}
	vpmovuswb	QWORD PTR [ecx]{k7}, xmm6	 # AVX512{BW,VL}
	vpmovuswb	QWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{BW,VL}
	vpmovuswb	QWORD PTR [edx+1016]{k7}, xmm6	 # AVX512{BW,VL} Disp8
	vpmovuswb	QWORD PTR [edx+1024]{k7}, xmm6	 # AVX512{BW,VL}
	vpmovuswb	QWORD PTR [edx-1024]{k7}, xmm6	 # AVX512{BW,VL} Disp8
	vpmovuswb	QWORD PTR [edx-1032]{k7}, xmm6	 # AVX512{BW,VL}
	vpmovuswb	XMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{BW,VL}
	vpmovuswb	XMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{BW,VL}
	vpmovuswb	XMMWORD PTR [edx+2032]{k7}, ymm6	 # AVX512{BW,VL} Disp8
	vpmovuswb	XMMWORD PTR [edx+2048]{k7}, ymm6	 # AVX512{BW,VL}
	vpmovuswb	XMMWORD PTR [edx-2048]{k7}, ymm6	 # AVX512{BW,VL} Disp8
	vpmovuswb	XMMWORD PTR [edx-2064]{k7}, ymm6	 # AVX512{BW,VL}
	vmovdqu8	XMMWORD PTR [ecx]{k7}, xmm6	 # AVX512{BW,VL}
	vmovdqu8	XMMWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{BW,VL}
	vmovdqu8	XMMWORD PTR [edx+2032]{k7}, xmm6	 # AVX512{BW,VL} Disp8
	vmovdqu8	XMMWORD PTR [edx+2048]{k7}, xmm6	 # AVX512{BW,VL}
	vmovdqu8	XMMWORD PTR [edx-2048]{k7}, xmm6	 # AVX512{BW,VL} Disp8
	vmovdqu8	XMMWORD PTR [edx-2064]{k7}, xmm6	 # AVX512{BW,VL}
	vmovdqu8	YMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{BW,VL}
	vmovdqu8	YMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{BW,VL}
	vmovdqu8	YMMWORD PTR [edx+4064]{k7}, ymm6	 # AVX512{BW,VL} Disp8
	vmovdqu8	YMMWORD PTR [edx+4096]{k7}, ymm6	 # AVX512{BW,VL}
	vmovdqu8	YMMWORD PTR [edx-4096]{k7}, ymm6	 # AVX512{BW,VL} Disp8
	vmovdqu8	YMMWORD PTR [edx-4128]{k7}, ymm6	 # AVX512{BW,VL}
	vmovdqu16	XMMWORD PTR [ecx]{k7}, xmm6	 # AVX512{BW,VL}
	vmovdqu16	XMMWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512{BW,VL}
	vmovdqu16	XMMWORD PTR [edx+2032]{k7}, xmm6	 # AVX512{BW,VL} Disp8
	vmovdqu16	XMMWORD PTR [edx+2048]{k7}, xmm6	 # AVX512{BW,VL}
	vmovdqu16	XMMWORD PTR [edx-2048]{k7}, xmm6	 # AVX512{BW,VL} Disp8
	vmovdqu16	XMMWORD PTR [edx-2064]{k7}, xmm6	 # AVX512{BW,VL}
	vmovdqu16	YMMWORD PTR [ecx]{k7}, ymm6	 # AVX512{BW,VL}
	vmovdqu16	YMMWORD PTR [esp+esi*8-123456]{k7}, ymm6	 # AVX512{BW,VL}
	vmovdqu16	YMMWORD PTR [edx+4064]{k7}, ymm6	 # AVX512{BW,VL} Disp8
	vmovdqu16	YMMWORD PTR [edx+4096]{k7}, ymm6	 # AVX512{BW,VL}
	vmovdqu16	YMMWORD PTR [edx-4096]{k7}, ymm6	 # AVX512{BW,VL} Disp8
	vmovdqu16	YMMWORD PTR [edx-4128]{k7}, ymm6	 # AVX512{BW,VL}
	vpermi2w	xmm6{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vpermi2w	xmm6{k7}{z}, xmm5, xmm4	 # AVX512{BW,VL}
	vpermi2w	xmm6{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpermi2w	xmm6{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpermi2w	xmm6{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vpermi2w	xmm6{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vpermi2w	xmm6{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vpermi2w	xmm6{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vpermi2w	ymm6{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vpermi2w	ymm6{k7}{z}, ymm5, ymm4	 # AVX512{BW,VL}
	vpermi2w	ymm6{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vpermi2w	ymm6{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vpermi2w	ymm6{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vpermi2w	ymm6{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vpermi2w	ymm6{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vpermi2w	ymm6{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vptestmb	k5{k7}, xmm6, xmm5	 # AVX512{BW,VL}
	vptestmb	k5{k7}, xmm6, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vptestmb	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vptestmb	k5{k7}, xmm6, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vptestmb	k5{k7}, xmm6, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vptestmb	k5{k7}, xmm6, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vptestmb	k5{k7}, xmm6, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vptestmb	k5{k7}, ymm6, ymm5	 # AVX512{BW,VL}
	vptestmb	k5{k7}, ymm6, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vptestmb	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vptestmb	k5{k7}, ymm6, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vptestmb	k5{k7}, ymm6, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vptestmb	k5{k7}, ymm6, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vptestmb	k5{k7}, ymm6, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vptestmw	k5{k7}, xmm6, xmm5	 # AVX512{BW,VL}
	vptestmw	k5{k7}, xmm6, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vptestmw	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vptestmw	k5{k7}, xmm6, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vptestmw	k5{k7}, xmm6, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vptestmw	k5{k7}, xmm6, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vptestmw	k5{k7}, xmm6, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vptestmw	k5{k7}, ymm6, ymm5	 # AVX512{BW,VL}
	vptestmw	k5{k7}, ymm6, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vptestmw	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vptestmw	k5{k7}, ymm6, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vptestmw	k5{k7}, ymm6, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vptestmw	k5{k7}, ymm6, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vptestmw	k5{k7}, ymm6, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpmovb2m	k5, xmm6	 # AVX512{BW,VL}
	vpmovb2m	k5, ymm6	 # AVX512{BW,VL}
	vpmovw2m	k5, xmm6	 # AVX512{BW,VL}
	vpmovw2m	k5, ymm6	 # AVX512{BW,VL}
	vpmovm2b	xmm6, k5	 # AVX512{BW,VL}
	vpmovm2b	ymm6, k5	 # AVX512{BW,VL}
	vpmovm2w	xmm6, k5	 # AVX512{BW,VL}
	vpmovm2w	ymm6, k5	 # AVX512{BW,VL}
	vptestnmb	k5{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vptestnmb	k5{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vptestnmb	k5{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vptestnmb	k5{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vptestnmb	k5{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vptestnmb	k5{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vptestnmb	k5{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vptestnmb	k5{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vptestnmb	k5{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vptestnmb	k5{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vptestnmb	k5{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vptestnmb	k5{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vptestnmb	k5{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vptestnmb	k5{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vptestnmw	k5{k7}, xmm5, xmm4	 # AVX512{BW,VL}
	vptestnmw	k5{k7}, xmm5, XMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vptestnmw	k5{k7}, xmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vptestnmw	k5{k7}, xmm5, XMMWORD PTR [edx+2032]	 # AVX512{BW,VL} Disp8
	vptestnmw	k5{k7}, xmm5, XMMWORD PTR [edx+2048]	 # AVX512{BW,VL}
	vptestnmw	k5{k7}, xmm5, XMMWORD PTR [edx-2048]	 # AVX512{BW,VL} Disp8
	vptestnmw	k5{k7}, xmm5, XMMWORD PTR [edx-2064]	 # AVX512{BW,VL}
	vptestnmw	k5{k7}, ymm5, ymm4	 # AVX512{BW,VL}
	vptestnmw	k5{k7}, ymm5, YMMWORD PTR [ecx]	 # AVX512{BW,VL}
	vptestnmw	k5{k7}, ymm5, YMMWORD PTR [esp+esi*8-123456]	 # AVX512{BW,VL}
	vptestnmw	k5{k7}, ymm5, YMMWORD PTR [edx+4064]	 # AVX512{BW,VL} Disp8
	vptestnmw	k5{k7}, ymm5, YMMWORD PTR [edx+4096]	 # AVX512{BW,VL}
	vptestnmw	k5{k7}, ymm5, YMMWORD PTR [edx-4096]	 # AVX512{BW,VL} Disp8
	vptestnmw	k5{k7}, ymm5, YMMWORD PTR [edx-4128]	 # AVX512{BW,VL}
	vpcmpb	k5{k7}, xmm6, xmm5, 0xab	 # AVX512{BW,VL}
	vpcmpb	k5{k7}, xmm6, xmm5, 123	 # AVX512{BW,VL}
	vpcmpb	k5{k7}, xmm6, XMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpcmpb	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpcmpb	k5{k7}, xmm6, XMMWORD PTR [edx+2032], 123	 # AVX512{BW,VL} Disp8
	vpcmpb	k5{k7}, xmm6, XMMWORD PTR [edx+2048], 123	 # AVX512{BW,VL}
	vpcmpb	k5{k7}, xmm6, XMMWORD PTR [edx-2048], 123	 # AVX512{BW,VL} Disp8
	vpcmpb	k5{k7}, xmm6, XMMWORD PTR [edx-2064], 123	 # AVX512{BW,VL}
	vpcmpb	k5{k7}, ymm6, ymm5, 0xab	 # AVX512{BW,VL}
	vpcmpb	k5{k7}, ymm6, ymm5, 123	 # AVX512{BW,VL}
	vpcmpb	k5{k7}, ymm6, YMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpcmpb	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpcmpb	k5{k7}, ymm6, YMMWORD PTR [edx+4064], 123	 # AVX512{BW,VL} Disp8
	vpcmpb	k5{k7}, ymm6, YMMWORD PTR [edx+4096], 123	 # AVX512{BW,VL}
	vpcmpb	k5{k7}, ymm6, YMMWORD PTR [edx-4096], 123	 # AVX512{BW,VL} Disp8
	vpcmpb	k5{k7}, ymm6, YMMWORD PTR [edx-4128], 123	 # AVX512{BW,VL}
	vpcmpw	k5{k7}, xmm6, xmm5, 0xab	 # AVX512{BW,VL}
	vpcmpw	k5{k7}, xmm6, xmm5, 123	 # AVX512{BW,VL}
	vpcmpw	k5{k7}, xmm6, XMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpcmpw	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpcmpw	k5{k7}, xmm6, XMMWORD PTR [edx+2032], 123	 # AVX512{BW,VL} Disp8
	vpcmpw	k5{k7}, xmm6, XMMWORD PTR [edx+2048], 123	 # AVX512{BW,VL}
	vpcmpw	k5{k7}, xmm6, XMMWORD PTR [edx-2048], 123	 # AVX512{BW,VL} Disp8
	vpcmpw	k5{k7}, xmm6, XMMWORD PTR [edx-2064], 123	 # AVX512{BW,VL}
	vpcmpw	k5{k7}, ymm6, ymm5, 0xab	 # AVX512{BW,VL}
	vpcmpw	k5{k7}, ymm6, ymm5, 123	 # AVX512{BW,VL}
	vpcmpw	k5{k7}, ymm6, YMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpcmpw	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpcmpw	k5{k7}, ymm6, YMMWORD PTR [edx+4064], 123	 # AVX512{BW,VL} Disp8
	vpcmpw	k5{k7}, ymm6, YMMWORD PTR [edx+4096], 123	 # AVX512{BW,VL}
	vpcmpw	k5{k7}, ymm6, YMMWORD PTR [edx-4096], 123	 # AVX512{BW,VL} Disp8
	vpcmpw	k5{k7}, ymm6, YMMWORD PTR [edx-4128], 123	 # AVX512{BW,VL}
	vpcmpub	k5{k7}, xmm6, xmm5, 0xab	 # AVX512{BW,VL}
	vpcmpub	k5{k7}, xmm6, xmm5, 123	 # AVX512{BW,VL}
	vpcmpub	k5{k7}, xmm6, XMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpcmpub	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpcmpub	k5{k7}, xmm6, XMMWORD PTR [edx+2032], 123	 # AVX512{BW,VL} Disp8
	vpcmpub	k5{k7}, xmm6, XMMWORD PTR [edx+2048], 123	 # AVX512{BW,VL}
	vpcmpub	k5{k7}, xmm6, XMMWORD PTR [edx-2048], 123	 # AVX512{BW,VL} Disp8
	vpcmpub	k5{k7}, xmm6, XMMWORD PTR [edx-2064], 123	 # AVX512{BW,VL}
	vpcmpub	k5{k7}, ymm6, ymm5, 0xab	 # AVX512{BW,VL}
	vpcmpub	k5{k7}, ymm6, ymm5, 123	 # AVX512{BW,VL}
	vpcmpub	k5{k7}, ymm6, YMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpcmpub	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpcmpub	k5{k7}, ymm6, YMMWORD PTR [edx+4064], 123	 # AVX512{BW,VL} Disp8
	vpcmpub	k5{k7}, ymm6, YMMWORD PTR [edx+4096], 123	 # AVX512{BW,VL}
	vpcmpub	k5{k7}, ymm6, YMMWORD PTR [edx-4096], 123	 # AVX512{BW,VL} Disp8
	vpcmpub	k5{k7}, ymm6, YMMWORD PTR [edx-4128], 123	 # AVX512{BW,VL}
	vpcmpuw	k5{k7}, xmm6, xmm5, 0xab	 # AVX512{BW,VL}
	vpcmpuw	k5{k7}, xmm6, xmm5, 123	 # AVX512{BW,VL}
	vpcmpuw	k5{k7}, xmm6, XMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpcmpuw	k5{k7}, xmm6, XMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpcmpuw	k5{k7}, xmm6, XMMWORD PTR [edx+2032], 123	 # AVX512{BW,VL} Disp8
	vpcmpuw	k5{k7}, xmm6, XMMWORD PTR [edx+2048], 123	 # AVX512{BW,VL}
	vpcmpuw	k5{k7}, xmm6, XMMWORD PTR [edx-2048], 123	 # AVX512{BW,VL} Disp8
	vpcmpuw	k5{k7}, xmm6, XMMWORD PTR [edx-2064], 123	 # AVX512{BW,VL}
	vpcmpuw	k5{k7}, ymm6, ymm5, 0xab	 # AVX512{BW,VL}
	vpcmpuw	k5{k7}, ymm6, ymm5, 123	 # AVX512{BW,VL}
	vpcmpuw	k5{k7}, ymm6, YMMWORD PTR [ecx], 123	 # AVX512{BW,VL}
	vpcmpuw	k5{k7}, ymm6, YMMWORD PTR [esp+esi*8-123456], 123	 # AVX512{BW,VL}
	vpcmpuw	k5{k7}, ymm6, YMMWORD PTR [edx+4064], 123	 # AVX512{BW,VL} Disp8
	vpcmpuw	k5{k7}, ymm6, YMMWORD PTR [edx+4096], 123	 # AVX512{BW,VL}
	vpcmpuw	k5{k7}, ymm6, YMMWORD PTR [edx-4096], 123	 # AVX512{BW,VL} Disp8
	vpcmpuw	k5{k7}, ymm6, YMMWORD PTR [edx-4128], 123	 # AVX512{BW,VL}
