# Check 32bit AVX512BW WIG instructions

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
	vpacksswb	%zmm4, %zmm5, %zmm6	 # AVX512BW
	vpacksswb	%zmm4, %zmm5, %zmm6{%k7}	 # AVX512BW
	vpacksswb	%zmm4, %zmm5, %zmm6{%k7}{z}	 # AVX512BW
	vpacksswb	(%ecx), %zmm5, %zmm6	 # AVX512BW
	vpacksswb	-123456(%esp,%esi,8), %zmm5, %zmm6	 # AVX512BW
	vpacksswb	8128(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpacksswb	8192(%edx), %zmm5, %zmm6	 # AVX512BW
	vpacksswb	-8192(%edx), %zmm5, %zmm6	 # AVX512BW Disp8
	vpacksswb	-8256(%edx), %zmm5, %zmm6	 # AVX512BW
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
	vpacksswb	zmm6, zmm5, zmm4	 # AVX512BW
	vpacksswb	zmm6{k7}, zmm5, zmm4	 # AVX512BW
	vpacksswb	zmm6{k7}{z}, zmm5, zmm4	 # AVX512BW
	vpacksswb	zmm6, zmm5, ZMMWORD PTR [ecx]	 # AVX512BW
	vpacksswb	zmm6, zmm5, ZMMWORD PTR [esp+esi*8-123456]	 # AVX512BW
	vpacksswb	zmm6, zmm5, ZMMWORD PTR [edx+8128]	 # AVX512BW Disp8
	vpacksswb	zmm6, zmm5, ZMMWORD PTR [edx+8192]	 # AVX512BW
	vpacksswb	zmm6, zmm5, ZMMWORD PTR [edx-8192]	 # AVX512BW Disp8
	vpacksswb	zmm6, zmm5, ZMMWORD PTR [edx-8256]	 # AVX512BW
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
