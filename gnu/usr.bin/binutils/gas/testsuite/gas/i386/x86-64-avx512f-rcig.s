# Check 64bit AVX512F-RCIG instructions

	.allow_index_reg
	.text
_start:
	vcmpeqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_oqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_ospd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_uqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_uspd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalsepd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalse_oqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalse_ospd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpgepd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpge_oqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpge_ospd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpgtpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpgt_oqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpgt_ospd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmplepd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmple_oqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmple_ospd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpltpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmplt_oqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmplt_ospd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_oqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_ospd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_uqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_uspd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpngepd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnge_uqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnge_uspd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpngtpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpngt_uqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpngt_uspd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnlepd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnle_uqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnle_uspd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnltpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnlt_uqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnlt_uspd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpordpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpord_qpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpord_spd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmptruepd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmptrue_uqpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmptrue_uspd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpunordpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpunord_qpd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpunord_spd	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmppd	$0xab, {sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmppd	$123, {sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_oqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_osps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_uqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeq_usps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalseps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalse_oqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpfalse_osps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpgeps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpge_oqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpge_osps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpgtps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpgt_oqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpgt_osps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpleps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmple_oqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmple_osps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpltps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmplt_oqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmplt_osps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_oqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_osps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_uqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpneq_usps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpngeps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnge_uqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnge_usps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpngtps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpngt_uqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpngt_usps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnleps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnle_uqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnle_usps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnltps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnlt_uqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpnlt_usps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpordps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpord_qps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpord_sps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmptrueps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmptrue_uqps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmptrue_usps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpunordps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpunord_qps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpunord_sps	{sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpps	$0xab, {sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpps	$123, {sae}, %zmm29, %zmm30, %k5	 # AVX512F
	vcmpeqsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpeq_oqsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpeq_ossd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpeq_uqsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpeq_ussd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpfalsesd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpfalse_oqsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpfalse_ossd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpgesd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpge_oqsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpge_ossd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpgtsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpgt_oqsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpgt_ossd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmplesd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmple_oqsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmple_ossd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpltsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmplt_oqsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmplt_ossd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpneqsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpneq_oqsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpneq_ossd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpneq_uqsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpneq_ussd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpngesd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpnge_uqsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpnge_ussd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpngtsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpngt_uqsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpngt_ussd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpnlesd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpnle_uqsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpnle_ussd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpnltsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpnlt_uqsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpnlt_ussd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpordsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpord_qsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpord_ssd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmptruesd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmptrue_uqsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmptrue_ussd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpunordsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpunord_qsd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpunord_ssd	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpsd	$0xab, {sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpsd	$123, {sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpeqss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpeq_oqss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpeq_osss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpeq_uqss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpeq_usss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpfalsess	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpfalse_oqss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpfalse_osss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpgess	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpge_oqss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpge_osss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpgtss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpgt_oqss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpgt_osss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpless	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmple_oqss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmple_osss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpltss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmplt_oqss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmplt_osss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpneqss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpneq_oqss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpneq_osss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpneq_uqss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpneq_usss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpngess	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpnge_uqss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpnge_usss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpngtss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpngt_uqss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpngt_usss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpnless	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpnle_uqss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpnle_usss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpnltss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpnlt_uqss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpnlt_usss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpordss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpord_qss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpord_sss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmptruess	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmptrue_uqss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmptrue_usss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpunordss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpunord_qss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpunord_sss	{sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpss	$0xab, {sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcmpss	$123, {sae}, %xmm28, %xmm29, %k5	 # AVX512F
	vcomisd	{sae}, %xmm29, %xmm30	 # AVX512F
	vcomiss	{sae}, %xmm29, %xmm30	 # AVX512F
	vcvtph2ps	{sae}, %ymm29, %zmm30	 # AVX512F
	vcvtps2pd	{sae}, %ymm29, %zmm30	 # AVX512F
	vcvtps2ph	$0xab, {sae}, %zmm29, %ymm30	 # AVX512F
	vcvtps2ph	$123, {sae}, %zmm29, %ymm30	 # AVX512F
	vcvtss2sd	{sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vcvttpd2dq	{sae}, %zmm29, %ymm30	 # AVX512F
	vcvttps2dq	{sae}, %zmm29, %zmm30	 # AVX512F
	vcvttsd2si	{sae}, %xmm30, %eax	 # AVX512F
	vcvttsd2si	{sae}, %xmm30, %ebp	 # AVX512F
	vcvttsd2si	{sae}, %xmm30, %r13d	 # AVX512F
	vcvttsd2si	{sae}, %xmm30, %rax	 # AVX512F
	vcvttsd2si	{sae}, %xmm30, %r8	 # AVX512F
	vcvttss2si	{sae}, %xmm30, %eax	 # AVX512F
	vcvttss2si	{sae}, %xmm30, %ebp	 # AVX512F
	vcvttss2si	{sae}, %xmm30, %r13d	 # AVX512F
	vcvttss2si	{sae}, %xmm30, %rax	 # AVX512F
	vcvttss2si	{sae}, %xmm30, %r8	 # AVX512F
	vgetexppd	{sae}, %zmm29, %zmm30	 # AVX512F
	vgetexpps	{sae}, %zmm29, %zmm30	 # AVX512F
	vgetexpsd	{sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vgetexpss	{sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vgetmantpd	$0xab, {sae}, %zmm29, %zmm30	 # AVX512F
	vgetmantpd	$123, {sae}, %zmm29, %zmm30	 # AVX512F
	vgetmantps	$0xab, {sae}, %zmm29, %zmm30	 # AVX512F
	vgetmantps	$123, {sae}, %zmm29, %zmm30	 # AVX512F
	vgetmantsd	$0xab, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vgetmantsd	$123, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vgetmantss	$0xab, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vgetmantss	$123, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vmaxpd	{sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vmaxps	{sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vmaxsd	{sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vmaxss	{sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vminpd	{sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vminps	{sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vminsd	{sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vminss	{sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vucomisd	{sae}, %xmm29, %xmm30	 # AVX512F
	vucomiss	{sae}, %xmm29, %xmm30	 # AVX512F
	vfixupimmpd	$0xab, {sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfixupimmpd	$123, {sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfixupimmps	$0xab, {sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfixupimmps	$123, {sae}, %zmm28, %zmm29, %zmm30	 # AVX512F
	vfixupimmsd	$0xab, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vfixupimmsd	$123, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vfixupimmss	$0xab, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vfixupimmss	$123, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vrndscalepd	$0xab, {sae}, %zmm29, %zmm30	 # AVX512F
	vrndscalepd	$123, {sae}, %zmm29, %zmm30	 # AVX512F
	vrndscaleps	$0xab, {sae}, %zmm29, %zmm30	 # AVX512F
	vrndscaleps	$123, {sae}, %zmm29, %zmm30	 # AVX512F
	vrndscalesd	$0xab, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vrndscalesd	$123, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vrndscaless	$0xab, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vrndscaless	$123, {sae}, %xmm28, %xmm29, %xmm30	 # AVX512F
	vcvttpd2udq	{sae}, %zmm29, %ymm30	 # AVX512F
	vcvttps2udq	{sae}, %zmm29, %zmm30	 # AVX512F
	vcvttsd2usi	{sae}, %xmm30, %eax	 # AVX512F
	vcvttsd2usi	{sae}, %xmm30, %ebp	 # AVX512F
	vcvttsd2usi	{sae}, %xmm30, %r13d	 # AVX512F
	vcvttsd2usi	{sae}, %xmm30, %rax	 # AVX512F
	vcvttsd2usi	{sae}, %xmm30, %r8	 # AVX512F
	vcvttss2usi	{sae}, %xmm30, %eax	 # AVX512F
	vcvttss2usi	{sae}, %xmm30, %ebp	 # AVX512F
	vcvttss2usi	{sae}, %xmm30, %r13d	 # AVX512F
	vcvttss2usi	{sae}, %xmm30, %rax	 # AVX512F
	vcvttss2usi	{sae}, %xmm30, %r8	 # AVX512F

	.intel_syntax noprefix
	vcmpeqpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpeq_oqpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpeq_ospd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpeq_uqpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpeq_uspd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpfalsepd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpfalse_oqpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpfalse_ospd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpgepd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpge_oqpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpge_ospd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpgtpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpgt_oqpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpgt_ospd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmplepd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmple_oqpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmple_ospd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpltpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmplt_oqpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmplt_ospd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpneqpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpneq_oqpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpneq_ospd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpneq_uqpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpneq_uspd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpngepd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpnge_uqpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpnge_uspd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpngtpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpngt_uqpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpngt_uspd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpnlepd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpnle_uqpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpnle_uspd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpnltpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpnlt_uqpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpnlt_uspd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpordpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpord_qpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpord_spd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmptruepd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmptrue_uqpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmptrue_uspd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpunordpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpunord_qpd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpunord_spd	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmppd	k5, zmm30, zmm29, {sae}, 0xab	 # AVX512F
	vcmppd	k5, zmm30, zmm29, {sae}, 123	 # AVX512F
	vcmpeqps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpeq_oqps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpeq_osps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpeq_uqps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpeq_usps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpfalseps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpfalse_oqps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpfalse_osps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpgeps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpge_oqps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpge_osps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpgtps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpgt_oqps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpgt_osps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpleps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmple_oqps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmple_osps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpltps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmplt_oqps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmplt_osps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpneqps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpneq_oqps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpneq_osps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpneq_uqps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpneq_usps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpngeps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpnge_uqps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpnge_usps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpngtps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpngt_uqps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpngt_usps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpnleps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpnle_uqps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpnle_usps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpnltps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpnlt_uqps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpnlt_usps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpordps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpord_qps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpord_sps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmptrueps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmptrue_uqps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmptrue_usps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpunordps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpunord_qps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpunord_sps	k5, zmm30, zmm29, {sae}	 # AVX512F
	vcmpps	k5, zmm30, zmm29, {sae}, 0xab	 # AVX512F
	vcmpps	k5, zmm30, zmm29, {sae}, 123	 # AVX512F
	vcmpeqsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpeq_oqsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpeq_ossd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpeq_uqsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpeq_ussd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpfalsesd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpfalse_oqsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpfalse_ossd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpgesd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpge_oqsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpge_ossd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpgtsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpgt_oqsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpgt_ossd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmplesd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmple_oqsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmple_ossd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpltsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmplt_oqsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmplt_ossd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpneqsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpneq_oqsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpneq_ossd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpneq_uqsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpneq_ussd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpngesd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpnge_uqsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpnge_ussd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpngtsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpngt_uqsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpngt_ussd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpnlesd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpnle_uqsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpnle_ussd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpnltsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpnlt_uqsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpnlt_ussd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpordsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpord_qsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpord_ssd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmptruesd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmptrue_uqsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmptrue_ussd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpunordsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpunord_qsd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpunord_ssd	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpsd	k5, xmm29, xmm28, {sae}, 0xab	 # AVX512F
	vcmpsd	k5, xmm29, xmm28, {sae}, 123	 # AVX512F
	vcmpeqss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpeq_oqss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpeq_osss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpeq_uqss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpeq_usss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpfalsess	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpfalse_oqss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpfalse_osss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpgess	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpge_oqss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpge_osss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpgtss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpgt_oqss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpgt_osss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpless	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmple_oqss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmple_osss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpltss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmplt_oqss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmplt_osss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpneqss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpneq_oqss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpneq_osss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpneq_uqss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpneq_usss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpngess	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpnge_uqss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpnge_usss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpngtss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpngt_uqss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpngt_usss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpnless	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpnle_uqss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpnle_usss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpnltss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpnlt_uqss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpnlt_usss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpordss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpord_qss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpord_sss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmptruess	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmptrue_uqss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmptrue_usss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpunordss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpunord_qss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpunord_sss	k5, xmm29, xmm28, {sae}	 # AVX512F
	vcmpss	k5, xmm29, xmm28, {sae}, 0xab	 # AVX512F
	vcmpss	k5, xmm29, xmm28, {sae}, 123	 # AVX512F
	vcomisd	xmm30, xmm29, {sae}	 # AVX512F
	vcomiss	xmm30, xmm29, {sae}	 # AVX512F
	vcvtph2ps	zmm30, ymm29, {sae}	 # AVX512F
	vcvtps2pd	zmm30, ymm29, {sae}	 # AVX512F
	vcvtps2ph	ymm30, zmm29, {sae}, 0xab	 # AVX512F
	vcvtps2ph	ymm30, zmm29, {sae}, 123	 # AVX512F
	vcvtss2sd	xmm30, xmm29, xmm28, {sae}	 # AVX512F
	vcvttpd2dq	ymm30, zmm29, {sae}	 # AVX512F
	vcvttps2dq	zmm30, zmm29, {sae}	 # AVX512F
	vcvttsd2si	eax, xmm30, {sae}	 # AVX512F
	vcvttsd2si	ebp, xmm30, {sae}	 # AVX512F
	vcvttsd2si	r13d, xmm30, {sae}	 # AVX512F
	vcvttsd2si	rax, xmm30, {sae}	 # AVX512F
	vcvttsd2si	r8, xmm30, {sae}	 # AVX512F
	vcvttss2si	eax, xmm30, {sae}	 # AVX512F
	vcvttss2si	ebp, xmm30, {sae}	 # AVX512F
	vcvttss2si	r13d, xmm30, {sae}	 # AVX512F
	vcvttss2si	rax, xmm30, {sae}	 # AVX512F
	vcvttss2si	r8, xmm30, {sae}	 # AVX512F
	vgetexppd	zmm30, zmm29, {sae}	 # AVX512F
	vgetexpps	zmm30, zmm29, {sae}	 # AVX512F
	vgetexpsd	xmm30, xmm29, xmm28, {sae}	 # AVX512F
	vgetexpss	xmm30, xmm29, xmm28, {sae}	 # AVX512F
	vgetmantpd	zmm30, zmm29, {sae}, 0xab	 # AVX512F
	vgetmantpd	zmm30, zmm29, {sae}, 123	 # AVX512F
	vgetmantps	zmm30, zmm29, {sae}, 0xab	 # AVX512F
	vgetmantps	zmm30, zmm29, {sae}, 123	 # AVX512F
	vgetmantsd	xmm30, xmm29, xmm28, {sae}, 0xab	 # AVX512F
	vgetmantsd	xmm30, xmm29, xmm28, {sae}, 123	 # AVX512F
	vgetmantss	xmm30, xmm29, xmm28, {sae}, 0xab	 # AVX512F
	vgetmantss	xmm30, xmm29, xmm28, {sae}, 123	 # AVX512F
	vmaxpd	zmm30, zmm29, zmm28, {sae}	 # AVX512F
	vmaxps	zmm30, zmm29, zmm28, {sae}	 # AVX512F
	vmaxsd	xmm30, xmm29, xmm28, {sae}	 # AVX512F
	vmaxss	xmm30, xmm29, xmm28, {sae}	 # AVX512F
	vminpd	zmm30, zmm29, zmm28, {sae}	 # AVX512F
	vminps	zmm30, zmm29, zmm28, {sae}	 # AVX512F
	vminsd	xmm30, xmm29, xmm28, {sae}	 # AVX512F
	vminss	xmm30, xmm29, xmm28, {sae}	 # AVX512F
	vucomisd	xmm30, xmm29, {sae}	 # AVX512F
	vucomiss	xmm30, xmm29, {sae}	 # AVX512F
	vfixupimmpd	zmm30, zmm29, zmm28, {sae}, 0xab	 # AVX512F
	vfixupimmpd	zmm30, zmm29, zmm28, {sae}, 123	 # AVX512F
	vfixupimmps	zmm30, zmm29, zmm28, {sae}, 0xab	 # AVX512F
	vfixupimmps	zmm30, zmm29, zmm28, {sae}, 123	 # AVX512F
	vfixupimmsd	xmm30, xmm29, xmm28, {sae}, 0xab	 # AVX512F
	vfixupimmsd	xmm30, xmm29, xmm28, {sae}, 123	 # AVX512F
	vfixupimmss	xmm30, xmm29, xmm28, {sae}, 0xab	 # AVX512F
	vfixupimmss	xmm30, xmm29, xmm28, {sae}, 123	 # AVX512F
	vrndscalepd	zmm30, zmm29, {sae}, 0xab	 # AVX512F
	vrndscalepd	zmm30, zmm29, {sae}, 123	 # AVX512F
	vrndscaleps	zmm30, zmm29, {sae}, 0xab	 # AVX512F
	vrndscaleps	zmm30, zmm29, {sae}, 123	 # AVX512F
	vrndscalesd	xmm30, xmm29, xmm28, {sae}, 0xab	 # AVX512F
	vrndscalesd	xmm30, xmm29, xmm28, {sae}, 123	 # AVX512F
	vrndscaless	xmm30, xmm29, xmm28, {sae}, 0xab	 # AVX512F
	vrndscaless	xmm30, xmm29, xmm28, {sae}, 123	 # AVX512F
	vcvttpd2udq	ymm30, zmm29, {sae}	 # AVX512F
	vcvttps2udq	zmm30, zmm29, {sae}	 # AVX512F
	vcvttsd2usi	eax, xmm30, {sae}	 # AVX512F
	vcvttsd2usi	ebp, xmm30, {sae}	 # AVX512F
	vcvttsd2usi	r13d, xmm30, {sae}	 # AVX512F
	vcvttsd2usi	rax, xmm30, {sae}	 # AVX512F
	vcvttsd2usi	r8, xmm30, {sae}	 # AVX512F
	vcvttss2usi	eax, xmm30, {sae}	 # AVX512F
	vcvttss2usi	ebp, xmm30, {sae}	 # AVX512F
	vcvttss2usi	r13d, xmm30, {sae}	 # AVX512F
	vcvttss2usi	rax, xmm30, {sae}	 # AVX512F
	vcvttss2usi	r8, xmm30, {sae}	 # AVX512F
