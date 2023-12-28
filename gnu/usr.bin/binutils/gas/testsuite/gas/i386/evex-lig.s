# Check EVEX LIG instructions

	.allow_index_reg
	.text
_start:

	vaddsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vaddsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vaddsd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vaddsd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vaddsd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vaddsd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vaddsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vaddsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vaddsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vaddsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vaddsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vaddsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vaddss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vaddss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vaddss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vaddss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vaddss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vaddss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vaddss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vaddss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vaddss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vaddss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vaddss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vaddss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vcmpsd	$0xab, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpsd	$0xab, {sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpsd	$123, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpsd	$123, {sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpsd	$123, (%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpsd	$123, -123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpsd	$123, 1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpsd	$123, 1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpsd	$123, -1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpsd	$123, -1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpeq_oqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_oqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_oqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_oqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_oqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_oqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_oqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_oqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpeqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpeqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmplt_ossd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmplt_ossd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmplt_ossd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmplt_ossd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmplt_ossd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmplt_ossd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmplt_ossd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmplt_ossd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpltsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpltsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpltsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpltsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpltsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpltsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpltsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpltsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmple_ossd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmple_ossd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmple_ossd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmple_ossd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmple_ossd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmple_ossd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmple_ossd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmple_ossd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmplesd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmplesd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmplesd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmplesd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmplesd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmplesd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmplesd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmplesd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpunord_qsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_qsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_qsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_qsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_qsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpunord_qsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_qsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpunord_qsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpunordsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpunordsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpunordsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpunordsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpunordsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpunordsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpunordsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpunordsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpneq_uqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_uqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_uqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_uqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_uqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_uqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_uqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_uqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpneqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpneqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpnlt_ussd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_ussd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_ussd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_ussd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_ussd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnlt_ussd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_ussd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnlt_ussd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpnltsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnltsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnltsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnltsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpnltsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnltsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnltsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnltsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpnle_ussd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_ussd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_ussd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_ussd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_ussd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnle_ussd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_ussd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnle_ussd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpnlesd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnlesd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnlesd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnlesd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpnlesd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnlesd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnlesd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnlesd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpord_qsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpord_qsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpord_qsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpord_qsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpord_qsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpord_qsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpord_qsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpord_qsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpordsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpordsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpordsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpordsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpordsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpordsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpordsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpordsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpeq_uqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_uqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_uqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_uqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_uqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_uqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_uqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_uqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpnge_ussd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_ussd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_ussd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_ussd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_ussd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnge_ussd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_ussd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnge_ussd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpngesd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpngesd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpngesd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpngesd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpngesd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpngesd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpngesd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpngesd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpngt_ussd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_ussd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_ussd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_ussd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_ussd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpngt_ussd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_ussd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpngt_ussd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpngtsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpngtsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpngtsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpngtsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpngtsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpngtsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpngtsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpngtsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpfalse_oqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_oqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_oqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_oqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_oqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpfalse_oqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_oqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpfalse_oqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpfalsesd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpfalsesd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpfalsesd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalsesd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalsesd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpfalsesd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalsesd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpfalsesd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpneq_oqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_oqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_oqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_oqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_oqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_oqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_oqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_oqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpge_ossd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpge_ossd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpge_ossd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpge_ossd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpge_ossd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpge_ossd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpge_ossd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpge_ossd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpgesd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpgesd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpgesd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpgesd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpgesd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpgesd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpgesd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpgesd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpgt_ossd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_ossd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_ossd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_ossd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_ossd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpgt_ossd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_ossd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpgt_ossd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpgtsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpgtsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpgtsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpgtsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpgtsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpgtsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpgtsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpgtsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmptrue_uqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_uqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_uqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_uqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_uqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmptrue_uqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_uqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmptrue_uqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmptruesd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmptruesd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmptruesd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmptruesd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmptruesd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmptruesd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmptruesd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmptruesd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpeq_ossd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_ossd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_ossd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_ossd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_ossd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_ossd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_ossd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_ossd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmplt_oqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmplt_oqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmplt_oqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmplt_oqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmplt_oqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmplt_oqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmplt_oqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmplt_oqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmple_oqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmple_oqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmple_oqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmple_oqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmple_oqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmple_oqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmple_oqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmple_oqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpunord_ssd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_ssd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_ssd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_ssd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_ssd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpunord_ssd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_ssd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpunord_ssd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpneq_ussd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_ussd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_ussd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_ussd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_ussd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_ussd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_ussd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_ussd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpnlt_uqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_uqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_uqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_uqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_uqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnlt_uqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_uqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnlt_uqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpnle_uqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_uqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_uqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_uqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_uqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnle_uqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_uqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnle_uqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpord_ssd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpord_ssd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpord_ssd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpord_ssd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpord_ssd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpord_ssd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpord_ssd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpord_ssd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpeq_ussd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_ussd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_ussd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_ussd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_ussd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_ussd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_ussd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_ussd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpnge_uqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_uqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_uqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_uqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_uqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnge_uqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_uqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnge_uqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpngt_uqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_uqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_uqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_uqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_uqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpngt_uqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_uqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpngt_uqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpfalse_ossd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_ossd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_ossd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_ossd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_ossd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpfalse_ossd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_ossd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpfalse_ossd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpneq_ossd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_ossd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_ossd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_ossd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_ossd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_ossd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_ossd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_ossd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpge_oqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpge_oqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpge_oqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpge_oqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpge_oqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpge_oqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpge_oqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpge_oqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpgt_oqsd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_oqsd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_oqsd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_oqsd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_oqsd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpgt_oqsd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_oqsd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpgt_oqsd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmptrue_ussd	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_ussd	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_ussd	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_ussd	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_ussd	1016(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmptrue_ussd	1024(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_ussd	-1024(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmptrue_ussd	-1032(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpss	$0xab, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpss	$0xab, {sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpss	$123, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpss	$123, {sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpss	$123, (%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpss	$123, -123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpss	$123, 508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpss	$123, 512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpss	$123, -512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpss	$123, -516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpeq_oqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_oqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_oqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_oqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_oqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_oqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_oqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_oqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpeqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpeqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmplt_osss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmplt_osss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmplt_osss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmplt_osss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmplt_osss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmplt_osss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmplt_osss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmplt_osss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpltss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpltss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpltss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpltss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpltss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpltss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpltss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpltss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmple_osss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmple_osss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmple_osss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmple_osss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmple_osss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmple_osss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmple_osss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmple_osss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpless	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpless	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpless	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpless	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpless	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpless	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpless	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpless	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpunord_qss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_qss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_qss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_qss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_qss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpunord_qss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_qss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpunord_qss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpunordss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpunordss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpunordss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpunordss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpunordss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpunordss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpunordss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpunordss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpneq_uqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_uqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_uqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_uqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_uqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_uqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_uqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_uqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpneqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpneqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpnlt_usss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_usss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_usss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_usss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_usss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnlt_usss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_usss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnlt_usss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpnltss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnltss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnltss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnltss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpnltss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnltss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnltss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnltss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpnle_usss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_usss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_usss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_usss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_usss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnle_usss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_usss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnle_usss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpnless	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnless	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnless	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnless	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpnless	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnless	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnless	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnless	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpord_qss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpord_qss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpord_qss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpord_qss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpord_qss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpord_qss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpord_qss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpord_qss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpordss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpordss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpordss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpordss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpordss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpordss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpordss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpordss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpeq_uqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_uqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_uqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_uqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_uqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_uqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_uqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_uqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpnge_usss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_usss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_usss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_usss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_usss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnge_usss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_usss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnge_usss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpngess	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpngess	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpngess	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpngess	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpngess	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpngess	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpngess	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpngess	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpngt_usss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_usss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_usss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_usss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_usss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpngt_usss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_usss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpngt_usss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpngtss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpngtss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpngtss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpngtss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpngtss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpngtss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpngtss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpngtss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpfalse_oqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_oqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_oqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_oqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_oqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpfalse_oqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_oqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpfalse_oqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpfalsess	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpfalsess	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpfalsess	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalsess	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalsess	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpfalsess	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalsess	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpfalsess	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpneq_oqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_oqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_oqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_oqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_oqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_oqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_oqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_oqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpge_osss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpge_osss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpge_osss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpge_osss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpge_osss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpge_osss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpge_osss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpge_osss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpgess	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpgess	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpgess	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpgess	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpgess	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpgess	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpgess	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpgess	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpgt_osss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_osss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_osss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_osss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_osss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpgt_osss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_osss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpgt_osss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpgtss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpgtss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpgtss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpgtss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpgtss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpgtss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpgtss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpgtss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmptrue_uqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_uqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_uqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_uqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_uqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmptrue_uqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_uqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmptrue_uqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmptruess	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmptruess	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmptruess	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmptruess	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmptruess	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmptruess	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmptruess	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmptruess	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpeq_osss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_osss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_osss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_osss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_osss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_osss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_osss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_osss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmplt_oqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmplt_oqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmplt_oqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmplt_oqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmplt_oqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmplt_oqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmplt_oqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmplt_oqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmple_oqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmple_oqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmple_oqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmple_oqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmple_oqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmple_oqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmple_oqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmple_oqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpunord_sss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_sss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_sss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_sss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_sss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpunord_sss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpunord_sss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpunord_sss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpneq_usss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_usss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_usss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_usss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_usss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_usss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_usss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_usss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpnlt_uqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_uqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_uqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_uqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_uqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnlt_uqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnlt_uqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnlt_uqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpnle_uqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_uqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_uqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_uqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_uqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnle_uqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnle_uqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnle_uqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpord_sss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpord_sss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpord_sss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpord_sss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpord_sss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpord_sss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpord_sss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpord_sss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpeq_usss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_usss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_usss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_usss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_usss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_usss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpeq_usss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_usss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpnge_uqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_uqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_uqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_uqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_uqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnge_uqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpnge_uqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpnge_uqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpngt_uqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_uqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_uqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_uqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_uqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpngt_uqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpngt_uqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpngt_uqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpfalse_osss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_osss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_osss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_osss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_osss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpfalse_osss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpfalse_osss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpfalse_osss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpneq_osss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_osss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_osss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_osss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_osss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_osss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpneq_osss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_osss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpge_oqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpge_oqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpge_oqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpge_oqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpge_oqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpge_oqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpge_oqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpge_oqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmpgt_oqss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_oqss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_oqss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_oqss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_oqss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpgt_oqss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmpgt_oqss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmpgt_oqss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcmptrue_usss	%xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_usss	{sae}, %xmm4, %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_usss	(%ecx), %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_usss	-123456(%esp,%esi,8), %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_usss	508(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmptrue_usss	512(%edx), %xmm5, %k5{%k7}	 # AVX512
	vcmptrue_usss	-512(%edx), %xmm5, %k5{%k7}	 # AVX512 Disp8
	vcmptrue_usss	-516(%edx), %xmm5, %k5{%k7}	 # AVX512

	vcomisd	{sae}, %xmm5, %xmm6	 # AVX512

	vcomiss	{sae}, %xmm5, %xmm6	 # AVX512

	vcvtsd2si	{rn-sae}, %xmm6, %eax	 # AVX512
	vcvtsd2si	{ru-sae}, %xmm6, %eax	 # AVX512
	vcvtsd2si	{rd-sae}, %xmm6, %eax	 # AVX512
	vcvtsd2si	{rz-sae}, %xmm6, %eax	 # AVX512
	vcvtsd2si	{rn-sae}, %xmm6, %ebp	 # AVX512
	vcvtsd2si	{ru-sae}, %xmm6, %ebp	 # AVX512
	vcvtsd2si	{rd-sae}, %xmm6, %ebp	 # AVX512
	vcvtsd2si	{rz-sae}, %xmm6, %ebp	 # AVX512

	vcvtsd2ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vcvtsd2ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vcvtsd2ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vcvtsd2ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vcvtsd2ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vcvtsd2ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vcvtsd2ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vcvtsd2ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vcvtsd2ss	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vcvtsd2ss	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vcvtsd2ss	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vcvtsd2ss	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512


	vcvtsi2ssl	%eax, {rn-sae}, %xmm5, %xmm6	 # AVX512
	vcvtsi2ssl	%eax, {ru-sae}, %xmm5, %xmm6	 # AVX512
	vcvtsi2ssl	%eax, {rd-sae}, %xmm5, %xmm6	 # AVX512
	vcvtsi2ssl	%eax, {rz-sae}, %xmm5, %xmm6	 # AVX512
	vcvtsi2ssl	%ebp, {rn-sae}, %xmm5, %xmm6	 # AVX512
	vcvtsi2ssl	%ebp, {ru-sae}, %xmm5, %xmm6	 # AVX512
	vcvtsi2ssl	%ebp, {rd-sae}, %xmm5, %xmm6	 # AVX512
	vcvtsi2ssl	%ebp, {rz-sae}, %xmm5, %xmm6	 # AVX512

	vcvtss2sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vcvtss2sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vcvtss2sd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vcvtss2sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vcvtss2sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vcvtss2sd	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vcvtss2sd	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vcvtss2sd	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vcvtss2sd	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vcvtss2si	{rn-sae}, %xmm6, %eax	 # AVX512
	vcvtss2si	{ru-sae}, %xmm6, %eax	 # AVX512
	vcvtss2si	{rd-sae}, %xmm6, %eax	 # AVX512
	vcvtss2si	{rz-sae}, %xmm6, %eax	 # AVX512
	vcvtss2si	{rn-sae}, %xmm6, %ebp	 # AVX512
	vcvtss2si	{ru-sae}, %xmm6, %ebp	 # AVX512
	vcvtss2si	{rd-sae}, %xmm6, %ebp	 # AVX512
	vcvtss2si	{rz-sae}, %xmm6, %ebp	 # AVX512

	vcvttsd2si	{sae}, %xmm6, %eax	 # AVX512
	vcvttsd2si	{sae}, %xmm6, %ebp	 # AVX512

	vcvttss2si	{sae}, %xmm6, %eax	 # AVX512
	vcvttss2si	{sae}, %xmm6, %ebp	 # AVX512

	vdivsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vdivsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vdivsd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vdivsd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vdivsd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vdivsd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vdivsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vdivsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vdivsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vdivsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vdivsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vdivsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vdivss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vdivss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vdivss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vdivss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vdivss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vdivss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vdivss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vdivss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vdivss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vdivss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vdivss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vdivss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfmadd132sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd132sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfmadd132sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd132sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd132sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd132sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd132sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd132sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd132sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmadd132sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd132sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmadd132sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfmadd132ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd132ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfmadd132ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd132ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd132ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd132ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd132ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd132ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd132ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmadd132ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd132ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmadd132ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfmadd213sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd213sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfmadd213sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd213sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd213sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd213sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd213sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd213sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd213sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmadd213sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd213sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmadd213sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfmadd213ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd213ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfmadd213ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd213ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd213ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd213ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd213ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd213ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd213ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmadd213ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd213ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmadd213ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfmadd231sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd231sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfmadd231sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd231sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd231sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd231sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd231sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd231sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd231sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmadd231sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd231sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmadd231sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfmadd231ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd231ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfmadd231ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd231ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd231ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd231ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd231ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd231ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd231ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmadd231ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmadd231ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmadd231ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfmsub132sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub132sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfmsub132sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub132sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub132sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub132sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub132sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub132sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub132sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmsub132sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub132sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmsub132sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfmsub132ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub132ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfmsub132ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub132ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub132ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub132ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub132ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub132ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub132ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmsub132ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub132ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmsub132ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfmsub213sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub213sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfmsub213sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub213sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub213sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub213sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub213sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub213sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub213sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmsub213sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub213sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmsub213sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfmsub213ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub213ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfmsub213ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub213ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub213ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub213ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub213ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub213ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub213ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmsub213ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub213ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmsub213ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfmsub231sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub231sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfmsub231sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub231sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub231sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub231sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub231sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub231sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub231sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmsub231sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub231sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmsub231sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfmsub231ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub231ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfmsub231ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub231ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub231ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub231ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub231ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub231ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub231ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmsub231ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfmsub231ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfmsub231ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfnmadd132sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd132sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfnmadd132sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd132sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd132sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd132sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd132sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd132sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd132sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmadd132sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd132sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmadd132sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfnmadd132ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd132ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfnmadd132ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd132ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd132ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd132ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd132ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd132ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd132ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmadd132ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd132ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmadd132ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfnmadd213sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd213sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfnmadd213sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd213sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd213sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd213sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd213sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd213sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd213sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmadd213sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd213sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmadd213sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfnmadd213ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd213ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfnmadd213ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd213ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd213ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd213ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd213ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd213ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd213ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmadd213ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd213ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmadd213ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfnmadd231sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd231sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfnmadd231sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd231sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd231sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd231sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd231sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd231sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd231sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmadd231sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd231sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmadd231sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfnmadd231ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd231ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfnmadd231ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd231ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd231ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd231ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd231ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd231ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd231ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmadd231ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmadd231ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmadd231ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfnmsub132sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub132sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfnmsub132sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub132sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub132sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub132sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub132sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub132sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub132sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmsub132sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub132sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmsub132sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfnmsub132ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub132ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfnmsub132ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub132ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub132ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub132ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub132ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub132ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub132ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmsub132ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub132ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmsub132ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfnmsub213sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub213sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfnmsub213sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub213sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub213sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub213sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub213sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub213sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub213sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmsub213sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub213sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmsub213sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfnmsub213ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub213ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfnmsub213ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub213ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub213ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub213ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub213ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub213ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub213ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmsub213ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub213ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmsub213ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfnmsub231sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub231sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfnmsub231sd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub231sd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub231sd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub231sd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub231sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub231sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub231sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmsub231sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub231sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmsub231sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfnmsub231ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub231ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfnmsub231ss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub231ss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub231ss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub231ss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub231ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub231ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub231ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmsub231ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfnmsub231ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfnmsub231ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vgetexpsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vgetexpsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vgetexpsd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vgetexpsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vgetexpsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vgetexpsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vgetexpsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vgetexpsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vgetexpsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vgetexpss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vgetexpss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vgetexpss	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vgetexpss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vgetexpss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vgetexpss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vgetexpss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vgetexpss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vgetexpss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vgetmantsd	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vgetmantsd	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vgetmantsd	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vgetmantsd	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vgetmantsd	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vgetmantsd	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vgetmantsd	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vgetmantsd	$123, 1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vgetmantsd	$123, 1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vgetmantsd	$123, -1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vgetmantsd	$123, -1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vgetmantss	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vgetmantss	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vgetmantss	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vgetmantss	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vgetmantss	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vgetmantss	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vgetmantss	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vgetmantss	$123, 508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vgetmantss	$123, 512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vgetmantss	$123, -512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vgetmantss	$123, -516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vmaxsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vmaxsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vmaxsd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vmaxsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vmaxsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vmaxsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vmaxsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vmaxsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vmaxsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vmaxss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vmaxss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vmaxss	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vmaxss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vmaxss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vmaxss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vmaxss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vmaxss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vmaxss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vminsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vminsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vminsd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vminsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vminsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vminsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vminsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vminsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vminsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vminss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vminss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vminss	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vminss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vminss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vminss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vminss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vminss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vminss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vmovsd	(%ecx), %xmm6{%k7}	 # AVX512
	vmovsd	(%ecx), %xmm6{%k7}{z}	 # AVX512
	vmovsd	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512
	vmovsd	1016(%edx), %xmm6{%k7}	 # AVX512 Disp8
	vmovsd	1024(%edx), %xmm6{%k7}	 # AVX512
	vmovsd	-1024(%edx), %xmm6{%k7}	 # AVX512 Disp8
	vmovsd	-1032(%edx), %xmm6{%k7}	 # AVX512

	vmovsd	%xmm6, (%ecx){%k7}	 # AVX512
	vmovsd	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512
	vmovsd	%xmm6, 1016(%edx){%k7}	 # AVX512 Disp8
	vmovsd	%xmm6, 1024(%edx){%k7}	 # AVX512
	vmovsd	%xmm6, -1024(%edx){%k7}	 # AVX512 Disp8
	vmovsd	%xmm6, -1032(%edx){%k7}	 # AVX512

	vmovsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vmovsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512

	vmovss	(%ecx), %xmm6{%k7}	 # AVX512
	vmovss	(%ecx), %xmm6{%k7}{z}	 # AVX512
	vmovss	-123456(%esp,%esi,8), %xmm6{%k7}	 # AVX512
	vmovss	508(%edx), %xmm6{%k7}	 # AVX512 Disp8
	vmovss	512(%edx), %xmm6{%k7}	 # AVX512
	vmovss	-512(%edx), %xmm6{%k7}	 # AVX512 Disp8
	vmovss	-516(%edx), %xmm6{%k7}	 # AVX512

	vmovss	%xmm6, (%ecx){%k7}	 # AVX512
	vmovss	%xmm6, -123456(%esp,%esi,8){%k7}	 # AVX512
	vmovss	%xmm6, 508(%edx){%k7}	 # AVX512 Disp8
	vmovss	%xmm6, 512(%edx){%k7}	 # AVX512
	vmovss	%xmm6, -512(%edx){%k7}	 # AVX512 Disp8
	vmovss	%xmm6, -516(%edx){%k7}	 # AVX512

	vmovss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vmovss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512

	vmulsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vmulsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vmulsd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vmulsd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vmulsd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vmulsd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vmulsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vmulsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vmulsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vmulsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vmulsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vmulsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vmulss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vmulss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vmulss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vmulss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vmulss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vmulss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vmulss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vmulss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vmulss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vmulss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vmulss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vmulss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vrcp14sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vrcp14sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vrcp14sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vrcp14sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vrcp14sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vrcp14sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vrcp14sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vrcp14sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vrcp14ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vrcp14ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vrcp14ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vrcp14ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vrcp14ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vrcp14ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vrcp14ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vrcp14ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vrcp28ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrcp28ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512EMI
	vrcp28ss	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrcp28ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrcp28ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrcp28ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512EMI Disp8
	vrcp28ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrcp28ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512EMI Disp8
	vrcp28ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512EMI

	vrcp28sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrcp28sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512EMI
	vrcp28sd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrcp28sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrcp28sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrcp28sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512EMI Disp8
	vrcp28sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrcp28sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512EMI Disp8
	vrcp28sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512EMI

	vrsqrt14sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vrsqrt14sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vrsqrt14sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vrsqrt14sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vrsqrt14sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vrsqrt14sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vrsqrt14sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vrsqrt14sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vrsqrt14ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vrsqrt14ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vrsqrt14ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vrsqrt14ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vrsqrt14ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vrsqrt14ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vrsqrt14ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vrsqrt14ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vrsqrt28ss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrsqrt28ss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512EMI
	vrsqrt28ss	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrsqrt28ss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrsqrt28ss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrsqrt28ss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512EMI Disp8
	vrsqrt28ss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrsqrt28ss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512EMI Disp8
	vrsqrt28ss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512EMI

	vrsqrt28sd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrsqrt28sd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512EMI
	vrsqrt28sd	{sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrsqrt28sd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrsqrt28sd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrsqrt28sd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512EMI Disp8
	vrsqrt28sd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512EMI
	vrsqrt28sd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512EMI Disp8
	vrsqrt28sd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512EMI

	vsqrtsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsqrtsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vsqrtsd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsqrtsd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsqrtsd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsqrtsd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsqrtsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vsqrtsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vsqrtsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vsqrtsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vsqrtsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vsqrtsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vsqrtss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsqrtss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vsqrtss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsqrtss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsqrtss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsqrtss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsqrtss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vsqrtss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vsqrtss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vsqrtss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vsqrtss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vsqrtss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vsubsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsubsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vsubsd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsubsd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsubsd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsubsd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsubsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vsubsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vsubsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vsubsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vsubsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vsubsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vsubss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsubss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vsubss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsubss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsubss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsubss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vsubss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vsubss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vsubss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vsubss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vsubss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vsubss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vucomisd	{sae}, %xmm5, %xmm6	 # AVX512

	vucomiss	{sae}, %xmm5, %xmm6	 # AVX512

	vcvtsd2usi	%xmm6, %eax	 # AVX512
	vcvtsd2usi	{rn-sae}, %xmm6, %eax	 # AVX512
	vcvtsd2usi	{ru-sae}, %xmm6, %eax	 # AVX512
	vcvtsd2usi	{rd-sae}, %xmm6, %eax	 # AVX512
	vcvtsd2usi	{rz-sae}, %xmm6, %eax	 # AVX512
	vcvtsd2usi	(%ecx), %eax	 # AVX512
	vcvtsd2usi	-123456(%esp,%esi,8), %eax	 # AVX512
	vcvtsd2usi	1016(%edx), %eax	 # AVX512 Disp8
	vcvtsd2usi	1024(%edx), %eax	 # AVX512
	vcvtsd2usi	-1024(%edx), %eax	 # AVX512 Disp8
	vcvtsd2usi	-1032(%edx), %eax	 # AVX512
	vcvtsd2usi	%xmm6, %ebp	 # AVX512
	vcvtsd2usi	{rn-sae}, %xmm6, %ebp	 # AVX512
	vcvtsd2usi	{ru-sae}, %xmm6, %ebp	 # AVX512
	vcvtsd2usi	{rd-sae}, %xmm6, %ebp	 # AVX512
	vcvtsd2usi	{rz-sae}, %xmm6, %ebp	 # AVX512
	vcvtsd2usi	(%ecx), %ebp	 # AVX512
	vcvtsd2usi	-123456(%esp,%esi,8), %ebp	 # AVX512
	vcvtsd2usi	1016(%edx), %ebp	 # AVX512 Disp8
	vcvtsd2usi	1024(%edx), %ebp	 # AVX512
	vcvtsd2usi	-1024(%edx), %ebp	 # AVX512 Disp8
	vcvtsd2usi	-1032(%edx), %ebp	 # AVX512

	vcvtss2usi	%xmm6, %eax	 # AVX512
	vcvtss2usi	{rn-sae}, %xmm6, %eax	 # AVX512
	vcvtss2usi	{ru-sae}, %xmm6, %eax	 # AVX512
	vcvtss2usi	{rd-sae}, %xmm6, %eax	 # AVX512
	vcvtss2usi	{rz-sae}, %xmm6, %eax	 # AVX512
	vcvtss2usi	(%ecx), %eax	 # AVX512
	vcvtss2usi	-123456(%esp,%esi,8), %eax	 # AVX512
	vcvtss2usi	508(%edx), %eax	 # AVX512 Disp8
	vcvtss2usi	512(%edx), %eax	 # AVX512
	vcvtss2usi	-512(%edx), %eax	 # AVX512 Disp8
	vcvtss2usi	-516(%edx), %eax	 # AVX512
	vcvtss2usi	%xmm6, %ebp	 # AVX512
	vcvtss2usi	{rn-sae}, %xmm6, %ebp	 # AVX512
	vcvtss2usi	{ru-sae}, %xmm6, %ebp	 # AVX512
	vcvtss2usi	{rd-sae}, %xmm6, %ebp	 # AVX512
	vcvtss2usi	{rz-sae}, %xmm6, %ebp	 # AVX512
	vcvtss2usi	(%ecx), %ebp	 # AVX512
	vcvtss2usi	-123456(%esp,%esi,8), %ebp	 # AVX512
	vcvtss2usi	508(%edx), %ebp	 # AVX512 Disp8
	vcvtss2usi	512(%edx), %ebp	 # AVX512
	vcvtss2usi	-512(%edx), %ebp	 # AVX512 Disp8
	vcvtss2usi	-516(%edx), %ebp	 # AVX512

	vcvtusi2sdl	%eax, %xmm5, %xmm6	 # AVX512
	vcvtusi2sdl	%ebp, %xmm5, %xmm6	 # AVX512
	vcvtusi2sdl	(%ecx), %xmm5, %xmm6	 # AVX512
	vcvtusi2sdl	-123456(%esp,%esi,8), %xmm5, %xmm6	 # AVX512
	vcvtusi2sdl	508(%edx), %xmm5, %xmm6	 # AVX512 Disp8
	vcvtusi2sdl	512(%edx), %xmm5, %xmm6	 # AVX512
	vcvtusi2sdl	-512(%edx), %xmm5, %xmm6	 # AVX512 Disp8
	vcvtusi2sdl	-516(%edx), %xmm5, %xmm6	 # AVX512

	vcvtusi2ssl	%eax, %xmm5, %xmm6	 # AVX512
	vcvtusi2ssl	%eax, {rn-sae}, %xmm5, %xmm6	 # AVX512
	vcvtusi2ssl	%eax, {ru-sae}, %xmm5, %xmm6	 # AVX512
	vcvtusi2ssl	%eax, {rd-sae}, %xmm5, %xmm6	 # AVX512
	vcvtusi2ssl	%eax, {rz-sae}, %xmm5, %xmm6	 # AVX512
	vcvtusi2ssl	%ebp, %xmm5, %xmm6	 # AVX512
	vcvtusi2ssl	%ebp, {rn-sae}, %xmm5, %xmm6	 # AVX512
	vcvtusi2ssl	%ebp, {ru-sae}, %xmm5, %xmm6	 # AVX512
	vcvtusi2ssl	%ebp, {rd-sae}, %xmm5, %xmm6	 # AVX512
	vcvtusi2ssl	%ebp, {rz-sae}, %xmm5, %xmm6	 # AVX512
	vcvtusi2ssl	(%ecx), %xmm5, %xmm6	 # AVX512
	vcvtusi2ssl	-123456(%esp,%esi,8), %xmm5, %xmm6	 # AVX512
	vcvtusi2ssl	508(%edx), %xmm5, %xmm6	 # AVX512 Disp8
	vcvtusi2ssl	512(%edx), %xmm5, %xmm6	 # AVX512
	vcvtusi2ssl	-512(%edx), %xmm5, %xmm6	 # AVX512 Disp8
	vcvtusi2ssl	-516(%edx), %xmm5, %xmm6	 # AVX512

	vscalefsd	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vscalefsd	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vscalefsd	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vscalefsd	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vscalefsd	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vscalefsd	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vscalefsd	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vscalefsd	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vscalefsd	1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vscalefsd	1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vscalefsd	-1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vscalefsd	-1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vscalefss	%xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vscalefss	%xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vscalefss	{rn-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vscalefss	{ru-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vscalefss	{rd-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vscalefss	{rz-sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vscalefss	(%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vscalefss	-123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vscalefss	508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vscalefss	512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vscalefss	-512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vscalefss	-516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfixupimmss	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfixupimmss	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfixupimmss	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfixupimmss	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfixupimmss	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfixupimmss	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfixupimmss	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfixupimmss	$123, 508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfixupimmss	$123, 512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfixupimmss	$123, -512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfixupimmss	$123, -516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vfixupimmsd	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfixupimmsd	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vfixupimmsd	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfixupimmsd	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfixupimmsd	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vfixupimmsd	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vfixupimmsd	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vfixupimmsd	$123, 1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfixupimmsd	$123, 1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vfixupimmsd	$123, -1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vfixupimmsd	$123, -1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vrndscalesd	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vrndscalesd	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vrndscalesd	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vrndscalesd	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vrndscalesd	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vrndscalesd	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vrndscalesd	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vrndscalesd	$123, 1016(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vrndscalesd	$123, 1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vrndscalesd	$123, -1024(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vrndscalesd	$123, -1032(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vrndscaless	$0xab, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vrndscaless	$0xab, %xmm4, %xmm5, %xmm6{%k7}{z}	 # AVX512
	vrndscaless	$0xab, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vrndscaless	$123, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vrndscaless	$123, {sae}, %xmm4, %xmm5, %xmm6{%k7}	 # AVX512
	vrndscaless	$123, (%ecx), %xmm5, %xmm6{%k7}	 # AVX512
	vrndscaless	$123, -123456(%esp,%esi,8), %xmm5, %xmm6{%k7}	 # AVX512
	vrndscaless	$123, 508(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vrndscaless	$123, 512(%edx), %xmm5, %xmm6{%k7}	 # AVX512
	vrndscaless	$123, -512(%edx), %xmm5, %xmm6{%k7}	 # AVX512 Disp8
	vrndscaless	$123, -516(%edx), %xmm5, %xmm6{%k7}	 # AVX512

	vcmpsh	$123, %xmm4, %xmm5, %k5	# AVX512-FP16
	vcmpsh	$123, {sae}, %xmm4, %xmm5, %k5{%k7}	# AVX512-FP16
	vcmpsh	$123, (%ecx), %xmm5, %k5	# AVX512-FP16
	vcmpsh	$123, -123456(%esp, %esi, 8), %xmm5, %k5{%k7}	# AVX512-FP16
	vcmpsh	$123, 254(%ecx), %xmm5, %k5	# AVX512-FP16 Disp8
	vcmpsh	$123, -256(%edx), %xmm5, %k5{%k7}	# AVX512-FP16 Disp8

	vfpclasssh	$123, %xmm4, %k5	# AVX512-FP16
	vfpclasssh	$123, (%ecx), %k5	# AVX512-FP16
	vfpclasssh	$123, -123456(%esp, %esi, 8), %k5{%k7}	# AVX512-FP16
	vfpclasssh	$123, 254(%ecx), %k5	# AVX512-FP16 Disp8
	vfpclasssh	$123, -256(%edx), %k5{%k7}	# AVX512-FP16 Disp8

	.intel_syntax noprefix
	vaddsd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vaddsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vaddsd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vaddsd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vaddsd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vaddsd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vaddsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vaddsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vaddsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vaddsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vaddsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vaddsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vaddss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vaddss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vaddss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vaddss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vaddss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vaddss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vaddss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vaddss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vaddss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vaddss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vaddss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vaddss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpsd	k5{k7}, xmm5, xmm4, 0xab	 # AVX512
	vcmpsd	k5{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512
	vcmpsd	k5{k7}, xmm5, xmm4, 123	 # AVX512
	vcmpsd	k5{k7}, xmm5, xmm4, {sae}, 123	 # AVX512
	vcmpsd	k5{k7}, xmm5, QWORD PTR [ecx], 123	 # AVX512
	vcmpsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456], 123	 # AVX512
	vcmpsd	k5{k7}, xmm5, QWORD PTR [edx+1016], 123	 # AVX512 Disp8
	vcmpsd	k5{k7}, xmm5, QWORD PTR [edx+1024], 123	 # AVX512
	vcmpsd	k5{k7}, xmm5, QWORD PTR [edx-1024], 123	 # AVX512 Disp8
	vcmpsd	k5{k7}, xmm5, QWORD PTR [edx-1032], 123	 # AVX512

	vcmpeq_oqsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpeq_oqsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpeq_oqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpeq_oqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpeq_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpeq_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpeq_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpeq_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpeqsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpeqsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpeqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpeqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpeqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpeqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpeqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpeqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmplt_ossd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmplt_ossd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmplt_ossd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmplt_ossd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmplt_ossd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmplt_ossd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmplt_ossd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmplt_ossd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpltsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpltsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpltsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpltsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpltsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpltsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpltsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpltsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmple_ossd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmple_ossd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmple_ossd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmple_ossd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmple_ossd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmple_ossd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmple_ossd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmple_ossd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmplesd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmplesd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmplesd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmplesd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmplesd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmplesd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmplesd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmplesd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpunord_qsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpunord_qsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpunord_qsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpunord_qsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpunord_qsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpunord_qsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpunord_qsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpunord_qsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpunordsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpunordsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpunordsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpunordsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpunordsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpunordsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpunordsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpunordsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpneq_uqsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpneq_uqsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpneq_uqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpneq_uqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpneq_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpneq_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpneq_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpneq_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpneqsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpneqsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpneqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpneqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpneqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpneqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpneqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpneqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpnlt_ussd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpnlt_ussd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpnlt_ussd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpnlt_ussd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpnlt_ussd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpnlt_ussd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpnlt_ussd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpnlt_ussd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpnltsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpnltsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpnltsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpnltsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpnltsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpnltsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpnltsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpnltsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpnle_ussd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpnle_ussd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpnle_ussd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpnle_ussd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpnle_ussd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpnle_ussd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpnle_ussd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpnle_ussd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpnlesd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpnlesd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpnlesd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpnlesd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpnlesd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpnlesd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpnlesd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpnlesd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpord_qsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpord_qsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpord_qsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpord_qsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpord_qsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpord_qsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpord_qsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpord_qsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpordsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpordsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpordsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpordsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpordsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpordsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpordsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpordsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpeq_uqsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpeq_uqsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpeq_uqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpeq_uqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpeq_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpeq_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpeq_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpeq_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpnge_ussd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpnge_ussd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpnge_ussd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpnge_ussd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpnge_ussd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpnge_ussd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpnge_ussd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpnge_ussd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpngesd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpngesd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpngesd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpngesd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpngesd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpngesd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpngesd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpngesd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpngt_ussd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpngt_ussd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpngt_ussd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpngt_ussd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpngt_ussd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpngt_ussd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpngt_ussd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpngt_ussd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpngtsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpngtsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpngtsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpngtsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpngtsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpngtsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpngtsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpngtsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpfalse_oqsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpfalse_oqsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpfalse_oqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpfalse_oqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpfalse_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpfalse_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpfalse_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpfalse_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpfalsesd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpfalsesd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpfalsesd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpfalsesd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpfalsesd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpfalsesd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpfalsesd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpfalsesd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpneq_oqsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpneq_oqsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpneq_oqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpneq_oqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpneq_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpneq_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpneq_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpneq_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpge_ossd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpge_ossd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpge_ossd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpge_ossd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpge_ossd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpge_ossd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpge_ossd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpge_ossd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpgesd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpgesd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpgesd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpgesd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpgesd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpgesd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpgesd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpgesd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpgt_ossd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpgt_ossd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpgt_ossd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpgt_ossd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpgt_ossd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpgt_ossd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpgt_ossd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpgt_ossd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpgtsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpgtsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpgtsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpgtsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpgtsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpgtsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpgtsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpgtsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmptrue_uqsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmptrue_uqsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmptrue_uqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmptrue_uqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmptrue_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmptrue_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmptrue_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmptrue_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmptruesd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmptruesd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmptruesd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmptruesd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmptruesd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmptruesd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmptruesd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmptruesd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpeq_ossd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpeq_ossd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpeq_ossd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpeq_ossd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpeq_ossd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpeq_ossd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpeq_ossd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpeq_ossd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmplt_oqsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmplt_oqsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmplt_oqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmplt_oqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmplt_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmplt_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmplt_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmplt_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmple_oqsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmple_oqsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmple_oqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmple_oqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmple_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmple_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmple_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmple_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpunord_ssd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpunord_ssd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpunord_ssd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpunord_ssd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpunord_ssd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpunord_ssd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpunord_ssd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpunord_ssd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpneq_ussd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpneq_ussd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpneq_ussd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpneq_ussd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpneq_ussd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpneq_ussd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpneq_ussd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpneq_ussd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpnlt_uqsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpnlt_uqsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpnlt_uqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpnlt_uqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpnlt_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpnlt_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpnlt_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpnlt_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpnle_uqsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpnle_uqsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpnle_uqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpnle_uqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpnle_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpnle_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpnle_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpnle_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpord_ssd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpord_ssd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpord_ssd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpord_ssd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpord_ssd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpord_ssd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpord_ssd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpord_ssd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpeq_ussd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpeq_ussd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpeq_ussd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpeq_ussd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpeq_ussd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpeq_ussd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpeq_ussd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpeq_ussd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpnge_uqsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpnge_uqsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpnge_uqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpnge_uqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpnge_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpnge_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpnge_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpnge_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpngt_uqsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpngt_uqsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpngt_uqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpngt_uqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpngt_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpngt_uqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpngt_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpngt_uqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpfalse_ossd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpfalse_ossd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpfalse_ossd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpfalse_ossd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpfalse_ossd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpfalse_ossd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpfalse_ossd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpfalse_ossd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpneq_ossd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpneq_ossd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpneq_ossd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpneq_ossd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpneq_ossd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpneq_ossd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpneq_ossd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpneq_ossd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpge_oqsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpge_oqsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpge_oqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpge_oqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpge_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpge_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpge_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpge_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpgt_oqsd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpgt_oqsd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpgt_oqsd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmpgt_oqsd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpgt_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmpgt_oqsd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmpgt_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmpgt_oqsd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmptrue_ussd	k5{k7}, xmm5, xmm4	 # AVX512
	vcmptrue_ussd	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmptrue_ussd	k5{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcmptrue_ussd	k5{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmptrue_ussd	k5{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcmptrue_ussd	k5{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcmptrue_ussd	k5{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcmptrue_ussd	k5{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vcmpss	k5{k7}, xmm5, xmm4, 0xab	 # AVX512
	vcmpss	k5{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512
	vcmpss	k5{k7}, xmm5, xmm4, 123	 # AVX512
	vcmpss	k5{k7}, xmm5, xmm4, {sae}, 123	 # AVX512
	vcmpss	k5{k7}, xmm5, DWORD PTR [ecx], 123	 # AVX512
	vcmpss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456], 123	 # AVX512
	vcmpss	k5{k7}, xmm5, DWORD PTR [edx+508], 123	 # AVX512 Disp8
	vcmpss	k5{k7}, xmm5, DWORD PTR [edx+512], 123	 # AVX512
	vcmpss	k5{k7}, xmm5, DWORD PTR [edx-512], 123	 # AVX512 Disp8
	vcmpss	k5{k7}, xmm5, DWORD PTR [edx-516], 123	 # AVX512

	vcmpeq_oqss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpeq_oqss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpeq_oqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpeq_oqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpeq_oqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpeq_oqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpeq_oqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpeq_oqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpeqss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpeqss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpeqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpeqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpeqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpeqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpeqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpeqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmplt_osss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmplt_osss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmplt_osss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmplt_osss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmplt_osss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmplt_osss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmplt_osss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmplt_osss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpltss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpltss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpltss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpltss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpltss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpltss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpltss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpltss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmple_osss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmple_osss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmple_osss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmple_osss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmple_osss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmple_osss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmple_osss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmple_osss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpless	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpless	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpless	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpless	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpless	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpless	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpless	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpless	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpunord_qss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpunord_qss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpunord_qss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpunord_qss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpunord_qss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpunord_qss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpunord_qss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpunord_qss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpunordss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpunordss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpunordss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpunordss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpunordss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpunordss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpunordss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpunordss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpneq_uqss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpneq_uqss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpneq_uqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpneq_uqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpneq_uqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpneq_uqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpneq_uqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpneq_uqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpneqss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpneqss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpneqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpneqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpneqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpneqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpneqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpneqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpnlt_usss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpnlt_usss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpnlt_usss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpnlt_usss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpnlt_usss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpnlt_usss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpnlt_usss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpnlt_usss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpnltss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpnltss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpnltss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpnltss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpnltss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpnltss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpnltss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpnltss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpnle_usss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpnle_usss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpnle_usss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpnle_usss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpnle_usss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpnle_usss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpnle_usss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpnle_usss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpnless	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpnless	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpnless	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpnless	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpnless	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpnless	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpnless	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpnless	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpord_qss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpord_qss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpord_qss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpord_qss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpord_qss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpord_qss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpord_qss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpord_qss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpordss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpordss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpordss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpordss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpordss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpordss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpordss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpordss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpeq_uqss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpeq_uqss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpeq_uqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpeq_uqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpeq_uqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpeq_uqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpeq_uqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpeq_uqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpnge_usss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpnge_usss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpnge_usss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpnge_usss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpnge_usss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpnge_usss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpnge_usss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpnge_usss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpngess	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpngess	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpngess	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpngess	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpngess	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpngess	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpngess	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpngess	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpngt_usss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpngt_usss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpngt_usss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpngt_usss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpngt_usss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpngt_usss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpngt_usss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpngt_usss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpngtss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpngtss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpngtss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpngtss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpngtss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpngtss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpngtss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpngtss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpfalse_oqss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpfalse_oqss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpfalse_oqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpfalse_oqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpfalse_oqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpfalse_oqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpfalse_oqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpfalse_oqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpfalsess	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpfalsess	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpfalsess	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpfalsess	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpfalsess	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpfalsess	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpfalsess	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpfalsess	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpneq_oqss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpneq_oqss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpneq_oqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpneq_oqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpneq_oqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpneq_oqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpneq_oqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpneq_oqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpge_osss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpge_osss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpge_osss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpge_osss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpge_osss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpge_osss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpge_osss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpge_osss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpgess	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpgess	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpgess	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpgess	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpgess	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpgess	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpgess	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpgess	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpgt_osss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpgt_osss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpgt_osss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpgt_osss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpgt_osss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpgt_osss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpgt_osss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpgt_osss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpgtss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpgtss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpgtss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpgtss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpgtss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpgtss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpgtss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpgtss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmptrue_uqss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmptrue_uqss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmptrue_uqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmptrue_uqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmptrue_uqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmptrue_uqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmptrue_uqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmptrue_uqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmptruess	k5{k7}, xmm5, xmm4	 # AVX512
	vcmptruess	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmptruess	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmptruess	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmptruess	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmptruess	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmptruess	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmptruess	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpeq_osss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpeq_osss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpeq_osss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpeq_osss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpeq_osss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpeq_osss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpeq_osss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpeq_osss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmplt_oqss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmplt_oqss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmplt_oqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmplt_oqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmplt_oqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmplt_oqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmplt_oqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmplt_oqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmple_oqss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmple_oqss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmple_oqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmple_oqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmple_oqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmple_oqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmple_oqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmple_oqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpunord_sss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpunord_sss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpunord_sss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpunord_sss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpunord_sss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpunord_sss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpunord_sss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpunord_sss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpneq_usss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpneq_usss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpneq_usss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpneq_usss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpneq_usss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpneq_usss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpneq_usss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpneq_usss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpnlt_uqss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpnlt_uqss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpnlt_uqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpnlt_uqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpnlt_uqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpnlt_uqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpnlt_uqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpnlt_uqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpnle_uqss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpnle_uqss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpnle_uqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpnle_uqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpnle_uqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpnle_uqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpnle_uqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpnle_uqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpord_sss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpord_sss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpord_sss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpord_sss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpord_sss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpord_sss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpord_sss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpord_sss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpeq_usss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpeq_usss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpeq_usss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpeq_usss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpeq_usss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpeq_usss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpeq_usss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpeq_usss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpnge_uqss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpnge_uqss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpnge_uqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpnge_uqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpnge_uqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpnge_uqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpnge_uqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpnge_uqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpngt_uqss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpngt_uqss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpngt_uqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpngt_uqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpngt_uqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpngt_uqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpngt_uqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpngt_uqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpfalse_osss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpfalse_osss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpfalse_osss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpfalse_osss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpfalse_osss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpfalse_osss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpfalse_osss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpfalse_osss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpneq_osss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpneq_osss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpneq_osss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpneq_osss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpneq_osss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpneq_osss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpneq_osss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpneq_osss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpge_oqss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpge_oqss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpge_oqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpge_oqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpge_oqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpge_oqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpge_oqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpge_oqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmpgt_oqss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmpgt_oqss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmpgt_oqss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmpgt_oqss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmpgt_oqss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmpgt_oqss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmpgt_oqss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmpgt_oqss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcmptrue_usss	k5{k7}, xmm5, xmm4	 # AVX512
	vcmptrue_usss	k5{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcmptrue_usss	k5{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcmptrue_usss	k5{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcmptrue_usss	k5{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcmptrue_usss	k5{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcmptrue_usss	k5{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcmptrue_usss	k5{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcomisd	xmm6, xmm5, {sae}	 # AVX512

	vcomiss	xmm6, xmm5, {sae}	 # AVX512

	vcvtsd2si	eax, xmm6, {rn-sae}	 # AVX512
	vcvtsd2si	eax, xmm6, {ru-sae}	 # AVX512
	vcvtsd2si	eax, xmm6, {rd-sae}	 # AVX512
	vcvtsd2si	eax, xmm6, {rz-sae}	 # AVX512
	vcvtsd2si	ebp, xmm6, {rn-sae}	 # AVX512
	vcvtsd2si	ebp, xmm6, {ru-sae}	 # AVX512
	vcvtsd2si	ebp, xmm6, {rd-sae}	 # AVX512
	vcvtsd2si	ebp, xmm6, {rz-sae}	 # AVX512

	vcvtsd2ss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vcvtsd2ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vcvtsd2ss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vcvtsd2ss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vcvtsd2ss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vcvtsd2ss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vcvtsd2ss	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vcvtsd2ss	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcvtsd2ss	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcvtsd2ss	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vcvtsd2ss	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcvtsd2ss	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512


	vcvtsi2ss	xmm6, xmm5, {rn-sae}, eax	 # AVX512
	vcvtsi2ss	xmm6, xmm5, {ru-sae}, eax	 # AVX512
	vcvtsi2ss	xmm6, xmm5, {rd-sae}, eax	 # AVX512
	vcvtsi2ss	xmm6, xmm5, {rz-sae}, eax	 # AVX512
	vcvtsi2ss	xmm6, xmm5, {rn-sae}, ebp	 # AVX512
	vcvtsi2ss	xmm6, xmm5, {ru-sae}, ebp	 # AVX512
	vcvtsi2ss	xmm6, xmm5, {rd-sae}, ebp	 # AVX512
	vcvtsi2ss	xmm6, xmm5, {rz-sae}, ebp	 # AVX512

	vcvtss2sd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vcvtss2sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vcvtss2sd	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512
	vcvtss2sd	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vcvtss2sd	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcvtss2sd	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcvtss2sd	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcvtss2sd	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcvtss2sd	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcvtss2si	eax, xmm6, {rn-sae}	 # AVX512
	vcvtss2si	eax, xmm6, {ru-sae}	 # AVX512
	vcvtss2si	eax, xmm6, {rd-sae}	 # AVX512
	vcvtss2si	eax, xmm6, {rz-sae}	 # AVX512
	vcvtss2si	ebp, xmm6, {rn-sae}	 # AVX512
	vcvtss2si	ebp, xmm6, {ru-sae}	 # AVX512
	vcvtss2si	ebp, xmm6, {rd-sae}	 # AVX512
	vcvtss2si	ebp, xmm6, {rz-sae}	 # AVX512

	vcvttsd2si	eax, xmm6, {sae}	 # AVX512
	vcvttsd2si	ebp, xmm6, {sae}	 # AVX512

	vcvttss2si	eax, xmm6, {sae}	 # AVX512
	vcvttss2si	ebp, xmm6, {sae}	 # AVX512

	vdivsd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vdivsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vdivsd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vdivsd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vdivsd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vdivsd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vdivsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vdivsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vdivsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vdivsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vdivsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vdivsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vdivss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vdivss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vdivss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vdivss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vdivss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vdivss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vdivss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vdivss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vdivss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vdivss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vdivss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vdivss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vfmadd132sd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfmadd132sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfmadd132sd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfmadd132sd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfmadd132sd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfmadd132sd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfmadd132sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vfmadd132sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vfmadd132sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vfmadd132sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vfmadd132sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vfmadd132sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vfmadd132ss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfmadd132ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfmadd132ss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfmadd132ss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfmadd132ss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfmadd132ss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfmadd132ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vfmadd132ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vfmadd132ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vfmadd132ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vfmadd132ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vfmadd132ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vfmadd213sd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfmadd213sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfmadd213sd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfmadd213sd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfmadd213sd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfmadd213sd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfmadd213sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vfmadd213sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vfmadd213sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vfmadd213sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vfmadd213sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vfmadd213sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vfmadd213ss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfmadd213ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfmadd213ss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfmadd213ss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfmadd213ss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfmadd213ss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfmadd213ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vfmadd213ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vfmadd213ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vfmadd213ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vfmadd213ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vfmadd213ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vfmadd231sd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfmadd231sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfmadd231sd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfmadd231sd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfmadd231sd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfmadd231sd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfmadd231sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vfmadd231sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vfmadd231sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vfmadd231sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vfmadd231sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vfmadd231sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vfmadd231ss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfmadd231ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfmadd231ss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfmadd231ss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfmadd231ss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfmadd231ss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfmadd231ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vfmadd231ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vfmadd231ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vfmadd231ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vfmadd231ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vfmadd231ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vfmsub132sd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfmsub132sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfmsub132sd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfmsub132sd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfmsub132sd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfmsub132sd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfmsub132sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vfmsub132sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vfmsub132sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vfmsub132sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vfmsub132sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vfmsub132sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vfmsub132ss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfmsub132ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfmsub132ss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfmsub132ss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfmsub132ss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfmsub132ss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfmsub132ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vfmsub132ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vfmsub132ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vfmsub132ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vfmsub132ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vfmsub132ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vfmsub213sd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfmsub213sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfmsub213sd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfmsub213sd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfmsub213sd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfmsub213sd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfmsub213sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vfmsub213sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vfmsub213sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vfmsub213sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vfmsub213sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vfmsub213sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vfmsub213ss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfmsub213ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfmsub213ss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfmsub213ss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfmsub213ss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfmsub213ss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfmsub213ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vfmsub213ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vfmsub213ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vfmsub213ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vfmsub213ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vfmsub213ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vfmsub231sd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfmsub231sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfmsub231sd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfmsub231sd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfmsub231sd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfmsub231sd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfmsub231sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vfmsub231sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vfmsub231sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vfmsub231sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vfmsub231sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vfmsub231sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vfmsub231ss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfmsub231ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfmsub231ss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfmsub231ss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfmsub231ss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfmsub231ss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfmsub231ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vfmsub231ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vfmsub231ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vfmsub231ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vfmsub231ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vfmsub231ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vfnmadd132sd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfnmadd132sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfnmadd132sd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfnmadd132sd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfnmadd132sd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfnmadd132sd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfnmadd132sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vfnmadd132sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vfnmadd132sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vfnmadd132sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vfnmadd132sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vfnmadd132sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vfnmadd132ss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfnmadd132ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfnmadd132ss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfnmadd132ss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfnmadd132ss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfnmadd132ss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfnmadd132ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vfnmadd132ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vfnmadd132ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vfnmadd132ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vfnmadd132ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vfnmadd132ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vfnmadd213sd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfnmadd213sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfnmadd213sd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfnmadd213sd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfnmadd213sd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfnmadd213sd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfnmadd213sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vfnmadd213sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vfnmadd213sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vfnmadd213sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vfnmadd213sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vfnmadd213sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vfnmadd213ss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfnmadd213ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfnmadd213ss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfnmadd213ss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfnmadd213ss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfnmadd213ss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfnmadd213ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vfnmadd213ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vfnmadd213ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vfnmadd213ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vfnmadd213ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vfnmadd213ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vfnmadd231sd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfnmadd231sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfnmadd231sd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfnmadd231sd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfnmadd231sd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfnmadd231sd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfnmadd231sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vfnmadd231sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vfnmadd231sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vfnmadd231sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vfnmadd231sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vfnmadd231sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vfnmadd231ss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfnmadd231ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfnmadd231ss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfnmadd231ss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfnmadd231ss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfnmadd231ss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfnmadd231ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vfnmadd231ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vfnmadd231ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vfnmadd231ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vfnmadd231ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vfnmadd231ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vfnmsub132sd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfnmsub132sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfnmsub132sd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfnmsub132sd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfnmsub132sd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfnmsub132sd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfnmsub132sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vfnmsub132sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vfnmsub132sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vfnmsub132sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vfnmsub132sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vfnmsub132sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vfnmsub132ss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfnmsub132ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfnmsub132ss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfnmsub132ss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfnmsub132ss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfnmsub132ss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfnmsub132ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vfnmsub132ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vfnmsub132ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vfnmsub132ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vfnmsub132ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vfnmsub132ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vfnmsub213sd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfnmsub213sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfnmsub213sd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfnmsub213sd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfnmsub213sd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfnmsub213sd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfnmsub213sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vfnmsub213sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vfnmsub213sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vfnmsub213sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vfnmsub213sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vfnmsub213sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vfnmsub213ss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfnmsub213ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfnmsub213ss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfnmsub213ss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfnmsub213ss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfnmsub213ss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfnmsub213ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vfnmsub213ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vfnmsub213ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vfnmsub213ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vfnmsub213ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vfnmsub213ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vfnmsub231sd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfnmsub231sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfnmsub231sd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfnmsub231sd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfnmsub231sd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfnmsub231sd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfnmsub231sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vfnmsub231sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vfnmsub231sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vfnmsub231sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vfnmsub231sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vfnmsub231sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vfnmsub231ss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vfnmsub231ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vfnmsub231ss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vfnmsub231ss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vfnmsub231ss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vfnmsub231ss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vfnmsub231ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vfnmsub231ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vfnmsub231ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vfnmsub231ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vfnmsub231ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vfnmsub231ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vgetexpsd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vgetexpsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vgetexpsd	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512
	vgetexpsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vgetexpsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vgetexpsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vgetexpsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vgetexpsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vgetexpsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vgetexpss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vgetexpss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vgetexpss	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512
	vgetexpss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vgetexpss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vgetexpss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vgetexpss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vgetexpss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vgetexpss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vgetmantsd	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512
	vgetmantsd	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512
	vgetmantsd	xmm6{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512
	vgetmantsd	xmm6{k7}, xmm5, xmm4, 123	 # AVX512
	vgetmantsd	xmm6{k7}, xmm5, xmm4, {sae}, 123	 # AVX512
	vgetmantsd	xmm6{k7}, xmm5, QWORD PTR [ecx], 123	 # AVX512
	vgetmantsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456], 123	 # AVX512
	vgetmantsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016], 123	 # AVX512 Disp8
	vgetmantsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024], 123	 # AVX512
	vgetmantsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024], 123	 # AVX512 Disp8
	vgetmantsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032], 123	 # AVX512

	vgetmantss	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512
	vgetmantss	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512
	vgetmantss	xmm6{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512
	vgetmantss	xmm6{k7}, xmm5, xmm4, 123	 # AVX512
	vgetmantss	xmm6{k7}, xmm5, xmm4, {sae}, 123	 # AVX512
	vgetmantss	xmm6{k7}, xmm5, DWORD PTR [ecx], 123	 # AVX512
	vgetmantss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456], 123	 # AVX512
	vgetmantss	xmm6{k7}, xmm5, DWORD PTR [edx+508], 123	 # AVX512 Disp8
	vgetmantss	xmm6{k7}, xmm5, DWORD PTR [edx+512], 123	 # AVX512
	vgetmantss	xmm6{k7}, xmm5, DWORD PTR [edx-512], 123	 # AVX512 Disp8
	vgetmantss	xmm6{k7}, xmm5, DWORD PTR [edx-516], 123	 # AVX512

	vmaxsd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vmaxsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vmaxsd	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512
	vmaxsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vmaxsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vmaxsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vmaxsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vmaxsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vmaxsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vmaxss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vmaxss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vmaxss	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512
	vmaxss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vmaxss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vmaxss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vmaxss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vmaxss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vmaxss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vminsd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vminsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vminsd	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512
	vminsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vminsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vminsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vminsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vminsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vminsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vminss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vminss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vminss	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512
	vminss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vminss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vminss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vminss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vminss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vminss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vmovsd	xmm6{k7}, QWORD PTR [ecx]	 # AVX512
	vmovsd	xmm6{k7}{z}, QWORD PTR [ecx]	 # AVX512
	vmovsd	xmm6{k7}, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vmovsd	xmm6{k7}, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vmovsd	xmm6{k7}, QWORD PTR [edx+1024]	 # AVX512
	vmovsd	xmm6{k7}, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vmovsd	xmm6{k7}, QWORD PTR [edx-1032]	 # AVX512

	vmovsd	QWORD PTR [ecx]{k7}, xmm6	 # AVX512
	vmovsd	QWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512
	vmovsd	QWORD PTR [edx+1016]{k7}, xmm6	 # AVX512 Disp8
	vmovsd	QWORD PTR [edx+1024]{k7}, xmm6	 # AVX512
	vmovsd	QWORD PTR [edx-1024]{k7}, xmm6	 # AVX512 Disp8
	vmovsd	QWORD PTR [edx-1032]{k7}, xmm6	 # AVX512

	vmovsd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vmovsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512

	vmovss	xmm6{k7}, DWORD PTR [ecx]	 # AVX512
	vmovss	xmm6{k7}{z}, DWORD PTR [ecx]	 # AVX512
	vmovss	xmm6{k7}, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vmovss	xmm6{k7}, DWORD PTR [edx+508]	 # AVX512 Disp8
	vmovss	xmm6{k7}, DWORD PTR [edx+512]	 # AVX512
	vmovss	xmm6{k7}, DWORD PTR [edx-512]	 # AVX512 Disp8
	vmovss	xmm6{k7}, DWORD PTR [edx-516]	 # AVX512

	vmovss	DWORD PTR [ecx]{k7}, xmm6	 # AVX512
	vmovss	DWORD PTR [esp+esi*8-123456]{k7}, xmm6	 # AVX512
	vmovss	DWORD PTR [edx+508]{k7}, xmm6	 # AVX512 Disp8
	vmovss	DWORD PTR [edx+512]{k7}, xmm6	 # AVX512
	vmovss	DWORD PTR [edx-512]{k7}, xmm6	 # AVX512 Disp8
	vmovss	DWORD PTR [edx-516]{k7}, xmm6	 # AVX512

	vmovss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vmovss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512

	vmulsd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vmulsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vmulsd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vmulsd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vmulsd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vmulsd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vmulsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vmulsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vmulsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vmulsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vmulsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vmulsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vmulss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vmulss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vmulss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vmulss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vmulss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vmulss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vmulss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vmulss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vmulss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vmulss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vmulss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vmulss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vrcp14sd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vrcp14sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vrcp14sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vrcp14sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vrcp14sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vrcp14sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vrcp14sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vrcp14sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vrcp14ss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vrcp14ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vrcp14ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vrcp14ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vrcp14ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vrcp14ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vrcp14ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vrcp14ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vrcp28ss	xmm6{k7}, xmm5, xmm4	 # AVX512EMI
	vrcp28ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512EMI
	vrcp28ss	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512EMI
	vrcp28ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512EMI
	vrcp28ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512EMI
	vrcp28ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512EMI Disp8
	vrcp28ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512EMI
	vrcp28ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512EMI Disp8
	vrcp28ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512EMI

	vrcp28sd	xmm6{k7}, xmm5, xmm4	 # AVX512EMI
	vrcp28sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512EMI
	vrcp28sd	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512EMI
	vrcp28sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512EMI
	vrcp28sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512EMI
	vrcp28sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512EMI Disp8
	vrcp28sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512EMI
	vrcp28sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512EMI Disp8
	vrcp28sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512EMI

	vrsqrt14sd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vrsqrt14sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vrsqrt14sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vrsqrt14sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vrsqrt14sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vrsqrt14sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vrsqrt14sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vrsqrt14sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vrsqrt14ss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vrsqrt14ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vrsqrt14ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vrsqrt14ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vrsqrt14ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vrsqrt14ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vrsqrt14ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vrsqrt14ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vrsqrt28ss	xmm6{k7}, xmm5, xmm4	 # AVX512EMI
	vrsqrt28ss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512EMI
	vrsqrt28ss	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512EMI
	vrsqrt28ss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512EMI
	vrsqrt28ss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512EMI
	vrsqrt28ss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512EMI Disp8
	vrsqrt28ss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512EMI
	vrsqrt28ss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512EMI Disp8
	vrsqrt28ss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512EMI

	vrsqrt28sd	xmm6{k7}, xmm5, xmm4	 # AVX512EMI
	vrsqrt28sd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512EMI
	vrsqrt28sd	xmm6{k7}, xmm5, xmm4, {sae}	 # AVX512EMI
	vrsqrt28sd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512EMI
	vrsqrt28sd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512EMI
	vrsqrt28sd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512EMI Disp8
	vrsqrt28sd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512EMI
	vrsqrt28sd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512EMI Disp8
	vrsqrt28sd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512EMI

	vsqrtsd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vsqrtsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vsqrtsd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vsqrtsd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vsqrtsd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vsqrtsd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vsqrtsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vsqrtsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vsqrtsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vsqrtsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vsqrtsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vsqrtsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vsqrtss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vsqrtss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vsqrtss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vsqrtss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vsqrtss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vsqrtss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vsqrtss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vsqrtss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vsqrtss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vsqrtss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vsqrtss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vsqrtss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vsubsd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vsubsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vsubsd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vsubsd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vsubsd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vsubsd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vsubsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vsubsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vsubsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vsubsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vsubsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vsubsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vsubss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vsubss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vsubss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vsubss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vsubss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vsubss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vsubss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vsubss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vsubss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vsubss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vsubss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vsubss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vucomisd	xmm6, xmm5, {sae}	 # AVX512

	vucomiss	xmm6, xmm5, {sae}	 # AVX512

	vcvtsd2usi	eax, xmm6	 # AVX512
	vcvtsd2usi	eax, xmm6, {rn-sae}	 # AVX512
	vcvtsd2usi	eax, xmm6, {ru-sae}	 # AVX512
	vcvtsd2usi	eax, xmm6, {rd-sae}	 # AVX512
	vcvtsd2usi	eax, xmm6, {rz-sae}	 # AVX512
	vcvtsd2usi	eax, QWORD PTR [ecx]	 # AVX512
	vcvtsd2usi	eax, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcvtsd2usi	eax, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcvtsd2usi	eax, QWORD PTR [edx+1024]	 # AVX512
	vcvtsd2usi	eax, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcvtsd2usi	eax, QWORD PTR [edx-1032]	 # AVX512
	vcvtsd2usi	ebp, xmm6	 # AVX512
	vcvtsd2usi	ebp, xmm6, {rn-sae}	 # AVX512
	vcvtsd2usi	ebp, xmm6, {ru-sae}	 # AVX512
	vcvtsd2usi	ebp, xmm6, {rd-sae}	 # AVX512
	vcvtsd2usi	ebp, xmm6, {rz-sae}	 # AVX512
	vcvtsd2usi	ebp, QWORD PTR [ecx]	 # AVX512
	vcvtsd2usi	ebp, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vcvtsd2usi	ebp, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vcvtsd2usi	ebp, QWORD PTR [edx+1024]	 # AVX512
	vcvtsd2usi	ebp, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vcvtsd2usi	ebp, QWORD PTR [edx-1032]	 # AVX512

	vcvtss2usi	eax, xmm6	 # AVX512
	vcvtss2usi	eax, xmm6, {rn-sae}	 # AVX512
	vcvtss2usi	eax, xmm6, {ru-sae}	 # AVX512
	vcvtss2usi	eax, xmm6, {rd-sae}	 # AVX512
	vcvtss2usi	eax, xmm6, {rz-sae}	 # AVX512
	vcvtss2usi	eax, DWORD PTR [ecx]	 # AVX512
	vcvtss2usi	eax, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcvtss2usi	eax, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcvtss2usi	eax, DWORD PTR [edx+512]	 # AVX512
	vcvtss2usi	eax, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcvtss2usi	eax, DWORD PTR [edx-516]	 # AVX512
	vcvtss2usi	ebp, xmm6	 # AVX512
	vcvtss2usi	ebp, xmm6, {rn-sae}	 # AVX512
	vcvtss2usi	ebp, xmm6, {ru-sae}	 # AVX512
	vcvtss2usi	ebp, xmm6, {rd-sae}	 # AVX512
	vcvtss2usi	ebp, xmm6, {rz-sae}	 # AVX512
	vcvtss2usi	ebp, DWORD PTR [ecx]	 # AVX512
	vcvtss2usi	ebp, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcvtss2usi	ebp, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcvtss2usi	ebp, DWORD PTR [edx+512]	 # AVX512
	vcvtss2usi	ebp, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcvtss2usi	ebp, DWORD PTR [edx-516]	 # AVX512

	vcvtusi2sd	xmm6, xmm5, eax	 # AVX512
	vcvtusi2sd	xmm6, xmm5, ebp	 # AVX512
	vcvtusi2sd	xmm6, xmm5, DWORD PTR [ecx]	 # AVX512
	vcvtusi2sd	xmm6, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcvtusi2sd	xmm6, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcvtusi2sd	xmm6, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcvtusi2sd	xmm6, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcvtusi2sd	xmm6, xmm5, DWORD PTR [edx-516]	 # AVX512

	vcvtusi2ss	xmm6, xmm5, eax	 # AVX512
	vcvtusi2ss	xmm6, xmm5, {rn-sae}, eax	 # AVX512
	vcvtusi2ss	xmm6, xmm5, {ru-sae}, eax	 # AVX512
	vcvtusi2ss	xmm6, xmm5, {rd-sae}, eax	 # AVX512
	vcvtusi2ss	xmm6, xmm5, {rz-sae}, eax	 # AVX512
	vcvtusi2ss	xmm6, xmm5, ebp	 # AVX512
	vcvtusi2ss	xmm6, xmm5, {rn-sae}, ebp	 # AVX512
	vcvtusi2ss	xmm6, xmm5, {ru-sae}, ebp	 # AVX512
	vcvtusi2ss	xmm6, xmm5, {rd-sae}, ebp	 # AVX512
	vcvtusi2ss	xmm6, xmm5, {rz-sae}, ebp	 # AVX512
	vcvtusi2ss	xmm6, xmm5, DWORD PTR [ecx]	 # AVX512
	vcvtusi2ss	xmm6, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vcvtusi2ss	xmm6, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vcvtusi2ss	xmm6, xmm5, DWORD PTR [edx+512]	 # AVX512
	vcvtusi2ss	xmm6, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vcvtusi2ss	xmm6, xmm5, DWORD PTR [edx-516]	 # AVX512

	vscalefsd	xmm6{k7}, xmm5, xmm4	 # AVX512
	vscalefsd	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vscalefsd	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vscalefsd	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vscalefsd	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vscalefsd	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vscalefsd	xmm6{k7}, xmm5, QWORD PTR [ecx]	 # AVX512
	vscalefsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456]	 # AVX512
	vscalefsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016]	 # AVX512 Disp8
	vscalefsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024]	 # AVX512
	vscalefsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024]	 # AVX512 Disp8
	vscalefsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032]	 # AVX512

	vscalefss	xmm6{k7}, xmm5, xmm4	 # AVX512
	vscalefss	xmm6{k7}{z}, xmm5, xmm4	 # AVX512
	vscalefss	xmm6{k7}, xmm5, xmm4, {rn-sae}	 # AVX512
	vscalefss	xmm6{k7}, xmm5, xmm4, {ru-sae}	 # AVX512
	vscalefss	xmm6{k7}, xmm5, xmm4, {rd-sae}	 # AVX512
	vscalefss	xmm6{k7}, xmm5, xmm4, {rz-sae}	 # AVX512
	vscalefss	xmm6{k7}, xmm5, DWORD PTR [ecx]	 # AVX512
	vscalefss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456]	 # AVX512
	vscalefss	xmm6{k7}, xmm5, DWORD PTR [edx+508]	 # AVX512 Disp8
	vscalefss	xmm6{k7}, xmm5, DWORD PTR [edx+512]	 # AVX512
	vscalefss	xmm6{k7}, xmm5, DWORD PTR [edx-512]	 # AVX512 Disp8
	vscalefss	xmm6{k7}, xmm5, DWORD PTR [edx-516]	 # AVX512

	vfixupimmss	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512
	vfixupimmss	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512
	vfixupimmss	xmm6{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512
	vfixupimmss	xmm6{k7}, xmm5, xmm4, 123	 # AVX512
	vfixupimmss	xmm6{k7}, xmm5, xmm4, {sae}, 123	 # AVX512
	vfixupimmss	xmm6{k7}, xmm5, DWORD PTR [ecx], 123	 # AVX512
	vfixupimmss	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456], 123	 # AVX512
	vfixupimmss	xmm6{k7}, xmm5, DWORD PTR [edx+508], 123	 # AVX512 Disp8
	vfixupimmss	xmm6{k7}, xmm5, DWORD PTR [edx+512], 123	 # AVX512
	vfixupimmss	xmm6{k7}, xmm5, DWORD PTR [edx-512], 123	 # AVX512 Disp8
	vfixupimmss	xmm6{k7}, xmm5, DWORD PTR [edx-516], 123	 # AVX512

	vfixupimmsd	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512
	vfixupimmsd	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512
	vfixupimmsd	xmm6{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512
	vfixupimmsd	xmm6{k7}, xmm5, xmm4, 123	 # AVX512
	vfixupimmsd	xmm6{k7}, xmm5, xmm4, {sae}, 123	 # AVX512
	vfixupimmsd	xmm6{k7}, xmm5, QWORD PTR [ecx], 123	 # AVX512
	vfixupimmsd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456], 123	 # AVX512
	vfixupimmsd	xmm6{k7}, xmm5, QWORD PTR [edx+1016], 123	 # AVX512 Disp8
	vfixupimmsd	xmm6{k7}, xmm5, QWORD PTR [edx+1024], 123	 # AVX512
	vfixupimmsd	xmm6{k7}, xmm5, QWORD PTR [edx-1024], 123	 # AVX512 Disp8
	vfixupimmsd	xmm6{k7}, xmm5, QWORD PTR [edx-1032], 123	 # AVX512

	vrndscalesd	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512
	vrndscalesd	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512
	vrndscalesd	xmm6{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512
	vrndscalesd	xmm6{k7}, xmm5, xmm4, 123	 # AVX512
	vrndscalesd	xmm6{k7}, xmm5, xmm4, {sae}, 123	 # AVX512
	vrndscalesd	xmm6{k7}, xmm5, QWORD PTR [ecx], 123	 # AVX512
	vrndscalesd	xmm6{k7}, xmm5, QWORD PTR [esp+esi*8-123456], 123	 # AVX512
	vrndscalesd	xmm6{k7}, xmm5, QWORD PTR [edx+1016], 123	 # AVX512 Disp8
	vrndscalesd	xmm6{k7}, xmm5, QWORD PTR [edx+1024], 123	 # AVX512
	vrndscalesd	xmm6{k7}, xmm5, QWORD PTR [edx-1024], 123	 # AVX512 Disp8
	vrndscalesd	xmm6{k7}, xmm5, QWORD PTR [edx-1032], 123	 # AVX512

	vrndscaless	xmm6{k7}, xmm5, xmm4, 0xab	 # AVX512
	vrndscaless	xmm6{k7}{z}, xmm5, xmm4, 0xab	 # AVX512
	vrndscaless	xmm6{k7}, xmm5, xmm4, {sae}, 0xab	 # AVX512
	vrndscaless	xmm6{k7}, xmm5, xmm4, 123	 # AVX512
	vrndscaless	xmm6{k7}, xmm5, xmm4, {sae}, 123	 # AVX512
	vrndscaless	xmm6{k7}, xmm5, DWORD PTR [ecx], 123	 # AVX512
	vrndscaless	xmm6{k7}, xmm5, DWORD PTR [esp+esi*8-123456], 123	 # AVX512
	vrndscaless	xmm6{k7}, xmm5, DWORD PTR [edx+508], 123	 # AVX512 Disp8
	vrndscaless	xmm6{k7}, xmm5, DWORD PTR [edx+512], 123	 # AVX512
	vrndscaless	xmm6{k7}, xmm5, DWORD PTR [edx-512], 123	 # AVX512 Disp8
	vrndscaless	xmm6{k7}, xmm5, DWORD PTR [edx-516], 123	 # AVX512

	vcmpsh	k5, xmm5, xmm4, 123	# AVX512-FP16
	vcmpsh	k5{k7}, xmm5, xmm4, {sae}, 123	# AVX512-FP16
	vcmpsh	k5, xmm5, WORD PTR [ecx], 123	# AVX512-FP16
	vcmpsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456], 123	# AVX512-FP16
	vcmpsh	k5, xmm5, WORD PTR [ecx+254], 123	# AVX512-FP16 Disp8
	vcmpsh	k5{k7}, xmm5, WORD PTR [edx-256], 123	# AVX512-FP16 Disp8

	vfpclasssh	k5, xmm4, 123	# AVX512-FP16
	vfpclasssh	k5, WORD PTR [ecx], 123	# AVX512-FP16
	vfpclasssh	k5{k7}, WORD PTR [esp+esi*8-123456], 123	# AVX512-FP16
	vfpclasssh	k5, WORD PTR [ecx+254], 123	# AVX512-FP16 Disp8
	vfpclasssh	k5{k7}, WORD PTR [edx-256], 123	# AVX512-FP16 Disp8
