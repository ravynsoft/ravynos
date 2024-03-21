# Check 32bit VCM.*{PH,SH} instructions

        .allow_index_reg
        .text
_start:
	vcmpeq_oqph	%zmm5, %zmm6, %k5
	vcmpeq_oqph	%zmm5, %zmm6, %k5{%k7}
	vcmpeq_oqph	{sae}, %zmm5, %zmm6, %k5
	vcmpeq_oqph	(%ecx), %zmm6, %k5
	vcmpeq_oqph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpeq_oqph	(%eax){1to32}, %zmm6, %k5
	vcmpeq_oqph	8128(%edx), %zmm6, %k5
	vcmpeq_oqph	8192(%edx), %zmm6, %k5
	vcmpeq_oqph	-8192(%edx), %zmm6, %k5
	vcmpeq_oqph	-8256(%edx), %zmm6, %k5
	vcmpeq_oqph	1016(%edx){1to32}, %zmm6, %k5
	vcmpeq_oqph	1024(%edx){1to32}, %zmm6, %k5
	vcmpeq_oqph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpeq_oqph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpeqph	%zmm5, %zmm6, %k5
	vcmpeqph	%zmm5, %zmm6, %k5{%k7}
	vcmpeqph	{sae}, %zmm5, %zmm6, %k5
	vcmpeqph	(%ecx), %zmm6, %k5
	vcmpeqph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpeqph	(%eax){1to32}, %zmm6, %k5
	vcmpeqph	8128(%edx), %zmm6, %k5
	vcmpeqph	8192(%edx), %zmm6, %k5
	vcmpeqph	-8192(%edx), %zmm6, %k5
	vcmpeqph	-8256(%edx), %zmm6, %k5
	vcmpeqph	1016(%edx){1to32}, %zmm6, %k5
	vcmpeqph	1024(%edx){1to32}, %zmm6, %k5
	vcmpeqph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpeqph	-1032(%edx){1to32}, %zmm6, %k5
	vcmplt_osph	%zmm5, %zmm6, %k5
	vcmplt_osph	%zmm5, %zmm6, %k5{%k7}
	vcmplt_osph	{sae}, %zmm5, %zmm6, %k5
	vcmplt_osph	(%ecx), %zmm6, %k5
	vcmplt_osph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmplt_osph	(%eax){1to32}, %zmm6, %k5
	vcmplt_osph	8128(%edx), %zmm6, %k5
	vcmplt_osph	8192(%edx), %zmm6, %k5
	vcmplt_osph	-8192(%edx), %zmm6, %k5
	vcmplt_osph	-8256(%edx), %zmm6, %k5
	vcmplt_osph	1016(%edx){1to32}, %zmm6, %k5
	vcmplt_osph	1024(%edx){1to32}, %zmm6, %k5
	vcmplt_osph	-1024(%edx){1to32}, %zmm6, %k5
	vcmplt_osph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpltph	%zmm5, %zmm6, %k5
	vcmpltph	%zmm5, %zmm6, %k5{%k7}
	vcmpltph	{sae}, %zmm5, %zmm6, %k5
	vcmpltph	(%ecx), %zmm6, %k5
	vcmpltph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpltph	(%eax){1to32}, %zmm6, %k5
	vcmpltph	8128(%edx), %zmm6, %k5
	vcmpltph	8192(%edx), %zmm6, %k5
	vcmpltph	-8192(%edx), %zmm6, %k5
	vcmpltph	-8256(%edx), %zmm6, %k5
	vcmpltph	1016(%edx){1to32}, %zmm6, %k5
	vcmpltph	1024(%edx){1to32}, %zmm6, %k5
	vcmpltph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpltph	-1032(%edx){1to32}, %zmm6, %k5
	vcmple_osph	%zmm5, %zmm6, %k5
	vcmple_osph	%zmm5, %zmm6, %k5{%k7}
	vcmple_osph	{sae}, %zmm5, %zmm6, %k5
	vcmple_osph	(%ecx), %zmm6, %k5
	vcmple_osph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmple_osph	(%eax){1to32}, %zmm6, %k5
	vcmple_osph	8128(%edx), %zmm6, %k5
	vcmple_osph	8192(%edx), %zmm6, %k5
	vcmple_osph	-8192(%edx), %zmm6, %k5
	vcmple_osph	-8256(%edx), %zmm6, %k5
	vcmple_osph	1016(%edx){1to32}, %zmm6, %k5
	vcmple_osph	1024(%edx){1to32}, %zmm6, %k5
	vcmple_osph	-1024(%edx){1to32}, %zmm6, %k5
	vcmple_osph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpleph	%zmm5, %zmm6, %k5
	vcmpleph	%zmm5, %zmm6, %k5{%k7}
	vcmpleph	{sae}, %zmm5, %zmm6, %k5
	vcmpleph	(%ecx), %zmm6, %k5
	vcmpleph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpleph	(%eax){1to32}, %zmm6, %k5
	vcmpleph	8128(%edx), %zmm6, %k5
	vcmpleph	8192(%edx), %zmm6, %k5
	vcmpleph	-8192(%edx), %zmm6, %k5
	vcmpleph	-8256(%edx), %zmm6, %k5
	vcmpleph	1016(%edx){1to32}, %zmm6, %k5
	vcmpleph	1024(%edx){1to32}, %zmm6, %k5
	vcmpleph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpleph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpunord_qph	%zmm5, %zmm6, %k5
	vcmpunord_qph	%zmm5, %zmm6, %k5{%k7}
	vcmpunord_qph	{sae}, %zmm5, %zmm6, %k5
	vcmpunord_qph	(%ecx), %zmm6, %k5
	vcmpunord_qph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpunord_qph	(%eax){1to32}, %zmm6, %k5
	vcmpunord_qph	8128(%edx), %zmm6, %k5
	vcmpunord_qph	8192(%edx), %zmm6, %k5
	vcmpunord_qph	-8192(%edx), %zmm6, %k5
	vcmpunord_qph	-8256(%edx), %zmm6, %k5
	vcmpunord_qph	1016(%edx){1to32}, %zmm6, %k5
	vcmpunord_qph	1024(%edx){1to32}, %zmm6, %k5
	vcmpunord_qph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpunord_qph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpunordph	%zmm5, %zmm6, %k5
	vcmpunordph	%zmm5, %zmm6, %k5{%k7}
	vcmpunordph	{sae}, %zmm5, %zmm6, %k5
	vcmpunordph	(%ecx), %zmm6, %k5
	vcmpunordph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpunordph	(%eax){1to32}, %zmm6, %k5
	vcmpunordph	8128(%edx), %zmm6, %k5
	vcmpunordph	8192(%edx), %zmm6, %k5
	vcmpunordph	-8192(%edx), %zmm6, %k5
	vcmpunordph	-8256(%edx), %zmm6, %k5
	vcmpunordph	1016(%edx){1to32}, %zmm6, %k5
	vcmpunordph	1024(%edx){1to32}, %zmm6, %k5
	vcmpunordph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpunordph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpneq_uqph	%zmm5, %zmm6, %k5
	vcmpneq_uqph	%zmm5, %zmm6, %k5{%k7}
	vcmpneq_uqph	{sae}, %zmm5, %zmm6, %k5
	vcmpneq_uqph	(%ecx), %zmm6, %k5
	vcmpneq_uqph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpneq_uqph	(%eax){1to32}, %zmm6, %k5
	vcmpneq_uqph	8128(%edx), %zmm6, %k5
	vcmpneq_uqph	8192(%edx), %zmm6, %k5
	vcmpneq_uqph	-8192(%edx), %zmm6, %k5
	vcmpneq_uqph	-8256(%edx), %zmm6, %k5
	vcmpneq_uqph	1016(%edx){1to32}, %zmm6, %k5
	vcmpneq_uqph	1024(%edx){1to32}, %zmm6, %k5
	vcmpneq_uqph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpneq_uqph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpneqph	%zmm5, %zmm6, %k5
	vcmpneqph	%zmm5, %zmm6, %k5{%k7}
	vcmpneqph	{sae}, %zmm5, %zmm6, %k5
	vcmpneqph	(%ecx), %zmm6, %k5
	vcmpneqph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpneqph	(%eax){1to32}, %zmm6, %k5
	vcmpneqph	8128(%edx), %zmm6, %k5
	vcmpneqph	8192(%edx), %zmm6, %k5
	vcmpneqph	-8192(%edx), %zmm6, %k5
	vcmpneqph	-8256(%edx), %zmm6, %k5
	vcmpneqph	1016(%edx){1to32}, %zmm6, %k5
	vcmpneqph	1024(%edx){1to32}, %zmm6, %k5
	vcmpneqph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpneqph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpnlt_usph	%zmm5, %zmm6, %k5
	vcmpnlt_usph	%zmm5, %zmm6, %k5{%k7}
	vcmpnlt_usph	{sae}, %zmm5, %zmm6, %k5
	vcmpnlt_usph	(%ecx), %zmm6, %k5
	vcmpnlt_usph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpnlt_usph	(%eax){1to32}, %zmm6, %k5
	vcmpnlt_usph	8128(%edx), %zmm6, %k5
	vcmpnlt_usph	8192(%edx), %zmm6, %k5
	vcmpnlt_usph	-8192(%edx), %zmm6, %k5
	vcmpnlt_usph	-8256(%edx), %zmm6, %k5
	vcmpnlt_usph	1016(%edx){1to32}, %zmm6, %k5
	vcmpnlt_usph	1024(%edx){1to32}, %zmm6, %k5
	vcmpnlt_usph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpnlt_usph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpnltph	%zmm5, %zmm6, %k5
	vcmpnltph	%zmm5, %zmm6, %k5{%k7}
	vcmpnltph	{sae}, %zmm5, %zmm6, %k5
	vcmpnltph	(%ecx), %zmm6, %k5
	vcmpnltph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpnltph	(%eax){1to32}, %zmm6, %k5
	vcmpnltph	8128(%edx), %zmm6, %k5
	vcmpnltph	8192(%edx), %zmm6, %k5
	vcmpnltph	-8192(%edx), %zmm6, %k5
	vcmpnltph	-8256(%edx), %zmm6, %k5
	vcmpnltph	1016(%edx){1to32}, %zmm6, %k5
	vcmpnltph	1024(%edx){1to32}, %zmm6, %k5
	vcmpnltph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpnltph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpnle_usph	%zmm5, %zmm6, %k5
	vcmpnle_usph	%zmm5, %zmm6, %k5{%k7}
	vcmpnle_usph	{sae}, %zmm5, %zmm6, %k5
	vcmpnle_usph	(%ecx), %zmm6, %k5
	vcmpnle_usph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpnle_usph	(%eax){1to32}, %zmm6, %k5
	vcmpnle_usph	8128(%edx), %zmm6, %k5
	vcmpnle_usph	8192(%edx), %zmm6, %k5
	vcmpnle_usph	-8192(%edx), %zmm6, %k5
	vcmpnle_usph	-8256(%edx), %zmm6, %k5
	vcmpnle_usph	1016(%edx){1to32}, %zmm6, %k5
	vcmpnle_usph	1024(%edx){1to32}, %zmm6, %k5
	vcmpnle_usph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpnle_usph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpnleph	%zmm5, %zmm6, %k5
	vcmpnleph	%zmm5, %zmm6, %k5{%k7}
	vcmpnleph	{sae}, %zmm5, %zmm6, %k5
	vcmpnleph	(%ecx), %zmm6, %k5
	vcmpnleph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpnleph	(%eax){1to32}, %zmm6, %k5
	vcmpnleph	8128(%edx), %zmm6, %k5
	vcmpnleph	8192(%edx), %zmm6, %k5
	vcmpnleph	-8192(%edx), %zmm6, %k5
	vcmpnleph	-8256(%edx), %zmm6, %k5
	vcmpnleph	1016(%edx){1to32}, %zmm6, %k5
	vcmpnleph	1024(%edx){1to32}, %zmm6, %k5
	vcmpnleph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpnleph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpord_qph	%zmm5, %zmm6, %k5
	vcmpord_qph	%zmm5, %zmm6, %k5{%k7}
	vcmpord_qph	{sae}, %zmm5, %zmm6, %k5
	vcmpord_qph	(%ecx), %zmm6, %k5
	vcmpord_qph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpord_qph	(%eax){1to32}, %zmm6, %k5
	vcmpord_qph	8128(%edx), %zmm6, %k5
	vcmpord_qph	8192(%edx), %zmm6, %k5
	vcmpord_qph	-8192(%edx), %zmm6, %k5
	vcmpord_qph	-8256(%edx), %zmm6, %k5
	vcmpord_qph	1016(%edx){1to32}, %zmm6, %k5
	vcmpord_qph	1024(%edx){1to32}, %zmm6, %k5
	vcmpord_qph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpord_qph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpordph	%zmm5, %zmm6, %k5
	vcmpordph	%zmm5, %zmm6, %k5{%k7}
	vcmpordph	{sae}, %zmm5, %zmm6, %k5
	vcmpordph	(%ecx), %zmm6, %k5
	vcmpordph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpordph	(%eax){1to32}, %zmm6, %k5
	vcmpordph	8128(%edx), %zmm6, %k5
	vcmpordph	8192(%edx), %zmm6, %k5
	vcmpordph	-8192(%edx), %zmm6, %k5
	vcmpordph	-8256(%edx), %zmm6, %k5
	vcmpordph	1016(%edx){1to32}, %zmm6, %k5
	vcmpordph	1024(%edx){1to32}, %zmm6, %k5
	vcmpordph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpordph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpeq_uqph	%zmm5, %zmm6, %k5
	vcmpeq_uqph	%zmm5, %zmm6, %k5{%k7}
	vcmpeq_uqph	{sae}, %zmm5, %zmm6, %k5
	vcmpeq_uqph	(%ecx), %zmm6, %k5
	vcmpeq_uqph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpeq_uqph	(%eax){1to32}, %zmm6, %k5
	vcmpeq_uqph	8128(%edx), %zmm6, %k5
	vcmpeq_uqph	8192(%edx), %zmm6, %k5
	vcmpeq_uqph	-8192(%edx), %zmm6, %k5
	vcmpeq_uqph	-8256(%edx), %zmm6, %k5
	vcmpeq_uqph	1016(%edx){1to32}, %zmm6, %k5
	vcmpeq_uqph	1024(%edx){1to32}, %zmm6, %k5
	vcmpeq_uqph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpeq_uqph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpnge_usph	%zmm5, %zmm6, %k5
	vcmpnge_usph	%zmm5, %zmm6, %k5{%k7}
	vcmpnge_usph	{sae}, %zmm5, %zmm6, %k5
	vcmpnge_usph	(%ecx), %zmm6, %k5
	vcmpnge_usph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpnge_usph	(%eax){1to32}, %zmm6, %k5
	vcmpnge_usph	8128(%edx), %zmm6, %k5
	vcmpnge_usph	8192(%edx), %zmm6, %k5
	vcmpnge_usph	-8192(%edx), %zmm6, %k5
	vcmpnge_usph	-8256(%edx), %zmm6, %k5
	vcmpnge_usph	1016(%edx){1to32}, %zmm6, %k5
	vcmpnge_usph	1024(%edx){1to32}, %zmm6, %k5
	vcmpnge_usph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpnge_usph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpngeph	%zmm5, %zmm6, %k5
	vcmpngeph	%zmm5, %zmm6, %k5{%k7}
	vcmpngeph	{sae}, %zmm5, %zmm6, %k5
	vcmpngeph	(%ecx), %zmm6, %k5
	vcmpngeph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpngeph	(%eax){1to32}, %zmm6, %k5
	vcmpngeph	8128(%edx), %zmm6, %k5
	vcmpngeph	8192(%edx), %zmm6, %k5
	vcmpngeph	-8192(%edx), %zmm6, %k5
	vcmpngeph	-8256(%edx), %zmm6, %k5
	vcmpngeph	1016(%edx){1to32}, %zmm6, %k5
	vcmpngeph	1024(%edx){1to32}, %zmm6, %k5
	vcmpngeph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpngeph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpngt_usph	%zmm5, %zmm6, %k5
	vcmpngt_usph	%zmm5, %zmm6, %k5{%k7}
	vcmpngt_usph	{sae}, %zmm5, %zmm6, %k5
	vcmpngt_usph	(%ecx), %zmm6, %k5
	vcmpngt_usph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpngt_usph	(%eax){1to32}, %zmm6, %k5
	vcmpngt_usph	8128(%edx), %zmm6, %k5
	vcmpngt_usph	8192(%edx), %zmm6, %k5
	vcmpngt_usph	-8192(%edx), %zmm6, %k5
	vcmpngt_usph	-8256(%edx), %zmm6, %k5
	vcmpngt_usph	1016(%edx){1to32}, %zmm6, %k5
	vcmpngt_usph	1024(%edx){1to32}, %zmm6, %k5
	vcmpngt_usph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpngt_usph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpngtph	%zmm5, %zmm6, %k5
	vcmpngtph	%zmm5, %zmm6, %k5{%k7}
	vcmpngtph	{sae}, %zmm5, %zmm6, %k5
	vcmpngtph	(%ecx), %zmm6, %k5
	vcmpngtph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpngtph	(%eax){1to32}, %zmm6, %k5
	vcmpngtph	8128(%edx), %zmm6, %k5
	vcmpngtph	8192(%edx), %zmm6, %k5
	vcmpngtph	-8192(%edx), %zmm6, %k5
	vcmpngtph	-8256(%edx), %zmm6, %k5
	vcmpngtph	1016(%edx){1to32}, %zmm6, %k5
	vcmpngtph	1024(%edx){1to32}, %zmm6, %k5
	vcmpngtph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpngtph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpfalse_oqph	%zmm5, %zmm6, %k5
	vcmpfalse_oqph	%zmm5, %zmm6, %k5{%k7}
	vcmpfalse_oqph	{sae}, %zmm5, %zmm6, %k5
	vcmpfalse_oqph	(%ecx), %zmm6, %k5
	vcmpfalse_oqph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpfalse_oqph	(%eax){1to32}, %zmm6, %k5
	vcmpfalse_oqph	8128(%edx), %zmm6, %k5
	vcmpfalse_oqph	8192(%edx), %zmm6, %k5
	vcmpfalse_oqph	-8192(%edx), %zmm6, %k5
	vcmpfalse_oqph	-8256(%edx), %zmm6, %k5
	vcmpfalse_oqph	1016(%edx){1to32}, %zmm6, %k5
	vcmpfalse_oqph	1024(%edx){1to32}, %zmm6, %k5
	vcmpfalse_oqph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpfalse_oqph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpfalseph	%zmm5, %zmm6, %k5
	vcmpfalseph	%zmm5, %zmm6, %k5{%k7}
	vcmpfalseph	{sae}, %zmm5, %zmm6, %k5
	vcmpfalseph	(%ecx), %zmm6, %k5
	vcmpfalseph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpfalseph	(%eax){1to32}, %zmm6, %k5
	vcmpfalseph	8128(%edx), %zmm6, %k5
	vcmpfalseph	8192(%edx), %zmm6, %k5
	vcmpfalseph	-8192(%edx), %zmm6, %k5
	vcmpfalseph	-8256(%edx), %zmm6, %k5
	vcmpfalseph	1016(%edx){1to32}, %zmm6, %k5
	vcmpfalseph	1024(%edx){1to32}, %zmm6, %k5
	vcmpfalseph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpfalseph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpneq_oqph	%zmm5, %zmm6, %k5
	vcmpneq_oqph	%zmm5, %zmm6, %k5{%k7}
	vcmpneq_oqph	{sae}, %zmm5, %zmm6, %k5
	vcmpneq_oqph	(%ecx), %zmm6, %k5
	vcmpneq_oqph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpneq_oqph	(%eax){1to32}, %zmm6, %k5
	vcmpneq_oqph	8128(%edx), %zmm6, %k5
	vcmpneq_oqph	8192(%edx), %zmm6, %k5
	vcmpneq_oqph	-8192(%edx), %zmm6, %k5
	vcmpneq_oqph	-8256(%edx), %zmm6, %k5
	vcmpneq_oqph	1016(%edx){1to32}, %zmm6, %k5
	vcmpneq_oqph	1024(%edx){1to32}, %zmm6, %k5
	vcmpneq_oqph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpneq_oqph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpge_osph	%zmm5, %zmm6, %k5
	vcmpge_osph	%zmm5, %zmm6, %k5{%k7}
	vcmpge_osph	{sae}, %zmm5, %zmm6, %k5
	vcmpge_osph	(%ecx), %zmm6, %k5
	vcmpge_osph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpge_osph	(%eax){1to32}, %zmm6, %k5
	vcmpge_osph	8128(%edx), %zmm6, %k5
	vcmpge_osph	8192(%edx), %zmm6, %k5
	vcmpge_osph	-8192(%edx), %zmm6, %k5
	vcmpge_osph	-8256(%edx), %zmm6, %k5
	vcmpge_osph	1016(%edx){1to32}, %zmm6, %k5
	vcmpge_osph	1024(%edx){1to32}, %zmm6, %k5
	vcmpge_osph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpge_osph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpgeph	%zmm5, %zmm6, %k5
	vcmpgeph	%zmm5, %zmm6, %k5{%k7}
	vcmpgeph	{sae}, %zmm5, %zmm6, %k5
	vcmpgeph	(%ecx), %zmm6, %k5
	vcmpgeph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpgeph	(%eax){1to32}, %zmm6, %k5
	vcmpgeph	8128(%edx), %zmm6, %k5
	vcmpgeph	8192(%edx), %zmm6, %k5
	vcmpgeph	-8192(%edx), %zmm6, %k5
	vcmpgeph	-8256(%edx), %zmm6, %k5
	vcmpgeph	1016(%edx){1to32}, %zmm6, %k5
	vcmpgeph	1024(%edx){1to32}, %zmm6, %k5
	vcmpgeph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpgeph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpgt_osph	%zmm5, %zmm6, %k5
	vcmpgt_osph	%zmm5, %zmm6, %k5{%k7}
	vcmpgt_osph	{sae}, %zmm5, %zmm6, %k5
	vcmpgt_osph	(%ecx), %zmm6, %k5
	vcmpgt_osph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpgt_osph	(%eax){1to32}, %zmm6, %k5
	vcmpgt_osph	8128(%edx), %zmm6, %k5
	vcmpgt_osph	8192(%edx), %zmm6, %k5
	vcmpgt_osph	-8192(%edx), %zmm6, %k5
	vcmpgt_osph	-8256(%edx), %zmm6, %k5
	vcmpgt_osph	1016(%edx){1to32}, %zmm6, %k5
	vcmpgt_osph	1024(%edx){1to32}, %zmm6, %k5
	vcmpgt_osph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpgt_osph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpgtph	%zmm5, %zmm6, %k5
	vcmpgtph	%zmm5, %zmm6, %k5{%k7}
	vcmpgtph	{sae}, %zmm5, %zmm6, %k5
	vcmpgtph	(%ecx), %zmm6, %k5
	vcmpgtph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpgtph	(%eax){1to32}, %zmm6, %k5
	vcmpgtph	8128(%edx), %zmm6, %k5
	vcmpgtph	8192(%edx), %zmm6, %k5
	vcmpgtph	-8192(%edx), %zmm6, %k5
	vcmpgtph	-8256(%edx), %zmm6, %k5
	vcmpgtph	1016(%edx){1to32}, %zmm6, %k5
	vcmpgtph	1024(%edx){1to32}, %zmm6, %k5
	vcmpgtph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpgtph	-1032(%edx){1to32}, %zmm6, %k5
	vcmptrue_uqph	%zmm5, %zmm6, %k5
	vcmptrue_uqph	%zmm5, %zmm6, %k5{%k7}
	vcmptrue_uqph	{sae}, %zmm5, %zmm6, %k5
	vcmptrue_uqph	(%ecx), %zmm6, %k5
	vcmptrue_uqph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmptrue_uqph	(%eax){1to32}, %zmm6, %k5
	vcmptrue_uqph	8128(%edx), %zmm6, %k5
	vcmptrue_uqph	8192(%edx), %zmm6, %k5
	vcmptrue_uqph	-8192(%edx), %zmm6, %k5
	vcmptrue_uqph	-8256(%edx), %zmm6, %k5
	vcmptrue_uqph	1016(%edx){1to32}, %zmm6, %k5
	vcmptrue_uqph	1024(%edx){1to32}, %zmm6, %k5
	vcmptrue_uqph	-1024(%edx){1to32}, %zmm6, %k5
	vcmptrue_uqph	-1032(%edx){1to32}, %zmm6, %k5
	vcmptrueph	%zmm5, %zmm6, %k5
	vcmptrueph	%zmm5, %zmm6, %k5{%k7}
	vcmptrueph	{sae}, %zmm5, %zmm6, %k5
	vcmptrueph	(%ecx), %zmm6, %k5
	vcmptrueph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmptrueph	(%eax){1to32}, %zmm6, %k5
	vcmptrueph	8128(%edx), %zmm6, %k5
	vcmptrueph	8192(%edx), %zmm6, %k5
	vcmptrueph	-8192(%edx), %zmm6, %k5
	vcmptrueph	-8256(%edx), %zmm6, %k5
	vcmptrueph	1016(%edx){1to32}, %zmm6, %k5
	vcmptrueph	1024(%edx){1to32}, %zmm6, %k5
	vcmptrueph	-1024(%edx){1to32}, %zmm6, %k5
	vcmptrueph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpeq_osph	%zmm5, %zmm6, %k5
	vcmpeq_osph	%zmm5, %zmm6, %k5{%k7}
	vcmpeq_osph	{sae}, %zmm5, %zmm6, %k5
	vcmpeq_osph	(%ecx), %zmm6, %k5
	vcmpeq_osph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpeq_osph	(%eax){1to32}, %zmm6, %k5
	vcmpeq_osph	8128(%edx), %zmm6, %k5
	vcmpeq_osph	8192(%edx), %zmm6, %k5
	vcmpeq_osph	-8192(%edx), %zmm6, %k5
	vcmpeq_osph	-8256(%edx), %zmm6, %k5
	vcmpeq_osph	1016(%edx){1to32}, %zmm6, %k5
	vcmpeq_osph	1024(%edx){1to32}, %zmm6, %k5
	vcmpeq_osph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpeq_osph	-1032(%edx){1to32}, %zmm6, %k5
	vcmplt_oqph	%zmm5, %zmm6, %k5
	vcmplt_oqph	%zmm5, %zmm6, %k5{%k7}
	vcmplt_oqph	{sae}, %zmm5, %zmm6, %k5
	vcmplt_oqph	(%ecx), %zmm6, %k5
	vcmplt_oqph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmplt_oqph	(%eax){1to32}, %zmm6, %k5
	vcmplt_oqph	8128(%edx), %zmm6, %k5
	vcmplt_oqph	8192(%edx), %zmm6, %k5
	vcmplt_oqph	-8192(%edx), %zmm6, %k5
	vcmplt_oqph	-8256(%edx), %zmm6, %k5
	vcmplt_oqph	1016(%edx){1to32}, %zmm6, %k5
	vcmplt_oqph	1024(%edx){1to32}, %zmm6, %k5
	vcmplt_oqph	-1024(%edx){1to32}, %zmm6, %k5
	vcmplt_oqph	-1032(%edx){1to32}, %zmm6, %k5
	vcmple_oqph	%zmm5, %zmm6, %k5
	vcmple_oqph	%zmm5, %zmm6, %k5{%k7}
	vcmple_oqph	{sae}, %zmm5, %zmm6, %k5
	vcmple_oqph	(%ecx), %zmm6, %k5
	vcmple_oqph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmple_oqph	(%eax){1to32}, %zmm6, %k5
	vcmple_oqph	8128(%edx), %zmm6, %k5
	vcmple_oqph	8192(%edx), %zmm6, %k5
	vcmple_oqph	-8192(%edx), %zmm6, %k5
	vcmple_oqph	-8256(%edx), %zmm6, %k5
	vcmple_oqph	1016(%edx){1to32}, %zmm6, %k5
	vcmple_oqph	1024(%edx){1to32}, %zmm6, %k5
	vcmple_oqph	-1024(%edx){1to32}, %zmm6, %k5
	vcmple_oqph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpunord_sph	%zmm5, %zmm6, %k5
	vcmpunord_sph	%zmm5, %zmm6, %k5{%k7}
	vcmpunord_sph	{sae}, %zmm5, %zmm6, %k5
	vcmpunord_sph	(%ecx), %zmm6, %k5
	vcmpunord_sph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpunord_sph	(%eax){1to32}, %zmm6, %k5
	vcmpunord_sph	8128(%edx), %zmm6, %k5
	vcmpunord_sph	8192(%edx), %zmm6, %k5
	vcmpunord_sph	-8192(%edx), %zmm6, %k5
	vcmpunord_sph	-8256(%edx), %zmm6, %k5
	vcmpunord_sph	1016(%edx){1to32}, %zmm6, %k5
	vcmpunord_sph	1024(%edx){1to32}, %zmm6, %k5
	vcmpunord_sph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpunord_sph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpneq_usph	%zmm5, %zmm6, %k5
	vcmpneq_usph	%zmm5, %zmm6, %k5{%k7}
	vcmpneq_usph	{sae}, %zmm5, %zmm6, %k5
	vcmpneq_usph	(%ecx), %zmm6, %k5
	vcmpneq_usph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpneq_usph	(%eax){1to32}, %zmm6, %k5
	vcmpneq_usph	8128(%edx), %zmm6, %k5
	vcmpneq_usph	8192(%edx), %zmm6, %k5
	vcmpneq_usph	-8192(%edx), %zmm6, %k5
	vcmpneq_usph	-8256(%edx), %zmm6, %k5
	vcmpneq_usph	1016(%edx){1to32}, %zmm6, %k5
	vcmpneq_usph	1024(%edx){1to32}, %zmm6, %k5
	vcmpneq_usph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpneq_usph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpnlt_uqph	%zmm5, %zmm6, %k5
	vcmpnlt_uqph	%zmm5, %zmm6, %k5{%k7}
	vcmpnlt_uqph	{sae}, %zmm5, %zmm6, %k5
	vcmpnlt_uqph	(%ecx), %zmm6, %k5
	vcmpnlt_uqph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpnlt_uqph	(%eax){1to32}, %zmm6, %k5
	vcmpnlt_uqph	8128(%edx), %zmm6, %k5
	vcmpnlt_uqph	8192(%edx), %zmm6, %k5
	vcmpnlt_uqph	-8192(%edx), %zmm6, %k5
	vcmpnlt_uqph	-8256(%edx), %zmm6, %k5
	vcmpnlt_uqph	1016(%edx){1to32}, %zmm6, %k5
	vcmpnlt_uqph	1024(%edx){1to32}, %zmm6, %k5
	vcmpnlt_uqph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpnlt_uqph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpnle_uqph	%zmm5, %zmm6, %k5
	vcmpnle_uqph	%zmm5, %zmm6, %k5{%k7}
	vcmpnle_uqph	{sae}, %zmm5, %zmm6, %k5
	vcmpnle_uqph	(%ecx), %zmm6, %k5
	vcmpnle_uqph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpnle_uqph	(%eax){1to32}, %zmm6, %k5
	vcmpnle_uqph	8128(%edx), %zmm6, %k5
	vcmpnle_uqph	8192(%edx), %zmm6, %k5
	vcmpnle_uqph	-8192(%edx), %zmm6, %k5
	vcmpnle_uqph	-8256(%edx), %zmm6, %k5
	vcmpnle_uqph	1016(%edx){1to32}, %zmm6, %k5
	vcmpnle_uqph	1024(%edx){1to32}, %zmm6, %k5
	vcmpnle_uqph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpnle_uqph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpord_sph	%zmm5, %zmm6, %k5
	vcmpord_sph	%zmm5, %zmm6, %k5{%k7}
	vcmpord_sph	{sae}, %zmm5, %zmm6, %k5
	vcmpord_sph	(%ecx), %zmm6, %k5
	vcmpord_sph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpord_sph	(%eax){1to32}, %zmm6, %k5
	vcmpord_sph	8128(%edx), %zmm6, %k5
	vcmpord_sph	8192(%edx), %zmm6, %k5
	vcmpord_sph	-8192(%edx), %zmm6, %k5
	vcmpord_sph	-8256(%edx), %zmm6, %k5
	vcmpord_sph	1016(%edx){1to32}, %zmm6, %k5
	vcmpord_sph	1024(%edx){1to32}, %zmm6, %k5
	vcmpord_sph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpord_sph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpeq_usph	%zmm5, %zmm6, %k5
	vcmpeq_usph	%zmm5, %zmm6, %k5{%k7}
	vcmpeq_usph	{sae}, %zmm5, %zmm6, %k5
	vcmpeq_usph	(%ecx), %zmm6, %k5
	vcmpeq_usph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpeq_usph	(%eax){1to32}, %zmm6, %k5
	vcmpeq_usph	8128(%edx), %zmm6, %k5
	vcmpeq_usph	8192(%edx), %zmm6, %k5
	vcmpeq_usph	-8192(%edx), %zmm6, %k5
	vcmpeq_usph	-8256(%edx), %zmm6, %k5
	vcmpeq_usph	1016(%edx){1to32}, %zmm6, %k5
	vcmpeq_usph	1024(%edx){1to32}, %zmm6, %k5
	vcmpeq_usph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpeq_usph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpnge_uqph	%zmm5, %zmm6, %k5
	vcmpnge_uqph	%zmm5, %zmm6, %k5{%k7}
	vcmpnge_uqph	{sae}, %zmm5, %zmm6, %k5
	vcmpnge_uqph	(%ecx), %zmm6, %k5
	vcmpnge_uqph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpnge_uqph	(%eax){1to32}, %zmm6, %k5
	vcmpnge_uqph	8128(%edx), %zmm6, %k5
	vcmpnge_uqph	8192(%edx), %zmm6, %k5
	vcmpnge_uqph	-8192(%edx), %zmm6, %k5
	vcmpnge_uqph	-8256(%edx), %zmm6, %k5
	vcmpnge_uqph	1016(%edx){1to32}, %zmm6, %k5
	vcmpnge_uqph	1024(%edx){1to32}, %zmm6, %k5
	vcmpnge_uqph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpnge_uqph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpngt_uqph	%zmm5, %zmm6, %k5
	vcmpngt_uqph	%zmm5, %zmm6, %k5{%k7}
	vcmpngt_uqph	{sae}, %zmm5, %zmm6, %k5
	vcmpngt_uqph	(%ecx), %zmm6, %k5
	vcmpngt_uqph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpngt_uqph	(%eax){1to32}, %zmm6, %k5
	vcmpngt_uqph	8128(%edx), %zmm6, %k5
	vcmpngt_uqph	8192(%edx), %zmm6, %k5
	vcmpngt_uqph	-8192(%edx), %zmm6, %k5
	vcmpngt_uqph	-8256(%edx), %zmm6, %k5
	vcmpngt_uqph	1016(%edx){1to32}, %zmm6, %k5
	vcmpngt_uqph	1024(%edx){1to32}, %zmm6, %k5
	vcmpngt_uqph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpngt_uqph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpfalse_osph	%zmm5, %zmm6, %k5
	vcmpfalse_osph	%zmm5, %zmm6, %k5{%k7}
	vcmpfalse_osph	{sae}, %zmm5, %zmm6, %k5
	vcmpfalse_osph	(%ecx), %zmm6, %k5
	vcmpfalse_osph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpfalse_osph	(%eax){1to32}, %zmm6, %k5
	vcmpfalse_osph	8128(%edx), %zmm6, %k5
	vcmpfalse_osph	8192(%edx), %zmm6, %k5
	vcmpfalse_osph	-8192(%edx), %zmm6, %k5
	vcmpfalse_osph	-8256(%edx), %zmm6, %k5
	vcmpfalse_osph	1016(%edx){1to32}, %zmm6, %k5
	vcmpfalse_osph	1024(%edx){1to32}, %zmm6, %k5
	vcmpfalse_osph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpfalse_osph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpneq_osph	%zmm5, %zmm6, %k5
	vcmpneq_osph	%zmm5, %zmm6, %k5{%k7}
	vcmpneq_osph	{sae}, %zmm5, %zmm6, %k5
	vcmpneq_osph	(%ecx), %zmm6, %k5
	vcmpneq_osph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpneq_osph	(%eax){1to32}, %zmm6, %k5
	vcmpneq_osph	8128(%edx), %zmm6, %k5
	vcmpneq_osph	8192(%edx), %zmm6, %k5
	vcmpneq_osph	-8192(%edx), %zmm6, %k5
	vcmpneq_osph	-8256(%edx), %zmm6, %k5
	vcmpneq_osph	1016(%edx){1to32}, %zmm6, %k5
	vcmpneq_osph	1024(%edx){1to32}, %zmm6, %k5
	vcmpneq_osph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpneq_osph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpge_oqph	%zmm5, %zmm6, %k5
	vcmpge_oqph	%zmm5, %zmm6, %k5{%k7}
	vcmpge_oqph	{sae}, %zmm5, %zmm6, %k5
	vcmpge_oqph	(%ecx), %zmm6, %k5
	vcmpge_oqph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpge_oqph	(%eax){1to32}, %zmm6, %k5
	vcmpge_oqph	8128(%edx), %zmm6, %k5
	vcmpge_oqph	8192(%edx), %zmm6, %k5
	vcmpge_oqph	-8192(%edx), %zmm6, %k5
	vcmpge_oqph	-8256(%edx), %zmm6, %k5
	vcmpge_oqph	1016(%edx){1to32}, %zmm6, %k5
	vcmpge_oqph	1024(%edx){1to32}, %zmm6, %k5
	vcmpge_oqph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpge_oqph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpgt_oqph	%zmm5, %zmm6, %k5
	vcmpgt_oqph	%zmm5, %zmm6, %k5{%k7}
	vcmpgt_oqph	{sae}, %zmm5, %zmm6, %k5
	vcmpgt_oqph	(%ecx), %zmm6, %k5
	vcmpgt_oqph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmpgt_oqph	(%eax){1to32}, %zmm6, %k5
	vcmpgt_oqph	8128(%edx), %zmm6, %k5
	vcmpgt_oqph	8192(%edx), %zmm6, %k5
	vcmpgt_oqph	-8192(%edx), %zmm6, %k5
	vcmpgt_oqph	-8256(%edx), %zmm6, %k5
	vcmpgt_oqph	1016(%edx){1to32}, %zmm6, %k5
	vcmpgt_oqph	1024(%edx){1to32}, %zmm6, %k5
	vcmpgt_oqph	-1024(%edx){1to32}, %zmm6, %k5
	vcmpgt_oqph	-1032(%edx){1to32}, %zmm6, %k5
	vcmptrue_usph	%zmm5, %zmm6, %k5
	vcmptrue_usph	%zmm5, %zmm6, %k5{%k7}
	vcmptrue_usph	{sae}, %zmm5, %zmm6, %k5
	vcmptrue_usph	(%ecx), %zmm6, %k5
	vcmptrue_usph	-123456(%esp,%esi,8), %zmm6, %k5
	vcmptrue_usph	(%eax){1to32}, %zmm6, %k5
	vcmptrue_usph	8128(%edx), %zmm6, %k5
	vcmptrue_usph	8192(%edx), %zmm6, %k5
	vcmptrue_usph	-8192(%edx), %zmm6, %k5
	vcmptrue_usph	-8256(%edx), %zmm6, %k5
	vcmptrue_usph	1016(%edx){1to32}, %zmm6, %k5
	vcmptrue_usph	1024(%edx){1to32}, %zmm6, %k5
	vcmptrue_usph	-1024(%edx){1to32}, %zmm6, %k5
	vcmptrue_usph	-1032(%edx){1to32}, %zmm6, %k5
	vcmpeq_oqsh	%xmm4, %xmm5, %k5{%k7}
	vcmpeq_oqsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpeq_oqsh	(%ecx), %xmm5, %k5{%k7}
	vcmpeq_oqsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpeq_oqsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpeq_oqsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpeq_oqsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpeq_oqsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpeqsh	%xmm4, %xmm5, %k5{%k7}
	vcmpeqsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpeqsh	(%ecx), %xmm5, %k5{%k7}
	vcmpeqsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpeqsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpeqsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpeqsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpeqsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmplt_ossh	%xmm4, %xmm5, %k5{%k7}
	vcmplt_ossh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmplt_ossh	(%ecx), %xmm5, %k5{%k7}
	vcmplt_ossh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmplt_ossh	1016(%edx), %xmm5, %k5{%k7}
	vcmplt_ossh	1024(%edx), %xmm5, %k5{%k7}
	vcmplt_ossh	-1024(%edx), %xmm5, %k5{%k7}
	vcmplt_ossh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpltsh	%xmm4, %xmm5, %k5{%k7}
	vcmpltsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpltsh	(%ecx), %xmm5, %k5{%k7}
	vcmpltsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpltsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpltsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpltsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpltsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmple_ossh	%xmm4, %xmm5, %k5{%k7}
	vcmple_ossh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmple_ossh	(%ecx), %xmm5, %k5{%k7}
	vcmple_ossh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmple_ossh	1016(%edx), %xmm5, %k5{%k7}
	vcmple_ossh	1024(%edx), %xmm5, %k5{%k7}
	vcmple_ossh	-1024(%edx), %xmm5, %k5{%k7}
	vcmple_ossh	-1032(%edx), %xmm5, %k5{%k7}
	vcmplesh	%xmm4, %xmm5, %k5{%k7}
	vcmplesh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmplesh	(%ecx), %xmm5, %k5{%k7}
	vcmplesh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmplesh	1016(%edx), %xmm5, %k5{%k7}
	vcmplesh	1024(%edx), %xmm5, %k5{%k7}
	vcmplesh	-1024(%edx), %xmm5, %k5{%k7}
	vcmplesh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpunord_qsh	%xmm4, %xmm5, %k5{%k7}
	vcmpunord_qsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpunord_qsh	(%ecx), %xmm5, %k5{%k7}
	vcmpunord_qsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpunord_qsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpunord_qsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpunord_qsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpunord_qsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpunordsh	%xmm4, %xmm5, %k5{%k7}
	vcmpunordsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpunordsh	(%ecx), %xmm5, %k5{%k7}
	vcmpunordsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpunordsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpunordsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpunordsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpunordsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpneq_uqsh	%xmm4, %xmm5, %k5{%k7}
	vcmpneq_uqsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpneq_uqsh	(%ecx), %xmm5, %k5{%k7}
	vcmpneq_uqsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpneq_uqsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpneq_uqsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpneq_uqsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpneq_uqsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpneqsh	%xmm4, %xmm5, %k5{%k7}
	vcmpneqsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpneqsh	(%ecx), %xmm5, %k5{%k7}
	vcmpneqsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpneqsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpneqsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpneqsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpneqsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpnlt_ussh	%xmm4, %xmm5, %k5{%k7}
	vcmpnlt_ussh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpnlt_ussh	(%ecx), %xmm5, %k5{%k7}
	vcmpnlt_ussh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpnlt_ussh	1016(%edx), %xmm5, %k5{%k7}
	vcmpnlt_ussh	1024(%edx), %xmm5, %k5{%k7}
	vcmpnlt_ussh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpnlt_ussh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpnltsh	%xmm4, %xmm5, %k5{%k7}
	vcmpnltsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpnltsh	(%ecx), %xmm5, %k5{%k7}
	vcmpnltsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpnltsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpnltsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpnltsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpnltsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpnle_ussh	%xmm4, %xmm5, %k5{%k7}
	vcmpnle_ussh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpnle_ussh	(%ecx), %xmm5, %k5{%k7}
	vcmpnle_ussh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpnle_ussh	1016(%edx), %xmm5, %k5{%k7}
	vcmpnle_ussh	1024(%edx), %xmm5, %k5{%k7}
	vcmpnle_ussh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpnle_ussh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpnlesh	%xmm4, %xmm5, %k5{%k7}
	vcmpnlesh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpnlesh	(%ecx), %xmm5, %k5{%k7}
	vcmpnlesh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpnlesh	1016(%edx), %xmm5, %k5{%k7}
	vcmpnlesh	1024(%edx), %xmm5, %k5{%k7}
	vcmpnlesh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpnlesh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpord_qsh	%xmm4, %xmm5, %k5{%k7}
	vcmpord_qsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpord_qsh	(%ecx), %xmm5, %k5{%k7}
	vcmpord_qsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpord_qsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpord_qsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpord_qsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpord_qsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpordsh	%xmm4, %xmm5, %k5{%k7}
	vcmpordsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpordsh	(%ecx), %xmm5, %k5{%k7}
	vcmpordsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpordsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpordsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpordsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpordsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpeq_uqsh	%xmm4, %xmm5, %k5{%k7}
	vcmpeq_uqsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpeq_uqsh	(%ecx), %xmm5, %k5{%k7}
	vcmpeq_uqsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpeq_uqsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpeq_uqsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpeq_uqsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpeq_uqsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpnge_ussh	%xmm4, %xmm5, %k5{%k7}
	vcmpnge_ussh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpnge_ussh	(%ecx), %xmm5, %k5{%k7}
	vcmpnge_ussh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpnge_ussh	1016(%edx), %xmm5, %k5{%k7}
	vcmpnge_ussh	1024(%edx), %xmm5, %k5{%k7}
	vcmpnge_ussh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpnge_ussh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpngesh	%xmm4, %xmm5, %k5{%k7}
	vcmpngesh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpngesh	(%ecx), %xmm5, %k5{%k7}
	vcmpngesh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpngesh	1016(%edx), %xmm5, %k5{%k7}
	vcmpngesh	1024(%edx), %xmm5, %k5{%k7}
	vcmpngesh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpngesh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpngt_ussh	%xmm4, %xmm5, %k5{%k7}
	vcmpngt_ussh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpngt_ussh	(%ecx), %xmm5, %k5{%k7}
	vcmpngt_ussh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpngt_ussh	1016(%edx), %xmm5, %k5{%k7}
	vcmpngt_ussh	1024(%edx), %xmm5, %k5{%k7}
	vcmpngt_ussh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpngt_ussh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpngtsh	%xmm4, %xmm5, %k5{%k7}
	vcmpngtsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpngtsh	(%ecx), %xmm5, %k5{%k7}
	vcmpngtsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpngtsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpngtsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpngtsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpngtsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpfalse_oqsh	%xmm4, %xmm5, %k5{%k7}
	vcmpfalse_oqsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpfalse_oqsh	(%ecx), %xmm5, %k5{%k7}
	vcmpfalse_oqsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpfalse_oqsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpfalse_oqsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpfalse_oqsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpfalse_oqsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpfalsesh	%xmm4, %xmm5, %k5{%k7}
	vcmpfalsesh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpfalsesh	(%ecx), %xmm5, %k5{%k7}
	vcmpfalsesh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpfalsesh	1016(%edx), %xmm5, %k5{%k7}
	vcmpfalsesh	1024(%edx), %xmm5, %k5{%k7}
	vcmpfalsesh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpfalsesh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpneq_oqsh	%xmm4, %xmm5, %k5{%k7}
	vcmpneq_oqsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpneq_oqsh	(%ecx), %xmm5, %k5{%k7}
	vcmpneq_oqsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpneq_oqsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpneq_oqsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpneq_oqsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpneq_oqsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpge_ossh	%xmm4, %xmm5, %k5{%k7}
	vcmpge_ossh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpge_ossh	(%ecx), %xmm5, %k5{%k7}
	vcmpge_ossh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpge_ossh	1016(%edx), %xmm5, %k5{%k7}
	vcmpge_ossh	1024(%edx), %xmm5, %k5{%k7}
	vcmpge_ossh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpge_ossh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpgesh	%xmm4, %xmm5, %k5{%k7}
	vcmpgesh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpgesh	(%ecx), %xmm5, %k5{%k7}
	vcmpgesh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpgesh	1016(%edx), %xmm5, %k5{%k7}
	vcmpgesh	1024(%edx), %xmm5, %k5{%k7}
	vcmpgesh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpgesh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpgt_ossh	%xmm4, %xmm5, %k5{%k7}
	vcmpgt_ossh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpgt_ossh	(%ecx), %xmm5, %k5{%k7}
	vcmpgt_ossh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpgt_ossh	1016(%edx), %xmm5, %k5{%k7}
	vcmpgt_ossh	1024(%edx), %xmm5, %k5{%k7}
	vcmpgt_ossh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpgt_ossh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpgtsh	%xmm4, %xmm5, %k5{%k7}
	vcmpgtsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpgtsh	(%ecx), %xmm5, %k5{%k7}
	vcmpgtsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpgtsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpgtsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpgtsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpgtsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmptrue_uqsh	%xmm4, %xmm5, %k5{%k7}
	vcmptrue_uqsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmptrue_uqsh	(%ecx), %xmm5, %k5{%k7}
	vcmptrue_uqsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmptrue_uqsh	1016(%edx), %xmm5, %k5{%k7}
	vcmptrue_uqsh	1024(%edx), %xmm5, %k5{%k7}
	vcmptrue_uqsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmptrue_uqsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmptruesh	%xmm4, %xmm5, %k5{%k7}
	vcmptruesh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmptruesh	(%ecx), %xmm5, %k5{%k7}
	vcmptruesh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmptruesh	1016(%edx), %xmm5, %k5{%k7}
	vcmptruesh	1024(%edx), %xmm5, %k5{%k7}
	vcmptruesh	-1024(%edx), %xmm5, %k5{%k7}
	vcmptruesh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpeq_ossh	%xmm4, %xmm5, %k5{%k7}
	vcmpeq_ossh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpeq_ossh	(%ecx), %xmm5, %k5{%k7}
	vcmpeq_ossh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpeq_ossh	1016(%edx), %xmm5, %k5{%k7}
	vcmpeq_ossh	1024(%edx), %xmm5, %k5{%k7}
	vcmpeq_ossh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpeq_ossh	-1032(%edx), %xmm5, %k5{%k7}
	vcmplt_oqsh	%xmm4, %xmm5, %k5{%k7}
	vcmplt_oqsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmplt_oqsh	(%ecx), %xmm5, %k5{%k7}
	vcmplt_oqsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmplt_oqsh	1016(%edx), %xmm5, %k5{%k7}
	vcmplt_oqsh	1024(%edx), %xmm5, %k5{%k7}
	vcmplt_oqsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmplt_oqsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmple_oqsh	%xmm4, %xmm5, %k5{%k7}
	vcmple_oqsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmple_oqsh	(%ecx), %xmm5, %k5{%k7}
	vcmple_oqsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmple_oqsh	1016(%edx), %xmm5, %k5{%k7}
	vcmple_oqsh	1024(%edx), %xmm5, %k5{%k7}
	vcmple_oqsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmple_oqsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpunord_ssh	%xmm4, %xmm5, %k5{%k7}
	vcmpunord_ssh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpunord_ssh	(%ecx), %xmm5, %k5{%k7}
	vcmpunord_ssh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpunord_ssh	1016(%edx), %xmm5, %k5{%k7}
	vcmpunord_ssh	1024(%edx), %xmm5, %k5{%k7}
	vcmpunord_ssh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpunord_ssh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpneq_ussh	%xmm4, %xmm5, %k5{%k7}
	vcmpneq_ussh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpneq_ussh	(%ecx), %xmm5, %k5{%k7}
	vcmpneq_ussh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpneq_ussh	1016(%edx), %xmm5, %k5{%k7}
	vcmpneq_ussh	1024(%edx), %xmm5, %k5{%k7}
	vcmpneq_ussh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpneq_ussh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpnlt_uqsh	%xmm4, %xmm5, %k5{%k7}
	vcmpnlt_uqsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpnlt_uqsh	(%ecx), %xmm5, %k5{%k7}
	vcmpnlt_uqsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpnlt_uqsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpnlt_uqsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpnlt_uqsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpnlt_uqsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpnle_uqsh	%xmm4, %xmm5, %k5{%k7}
	vcmpnle_uqsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpnle_uqsh	(%ecx), %xmm5, %k5{%k7}
	vcmpnle_uqsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpnle_uqsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpnle_uqsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpnle_uqsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpnle_uqsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpord_ssh	%xmm4, %xmm5, %k5{%k7}
	vcmpord_ssh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpord_ssh	(%ecx), %xmm5, %k5{%k7}
	vcmpord_ssh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpord_ssh	1016(%edx), %xmm5, %k5{%k7}
	vcmpord_ssh	1024(%edx), %xmm5, %k5{%k7}
	vcmpord_ssh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpord_ssh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpeq_ussh	%xmm4, %xmm5, %k5{%k7}
	vcmpeq_ussh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpeq_ussh	(%ecx), %xmm5, %k5{%k7}
	vcmpeq_ussh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpeq_ussh	1016(%edx), %xmm5, %k5{%k7}
	vcmpeq_ussh	1024(%edx), %xmm5, %k5{%k7}
	vcmpeq_ussh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpeq_ussh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpnge_uqsh	%xmm4, %xmm5, %k5{%k7}
	vcmpnge_uqsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpnge_uqsh	(%ecx), %xmm5, %k5{%k7}
	vcmpnge_uqsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpnge_uqsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpnge_uqsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpnge_uqsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpnge_uqsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpngt_uqsh	%xmm4, %xmm5, %k5{%k7}
	vcmpngt_uqsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpngt_uqsh	(%ecx), %xmm5, %k5{%k7}
	vcmpngt_uqsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpngt_uqsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpngt_uqsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpngt_uqsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpngt_uqsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpfalse_ossh	%xmm4, %xmm5, %k5{%k7}
	vcmpfalse_ossh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpfalse_ossh	(%ecx), %xmm5, %k5{%k7}
	vcmpfalse_ossh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpfalse_ossh	1016(%edx), %xmm5, %k5{%k7}
	vcmpfalse_ossh	1024(%edx), %xmm5, %k5{%k7}
	vcmpfalse_ossh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpfalse_ossh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpneq_ossh	%xmm4, %xmm5, %k5{%k7}
	vcmpneq_ossh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpneq_ossh	(%ecx), %xmm5, %k5{%k7}
	vcmpneq_ossh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpneq_ossh	1016(%edx), %xmm5, %k5{%k7}
	vcmpneq_ossh	1024(%edx), %xmm5, %k5{%k7}
	vcmpneq_ossh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpneq_ossh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpge_oqsh	%xmm4, %xmm5, %k5{%k7}
	vcmpge_oqsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpge_oqsh	(%ecx), %xmm5, %k5{%k7}
	vcmpge_oqsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpge_oqsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpge_oqsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpge_oqsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpge_oqsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmpgt_oqsh	%xmm4, %xmm5, %k5{%k7}
	vcmpgt_oqsh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmpgt_oqsh	(%ecx), %xmm5, %k5{%k7}
	vcmpgt_oqsh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmpgt_oqsh	1016(%edx), %xmm5, %k5{%k7}
	vcmpgt_oqsh	1024(%edx), %xmm5, %k5{%k7}
	vcmpgt_oqsh	-1024(%edx), %xmm5, %k5{%k7}
	vcmpgt_oqsh	-1032(%edx), %xmm5, %k5{%k7}
	vcmptrue_ussh	%xmm4, %xmm5, %k5{%k7}
	vcmptrue_ussh	{sae}, %xmm4, %xmm5, %k5{%k7}
	vcmptrue_ussh	(%ecx), %xmm5, %k5{%k7}
	vcmptrue_ussh	-123456(%esp,%esi,8), %xmm5, %k5{%k7}
	vcmptrue_ussh	1016(%edx), %xmm5, %k5{%k7}
	vcmptrue_ussh	1024(%edx), %xmm5, %k5{%k7}
	vcmptrue_ussh	-1024(%edx), %xmm5, %k5{%k7}
	vcmptrue_ussh	-1032(%edx), %xmm5, %k5{%k7}
