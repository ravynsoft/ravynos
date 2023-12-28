# Check 32bit AVX512BW instructions

	.allow_index_reg
	.text
_start:
	vpabsb	%zmm5, %zmm6	 # AVX512BW
	vpabsb	%zmm5, %zmm6{%k7}	 # AVX512BW
	vpabsb	%zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpabsb	(%ecx), %zmm6	 # AVX512BW
	vpabsb	-123456(%esp,%esi,8), %zmm6	 # AVX512BW
	vpabsb	8128(%edx), %zmm6	 # AVX512BW Disp8
	vpabsb	8192(%edx), %zmm6	 # AVX512BW
	vpabsb	-8192(%edx), %zmm6	 # AVX512BW Disp8
	vpabsb	-8256(%edx), %zmm6	 # AVX512BW
	vpabsw	%zmm5, %zmm6	 # AVX512BW
	vpabsw	%zmm5, %zmm6{%k7}	 # AVX512BW
	vpabsw	%zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpabsw	(%ecx), %zmm6	 # AVX512BW
	vpabsw	-123456(%esp,%esi,8), %zmm6	 # AVX512BW
	vpabsw	8128(%edx), %zmm6	 # AVX512BW Disp8
	vpabsw	8192(%edx), %zmm6	 # AVX512BW
	vpabsw	-8192(%edx), %zmm6	 # AVX512BW Disp8
	vpabsw	-8256(%edx), %zmm6	 # AVX512BW
	vpackssdw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpackssdw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpackssdw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpackssdw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpackssdw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpackssdw	(%eax){1to16}, %zmm5, %zmm6	 # AVX512BW
	vpackssdw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpackssdw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpackssdw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpackssdw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpackssdw	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512BW Disp8
	vpackssdw	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512BW
	vpackssdw	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512BW Disp8
	vpackssdw	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512BW
	vpacksswb	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpacksswb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpacksswb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpacksswb	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpacksswb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpacksswb	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpacksswb	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpacksswb	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpacksswb	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpackusdw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpackusdw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpackusdw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpackusdw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpackusdw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpackusdw	(%eax){1to16}, %zmm5, %zmm6	 # AVX512BW
	vpackusdw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpackusdw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpackusdw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpackusdw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpackusdw	508(%edx){1to16}, %zmm5, %zmm6	 # AVX512BW Disp8
	vpackusdw	512(%edx){1to16}, %zmm5, %zmm6	 # AVX512BW
	vpackusdw	-512(%edx){1to16}, %zmm5, %zmm6	 # AVX512BW Disp8
	vpackusdw	-516(%edx){1to16}, %zmm5, %zmm6	 # AVX512BW
	vpackuswb	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpackuswb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpackuswb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpackuswb	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpackuswb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpackuswb	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpackuswb	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpackuswb	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpackuswb	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpaddb	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpaddb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpaddb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpaddb	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpaddb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpaddb	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpaddb	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpaddb	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpaddb	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpaddsb	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpaddsb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpaddsb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpaddsb	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpaddsb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpaddsb	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpaddsb	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpaddsb	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpaddsb	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpaddsw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpaddsw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpaddsw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpaddsw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpaddsw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpaddsw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpaddsw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpaddsw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpaddsw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpaddusb	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpaddusb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpaddusb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpaddusb	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpaddusb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpaddusb	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpaddusb	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpaddusb	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpaddusb	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpaddusw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpaddusw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpaddusw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpaddusw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpaddusw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpaddusw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpaddusw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpaddusw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpaddusw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpaddw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpaddw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpaddw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpaddw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpaddw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpaddw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpaddw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpaddw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpaddw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpalignr	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512BW
	vpalignr	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpalignr	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpalignr	$123, %zmm4, %zmm5, %zmm6	 # AVX512BW
	vpalignr	$123, (%ecx), %zmm5, %zmm6	 # AVX512BW
	vpalignr	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpalignr	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpalignr	$123, 8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpalignr	$123, -8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpalignr	$123, -8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpavgb	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpavgb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpavgb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpavgb	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpavgb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpavgb	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpavgb	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpavgb	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpavgb	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpavgw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpavgw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpavgw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpavgw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpavgw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpavgw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpavgw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpavgw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpavgw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpblendmb	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpblendmb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpblendmb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpblendmb	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpblendmb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpblendmb	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpblendmb	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpblendmb	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpblendmb	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpbroadcastb	%xmm5, %zmm6{%k7}	 # AVX512BW
	vpbroadcastb	%xmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpbroadcastb	(%ecx), %zmm6{%k7}	 # AVX512BW
	vpbroadcastb	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512BW
	vpbroadcastb	127(%edx), %zmm6{%k7}	 # AVX512BW Disp8
	vpbroadcastb	128(%edx), %zmm6{%k7}	 # AVX512BW
	vpbroadcastb	-128(%edx), %zmm6{%k7}	 # AVX512BW Disp8
	vpbroadcastb	-129(%edx), %zmm6{%k7}	 # AVX512BW
	vpbroadcastb	%eax, %zmm6	 # AVX512BW
	vpbroadcastb	%eax, %zmm6{%k7}	 # AVX512BW
	vpbroadcastb	%eax, %zmm6{%k7}{z}	 # AVX512BW
	vpbroadcastb	%ebp, %zmm6	 # AVX512BW
	vpbroadcastw	%xmm5, %zmm6{%k7}	 # AVX512BW
	vpbroadcastw	%xmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpbroadcastw	(%ecx), %zmm6{%k7}	 # AVX512BW
	vpbroadcastw	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512BW
	vpbroadcastw	254(%edx), %zmm6{%k7}	 # AVX512BW Disp8
	vpbroadcastw	256(%edx), %zmm6{%k7}	 # AVX512BW
	vpbroadcastw	-256(%edx), %zmm6{%k7}	 # AVX512BW Disp8
	vpbroadcastw	-258(%edx), %zmm6{%k7}	 # AVX512BW
	vpbroadcastw	%eax, %zmm6	 # AVX512BW
	vpbroadcastw	%eax, %zmm6{%k7}	 # AVX512BW
	vpbroadcastw	%eax, %zmm6{%k7}{z}	 # AVX512BW
	vpbroadcastw	%ebp, %zmm6	 # AVX512BW
	vpcmpeqb	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpeqb	%zmm5, %zmm6, %k5{%k7}	 # AVX512BW
	vpcmpeqb	(%ecx), %zmm6, %k5	 # AVX512BW
	vpcmpeqb	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512BW
	vpcmpeqb	8128(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vpcmpeqb	8192(%edx), %zmm6, %k5	 # AVX512BW
	vpcmpeqb	-8192(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vpcmpeqb	-8256(%edx), %zmm6, %k5	 # AVX512BW
	vpcmpeqw	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpeqw	%zmm5, %zmm6, %k5{%k7}	 # AVX512BW
	vpcmpeqw	(%ecx), %zmm6, %k5	 # AVX512BW
	vpcmpeqw	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512BW
	vpcmpeqw	8128(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vpcmpeqw	8192(%edx), %zmm6, %k5	 # AVX512BW
	vpcmpeqw	-8192(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vpcmpeqw	-8256(%edx), %zmm6, %k5	 # AVX512BW
	vpcmpgtb	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpgtb	%zmm5, %zmm6, %k5{%k7}	 # AVX512BW
	vpcmpgtb	(%ecx), %zmm6, %k5	 # AVX512BW
	vpcmpgtb	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512BW
	vpcmpgtb	8128(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vpcmpgtb	8192(%edx), %zmm6, %k5	 # AVX512BW
	vpcmpgtb	-8192(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vpcmpgtb	-8256(%edx), %zmm6, %k5	 # AVX512BW
	vpcmpgtw	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpgtw	%zmm5, %zmm6, %k5{%k7}	 # AVX512BW
	vpcmpgtw	(%ecx), %zmm6, %k5	 # AVX512BW
	vpcmpgtw	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512BW
	vpcmpgtw	8128(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vpcmpgtw	8192(%edx), %zmm6, %k5	 # AVX512BW
	vpcmpgtw	-8192(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vpcmpgtw	-8256(%edx), %zmm6, %k5	 # AVX512BW
	vpblendmw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpblendmw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpblendmw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpblendmw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpblendmw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpblendmw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpblendmw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpblendmw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpblendmw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmaddubsw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpmaddubsw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpmaddubsw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpmaddubsw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpmaddubsw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpmaddubsw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmaddubsw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmaddubsw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmaddubsw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmaddwd	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpmaddwd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpmaddwd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpmaddwd	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpmaddwd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpmaddwd	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmaddwd	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmaddwd	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmaddwd	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmaxsb	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpmaxsb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpmaxsb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpmaxsb	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpmaxsb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpmaxsb	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmaxsb	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmaxsb	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmaxsb	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmaxsw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpmaxsw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpmaxsw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpmaxsw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpmaxsw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpmaxsw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmaxsw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmaxsw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmaxsw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmaxub	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpmaxub	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpmaxub	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpmaxub	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpmaxub	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpmaxub	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmaxub	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmaxub	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmaxub	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmaxuw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpmaxuw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpmaxuw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpmaxuw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpmaxuw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpmaxuw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmaxuw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmaxuw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmaxuw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpminsb	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpminsb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpminsb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpminsb	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpminsb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpminsb	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpminsb	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpminsb	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpminsb	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpminsw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpminsw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpminsw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpminsw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpminsw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpminsw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpminsw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpminsw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpminsw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpminub	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpminub	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpminub	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpminub	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpminub	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpminub	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpminub	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpminub	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpminub	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpminuw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpminuw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpminuw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpminuw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpminuw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpminuw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpminuw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpminuw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpminuw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmovsxbw	%ymm5, %zmm6{%k7}	 # AVX512BW
	vpmovsxbw	%ymm5, %zmm6{%k7}{z}	 # AVX512BW
	vpmovsxbw	(%ecx), %zmm6{%k7}	 # AVX512BW
	vpmovsxbw	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512BW
	vpmovsxbw	4064(%edx), %zmm6{%k7}	 # AVX512BW Disp8
	vpmovsxbw	4096(%edx), %zmm6{%k7}	 # AVX512BW
	vpmovsxbw	-4096(%edx), %zmm6{%k7}	 # AVX512BW Disp8
	vpmovsxbw	-4128(%edx), %zmm6{%k7}	 # AVX512BW
	vpmovzxbw	%ymm5, %zmm6{%k7}	 # AVX512BW
	vpmovzxbw	%ymm5, %zmm6{%k7}{z}	 # AVX512BW
	vpmovzxbw	(%ecx), %zmm6{%k7}	 # AVX512BW
	vpmovzxbw	-123456(%esp,%esi,8), %zmm6{%k7}	 # AVX512BW
	vpmovzxbw	4064(%edx), %zmm6{%k7}	 # AVX512BW Disp8
	vpmovzxbw	4096(%edx), %zmm6{%k7}	 # AVX512BW
	vpmovzxbw	-4096(%edx), %zmm6{%k7}	 # AVX512BW Disp8
	vpmovzxbw	-4128(%edx), %zmm6{%k7}	 # AVX512BW
	vpmulhrsw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpmulhrsw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpmulhrsw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpmulhrsw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpmulhrsw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpmulhrsw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmulhrsw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmulhrsw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmulhrsw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmulhuw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpmulhuw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpmulhuw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpmulhuw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpmulhuw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpmulhuw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmulhuw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmulhuw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmulhuw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmulhw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpmulhw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpmulhw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpmulhw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpmulhw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpmulhw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmulhw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmulhw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmulhw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmullw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpmullw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpmullw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpmullw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpmullw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpmullw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmullw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmullw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpmullw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsadbw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpsadbw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpsadbw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpsadbw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsadbw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsadbw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsadbw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpshufb	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpshufb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpshufb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpshufb	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpshufb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpshufb	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpshufb	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpshufb	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpshufb	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpshufhw	$0xab, %zmm5, %zmm6	 # AVX512BW
	vpshufhw	$0xab, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpshufhw	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpshufhw	$123, %zmm5, %zmm6	 # AVX512BW
	vpshufhw	$123, (%ecx), %zmm6	 # AVX512BW
	vpshufhw	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512BW
	vpshufhw	$123, 8128(%edx), %zmm6	 # AVX512BW Disp8
	vpshufhw	$123, 8192(%edx), %zmm6	 # AVX512BW
	vpshufhw	$123, -8192(%edx), %zmm6	 # AVX512BW Disp8
	vpshufhw	$123, -8256(%edx), %zmm6	 # AVX512BW
	vpshuflw	$0xab, %zmm5, %zmm6	 # AVX512BW
	vpshuflw	$0xab, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpshuflw	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpshuflw	$123, %zmm5, %zmm6	 # AVX512BW
	vpshuflw	$123, (%ecx), %zmm6	 # AVX512BW
	vpshuflw	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512BW
	vpshuflw	$123, 8128(%edx), %zmm6	 # AVX512BW Disp8
	vpshuflw	$123, 8192(%edx), %zmm6	 # AVX512BW
	vpshuflw	$123, -8192(%edx), %zmm6	 # AVX512BW Disp8
	vpshuflw	$123, -8256(%edx), %zmm6	 # AVX512BW
	vpsllw	%xmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsllw	%xmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpsllw	(%ecx), %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsllw	-123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsllw	2032(%edx), %zmm5, %zmm6{%k7}	 # AVX512BW Disp8
	vpsllw	2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsllw	-2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512BW Disp8
	vpsllw	-2064(%edx), %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsraw	%xmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsraw	%xmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpsraw	(%ecx), %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsraw	-123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsraw	2032(%edx), %zmm5, %zmm6{%k7}	 # AVX512BW Disp8
	vpsraw	2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsraw	-2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512BW Disp8
	vpsraw	-2064(%edx), %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsrlw	%xmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsrlw	%xmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpsrlw	(%ecx), %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsrlw	-123456(%esp,%esi,8), %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsrlw	2032(%edx), %zmm5, %zmm6{%k7}	 # AVX512BW Disp8
	vpsrlw	2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsrlw	-2048(%edx), %zmm5, %zmm6{%k7}	 # AVX512BW Disp8
	vpsrlw	-2064(%edx), %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsrldq	$0xab, %zmm5, %zmm6	 # AVX512BW
	vpsrldq	$123, %zmm5, %zmm6	 # AVX512BW
	vpsrldq	$123, (%ecx), %zmm6	 # AVX512BW
	vpsrldq	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512BW
	vpsrldq	$123, 8128(%edx), %zmm6	 # AVX512BW Disp8
	vpsrldq	$123, 8192(%edx), %zmm6	 # AVX512BW
	vpsrldq	$123, -8192(%edx), %zmm6	 # AVX512BW Disp8
	vpsrldq	$123, -8256(%edx), %zmm6	 # AVX512BW
	vpsrlw	$0xab, %zmm5, %zmm6	 # AVX512BW
	vpsrlw	$0xab, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsrlw	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpsrlw	$123, %zmm5, %zmm6	 # AVX512BW
	vpsrlw	$123, (%ecx), %zmm6	 # AVX512BW
	vpsrlw	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512BW
	vpsrlw	$123, 8128(%edx), %zmm6	 # AVX512BW Disp8
	vpsrlw	$123, 8192(%edx), %zmm6	 # AVX512BW
	vpsrlw	$123, -8192(%edx), %zmm6	 # AVX512BW Disp8
	vpsrlw	$123, -8256(%edx), %zmm6	 # AVX512BW
	vpsraw	$0xab, %zmm5, %zmm6	 # AVX512BW
	vpsraw	$0xab, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsraw	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpsraw	$123, %zmm5, %zmm6	 # AVX512BW
	vpsraw	$123, (%ecx), %zmm6	 # AVX512BW
	vpsraw	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512BW
	vpsraw	$123, 8128(%edx), %zmm6	 # AVX512BW Disp8
	vpsraw	$123, 8192(%edx), %zmm6	 # AVX512BW
	vpsraw	$123, -8192(%edx), %zmm6	 # AVX512BW Disp8
	vpsraw	$123, -8256(%edx), %zmm6	 # AVX512BW
	vpsrlvw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpsrlvw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsrlvw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpsrlvw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpsrlvw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpsrlvw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsrlvw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsrlvw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsrlvw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsravw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpsravw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsravw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpsravw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpsravw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpsravw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsravw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsravw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsravw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsubb	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpsubb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsubb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpsubb	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpsubb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpsubb	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsubb	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsubb	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsubb	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsubsb	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpsubsb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsubsb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpsubsb	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpsubsb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpsubsb	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsubsb	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsubsb	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsubsb	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsubsw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpsubsw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsubsw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpsubsw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpsubsw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpsubsw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsubsw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsubsw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsubsw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsubusb	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpsubusb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsubusb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpsubusb	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpsubusb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpsubusb	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsubusb	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsubusb	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsubusb	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsubusw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpsubusw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsubusw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpsubusw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpsubusw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpsubusw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsubusw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsubusw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsubusw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsubw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpsubw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsubw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpsubw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpsubw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpsubw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsubw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsubw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsubw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpunpckhbw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpunpckhbw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpunpckhbw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpunpckhbw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpunpckhbw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpunpckhbw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpunpckhbw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpunpckhbw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpunpckhbw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpunpckhwd	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpunpckhwd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpunpckhwd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpunpckhwd	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpunpckhwd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpunpckhwd	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpunpckhwd	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpunpckhwd	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpunpckhwd	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpunpcklbw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpunpcklbw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpunpcklbw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpunpcklbw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpunpcklbw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpunpcklbw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpunpcklbw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpunpcklbw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpunpcklbw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpunpcklwd	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpunpcklwd	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpunpcklwd	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpunpcklwd	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpunpcklwd	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpunpcklwd	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpunpcklwd	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpunpcklwd	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpunpcklwd	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpmovwb	%zmm5, %ymm6{%k7}	 # AVX512BW
	vpmovwb	%zmm5, %ymm6{%k7}{z}	 # AVX512BW
	vpmovswb	%zmm5, %ymm6{%k7}	 # AVX512BW
	vpmovswb	%zmm5, %ymm6{%k7}{z}	 # AVX512BW
	vpmovuswb	%zmm5, %ymm6{%k7}	 # AVX512BW
	vpmovuswb	%zmm5, %ymm6{%k7}{z}	 # AVX512BW
	vdbpsadbw	$0xab, %zmm4, %zmm5, %zmm6	 # AVX512BW
	vdbpsadbw	$0xab, %zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vdbpsadbw	$0xab, %zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vdbpsadbw	$123, %zmm4, %zmm5, %zmm6	 # AVX512BW
	vdbpsadbw	$123, (%ecx), %zmm5, %zmm6	 # AVX512BW
	vdbpsadbw	$123, -123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vdbpsadbw	$123, 8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vdbpsadbw	$123, 8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vdbpsadbw	$123, -8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vdbpsadbw	$123, -8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpermw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpermw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpermw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpermw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpermw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpermw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpermw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpermw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpermw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpermt2w	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpermt2w	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpermt2w	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpermt2w	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpermt2w	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpermt2w	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpermt2w	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpermt2w	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpermt2w	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vpslldq	$0xab, %zmm5, %zmm6	 # AVX512BW
	vpslldq	$123, %zmm5, %zmm6	 # AVX512BW
	vpslldq	$123, (%ecx), %zmm6	 # AVX512BW
	vpslldq	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512BW
	vpslldq	$123, 8128(%edx), %zmm6	 # AVX512BW Disp8
	vpslldq	$123, 8192(%edx), %zmm6	 # AVX512BW
	vpslldq	$123, -8192(%edx), %zmm6	 # AVX512BW Disp8
	vpslldq	$123, -8256(%edx), %zmm6	 # AVX512BW
	vpsllw	$0xab, %zmm5, %zmm6	 # AVX512BW
	vpsllw	$0xab, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsllw	$0xab, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpsllw	$123, %zmm5, %zmm6	 # AVX512BW
	vpsllw	$123, (%ecx), %zmm6	 # AVX512BW
	vpsllw	$123, -123456(%esp,%esi,8), %zmm6	 # AVX512BW
	vpsllw	$123, 8128(%edx), %zmm6	 # AVX512BW Disp8
	vpsllw	$123, 8192(%edx), %zmm6	 # AVX512BW
	vpsllw	$123, -8192(%edx), %zmm6	 # AVX512BW Disp8
	vpsllw	$123, -8256(%edx), %zmm6	 # AVX512BW
	vpsllvw	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpsllvw	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpsllvw	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpsllvw	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpsllvw	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpsllvw	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsllvw	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpsllvw	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpsllvw	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vmovdqu8	%zmm5, %zmm6	 # AVX512BW
	vmovdqu8	%zmm5, %zmm6{%k7}	 # AVX512BW
	vmovdqu8	%zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vmovdqu8	(%ecx), %zmm6	 # AVX512BW
	vmovdqu8	-123456(%esp,%esi,8), %zmm6	 # AVX512BW
	vmovdqu8	8128(%edx), %zmm6	 # AVX512BW Disp8
	vmovdqu8	8192(%edx), %zmm6	 # AVX512BW
	vmovdqu8	-8192(%edx), %zmm6	 # AVX512BW Disp8
	vmovdqu8	-8256(%edx), %zmm6	 # AVX512BW
	vmovdqu16	%zmm5, %zmm6	 # AVX512BW
	vmovdqu16	%zmm5, %zmm6{%k7}	 # AVX512BW
	vmovdqu16	%zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vmovdqu16	(%ecx), %zmm6	 # AVX512BW
	vmovdqu16	-123456(%esp,%esi,8), %zmm6	 # AVX512BW
	vmovdqu16	8128(%edx), %zmm6	 # AVX512BW Disp8
	vmovdqu16	8192(%edx), %zmm6	 # AVX512BW
	vmovdqu16	-8192(%edx), %zmm6	 # AVX512BW Disp8
	vmovdqu16	-8256(%edx), %zmm6	 # AVX512BW
	kandq	%k7, %k6, %k5	 # AVX512BW
	kandd	%k7, %k6, %k5	 # AVX512BW
	kandnq	%k7, %k6, %k5	 # AVX512BW
	kandnd	%k7, %k6, %k5	 # AVX512BW
	korq	%k7, %k6, %k5	 # AVX512BW
	kord	%k7, %k6, %k5	 # AVX512BW
	kxnorq	%k7, %k6, %k5	 # AVX512BW
	kxnord	%k7, %k6, %k5	 # AVX512BW
	kxorq	%k7, %k6, %k5	 # AVX512BW
	kxord	%k7, %k6, %k5	 # AVX512BW
	knotq	%k6, %k5	 # AVX512BW
	knotd	%k6, %k5	 # AVX512BW
	kortestq	%k6, %k5	 # AVX512BW
	kortestd	%k6, %k5	 # AVX512BW
	ktestq	%k6, %k5	 # AVX512BW
	ktestd	%k6, %k5	 # AVX512BW
	kshiftrq	$0xab, %k6, %k5	 # AVX512BW
	kshiftrq	$123, %k6, %k5	 # AVX512BW
	kshiftrd	$0xab, %k6, %k5	 # AVX512BW
	kshiftrd	$123, %k6, %k5	 # AVX512BW
	kshiftlq	$0xab, %k6, %k5	 # AVX512BW
	kshiftlq	$123, %k6, %k5	 # AVX512BW
	kshiftld	$0xab, %k6, %k5	 # AVX512BW
	kshiftld	$123, %k6, %k5	 # AVX512BW
	kmovq	%k6, %k5	 # AVX512BW
	kmovq	(%ecx), %k5	 # AVX512BW
	kmovq	-123456(%esp,%esi,8), %k5	 # AVX512BW
	kmovd	%k6, %k5	 # AVX512BW
	kmovd	(%ecx), %k5	 # AVX512BW
	kmovd	-123456(%esp,%esi,8), %k5	 # AVX512BW
	kmovq	%k5, (%ecx)	 # AVX512BW
	kmovq	%k5, -123456(%esp,%esi,8)	 # AVX512BW
	kmovd	%k5, (%ecx)	 # AVX512BW
	kmovd	%k5, -123456(%esp,%esi,8)	 # AVX512BW
	kmovd	%eax, %k5	 # AVX512BW
	kmovd	%ebp, %k5	 # AVX512BW
	kmovd	%k5, %eax	 # AVX512BW
	kmovd	%k5, %ebp	 # AVX512BW
	kaddq	%k7, %k6, %k5	 # AVX512BW
	kaddd	%k7, %k6, %k5	 # AVX512BW
	kunpckwd	%k7, %k6, %k5	 # AVX512BW
	kunpckdq	%k7, %k6, %k5	 # AVX512BW
	vpmovwb	%zmm6, (%ecx)	 # AVX512BW
	vpmovwb	%zmm6, (%ecx){%k7}	 # AVX512BW
	vpmovwb	%zmm6, -123456(%esp,%esi,8)	 # AVX512BW
	vpmovwb	%zmm6, 4064(%edx)	 # AVX512BW Disp8
	vpmovwb	%zmm6, 4096(%edx)	 # AVX512BW
	vpmovwb	%zmm6, -4096(%edx)	 # AVX512BW Disp8
	vpmovwb	%zmm6, -4128(%edx)	 # AVX512BW
	vpmovswb	%zmm6, (%ecx)	 # AVX512BW
	vpmovswb	%zmm6, (%ecx){%k7}	 # AVX512BW
	vpmovswb	%zmm6, -123456(%esp,%esi,8)	 # AVX512BW
	vpmovswb	%zmm6, 4064(%edx)	 # AVX512BW Disp8
	vpmovswb	%zmm6, 4096(%edx)	 # AVX512BW
	vpmovswb	%zmm6, -4096(%edx)	 # AVX512BW Disp8
	vpmovswb	%zmm6, -4128(%edx)	 # AVX512BW
	vpmovuswb	%zmm6, (%ecx)	 # AVX512BW
	vpmovuswb	%zmm6, (%ecx){%k7}	 # AVX512BW
	vpmovuswb	%zmm6, -123456(%esp,%esi,8)	 # AVX512BW
	vpmovuswb	%zmm6, 4064(%edx)	 # AVX512BW Disp8
	vpmovuswb	%zmm6, 4096(%edx)	 # AVX512BW
	vpmovuswb	%zmm6, -4096(%edx)	 # AVX512BW Disp8
	vpmovuswb	%zmm6, -4128(%edx)	 # AVX512BW
	vmovdqu8	%zmm6, (%ecx)	 # AVX512BW
	vmovdqu8	%zmm6, (%ecx){%k7}	 # AVX512BW
	vmovdqu8	%zmm6, -123456(%esp,%esi,8)	 # AVX512BW
	vmovdqu8	%zmm6, 8128(%edx)	 # AVX512BW Disp8
	vmovdqu8	%zmm6, 8192(%edx)	 # AVX512BW
	vmovdqu8	%zmm6, -8192(%edx)	 # AVX512BW Disp8
	vmovdqu8	%zmm6, -8256(%edx)	 # AVX512BW
	vmovdqu16	%zmm6, (%ecx)	 # AVX512BW
	vmovdqu16	%zmm6, (%ecx){%k7}	 # AVX512BW
	vmovdqu16	%zmm6, -123456(%esp,%esi,8)	 # AVX512BW
	vmovdqu16	%zmm6, 8128(%edx)	 # AVX512BW Disp8
	vmovdqu16	%zmm6, 8192(%edx)	 # AVX512BW
	vmovdqu16	%zmm6, -8192(%edx)	 # AVX512BW Disp8
	vmovdqu16	%zmm6, -8256(%edx)	 # AVX512BW
	vpermi2w	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpermi2w	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpermi2w	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpermi2w	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpermi2w	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpermi2w	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpermi2w	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpermi2w	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpermi2w	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
	vptestmb	%zmm5, %zmm6, %k5	 # AVX512BW
	vptestmb	%zmm5, %zmm6, %k5{%k7}	 # AVX512BW
	vptestmb	(%ecx), %zmm6, %k5	 # AVX512BW
	vptestmb	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512BW
	vptestmb	8128(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vptestmb	8192(%edx), %zmm6, %k5	 # AVX512BW
	vptestmb	-8192(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vptestmb	-8256(%edx), %zmm6, %k5	 # AVX512BW
	vptestmw	%zmm5, %zmm6, %k5	 # AVX512BW
	vptestmw	%zmm5, %zmm6, %k5{%k7}	 # AVX512BW
	vptestmw	(%ecx), %zmm6, %k5	 # AVX512BW
	vptestmw	-123456(%esp,%esi,8), %zmm6, %k5	 # AVX512BW
	vptestmw	8128(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vptestmw	8192(%edx), %zmm6, %k5	 # AVX512BW
	vptestmw	-8192(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vptestmw	-8256(%edx), %zmm6, %k5	 # AVX512BW
	vpmovb2m	%zmm6, %k5	 # AVX512BW
	vpmovw2m	%zmm6, %k5	 # AVX512BW
	vpmovm2b	%k5, %zmm6	 # AVX512BW
	vpmovm2w	%k5, %zmm6	 # AVX512BW
	vptestnmb	%zmm4, %zmm5, %k5	 # AVX512BW
	vptestnmb	%zmm4, %zmm5, %k5{%k7}	 # AVX512BW
	vptestnmb	(%ecx), %zmm5, %k5	 # AVX512BW
	vptestnmb	-123456(%esp,%esi,8), %zmm5, %k5	 # AVX512BW
	vptestnmb	8128(%edx), %zmm5, %k5	 # AVX512BW Disp8
	vptestnmb	8192(%edx), %zmm5, %k5	 # AVX512BW
	vptestnmb	-8192(%edx), %zmm5, %k5	 # AVX512BW Disp8
	vptestnmb	-8256(%edx), %zmm5, %k5	 # AVX512BW
	vptestnmw	%zmm4, %zmm5, %k5	 # AVX512BW
	vptestnmw	%zmm4, %zmm5, %k5{%k7}	 # AVX512BW
	vptestnmw	(%ecx), %zmm5, %k5	 # AVX512BW
	vptestnmw	-123456(%esp,%esi,8), %zmm5, %k5	 # AVX512BW
	vptestnmw	8128(%edx), %zmm5, %k5	 # AVX512BW Disp8
	vptestnmw	8192(%edx), %zmm5, %k5	 # AVX512BW
	vptestnmw	-8192(%edx), %zmm5, %k5	 # AVX512BW Disp8
	vptestnmw	-8256(%edx), %zmm5, %k5	 # AVX512BW
	vpcmpb	$0xab, %zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpb	$0xab, %zmm5, %zmm6, %k5{%k7}	 # AVX512BW
	vpcmpb	$123, %zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpb	$123, (%ecx), %zmm6, %k5	 # AVX512BW
	vpcmpb	$123, -123456(%esp,%esi,8), %zmm6, %k5	 # AVX512BW
	vpcmpb	$123, 8128(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vpcmpb	$123, 8192(%edx), %zmm6, %k5	 # AVX512BW
	vpcmpb	$123, -8192(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vpcmpb	$123, -8256(%edx), %zmm6, %k5	 # AVX512BW
	vpcmpb	$0, %zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpleb	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpleb	0x1fc0(%eax), %zmm6, %k5 # AVX512{BW,VL} Disp8
	vpcmpleb	0x2000(%eax), %zmm6, %k5 # AVX512{BW,VL}
	vpcmpltb	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpneqb	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpnleb	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpnltb	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpw	$0xab, %zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpw	$0xab, %zmm5, %zmm6, %k5{%k7}	 # AVX512BW
	vpcmpw	$123, %zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpw	$123, (%ecx), %zmm6, %k5	 # AVX512BW
	vpcmpw	$123, -123456(%esp,%esi,8), %zmm6, %k5	 # AVX512BW
	vpcmpw	$123, 8128(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vpcmpw	$123, 8192(%edx), %zmm6, %k5	 # AVX512BW
	vpcmpw	$123, -8192(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vpcmpw	$123, -8256(%edx), %zmm6, %k5	 # AVX512BW
	vpcmpw	$0, %zmm5, %zmm6, %k5	 # AVX512BW
	vpcmplew	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmplew	0x1fc0(%eax), %zmm6, %k5 # AVX512{BW,VL} Disp8
	vpcmplew	0x2000(%eax), %zmm6, %k5 # AVX512{BW,VL}
	vpcmpltw	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpneqw	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpnlew	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpnltw	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpub	$0xab, %zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpub	$0xab, %zmm5, %zmm6, %k5{%k7}	 # AVX512BW
	vpcmpub	$123, %zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpub	$123, (%ecx), %zmm6, %k5	 # AVX512BW
	vpcmpub	$123, -123456(%esp,%esi,8), %zmm6, %k5	 # AVX512BW
	vpcmpub	$123, 8128(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vpcmpub	$123, 8192(%edx), %zmm6, %k5	 # AVX512BW
	vpcmpub	$123, -8192(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vpcmpub	$123, -8256(%edx), %zmm6, %k5	 # AVX512BW
	vpcmpequb	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpleub	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpltub	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpnequb	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpnleub	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpnltub	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpuw	$0xab, %zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpuw	$0xab, %zmm5, %zmm6, %k5{%k7}	 # AVX512BW
	vpcmpuw	$123, %zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpuw	$123, (%ecx), %zmm6, %k5	 # AVX512BW
	vpcmpuw	$123, -123456(%esp,%esi,8), %zmm6, %k5	 # AVX512BW
	vpcmpuw	$123, 8128(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vpcmpuw	$123, 8192(%edx), %zmm6, %k5	 # AVX512BW
	vpcmpuw	$123, -8192(%edx), %zmm6, %k5	 # AVX512BW Disp8
	vpcmpuw	$123, -8256(%edx), %zmm6, %k5	 # AVX512BW
	vpcmpequw	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpleuw	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpltuw	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpnequw	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpnleuw	%zmm5, %zmm6, %k5	 # AVX512BW
	vpcmpnltuw	%zmm5, %zmm6, %k5	 # AVX512BW

	.intel_syntax noprefix
	vpabsb	zmm6, zmm5	 # AVX512BW
	vpabsb	zmm6{k7}, zmm5	 # AVX512BW
	vpabsb	zmm6{k7}{z}, zmm5	 # AVX512BW
	vpabsb	zmm6, ZMMWORD PTR [ecx]	 # AVX512BW
	vpabsb	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpabsb	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpabsb	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpabsb	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpabsb	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpabsw	zmm6, zmm5	 # AVX512BW
	vpabsw	zmm6{k7}, zmm5	 # AVX512BW
	vpabsw	zmm6{k7}{z}, zmm5	 # AVX512BW
	vpabsw	zmm6, ZMMWORD PTR [ecx]	 # AVX512BW
	vpabsw	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpabsw	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpabsw	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpabsw	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpabsw	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpackssdw	zmm6, zmm5, zmm4	 # AVX512BW
	vpackssdw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpackssdw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpackssdw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpackssdw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpackssdw	zmm6, zmm5, [eax]{1to16}	 # AVX512BW
	vpackssdw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpackssdw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpackssdw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpackssdw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpackssdw	zmm6, zmm5, [edx+508]{1to16}	 # AVX512BW Disp8
	vpackssdw	zmm6, zmm5, [edx+512]{1to16}	 # AVX512BW
	vpackssdw	zmm6, zmm5, [edx-512]{1to16}	 # AVX512BW Disp8
	vpackssdw	zmm6, zmm5, [edx-516]{1to16}	 # AVX512BW
	vpacksswb	zmm6, zmm5, zmm4	 # AVX512BW
	vpacksswb	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpacksswb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpacksswb	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpacksswb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpacksswb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpacksswb	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpacksswb	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpacksswb	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpackusdw	zmm6, zmm5, zmm4	 # AVX512BW
	vpackusdw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpackusdw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpackusdw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpackusdw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpackusdw	zmm6, zmm5, [eax]{1to16}	 # AVX512BW
	vpackusdw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpackusdw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpackusdw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpackusdw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpackusdw	zmm6, zmm5, [edx+508]{1to16}	 # AVX512BW Disp8
	vpackusdw	zmm6, zmm5, [edx+512]{1to16}	 # AVX512BW
	vpackusdw	zmm6, zmm5, [edx-512]{1to16}	 # AVX512BW Disp8
	vpackusdw	zmm6, zmm5, [edx-516]{1to16}	 # AVX512BW
	vpackuswb	zmm6, zmm5, zmm4	 # AVX512BW
	vpackuswb	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpackuswb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpackuswb	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpackuswb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpackuswb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpackuswb	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpackuswb	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpackuswb	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpaddb	zmm6, zmm5, zmm4	 # AVX512BW
	vpaddb	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpaddb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpaddb	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpaddb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpaddb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpaddb	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpaddb	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpaddb	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpaddsb	zmm6, zmm5, zmm4	 # AVX512BW
	vpaddsb	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpaddsb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpaddsb	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpaddsb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpaddsb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpaddsb	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpaddsb	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpaddsb	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpaddsw	zmm6, zmm5, zmm4	 # AVX512BW
	vpaddsw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpaddsw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpaddsw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpaddsw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpaddsw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpaddsw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpaddsw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpaddsw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpaddusb	zmm6, zmm5, zmm4	 # AVX512BW
	vpaddusb	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpaddusb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpaddusb	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpaddusb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpaddusb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpaddusb	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpaddusb	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpaddusb	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpaddusw	zmm6, zmm5, zmm4	 # AVX512BW
	vpaddusw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpaddusw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpaddusw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpaddusw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpaddusw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpaddusw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpaddusw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpaddusw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpaddw	zmm6, zmm5, zmm4	 # AVX512BW
	vpaddw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpaddw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpaddw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpaddw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpaddw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpaddw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpaddw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpaddw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpalignr	zmm6, zmm5, zmm4, 0xab	 # AVX512BW
	vpalignr	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512BW
	vpalignr	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512BW
	vpalignr	zmm6, zmm5, zmm4, 123	 # AVX512BW
	vpalignr	zmm6, zmm5, ZMMWORD PTR [ecx], 123	 # AVX512BW
	vpalignr	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512BW
	vpalignr	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512BW Disp8
	vpalignr	zmm6, zmm5, ZMMWORD PTR [edx+8192], 123	 # AVX512BW
	vpalignr	zmm6, zmm5, ZMMWORD PTR [edx-8192], 123	 # AVX512BW Disp8
	vpalignr	zmm6, zmm5, ZMMWORD PTR [edx-8256], 123	 # AVX512BW
	vpavgb	zmm6, zmm5, zmm4	 # AVX512BW
	vpavgb	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpavgb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpavgb	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpavgb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpavgb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpavgb	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpavgb	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpavgb	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpavgw	zmm6, zmm5, zmm4	 # AVX512BW
	vpavgw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpavgw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpavgw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpavgw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpavgw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpavgw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpavgw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpavgw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpblendmb	zmm6, zmm5, zmm4	 # AVX512BW
	vpblendmb	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpblendmb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpblendmb	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpblendmb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpblendmb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpblendmb	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpblendmb	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpblendmb	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpbroadcastb	zmm6{k7}, xmm5	 # AVX512BW
	vpbroadcastb	zmm6{k7}{z}, xmm5	 # AVX512BW
	vpbroadcastb	zmm6{k7}, BYTE PTR [ecx]	 # AVX512BW
	vpbroadcastb	zmm6{k7}, BYTE PTR [esp+esi*8-123456]	 # AVX512BW
	vpbroadcastb	zmm6{k7}, BYTE PTR [edx+127]	 # AVX512BW Disp8
	vpbroadcastb	zmm6{k7}, BYTE PTR [edx+128]	 # AVX512BW
	vpbroadcastb	zmm6{k7}, BYTE PTR [edx-128]	 # AVX512BW Disp8
	vpbroadcastb	zmm6{k7}, BYTE PTR [edx-129]	 # AVX512BW
	vpbroadcastb	zmm6, eax	 # AVX512BW
	vpbroadcastb	zmm6{k7}, eax	 # AVX512BW
	vpbroadcastb	zmm6{k7}{z}, eax	 # AVX512BW
	vpbroadcastb	zmm6, ebp	 # AVX512BW
	vpbroadcastw	zmm6{k7}, xmm5	 # AVX512BW
	vpbroadcastw	zmm6{k7}{z}, xmm5	 # AVX512BW
	vpbroadcastw	zmm6{k7}, WORD PTR [ecx]	 # AVX512BW
	vpbroadcastw	zmm6{k7}, WORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpbroadcastw	zmm6{k7}, WORD PTR [edx+254]	 # AVX512BW Disp8
	vpbroadcastw	zmm6{k7}, WORD PTR [edx+256]	 # AVX512BW
	vpbroadcastw	zmm6{k7}, WORD PTR [edx-256]	 # AVX512BW Disp8
	vpbroadcastw	zmm6{k7}, WORD PTR [edx-258]	 # AVX512BW
	vpbroadcastw	zmm6, eax	 # AVX512BW
	vpbroadcastw	zmm6{k7}, eax	 # AVX512BW
	vpbroadcastw	zmm6{k7}{z}, eax	 # AVX512BW
	vpbroadcastw	zmm6, ebp	 # AVX512BW
	vpcmpeqb	k5, zmm6, zmm5	 # AVX512BW
	vpcmpeqb	k5{k7}, zmm6, zmm5	 # AVX512BW
	vpcmpeqb	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512BW
	vpcmpeqb	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpcmpeqb	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpcmpeqb	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpcmpeqb	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpcmpeqb	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpcmpeqw	k5, zmm6, zmm5	 # AVX512BW
	vpcmpeqw	k5{k7}, zmm6, zmm5	 # AVX512BW
	vpcmpeqw	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512BW
	vpcmpeqw	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpcmpeqw	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpcmpeqw	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpcmpeqw	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpcmpeqw	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpcmpgtb	k5, zmm6, zmm5	 # AVX512BW
	vpcmpgtb	k5{k7}, zmm6, zmm5	 # AVX512BW
	vpcmpgtb	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512BW
	vpcmpgtb	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpcmpgtb	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpcmpgtb	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpcmpgtb	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpcmpgtb	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpcmpgtw	k5, zmm6, zmm5	 # AVX512BW
	vpcmpgtw	k5{k7}, zmm6, zmm5	 # AVX512BW
	vpcmpgtw	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512BW
	vpcmpgtw	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpcmpgtw	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpcmpgtw	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpcmpgtw	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpcmpgtw	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpblendmw	zmm6, zmm5, zmm4	 # AVX512BW
	vpblendmw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpblendmw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpblendmw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpblendmw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpblendmw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpblendmw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpblendmw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpblendmw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpmaddubsw	zmm6, zmm5, zmm4	 # AVX512BW
	vpmaddubsw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpmaddubsw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpmaddubsw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpmaddubsw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpmaddubsw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpmaddubsw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpmaddubsw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpmaddubsw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpmaddwd	zmm6, zmm5, zmm4	 # AVX512BW
	vpmaddwd	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpmaddwd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpmaddwd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpmaddwd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpmaddwd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpmaddwd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpmaddwd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpmaddwd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpmaxsb	zmm6, zmm5, zmm4	 # AVX512BW
	vpmaxsb	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpmaxsb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpmaxsb	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpmaxsb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpmaxsb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpmaxsb	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpmaxsb	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpmaxsb	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpmaxsw	zmm6, zmm5, zmm4	 # AVX512BW
	vpmaxsw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpmaxsw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpmaxsw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpmaxsw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpmaxsw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpmaxsw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpmaxsw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpmaxsw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpmaxub	zmm6, zmm5, zmm4	 # AVX512BW
	vpmaxub	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpmaxub	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpmaxub	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpmaxub	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpmaxub	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpmaxub	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpmaxub	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpmaxub	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpmaxuw	zmm6, zmm5, zmm4	 # AVX512BW
	vpmaxuw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpmaxuw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpmaxuw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpmaxuw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpmaxuw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpmaxuw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpmaxuw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpmaxuw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpminsb	zmm6, zmm5, zmm4	 # AVX512BW
	vpminsb	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpminsb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpminsb	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpminsb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpminsb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpminsb	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpminsb	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpminsb	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpminsw	zmm6, zmm5, zmm4	 # AVX512BW
	vpminsw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpminsw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpminsw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpminsw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpminsw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpminsw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpminsw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpminsw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpminub	zmm6, zmm5, zmm4	 # AVX512BW
	vpminub	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpminub	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpminub	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpminub	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpminub	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpminub	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpminub	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpminub	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpminuw	zmm6, zmm5, zmm4	 # AVX512BW
	vpminuw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpminuw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpminuw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpminuw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpminuw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpminuw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpminuw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpminuw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpmovsxbw	zmm6{k7}, ymm5	 # AVX512BW
	vpmovsxbw	zmm6{k7}{z}, ymm5	 # AVX512BW
	vpmovsxbw	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512BW
	vpmovsxbw	zmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpmovsxbw	zmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512BW Disp8
	vpmovsxbw	zmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512BW
	vpmovsxbw	zmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512BW Disp8
	vpmovsxbw	zmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512BW
	vpmovzxbw	zmm6{k7}, ymm5	 # AVX512BW
	vpmovzxbw	zmm6{k7}{z}, ymm5	 # AVX512BW
	vpmovzxbw	zmm6{k7}, YMMWORD PTR [ecx]	 # AVX512BW
	vpmovzxbw	zmm6{k7}, YMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpmovzxbw	zmm6{k7}, YMMWORD PTR [edx+4064]	 # AVX512BW Disp8
	vpmovzxbw	zmm6{k7}, YMMWORD PTR [edx+4096]	 # AVX512BW
	vpmovzxbw	zmm6{k7}, YMMWORD PTR [edx-4096]	 # AVX512BW Disp8
	vpmovzxbw	zmm6{k7}, YMMWORD PTR [edx-4128]	 # AVX512BW
	vpmulhrsw	zmm6, zmm5, zmm4	 # AVX512BW
	vpmulhrsw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpmulhrsw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpmulhrsw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpmulhrsw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpmulhrsw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpmulhrsw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpmulhrsw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpmulhrsw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpmulhuw	zmm6, zmm5, zmm4	 # AVX512BW
	vpmulhuw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpmulhuw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpmulhuw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpmulhuw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpmulhuw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpmulhuw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpmulhuw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpmulhuw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpmulhw	zmm6, zmm5, zmm4	 # AVX512BW
	vpmulhw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpmulhw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpmulhw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpmulhw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpmulhw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpmulhw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpmulhw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpmulhw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpmullw	zmm6, zmm5, zmm4	 # AVX512BW
	vpmullw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpmullw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpmullw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpmullw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpmullw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpmullw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpmullw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpmullw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpsadbw	zmm6, zmm5, zmm4	 # AVX512BW
	vpsadbw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpsadbw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpsadbw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpsadbw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpsadbw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpsadbw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpshufb	zmm6, zmm5, zmm4	 # AVX512BW
	vpshufb	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpshufb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpshufb	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpshufb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpshufb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpshufb	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpshufb	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpshufb	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpshufhw	zmm6, zmm5, 0xab	 # AVX512BW
	vpshufhw	zmm6{k7}, zmm5, 0xab	 # AVX512BW
	vpshufhw	zmm6{k7}{z}, zmm5, 0xab	 # AVX512BW
	vpshufhw	zmm6, zmm5, 123	 # AVX512BW
	vpshufhw	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512BW
	vpshufhw	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512BW
	vpshufhw	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512BW Disp8
	vpshufhw	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512BW
	vpshufhw	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512BW Disp8
	vpshufhw	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512BW
	vpshuflw	zmm6, zmm5, 0xab	 # AVX512BW
	vpshuflw	zmm6{k7}, zmm5, 0xab	 # AVX512BW
	vpshuflw	zmm6{k7}{z}, zmm5, 0xab	 # AVX512BW
	vpshuflw	zmm6, zmm5, 123	 # AVX512BW
	vpshuflw	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512BW
	vpshuflw	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512BW
	vpshuflw	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512BW Disp8
	vpshuflw	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512BW
	vpshuflw	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512BW Disp8
	vpshuflw	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512BW
	vpsllw	zmm6{k7}, zmm5, xmm4	 # AVX512BW
	vpsllw	zmm6{k7}{z}, zmm5, xmm4	 # AVX512BW
	vpsllw	zmm6{k7}, zmm5, XMMWORD PTR [ecx]	 # AVX512BW
	vpsllw	zmm6{k7}, zmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpsllw	zmm6{k7}, zmm5, XMMWORD PTR [edx+2032]	 # AVX512BW Disp8
	vpsllw	zmm6{k7}, zmm5, XMMWORD PTR [edx+2048]	 # AVX512BW
	vpsllw	zmm6{k7}, zmm5, XMMWORD PTR [edx-2048]	 # AVX512BW Disp8
	vpsllw	zmm6{k7}, zmm5, XMMWORD PTR [edx-2064]	 # AVX512BW
	vpsraw	zmm6{k7}, zmm5, xmm4	 # AVX512BW
	vpsraw	zmm6{k7}{z}, zmm5, xmm4	 # AVX512BW
	vpsraw	zmm6{k7}, zmm5, XMMWORD PTR [ecx]	 # AVX512BW
	vpsraw	zmm6{k7}, zmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpsraw	zmm6{k7}, zmm5, XMMWORD PTR [edx+2032]	 # AVX512BW Disp8
	vpsraw	zmm6{k7}, zmm5, XMMWORD PTR [edx+2048]	 # AVX512BW
	vpsraw	zmm6{k7}, zmm5, XMMWORD PTR [edx-2048]	 # AVX512BW Disp8
	vpsraw	zmm6{k7}, zmm5, XMMWORD PTR [edx-2064]	 # AVX512BW
	vpsrlw	zmm6{k7}, zmm5, xmm4	 # AVX512BW
	vpsrlw	zmm6{k7}{z}, zmm5, xmm4	 # AVX512BW
	vpsrlw	zmm6{k7}, zmm5, XMMWORD PTR [ecx]	 # AVX512BW
	vpsrlw	zmm6{k7}, zmm5, XMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpsrlw	zmm6{k7}, zmm5, XMMWORD PTR [edx+2032]	 # AVX512BW Disp8
	vpsrlw	zmm6{k7}, zmm5, XMMWORD PTR [edx+2048]	 # AVX512BW
	vpsrlw	zmm6{k7}, zmm5, XMMWORD PTR [edx-2048]	 # AVX512BW Disp8
	vpsrlw	zmm6{k7}, zmm5, XMMWORD PTR [edx-2064]	 # AVX512BW
	vpsrldq	zmm6, zmm5, 0xab	 # AVX512BW
	vpsrldq	zmm6, zmm5, 123	 # AVX512BW
	vpsrldq	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512BW
	vpsrldq	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512BW
	vpsrldq	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512BW Disp8
	vpsrldq	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512BW
	vpsrldq	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512BW Disp8
	vpsrldq	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512BW
	vpsrlw	zmm6, zmm5, 0xab	 # AVX512BW
	vpsrlw	zmm6{k7}, zmm5, 0xab	 # AVX512BW
	vpsrlw	zmm6{k7}{z}, zmm5, 0xab	 # AVX512BW
	vpsrlw	zmm6, zmm5, 123	 # AVX512BW
	vpsrlw	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512BW
	vpsrlw	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512BW
	vpsrlw	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512BW Disp8
	vpsrlw	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512BW
	vpsrlw	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512BW Disp8
	vpsrlw	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512BW
	vpsraw	zmm6, zmm5, 0xab	 # AVX512BW
	vpsraw	zmm6{k7}, zmm5, 0xab	 # AVX512BW
	vpsraw	zmm6{k7}{z}, zmm5, 0xab	 # AVX512BW
	vpsraw	zmm6, zmm5, 123	 # AVX512BW
	vpsraw	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512BW
	vpsraw	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512BW
	vpsraw	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512BW Disp8
	vpsraw	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512BW
	vpsraw	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512BW Disp8
	vpsraw	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512BW
	vpsrlvw	zmm6, zmm5, zmm4	 # AVX512BW
	vpsrlvw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpsrlvw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpsrlvw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpsrlvw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpsrlvw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpsrlvw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpsrlvw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpsrlvw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpsravw	zmm6, zmm5, zmm4	 # AVX512BW
	vpsravw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpsravw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpsravw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpsravw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpsravw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpsravw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpsravw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpsravw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpsubb	zmm6, zmm5, zmm4	 # AVX512BW
	vpsubb	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpsubb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpsubb	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpsubb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpsubb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpsubb	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpsubb	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpsubb	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpsubsb	zmm6, zmm5, zmm4	 # AVX512BW
	vpsubsb	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpsubsb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpsubsb	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpsubsb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpsubsb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpsubsb	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpsubsb	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpsubsb	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpsubsw	zmm6, zmm5, zmm4	 # AVX512BW
	vpsubsw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpsubsw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpsubsw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpsubsw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpsubsw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpsubsw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpsubsw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpsubsw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpsubusb	zmm6, zmm5, zmm4	 # AVX512BW
	vpsubusb	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpsubusb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpsubusb	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpsubusb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpsubusb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpsubusb	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpsubusb	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpsubusb	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpsubusw	zmm6, zmm5, zmm4	 # AVX512BW
	vpsubusw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpsubusw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpsubusw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpsubusw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpsubusw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpsubusw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpsubusw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpsubusw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpsubw	zmm6, zmm5, zmm4	 # AVX512BW
	vpsubw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpsubw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpsubw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpsubw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpsubw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpsubw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpsubw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpsubw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpunpckhbw	zmm6, zmm5, zmm4	 # AVX512BW
	vpunpckhbw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpunpckhbw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpunpckhbw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpunpckhbw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpunpckhbw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpunpckhbw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpunpckhbw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpunpckhbw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpunpckhwd	zmm6, zmm5, zmm4	 # AVX512BW
	vpunpckhwd	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpunpckhwd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpunpckhwd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpunpckhwd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpunpckhwd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpunpckhwd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpunpckhwd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpunpckhwd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpunpcklbw	zmm6, zmm5, zmm4	 # AVX512BW
	vpunpcklbw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpunpcklbw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpunpcklbw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpunpcklbw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpunpcklbw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpunpcklbw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpunpcklbw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpunpcklbw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpunpcklwd	zmm6, zmm5, zmm4	 # AVX512BW
	vpunpcklwd	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpunpcklwd	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpunpcklwd	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpunpcklwd	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpunpcklwd	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpunpcklwd	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpunpcklwd	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpunpcklwd	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpmovwb	ymm6{k7}, zmm5	 # AVX512BW
	vpmovwb	ymm6{k7}{z}, zmm5	 # AVX512BW
	vpmovswb	ymm6{k7}, zmm5	 # AVX512BW
	vpmovswb	ymm6{k7}{z}, zmm5	 # AVX512BW
	vpmovuswb	ymm6{k7}, zmm5	 # AVX512BW
	vpmovuswb	ymm6{k7}{z}, zmm5	 # AVX512BW
	vdbpsadbw	zmm6, zmm5, zmm4, 0xab	 # AVX512BW
	vdbpsadbw	zmm6{k7}, zmm5, zmm4, 0xab	 # AVX512BW
	vdbpsadbw	zmm6{k7}{z}, zmm5, zmm4, 0xab	 # AVX512BW
	vdbpsadbw	zmm6, zmm5, zmm4, 123	 # AVX512BW
	vdbpsadbw	zmm6, zmm5, ZMMWORD PTR [ecx], 123	 # AVX512BW
	vdbpsadbw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512BW
	vdbpsadbw	zmm6, zmm5, ZMMWORD PTR [edx+8128], 123	 # AVX512BW Disp8
	vdbpsadbw	zmm6, zmm5, ZMMWORD PTR [edx+8192], 123	 # AVX512BW
	vdbpsadbw	zmm6, zmm5, ZMMWORD PTR [edx-8192], 123	 # AVX512BW Disp8
	vdbpsadbw	zmm6, zmm5, ZMMWORD PTR [edx-8256], 123	 # AVX512BW
	vpermw	zmm6, zmm5, zmm4	 # AVX512BW
	vpermw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpermw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpermw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpermw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpermw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpermw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpermw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpermw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpermt2w	zmm6, zmm5, zmm4	 # AVX512BW
	vpermt2w	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpermt2w	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpermt2w	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpermt2w	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpermt2w	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpermt2w	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpermt2w	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpermt2w	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpslldq	zmm6, zmm5, 0xab	 # AVX512BW
	vpslldq	zmm6, zmm5, 123	 # AVX512BW
	vpslldq	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512BW
	vpslldq	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512BW
	vpslldq	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512BW Disp8
	vpslldq	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512BW
	vpslldq	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512BW Disp8
	vpslldq	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512BW
	vpsllw	zmm6, zmm5, 0xab	 # AVX512BW
	vpsllw	zmm6{k7}, zmm5, 0xab	 # AVX512BW
	vpsllw	zmm6{k7}{z}, zmm5, 0xab	 # AVX512BW
	vpsllw	zmm6, zmm5, 123	 # AVX512BW
	vpsllw	zmm6, ZMMWORD PTR [ecx], 123	 # AVX512BW
	vpsllw	zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512BW
	vpsllw	zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512BW Disp8
	vpsllw	zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512BW
	vpsllw	zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512BW Disp8
	vpsllw	zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512BW
	vpsllvw	zmm6, zmm5, zmm4	 # AVX512BW
	vpsllvw	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpsllvw	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpsllvw	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpsllvw	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpsllvw	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpsllvw	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpsllvw	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpsllvw	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vmovdqu8	zmm6, zmm5	 # AVX512BW
	vmovdqu8	zmm6{k7}, zmm5	 # AVX512BW
	vmovdqu8	zmm6{k7}{z}, zmm5	 # AVX512BW
	vmovdqu8	zmm6, ZMMWORD PTR [ecx]	 # AVX512BW
	vmovdqu8	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vmovdqu8	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vmovdqu8	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vmovdqu8	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vmovdqu8	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vmovdqu16	zmm6, zmm5	 # AVX512BW
	vmovdqu16	zmm6{k7}, zmm5	 # AVX512BW
	vmovdqu16	zmm6{k7}{z}, zmm5	 # AVX512BW
	vmovdqu16	zmm6, ZMMWORD PTR [ecx]	 # AVX512BW
	vmovdqu16	zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vmovdqu16	zmm6, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vmovdqu16	zmm6, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vmovdqu16	zmm6, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vmovdqu16	zmm6, ZMMWORD PTR [edx-8256]	 # AVX512BW
	kandq	k5, k6, k7	 # AVX512BW
	kandd	k5, k6, k7	 # AVX512BW
	kandnq	k5, k6, k7	 # AVX512BW
	kandnd	k5, k6, k7	 # AVX512BW
	korq	k5, k6, k7	 # AVX512BW
	kord	k5, k6, k7	 # AVX512BW
	kxnorq	k5, k6, k7	 # AVX512BW
	kxnord	k5, k6, k7	 # AVX512BW
	kxorq	k5, k6, k7	 # AVX512BW
	kxord	k5, k6, k7	 # AVX512BW
	knotq	k5, k6	 # AVX512BW
	knotd	k5, k6	 # AVX512BW
	kortestq	k5, k6	 # AVX512BW
	kortestd	k5, k6	 # AVX512BW
	ktestq	k5, k6	 # AVX512BW
	ktestd	k5, k6	 # AVX512BW
	kshiftrq	k5, k6, 0xab	 # AVX512BW
	kshiftrq	k5, k6, 123	 # AVX512BW
	kshiftrd	k5, k6, 0xab	 # AVX512BW
	kshiftrd	k5, k6, 123	 # AVX512BW
	kshiftlq	k5, k6, 0xab	 # AVX512BW
	kshiftlq	k5, k6, 123	 # AVX512BW
	kshiftld	k5, k6, 0xab	 # AVX512BW
	kshiftld	k5, k6, 123	 # AVX512BW
	kmovq	k5, k6	 # AVX512BW
	kmovq	k5, QWORD PTR [ecx]	 # AVX512BW
	kmovq	k5, QWORD PTR [esp+esi*8-123456]	 # AVX512BW
	kmovd	k5, k6	 # AVX512BW
	kmovd	k5, DWORD PTR [ecx]	 # AVX512BW
	kmovd	k5, DWORD PTR [esp+esi*8-123456]	 # AVX512BW
	kmovq	QWORD PTR [ecx], k5	 # AVX512BW
	kmovq	QWORD PTR [esp+esi*8-123456], k5	 # AVX512BW
	kmovd	DWORD PTR [ecx], k5	 # AVX512BW
	kmovd	DWORD PTR [esp+esi*8-123456], k5	 # AVX512BW
	kmovd	k5, eax	 # AVX512BW
	kmovd	k5, ebp	 # AVX512BW
	kmovd	eax, k5	 # AVX512BW
	kmovd	ebp, k5	 # AVX512BW
	kaddq	k5, k6, k7	 # AVX512BW
	kaddd	k5, k6, k7	 # AVX512BW
	kunpckwd	k5, k6, k7	 # AVX512BW
	kunpckdq	k5, k6, k7	 # AVX512BW
	vpmovwb	YMMWORD PTR [ecx], zmm6	 # AVX512BW
	vpmovwb	YMMWORD PTR [ecx]{k7}, zmm6	 # AVX512BW
	vpmovwb	YMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512BW
	vpmovwb	YMMWORD PTR [edx+4064], zmm6	 # AVX512BW Disp8
	vpmovwb	YMMWORD PTR [edx+4096], zmm6	 # AVX512BW
	vpmovwb	YMMWORD PTR [edx-4096], zmm6	 # AVX512BW Disp8
	vpmovwb	YMMWORD PTR [edx-4128], zmm6	 # AVX512BW
	vpmovswb	YMMWORD PTR [ecx], zmm6	 # AVX512BW
	vpmovswb	YMMWORD PTR [ecx]{k7}, zmm6	 # AVX512BW
	vpmovswb	YMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512BW
	vpmovswb	YMMWORD PTR [edx+4064], zmm6	 # AVX512BW Disp8
	vpmovswb	YMMWORD PTR [edx+4096], zmm6	 # AVX512BW
	vpmovswb	YMMWORD PTR [edx-4096], zmm6	 # AVX512BW Disp8
	vpmovswb	YMMWORD PTR [edx-4128], zmm6	 # AVX512BW
	vpmovuswb	YMMWORD PTR [ecx], zmm6	 # AVX512BW
	vpmovuswb	YMMWORD PTR [ecx]{k7}, zmm6	 # AVX512BW
	vpmovuswb	YMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512BW
	vpmovuswb	YMMWORD PTR [edx+4064], zmm6	 # AVX512BW Disp8
	vpmovuswb	YMMWORD PTR [edx+4096], zmm6	 # AVX512BW
	vpmovuswb	YMMWORD PTR [edx-4096], zmm6	 # AVX512BW Disp8
	vpmovuswb	YMMWORD PTR [edx-4128], zmm6	 # AVX512BW
	vmovdqu8	ZMMWORD PTR [ecx], zmm6	 # AVX512BW
	vmovdqu8	ZMMWORD PTR [ecx]{k7}, zmm6	 # AVX512BW
	vmovdqu8	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512BW
	vmovdqu8	ZMMWORD PTR [edx+8128], zmm6	 # AVX512BW Disp8
	vmovdqu8	ZMMWORD PTR [edx+8192], zmm6	 # AVX512BW
	vmovdqu8	ZMMWORD PTR [edx-8192], zmm6	 # AVX512BW Disp8
	vmovdqu8	ZMMWORD PTR [edx-8256], zmm6	 # AVX512BW
	vmovdqu16	ZMMWORD PTR [ecx], zmm6	 # AVX512BW
	vmovdqu16	ZMMWORD PTR [ecx]{k7}, zmm6	 # AVX512BW
	vmovdqu16	ZMMWORD PTR [esp+esi*8-123456], zmm6	 # AVX512BW
	vmovdqu16	ZMMWORD PTR [edx+8128], zmm6	 # AVX512BW Disp8
	vmovdqu16	ZMMWORD PTR [edx+8192], zmm6	 # AVX512BW
	vmovdqu16	ZMMWORD PTR [edx-8192], zmm6	 # AVX512BW Disp8
	vmovdqu16	ZMMWORD PTR [edx-8256], zmm6	 # AVX512BW
	vpermi2w	zmm6, zmm5, zmm4	 # AVX512BW
	vpermi2w	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpermi2w	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpermi2w	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpermi2w	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpermi2w	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpermi2w	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpermi2w	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpermi2w	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vptestmb	k5, zmm6, zmm5	 # AVX512BW
	vptestmb	k5{k7}, zmm6, zmm5	 # AVX512BW
	vptestmb	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512BW
	vptestmb	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vptestmb	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vptestmb	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vptestmb	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vptestmb	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vptestmw	k5, zmm6, zmm5	 # AVX512BW
	vptestmw	k5{k7}, zmm6, zmm5	 # AVX512BW
	vptestmw	k5, zmm6, ZMMWORD PTR [ecx]	 # AVX512BW
	vptestmw	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vptestmw	k5, zmm6, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vptestmw	k5, zmm6, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vptestmw	k5, zmm6, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vptestmw	k5, zmm6, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpmovb2m	k5, zmm6	 # AVX512BW
	vpmovw2m	k5, zmm6	 # AVX512BW
	vpmovm2b	zmm6, k5	 # AVX512BW
	vpmovm2w	zmm6, k5	 # AVX512BW
	vptestnmb	k5, zmm5, zmm4	 # AVX512BW
	vptestnmb	k5{k7}, zmm5, zmm4	 # AVX512BW
	vptestnmb	k5, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vptestnmb	k5, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vptestnmb	k5, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vptestnmb	k5, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vptestnmb	k5, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vptestnmb	k5, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vptestnmw	k5, zmm5, zmm4	 # AVX512BW
	vptestnmw	k5{k7}, zmm5, zmm4	 # AVX512BW
	vptestnmw	k5, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vptestnmw	k5, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vptestnmw	k5, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vptestnmw	k5, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vptestnmw	k5, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vptestnmw	k5, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
	vpcmpb	k5, zmm6, zmm5, 0xab	 # AVX512BW
	vpcmpb	k5{k7}, zmm6, zmm5, 0xab	 # AVX512BW
	vpcmpb	k5, zmm6, zmm5, 123	 # AVX512BW
	vpcmpb	k5, zmm6, ZMMWORD PTR [ecx], 123	 # AVX512BW
	vpcmpb	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512BW
	vpcmpb	k5, zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512BW Disp8
	vpcmpb	k5, zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512BW
	vpcmpb	k5, zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512BW Disp8
	vpcmpb	k5, zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512BW
	vpcmpw	k5, zmm6, zmm5, 0xab	 # AVX512BW
	vpcmpw	k5{k7}, zmm6, zmm5, 0xab	 # AVX512BW
	vpcmpw	k5, zmm6, zmm5, 123	 # AVX512BW
	vpcmpw	k5, zmm6, ZMMWORD PTR [ecx], 123	 # AVX512BW
	vpcmpw	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512BW
	vpcmpw	k5, zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512BW Disp8
	vpcmpw	k5, zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512BW
	vpcmpw	k5, zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512BW Disp8
	vpcmpw	k5, zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512BW
	vpcmpub	k5, zmm6, zmm5, 0xab	 # AVX512BW
	vpcmpub	k5{k7}, zmm6, zmm5, 0xab	 # AVX512BW
	vpcmpub	k5, zmm6, zmm5, 123	 # AVX512BW
	vpcmpub	k5, zmm6, ZMMWORD PTR [ecx], 123	 # AVX512BW
	vpcmpub	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512BW
	vpcmpub	k5, zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512BW Disp8
	vpcmpub	k5, zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512BW
	vpcmpub	k5, zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512BW Disp8
	vpcmpub	k5, zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512BW
	vpcmpuw	k5, zmm6, zmm5, 0xab	 # AVX512BW
	vpcmpuw	k5{k7}, zmm6, zmm5, 0xab	 # AVX512BW
	vpcmpuw	k5, zmm6, zmm5, 123	 # AVX512BW
	vpcmpuw	k5, zmm6, ZMMWORD PTR [ecx], 123	 # AVX512BW
	vpcmpuw	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456], 123	 # AVX512BW
	vpcmpuw	k5, zmm6, ZMMWORD PTR [edx+8128], 123	 # AVX512BW Disp8
	vpcmpuw	k5, zmm6, ZMMWORD PTR [edx+8192], 123	 # AVX512BW
	vpcmpuw	k5, zmm6, ZMMWORD PTR [edx-8192], 123	 # AVX512BW Disp8
	vpcmpuw	k5, zmm6, ZMMWORD PTR [edx-8256], 123	 # AVX512BW
