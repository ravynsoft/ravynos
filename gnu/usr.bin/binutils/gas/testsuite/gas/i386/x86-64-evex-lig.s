# Check EVEX LIG instructions

	.allow_index_reg
	.text
_start:

	vaddsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vaddsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vaddsd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vaddsd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vaddsd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vaddsd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vaddsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vaddsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vaddsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vaddsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vaddsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vaddsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vaddss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vaddss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vaddss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vaddss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vaddss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vaddss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vaddss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vaddss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vaddss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vaddss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vaddss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vaddss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vcmpsd	$0xab, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpsd	$0xab, {sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpsd	$123, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpsd	$123, {sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpsd	$123, (%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpsd	$123, 0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpsd	$123, 1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpsd	$123, 1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpsd	$123, -1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpsd	$123, -1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpeq_oqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_oqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_oqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_oqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_oqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_oqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_oqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_oqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpeqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpeqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmplt_ossd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmplt_ossd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmplt_ossd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmplt_ossd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmplt_ossd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmplt_ossd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmplt_ossd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmplt_ossd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpltsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpltsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpltsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpltsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpltsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpltsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpltsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpltsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmple_ossd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmple_ossd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmple_ossd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmple_ossd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmple_ossd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmple_ossd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmple_ossd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmple_ossd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmplesd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmplesd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmplesd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmplesd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmplesd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmplesd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmplesd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmplesd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpunord_qsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_qsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_qsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_qsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_qsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpunord_qsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_qsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpunord_qsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpunordsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpunordsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpunordsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpunordsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpunordsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpunordsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpunordsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpunordsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpneq_uqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_uqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_uqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_uqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_uqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_uqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_uqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_uqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpneqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpneqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpnlt_ussd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_ussd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_ussd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_ussd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_ussd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnlt_ussd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_ussd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnlt_ussd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpnltsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnltsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnltsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnltsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpnltsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnltsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnltsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnltsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpnle_ussd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_ussd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_ussd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_ussd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_ussd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnle_ussd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_ussd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnle_ussd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpnlesd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnlesd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnlesd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnlesd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpnlesd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnlesd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnlesd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnlesd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpord_qsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpord_qsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpord_qsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpord_qsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpord_qsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpord_qsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpord_qsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpord_qsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpordsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpordsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpordsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpordsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpordsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpordsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpordsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpordsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpeq_uqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_uqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_uqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_uqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_uqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_uqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_uqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_uqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpnge_ussd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_ussd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_ussd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_ussd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_ussd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnge_ussd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_ussd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnge_ussd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpngesd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpngesd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpngesd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpngesd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpngesd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpngesd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpngesd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpngesd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpngt_ussd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_ussd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_ussd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_ussd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_ussd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpngt_ussd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_ussd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpngt_ussd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpngtsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpngtsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpngtsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpngtsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpngtsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpngtsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpngtsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpngtsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpfalse_oqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_oqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_oqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_oqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_oqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpfalse_oqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_oqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpfalse_oqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpfalsesd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpfalsesd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpfalsesd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalsesd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalsesd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpfalsesd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalsesd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpfalsesd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpneq_oqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_oqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_oqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_oqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_oqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_oqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_oqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_oqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpge_ossd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpge_ossd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpge_ossd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpge_ossd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpge_ossd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpge_ossd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpge_ossd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpge_ossd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpgesd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpgesd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpgesd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpgesd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpgesd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpgesd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpgesd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpgesd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpgt_ossd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_ossd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_ossd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_ossd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_ossd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpgt_ossd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_ossd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpgt_ossd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpgtsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpgtsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpgtsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpgtsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpgtsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpgtsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpgtsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpgtsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmptrue_uqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_uqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_uqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_uqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_uqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmptrue_uqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_uqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmptrue_uqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmptruesd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmptruesd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmptruesd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmptruesd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmptruesd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmptruesd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmptruesd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmptruesd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpeq_ossd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_ossd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_ossd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_ossd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_ossd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_ossd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_ossd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_ossd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmplt_oqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmplt_oqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmplt_oqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmplt_oqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmplt_oqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmplt_oqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmplt_oqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmplt_oqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmple_oqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmple_oqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmple_oqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmple_oqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmple_oqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmple_oqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmple_oqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmple_oqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpunord_ssd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_ssd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_ssd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_ssd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_ssd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpunord_ssd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_ssd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpunord_ssd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpneq_ussd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_ussd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_ussd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_ussd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_ussd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_ussd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_ussd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_ussd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpnlt_uqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_uqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_uqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_uqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_uqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnlt_uqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_uqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnlt_uqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpnle_uqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_uqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_uqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_uqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_uqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnle_uqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_uqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnle_uqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpord_ssd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpord_ssd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpord_ssd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpord_ssd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpord_ssd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpord_ssd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpord_ssd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpord_ssd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpeq_ussd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_ussd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_ussd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_ussd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_ussd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_ussd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_ussd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_ussd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpnge_uqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_uqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_uqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_uqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_uqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnge_uqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_uqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnge_uqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpngt_uqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_uqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_uqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_uqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_uqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpngt_uqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_uqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpngt_uqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpfalse_ossd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_ossd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_ossd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_ossd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_ossd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpfalse_ossd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_ossd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpfalse_ossd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpneq_ossd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_ossd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_ossd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_ossd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_ossd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_ossd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_ossd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_ossd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpge_oqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpge_oqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpge_oqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpge_oqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpge_oqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpge_oqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpge_oqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpge_oqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpgt_oqsd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_oqsd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_oqsd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_oqsd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_oqsd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpgt_oqsd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_oqsd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpgt_oqsd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmptrue_ussd	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_ussd	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_ussd	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_ussd	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_ussd	1016(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmptrue_ussd	1024(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_ussd	-1024(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmptrue_ussd	-1032(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpss	$0xab, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpss	$0xab, {sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpss	$123, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpss	$123, {sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpss	$123, (%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpss	$123, 0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpss	$123, 508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpss	$123, 512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpss	$123, -512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpss	$123, -516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpeq_oqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_oqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_oqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_oqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_oqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_oqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_oqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_oqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpeqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpeqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmplt_osss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmplt_osss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmplt_osss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmplt_osss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmplt_osss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmplt_osss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmplt_osss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmplt_osss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpltss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpltss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpltss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpltss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpltss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpltss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpltss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpltss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmple_osss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmple_osss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmple_osss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmple_osss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmple_osss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmple_osss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmple_osss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmple_osss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpless	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpless	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpless	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpless	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpless	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpless	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpless	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpless	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpunord_qss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_qss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_qss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_qss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_qss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpunord_qss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_qss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpunord_qss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpunordss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpunordss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpunordss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpunordss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpunordss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpunordss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpunordss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpunordss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpneq_uqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_uqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_uqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_uqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_uqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_uqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_uqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_uqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpneqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpneqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpnlt_usss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_usss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_usss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_usss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_usss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnlt_usss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_usss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnlt_usss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpnltss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnltss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnltss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnltss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpnltss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnltss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnltss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnltss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpnle_usss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_usss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_usss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_usss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_usss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnle_usss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_usss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnle_usss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpnless	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnless	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnless	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnless	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpnless	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnless	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnless	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnless	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpord_qss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpord_qss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpord_qss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpord_qss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpord_qss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpord_qss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpord_qss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpord_qss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpordss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpordss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpordss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpordss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpordss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpordss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpordss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpordss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpeq_uqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_uqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_uqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_uqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_uqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_uqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_uqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_uqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpnge_usss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_usss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_usss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_usss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_usss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnge_usss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_usss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnge_usss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpngess	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpngess	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpngess	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpngess	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpngess	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpngess	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpngess	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpngess	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpngt_usss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_usss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_usss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_usss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_usss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpngt_usss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_usss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpngt_usss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpngtss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpngtss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpngtss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpngtss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpngtss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpngtss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpngtss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpngtss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpfalse_oqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_oqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_oqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_oqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_oqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpfalse_oqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_oqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpfalse_oqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpfalsess	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpfalsess	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpfalsess	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalsess	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalsess	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpfalsess	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalsess	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpfalsess	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpneq_oqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_oqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_oqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_oqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_oqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_oqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_oqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_oqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpge_osss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpge_osss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpge_osss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpge_osss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpge_osss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpge_osss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpge_osss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpge_osss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpgess	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpgess	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpgess	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpgess	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpgess	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpgess	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpgess	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpgess	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpgt_osss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_osss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_osss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_osss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_osss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpgt_osss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_osss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpgt_osss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpgtss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpgtss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpgtss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpgtss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpgtss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpgtss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpgtss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpgtss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmptrue_uqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_uqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_uqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_uqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_uqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmptrue_uqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_uqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmptrue_uqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmptruess	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmptruess	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmptruess	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmptruess	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmptruess	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmptruess	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmptruess	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmptruess	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpeq_osss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_osss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_osss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_osss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_osss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_osss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_osss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_osss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmplt_oqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmplt_oqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmplt_oqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmplt_oqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmplt_oqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmplt_oqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmplt_oqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmplt_oqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmple_oqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmple_oqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmple_oqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmple_oqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmple_oqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmple_oqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmple_oqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmple_oqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpunord_sss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_sss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_sss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_sss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_sss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpunord_sss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpunord_sss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpunord_sss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpneq_usss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_usss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_usss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_usss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_usss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_usss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_usss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_usss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpnlt_uqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_uqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_uqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_uqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_uqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnlt_uqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnlt_uqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnlt_uqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpnle_uqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_uqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_uqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_uqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_uqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnle_uqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnle_uqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnle_uqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpord_sss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpord_sss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpord_sss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpord_sss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpord_sss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpord_sss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpord_sss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpord_sss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpeq_usss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_usss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_usss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_usss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_usss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_usss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpeq_usss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpeq_usss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpnge_uqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_uqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_uqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_uqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_uqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnge_uqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpnge_uqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpnge_uqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpngt_uqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_uqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_uqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_uqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_uqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpngt_uqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpngt_uqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpngt_uqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpfalse_osss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_osss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_osss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_osss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_osss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpfalse_osss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpfalse_osss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpfalse_osss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpneq_osss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_osss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_osss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_osss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_osss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_osss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpneq_osss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpneq_osss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpge_oqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpge_oqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpge_oqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpge_oqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpge_oqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpge_oqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpge_oqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpge_oqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmpgt_oqss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_oqss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_oqss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_oqss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_oqss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpgt_oqss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmpgt_oqss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmpgt_oqss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcmptrue_usss	%xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_usss	{sae}, %xmm28, %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_usss	(%rcx), %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_usss	0x123(%rax,%r14,8), %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_usss	508(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmptrue_usss	512(%rdx), %xmm29, %k5{%k7}	 # AVX512
	vcmptrue_usss	-512(%rdx), %xmm29, %k5{%k7}	 # AVX512 Disp8
	vcmptrue_usss	-516(%rdx), %xmm29, %k5{%k7}	 # AVX512

	vcomisd	%xmm29, %xmm30	 # AVX512
	vcomisd	{sae}, %xmm29, %xmm30	 # AVX512
	vcomisd	(%rcx), %xmm30	 # AVX512
	vcomisd	0x123(%rax,%r14,8), %xmm30	 # AVX512
	vcomisd	1016(%rdx), %xmm30	 # AVX512 Disp8
	vcomisd	1024(%rdx), %xmm30	 # AVX512
	vcomisd	-1024(%rdx), %xmm30	 # AVX512 Disp8
	vcomisd	-1032(%rdx), %xmm30	 # AVX512

	vcomiss	%xmm29, %xmm30	 # AVX512
	vcomiss	{sae}, %xmm29, %xmm30	 # AVX512
	vcomiss	(%rcx), %xmm30	 # AVX512
	vcomiss	0x123(%rax,%r14,8), %xmm30	 # AVX512
	vcomiss	508(%rdx), %xmm30	 # AVX512 Disp8
	vcomiss	512(%rdx), %xmm30	 # AVX512
	vcomiss	-512(%rdx), %xmm30	 # AVX512 Disp8
	vcomiss	-516(%rdx), %xmm30	 # AVX512

	vcvtsd2si	{rn-sae}, %xmm30, %eax	 # AVX512
	vcvtsd2si	{ru-sae}, %xmm30, %eax	 # AVX512
	vcvtsd2si	{rd-sae}, %xmm30, %eax	 # AVX512
	vcvtsd2si	{rz-sae}, %xmm30, %eax	 # AVX512
	vcvtsd2si	{rn-sae}, %xmm30, %ebp	 # AVX512
	vcvtsd2si	{ru-sae}, %xmm30, %ebp	 # AVX512
	vcvtsd2si	{rd-sae}, %xmm30, %ebp	 # AVX512
	vcvtsd2si	{rz-sae}, %xmm30, %ebp	 # AVX512
	vcvtsd2si	{rn-sae}, %xmm30, %r13d	 # AVX512
	vcvtsd2si	{ru-sae}, %xmm30, %r13d	 # AVX512
	vcvtsd2si	{rd-sae}, %xmm30, %r13d	 # AVX512
	vcvtsd2si	{rz-sae}, %xmm30, %r13d	 # AVX512

	vcvtsd2si	{rn-sae}, %xmm30, %rax	 # AVX512
	vcvtsd2si	{ru-sae}, %xmm30, %rax	 # AVX512
	vcvtsd2si	{rd-sae}, %xmm30, %rax	 # AVX512
	vcvtsd2si	{rz-sae}, %xmm30, %rax	 # AVX512
	vcvtsd2si	{rn-sae}, %xmm30, %r8	 # AVX512
	vcvtsd2si	{ru-sae}, %xmm30, %r8	 # AVX512
	vcvtsd2si	{rd-sae}, %xmm30, %r8	 # AVX512
	vcvtsd2si	{rz-sae}, %xmm30, %r8	 # AVX512

	vcvtsd2ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vcvtsd2ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vcvtsd2ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vcvtsd2ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vcvtsd2ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vcvtsd2ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vcvtsd2ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vcvtsd2ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vcvtsd2ss	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vcvtsd2ss	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vcvtsd2ss	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vcvtsd2ss	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vcvtsi2sdl	%eax, %xmm29, %xmm30	 # AVX512
	vcvtsi2sdl	%ebp, %xmm29, %xmm30	 # AVX512
	vcvtsi2sdl	%r13d, %xmm29, %xmm30	 # AVX512
	vcvtsi2sdl	(%rcx), %xmm29, %xmm30	 # AVX512
	vcvtsi2sdl	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512
	vcvtsi2sdl	508(%rdx), %xmm29, %xmm30	 # AVX512 Disp8
	vcvtsi2sdl	512(%rdx), %xmm29, %xmm30	 # AVX512
	vcvtsi2sdl	-512(%rdx), %xmm29, %xmm30	 # AVX512 Disp8
	vcvtsi2sdl	-516(%rdx), %xmm29, %xmm30	 # AVX512

	vcvtsi2sdq	%rax, %xmm29, %xmm30	 # AVX512
	vcvtsi2sdq	%rax, {rn-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2sdq	%rax, {ru-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2sdq	%rax, {rd-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2sdq	%rax, {rz-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2sdq	%r8, %xmm29, %xmm30	 # AVX512
	vcvtsi2sdq	%r8, {rn-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2sdq	%r8, {ru-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2sdq	%r8, {rd-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2sdq	%r8, {rz-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2sdq	(%rcx), %xmm29, %xmm30	 # AVX512
	vcvtsi2sdq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512
	vcvtsi2sdq	1016(%rdx), %xmm29, %xmm30	 # AVX512 Disp8
	vcvtsi2sdq	1024(%rdx), %xmm29, %xmm30	 # AVX512
	vcvtsi2sdq	-1024(%rdx), %xmm29, %xmm30	 # AVX512 Disp8
	vcvtsi2sdq	-1032(%rdx), %xmm29, %xmm30	 # AVX512

	vcvtsi2ssl	%eax, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	%eax, {rn-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	%eax, {ru-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	%eax, {rd-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	%eax, {rz-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	%ebp, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	%ebp, {rn-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	%ebp, {ru-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	%ebp, {rd-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	%ebp, {rz-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	%r13d, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	%r13d, {rn-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	%r13d, {ru-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	%r13d, {rd-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	%r13d, {rz-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	(%rcx), %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	508(%rdx), %xmm29, %xmm30	 # AVX512 Disp8
	vcvtsi2ssl	512(%rdx), %xmm29, %xmm30	 # AVX512
	vcvtsi2ssl	-512(%rdx), %xmm29, %xmm30	 # AVX512 Disp8
	vcvtsi2ssl	-516(%rdx), %xmm29, %xmm30	 # AVX512

	vcvtsi2ssq	%rax, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssq	%rax, {rn-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssq	%rax, {ru-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssq	%rax, {rd-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssq	%rax, {rz-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssq	%r8, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssq	%r8, {rn-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssq	%r8, {ru-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssq	%r8, {rd-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssq	%r8, {rz-sae}, %xmm29, %xmm30	 # AVX512
	vcvtsi2ssq	(%rcx), %xmm29, %xmm30	 # AVX512
	vcvtsi2ssq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512
	vcvtsi2ssq	1016(%rdx), %xmm29, %xmm30	 # AVX512 Disp8
	vcvtsi2ssq	1024(%rdx), %xmm29, %xmm30	 # AVX512
	vcvtsi2ssq	-1024(%rdx), %xmm29, %xmm30	 # AVX512 Disp8
	vcvtsi2ssq	-1032(%rdx), %xmm29, %xmm30	 # AVX512

	vcvtss2sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vcvtss2sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vcvtss2sd	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vcvtss2sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vcvtss2sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vcvtss2sd	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vcvtss2sd	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vcvtss2sd	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vcvtss2sd	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vcvtss2si	{rn-sae}, %xmm30, %eax	 # AVX512
	vcvtss2si	{ru-sae}, %xmm30, %eax	 # AVX512
	vcvtss2si	{rd-sae}, %xmm30, %eax	 # AVX512
	vcvtss2si	{rz-sae}, %xmm30, %eax	 # AVX512
	vcvtss2si	{rn-sae}, %xmm30, %ebp	 # AVX512
	vcvtss2si	{ru-sae}, %xmm30, %ebp	 # AVX512
	vcvtss2si	{rd-sae}, %xmm30, %ebp	 # AVX512
	vcvtss2si	{rz-sae}, %xmm30, %ebp	 # AVX512
	vcvtss2si	{rn-sae}, %xmm30, %r13d	 # AVX512
	vcvtss2si	{ru-sae}, %xmm30, %r13d	 # AVX512
	vcvtss2si	{rd-sae}, %xmm30, %r13d	 # AVX512
	vcvtss2si	{rz-sae}, %xmm30, %r13d	 # AVX512

	vcvtss2si	{rn-sae}, %xmm30, %rax	 # AVX512
	vcvtss2si	{ru-sae}, %xmm30, %rax	 # AVX512
	vcvtss2si	{rd-sae}, %xmm30, %rax	 # AVX512
	vcvtss2si	{rz-sae}, %xmm30, %rax	 # AVX512
	vcvtss2si	{rn-sae}, %xmm30, %r8	 # AVX512
	vcvtss2si	{ru-sae}, %xmm30, %r8	 # AVX512
	vcvtss2si	{rd-sae}, %xmm30, %r8	 # AVX512
	vcvtss2si	{rz-sae}, %xmm30, %r8	 # AVX512

	vcvttsd2si	{sae}, %xmm30, %eax	 # AVX512
	vcvttsd2si	{sae}, %xmm30, %ebp	 # AVX512
	vcvttsd2si	{sae}, %xmm30, %r13d	 # AVX512

	vcvttsd2si	{sae}, %xmm30, %rax	 # AVX512
	vcvttsd2si	{sae}, %xmm30, %r8	 # AVX512

	vcvttss2si	{sae}, %xmm30, %eax	 # AVX512
	vcvttss2si	{sae}, %xmm30, %ebp	 # AVX512
	vcvttss2si	{sae}, %xmm30, %r13d	 # AVX512

	vcvttss2si	{sae}, %xmm30, %rax	 # AVX512
	vcvttss2si	{sae}, %xmm30, %r8	 # AVX512

	vdivsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vdivsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vdivsd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vdivsd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vdivsd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vdivsd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vdivsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vdivsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vdivsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vdivsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vdivsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vdivsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vdivss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vdivss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vdivss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vdivss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vdivss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vdivss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vdivss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vdivss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vdivss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vdivss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vdivss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vdivss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfmadd132sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd132sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfmadd132sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd132sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd132sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd132sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd132sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd132sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd132sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmadd132sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd132sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmadd132sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfmadd132ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd132ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfmadd132ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd132ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd132ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd132ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd132ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd132ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd132ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmadd132ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd132ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmadd132ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfmadd213sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd213sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfmadd213sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd213sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd213sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd213sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd213sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd213sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd213sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmadd213sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd213sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmadd213sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfmadd213ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd213ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfmadd213ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd213ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd213ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd213ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd213ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd213ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd213ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmadd213ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd213ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmadd213ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfmadd231sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd231sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfmadd231sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd231sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd231sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd231sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd231sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd231sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd231sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmadd231sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd231sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmadd231sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfmadd231ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd231ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfmadd231ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd231ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd231ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd231ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd231ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd231ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd231ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmadd231ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmadd231ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmadd231ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfmsub132sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub132sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfmsub132sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub132sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub132sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub132sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub132sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub132sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub132sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmsub132sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub132sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmsub132sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfmsub132ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub132ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfmsub132ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub132ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub132ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub132ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub132ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub132ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub132ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmsub132ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub132ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmsub132ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfmsub213sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub213sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfmsub213sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub213sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub213sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub213sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub213sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub213sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub213sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmsub213sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub213sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmsub213sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfmsub213ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub213ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfmsub213ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub213ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub213ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub213ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub213ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub213ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub213ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmsub213ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub213ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmsub213ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfmsub231sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub231sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfmsub231sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub231sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub231sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub231sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub231sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub231sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub231sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmsub231sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub231sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmsub231sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfmsub231ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub231ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfmsub231ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub231ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub231ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub231ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub231ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub231ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub231ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmsub231ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfmsub231ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfmsub231ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfnmadd132sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd132sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfnmadd132sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd132sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd132sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd132sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd132sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd132sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd132sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmadd132sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd132sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmadd132sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfnmadd132ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd132ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfnmadd132ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd132ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd132ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd132ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd132ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd132ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd132ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmadd132ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd132ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmadd132ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfnmadd213sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd213sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfnmadd213sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd213sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd213sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd213sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd213sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd213sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd213sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmadd213sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd213sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmadd213sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfnmadd213ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd213ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfnmadd213ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd213ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd213ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd213ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd213ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd213ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd213ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmadd213ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd213ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmadd213ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfnmadd231sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd231sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfnmadd231sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd231sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd231sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd231sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd231sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd231sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd231sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmadd231sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd231sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmadd231sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfnmadd231ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd231ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfnmadd231ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd231ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd231ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd231ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd231ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd231ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd231ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmadd231ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmadd231ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmadd231ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfnmsub132sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub132sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfnmsub132sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub132sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub132sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub132sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub132sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub132sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub132sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmsub132sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub132sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmsub132sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfnmsub132ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub132ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfnmsub132ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub132ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub132ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub132ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub132ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub132ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub132ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmsub132ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub132ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmsub132ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfnmsub213sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub213sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfnmsub213sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub213sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub213sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub213sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub213sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub213sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub213sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmsub213sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub213sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmsub213sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfnmsub213ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub213ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfnmsub213ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub213ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub213ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub213ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub213ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub213ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub213ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmsub213ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub213ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmsub213ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfnmsub231sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub231sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfnmsub231sd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub231sd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub231sd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub231sd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub231sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub231sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub231sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmsub231sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub231sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmsub231sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfnmsub231ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub231ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfnmsub231ss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub231ss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub231ss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub231ss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub231ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub231ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub231ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmsub231ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfnmsub231ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfnmsub231ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vgetexpsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vgetexpsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vgetexpsd	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vgetexpsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vgetexpsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vgetexpsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vgetexpsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vgetexpsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vgetexpsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vgetexpss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vgetexpss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vgetexpss	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vgetexpss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vgetexpss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vgetexpss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vgetexpss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vgetexpss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vgetexpss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vgetmantsd	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vgetmantsd	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vgetmantsd	$0xab, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vgetmantsd	$123, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vgetmantsd	$123, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vgetmantsd	$123, (%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vgetmantsd	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vgetmantsd	$123, 1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vgetmantsd	$123, 1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vgetmantsd	$123, -1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vgetmantsd	$123, -1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vgetmantss	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vgetmantss	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vgetmantss	$0xab, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vgetmantss	$123, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vgetmantss	$123, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vgetmantss	$123, (%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vgetmantss	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vgetmantss	$123, 508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vgetmantss	$123, 512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vgetmantss	$123, -512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vgetmantss	$123, -516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vmaxsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vmaxsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vmaxsd	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vmaxsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vmaxsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vmaxsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vmaxsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vmaxsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vmaxsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vmaxss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vmaxss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vmaxss	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vmaxss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vmaxss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vmaxss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vmaxss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vmaxss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vmaxss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vminsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vminsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vminsd	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vminsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vminsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vminsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vminsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vminsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vminsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vminss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vminss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vminss	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vminss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vminss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vminss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vminss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vminss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vminss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vmovsd	(%rcx), %xmm30{%k7}	 # AVX512
	vmovsd	(%rcx), %xmm30{%k7}{z}	 # AVX512
	vmovsd	0x123(%rax,%r14,8), %xmm30{%k7}	 # AVX512
	vmovsd	1016(%rdx), %xmm30{%k7}	 # AVX512 Disp8
	vmovsd	1024(%rdx), %xmm30{%k7}	 # AVX512
	vmovsd	-1024(%rdx), %xmm30{%k7}	 # AVX512 Disp8
	vmovsd	-1032(%rdx), %xmm30{%k7}	 # AVX512

	vmovsd	%xmm30, (%rcx){%k7}	 # AVX512
	vmovsd	%xmm30, 0x123(%rax,%r14,8){%k7}	 # AVX512
	vmovsd	%xmm30, 1016(%rdx){%k7}	 # AVX512 Disp8
	vmovsd	%xmm30, 1024(%rdx){%k7}	 # AVX512
	vmovsd	%xmm30, -1024(%rdx){%k7}	 # AVX512 Disp8
	vmovsd	%xmm30, -1032(%rdx){%k7}	 # AVX512

	vmovsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vmovsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512

	vmovss	(%rcx), %xmm30{%k7}	 # AVX512
	vmovss	(%rcx), %xmm30{%k7}{z}	 # AVX512
	vmovss	0x123(%rax,%r14,8), %xmm30{%k7}	 # AVX512
	vmovss	508(%rdx), %xmm30{%k7}	 # AVX512 Disp8
	vmovss	512(%rdx), %xmm30{%k7}	 # AVX512
	vmovss	-512(%rdx), %xmm30{%k7}	 # AVX512 Disp8
	vmovss	-516(%rdx), %xmm30{%k7}	 # AVX512

	vmovss	%xmm30, (%rcx){%k7}	 # AVX512
	vmovss	%xmm30, 0x123(%rax,%r14,8){%k7}	 # AVX512
	vmovss	%xmm30, 508(%rdx){%k7}	 # AVX512 Disp8
	vmovss	%xmm30, 512(%rdx){%k7}	 # AVX512
	vmovss	%xmm30, -512(%rdx){%k7}	 # AVX512 Disp8
	vmovss	%xmm30, -516(%rdx){%k7}	 # AVX512

	vmovss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vmovss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512

	vmulsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vmulsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vmulsd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vmulsd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vmulsd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vmulsd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vmulsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vmulsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vmulsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vmulsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vmulsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vmulsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vmulss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vmulss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vmulss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vmulss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vmulss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vmulss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vmulss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vmulss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vmulss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vmulss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vmulss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vmulss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vrcp14sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vrcp14sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vrcp14sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vrcp14sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vrcp14sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vrcp14sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vrcp14sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vrcp14sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vrcp14ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vrcp14ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vrcp14ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vrcp14ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vrcp14ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vrcp14ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vrcp14ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vrcp14ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vrcp28ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrcp28ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512EMI
	vrcp28ss	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrcp28ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrcp28ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrcp28ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512EMI Disp8
	vrcp28ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrcp28ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512EMI Disp8
	vrcp28ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512EMI

	vrcp28sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrcp28sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512EMI
	vrcp28sd	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrcp28sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrcp28sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrcp28sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512EMI Disp8
	vrcp28sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrcp28sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512EMI Disp8
	vrcp28sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512EMI

	vrsqrt14sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vrsqrt14sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vrsqrt14sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vrsqrt14sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vrsqrt14sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vrsqrt14sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vrsqrt14sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vrsqrt14sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vrsqrt14ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vrsqrt14ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vrsqrt14ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vrsqrt14ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vrsqrt14ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vrsqrt14ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vrsqrt14ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vrsqrt14ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vrsqrt28ss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrsqrt28ss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512EMI
	vrsqrt28ss	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrsqrt28ss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrsqrt28ss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrsqrt28ss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512EMI Disp8
	vrsqrt28ss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrsqrt28ss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512EMI Disp8
	vrsqrt28ss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512EMI

	vrsqrt28sd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrsqrt28sd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512EMI
	vrsqrt28sd	{sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrsqrt28sd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrsqrt28sd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrsqrt28sd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512EMI Disp8
	vrsqrt28sd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512EMI
	vrsqrt28sd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512EMI Disp8
	vrsqrt28sd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512EMI

	vsqrtsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsqrtsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vsqrtsd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsqrtsd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsqrtsd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsqrtsd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsqrtsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vsqrtsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vsqrtsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vsqrtsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vsqrtsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vsqrtsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vsqrtss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsqrtss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vsqrtss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsqrtss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsqrtss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsqrtss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsqrtss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vsqrtss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vsqrtss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vsqrtss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vsqrtss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vsqrtss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vsubsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsubsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vsubsd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsubsd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsubsd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsubsd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsubsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vsubsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vsubsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vsubsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vsubsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vsubsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vsubss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsubss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vsubss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsubss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsubss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsubss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vsubss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vsubss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vsubss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vsubss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vsubss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vsubss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vucomisd	%xmm29, %xmm30	 # AVX512
	vucomisd	{sae}, %xmm29, %xmm30	 # AVX512
	vucomisd	(%rcx), %xmm30	 # AVX512
	vucomisd	0x123(%rax,%r14,8), %xmm30	 # AVX512
	vucomisd	1016(%rdx), %xmm30	 # AVX512 Disp8
	vucomisd	1024(%rdx), %xmm30	 # AVX512
	vucomisd	-1024(%rdx), %xmm30	 # AVX512 Disp8
	vucomisd	-1032(%rdx), %xmm30	 # AVX512

	vucomiss	%xmm29, %xmm30	 # AVX512
	vucomiss	{sae}, %xmm29, %xmm30	 # AVX512
	vucomiss	(%rcx), %xmm30	 # AVX512
	vucomiss	0x123(%rax,%r14,8), %xmm30	 # AVX512
	vucomiss	508(%rdx), %xmm30	 # AVX512 Disp8
	vucomiss	512(%rdx), %xmm30	 # AVX512
	vucomiss	-512(%rdx), %xmm30	 # AVX512 Disp8
	vucomiss	-516(%rdx), %xmm30	 # AVX512

	vcvtsd2usi	%xmm30, %eax	 # AVX512
	vcvtsd2usi	{rn-sae}, %xmm30, %eax	 # AVX512
	vcvtsd2usi	{ru-sae}, %xmm30, %eax	 # AVX512
	vcvtsd2usi	{rd-sae}, %xmm30, %eax	 # AVX512
	vcvtsd2usi	{rz-sae}, %xmm30, %eax	 # AVX512
	vcvtsd2usi	(%rcx), %eax	 # AVX512
	vcvtsd2usi	0x123(%rax,%r14,8), %eax	 # AVX512
	vcvtsd2usi	1016(%rdx), %eax	 # AVX512 Disp8
	vcvtsd2usi	1024(%rdx), %eax	 # AVX512
	vcvtsd2usi	-1024(%rdx), %eax	 # AVX512 Disp8
	vcvtsd2usi	-1032(%rdx), %eax	 # AVX512
	vcvtsd2usi	%xmm30, %ebp	 # AVX512
	vcvtsd2usi	{rn-sae}, %xmm30, %ebp	 # AVX512
	vcvtsd2usi	{ru-sae}, %xmm30, %ebp	 # AVX512
	vcvtsd2usi	{rd-sae}, %xmm30, %ebp	 # AVX512
	vcvtsd2usi	{rz-sae}, %xmm30, %ebp	 # AVX512
	vcvtsd2usi	(%rcx), %ebp	 # AVX512
	vcvtsd2usi	0x123(%rax,%r14,8), %ebp	 # AVX512
	vcvtsd2usi	1016(%rdx), %ebp	 # AVX512 Disp8
	vcvtsd2usi	1024(%rdx), %ebp	 # AVX512
	vcvtsd2usi	-1024(%rdx), %ebp	 # AVX512 Disp8
	vcvtsd2usi	-1032(%rdx), %ebp	 # AVX512
	vcvtsd2usi	%xmm30, %r13d	 # AVX512
	vcvtsd2usi	{rn-sae}, %xmm30, %r13d	 # AVX512
	vcvtsd2usi	{ru-sae}, %xmm30, %r13d	 # AVX512
	vcvtsd2usi	{rd-sae}, %xmm30, %r13d	 # AVX512
	vcvtsd2usi	{rz-sae}, %xmm30, %r13d	 # AVX512
	vcvtsd2usi	(%rcx), %r13d	 # AVX512
	vcvtsd2usi	0x123(%rax,%r14,8), %r13d	 # AVX512
	vcvtsd2usi	1016(%rdx), %r13d	 # AVX512 Disp8
	vcvtsd2usi	1024(%rdx), %r13d	 # AVX512
	vcvtsd2usi	-1024(%rdx), %r13d	 # AVX512 Disp8
	vcvtsd2usi	-1032(%rdx), %r13d	 # AVX512

	vcvtsd2usi	%xmm30, %rax	 # AVX512
	vcvtsd2usi	{rn-sae}, %xmm30, %rax	 # AVX512
	vcvtsd2usi	{ru-sae}, %xmm30, %rax	 # AVX512
	vcvtsd2usi	{rd-sae}, %xmm30, %rax	 # AVX512
	vcvtsd2usi	{rz-sae}, %xmm30, %rax	 # AVX512
	vcvtsd2usi	(%rcx), %rax	 # AVX512
	vcvtsd2usi	0x123(%rax,%r14,8), %rax	 # AVX512
	vcvtsd2usi	1016(%rdx), %rax	 # AVX512 Disp8
	vcvtsd2usi	1024(%rdx), %rax	 # AVX512
	vcvtsd2usi	-1024(%rdx), %rax	 # AVX512 Disp8
	vcvtsd2usi	-1032(%rdx), %rax	 # AVX512
	vcvtsd2usi	%xmm30, %r8	 # AVX512
	vcvtsd2usi	{rn-sae}, %xmm30, %r8	 # AVX512
	vcvtsd2usi	{ru-sae}, %xmm30, %r8	 # AVX512
	vcvtsd2usi	{rd-sae}, %xmm30, %r8	 # AVX512
	vcvtsd2usi	{rz-sae}, %xmm30, %r8	 # AVX512
	vcvtsd2usi	(%rcx), %r8	 # AVX512
	vcvtsd2usi	0x123(%rax,%r14,8), %r8	 # AVX512
	vcvtsd2usi	1016(%rdx), %r8	 # AVX512 Disp8
	vcvtsd2usi	1024(%rdx), %r8	 # AVX512
	vcvtsd2usi	-1024(%rdx), %r8	 # AVX512 Disp8
	vcvtsd2usi	-1032(%rdx), %r8	 # AVX512

	vcvtss2usi	%xmm30, %eax	 # AVX512
	vcvtss2usi	{rn-sae}, %xmm30, %eax	 # AVX512
	vcvtss2usi	{ru-sae}, %xmm30, %eax	 # AVX512
	vcvtss2usi	{rd-sae}, %xmm30, %eax	 # AVX512
	vcvtss2usi	{rz-sae}, %xmm30, %eax	 # AVX512
	vcvtss2usi	(%rcx), %eax	 # AVX512
	vcvtss2usi	0x123(%rax,%r14,8), %eax	 # AVX512
	vcvtss2usi	508(%rdx), %eax	 # AVX512 Disp8
	vcvtss2usi	512(%rdx), %eax	 # AVX512
	vcvtss2usi	-512(%rdx), %eax	 # AVX512 Disp8
	vcvtss2usi	-516(%rdx), %eax	 # AVX512
	vcvtss2usi	%xmm30, %ebp	 # AVX512
	vcvtss2usi	{rn-sae}, %xmm30, %ebp	 # AVX512
	vcvtss2usi	{ru-sae}, %xmm30, %ebp	 # AVX512
	vcvtss2usi	{rd-sae}, %xmm30, %ebp	 # AVX512
	vcvtss2usi	{rz-sae}, %xmm30, %ebp	 # AVX512
	vcvtss2usi	(%rcx), %ebp	 # AVX512
	vcvtss2usi	0x123(%rax,%r14,8), %ebp	 # AVX512
	vcvtss2usi	508(%rdx), %ebp	 # AVX512 Disp8
	vcvtss2usi	512(%rdx), %ebp	 # AVX512
	vcvtss2usi	-512(%rdx), %ebp	 # AVX512 Disp8
	vcvtss2usi	-516(%rdx), %ebp	 # AVX512
	vcvtss2usi	%xmm30, %r13d	 # AVX512
	vcvtss2usi	{rn-sae}, %xmm30, %r13d	 # AVX512
	vcvtss2usi	{ru-sae}, %xmm30, %r13d	 # AVX512
	vcvtss2usi	{rd-sae}, %xmm30, %r13d	 # AVX512
	vcvtss2usi	{rz-sae}, %xmm30, %r13d	 # AVX512
	vcvtss2usi	(%rcx), %r13d	 # AVX512
	vcvtss2usi	0x123(%rax,%r14,8), %r13d	 # AVX512
	vcvtss2usi	508(%rdx), %r13d	 # AVX512 Disp8
	vcvtss2usi	512(%rdx), %r13d	 # AVX512
	vcvtss2usi	-512(%rdx), %r13d	 # AVX512 Disp8
	vcvtss2usi	-516(%rdx), %r13d	 # AVX512

	vcvtss2usi	%xmm30, %rax	 # AVX512
	vcvtss2usi	{rn-sae}, %xmm30, %rax	 # AVX512
	vcvtss2usi	{ru-sae}, %xmm30, %rax	 # AVX512
	vcvtss2usi	{rd-sae}, %xmm30, %rax	 # AVX512
	vcvtss2usi	{rz-sae}, %xmm30, %rax	 # AVX512
	vcvtss2usi	(%rcx), %rax	 # AVX512
	vcvtss2usi	0x123(%rax,%r14,8), %rax	 # AVX512
	vcvtss2usi	508(%rdx), %rax	 # AVX512 Disp8
	vcvtss2usi	512(%rdx), %rax	 # AVX512
	vcvtss2usi	-512(%rdx), %rax	 # AVX512 Disp8
	vcvtss2usi	-516(%rdx), %rax	 # AVX512
	vcvtss2usi	%xmm30, %r8	 # AVX512
	vcvtss2usi	{rn-sae}, %xmm30, %r8	 # AVX512
	vcvtss2usi	{ru-sae}, %xmm30, %r8	 # AVX512
	vcvtss2usi	{rd-sae}, %xmm30, %r8	 # AVX512
	vcvtss2usi	{rz-sae}, %xmm30, %r8	 # AVX512
	vcvtss2usi	(%rcx), %r8	 # AVX512
	vcvtss2usi	0x123(%rax,%r14,8), %r8	 # AVX512
	vcvtss2usi	508(%rdx), %r8	 # AVX512 Disp8
	vcvtss2usi	512(%rdx), %r8	 # AVX512
	vcvtss2usi	-512(%rdx), %r8	 # AVX512 Disp8
	vcvtss2usi	-516(%rdx), %r8	 # AVX512

	vcvtusi2sdl	%eax, %xmm29, %xmm30	 # AVX512
	vcvtusi2sdl	%ebp, %xmm29, %xmm30	 # AVX512
	vcvtusi2sdl	%r13d, %xmm29, %xmm30	 # AVX512
	vcvtusi2sdl	(%rcx), %xmm29, %xmm30	 # AVX512
	vcvtusi2sdl	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512
	vcvtusi2sdl	508(%rdx), %xmm29, %xmm30	 # AVX512 Disp8
	vcvtusi2sdl	512(%rdx), %xmm29, %xmm30	 # AVX512
	vcvtusi2sdl	-512(%rdx), %xmm29, %xmm30	 # AVX512 Disp8
	vcvtusi2sdl	-516(%rdx), %xmm29, %xmm30	 # AVX512

	vcvtusi2sdq	%rax, %xmm29, %xmm30	 # AVX512
	vcvtusi2sdq	%rax, {rn-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2sdq	%rax, {ru-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2sdq	%rax, {rd-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2sdq	%rax, {rz-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2sdq	%r8, %xmm29, %xmm30	 # AVX512
	vcvtusi2sdq	%r8, {rn-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2sdq	%r8, {ru-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2sdq	%r8, {rd-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2sdq	%r8, {rz-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2sdq	(%rcx), %xmm29, %xmm30	 # AVX512
	vcvtusi2sdq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512
	vcvtusi2sdq	1016(%rdx), %xmm29, %xmm30	 # AVX512 Disp8
	vcvtusi2sdq	1024(%rdx), %xmm29, %xmm30	 # AVX512
	vcvtusi2sdq	-1024(%rdx), %xmm29, %xmm30	 # AVX512 Disp8
	vcvtusi2sdq	-1032(%rdx), %xmm29, %xmm30	 # AVX512

	vcvtusi2ssl	%eax, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	%eax, {rn-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	%eax, {ru-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	%eax, {rd-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	%eax, {rz-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	%ebp, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	%ebp, {rn-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	%ebp, {ru-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	%ebp, {rd-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	%ebp, {rz-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	%r13d, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	%r13d, {rn-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	%r13d, {ru-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	%r13d, {rd-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	%r13d, {rz-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	(%rcx), %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	508(%rdx), %xmm29, %xmm30	 # AVX512 Disp8
	vcvtusi2ssl	512(%rdx), %xmm29, %xmm30	 # AVX512
	vcvtusi2ssl	-512(%rdx), %xmm29, %xmm30	 # AVX512 Disp8
	vcvtusi2ssl	-516(%rdx), %xmm29, %xmm30	 # AVX512

	vcvtusi2ssq	%rax, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssq	%rax, {rn-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssq	%rax, {ru-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssq	%rax, {rd-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssq	%rax, {rz-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssq	%r8, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssq	%r8, {rn-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssq	%r8, {ru-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssq	%r8, {rd-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssq	%r8, {rz-sae}, %xmm29, %xmm30	 # AVX512
	vcvtusi2ssq	(%rcx), %xmm29, %xmm30	 # AVX512
	vcvtusi2ssq	0x123(%rax,%r14,8), %xmm29, %xmm30	 # AVX512
	vcvtusi2ssq	1016(%rdx), %xmm29, %xmm30	 # AVX512 Disp8
	vcvtusi2ssq	1024(%rdx), %xmm29, %xmm30	 # AVX512
	vcvtusi2ssq	-1024(%rdx), %xmm29, %xmm30	 # AVX512 Disp8
	vcvtusi2ssq	-1032(%rdx), %xmm29, %xmm30	 # AVX512

	vscalefsd	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vscalefsd	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vscalefsd	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vscalefsd	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vscalefsd	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vscalefsd	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vscalefsd	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vscalefsd	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vscalefsd	1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vscalefsd	1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vscalefsd	-1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vscalefsd	-1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vscalefss	%xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vscalefss	%xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vscalefss	{rn-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vscalefss	{ru-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vscalefss	{rd-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vscalefss	{rz-sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vscalefss	(%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vscalefss	0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vscalefss	508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vscalefss	512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vscalefss	-512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vscalefss	-516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfixupimmss	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfixupimmss	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfixupimmss	$0xab, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfixupimmss	$123, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfixupimmss	$123, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfixupimmss	$123, (%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfixupimmss	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfixupimmss	$123, 508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfixupimmss	$123, 512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfixupimmss	$123, -512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfixupimmss	$123, -516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vfixupimmsd	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfixupimmsd	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vfixupimmsd	$0xab, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfixupimmsd	$123, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfixupimmsd	$123, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vfixupimmsd	$123, (%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vfixupimmsd	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vfixupimmsd	$123, 1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfixupimmsd	$123, 1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vfixupimmsd	$123, -1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vfixupimmsd	$123, -1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vrndscalesd	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vrndscalesd	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vrndscalesd	$0xab, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vrndscalesd	$123, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vrndscalesd	$123, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vrndscalesd	$123, (%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vrndscalesd	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vrndscalesd	$123, 1016(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vrndscalesd	$123, 1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vrndscalesd	$123, -1024(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vrndscalesd	$123, -1032(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	vrndscaless	$0xab, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vrndscaless	$0xab, %xmm28, %xmm29, %xmm30{%k7}{z}	 # AVX512
	vrndscaless	$0xab, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vrndscaless	$123, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vrndscaless	$123, {sae}, %xmm28, %xmm29, %xmm30{%k7}	 # AVX512
	vrndscaless	$123, (%rcx), %xmm29, %xmm30{%k7}	 # AVX512
	vrndscaless	$123, 0x123(%rax,%r14,8), %xmm29, %xmm30{%k7}	 # AVX512
	vrndscaless	$123, 508(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vrndscaless	$123, 512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512
	vrndscaless	$123, -512(%rdx), %xmm29, %xmm30{%k7}	 # AVX512 Disp8
	vrndscaless	$123, -516(%rdx), %xmm29, %xmm30{%k7}	 # AVX512

	.intel_syntax noprefix
	vaddsd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vaddsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vaddsd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vaddsd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vaddsd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vaddsd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vaddsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vaddsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vaddsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vaddsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vaddsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vaddsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vaddss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vaddss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vaddss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vaddss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vaddss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vaddss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vaddss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vaddss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vaddss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vaddss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vaddss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vaddss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpsd	k5{k7}, xmm29, xmm28, 0xab	 # AVX512
	vcmpsd	k5{k7}, xmm29, xmm28, {sae}, 0xab	 # AVX512
	vcmpsd	k5{k7}, xmm29, xmm28, 123	 # AVX512
	vcmpsd	k5{k7}, xmm29, xmm28, {sae}, 123	 # AVX512
	vcmpsd	k5{k7}, xmm29, QWORD PTR [rcx], 123	 # AVX512
	vcmpsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234], 123	 # AVX512
	vcmpsd	k5{k7}, xmm29, QWORD PTR [rdx+1016], 123	 # AVX512 Disp8
	vcmpsd	k5{k7}, xmm29, QWORD PTR [rdx+1024], 123	 # AVX512
	vcmpsd	k5{k7}, xmm29, QWORD PTR [rdx-1024], 123	 # AVX512 Disp8
	vcmpsd	k5{k7}, xmm29, QWORD PTR [rdx-1032], 123	 # AVX512

	vcmpeq_oqsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpeq_oqsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpeq_oqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpeq_oqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpeq_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpeq_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpeq_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpeq_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpeqsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpeqsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpeqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpeqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpeqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpeqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpeqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpeqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmplt_ossd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmplt_ossd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmplt_ossd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmplt_ossd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmplt_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmplt_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmplt_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmplt_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpltsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpltsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpltsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpltsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpltsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpltsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpltsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpltsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmple_ossd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmple_ossd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmple_ossd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmple_ossd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmple_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmple_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmple_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmple_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmplesd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmplesd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmplesd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmplesd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmplesd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmplesd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmplesd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmplesd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpunord_qsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpunord_qsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpunord_qsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpunord_qsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpunord_qsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpunord_qsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpunord_qsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpunord_qsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpunordsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpunordsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpunordsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpunordsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpunordsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpunordsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpunordsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpunordsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpneq_uqsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpneq_uqsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpneq_uqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpneq_uqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpneq_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpneq_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpneq_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpneq_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpneqsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpneqsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpneqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpneqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpneqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpneqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpneqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpneqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpnlt_ussd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpnlt_ussd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpnlt_ussd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpnlt_ussd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpnlt_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpnlt_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpnlt_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpnlt_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpnltsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpnltsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpnltsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpnltsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpnltsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpnltsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpnltsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpnltsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpnle_ussd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpnle_ussd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpnle_ussd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpnle_ussd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpnle_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpnle_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpnle_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpnle_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpnlesd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpnlesd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpnlesd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpnlesd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpnlesd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpnlesd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpnlesd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpnlesd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpord_qsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpord_qsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpord_qsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpord_qsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpord_qsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpord_qsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpord_qsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpord_qsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpordsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpordsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpordsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpordsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpordsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpordsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpordsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpordsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpeq_uqsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpeq_uqsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpeq_uqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpeq_uqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpeq_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpeq_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpeq_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpeq_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpnge_ussd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpnge_ussd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpnge_ussd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpnge_ussd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpnge_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpnge_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpnge_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpnge_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpngesd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpngesd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpngesd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpngesd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpngesd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpngesd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpngesd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpngesd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpngt_ussd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpngt_ussd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpngt_ussd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpngt_ussd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpngt_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpngt_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpngt_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpngt_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpngtsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpngtsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpngtsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpngtsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpngtsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpngtsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpngtsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpngtsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpfalse_oqsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpfalse_oqsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpfalse_oqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpfalse_oqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpfalse_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpfalse_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpfalse_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpfalse_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpfalsesd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpfalsesd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpfalsesd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpfalsesd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpfalsesd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpfalsesd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpfalsesd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpfalsesd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpneq_oqsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpneq_oqsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpneq_oqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpneq_oqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpneq_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpneq_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpneq_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpneq_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpge_ossd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpge_ossd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpge_ossd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpge_ossd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpge_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpge_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpge_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpge_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpgesd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpgesd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpgesd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpgesd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpgesd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpgesd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpgesd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpgesd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpgt_ossd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpgt_ossd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpgt_ossd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpgt_ossd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpgt_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpgt_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpgt_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpgt_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpgtsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpgtsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpgtsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpgtsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpgtsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpgtsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpgtsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpgtsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmptrue_uqsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmptrue_uqsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmptrue_uqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmptrue_uqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmptrue_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmptrue_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmptrue_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmptrue_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmptruesd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmptruesd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmptruesd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmptruesd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmptruesd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmptruesd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmptruesd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmptruesd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpeq_ossd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpeq_ossd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpeq_ossd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpeq_ossd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpeq_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpeq_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpeq_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpeq_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmplt_oqsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmplt_oqsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmplt_oqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmplt_oqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmplt_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmplt_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmplt_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmplt_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmple_oqsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmple_oqsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmple_oqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmple_oqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmple_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmple_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmple_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmple_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpunord_ssd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpunord_ssd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpunord_ssd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpunord_ssd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpunord_ssd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpunord_ssd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpunord_ssd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpunord_ssd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpneq_ussd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpneq_ussd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpneq_ussd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpneq_ussd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpneq_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpneq_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpneq_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpneq_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpnlt_uqsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpnlt_uqsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpnlt_uqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpnlt_uqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpnlt_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpnlt_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpnlt_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpnlt_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpnle_uqsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpnle_uqsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpnle_uqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpnle_uqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpnle_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpnle_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpnle_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpnle_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpord_ssd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpord_ssd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpord_ssd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpord_ssd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpord_ssd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpord_ssd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpord_ssd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpord_ssd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpeq_ussd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpeq_ussd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpeq_ussd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpeq_ussd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpeq_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpeq_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpeq_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpeq_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpnge_uqsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpnge_uqsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpnge_uqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpnge_uqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpnge_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpnge_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpnge_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpnge_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpngt_uqsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpngt_uqsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpngt_uqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpngt_uqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpngt_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpngt_uqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpngt_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpngt_uqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpfalse_ossd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpfalse_ossd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpfalse_ossd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpfalse_ossd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpfalse_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpfalse_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpfalse_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpfalse_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpneq_ossd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpneq_ossd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpneq_ossd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpneq_ossd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpneq_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpneq_ossd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpneq_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpneq_ossd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpge_oqsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpge_oqsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpge_oqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpge_oqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpge_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpge_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpge_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpge_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpgt_oqsd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpgt_oqsd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpgt_oqsd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmpgt_oqsd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpgt_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmpgt_oqsd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmpgt_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmpgt_oqsd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmptrue_ussd	k5{k7}, xmm29, xmm28	 # AVX512
	vcmptrue_ussd	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmptrue_ussd	k5{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcmptrue_ussd	k5{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmptrue_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcmptrue_ussd	k5{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcmptrue_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcmptrue_ussd	k5{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcmpss	k5{k7}, xmm29, xmm28, 0xab	 # AVX512
	vcmpss	k5{k7}, xmm29, xmm28, {sae}, 0xab	 # AVX512
	vcmpss	k5{k7}, xmm29, xmm28, 123	 # AVX512
	vcmpss	k5{k7}, xmm29, xmm28, {sae}, 123	 # AVX512
	vcmpss	k5{k7}, xmm29, DWORD PTR [rcx], 123	 # AVX512
	vcmpss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234], 123	 # AVX512
	vcmpss	k5{k7}, xmm29, DWORD PTR [rdx+508], 123	 # AVX512 Disp8
	vcmpss	k5{k7}, xmm29, DWORD PTR [rdx+512], 123	 # AVX512
	vcmpss	k5{k7}, xmm29, DWORD PTR [rdx-512], 123	 # AVX512 Disp8
	vcmpss	k5{k7}, xmm29, DWORD PTR [rdx-516], 123	 # AVX512

	vcmpeq_oqss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpeq_oqss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpeq_oqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpeq_oqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpeq_oqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpeq_oqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpeq_oqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpeq_oqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpeqss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpeqss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpeqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpeqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpeqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpeqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpeqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpeqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmplt_osss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmplt_osss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmplt_osss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmplt_osss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmplt_osss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmplt_osss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmplt_osss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmplt_osss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpltss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpltss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpltss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpltss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpltss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpltss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpltss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpltss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmple_osss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmple_osss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmple_osss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmple_osss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmple_osss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmple_osss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmple_osss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmple_osss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpless	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpless	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpless	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpless	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpless	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpless	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpless	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpless	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpunord_qss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpunord_qss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpunord_qss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpunord_qss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpunord_qss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpunord_qss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpunord_qss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpunord_qss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpunordss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpunordss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpunordss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpunordss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpunordss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpunordss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpunordss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpunordss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpneq_uqss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpneq_uqss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpneq_uqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpneq_uqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpneq_uqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpneq_uqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpneq_uqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpneq_uqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpneqss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpneqss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpneqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpneqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpneqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpneqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpneqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpneqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpnlt_usss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpnlt_usss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpnlt_usss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpnlt_usss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpnlt_usss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpnlt_usss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpnlt_usss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpnlt_usss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpnltss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpnltss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpnltss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpnltss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpnltss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpnltss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpnltss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpnltss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpnle_usss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpnle_usss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpnle_usss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpnle_usss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpnle_usss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpnle_usss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpnle_usss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpnle_usss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpnless	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpnless	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpnless	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpnless	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpnless	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpnless	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpnless	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpnless	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpord_qss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpord_qss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpord_qss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpord_qss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpord_qss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpord_qss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpord_qss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpord_qss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpordss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpordss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpordss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpordss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpordss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpordss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpordss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpordss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpeq_uqss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpeq_uqss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpeq_uqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpeq_uqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpeq_uqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpeq_uqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpeq_uqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpeq_uqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpnge_usss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpnge_usss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpnge_usss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpnge_usss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpnge_usss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpnge_usss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpnge_usss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpnge_usss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpngess	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpngess	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpngess	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpngess	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpngess	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpngess	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpngess	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpngess	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpngt_usss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpngt_usss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpngt_usss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpngt_usss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpngt_usss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpngt_usss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpngt_usss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpngt_usss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpngtss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpngtss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpngtss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpngtss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpngtss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpngtss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpngtss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpngtss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpfalse_oqss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpfalse_oqss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpfalse_oqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpfalse_oqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpfalse_oqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpfalse_oqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpfalse_oqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpfalse_oqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpfalsess	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpfalsess	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpfalsess	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpfalsess	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpfalsess	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpfalsess	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpfalsess	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpfalsess	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpneq_oqss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpneq_oqss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpneq_oqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpneq_oqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpneq_oqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpneq_oqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpneq_oqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpneq_oqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpge_osss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpge_osss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpge_osss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpge_osss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpge_osss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpge_osss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpge_osss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpge_osss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpgess	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpgess	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpgess	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpgess	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpgess	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpgess	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpgess	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpgess	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpgt_osss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpgt_osss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpgt_osss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpgt_osss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpgt_osss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpgt_osss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpgt_osss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpgt_osss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpgtss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpgtss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpgtss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpgtss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpgtss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpgtss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpgtss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpgtss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmptrue_uqss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmptrue_uqss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmptrue_uqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmptrue_uqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmptrue_uqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmptrue_uqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmptrue_uqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmptrue_uqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmptruess	k5{k7}, xmm29, xmm28	 # AVX512
	vcmptruess	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmptruess	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmptruess	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmptruess	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmptruess	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmptruess	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmptruess	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpeq_osss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpeq_osss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpeq_osss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpeq_osss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpeq_osss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpeq_osss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpeq_osss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpeq_osss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmplt_oqss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmplt_oqss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmplt_oqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmplt_oqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmplt_oqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmplt_oqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmplt_oqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmplt_oqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmple_oqss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmple_oqss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmple_oqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmple_oqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmple_oqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmple_oqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmple_oqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmple_oqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpunord_sss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpunord_sss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpunord_sss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpunord_sss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpunord_sss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpunord_sss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpunord_sss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpunord_sss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpneq_usss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpneq_usss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpneq_usss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpneq_usss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpneq_usss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpneq_usss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpneq_usss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpneq_usss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpnlt_uqss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpnlt_uqss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpnlt_uqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpnlt_uqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpnlt_uqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpnlt_uqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpnlt_uqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpnlt_uqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpnle_uqss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpnle_uqss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpnle_uqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpnle_uqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpnle_uqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpnle_uqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpnle_uqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpnle_uqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpord_sss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpord_sss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpord_sss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpord_sss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpord_sss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpord_sss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpord_sss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpord_sss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpeq_usss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpeq_usss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpeq_usss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpeq_usss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpeq_usss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpeq_usss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpeq_usss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpeq_usss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpnge_uqss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpnge_uqss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpnge_uqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpnge_uqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpnge_uqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpnge_uqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpnge_uqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpnge_uqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpngt_uqss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpngt_uqss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpngt_uqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpngt_uqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpngt_uqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpngt_uqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpngt_uqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpngt_uqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpfalse_osss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpfalse_osss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpfalse_osss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpfalse_osss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpfalse_osss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpfalse_osss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpfalse_osss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpfalse_osss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpneq_osss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpneq_osss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpneq_osss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpneq_osss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpneq_osss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpneq_osss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpneq_osss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpneq_osss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpge_oqss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpge_oqss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpge_oqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpge_oqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpge_oqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpge_oqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpge_oqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpge_oqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmpgt_oqss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmpgt_oqss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmpgt_oqss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmpgt_oqss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmpgt_oqss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmpgt_oqss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmpgt_oqss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmpgt_oqss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcmptrue_usss	k5{k7}, xmm29, xmm28	 # AVX512
	vcmptrue_usss	k5{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcmptrue_usss	k5{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcmptrue_usss	k5{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcmptrue_usss	k5{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcmptrue_usss	k5{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcmptrue_usss	k5{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcmptrue_usss	k5{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcomisd	xmm30, xmm29	 # AVX512
	vcomisd	xmm30, xmm29, {sae}	 # AVX512
	vcomisd	xmm30, QWORD PTR [rcx]	 # AVX512
	vcomisd	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcomisd	xmm30, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcomisd	xmm30, QWORD PTR [rdx+1024]	 # AVX512
	vcomisd	xmm30, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcomisd	xmm30, QWORD PTR [rdx-1032]	 # AVX512

	vcomiss	xmm30, xmm29	 # AVX512
	vcomiss	xmm30, xmm29, {sae}	 # AVX512
	vcomiss	xmm30, DWORD PTR [rcx]	 # AVX512
	vcomiss	xmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcomiss	xmm30, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcomiss	xmm30, DWORD PTR [rdx+512]	 # AVX512
	vcomiss	xmm30, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcomiss	xmm30, DWORD PTR [rdx-516]	 # AVX512

	vcvtsd2si	eax, xmm30, {rn-sae}	 # AVX512
	vcvtsd2si	eax, xmm30, {ru-sae}	 # AVX512
	vcvtsd2si	eax, xmm30, {rd-sae}	 # AVX512
	vcvtsd2si	eax, xmm30, {rz-sae}	 # AVX512
	vcvtsd2si	ebp, xmm30, {rn-sae}	 # AVX512
	vcvtsd2si	ebp, xmm30, {ru-sae}	 # AVX512
	vcvtsd2si	ebp, xmm30, {rd-sae}	 # AVX512
	vcvtsd2si	ebp, xmm30, {rz-sae}	 # AVX512
	vcvtsd2si	r13d, xmm30, {rn-sae}	 # AVX512
	vcvtsd2si	r13d, xmm30, {ru-sae}	 # AVX512
	vcvtsd2si	r13d, xmm30, {rd-sae}	 # AVX512
	vcvtsd2si	r13d, xmm30, {rz-sae}	 # AVX512

	vcvtsd2si	rax, xmm30, {rn-sae}	 # AVX512
	vcvtsd2si	rax, xmm30, {ru-sae}	 # AVX512
	vcvtsd2si	rax, xmm30, {rd-sae}	 # AVX512
	vcvtsd2si	rax, xmm30, {rz-sae}	 # AVX512
	vcvtsd2si	r8, xmm30, {rn-sae}	 # AVX512
	vcvtsd2si	r8, xmm30, {ru-sae}	 # AVX512
	vcvtsd2si	r8, xmm30, {rd-sae}	 # AVX512
	vcvtsd2si	r8, xmm30, {rz-sae}	 # AVX512

	vcvtsd2ss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vcvtsd2ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vcvtsd2ss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vcvtsd2ss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vcvtsd2ss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vcvtsd2ss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vcvtsd2ss	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vcvtsd2ss	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtsd2ss	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcvtsd2ss	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcvtsd2ss	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcvtsd2ss	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcvtsi2sd	xmm30, xmm29, eax	 # AVX512
	vcvtsi2sd	xmm30, xmm29, ebp	 # AVX512
	vcvtsi2sd	xmm30, xmm29, r13d	 # AVX512
	vcvtsi2sd	xmm30, xmm29, DWORD PTR [rcx]	 # AVX512
	vcvtsi2sd	xmm30, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtsi2sd	xmm30, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcvtsi2sd	xmm30, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcvtsi2sd	xmm30, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcvtsi2sd	xmm30, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcvtsi2sd	xmm30, xmm29, rax	 # AVX512
	vcvtsi2sd	xmm30, xmm29, {rn-sae}, rax	 # AVX512
	vcvtsi2sd	xmm30, xmm29, {ru-sae}, rax	 # AVX512
	vcvtsi2sd	xmm30, xmm29, {rd-sae}, rax	 # AVX512
	vcvtsi2sd	xmm30, xmm29, {rz-sae}, rax	 # AVX512
	vcvtsi2sd	xmm30, xmm29, r8	 # AVX512
	vcvtsi2sd	xmm30, xmm29, {rn-sae}, r8	 # AVX512
	vcvtsi2sd	xmm30, xmm29, {ru-sae}, r8	 # AVX512
	vcvtsi2sd	xmm30, xmm29, {rd-sae}, r8	 # AVX512
	vcvtsi2sd	xmm30, xmm29, {rz-sae}, r8	 # AVX512
	vcvtsi2sd	xmm30, xmm29, QWORD PTR [rcx]	 # AVX512
	vcvtsi2sd	xmm30, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtsi2sd	xmm30, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcvtsi2sd	xmm30, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcvtsi2sd	xmm30, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcvtsi2sd	xmm30, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcvtsi2ss	xmm30, xmm29, eax	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {rn-sae}, eax	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {ru-sae}, eax	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {rd-sae}, eax	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {rz-sae}, eax	 # AVX512
	vcvtsi2ss	xmm30, xmm29, ebp	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {rn-sae}, ebp	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {ru-sae}, ebp	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {rd-sae}, ebp	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {rz-sae}, ebp	 # AVX512
	vcvtsi2ss	xmm30, xmm29, r13d	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {rn-sae}, r13d	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {ru-sae}, r13d	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {rd-sae}, r13d	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {rz-sae}, r13d	 # AVX512
	vcvtsi2ss	xmm30, xmm29, DWORD PTR [rcx]	 # AVX512
	vcvtsi2ss	xmm30, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtsi2ss	xmm30, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcvtsi2ss	xmm30, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcvtsi2ss	xmm30, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcvtsi2ss	xmm30, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcvtsi2ss	xmm30, xmm29, rax	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {rn-sae}, rax	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {ru-sae}, rax	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {rd-sae}, rax	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {rz-sae}, rax	 # AVX512
	vcvtsi2ss	xmm30, xmm29, r8	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {rn-sae}, r8	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {ru-sae}, r8	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {rd-sae}, r8	 # AVX512
	vcvtsi2ss	xmm30, xmm29, {rz-sae}, r8	 # AVX512
	vcvtsi2ss	xmm30, xmm29, QWORD PTR [rcx]	 # AVX512
	vcvtsi2ss	xmm30, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtsi2ss	xmm30, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcvtsi2ss	xmm30, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcvtsi2ss	xmm30, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcvtsi2ss	xmm30, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcvtss2sd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vcvtss2sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vcvtss2sd	xmm30{k7}, xmm29, xmm28, {sae}	 # AVX512
	vcvtss2sd	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vcvtss2sd	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtss2sd	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcvtss2sd	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcvtss2sd	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcvtss2sd	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcvtss2si	eax, xmm30, {rn-sae}	 # AVX512
	vcvtss2si	eax, xmm30, {ru-sae}	 # AVX512
	vcvtss2si	eax, xmm30, {rd-sae}	 # AVX512
	vcvtss2si	eax, xmm30, {rz-sae}	 # AVX512
	vcvtss2si	ebp, xmm30, {rn-sae}	 # AVX512
	vcvtss2si	ebp, xmm30, {ru-sae}	 # AVX512
	vcvtss2si	ebp, xmm30, {rd-sae}	 # AVX512
	vcvtss2si	ebp, xmm30, {rz-sae}	 # AVX512
	vcvtss2si	r13d, xmm30, {rn-sae}	 # AVX512
	vcvtss2si	r13d, xmm30, {ru-sae}	 # AVX512
	vcvtss2si	r13d, xmm30, {rd-sae}	 # AVX512
	vcvtss2si	r13d, xmm30, {rz-sae}	 # AVX512

	vcvtss2si	rax, xmm30, {rn-sae}	 # AVX512
	vcvtss2si	rax, xmm30, {ru-sae}	 # AVX512
	vcvtss2si	rax, xmm30, {rd-sae}	 # AVX512
	vcvtss2si	rax, xmm30, {rz-sae}	 # AVX512
	vcvtss2si	r8, xmm30, {rn-sae}	 # AVX512
	vcvtss2si	r8, xmm30, {ru-sae}	 # AVX512
	vcvtss2si	r8, xmm30, {rd-sae}	 # AVX512
	vcvtss2si	r8, xmm30, {rz-sae}	 # AVX512

	vcvttsd2si	eax, xmm30, {sae}	 # AVX512
	vcvttsd2si	ebp, xmm30, {sae}	 # AVX512
	vcvttsd2si	r13d, xmm30, {sae}	 # AVX512

	vcvttsd2si	rax, xmm30, {sae}	 # AVX512
	vcvttsd2si	r8, xmm30, {sae}	 # AVX512

	vcvttss2si	eax, xmm30, {sae}	 # AVX512
	vcvttss2si	ebp, xmm30, {sae}	 # AVX512
	vcvttss2si	r13d, xmm30, {sae}	 # AVX512

	vcvttss2si	rax, xmm30, {sae}	 # AVX512
	vcvttss2si	r8, xmm30, {sae}	 # AVX512

	vdivsd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vdivsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vdivsd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vdivsd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vdivsd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vdivsd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vdivsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vdivsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vdivsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vdivsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vdivsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vdivsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vdivss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vdivss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vdivss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vdivss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vdivss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vdivss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vdivss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vdivss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vdivss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vdivss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vdivss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vdivss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vfmadd132sd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfmadd132sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfmadd132sd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfmadd132sd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfmadd132sd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfmadd132sd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vfmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vfmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vfmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vfmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vfmadd132ss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfmadd132ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfmadd132ss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfmadd132ss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfmadd132ss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfmadd132ss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vfmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vfmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vfmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vfmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vfmadd213sd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfmadd213sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfmadd213sd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfmadd213sd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfmadd213sd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfmadd213sd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vfmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vfmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vfmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vfmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vfmadd213ss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfmadd213ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfmadd213ss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfmadd213ss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfmadd213ss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfmadd213ss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vfmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vfmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vfmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vfmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vfmadd231sd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfmadd231sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfmadd231sd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfmadd231sd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfmadd231sd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfmadd231sd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vfmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vfmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vfmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vfmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vfmadd231ss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfmadd231ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfmadd231ss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfmadd231ss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfmadd231ss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfmadd231ss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vfmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vfmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vfmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vfmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vfmsub132sd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfmsub132sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfmsub132sd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfmsub132sd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfmsub132sd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfmsub132sd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vfmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vfmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vfmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vfmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vfmsub132ss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfmsub132ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfmsub132ss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfmsub132ss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfmsub132ss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfmsub132ss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vfmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vfmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vfmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vfmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vfmsub213sd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfmsub213sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfmsub213sd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfmsub213sd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfmsub213sd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfmsub213sd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vfmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vfmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vfmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vfmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vfmsub213ss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfmsub213ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfmsub213ss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfmsub213ss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfmsub213ss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfmsub213ss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vfmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vfmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vfmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vfmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vfmsub231sd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfmsub231sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfmsub231sd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfmsub231sd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfmsub231sd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfmsub231sd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vfmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vfmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vfmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vfmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vfmsub231ss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfmsub231ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfmsub231ss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfmsub231ss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfmsub231ss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfmsub231ss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vfmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vfmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vfmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vfmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vfnmadd132sd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfnmadd132sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfnmadd132sd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfnmadd132sd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfnmadd132sd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfnmadd132sd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfnmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vfnmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfnmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vfnmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vfnmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vfnmadd132sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vfnmadd132ss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfnmadd132ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfnmadd132ss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfnmadd132ss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfnmadd132ss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfnmadd132ss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfnmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vfnmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfnmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vfnmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vfnmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vfnmadd132ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vfnmadd213sd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfnmadd213sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfnmadd213sd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfnmadd213sd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfnmadd213sd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfnmadd213sd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfnmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vfnmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfnmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vfnmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vfnmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vfnmadd213sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vfnmadd213ss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfnmadd213ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfnmadd213ss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfnmadd213ss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfnmadd213ss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfnmadd213ss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfnmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vfnmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfnmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vfnmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vfnmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vfnmadd213ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vfnmadd231sd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfnmadd231sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfnmadd231sd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfnmadd231sd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfnmadd231sd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfnmadd231sd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfnmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vfnmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfnmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vfnmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vfnmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vfnmadd231sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vfnmadd231ss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfnmadd231ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfnmadd231ss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfnmadd231ss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfnmadd231ss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfnmadd231ss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfnmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vfnmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfnmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vfnmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vfnmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vfnmadd231ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vfnmsub132sd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfnmsub132sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfnmsub132sd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfnmsub132sd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfnmsub132sd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfnmsub132sd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfnmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vfnmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfnmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vfnmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vfnmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vfnmsub132sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vfnmsub132ss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfnmsub132ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfnmsub132ss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfnmsub132ss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfnmsub132ss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfnmsub132ss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfnmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vfnmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfnmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vfnmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vfnmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vfnmsub132ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vfnmsub213sd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfnmsub213sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfnmsub213sd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfnmsub213sd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfnmsub213sd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfnmsub213sd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfnmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vfnmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfnmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vfnmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vfnmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vfnmsub213sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vfnmsub213ss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfnmsub213ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfnmsub213ss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfnmsub213ss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfnmsub213ss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfnmsub213ss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfnmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vfnmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfnmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vfnmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vfnmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vfnmsub213ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vfnmsub231sd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfnmsub231sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfnmsub231sd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfnmsub231sd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfnmsub231sd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfnmsub231sd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfnmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vfnmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfnmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vfnmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vfnmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vfnmsub231sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vfnmsub231ss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vfnmsub231ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vfnmsub231ss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vfnmsub231ss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vfnmsub231ss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vfnmsub231ss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vfnmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vfnmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vfnmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vfnmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vfnmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vfnmsub231ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vgetexpsd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vgetexpsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vgetexpsd	xmm30{k7}, xmm29, xmm28, {sae}	 # AVX512
	vgetexpsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vgetexpsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vgetexpsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vgetexpsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vgetexpsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vgetexpsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vgetexpss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vgetexpss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vgetexpss	xmm30{k7}, xmm29, xmm28, {sae}	 # AVX512
	vgetexpss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vgetexpss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vgetexpss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vgetexpss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vgetexpss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vgetexpss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vgetmantsd	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512
	vgetmantsd	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512
	vgetmantsd	xmm30{k7}, xmm29, xmm28, {sae}, 0xab	 # AVX512
	vgetmantsd	xmm30{k7}, xmm29, xmm28, 123	 # AVX512
	vgetmantsd	xmm30{k7}, xmm29, xmm28, {sae}, 123	 # AVX512
	vgetmantsd	xmm30{k7}, xmm29, QWORD PTR [rcx], 123	 # AVX512
	vgetmantsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234], 123	 # AVX512
	vgetmantsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016], 123	 # AVX512 Disp8
	vgetmantsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024], 123	 # AVX512
	vgetmantsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024], 123	 # AVX512 Disp8
	vgetmantsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032], 123	 # AVX512

	vgetmantss	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512
	vgetmantss	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512
	vgetmantss	xmm30{k7}, xmm29, xmm28, {sae}, 0xab	 # AVX512
	vgetmantss	xmm30{k7}, xmm29, xmm28, 123	 # AVX512
	vgetmantss	xmm30{k7}, xmm29, xmm28, {sae}, 123	 # AVX512
	vgetmantss	xmm30{k7}, xmm29, DWORD PTR [rcx], 123	 # AVX512
	vgetmantss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234], 123	 # AVX512
	vgetmantss	xmm30{k7}, xmm29, DWORD PTR [rdx+508], 123	 # AVX512 Disp8
	vgetmantss	xmm30{k7}, xmm29, DWORD PTR [rdx+512], 123	 # AVX512
	vgetmantss	xmm30{k7}, xmm29, DWORD PTR [rdx-512], 123	 # AVX512 Disp8
	vgetmantss	xmm30{k7}, xmm29, DWORD PTR [rdx-516], 123	 # AVX512

	vmaxsd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vmaxsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vmaxsd	xmm30{k7}, xmm29, xmm28, {sae}	 # AVX512
	vmaxsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vmaxsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vmaxsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vmaxsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vmaxsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vmaxsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vmaxss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vmaxss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vmaxss	xmm30{k7}, xmm29, xmm28, {sae}	 # AVX512
	vmaxss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vmaxss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vmaxss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vmaxss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vmaxss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vmaxss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vminsd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vminsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vminsd	xmm30{k7}, xmm29, xmm28, {sae}	 # AVX512
	vminsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vminsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vminsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vminsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vminsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vminsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vminss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vminss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vminss	xmm30{k7}, xmm29, xmm28, {sae}	 # AVX512
	vminss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vminss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vminss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vminss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vminss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vminss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vmovsd	xmm30{k7}, QWORD PTR [rcx]	 # AVX512
	vmovsd	xmm30{k7}{z}, QWORD PTR [rcx]	 # AVX512
	vmovsd	xmm30{k7}, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vmovsd	xmm30{k7}, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vmovsd	xmm30{k7}, QWORD PTR [rdx+1024]	 # AVX512
	vmovsd	xmm30{k7}, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vmovsd	xmm30{k7}, QWORD PTR [rdx-1032]	 # AVX512

	vmovsd	QWORD PTR [rcx]{k7}, xmm30	 # AVX512
	vmovsd	QWORD PTR [rax+r14*8+0x1234]{k7}, xmm30	 # AVX512
	vmovsd	QWORD PTR [rdx+1016]{k7}, xmm30	 # AVX512 Disp8
	vmovsd	QWORD PTR [rdx+1024]{k7}, xmm30	 # AVX512
	vmovsd	QWORD PTR [rdx-1024]{k7}, xmm30	 # AVX512 Disp8
	vmovsd	QWORD PTR [rdx-1032]{k7}, xmm30	 # AVX512

	vmovsd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vmovsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512

	vmovss	xmm30{k7}, DWORD PTR [rcx]	 # AVX512
	vmovss	xmm30{k7}{z}, DWORD PTR [rcx]	 # AVX512
	vmovss	xmm30{k7}, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vmovss	xmm30{k7}, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vmovss	xmm30{k7}, DWORD PTR [rdx+512]	 # AVX512
	vmovss	xmm30{k7}, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vmovss	xmm30{k7}, DWORD PTR [rdx-516]	 # AVX512

	vmovss	DWORD PTR [rcx]{k7}, xmm30	 # AVX512
	vmovss	DWORD PTR [rax+r14*8+0x1234]{k7}, xmm30	 # AVX512
	vmovss	DWORD PTR [rdx+508]{k7}, xmm30	 # AVX512 Disp8
	vmovss	DWORD PTR [rdx+512]{k7}, xmm30	 # AVX512
	vmovss	DWORD PTR [rdx-512]{k7}, xmm30	 # AVX512 Disp8
	vmovss	DWORD PTR [rdx-516]{k7}, xmm30	 # AVX512

	vmovss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vmovss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512

	vmulsd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vmulsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vmulsd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vmulsd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vmulsd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vmulsd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vmulsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vmulsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vmulsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vmulsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vmulsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vmulsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vmulss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vmulss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vmulss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vmulss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vmulss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vmulss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vmulss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vmulss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vmulss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vmulss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vmulss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vmulss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vrcp14sd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vrcp14sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vrcp14sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vrcp14sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vrcp14sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vrcp14sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vrcp14sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vrcp14sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vrcp14ss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vrcp14ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vrcp14ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vrcp14ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vrcp14ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vrcp14ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vrcp14ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vrcp14ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vrcp28ss	xmm30{k7}, xmm29, xmm28	 # AVX512EMI
	vrcp28ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512EMI
	vrcp28ss	xmm30{k7}, xmm29, xmm28, {sae}	 # AVX512EMI
	vrcp28ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512EMI
	vrcp28ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512EMI
	vrcp28ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512EMI Disp8
	vrcp28ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512EMI
	vrcp28ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512EMI Disp8
	vrcp28ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512EMI

	vrcp28sd	xmm30{k7}, xmm29, xmm28	 # AVX512EMI
	vrcp28sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512EMI
	vrcp28sd	xmm30{k7}, xmm29, xmm28, {sae}	 # AVX512EMI
	vrcp28sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512EMI
	vrcp28sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512EMI
	vrcp28sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512EMI Disp8
	vrcp28sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512EMI
	vrcp28sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512EMI Disp8
	vrcp28sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512EMI

	vrsqrt14sd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vrsqrt14sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vrsqrt14sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vrsqrt14sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vrsqrt14sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vrsqrt14sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vrsqrt14sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vrsqrt14sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vrsqrt14ss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vrsqrt14ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vrsqrt14ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vrsqrt14ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vrsqrt14ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vrsqrt14ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vrsqrt14ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vrsqrt14ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vrsqrt28ss	xmm30{k7}, xmm29, xmm28	 # AVX512EMI
	vrsqrt28ss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512EMI
	vrsqrt28ss	xmm30{k7}, xmm29, xmm28, {sae}	 # AVX512EMI
	vrsqrt28ss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512EMI
	vrsqrt28ss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512EMI
	vrsqrt28ss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512EMI Disp8
	vrsqrt28ss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512EMI
	vrsqrt28ss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512EMI Disp8
	vrsqrt28ss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512EMI

	vrsqrt28sd	xmm30{k7}, xmm29, xmm28	 # AVX512EMI
	vrsqrt28sd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512EMI
	vrsqrt28sd	xmm30{k7}, xmm29, xmm28, {sae}	 # AVX512EMI
	vrsqrt28sd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512EMI
	vrsqrt28sd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512EMI
	vrsqrt28sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512EMI Disp8
	vrsqrt28sd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512EMI
	vrsqrt28sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512EMI Disp8
	vrsqrt28sd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512EMI

	vsqrtsd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vsqrtsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vsqrtsd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vsqrtsd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vsqrtsd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vsqrtsd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vsqrtsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vsqrtsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vsqrtsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vsqrtsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vsqrtsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vsqrtsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vsqrtss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vsqrtss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vsqrtss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vsqrtss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vsqrtss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vsqrtss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vsqrtss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vsqrtss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vsqrtss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vsqrtss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vsqrtss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vsqrtss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vsubsd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vsubsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vsubsd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vsubsd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vsubsd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vsubsd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vsubsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vsubsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vsubsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vsubsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vsubsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vsubsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vsubss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vsubss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vsubss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vsubss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vsubss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vsubss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vsubss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vsubss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vsubss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vsubss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vsubss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vsubss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vucomisd	xmm30, xmm29	 # AVX512
	vucomisd	xmm30, xmm29, {sae}	 # AVX512
	vucomisd	xmm30, QWORD PTR [rcx]	 # AVX512
	vucomisd	xmm30, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vucomisd	xmm30, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vucomisd	xmm30, QWORD PTR [rdx+1024]	 # AVX512
	vucomisd	xmm30, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vucomisd	xmm30, QWORD PTR [rdx-1032]	 # AVX512

	vucomiss	xmm30, xmm29	 # AVX512
	vucomiss	xmm30, xmm29, {sae}	 # AVX512
	vucomiss	xmm30, DWORD PTR [rcx]	 # AVX512
	vucomiss	xmm30, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vucomiss	xmm30, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vucomiss	xmm30, DWORD PTR [rdx+512]	 # AVX512
	vucomiss	xmm30, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vucomiss	xmm30, DWORD PTR [rdx-516]	 # AVX512

	vcvtsd2usi	eax, xmm30	 # AVX512
	vcvtsd2usi	eax, xmm30, {rn-sae}	 # AVX512
	vcvtsd2usi	eax, xmm30, {ru-sae}	 # AVX512
	vcvtsd2usi	eax, xmm30, {rd-sae}	 # AVX512
	vcvtsd2usi	eax, xmm30, {rz-sae}	 # AVX512
	vcvtsd2usi	eax, QWORD PTR [rcx]	 # AVX512
	vcvtsd2usi	eax, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtsd2usi	eax, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcvtsd2usi	eax, QWORD PTR [rdx+1024]	 # AVX512
	vcvtsd2usi	eax, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcvtsd2usi	eax, QWORD PTR [rdx-1032]	 # AVX512
	vcvtsd2usi	ebp, xmm30	 # AVX512
	vcvtsd2usi	ebp, xmm30, {rn-sae}	 # AVX512
	vcvtsd2usi	ebp, xmm30, {ru-sae}	 # AVX512
	vcvtsd2usi	ebp, xmm30, {rd-sae}	 # AVX512
	vcvtsd2usi	ebp, xmm30, {rz-sae}	 # AVX512
	vcvtsd2usi	ebp, QWORD PTR [rcx]	 # AVX512
	vcvtsd2usi	ebp, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtsd2usi	ebp, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcvtsd2usi	ebp, QWORD PTR [rdx+1024]	 # AVX512
	vcvtsd2usi	ebp, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcvtsd2usi	ebp, QWORD PTR [rdx-1032]	 # AVX512
	vcvtsd2usi	r13d, xmm30	 # AVX512
	vcvtsd2usi	r13d, xmm30, {rn-sae}	 # AVX512
	vcvtsd2usi	r13d, xmm30, {ru-sae}	 # AVX512
	vcvtsd2usi	r13d, xmm30, {rd-sae}	 # AVX512
	vcvtsd2usi	r13d, xmm30, {rz-sae}	 # AVX512
	vcvtsd2usi	r13d, QWORD PTR [rcx]	 # AVX512
	vcvtsd2usi	r13d, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtsd2usi	r13d, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcvtsd2usi	r13d, QWORD PTR [rdx+1024]	 # AVX512
	vcvtsd2usi	r13d, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcvtsd2usi	r13d, QWORD PTR [rdx-1032]	 # AVX512

	vcvtsd2usi	rax, xmm30	 # AVX512
	vcvtsd2usi	rax, xmm30, {rn-sae}	 # AVX512
	vcvtsd2usi	rax, xmm30, {ru-sae}	 # AVX512
	vcvtsd2usi	rax, xmm30, {rd-sae}	 # AVX512
	vcvtsd2usi	rax, xmm30, {rz-sae}	 # AVX512
	vcvtsd2usi	rax, QWORD PTR [rcx]	 # AVX512
	vcvtsd2usi	rax, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtsd2usi	rax, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcvtsd2usi	rax, QWORD PTR [rdx+1024]	 # AVX512
	vcvtsd2usi	rax, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcvtsd2usi	rax, QWORD PTR [rdx-1032]	 # AVX512
	vcvtsd2usi	r8, xmm30	 # AVX512
	vcvtsd2usi	r8, xmm30, {rn-sae}	 # AVX512
	vcvtsd2usi	r8, xmm30, {ru-sae}	 # AVX512
	vcvtsd2usi	r8, xmm30, {rd-sae}	 # AVX512
	vcvtsd2usi	r8, xmm30, {rz-sae}	 # AVX512
	vcvtsd2usi	r8, QWORD PTR [rcx]	 # AVX512
	vcvtsd2usi	r8, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtsd2usi	r8, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcvtsd2usi	r8, QWORD PTR [rdx+1024]	 # AVX512
	vcvtsd2usi	r8, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcvtsd2usi	r8, QWORD PTR [rdx-1032]	 # AVX512

	vcvtss2usi	eax, xmm30	 # AVX512
	vcvtss2usi	eax, xmm30, {rn-sae}	 # AVX512
	vcvtss2usi	eax, xmm30, {ru-sae}	 # AVX512
	vcvtss2usi	eax, xmm30, {rd-sae}	 # AVX512
	vcvtss2usi	eax, xmm30, {rz-sae}	 # AVX512
	vcvtss2usi	eax, DWORD PTR [rcx]	 # AVX512
	vcvtss2usi	eax, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtss2usi	eax, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcvtss2usi	eax, DWORD PTR [rdx+512]	 # AVX512
	vcvtss2usi	eax, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcvtss2usi	eax, DWORD PTR [rdx-516]	 # AVX512
	vcvtss2usi	ebp, xmm30	 # AVX512
	vcvtss2usi	ebp, xmm30, {rn-sae}	 # AVX512
	vcvtss2usi	ebp, xmm30, {ru-sae}	 # AVX512
	vcvtss2usi	ebp, xmm30, {rd-sae}	 # AVX512
	vcvtss2usi	ebp, xmm30, {rz-sae}	 # AVX512
	vcvtss2usi	ebp, DWORD PTR [rcx]	 # AVX512
	vcvtss2usi	ebp, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtss2usi	ebp, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcvtss2usi	ebp, DWORD PTR [rdx+512]	 # AVX512
	vcvtss2usi	ebp, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcvtss2usi	ebp, DWORD PTR [rdx-516]	 # AVX512
	vcvtss2usi	r13d, xmm30	 # AVX512
	vcvtss2usi	r13d, xmm30, {rn-sae}	 # AVX512
	vcvtss2usi	r13d, xmm30, {ru-sae}	 # AVX512
	vcvtss2usi	r13d, xmm30, {rd-sae}	 # AVX512
	vcvtss2usi	r13d, xmm30, {rz-sae}	 # AVX512
	vcvtss2usi	r13d, DWORD PTR [rcx]	 # AVX512
	vcvtss2usi	r13d, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtss2usi	r13d, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcvtss2usi	r13d, DWORD PTR [rdx+512]	 # AVX512
	vcvtss2usi	r13d, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcvtss2usi	r13d, DWORD PTR [rdx-516]	 # AVX512

	vcvtss2usi	rax, xmm30	 # AVX512
	vcvtss2usi	rax, xmm30, {rn-sae}	 # AVX512
	vcvtss2usi	rax, xmm30, {ru-sae}	 # AVX512
	vcvtss2usi	rax, xmm30, {rd-sae}	 # AVX512
	vcvtss2usi	rax, xmm30, {rz-sae}	 # AVX512
	vcvtss2usi	rax, DWORD PTR [rcx]	 # AVX512
	vcvtss2usi	rax, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtss2usi	rax, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcvtss2usi	rax, DWORD PTR [rdx+512]	 # AVX512
	vcvtss2usi	rax, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcvtss2usi	rax, DWORD PTR [rdx-516]	 # AVX512
	vcvtss2usi	r8, xmm30	 # AVX512
	vcvtss2usi	r8, xmm30, {rn-sae}	 # AVX512
	vcvtss2usi	r8, xmm30, {ru-sae}	 # AVX512
	vcvtss2usi	r8, xmm30, {rd-sae}	 # AVX512
	vcvtss2usi	r8, xmm30, {rz-sae}	 # AVX512
	vcvtss2usi	r8, DWORD PTR [rcx]	 # AVX512
	vcvtss2usi	r8, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtss2usi	r8, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcvtss2usi	r8, DWORD PTR [rdx+512]	 # AVX512
	vcvtss2usi	r8, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcvtss2usi	r8, DWORD PTR [rdx-516]	 # AVX512

	vcvtusi2sd	xmm30, xmm29, eax	 # AVX512
	vcvtusi2sd	xmm30, xmm29, ebp	 # AVX512
	vcvtusi2sd	xmm30, xmm29, r13d	 # AVX512
	vcvtusi2sd	xmm30, xmm29, DWORD PTR [rcx]	 # AVX512
	vcvtusi2sd	xmm30, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtusi2sd	xmm30, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcvtusi2sd	xmm30, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcvtusi2sd	xmm30, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcvtusi2sd	xmm30, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcvtusi2sd	xmm30, xmm29, rax	 # AVX512
	vcvtusi2sd	xmm30, xmm29, {rn-sae}, rax	 # AVX512
	vcvtusi2sd	xmm30, xmm29, {ru-sae}, rax	 # AVX512
	vcvtusi2sd	xmm30, xmm29, {rd-sae}, rax	 # AVX512
	vcvtusi2sd	xmm30, xmm29, {rz-sae}, rax	 # AVX512
	vcvtusi2sd	xmm30, xmm29, r8	 # AVX512
	vcvtusi2sd	xmm30, xmm29, {rn-sae}, r8	 # AVX512
	vcvtusi2sd	xmm30, xmm29, {ru-sae}, r8	 # AVX512
	vcvtusi2sd	xmm30, xmm29, {rd-sae}, r8	 # AVX512
	vcvtusi2sd	xmm30, xmm29, {rz-sae}, r8	 # AVX512
	vcvtusi2sd	xmm30, xmm29, QWORD PTR [rcx]	 # AVX512
	vcvtusi2sd	xmm30, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtusi2sd	xmm30, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcvtusi2sd	xmm30, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcvtusi2sd	xmm30, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcvtusi2sd	xmm30, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vcvtusi2ss	xmm30, xmm29, eax	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {rn-sae}, eax	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {ru-sae}, eax	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {rd-sae}, eax	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {rz-sae}, eax	 # AVX512
	vcvtusi2ss	xmm30, xmm29, ebp	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {rn-sae}, ebp	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {ru-sae}, ebp	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {rd-sae}, ebp	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {rz-sae}, ebp	 # AVX512
	vcvtusi2ss	xmm30, xmm29, r13d	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {rn-sae}, r13d	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {ru-sae}, r13d	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {rd-sae}, r13d	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {rz-sae}, r13d	 # AVX512
	vcvtusi2ss	xmm30, xmm29, DWORD PTR [rcx]	 # AVX512
	vcvtusi2ss	xmm30, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtusi2ss	xmm30, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vcvtusi2ss	xmm30, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vcvtusi2ss	xmm30, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vcvtusi2ss	xmm30, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vcvtusi2ss	xmm30, xmm29, rax	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {rn-sae}, rax	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {ru-sae}, rax	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {rd-sae}, rax	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {rz-sae}, rax	 # AVX512
	vcvtusi2ss	xmm30, xmm29, r8	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {rn-sae}, r8	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {ru-sae}, r8	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {rd-sae}, r8	 # AVX512
	vcvtusi2ss	xmm30, xmm29, {rz-sae}, r8	 # AVX512
	vcvtusi2ss	xmm30, xmm29, QWORD PTR [rcx]	 # AVX512
	vcvtusi2ss	xmm30, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vcvtusi2ss	xmm30, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vcvtusi2ss	xmm30, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vcvtusi2ss	xmm30, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vcvtusi2ss	xmm30, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vscalefsd	xmm30{k7}, xmm29, xmm28	 # AVX512
	vscalefsd	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vscalefsd	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vscalefsd	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vscalefsd	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vscalefsd	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vscalefsd	xmm30{k7}, xmm29, QWORD PTR [rcx]	 # AVX512
	vscalefsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vscalefsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016]	 # AVX512 Disp8
	vscalefsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024]	 # AVX512
	vscalefsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024]	 # AVX512 Disp8
	vscalefsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032]	 # AVX512

	vscalefss	xmm30{k7}, xmm29, xmm28	 # AVX512
	vscalefss	xmm30{k7}{z}, xmm29, xmm28	 # AVX512
	vscalefss	xmm30{k7}, xmm29, xmm28, {rn-sae}	 # AVX512
	vscalefss	xmm30{k7}, xmm29, xmm28, {ru-sae}	 # AVX512
	vscalefss	xmm30{k7}, xmm29, xmm28, {rd-sae}	 # AVX512
	vscalefss	xmm30{k7}, xmm29, xmm28, {rz-sae}	 # AVX512
	vscalefss	xmm30{k7}, xmm29, DWORD PTR [rcx]	 # AVX512
	vscalefss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234]	 # AVX512
	vscalefss	xmm30{k7}, xmm29, DWORD PTR [rdx+508]	 # AVX512 Disp8
	vscalefss	xmm30{k7}, xmm29, DWORD PTR [rdx+512]	 # AVX512
	vscalefss	xmm30{k7}, xmm29, DWORD PTR [rdx-512]	 # AVX512 Disp8
	vscalefss	xmm30{k7}, xmm29, DWORD PTR [rdx-516]	 # AVX512

	vfixupimmss	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512
	vfixupimmss	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512
	vfixupimmss	xmm30{k7}, xmm29, xmm28, {sae}, 0xab	 # AVX512
	vfixupimmss	xmm30{k7}, xmm29, xmm28, 123	 # AVX512
	vfixupimmss	xmm30{k7}, xmm29, xmm28, {sae}, 123	 # AVX512
	vfixupimmss	xmm30{k7}, xmm29, DWORD PTR [rcx], 123	 # AVX512
	vfixupimmss	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234], 123	 # AVX512
	vfixupimmss	xmm30{k7}, xmm29, DWORD PTR [rdx+508], 123	 # AVX512 Disp8
	vfixupimmss	xmm30{k7}, xmm29, DWORD PTR [rdx+512], 123	 # AVX512
	vfixupimmss	xmm30{k7}, xmm29, DWORD PTR [rdx-512], 123	 # AVX512 Disp8
	vfixupimmss	xmm30{k7}, xmm29, DWORD PTR [rdx-516], 123	 # AVX512

	vfixupimmsd	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512
	vfixupimmsd	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512
	vfixupimmsd	xmm30{k7}, xmm29, xmm28, {sae}, 0xab	 # AVX512
	vfixupimmsd	xmm30{k7}, xmm29, xmm28, 123	 # AVX512
	vfixupimmsd	xmm30{k7}, xmm29, xmm28, {sae}, 123	 # AVX512
	vfixupimmsd	xmm30{k7}, xmm29, QWORD PTR [rcx], 123	 # AVX512
	vfixupimmsd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234], 123	 # AVX512
	vfixupimmsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016], 123	 # AVX512 Disp8
	vfixupimmsd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024], 123	 # AVX512
	vfixupimmsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024], 123	 # AVX512 Disp8
	vfixupimmsd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032], 123	 # AVX512

	vrndscalesd	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512
	vrndscalesd	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512
	vrndscalesd	xmm30{k7}, xmm29, xmm28, {sae}, 0xab	 # AVX512
	vrndscalesd	xmm30{k7}, xmm29, xmm28, 123	 # AVX512
	vrndscalesd	xmm30{k7}, xmm29, xmm28, {sae}, 123	 # AVX512
	vrndscalesd	xmm30{k7}, xmm29, QWORD PTR [rcx], 123	 # AVX512
	vrndscalesd	xmm30{k7}, xmm29, QWORD PTR [rax+r14*8+0x1234], 123	 # AVX512
	vrndscalesd	xmm30{k7}, xmm29, QWORD PTR [rdx+1016], 123	 # AVX512 Disp8
	vrndscalesd	xmm30{k7}, xmm29, QWORD PTR [rdx+1024], 123	 # AVX512
	vrndscalesd	xmm30{k7}, xmm29, QWORD PTR [rdx-1024], 123	 # AVX512 Disp8
	vrndscalesd	xmm30{k7}, xmm29, QWORD PTR [rdx-1032], 123	 # AVX512

	vrndscaless	xmm30{k7}, xmm29, xmm28, 0xab	 # AVX512
	vrndscaless	xmm30{k7}{z}, xmm29, xmm28, 0xab	 # AVX512
	vrndscaless	xmm30{k7}, xmm29, xmm28, {sae}, 0xab	 # AVX512
	vrndscaless	xmm30{k7}, xmm29, xmm28, 123	 # AVX512
	vrndscaless	xmm30{k7}, xmm29, xmm28, {sae}, 123	 # AVX512
	vrndscaless	xmm30{k7}, xmm29, DWORD PTR [rcx], 123	 # AVX512
	vrndscaless	xmm30{k7}, xmm29, DWORD PTR [rax+r14*8+0x1234], 123	 # AVX512
	vrndscaless	xmm30{k7}, xmm29, DWORD PTR [rdx+508], 123	 # AVX512 Disp8
	vrndscaless	xmm30{k7}, xmm29, DWORD PTR [rdx+512], 123	 # AVX512
	vrndscaless	xmm30{k7}, xmm29, DWORD PTR [rdx-512], 123	 # AVX512 Disp8
	vrndscaless	xmm30{k7}, xmm29, DWORD PTR [rdx-516], 123	 # AVX512