.intel_syntax noprefix	
	vcmpeq_oqph	k5, zmm6, zmm5
	vcmpeq_oqph	k5{k7}, zmm6, zmm5
	vcmpeq_oqph	k5, zmm6, zmm5, {sae}
	vcmpeq_oqph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpeq_oqph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpeq_oqph	k5, zmm6, [eax]{1to32}
	vcmpeq_oqph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpeq_oqph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpeq_oqph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpeq_oqph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpeq_oqph	k5, zmm6, [edx+1016]{1to32}
	vcmpeq_oqph	k5, zmm6, [edx+1024]{1to32}
	vcmpeq_oqph	k5, zmm6, [edx-1024]{1to32}
	vcmpeq_oqph	k5, zmm6, [edx-1032]{1to32}
	vcmpeqph	k5, zmm6, zmm5
	vcmpeqph	k5{k7}, zmm6, zmm5
	vcmpeqph	k5, zmm6, zmm5, {sae}
	vcmpeqph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpeqph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpeqph	k5, zmm6, [eax]{1to32}
	vcmpeqph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpeqph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpeqph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpeqph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpeqph	k5, zmm6, [edx+1016]{1to32}
	vcmpeqph	k5, zmm6, [edx+1024]{1to32}
	vcmpeqph	k5, zmm6, [edx-1024]{1to32}
	vcmpeqph	k5, zmm6, [edx-1032]{1to32}
	vcmplt_osph	k5, zmm6, zmm5
	vcmplt_osph	k5{k7}, zmm6, zmm5
	vcmplt_osph	k5, zmm6, zmm5, {sae}
	vcmplt_osph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmplt_osph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmplt_osph	k5, zmm6, [eax]{1to32}
	vcmplt_osph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmplt_osph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmplt_osph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmplt_osph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmplt_osph	k5, zmm6, [edx+1016]{1to32}
	vcmplt_osph	k5, zmm6, [edx+1024]{1to32}
	vcmplt_osph	k5, zmm6, [edx-1024]{1to32}
	vcmplt_osph	k5, zmm6, [edx-1032]{1to32}
	vcmpltph	k5, zmm6, zmm5
	vcmpltph	k5{k7}, zmm6, zmm5
	vcmpltph	k5, zmm6, zmm5, {sae}
	vcmpltph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpltph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpltph	k5, zmm6, [eax]{1to32}
	vcmpltph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpltph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpltph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpltph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpltph	k5, zmm6, [edx+1016]{1to32}
	vcmpltph	k5, zmm6, [edx+1024]{1to32}
	vcmpltph	k5, zmm6, [edx-1024]{1to32}
	vcmpltph	k5, zmm6, [edx-1032]{1to32}
	vcmple_osph	k5, zmm6, zmm5
	vcmple_osph	k5{k7}, zmm6, zmm5
	vcmple_osph	k5, zmm6, zmm5, {sae}
	vcmple_osph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmple_osph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmple_osph	k5, zmm6, [eax]{1to32}
	vcmple_osph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmple_osph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmple_osph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmple_osph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmple_osph	k5, zmm6, [edx+1016]{1to32}
	vcmple_osph	k5, zmm6, [edx+1024]{1to32}
	vcmple_osph	k5, zmm6, [edx-1024]{1to32}
	vcmple_osph	k5, zmm6, [edx-1032]{1to32}
	vcmpleph	k5, zmm6, zmm5
	vcmpleph	k5{k7}, zmm6, zmm5
	vcmpleph	k5, zmm6, zmm5, {sae}
	vcmpleph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpleph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpleph	k5, zmm6, [eax]{1to32}
	vcmpleph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpleph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpleph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpleph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpleph	k5, zmm6, [edx+1016]{1to32}
	vcmpleph	k5, zmm6, [edx+1024]{1to32}
	vcmpleph	k5, zmm6, [edx-1024]{1to32}
	vcmpleph	k5, zmm6, [edx-1032]{1to32}
	vcmpunord_qph	k5, zmm6, zmm5
	vcmpunord_qph	k5{k7}, zmm6, zmm5
	vcmpunord_qph	k5, zmm6, zmm5, {sae}
	vcmpunord_qph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpunord_qph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpunord_qph	k5, zmm6, [eax]{1to32}
	vcmpunord_qph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpunord_qph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpunord_qph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpunord_qph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpunord_qph	k5, zmm6, [edx+1016]{1to32}
	vcmpunord_qph	k5, zmm6, [edx+1024]{1to32}
	vcmpunord_qph	k5, zmm6, [edx-1024]{1to32}
	vcmpunord_qph	k5, zmm6, [edx-1032]{1to32}
	vcmpunordph	k5, zmm6, zmm5
	vcmpunordph	k5{k7}, zmm6, zmm5
	vcmpunordph	k5, zmm6, zmm5, {sae}
	vcmpunordph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpunordph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpunordph	k5, zmm6, [eax]{1to32}
	vcmpunordph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpunordph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpunordph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpunordph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpunordph	k5, zmm6, [edx+1016]{1to32}
	vcmpunordph	k5, zmm6, [edx+1024]{1to32}
	vcmpunordph	k5, zmm6, [edx-1024]{1to32}
	vcmpunordph	k5, zmm6, [edx-1032]{1to32}
	vcmpneq_uqph	k5, zmm6, zmm5
	vcmpneq_uqph	k5{k7}, zmm6, zmm5
	vcmpneq_uqph	k5, zmm6, zmm5, {sae}
	vcmpneq_uqph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpneq_uqph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpneq_uqph	k5, zmm6, [eax]{1to32}
	vcmpneq_uqph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpneq_uqph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpneq_uqph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpneq_uqph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpneq_uqph	k5, zmm6, [edx+1016]{1to32}
	vcmpneq_uqph	k5, zmm6, [edx+1024]{1to32}
	vcmpneq_uqph	k5, zmm6, [edx-1024]{1to32}
	vcmpneq_uqph	k5, zmm6, [edx-1032]{1to32}
	vcmpneqph	k5, zmm6, zmm5
	vcmpneqph	k5{k7}, zmm6, zmm5
	vcmpneqph	k5, zmm6, zmm5, {sae}
	vcmpneqph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpneqph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpneqph	k5, zmm6, [eax]{1to32}
	vcmpneqph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpneqph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpneqph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpneqph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpneqph	k5, zmm6, [edx+1016]{1to32}
	vcmpneqph	k5, zmm6, [edx+1024]{1to32}
	vcmpneqph	k5, zmm6, [edx-1024]{1to32}
	vcmpneqph	k5, zmm6, [edx-1032]{1to32}
	vcmpnlt_usph	k5, zmm6, zmm5
	vcmpnlt_usph	k5{k7}, zmm6, zmm5
	vcmpnlt_usph	k5, zmm6, zmm5, {sae}
	vcmpnlt_usph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpnlt_usph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpnlt_usph	k5, zmm6, [eax]{1to32}
	vcmpnlt_usph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpnlt_usph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpnlt_usph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpnlt_usph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpnlt_usph	k5, zmm6, [edx+1016]{1to32}
	vcmpnlt_usph	k5, zmm6, [edx+1024]{1to32}
	vcmpnlt_usph	k5, zmm6, [edx-1024]{1to32}
	vcmpnlt_usph	k5, zmm6, [edx-1032]{1to32}
	vcmpnltph	k5, zmm6, zmm5
	vcmpnltph	k5{k7}, zmm6, zmm5
	vcmpnltph	k5, zmm6, zmm5, {sae}
	vcmpnltph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpnltph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpnltph	k5, zmm6, [eax]{1to32}
	vcmpnltph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpnltph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpnltph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpnltph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpnltph	k5, zmm6, [edx+1016]{1to32}
	vcmpnltph	k5, zmm6, [edx+1024]{1to32}
	vcmpnltph	k5, zmm6, [edx-1024]{1to32}
	vcmpnltph	k5, zmm6, [edx-1032]{1to32}
	vcmpnle_usph	k5, zmm6, zmm5
	vcmpnle_usph	k5{k7}, zmm6, zmm5
	vcmpnle_usph	k5, zmm6, zmm5, {sae}
	vcmpnle_usph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpnle_usph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpnle_usph	k5, zmm6, [eax]{1to32}
	vcmpnle_usph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpnle_usph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpnle_usph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpnle_usph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpnle_usph	k5, zmm6, [edx+1016]{1to32}
	vcmpnle_usph	k5, zmm6, [edx+1024]{1to32}
	vcmpnle_usph	k5, zmm6, [edx-1024]{1to32}
	vcmpnle_usph	k5, zmm6, [edx-1032]{1to32}
	vcmpnleph	k5, zmm6, zmm5
	vcmpnleph	k5{k7}, zmm6, zmm5
	vcmpnleph	k5, zmm6, zmm5, {sae}
	vcmpnleph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpnleph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpnleph	k5, zmm6, [eax]{1to32}
	vcmpnleph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpnleph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpnleph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpnleph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpnleph	k5, zmm6, [edx+1016]{1to32}
	vcmpnleph	k5, zmm6, [edx+1024]{1to32}
	vcmpnleph	k5, zmm6, [edx-1024]{1to32}
	vcmpnleph	k5, zmm6, [edx-1032]{1to32}
	vcmpord_qph	k5, zmm6, zmm5
	vcmpord_qph	k5{k7}, zmm6, zmm5
	vcmpord_qph	k5, zmm6, zmm5, {sae}
	vcmpord_qph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpord_qph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpord_qph	k5, zmm6, [eax]{1to32}
	vcmpord_qph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpord_qph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpord_qph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpord_qph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpord_qph	k5, zmm6, [edx+1016]{1to32}
	vcmpord_qph	k5, zmm6, [edx+1024]{1to32}
	vcmpord_qph	k5, zmm6, [edx-1024]{1to32}
	vcmpord_qph	k5, zmm6, [edx-1032]{1to32}
	vcmpordph	k5, zmm6, zmm5
	vcmpordph	k5{k7}, zmm6, zmm5
	vcmpordph	k5, zmm6, zmm5, {sae}
	vcmpordph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpordph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpordph	k5, zmm6, [eax]{1to32}
	vcmpordph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpordph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpordph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpordph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpordph	k5, zmm6, [edx+1016]{1to32}
	vcmpordph	k5, zmm6, [edx+1024]{1to32}
	vcmpordph	k5, zmm6, [edx-1024]{1to32}
	vcmpordph	k5, zmm6, [edx-1032]{1to32}
	vcmpeq_uqph	k5, zmm6, zmm5
	vcmpeq_uqph	k5{k7}, zmm6, zmm5
	vcmpeq_uqph	k5, zmm6, zmm5, {sae}
	vcmpeq_uqph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpeq_uqph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpeq_uqph	k5, zmm6, [eax]{1to32}
	vcmpeq_uqph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpeq_uqph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpeq_uqph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpeq_uqph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpeq_uqph	k5, zmm6, [edx+1016]{1to32}
	vcmpeq_uqph	k5, zmm6, [edx+1024]{1to32}
	vcmpeq_uqph	k5, zmm6, [edx-1024]{1to32}
	vcmpeq_uqph	k5, zmm6, [edx-1032]{1to32}
	vcmpnge_usph	k5, zmm6, zmm5
	vcmpnge_usph	k5{k7}, zmm6, zmm5
	vcmpnge_usph	k5, zmm6, zmm5, {sae}
	vcmpnge_usph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpnge_usph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpnge_usph	k5, zmm6, [eax]{1to32}
	vcmpnge_usph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpnge_usph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpnge_usph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpnge_usph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpnge_usph	k5, zmm6, [edx+1016]{1to32}
	vcmpnge_usph	k5, zmm6, [edx+1024]{1to32}
	vcmpnge_usph	k5, zmm6, [edx-1024]{1to32}
	vcmpnge_usph	k5, zmm6, [edx-1032]{1to32}
	vcmpngeph	k5, zmm6, zmm5
	vcmpngeph	k5{k7}, zmm6, zmm5
	vcmpngeph	k5, zmm6, zmm5, {sae}
	vcmpngeph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpngeph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpngeph	k5, zmm6, [eax]{1to32}
	vcmpngeph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpngeph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpngeph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpngeph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpngeph	k5, zmm6, [edx+1016]{1to32}
	vcmpngeph	k5, zmm6, [edx+1024]{1to32}
	vcmpngeph	k5, zmm6, [edx-1024]{1to32}
	vcmpngeph	k5, zmm6, [edx-1032]{1to32}
	vcmpngt_usph	k5, zmm6, zmm5
	vcmpngt_usph	k5{k7}, zmm6, zmm5
	vcmpngt_usph	k5, zmm6, zmm5, {sae}
	vcmpngt_usph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpngt_usph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpngt_usph	k5, zmm6, [eax]{1to32}
	vcmpngt_usph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpngt_usph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpngt_usph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpngt_usph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpngt_usph	k5, zmm6, [edx+1016]{1to32}
	vcmpngt_usph	k5, zmm6, [edx+1024]{1to32}
	vcmpngt_usph	k5, zmm6, [edx-1024]{1to32}
	vcmpngt_usph	k5, zmm6, [edx-1032]{1to32}
	vcmpngtph	k5, zmm6, zmm5
	vcmpngtph	k5{k7}, zmm6, zmm5
	vcmpngtph	k5, zmm6, zmm5, {sae}
	vcmpngtph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpngtph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpngtph	k5, zmm6, [eax]{1to32}
	vcmpngtph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpngtph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpngtph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpngtph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpngtph	k5, zmm6, [edx+1016]{1to32}
	vcmpngtph	k5, zmm6, [edx+1024]{1to32}
	vcmpngtph	k5, zmm6, [edx-1024]{1to32}
	vcmpngtph	k5, zmm6, [edx-1032]{1to32}
	vcmpfalse_oqph	k5, zmm6, zmm5
	vcmpfalse_oqph	k5{k7}, zmm6, zmm5
	vcmpfalse_oqph	k5, zmm6, zmm5, {sae}
	vcmpfalse_oqph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpfalse_oqph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpfalse_oqph	k5, zmm6, [eax]{1to32}
	vcmpfalse_oqph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpfalse_oqph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpfalse_oqph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpfalse_oqph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpfalse_oqph	k5, zmm6, [edx+1016]{1to32}
	vcmpfalse_oqph	k5, zmm6, [edx+1024]{1to32}
	vcmpfalse_oqph	k5, zmm6, [edx-1024]{1to32}
	vcmpfalse_oqph	k5, zmm6, [edx-1032]{1to32}
	vcmpfalseph	k5, zmm6, zmm5
	vcmpfalseph	k5{k7}, zmm6, zmm5
	vcmpfalseph	k5, zmm6, zmm5, {sae}
	vcmpfalseph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpfalseph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpfalseph	k5, zmm6, [eax]{1to32}
	vcmpfalseph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpfalseph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpfalseph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpfalseph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpfalseph	k5, zmm6, [edx+1016]{1to32}
	vcmpfalseph	k5, zmm6, [edx+1024]{1to32}
	vcmpfalseph	k5, zmm6, [edx-1024]{1to32}
	vcmpfalseph	k5, zmm6, [edx-1032]{1to32}
	vcmpneq_oqph	k5, zmm6, zmm5
	vcmpneq_oqph	k5{k7}, zmm6, zmm5
	vcmpneq_oqph	k5, zmm6, zmm5, {sae}
	vcmpneq_oqph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpneq_oqph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpneq_oqph	k5, zmm6, [eax]{1to32}
	vcmpneq_oqph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpneq_oqph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpneq_oqph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpneq_oqph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpneq_oqph	k5, zmm6, [edx+1016]{1to32}
	vcmpneq_oqph	k5, zmm6, [edx+1024]{1to32}
	vcmpneq_oqph	k5, zmm6, [edx-1024]{1to32}
	vcmpneq_oqph	k5, zmm6, [edx-1032]{1to32}
	vcmpge_osph	k5, zmm6, zmm5
	vcmpge_osph	k5{k7}, zmm6, zmm5
	vcmpge_osph	k5, zmm6, zmm5, {sae}
	vcmpge_osph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpge_osph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpge_osph	k5, zmm6, [eax]{1to32}
	vcmpge_osph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpge_osph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpge_osph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpge_osph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpge_osph	k5, zmm6, [edx+1016]{1to32}
	vcmpge_osph	k5, zmm6, [edx+1024]{1to32}
	vcmpge_osph	k5, zmm6, [edx-1024]{1to32}
	vcmpge_osph	k5, zmm6, [edx-1032]{1to32}
	vcmpgeph	k5, zmm6, zmm5
	vcmpgeph	k5{k7}, zmm6, zmm5
	vcmpgeph	k5, zmm6, zmm5, {sae}
	vcmpgeph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpgeph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpgeph	k5, zmm6, [eax]{1to32}
	vcmpgeph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpgeph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpgeph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpgeph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpgeph	k5, zmm6, [edx+1016]{1to32}
	vcmpgeph	k5, zmm6, [edx+1024]{1to32}
	vcmpgeph	k5, zmm6, [edx-1024]{1to32}
	vcmpgeph	k5, zmm6, [edx-1032]{1to32}
	vcmpgt_osph	k5, zmm6, zmm5
	vcmpgt_osph	k5{k7}, zmm6, zmm5
	vcmpgt_osph	k5, zmm6, zmm5, {sae}
	vcmpgt_osph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpgt_osph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpgt_osph	k5, zmm6, [eax]{1to32}
	vcmpgt_osph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpgt_osph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpgt_osph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpgt_osph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpgt_osph	k5, zmm6, [edx+1016]{1to32}
	vcmpgt_osph	k5, zmm6, [edx+1024]{1to32}
	vcmpgt_osph	k5, zmm6, [edx-1024]{1to32}
	vcmpgt_osph	k5, zmm6, [edx-1032]{1to32}
	vcmpgtph	k5, zmm6, zmm5
	vcmpgtph	k5{k7}, zmm6, zmm5
	vcmpgtph	k5, zmm6, zmm5, {sae}
	vcmpgtph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpgtph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpgtph	k5, zmm6, [eax]{1to32}
	vcmpgtph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpgtph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpgtph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpgtph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpgtph	k5, zmm6, [edx+1016]{1to32}
	vcmpgtph	k5, zmm6, [edx+1024]{1to32}
	vcmpgtph	k5, zmm6, [edx-1024]{1to32}
	vcmpgtph	k5, zmm6, [edx-1032]{1to32}
	vcmptrue_uqph	k5, zmm6, zmm5
	vcmptrue_uqph	k5{k7}, zmm6, zmm5
	vcmptrue_uqph	k5, zmm6, zmm5, {sae}
	vcmptrue_uqph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmptrue_uqph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmptrue_uqph	k5, zmm6, [eax]{1to32}
	vcmptrue_uqph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmptrue_uqph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmptrue_uqph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmptrue_uqph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmptrue_uqph	k5, zmm6, [edx+1016]{1to32}
	vcmptrue_uqph	k5, zmm6, [edx+1024]{1to32}
	vcmptrue_uqph	k5, zmm6, [edx-1024]{1to32}
	vcmptrue_uqph	k5, zmm6, [edx-1032]{1to32}
	vcmptrueph	k5, zmm6, zmm5
	vcmptrueph	k5{k7}, zmm6, zmm5
	vcmptrueph	k5, zmm6, zmm5, {sae}
	vcmptrueph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmptrueph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmptrueph	k5, zmm6, [eax]{1to32}
	vcmptrueph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmptrueph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmptrueph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmptrueph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmptrueph	k5, zmm6, [edx+1016]{1to32}
	vcmptrueph	k5, zmm6, [edx+1024]{1to32}
	vcmptrueph	k5, zmm6, [edx-1024]{1to32}
	vcmptrueph	k5, zmm6, [edx-1032]{1to32}
	vcmpeq_osph	k5, zmm6, zmm5
	vcmpeq_osph	k5{k7}, zmm6, zmm5
	vcmpeq_osph	k5, zmm6, zmm5, {sae}
	vcmpeq_osph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpeq_osph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpeq_osph	k5, zmm6, [eax]{1to32}
	vcmpeq_osph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpeq_osph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpeq_osph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpeq_osph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpeq_osph	k5, zmm6, [edx+1016]{1to32}
	vcmpeq_osph	k5, zmm6, [edx+1024]{1to32}
	vcmpeq_osph	k5, zmm6, [edx-1024]{1to32}
	vcmpeq_osph	k5, zmm6, [edx-1032]{1to32}
	vcmplt_oqph	k5, zmm6, zmm5
	vcmplt_oqph	k5{k7}, zmm6, zmm5
	vcmplt_oqph	k5, zmm6, zmm5, {sae}
	vcmplt_oqph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmplt_oqph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmplt_oqph	k5, zmm6, [eax]{1to32}
	vcmplt_oqph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmplt_oqph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmplt_oqph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmplt_oqph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmplt_oqph	k5, zmm6, [edx+1016]{1to32}
	vcmplt_oqph	k5, zmm6, [edx+1024]{1to32}
	vcmplt_oqph	k5, zmm6, [edx-1024]{1to32}
	vcmplt_oqph	k5, zmm6, [edx-1032]{1to32}
	vcmple_oqph	k5, zmm6, zmm5
	vcmple_oqph	k5{k7}, zmm6, zmm5
	vcmple_oqph	k5, zmm6, zmm5, {sae}
	vcmple_oqph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmple_oqph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmple_oqph	k5, zmm6, [eax]{1to32}
	vcmple_oqph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmple_oqph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmple_oqph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmple_oqph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmple_oqph	k5, zmm6, [edx+1016]{1to32}
	vcmple_oqph	k5, zmm6, [edx+1024]{1to32}
	vcmple_oqph	k5, zmm6, [edx-1024]{1to32}
	vcmple_oqph	k5, zmm6, [edx-1032]{1to32}
	vcmpunord_sph	k5, zmm6, zmm5
	vcmpunord_sph	k5{k7}, zmm6, zmm5
	vcmpunord_sph	k5, zmm6, zmm5, {sae}
	vcmpunord_sph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpunord_sph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpunord_sph	k5, zmm6, [eax]{1to32}
	vcmpunord_sph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpunord_sph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpunord_sph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpunord_sph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpunord_sph	k5, zmm6, [edx+1016]{1to32}
	vcmpunord_sph	k5, zmm6, [edx+1024]{1to32}
	vcmpunord_sph	k5, zmm6, [edx-1024]{1to32}
	vcmpunord_sph	k5, zmm6, [edx-1032]{1to32}
	vcmpneq_usph	k5, zmm6, zmm5
	vcmpneq_usph	k5{k7}, zmm6, zmm5
	vcmpneq_usph	k5, zmm6, zmm5, {sae}
	vcmpneq_usph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpneq_usph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpneq_usph	k5, zmm6, [eax]{1to32}
	vcmpneq_usph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpneq_usph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpneq_usph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpneq_usph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpneq_usph	k5, zmm6, [edx+1016]{1to32}
	vcmpneq_usph	k5, zmm6, [edx+1024]{1to32}
	vcmpneq_usph	k5, zmm6, [edx-1024]{1to32}
	vcmpneq_usph	k5, zmm6, [edx-1032]{1to32}
	vcmpnlt_uqph	k5, zmm6, zmm5
	vcmpnlt_uqph	k5{k7}, zmm6, zmm5
	vcmpnlt_uqph	k5, zmm6, zmm5, {sae}
	vcmpnlt_uqph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpnlt_uqph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpnlt_uqph	k5, zmm6, [eax]{1to32}
	vcmpnlt_uqph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpnlt_uqph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpnlt_uqph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpnlt_uqph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpnlt_uqph	k5, zmm6, [edx+1016]{1to32}
	vcmpnlt_uqph	k5, zmm6, [edx+1024]{1to32}
	vcmpnlt_uqph	k5, zmm6, [edx-1024]{1to32}
	vcmpnlt_uqph	k5, zmm6, [edx-1032]{1to32}
	vcmpnle_uqph	k5, zmm6, zmm5
	vcmpnle_uqph	k5{k7}, zmm6, zmm5
	vcmpnle_uqph	k5, zmm6, zmm5, {sae}
	vcmpnle_uqph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpnle_uqph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpnle_uqph	k5, zmm6, [eax]{1to32}
	vcmpnle_uqph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpnle_uqph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpnle_uqph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpnle_uqph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpnle_uqph	k5, zmm6, [edx+1016]{1to32}
	vcmpnle_uqph	k5, zmm6, [edx+1024]{1to32}
	vcmpnle_uqph	k5, zmm6, [edx-1024]{1to32}
	vcmpnle_uqph	k5, zmm6, [edx-1032]{1to32}
	vcmpord_sph	k5, zmm6, zmm5
	vcmpord_sph	k5{k7}, zmm6, zmm5
	vcmpord_sph	k5, zmm6, zmm5, {sae}
	vcmpord_sph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpord_sph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpord_sph	k5, zmm6, [eax]{1to32}
	vcmpord_sph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpord_sph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpord_sph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpord_sph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpord_sph	k5, zmm6, [edx+1016]{1to32}
	vcmpord_sph	k5, zmm6, [edx+1024]{1to32}
	vcmpord_sph	k5, zmm6, [edx-1024]{1to32}
	vcmpord_sph	k5, zmm6, [edx-1032]{1to32}
	vcmpeq_usph	k5, zmm6, zmm5
	vcmpeq_usph	k5{k7}, zmm6, zmm5
	vcmpeq_usph	k5, zmm6, zmm5, {sae}
	vcmpeq_usph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpeq_usph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpeq_usph	k5, zmm6, [eax]{1to32}
	vcmpeq_usph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpeq_usph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpeq_usph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpeq_usph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpeq_usph	k5, zmm6, [edx+1016]{1to32}
	vcmpeq_usph	k5, zmm6, [edx+1024]{1to32}
	vcmpeq_usph	k5, zmm6, [edx-1024]{1to32}
	vcmpeq_usph	k5, zmm6, [edx-1032]{1to32}
	vcmpnge_uqph	k5, zmm6, zmm5
	vcmpnge_uqph	k5{k7}, zmm6, zmm5
	vcmpnge_uqph	k5, zmm6, zmm5, {sae}
	vcmpnge_uqph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpnge_uqph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpnge_uqph	k5, zmm6, [eax]{1to32}
	vcmpnge_uqph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpnge_uqph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpnge_uqph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpnge_uqph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpnge_uqph	k5, zmm6, [edx+1016]{1to32}
	vcmpnge_uqph	k5, zmm6, [edx+1024]{1to32}
	vcmpnge_uqph	k5, zmm6, [edx-1024]{1to32}
	vcmpnge_uqph	k5, zmm6, [edx-1032]{1to32}
	vcmpngt_uqph	k5, zmm6, zmm5
	vcmpngt_uqph	k5{k7}, zmm6, zmm5
	vcmpngt_uqph	k5, zmm6, zmm5, {sae}
	vcmpngt_uqph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpngt_uqph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpngt_uqph	k5, zmm6, [eax]{1to32}
	vcmpngt_uqph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpngt_uqph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpngt_uqph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpngt_uqph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpngt_uqph	k5, zmm6, [edx+1016]{1to32}
	vcmpngt_uqph	k5, zmm6, [edx+1024]{1to32}
	vcmpngt_uqph	k5, zmm6, [edx-1024]{1to32}
	vcmpngt_uqph	k5, zmm6, [edx-1032]{1to32}
	vcmpfalse_osph	k5, zmm6, zmm5
	vcmpfalse_osph	k5{k7}, zmm6, zmm5
	vcmpfalse_osph	k5, zmm6, zmm5, {sae}
	vcmpfalse_osph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpfalse_osph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpfalse_osph	k5, zmm6, [eax]{1to32}
	vcmpfalse_osph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpfalse_osph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpfalse_osph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpfalse_osph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpfalse_osph	k5, zmm6, [edx+1016]{1to32}
	vcmpfalse_osph	k5, zmm6, [edx+1024]{1to32}
	vcmpfalse_osph	k5, zmm6, [edx-1024]{1to32}
	vcmpfalse_osph	k5, zmm6, [edx-1032]{1to32}
	vcmpneq_osph	k5, zmm6, zmm5
	vcmpneq_osph	k5{k7}, zmm6, zmm5
	vcmpneq_osph	k5, zmm6, zmm5, {sae}
	vcmpneq_osph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpneq_osph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpneq_osph	k5, zmm6, [eax]{1to32}
	vcmpneq_osph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpneq_osph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpneq_osph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpneq_osph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpneq_osph	k5, zmm6, [edx+1016]{1to32}
	vcmpneq_osph	k5, zmm6, [edx+1024]{1to32}
	vcmpneq_osph	k5, zmm6, [edx-1024]{1to32}
	vcmpneq_osph	k5, zmm6, [edx-1032]{1to32}
	vcmpge_oqph	k5, zmm6, zmm5
	vcmpge_oqph	k5{k7}, zmm6, zmm5
	vcmpge_oqph	k5, zmm6, zmm5, {sae}
	vcmpge_oqph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpge_oqph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpge_oqph	k5, zmm6, [eax]{1to32}
	vcmpge_oqph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpge_oqph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpge_oqph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpge_oqph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpge_oqph	k5, zmm6, [edx+1016]{1to32}
	vcmpge_oqph	k5, zmm6, [edx+1024]{1to32}
	vcmpge_oqph	k5, zmm6, [edx-1024]{1to32}
	vcmpge_oqph	k5, zmm6, [edx-1032]{1to32}
	vcmpgt_oqph	k5, zmm6, zmm5
	vcmpgt_oqph	k5{k7}, zmm6, zmm5
	vcmpgt_oqph	k5, zmm6, zmm5, {sae}
	vcmpgt_oqph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmpgt_oqph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmpgt_oqph	k5, zmm6, [eax]{1to32}
	vcmpgt_oqph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmpgt_oqph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmpgt_oqph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmpgt_oqph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmpgt_oqph	k5, zmm6, [edx+1016]{1to32}
	vcmpgt_oqph	k5, zmm6, [edx+1024]{1to32}
	vcmpgt_oqph	k5, zmm6, [edx-1024]{1to32}
	vcmpgt_oqph	k5, zmm6, [edx-1032]{1to32}
	vcmptrue_usph	k5, zmm6, zmm5
	vcmptrue_usph	k5{k7}, zmm6, zmm5
	vcmptrue_usph	k5, zmm6, zmm5, {sae}
	vcmptrue_usph	k5, zmm6, ZMMWORD PTR [ecx]
	vcmptrue_usph	k5, zmm6, ZMMWORD PTR [esp+esi*8-123456]
	vcmptrue_usph	k5, zmm6, [eax]{1to32}
	vcmptrue_usph	k5, zmm6, ZMMWORD PTR [edx+8128]
	vcmptrue_usph	k5, zmm6, ZMMWORD PTR [edx+8192]
	vcmptrue_usph	k5, zmm6, ZMMWORD PTR [edx-8192]
	vcmptrue_usph	k5, zmm6, ZMMWORD PTR [edx-8256]
	vcmptrue_usph	k5, zmm6, [edx+1016]{1to32}
	vcmptrue_usph	k5, zmm6, [edx+1024]{1to32}
	vcmptrue_usph	k5, zmm6, [edx-1024]{1to32}
	vcmptrue_usph	k5, zmm6, [edx-1032]{1to32}
	vcmpeq_oqsh	k5{k7}, xmm5, xmm4
	vcmpeq_oqsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpeq_oqsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpeq_oqsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpeq_oqsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpeq_oqsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpeq_oqsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpeq_oqsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpeqsh	k5{k7}, xmm5, xmm4
	vcmpeqsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpeqsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpeqsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpeqsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpeqsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpeqsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpeqsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmplt_ossh	k5{k7}, xmm5, xmm4
	vcmplt_ossh	k5{k7}, xmm5, xmm4, {sae}
	vcmplt_ossh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmplt_ossh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmplt_ossh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmplt_ossh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmplt_ossh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmplt_ossh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpltsh	k5{k7}, xmm5, xmm4
	vcmpltsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpltsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpltsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpltsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpltsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpltsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpltsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmple_ossh	k5{k7}, xmm5, xmm4
	vcmple_ossh	k5{k7}, xmm5, xmm4, {sae}
	vcmple_ossh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmple_ossh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmple_ossh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmple_ossh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmple_ossh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmple_ossh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmplesh	k5{k7}, xmm5, xmm4
	vcmplesh	k5{k7}, xmm5, xmm4, {sae}
	vcmplesh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmplesh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmplesh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmplesh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmplesh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmplesh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpunord_qsh	k5{k7}, xmm5, xmm4
	vcmpunord_qsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpunord_qsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpunord_qsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpunord_qsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpunord_qsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpunord_qsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpunord_qsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpunordsh	k5{k7}, xmm5, xmm4
	vcmpunordsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpunordsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpunordsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpunordsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpunordsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpunordsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpunordsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpneq_uqsh	k5{k7}, xmm5, xmm4
	vcmpneq_uqsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpneq_uqsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpneq_uqsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpneq_uqsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpneq_uqsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpneq_uqsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpneq_uqsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpneqsh	k5{k7}, xmm5, xmm4
	vcmpneqsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpneqsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpneqsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpneqsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpneqsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpneqsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpneqsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpnlt_ussh	k5{k7}, xmm5, xmm4
	vcmpnlt_ussh	k5{k7}, xmm5, xmm4, {sae}
	vcmpnlt_ussh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpnlt_ussh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpnlt_ussh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpnlt_ussh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpnlt_ussh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpnlt_ussh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpnltsh	k5{k7}, xmm5, xmm4
	vcmpnltsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpnltsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpnltsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpnltsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpnltsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpnltsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpnltsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpnle_ussh	k5{k7}, xmm5, xmm4
	vcmpnle_ussh	k5{k7}, xmm5, xmm4, {sae}
	vcmpnle_ussh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpnle_ussh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpnle_ussh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpnle_ussh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpnle_ussh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpnle_ussh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpnlesh	k5{k7}, xmm5, xmm4
	vcmpnlesh	k5{k7}, xmm5, xmm4, {sae}
	vcmpnlesh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpnlesh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpnlesh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpnlesh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpnlesh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpnlesh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpord_qsh	k5{k7}, xmm5, xmm4
	vcmpord_qsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpord_qsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpord_qsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpord_qsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpord_qsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpord_qsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpord_qsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpordsh	k5{k7}, xmm5, xmm4
	vcmpordsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpordsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpordsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpordsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpordsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpordsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpordsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpeq_uqsh	k5{k7}, xmm5, xmm4
	vcmpeq_uqsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpeq_uqsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpeq_uqsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpeq_uqsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpeq_uqsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpeq_uqsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpeq_uqsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpnge_ussh	k5{k7}, xmm5, xmm4
	vcmpnge_ussh	k5{k7}, xmm5, xmm4, {sae}
	vcmpnge_ussh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpnge_ussh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpnge_ussh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpnge_ussh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpnge_ussh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpnge_ussh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpngesh	k5{k7}, xmm5, xmm4
	vcmpngesh	k5{k7}, xmm5, xmm4, {sae}
	vcmpngesh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpngesh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpngesh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpngesh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpngesh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpngesh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpngt_ussh	k5{k7}, xmm5, xmm4
	vcmpngt_ussh	k5{k7}, xmm5, xmm4, {sae}
	vcmpngt_ussh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpngt_ussh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpngt_ussh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpngt_ussh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpngt_ussh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpngt_ussh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpngtsh	k5{k7}, xmm5, xmm4
	vcmpngtsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpngtsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpngtsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpngtsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpngtsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpngtsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpngtsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpfalse_oqsh	k5{k7}, xmm5, xmm4
	vcmpfalse_oqsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpfalse_oqsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpfalse_oqsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpfalse_oqsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpfalse_oqsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpfalse_oqsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpfalse_oqsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpfalsesh	k5{k7}, xmm5, xmm4
	vcmpfalsesh	k5{k7}, xmm5, xmm4, {sae}
	vcmpfalsesh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpfalsesh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpfalsesh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpfalsesh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpfalsesh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpfalsesh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpneq_oqsh	k5{k7}, xmm5, xmm4
	vcmpneq_oqsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpneq_oqsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpneq_oqsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpneq_oqsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpneq_oqsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpneq_oqsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpneq_oqsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpge_ossh	k5{k7}, xmm5, xmm4
	vcmpge_ossh	k5{k7}, xmm5, xmm4, {sae}
	vcmpge_ossh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpge_ossh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpge_ossh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpge_ossh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpge_ossh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpge_ossh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpgesh	k5{k7}, xmm5, xmm4
	vcmpgesh	k5{k7}, xmm5, xmm4, {sae}
	vcmpgesh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpgesh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpgesh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpgesh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpgesh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpgesh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpgt_ossh	k5{k7}, xmm5, xmm4
	vcmpgt_ossh	k5{k7}, xmm5, xmm4, {sae}
	vcmpgt_ossh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpgt_ossh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpgt_ossh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpgt_ossh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpgt_ossh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpgt_ossh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpgtsh	k5{k7}, xmm5, xmm4
	vcmpgtsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpgtsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpgtsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpgtsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpgtsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpgtsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpgtsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmptrue_uqsh	k5{k7}, xmm5, xmm4
	vcmptrue_uqsh	k5{k7}, xmm5, xmm4, {sae}
	vcmptrue_uqsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmptrue_uqsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmptrue_uqsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmptrue_uqsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmptrue_uqsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmptrue_uqsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmptruesh	k5{k7}, xmm5, xmm4
	vcmptruesh	k5{k7}, xmm5, xmm4, {sae}
	vcmptruesh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmptruesh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmptruesh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmptruesh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmptruesh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmptruesh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpeq_ossh	k5{k7}, xmm5, xmm4
	vcmpeq_ossh	k5{k7}, xmm5, xmm4, {sae}
	vcmpeq_ossh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpeq_ossh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpeq_ossh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpeq_ossh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpeq_ossh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpeq_ossh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmplt_oqsh	k5{k7}, xmm5, xmm4
	vcmplt_oqsh	k5{k7}, xmm5, xmm4, {sae}
	vcmplt_oqsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmplt_oqsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmplt_oqsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmplt_oqsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmplt_oqsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmplt_oqsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmple_oqsh	k5{k7}, xmm5, xmm4
	vcmple_oqsh	k5{k7}, xmm5, xmm4, {sae}
	vcmple_oqsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmple_oqsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmple_oqsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmple_oqsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmple_oqsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmple_oqsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpunord_ssh	k5{k7}, xmm5, xmm4
	vcmpunord_ssh	k5{k7}, xmm5, xmm4, {sae}
	vcmpunord_ssh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpunord_ssh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpunord_ssh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpunord_ssh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpunord_ssh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpunord_ssh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpneq_ussh	k5{k7}, xmm5, xmm4
	vcmpneq_ussh	k5{k7}, xmm5, xmm4, {sae}
	vcmpneq_ussh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpneq_ussh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpneq_ussh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpneq_ussh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpneq_ussh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpneq_ussh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpnlt_uqsh	k5{k7}, xmm5, xmm4
	vcmpnlt_uqsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpnlt_uqsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpnlt_uqsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpnlt_uqsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpnlt_uqsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpnlt_uqsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpnlt_uqsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpnle_uqsh	k5{k7}, xmm5, xmm4
	vcmpnle_uqsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpnle_uqsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpnle_uqsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpnle_uqsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpnle_uqsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpnle_uqsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpnle_uqsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpord_ssh	k5{k7}, xmm5, xmm4
	vcmpord_ssh	k5{k7}, xmm5, xmm4, {sae}
	vcmpord_ssh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpord_ssh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpord_ssh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpord_ssh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpord_ssh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpord_ssh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpeq_ussh	k5{k7}, xmm5, xmm4
	vcmpeq_ussh	k5{k7}, xmm5, xmm4, {sae}
	vcmpeq_ussh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpeq_ussh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpeq_ussh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpeq_ussh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpeq_ussh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpeq_ussh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpnge_uqsh	k5{k7}, xmm5, xmm4
	vcmpnge_uqsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpnge_uqsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpnge_uqsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpnge_uqsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpnge_uqsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpnge_uqsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpnge_uqsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpngt_uqsh	k5{k7}, xmm5, xmm4
	vcmpngt_uqsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpngt_uqsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpngt_uqsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpngt_uqsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpngt_uqsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpngt_uqsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpngt_uqsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpfalse_ossh	k5{k7}, xmm5, xmm4
	vcmpfalse_ossh	k5{k7}, xmm5, xmm4, {sae}
	vcmpfalse_ossh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpfalse_ossh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpfalse_ossh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpfalse_ossh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpfalse_ossh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpfalse_ossh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpneq_ossh	k5{k7}, xmm5, xmm4
	vcmpneq_ossh	k5{k7}, xmm5, xmm4, {sae}
	vcmpneq_ossh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpneq_ossh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpneq_ossh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpneq_ossh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpneq_ossh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpneq_ossh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpge_oqsh	k5{k7}, xmm5, xmm4
	vcmpge_oqsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpge_oqsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpge_oqsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpge_oqsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpge_oqsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpge_oqsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpge_oqsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmpgt_oqsh	k5{k7}, xmm5, xmm4
	vcmpgt_oqsh	k5{k7}, xmm5, xmm4, {sae}
	vcmpgt_oqsh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmpgt_oqsh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmpgt_oqsh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmpgt_oqsh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmpgt_oqsh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmpgt_oqsh	k5{k7}, xmm5, WORD PTR [edx-1032]
	vcmptrue_ussh	k5{k7}, xmm5, xmm4
	vcmptrue_ussh	k5{k7}, xmm5, xmm4, {sae}
	vcmptrue_ussh	k5{k7}, xmm5, WORD PTR [ecx]
	vcmptrue_ussh	k5{k7}, xmm5, WORD PTR [esp+esi*8-123456]
	vcmptrue_ussh	k5{k7}, xmm5, WORD PTR [edx+1016]
	vcmptrue_ussh	k5{k7}, xmm5, WORD PTR [edx+1024]
	vcmptrue_ussh	k5{k7}, xmm5, WORD PTR [edx-1024]
	vcmptrue_ussh	k5{k7}, xmm5, WORD PTR [edx-1032]
