# Check 64bit VCM.*{PH,SH} instructions

        .allow_index_reg
        .text
_start:
	vcmpeq_oqph	%zmm29, %zmm30, %k5
	vcmpeq_oqph	%zmm29, %zmm30, %k5{%k7}
	vcmpeq_oqph	{sae}, %zmm29, %zmm30, %k5
	vcmpeq_oqph	(%rcx), %zmm30, %k5
	vcmpeq_oqph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpeq_oqph	(%rcx){1to32}, %zmm30, %k5
	vcmpeq_oqph	8128(%rdx), %zmm30, %k5
	vcmpeq_oqph	8192(%rdx), %zmm30, %k5
	vcmpeq_oqph	-8192(%rdx), %zmm30, %k5
	vcmpeq_oqph	-8256(%rdx), %zmm30, %k5
	vcmpeq_oqph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpeq_oqph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpeq_oqph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpeq_oqph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpeqph	%zmm29, %zmm30, %k5
	vcmpeqph	%zmm29, %zmm30, %k5{%k7}
	vcmpeqph	{sae}, %zmm29, %zmm30, %k5
	vcmpeqph	(%rcx), %zmm30, %k5
	vcmpeqph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpeqph	(%rcx){1to32}, %zmm30, %k5
	vcmpeqph	8128(%rdx), %zmm30, %k5
	vcmpeqph	8192(%rdx), %zmm30, %k5
	vcmpeqph	-8192(%rdx), %zmm30, %k5
	vcmpeqph	-8256(%rdx), %zmm30, %k5
	vcmpeqph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpeqph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpeqph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpeqph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmplt_osph	%zmm29, %zmm30, %k5
	vcmplt_osph	%zmm29, %zmm30, %k5{%k7}
	vcmplt_osph	{sae}, %zmm29, %zmm30, %k5
	vcmplt_osph	(%rcx), %zmm30, %k5
	vcmplt_osph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmplt_osph	(%rcx){1to32}, %zmm30, %k5
	vcmplt_osph	8128(%rdx), %zmm30, %k5
	vcmplt_osph	8192(%rdx), %zmm30, %k5
	vcmplt_osph	-8192(%rdx), %zmm30, %k5
	vcmplt_osph	-8256(%rdx), %zmm30, %k5
	vcmplt_osph	1016(%rdx){1to32}, %zmm30, %k5
	vcmplt_osph	1024(%rdx){1to32}, %zmm30, %k5
	vcmplt_osph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmplt_osph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpltph	%zmm29, %zmm30, %k5
	vcmpltph	%zmm29, %zmm30, %k5{%k7}
	vcmpltph	{sae}, %zmm29, %zmm30, %k5
	vcmpltph	(%rcx), %zmm30, %k5
	vcmpltph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpltph	(%rcx){1to32}, %zmm30, %k5
	vcmpltph	8128(%rdx), %zmm30, %k5
	vcmpltph	8192(%rdx), %zmm30, %k5
	vcmpltph	-8192(%rdx), %zmm30, %k5
	vcmpltph	-8256(%rdx), %zmm30, %k5
	vcmpltph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpltph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpltph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpltph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmple_osph	%zmm29, %zmm30, %k5
	vcmple_osph	%zmm29, %zmm30, %k5{%k7}
	vcmple_osph	{sae}, %zmm29, %zmm30, %k5
	vcmple_osph	(%rcx), %zmm30, %k5
	vcmple_osph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmple_osph	(%rcx){1to32}, %zmm30, %k5
	vcmple_osph	8128(%rdx), %zmm30, %k5
	vcmple_osph	8192(%rdx), %zmm30, %k5
	vcmple_osph	-8192(%rdx), %zmm30, %k5
	vcmple_osph	-8256(%rdx), %zmm30, %k5
	vcmple_osph	1016(%rdx){1to32}, %zmm30, %k5
	vcmple_osph	1024(%rdx){1to32}, %zmm30, %k5
	vcmple_osph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmple_osph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpleph	%zmm29, %zmm30, %k5
	vcmpleph	%zmm29, %zmm30, %k5{%k7}
	vcmpleph	{sae}, %zmm29, %zmm30, %k5
	vcmpleph	(%rcx), %zmm30, %k5
	vcmpleph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpleph	(%rcx){1to32}, %zmm30, %k5
	vcmpleph	8128(%rdx), %zmm30, %k5
	vcmpleph	8192(%rdx), %zmm30, %k5
	vcmpleph	-8192(%rdx), %zmm30, %k5
	vcmpleph	-8256(%rdx), %zmm30, %k5
	vcmpleph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpleph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpleph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpleph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpunord_qph	%zmm29, %zmm30, %k5
	vcmpunord_qph	%zmm29, %zmm30, %k5{%k7}
	vcmpunord_qph	{sae}, %zmm29, %zmm30, %k5
	vcmpunord_qph	(%rcx), %zmm30, %k5
	vcmpunord_qph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpunord_qph	(%rcx){1to32}, %zmm30, %k5
	vcmpunord_qph	8128(%rdx), %zmm30, %k5
	vcmpunord_qph	8192(%rdx), %zmm30, %k5
	vcmpunord_qph	-8192(%rdx), %zmm30, %k5
	vcmpunord_qph	-8256(%rdx), %zmm30, %k5
	vcmpunord_qph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpunord_qph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpunord_qph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpunord_qph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpunordph	%zmm29, %zmm30, %k5
	vcmpunordph	%zmm29, %zmm30, %k5{%k7}
	vcmpunordph	{sae}, %zmm29, %zmm30, %k5
	vcmpunordph	(%rcx), %zmm30, %k5
	vcmpunordph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpunordph	(%rcx){1to32}, %zmm30, %k5
	vcmpunordph	8128(%rdx), %zmm30, %k5
	vcmpunordph	8192(%rdx), %zmm30, %k5
	vcmpunordph	-8192(%rdx), %zmm30, %k5
	vcmpunordph	-8256(%rdx), %zmm30, %k5
	vcmpunordph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpunordph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpunordph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpunordph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpneq_uqph	%zmm29, %zmm30, %k5
	vcmpneq_uqph	%zmm29, %zmm30, %k5{%k7}
	vcmpneq_uqph	{sae}, %zmm29, %zmm30, %k5
	vcmpneq_uqph	(%rcx), %zmm30, %k5
	vcmpneq_uqph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpneq_uqph	(%rcx){1to32}, %zmm30, %k5
	vcmpneq_uqph	8128(%rdx), %zmm30, %k5
	vcmpneq_uqph	8192(%rdx), %zmm30, %k5
	vcmpneq_uqph	-8192(%rdx), %zmm30, %k5
	vcmpneq_uqph	-8256(%rdx), %zmm30, %k5
	vcmpneq_uqph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpneq_uqph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpneq_uqph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpneq_uqph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpneqph	%zmm29, %zmm30, %k5
	vcmpneqph	%zmm29, %zmm30, %k5{%k7}
	vcmpneqph	{sae}, %zmm29, %zmm30, %k5
	vcmpneqph	(%rcx), %zmm30, %k5
	vcmpneqph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpneqph	(%rcx){1to32}, %zmm30, %k5
	vcmpneqph	8128(%rdx), %zmm30, %k5
	vcmpneqph	8192(%rdx), %zmm30, %k5
	vcmpneqph	-8192(%rdx), %zmm30, %k5
	vcmpneqph	-8256(%rdx), %zmm30, %k5
	vcmpneqph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpneqph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpneqph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpneqph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpnlt_usph	%zmm29, %zmm30, %k5
	vcmpnlt_usph	%zmm29, %zmm30, %k5{%k7}
	vcmpnlt_usph	{sae}, %zmm29, %zmm30, %k5
	vcmpnlt_usph	(%rcx), %zmm30, %k5
	vcmpnlt_usph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpnlt_usph	(%rcx){1to32}, %zmm30, %k5
	vcmpnlt_usph	8128(%rdx), %zmm30, %k5
	vcmpnlt_usph	8192(%rdx), %zmm30, %k5
	vcmpnlt_usph	-8192(%rdx), %zmm30, %k5
	vcmpnlt_usph	-8256(%rdx), %zmm30, %k5
	vcmpnlt_usph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpnlt_usph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpnlt_usph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpnlt_usph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpnltph	%zmm29, %zmm30, %k5
	vcmpnltph	%zmm29, %zmm30, %k5{%k7}
	vcmpnltph	{sae}, %zmm29, %zmm30, %k5
	vcmpnltph	(%rcx), %zmm30, %k5
	vcmpnltph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpnltph	(%rcx){1to32}, %zmm30, %k5
	vcmpnltph	8128(%rdx), %zmm30, %k5
	vcmpnltph	8192(%rdx), %zmm30, %k5
	vcmpnltph	-8192(%rdx), %zmm30, %k5
	vcmpnltph	-8256(%rdx), %zmm30, %k5
	vcmpnltph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpnltph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpnltph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpnltph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpnle_usph	%zmm29, %zmm30, %k5
	vcmpnle_usph	%zmm29, %zmm30, %k5{%k7}
	vcmpnle_usph	{sae}, %zmm29, %zmm30, %k5
	vcmpnle_usph	(%rcx), %zmm30, %k5
	vcmpnle_usph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpnle_usph	(%rcx){1to32}, %zmm30, %k5
	vcmpnle_usph	8128(%rdx), %zmm30, %k5
	vcmpnle_usph	8192(%rdx), %zmm30, %k5
	vcmpnle_usph	-8192(%rdx), %zmm30, %k5
	vcmpnle_usph	-8256(%rdx), %zmm30, %k5
	vcmpnle_usph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpnle_usph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpnle_usph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpnle_usph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpnleph	%zmm29, %zmm30, %k5
	vcmpnleph	%zmm29, %zmm30, %k5{%k7}
	vcmpnleph	{sae}, %zmm29, %zmm30, %k5
	vcmpnleph	(%rcx), %zmm30, %k5
	vcmpnleph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpnleph	(%rcx){1to32}, %zmm30, %k5
	vcmpnleph	8128(%rdx), %zmm30, %k5
	vcmpnleph	8192(%rdx), %zmm30, %k5
	vcmpnleph	-8192(%rdx), %zmm30, %k5
	vcmpnleph	-8256(%rdx), %zmm30, %k5
	vcmpnleph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpnleph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpnleph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpnleph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpord_qph	%zmm29, %zmm30, %k5
	vcmpord_qph	%zmm29, %zmm30, %k5{%k7}
	vcmpord_qph	{sae}, %zmm29, %zmm30, %k5
	vcmpord_qph	(%rcx), %zmm30, %k5
	vcmpord_qph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpord_qph	(%rcx){1to32}, %zmm30, %k5
	vcmpord_qph	8128(%rdx), %zmm30, %k5
	vcmpord_qph	8192(%rdx), %zmm30, %k5
	vcmpord_qph	-8192(%rdx), %zmm30, %k5
	vcmpord_qph	-8256(%rdx), %zmm30, %k5
	vcmpord_qph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpord_qph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpord_qph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpord_qph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpordph	%zmm29, %zmm30, %k5
	vcmpordph	%zmm29, %zmm30, %k5{%k7}
	vcmpordph	{sae}, %zmm29, %zmm30, %k5
	vcmpordph	(%rcx), %zmm30, %k5
	vcmpordph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpordph	(%rcx){1to32}, %zmm30, %k5
	vcmpordph	8128(%rdx), %zmm30, %k5
	vcmpordph	8192(%rdx), %zmm30, %k5
	vcmpordph	-8192(%rdx), %zmm30, %k5
	vcmpordph	-8256(%rdx), %zmm30, %k5
	vcmpordph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpordph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpordph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpordph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpeq_uqph	%zmm29, %zmm30, %k5
	vcmpeq_uqph	%zmm29, %zmm30, %k5{%k7}
	vcmpeq_uqph	{sae}, %zmm29, %zmm30, %k5
	vcmpeq_uqph	(%rcx), %zmm30, %k5
	vcmpeq_uqph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpeq_uqph	(%rcx){1to32}, %zmm30, %k5
	vcmpeq_uqph	8128(%rdx), %zmm30, %k5
	vcmpeq_uqph	8192(%rdx), %zmm30, %k5
	vcmpeq_uqph	-8192(%rdx), %zmm30, %k5
	vcmpeq_uqph	-8256(%rdx), %zmm30, %k5
	vcmpeq_uqph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpeq_uqph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpeq_uqph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpeq_uqph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpnge_usph	%zmm29, %zmm30, %k5
	vcmpnge_usph	%zmm29, %zmm30, %k5{%k7}
	vcmpnge_usph	{sae}, %zmm29, %zmm30, %k5
	vcmpnge_usph	(%rcx), %zmm30, %k5
	vcmpnge_usph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpnge_usph	(%rcx){1to32}, %zmm30, %k5
	vcmpnge_usph	8128(%rdx), %zmm30, %k5
	vcmpnge_usph	8192(%rdx), %zmm30, %k5
	vcmpnge_usph	-8192(%rdx), %zmm30, %k5
	vcmpnge_usph	-8256(%rdx), %zmm30, %k5
	vcmpnge_usph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpnge_usph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpnge_usph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpnge_usph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpngeph	%zmm29, %zmm30, %k5
	vcmpngeph	%zmm29, %zmm30, %k5{%k7}
	vcmpngeph	{sae}, %zmm29, %zmm30, %k5
	vcmpngeph	(%rcx), %zmm30, %k5
	vcmpngeph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpngeph	(%rcx){1to32}, %zmm30, %k5
	vcmpngeph	8128(%rdx), %zmm30, %k5
	vcmpngeph	8192(%rdx), %zmm30, %k5
	vcmpngeph	-8192(%rdx), %zmm30, %k5
	vcmpngeph	-8256(%rdx), %zmm30, %k5
	vcmpngeph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpngeph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpngeph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpngeph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpngt_usph	%zmm29, %zmm30, %k5
	vcmpngt_usph	%zmm29, %zmm30, %k5{%k7}
	vcmpngt_usph	{sae}, %zmm29, %zmm30, %k5
	vcmpngt_usph	(%rcx), %zmm30, %k5
	vcmpngt_usph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpngt_usph	(%rcx){1to32}, %zmm30, %k5
	vcmpngt_usph	8128(%rdx), %zmm30, %k5
	vcmpngt_usph	8192(%rdx), %zmm30, %k5
	vcmpngt_usph	-8192(%rdx), %zmm30, %k5
	vcmpngt_usph	-8256(%rdx), %zmm30, %k5
	vcmpngt_usph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpngt_usph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpngt_usph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpngt_usph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpngtph	%zmm29, %zmm30, %k5
	vcmpngtph	%zmm29, %zmm30, %k5{%k7}
	vcmpngtph	{sae}, %zmm29, %zmm30, %k5
	vcmpngtph	(%rcx), %zmm30, %k5
	vcmpngtph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpngtph	(%rcx){1to32}, %zmm30, %k5
	vcmpngtph	8128(%rdx), %zmm30, %k5
	vcmpngtph	8192(%rdx), %zmm30, %k5
	vcmpngtph	-8192(%rdx), %zmm30, %k5
	vcmpngtph	-8256(%rdx), %zmm30, %k5
	vcmpngtph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpngtph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpngtph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpngtph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpfalse_oqph	%zmm29, %zmm30, %k5
	vcmpfalse_oqph	%zmm29, %zmm30, %k5{%k7}
	vcmpfalse_oqph	{sae}, %zmm29, %zmm30, %k5
	vcmpfalse_oqph	(%rcx), %zmm30, %k5
	vcmpfalse_oqph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpfalse_oqph	(%rcx){1to32}, %zmm30, %k5
	vcmpfalse_oqph	8128(%rdx), %zmm30, %k5
	vcmpfalse_oqph	8192(%rdx), %zmm30, %k5
	vcmpfalse_oqph	-8192(%rdx), %zmm30, %k5
	vcmpfalse_oqph	-8256(%rdx), %zmm30, %k5
	vcmpfalse_oqph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpfalse_oqph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpfalse_oqph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpfalse_oqph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpfalseph	%zmm29, %zmm30, %k5
	vcmpfalseph	%zmm29, %zmm30, %k5{%k7}
	vcmpfalseph	{sae}, %zmm29, %zmm30, %k5
	vcmpfalseph	(%rcx), %zmm30, %k5
	vcmpfalseph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpfalseph	(%rcx){1to32}, %zmm30, %k5
	vcmpfalseph	8128(%rdx), %zmm30, %k5
	vcmpfalseph	8192(%rdx), %zmm30, %k5
	vcmpfalseph	-8192(%rdx), %zmm30, %k5
	vcmpfalseph	-8256(%rdx), %zmm30, %k5
	vcmpfalseph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpfalseph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpfalseph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpfalseph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpneq_oqph	%zmm29, %zmm30, %k5
	vcmpneq_oqph	%zmm29, %zmm30, %k5{%k7}
	vcmpneq_oqph	{sae}, %zmm29, %zmm30, %k5
	vcmpneq_oqph	(%rcx), %zmm30, %k5
	vcmpneq_oqph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpneq_oqph	(%rcx){1to32}, %zmm30, %k5
	vcmpneq_oqph	8128(%rdx), %zmm30, %k5
	vcmpneq_oqph	8192(%rdx), %zmm30, %k5
	vcmpneq_oqph	-8192(%rdx), %zmm30, %k5
	vcmpneq_oqph	-8256(%rdx), %zmm30, %k5
	vcmpneq_oqph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpneq_oqph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpneq_oqph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpneq_oqph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpge_osph	%zmm29, %zmm30, %k5
	vcmpge_osph	%zmm29, %zmm30, %k5{%k7}
	vcmpge_osph	{sae}, %zmm29, %zmm30, %k5
	vcmpge_osph	(%rcx), %zmm30, %k5
	vcmpge_osph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpge_osph	(%rcx){1to32}, %zmm30, %k5
	vcmpge_osph	8128(%rdx), %zmm30, %k5
	vcmpge_osph	8192(%rdx), %zmm30, %k5
	vcmpge_osph	-8192(%rdx), %zmm30, %k5
	vcmpge_osph	-8256(%rdx), %zmm30, %k5
	vcmpge_osph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpge_osph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpge_osph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpge_osph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpgeph	%zmm29, %zmm30, %k5
	vcmpgeph	%zmm29, %zmm30, %k5{%k7}
	vcmpgeph	{sae}, %zmm29, %zmm30, %k5
	vcmpgeph	(%rcx), %zmm30, %k5
	vcmpgeph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpgeph	(%rcx){1to32}, %zmm30, %k5
	vcmpgeph	8128(%rdx), %zmm30, %k5
	vcmpgeph	8192(%rdx), %zmm30, %k5
	vcmpgeph	-8192(%rdx), %zmm30, %k5
	vcmpgeph	-8256(%rdx), %zmm30, %k5
	vcmpgeph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpgeph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpgeph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpgeph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpgt_osph	%zmm29, %zmm30, %k5
	vcmpgt_osph	%zmm29, %zmm30, %k5{%k7}
	vcmpgt_osph	{sae}, %zmm29, %zmm30, %k5
	vcmpgt_osph	(%rcx), %zmm30, %k5
	vcmpgt_osph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpgt_osph	(%rcx){1to32}, %zmm30, %k5
	vcmpgt_osph	8128(%rdx), %zmm30, %k5
	vcmpgt_osph	8192(%rdx), %zmm30, %k5
	vcmpgt_osph	-8192(%rdx), %zmm30, %k5
	vcmpgt_osph	-8256(%rdx), %zmm30, %k5
	vcmpgt_osph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpgt_osph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpgt_osph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpgt_osph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpgtph	%zmm29, %zmm30, %k5
	vcmpgtph	%zmm29, %zmm30, %k5{%k7}
	vcmpgtph	{sae}, %zmm29, %zmm30, %k5
	vcmpgtph	(%rcx), %zmm30, %k5
	vcmpgtph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpgtph	(%rcx){1to32}, %zmm30, %k5
	vcmpgtph	8128(%rdx), %zmm30, %k5
	vcmpgtph	8192(%rdx), %zmm30, %k5
	vcmpgtph	-8192(%rdx), %zmm30, %k5
	vcmpgtph	-8256(%rdx), %zmm30, %k5
	vcmpgtph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpgtph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpgtph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpgtph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmptrue_uqph	%zmm29, %zmm30, %k5
	vcmptrue_uqph	%zmm29, %zmm30, %k5{%k7}
	vcmptrue_uqph	{sae}, %zmm29, %zmm30, %k5
	vcmptrue_uqph	(%rcx), %zmm30, %k5
	vcmptrue_uqph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmptrue_uqph	(%rcx){1to32}, %zmm30, %k5
	vcmptrue_uqph	8128(%rdx), %zmm30, %k5
	vcmptrue_uqph	8192(%rdx), %zmm30, %k5
	vcmptrue_uqph	-8192(%rdx), %zmm30, %k5
	vcmptrue_uqph	-8256(%rdx), %zmm30, %k5
	vcmptrue_uqph	1016(%rdx){1to32}, %zmm30, %k5
	vcmptrue_uqph	1024(%rdx){1to32}, %zmm30, %k5
	vcmptrue_uqph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmptrue_uqph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmptrueph	%zmm29, %zmm30, %k5
	vcmptrueph	%zmm29, %zmm30, %k5{%k7}
	vcmptrueph	{sae}, %zmm29, %zmm30, %k5
	vcmptrueph	(%rcx), %zmm30, %k5
	vcmptrueph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmptrueph	(%rcx){1to32}, %zmm30, %k5
	vcmptrueph	8128(%rdx), %zmm30, %k5
	vcmptrueph	8192(%rdx), %zmm30, %k5
	vcmptrueph	-8192(%rdx), %zmm30, %k5
	vcmptrueph	-8256(%rdx), %zmm30, %k5
	vcmptrueph	1016(%rdx){1to32}, %zmm30, %k5
	vcmptrueph	1024(%rdx){1to32}, %zmm30, %k5
	vcmptrueph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmptrueph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpeq_osph	%zmm29, %zmm30, %k5
	vcmpeq_osph	%zmm29, %zmm30, %k5{%k7}
	vcmpeq_osph	{sae}, %zmm29, %zmm30, %k5
	vcmpeq_osph	(%rcx), %zmm30, %k5
	vcmpeq_osph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpeq_osph	(%rcx){1to32}, %zmm30, %k5
	vcmpeq_osph	8128(%rdx), %zmm30, %k5
	vcmpeq_osph	8192(%rdx), %zmm30, %k5
	vcmpeq_osph	-8192(%rdx), %zmm30, %k5
	vcmpeq_osph	-8256(%rdx), %zmm30, %k5
	vcmpeq_osph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpeq_osph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpeq_osph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpeq_osph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmplt_oqph	%zmm29, %zmm30, %k5
	vcmplt_oqph	%zmm29, %zmm30, %k5{%k7}
	vcmplt_oqph	{sae}, %zmm29, %zmm30, %k5
	vcmplt_oqph	(%rcx), %zmm30, %k5
	vcmplt_oqph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmplt_oqph	(%rcx){1to32}, %zmm30, %k5
	vcmplt_oqph	8128(%rdx), %zmm30, %k5
	vcmplt_oqph	8192(%rdx), %zmm30, %k5
	vcmplt_oqph	-8192(%rdx), %zmm30, %k5
	vcmplt_oqph	-8256(%rdx), %zmm30, %k5
	vcmplt_oqph	1016(%rdx){1to32}, %zmm30, %k5
	vcmplt_oqph	1024(%rdx){1to32}, %zmm30, %k5
	vcmplt_oqph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmplt_oqph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmple_oqph	%zmm29, %zmm30, %k5
	vcmple_oqph	%zmm29, %zmm30, %k5{%k7}
	vcmple_oqph	{sae}, %zmm29, %zmm30, %k5
	vcmple_oqph	(%rcx), %zmm30, %k5
	vcmple_oqph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmple_oqph	(%rcx){1to32}, %zmm30, %k5
	vcmple_oqph	8128(%rdx), %zmm30, %k5
	vcmple_oqph	8192(%rdx), %zmm30, %k5
	vcmple_oqph	-8192(%rdx), %zmm30, %k5
	vcmple_oqph	-8256(%rdx), %zmm30, %k5
	vcmple_oqph	1016(%rdx){1to32}, %zmm30, %k5
	vcmple_oqph	1024(%rdx){1to32}, %zmm30, %k5
	vcmple_oqph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmple_oqph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpunord_sph	%zmm29, %zmm30, %k5
	vcmpunord_sph	%zmm29, %zmm30, %k5{%k7}
	vcmpunord_sph	{sae}, %zmm29, %zmm30, %k5
	vcmpunord_sph	(%rcx), %zmm30, %k5
	vcmpunord_sph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpunord_sph	(%rcx){1to32}, %zmm30, %k5
	vcmpunord_sph	8128(%rdx), %zmm30, %k5
	vcmpunord_sph	8192(%rdx), %zmm30, %k5
	vcmpunord_sph	-8192(%rdx), %zmm30, %k5
	vcmpunord_sph	-8256(%rdx), %zmm30, %k5
	vcmpunord_sph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpunord_sph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpunord_sph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpunord_sph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpneq_usph	%zmm29, %zmm30, %k5
	vcmpneq_usph	%zmm29, %zmm30, %k5{%k7}
	vcmpneq_usph	{sae}, %zmm29, %zmm30, %k5
	vcmpneq_usph	(%rcx), %zmm30, %k5
	vcmpneq_usph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpneq_usph	(%rcx){1to32}, %zmm30, %k5
	vcmpneq_usph	8128(%rdx), %zmm30, %k5
	vcmpneq_usph	8192(%rdx), %zmm30, %k5
	vcmpneq_usph	-8192(%rdx), %zmm30, %k5
	vcmpneq_usph	-8256(%rdx), %zmm30, %k5
	vcmpneq_usph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpneq_usph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpneq_usph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpneq_usph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpnlt_uqph	%zmm29, %zmm30, %k5
	vcmpnlt_uqph	%zmm29, %zmm30, %k5{%k7}
	vcmpnlt_uqph	{sae}, %zmm29, %zmm30, %k5
	vcmpnlt_uqph	(%rcx), %zmm30, %k5
	vcmpnlt_uqph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpnlt_uqph	(%rcx){1to32}, %zmm30, %k5
	vcmpnlt_uqph	8128(%rdx), %zmm30, %k5
	vcmpnlt_uqph	8192(%rdx), %zmm30, %k5
	vcmpnlt_uqph	-8192(%rdx), %zmm30, %k5
	vcmpnlt_uqph	-8256(%rdx), %zmm30, %k5
	vcmpnlt_uqph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpnlt_uqph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpnlt_uqph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpnlt_uqph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpnle_uqph	%zmm29, %zmm30, %k5
	vcmpnle_uqph	%zmm29, %zmm30, %k5{%k7}
	vcmpnle_uqph	{sae}, %zmm29, %zmm30, %k5
	vcmpnle_uqph	(%rcx), %zmm30, %k5
	vcmpnle_uqph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpnle_uqph	(%rcx){1to32}, %zmm30, %k5
	vcmpnle_uqph	8128(%rdx), %zmm30, %k5
	vcmpnle_uqph	8192(%rdx), %zmm30, %k5
	vcmpnle_uqph	-8192(%rdx), %zmm30, %k5
	vcmpnle_uqph	-8256(%rdx), %zmm30, %k5
	vcmpnle_uqph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpnle_uqph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpnle_uqph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpnle_uqph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpord_sph	%zmm29, %zmm30, %k5
	vcmpord_sph	%zmm29, %zmm30, %k5{%k7}
	vcmpord_sph	{sae}, %zmm29, %zmm30, %k5
	vcmpord_sph	(%rcx), %zmm30, %k5
	vcmpord_sph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpord_sph	(%rcx){1to32}, %zmm30, %k5
	vcmpord_sph	8128(%rdx), %zmm30, %k5
	vcmpord_sph	8192(%rdx), %zmm30, %k5
	vcmpord_sph	-8192(%rdx), %zmm30, %k5
	vcmpord_sph	-8256(%rdx), %zmm30, %k5
	vcmpord_sph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpord_sph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpord_sph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpord_sph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpeq_usph	%zmm29, %zmm30, %k5
	vcmpeq_usph	%zmm29, %zmm30, %k5{%k7}
	vcmpeq_usph	{sae}, %zmm29, %zmm30, %k5
	vcmpeq_usph	(%rcx), %zmm30, %k5
	vcmpeq_usph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpeq_usph	(%rcx){1to32}, %zmm30, %k5
	vcmpeq_usph	8128(%rdx), %zmm30, %k5
	vcmpeq_usph	8192(%rdx), %zmm30, %k5
	vcmpeq_usph	-8192(%rdx), %zmm30, %k5
	vcmpeq_usph	-8256(%rdx), %zmm30, %k5
	vcmpeq_usph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpeq_usph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpeq_usph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpeq_usph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpnge_uqph	%zmm29, %zmm30, %k5
	vcmpnge_uqph	%zmm29, %zmm30, %k5{%k7}
	vcmpnge_uqph	{sae}, %zmm29, %zmm30, %k5
	vcmpnge_uqph	(%rcx), %zmm30, %k5
	vcmpnge_uqph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpnge_uqph	(%rcx){1to32}, %zmm30, %k5
	vcmpnge_uqph	8128(%rdx), %zmm30, %k5
	vcmpnge_uqph	8192(%rdx), %zmm30, %k5
	vcmpnge_uqph	-8192(%rdx), %zmm30, %k5
	vcmpnge_uqph	-8256(%rdx), %zmm30, %k5
	vcmpnge_uqph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpnge_uqph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpnge_uqph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpnge_uqph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpngt_uqph	%zmm29, %zmm30, %k5
	vcmpngt_uqph	%zmm29, %zmm30, %k5{%k7}
	vcmpngt_uqph	{sae}, %zmm29, %zmm30, %k5
	vcmpngt_uqph	(%rcx), %zmm30, %k5
	vcmpngt_uqph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpngt_uqph	(%rcx){1to32}, %zmm30, %k5
	vcmpngt_uqph	8128(%rdx), %zmm30, %k5
	vcmpngt_uqph	8192(%rdx), %zmm30, %k5
	vcmpngt_uqph	-8192(%rdx), %zmm30, %k5
	vcmpngt_uqph	-8256(%rdx), %zmm30, %k5
	vcmpngt_uqph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpngt_uqph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpngt_uqph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpngt_uqph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpfalse_osph	%zmm29, %zmm30, %k5
	vcmpfalse_osph	%zmm29, %zmm30, %k5{%k7}
	vcmpfalse_osph	{sae}, %zmm29, %zmm30, %k5
	vcmpfalse_osph	(%rcx), %zmm30, %k5
	vcmpfalse_osph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpfalse_osph	(%rcx){1to32}, %zmm30, %k5
	vcmpfalse_osph	8128(%rdx), %zmm30, %k5
	vcmpfalse_osph	8192(%rdx), %zmm30, %k5
	vcmpfalse_osph	-8192(%rdx), %zmm30, %k5
	vcmpfalse_osph	-8256(%rdx), %zmm30, %k5
	vcmpfalse_osph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpfalse_osph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpfalse_osph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpfalse_osph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpneq_osph	%zmm29, %zmm30, %k5
	vcmpneq_osph	%zmm29, %zmm30, %k5{%k7}
	vcmpneq_osph	{sae}, %zmm29, %zmm30, %k5
	vcmpneq_osph	(%rcx), %zmm30, %k5
	vcmpneq_osph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpneq_osph	(%rcx){1to32}, %zmm30, %k5
	vcmpneq_osph	8128(%rdx), %zmm30, %k5
	vcmpneq_osph	8192(%rdx), %zmm30, %k5
	vcmpneq_osph	-8192(%rdx), %zmm30, %k5
	vcmpneq_osph	-8256(%rdx), %zmm30, %k5
	vcmpneq_osph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpneq_osph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpneq_osph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpneq_osph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpge_oqph	%zmm29, %zmm30, %k5
	vcmpge_oqph	%zmm29, %zmm30, %k5{%k7}
	vcmpge_oqph	{sae}, %zmm29, %zmm30, %k5
	vcmpge_oqph	(%rcx), %zmm30, %k5
	vcmpge_oqph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpge_oqph	(%rcx){1to32}, %zmm30, %k5
	vcmpge_oqph	8128(%rdx), %zmm30, %k5
	vcmpge_oqph	8192(%rdx), %zmm30, %k5
	vcmpge_oqph	-8192(%rdx), %zmm30, %k5
	vcmpge_oqph	-8256(%rdx), %zmm30, %k5
	vcmpge_oqph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpge_oqph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpge_oqph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpge_oqph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpgt_oqph	%zmm29, %zmm30, %k5
	vcmpgt_oqph	%zmm29, %zmm30, %k5{%k7}
	vcmpgt_oqph	{sae}, %zmm29, %zmm30, %k5
	vcmpgt_oqph	(%rcx), %zmm30, %k5
	vcmpgt_oqph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmpgt_oqph	(%rcx){1to32}, %zmm30, %k5
	vcmpgt_oqph	8128(%rdx), %zmm30, %k5
	vcmpgt_oqph	8192(%rdx), %zmm30, %k5
	vcmpgt_oqph	-8192(%rdx), %zmm30, %k5
	vcmpgt_oqph	-8256(%rdx), %zmm30, %k5
	vcmpgt_oqph	1016(%rdx){1to32}, %zmm30, %k5
	vcmpgt_oqph	1024(%rdx){1to32}, %zmm30, %k5
	vcmpgt_oqph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmpgt_oqph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmptrue_usph	%zmm29, %zmm30, %k5
	vcmptrue_usph	%zmm29, %zmm30, %k5{%k7}
	vcmptrue_usph	{sae}, %zmm29, %zmm30, %k5
	vcmptrue_usph	(%rcx), %zmm30, %k5
	vcmptrue_usph	0x123(%rax,%r14,8), %zmm30, %k5
	vcmptrue_usph	(%rcx){1to32}, %zmm30, %k5
	vcmptrue_usph	8128(%rdx), %zmm30, %k5
	vcmptrue_usph	8192(%rdx), %zmm30, %k5
	vcmptrue_usph	-8192(%rdx), %zmm30, %k5
	vcmptrue_usph	-8256(%rdx), %zmm30, %k5
	vcmptrue_usph	1016(%rdx){1to32}, %zmm30, %k5
	vcmptrue_usph	1024(%rdx){1to32}, %zmm30, %k5
	vcmptrue_usph	-1024(%rdx){1to32}, %zmm30, %k5
	vcmptrue_usph	-1032(%rdx){1to32}, %zmm30, %k5
	vcmpeq_oqsh	%xmm28, %xmm29, %k5{%k7}
	vcmpeq_oqsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpeq_oqsh	(%rcx), %xmm29, %k5{%k7}
	vcmpeq_oqsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpeq_oqsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpeq_oqsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpeq_oqsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpeq_oqsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpeqsh	%xmm28, %xmm29, %k5{%k7}
	vcmpeqsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpeqsh	(%rcx), %xmm29, %k5{%k7}
	vcmpeqsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpeqsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpeqsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpeqsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpeqsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmplt_ossh	%xmm28, %xmm29, %k5{%k7}
	vcmplt_ossh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmplt_ossh	(%rcx), %xmm29, %k5{%k7}
	vcmplt_ossh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmplt_ossh	1016(%rdx), %xmm29, %k5{%k7}
	vcmplt_ossh	1024(%rdx), %xmm29, %k5{%k7}
	vcmplt_ossh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmplt_ossh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpltsh	%xmm28, %xmm29, %k5{%k7}
	vcmpltsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpltsh	(%rcx), %xmm29, %k5{%k7}
	vcmpltsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpltsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpltsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpltsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpltsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmple_ossh	%xmm28, %xmm29, %k5{%k7}
	vcmple_ossh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmple_ossh	(%rcx), %xmm29, %k5{%k7}
	vcmple_ossh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmple_ossh	1016(%rdx), %xmm29, %k5{%k7}
	vcmple_ossh	1024(%rdx), %xmm29, %k5{%k7}
	vcmple_ossh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmple_ossh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmplesh	%xmm28, %xmm29, %k5{%k7}
	vcmplesh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmplesh	(%rcx), %xmm29, %k5{%k7}
	vcmplesh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmplesh	1016(%rdx), %xmm29, %k5{%k7}
	vcmplesh	1024(%rdx), %xmm29, %k5{%k7}
	vcmplesh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmplesh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpunord_qsh	%xmm28, %xmm29, %k5{%k7}
	vcmpunord_qsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpunord_qsh	(%rcx), %xmm29, %k5{%k7}
	vcmpunord_qsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpunord_qsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpunord_qsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpunord_qsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpunord_qsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpunordsh	%xmm28, %xmm29, %k5{%k7}
	vcmpunordsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpunordsh	(%rcx), %xmm29, %k5{%k7}
	vcmpunordsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpunordsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpunordsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpunordsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpunordsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpneq_uqsh	%xmm28, %xmm29, %k5{%k7}
	vcmpneq_uqsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpneq_uqsh	(%rcx), %xmm29, %k5{%k7}
	vcmpneq_uqsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpneq_uqsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpneq_uqsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpneq_uqsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpneq_uqsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpneqsh	%xmm28, %xmm29, %k5{%k7}
	vcmpneqsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpneqsh	(%rcx), %xmm29, %k5{%k7}
	vcmpneqsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpneqsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpneqsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpneqsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpneqsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpnlt_ussh	%xmm28, %xmm29, %k5{%k7}
	vcmpnlt_ussh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpnlt_ussh	(%rcx), %xmm29, %k5{%k7}
	vcmpnlt_ussh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpnlt_ussh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpnlt_ussh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpnlt_ussh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpnlt_ussh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpnltsh	%xmm28, %xmm29, %k5{%k7}
	vcmpnltsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpnltsh	(%rcx), %xmm29, %k5{%k7}
	vcmpnltsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpnltsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpnltsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpnltsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpnltsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpnle_ussh	%xmm28, %xmm29, %k5{%k7}
	vcmpnle_ussh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpnle_ussh	(%rcx), %xmm29, %k5{%k7}
	vcmpnle_ussh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpnle_ussh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpnle_ussh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpnle_ussh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpnle_ussh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpnlesh	%xmm28, %xmm29, %k5{%k7}
	vcmpnlesh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpnlesh	(%rcx), %xmm29, %k5{%k7}
	vcmpnlesh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpnlesh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpnlesh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpnlesh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpnlesh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpord_qsh	%xmm28, %xmm29, %k5{%k7}
	vcmpord_qsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpord_qsh	(%rcx), %xmm29, %k5{%k7}
	vcmpord_qsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpord_qsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpord_qsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpord_qsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpord_qsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpordsh	%xmm28, %xmm29, %k5{%k7}
	vcmpordsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpordsh	(%rcx), %xmm29, %k5{%k7}
	vcmpordsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpordsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpordsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpordsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpordsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpeq_uqsh	%xmm28, %xmm29, %k5{%k7}
	vcmpeq_uqsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpeq_uqsh	(%rcx), %xmm29, %k5{%k7}
	vcmpeq_uqsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpeq_uqsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpeq_uqsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpeq_uqsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpeq_uqsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpnge_ussh	%xmm28, %xmm29, %k5{%k7}
	vcmpnge_ussh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpnge_ussh	(%rcx), %xmm29, %k5{%k7}
	vcmpnge_ussh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpnge_ussh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpnge_ussh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpnge_ussh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpnge_ussh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpngesh	%xmm28, %xmm29, %k5{%k7}
	vcmpngesh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpngesh	(%rcx), %xmm29, %k5{%k7}
	vcmpngesh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpngesh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpngesh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpngesh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpngesh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpngt_ussh	%xmm28, %xmm29, %k5{%k7}
	vcmpngt_ussh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpngt_ussh	(%rcx), %xmm29, %k5{%k7}
	vcmpngt_ussh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpngt_ussh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpngt_ussh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpngt_ussh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpngt_ussh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpngtsh	%xmm28, %xmm29, %k5{%k7}
	vcmpngtsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpngtsh	(%rcx), %xmm29, %k5{%k7}
	vcmpngtsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpngtsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpngtsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpngtsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpngtsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpfalse_oqsh	%xmm28, %xmm29, %k5{%k7}
	vcmpfalse_oqsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpfalse_oqsh	(%rcx), %xmm29, %k5{%k7}
	vcmpfalse_oqsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpfalse_oqsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpfalse_oqsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpfalse_oqsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpfalse_oqsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpfalsesh	%xmm28, %xmm29, %k5{%k7}
	vcmpfalsesh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpfalsesh	(%rcx), %xmm29, %k5{%k7}
	vcmpfalsesh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpfalsesh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpfalsesh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpfalsesh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpfalsesh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpneq_oqsh	%xmm28, %xmm29, %k5{%k7}
	vcmpneq_oqsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpneq_oqsh	(%rcx), %xmm29, %k5{%k7}
	vcmpneq_oqsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpneq_oqsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpneq_oqsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpneq_oqsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpneq_oqsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpge_ossh	%xmm28, %xmm29, %k5{%k7}
	vcmpge_ossh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpge_ossh	(%rcx), %xmm29, %k5{%k7}
	vcmpge_ossh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpge_ossh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpge_ossh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpge_ossh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpge_ossh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpgesh	%xmm28, %xmm29, %k5{%k7}
	vcmpgesh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpgesh	(%rcx), %xmm29, %k5{%k7}
	vcmpgesh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpgesh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpgesh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpgesh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpgesh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpgt_ossh	%xmm28, %xmm29, %k5{%k7}
	vcmpgt_ossh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpgt_ossh	(%rcx), %xmm29, %k5{%k7}
	vcmpgt_ossh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpgt_ossh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpgt_ossh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpgt_ossh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpgt_ossh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpgtsh	%xmm28, %xmm29, %k5{%k7}
	vcmpgtsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpgtsh	(%rcx), %xmm29, %k5{%k7}
	vcmpgtsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpgtsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpgtsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpgtsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpgtsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmptrue_uqsh	%xmm28, %xmm29, %k5{%k7}
	vcmptrue_uqsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmptrue_uqsh	(%rcx), %xmm29, %k5{%k7}
	vcmptrue_uqsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmptrue_uqsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmptrue_uqsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmptrue_uqsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmptrue_uqsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmptruesh	%xmm28, %xmm29, %k5{%k7}
	vcmptruesh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmptruesh	(%rcx), %xmm29, %k5{%k7}
	vcmptruesh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmptruesh	1016(%rdx), %xmm29, %k5{%k7}
	vcmptruesh	1024(%rdx), %xmm29, %k5{%k7}
	vcmptruesh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmptruesh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpeq_ossh	%xmm28, %xmm29, %k5{%k7}
	vcmpeq_ossh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpeq_ossh	(%rcx), %xmm29, %k5{%k7}
	vcmpeq_ossh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpeq_ossh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpeq_ossh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpeq_ossh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpeq_ossh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmplt_oqsh	%xmm28, %xmm29, %k5{%k7}
	vcmplt_oqsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmplt_oqsh	(%rcx), %xmm29, %k5{%k7}
	vcmplt_oqsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmplt_oqsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmplt_oqsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmplt_oqsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmplt_oqsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmple_oqsh	%xmm28, %xmm29, %k5{%k7}
	vcmple_oqsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmple_oqsh	(%rcx), %xmm29, %k5{%k7}
	vcmple_oqsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmple_oqsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmple_oqsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmple_oqsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmple_oqsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpunord_ssh	%xmm28, %xmm29, %k5{%k7}
	vcmpunord_ssh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpunord_ssh	(%rcx), %xmm29, %k5{%k7}
	vcmpunord_ssh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpunord_ssh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpunord_ssh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpunord_ssh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpunord_ssh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpneq_ussh	%xmm28, %xmm29, %k5{%k7}
	vcmpneq_ussh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpneq_ussh	(%rcx), %xmm29, %k5{%k7}
	vcmpneq_ussh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpneq_ussh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpneq_ussh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpneq_ussh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpneq_ussh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpnlt_uqsh	%xmm28, %xmm29, %k5{%k7}
	vcmpnlt_uqsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpnlt_uqsh	(%rcx), %xmm29, %k5{%k7}
	vcmpnlt_uqsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpnlt_uqsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpnlt_uqsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpnlt_uqsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpnlt_uqsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpnle_uqsh	%xmm28, %xmm29, %k5{%k7}
	vcmpnle_uqsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpnle_uqsh	(%rcx), %xmm29, %k5{%k7}
	vcmpnle_uqsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpnle_uqsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpnle_uqsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpnle_uqsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpnle_uqsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpord_ssh	%xmm28, %xmm29, %k5{%k7}
	vcmpord_ssh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpord_ssh	(%rcx), %xmm29, %k5{%k7}
	vcmpord_ssh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpord_ssh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpord_ssh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpord_ssh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpord_ssh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpeq_ussh	%xmm28, %xmm29, %k5{%k7}
	vcmpeq_ussh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpeq_ussh	(%rcx), %xmm29, %k5{%k7}
	vcmpeq_ussh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpeq_ussh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpeq_ussh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpeq_ussh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpeq_ussh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpnge_uqsh	%xmm28, %xmm29, %k5{%k7}
	vcmpnge_uqsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpnge_uqsh	(%rcx), %xmm29, %k5{%k7}
	vcmpnge_uqsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpnge_uqsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpnge_uqsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpnge_uqsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpnge_uqsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpngt_uqsh	%xmm28, %xmm29, %k5{%k7}
	vcmpngt_uqsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpngt_uqsh	(%rcx), %xmm29, %k5{%k7}
	vcmpngt_uqsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpngt_uqsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpngt_uqsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpngt_uqsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpngt_uqsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpfalse_ossh	%xmm28, %xmm29, %k5{%k7}
	vcmpfalse_ossh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpfalse_ossh	(%rcx), %xmm29, %k5{%k7}
	vcmpfalse_ossh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpfalse_ossh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpfalse_ossh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpfalse_ossh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpfalse_ossh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpneq_ossh	%xmm28, %xmm29, %k5{%k7}
	vcmpneq_ossh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpneq_ossh	(%rcx), %xmm29, %k5{%k7}
	vcmpneq_ossh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpneq_ossh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpneq_ossh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpneq_ossh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpneq_ossh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpge_oqsh	%xmm28, %xmm29, %k5{%k7}
	vcmpge_oqsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpge_oqsh	(%rcx), %xmm29, %k5{%k7}
	vcmpge_oqsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpge_oqsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpge_oqsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpge_oqsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpge_oqsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmpgt_oqsh	%xmm28, %xmm29, %k5{%k7}
	vcmpgt_oqsh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmpgt_oqsh	(%rcx), %xmm29, %k5{%k7}
	vcmpgt_oqsh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmpgt_oqsh	1016(%rdx), %xmm29, %k5{%k7}
	vcmpgt_oqsh	1024(%rdx), %xmm29, %k5{%k7}
	vcmpgt_oqsh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmpgt_oqsh	-1032(%rdx), %xmm29, %k5{%k7}
	vcmptrue_ussh	%xmm28, %xmm29, %k5{%k7}
	vcmptrue_ussh	{sae}, %xmm28, %xmm29, %k5{%k7}
	vcmptrue_ussh	(%rcx), %xmm29, %k5{%k7}
	vcmptrue_ussh	0x123(%rax,%r14,8), %xmm29, %k5{%k7}
	vcmptrue_ussh	1016(%rdx), %xmm29, %k5{%k7}
	vcmptrue_ussh	1024(%rdx), %xmm29, %k5{%k7}
	vcmptrue_ussh	-1024(%rdx), %xmm29, %k5{%k7}
	vcmptrue_ussh	-1032(%rdx), %xmm29, %k5{%k7}

.intel_syntax noprefix
	vcmpeq_oqph	k5, zmm30, zmm29
	vcmpeq_oqph	k5{k7}, zmm30, zmm29
	vcmpeq_oqph	k5, zmm30, zmm29, {sae}
	vcmpeq_oqph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpeq_oqph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpeq_oqph	k5, zmm30, [rcx]{1to32}
	vcmpeq_oqph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpeq_oqph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpeq_oqph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpeq_oqph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpeq_oqph	k5, zmm30, [rdx+1016]{1to32}
	vcmpeq_oqph	k5, zmm30, [rdx+1024]{1to32}
	vcmpeq_oqph	k5, zmm30, [rdx-1024]{1to32}
	vcmpeq_oqph	k5, zmm30, [rdx-1032]{1to32}
	vcmpeqph	k5, zmm30, zmm29
	vcmpeqph	k5{k7}, zmm30, zmm29
	vcmpeqph	k5, zmm30, zmm29, {sae}
	vcmpeqph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpeqph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpeqph	k5, zmm30, [rcx]{1to32}
	vcmpeqph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpeqph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpeqph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpeqph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpeqph	k5, zmm30, [rdx+1016]{1to32}
	vcmpeqph	k5, zmm30, [rdx+1024]{1to32}
	vcmpeqph	k5, zmm30, [rdx-1024]{1to32}
	vcmpeqph	k5, zmm30, [rdx-1032]{1to32}
	vcmplt_osph	k5, zmm30, zmm29
	vcmplt_osph	k5{k7}, zmm30, zmm29
	vcmplt_osph	k5, zmm30, zmm29, {sae}
	vcmplt_osph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmplt_osph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmplt_osph	k5, zmm30, [rcx]{1to32}
	vcmplt_osph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmplt_osph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmplt_osph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmplt_osph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmplt_osph	k5, zmm30, [rdx+1016]{1to32}
	vcmplt_osph	k5, zmm30, [rdx+1024]{1to32}
	vcmplt_osph	k5, zmm30, [rdx-1024]{1to32}
	vcmplt_osph	k5, zmm30, [rdx-1032]{1to32}
	vcmpltph	k5, zmm30, zmm29
	vcmpltph	k5{k7}, zmm30, zmm29
	vcmpltph	k5, zmm30, zmm29, {sae}
	vcmpltph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpltph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpltph	k5, zmm30, [rcx]{1to32}
	vcmpltph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpltph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpltph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpltph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpltph	k5, zmm30, [rdx+1016]{1to32}
	vcmpltph	k5, zmm30, [rdx+1024]{1to32}
	vcmpltph	k5, zmm30, [rdx-1024]{1to32}
	vcmpltph	k5, zmm30, [rdx-1032]{1to32}
	vcmple_osph	k5, zmm30, zmm29
	vcmple_osph	k5{k7}, zmm30, zmm29
	vcmple_osph	k5, zmm30, zmm29, {sae}
	vcmple_osph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmple_osph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmple_osph	k5, zmm30, [rcx]{1to32}
	vcmple_osph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmple_osph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmple_osph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmple_osph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmple_osph	k5, zmm30, [rdx+1016]{1to32}
	vcmple_osph	k5, zmm30, [rdx+1024]{1to32}
	vcmple_osph	k5, zmm30, [rdx-1024]{1to32}
	vcmple_osph	k5, zmm30, [rdx-1032]{1to32}
	vcmpleph	k5, zmm30, zmm29
	vcmpleph	k5{k7}, zmm30, zmm29
	vcmpleph	k5, zmm30, zmm29, {sae}
	vcmpleph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpleph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpleph	k5, zmm30, [rcx]{1to32}
	vcmpleph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpleph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpleph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpleph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpleph	k5, zmm30, [rdx+1016]{1to32}
	vcmpleph	k5, zmm30, [rdx+1024]{1to32}
	vcmpleph	k5, zmm30, [rdx-1024]{1to32}
	vcmpleph	k5, zmm30, [rdx-1032]{1to32}
	vcmpunord_qph	k5, zmm30, zmm29
	vcmpunord_qph	k5{k7}, zmm30, zmm29
	vcmpunord_qph	k5, zmm30, zmm29, {sae}
	vcmpunord_qph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpunord_qph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpunord_qph	k5, zmm30, [rcx]{1to32}
	vcmpunord_qph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpunord_qph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpunord_qph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpunord_qph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpunord_qph	k5, zmm30, [rdx+1016]{1to32}
	vcmpunord_qph	k5, zmm30, [rdx+1024]{1to32}
	vcmpunord_qph	k5, zmm30, [rdx-1024]{1to32}
	vcmpunord_qph	k5, zmm30, [rdx-1032]{1to32}
	vcmpunordph	k5, zmm30, zmm29
	vcmpunordph	k5{k7}, zmm30, zmm29
	vcmpunordph	k5, zmm30, zmm29, {sae}
	vcmpunordph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpunordph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpunordph	k5, zmm30, [rcx]{1to32}
	vcmpunordph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpunordph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpunordph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpunordph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpunordph	k5, zmm30, [rdx+1016]{1to32}
	vcmpunordph	k5, zmm30, [rdx+1024]{1to32}
	vcmpunordph	k5, zmm30, [rdx-1024]{1to32}
	vcmpunordph	k5, zmm30, [rdx-1032]{1to32}
	vcmpneq_uqph	k5, zmm30, zmm29
	vcmpneq_uqph	k5{k7}, zmm30, zmm29
	vcmpneq_uqph	k5, zmm30, zmm29, {sae}
	vcmpneq_uqph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpneq_uqph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpneq_uqph	k5, zmm30, [rcx]{1to32}
	vcmpneq_uqph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpneq_uqph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpneq_uqph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpneq_uqph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpneq_uqph	k5, zmm30, [rdx+1016]{1to32}
	vcmpneq_uqph	k5, zmm30, [rdx+1024]{1to32}
	vcmpneq_uqph	k5, zmm30, [rdx-1024]{1to32}
	vcmpneq_uqph	k5, zmm30, [rdx-1032]{1to32}
	vcmpneqph	k5, zmm30, zmm29
	vcmpneqph	k5{k7}, zmm30, zmm29
	vcmpneqph	k5, zmm30, zmm29, {sae}
	vcmpneqph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpneqph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpneqph	k5, zmm30, [rcx]{1to32}
	vcmpneqph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpneqph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpneqph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpneqph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpneqph	k5, zmm30, [rdx+1016]{1to32}
	vcmpneqph	k5, zmm30, [rdx+1024]{1to32}
	vcmpneqph	k5, zmm30, [rdx-1024]{1to32}
	vcmpneqph	k5, zmm30, [rdx-1032]{1to32}
	vcmpnlt_usph	k5, zmm30, zmm29
	vcmpnlt_usph	k5{k7}, zmm30, zmm29
	vcmpnlt_usph	k5, zmm30, zmm29, {sae}
	vcmpnlt_usph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpnlt_usph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpnlt_usph	k5, zmm30, [rcx]{1to32}
	vcmpnlt_usph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpnlt_usph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpnlt_usph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpnlt_usph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpnlt_usph	k5, zmm30, [rdx+1016]{1to32}
	vcmpnlt_usph	k5, zmm30, [rdx+1024]{1to32}
	vcmpnlt_usph	k5, zmm30, [rdx-1024]{1to32}
	vcmpnlt_usph	k5, zmm30, [rdx-1032]{1to32}
	vcmpnltph	k5, zmm30, zmm29
	vcmpnltph	k5{k7}, zmm30, zmm29
	vcmpnltph	k5, zmm30, zmm29, {sae}
	vcmpnltph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpnltph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpnltph	k5, zmm30, [rcx]{1to32}
	vcmpnltph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpnltph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpnltph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpnltph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpnltph	k5, zmm30, [rdx+1016]{1to32}
	vcmpnltph	k5, zmm30, [rdx+1024]{1to32}
	vcmpnltph	k5, zmm30, [rdx-1024]{1to32}
	vcmpnltph	k5, zmm30, [rdx-1032]{1to32}
	vcmpnle_usph	k5, zmm30, zmm29
	vcmpnle_usph	k5{k7}, zmm30, zmm29
	vcmpnle_usph	k5, zmm30, zmm29, {sae}
	vcmpnle_usph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpnle_usph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpnle_usph	k5, zmm30, [rcx]{1to32}
	vcmpnle_usph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpnle_usph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpnle_usph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpnle_usph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpnle_usph	k5, zmm30, [rdx+1016]{1to32}
	vcmpnle_usph	k5, zmm30, [rdx+1024]{1to32}
	vcmpnle_usph	k5, zmm30, [rdx-1024]{1to32}
	vcmpnle_usph	k5, zmm30, [rdx-1032]{1to32}
	vcmpnleph	k5, zmm30, zmm29
	vcmpnleph	k5{k7}, zmm30, zmm29
	vcmpnleph	k5, zmm30, zmm29, {sae}
	vcmpnleph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpnleph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpnleph	k5, zmm30, [rcx]{1to32}
	vcmpnleph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpnleph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpnleph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpnleph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpnleph	k5, zmm30, [rdx+1016]{1to32}
	vcmpnleph	k5, zmm30, [rdx+1024]{1to32}
	vcmpnleph	k5, zmm30, [rdx-1024]{1to32}
	vcmpnleph	k5, zmm30, [rdx-1032]{1to32}
	vcmpord_qph	k5, zmm30, zmm29
	vcmpord_qph	k5{k7}, zmm30, zmm29
	vcmpord_qph	k5, zmm30, zmm29, {sae}
	vcmpord_qph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpord_qph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpord_qph	k5, zmm30, [rcx]{1to32}
	vcmpord_qph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpord_qph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpord_qph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpord_qph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpord_qph	k5, zmm30, [rdx+1016]{1to32}
	vcmpord_qph	k5, zmm30, [rdx+1024]{1to32}
	vcmpord_qph	k5, zmm30, [rdx-1024]{1to32}
	vcmpord_qph	k5, zmm30, [rdx-1032]{1to32}
	vcmpordph	k5, zmm30, zmm29
	vcmpordph	k5{k7}, zmm30, zmm29
	vcmpordph	k5, zmm30, zmm29, {sae}
	vcmpordph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpordph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpordph	k5, zmm30, [rcx]{1to32}
	vcmpordph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpordph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpordph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpordph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpordph	k5, zmm30, [rdx+1016]{1to32}
	vcmpordph	k5, zmm30, [rdx+1024]{1to32}
	vcmpordph	k5, zmm30, [rdx-1024]{1to32}
	vcmpordph	k5, zmm30, [rdx-1032]{1to32}
	vcmpeq_uqph	k5, zmm30, zmm29
	vcmpeq_uqph	k5{k7}, zmm30, zmm29
	vcmpeq_uqph	k5, zmm30, zmm29, {sae}
	vcmpeq_uqph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpeq_uqph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpeq_uqph	k5, zmm30, [rcx]{1to32}
	vcmpeq_uqph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpeq_uqph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpeq_uqph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpeq_uqph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpeq_uqph	k5, zmm30, [rdx+1016]{1to32}
	vcmpeq_uqph	k5, zmm30, [rdx+1024]{1to32}
	vcmpeq_uqph	k5, zmm30, [rdx-1024]{1to32}
	vcmpeq_uqph	k5, zmm30, [rdx-1032]{1to32}
	vcmpnge_usph	k5, zmm30, zmm29
	vcmpnge_usph	k5{k7}, zmm30, zmm29
	vcmpnge_usph	k5, zmm30, zmm29, {sae}
	vcmpnge_usph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpnge_usph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpnge_usph	k5, zmm30, [rcx]{1to32}
	vcmpnge_usph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpnge_usph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpnge_usph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpnge_usph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpnge_usph	k5, zmm30, [rdx+1016]{1to32}
	vcmpnge_usph	k5, zmm30, [rdx+1024]{1to32}
	vcmpnge_usph	k5, zmm30, [rdx-1024]{1to32}
	vcmpnge_usph	k5, zmm30, [rdx-1032]{1to32}
	vcmpngeph	k5, zmm30, zmm29
	vcmpngeph	k5{k7}, zmm30, zmm29
	vcmpngeph	k5, zmm30, zmm29, {sae}
	vcmpngeph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpngeph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpngeph	k5, zmm30, [rcx]{1to32}
	vcmpngeph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpngeph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpngeph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpngeph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpngeph	k5, zmm30, [rdx+1016]{1to32}
	vcmpngeph	k5, zmm30, [rdx+1024]{1to32}
	vcmpngeph	k5, zmm30, [rdx-1024]{1to32}
	vcmpngeph	k5, zmm30, [rdx-1032]{1to32}
	vcmpngt_usph	k5, zmm30, zmm29
	vcmpngt_usph	k5{k7}, zmm30, zmm29
	vcmpngt_usph	k5, zmm30, zmm29, {sae}
	vcmpngt_usph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpngt_usph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpngt_usph	k5, zmm30, [rcx]{1to32}
	vcmpngt_usph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpngt_usph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpngt_usph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpngt_usph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpngt_usph	k5, zmm30, [rdx+1016]{1to32}
	vcmpngt_usph	k5, zmm30, [rdx+1024]{1to32}
	vcmpngt_usph	k5, zmm30, [rdx-1024]{1to32}
	vcmpngt_usph	k5, zmm30, [rdx-1032]{1to32}
	vcmpngtph	k5, zmm30, zmm29
	vcmpngtph	k5{k7}, zmm30, zmm29
	vcmpngtph	k5, zmm30, zmm29, {sae}
	vcmpngtph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpngtph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpngtph	k5, zmm30, [rcx]{1to32}
	vcmpngtph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpngtph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpngtph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpngtph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpngtph	k5, zmm30, [rdx+1016]{1to32}
	vcmpngtph	k5, zmm30, [rdx+1024]{1to32}
	vcmpngtph	k5, zmm30, [rdx-1024]{1to32}
	vcmpngtph	k5, zmm30, [rdx-1032]{1to32}
	vcmpfalse_oqph	k5, zmm30, zmm29
	vcmpfalse_oqph	k5{k7}, zmm30, zmm29
	vcmpfalse_oqph	k5, zmm30, zmm29, {sae}
	vcmpfalse_oqph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpfalse_oqph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpfalse_oqph	k5, zmm30, [rcx]{1to32}
	vcmpfalse_oqph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpfalse_oqph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpfalse_oqph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpfalse_oqph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpfalse_oqph	k5, zmm30, [rdx+1016]{1to32}
	vcmpfalse_oqph	k5, zmm30, [rdx+1024]{1to32}
	vcmpfalse_oqph	k5, zmm30, [rdx-1024]{1to32}
	vcmpfalse_oqph	k5, zmm30, [rdx-1032]{1to32}
	vcmpfalseph	k5, zmm30, zmm29
	vcmpfalseph	k5{k7}, zmm30, zmm29
	vcmpfalseph	k5, zmm30, zmm29, {sae}
	vcmpfalseph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpfalseph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpfalseph	k5, zmm30, [rcx]{1to32}
	vcmpfalseph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpfalseph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpfalseph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpfalseph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpfalseph	k5, zmm30, [rdx+1016]{1to32}
	vcmpfalseph	k5, zmm30, [rdx+1024]{1to32}
	vcmpfalseph	k5, zmm30, [rdx-1024]{1to32}
	vcmpfalseph	k5, zmm30, [rdx-1032]{1to32}
	vcmpneq_oqph	k5, zmm30, zmm29
	vcmpneq_oqph	k5{k7}, zmm30, zmm29
	vcmpneq_oqph	k5, zmm30, zmm29, {sae}
	vcmpneq_oqph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpneq_oqph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpneq_oqph	k5, zmm30, [rcx]{1to32}
	vcmpneq_oqph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpneq_oqph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpneq_oqph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpneq_oqph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpneq_oqph	k5, zmm30, [rdx+1016]{1to32}
	vcmpneq_oqph	k5, zmm30, [rdx+1024]{1to32}
	vcmpneq_oqph	k5, zmm30, [rdx-1024]{1to32}
	vcmpneq_oqph	k5, zmm30, [rdx-1032]{1to32}
	vcmpge_osph	k5, zmm30, zmm29
	vcmpge_osph	k5{k7}, zmm30, zmm29
	vcmpge_osph	k5, zmm30, zmm29, {sae}
	vcmpge_osph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpge_osph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpge_osph	k5, zmm30, [rcx]{1to32}
	vcmpge_osph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpge_osph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpge_osph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpge_osph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpge_osph	k5, zmm30, [rdx+1016]{1to32}
	vcmpge_osph	k5, zmm30, [rdx+1024]{1to32}
	vcmpge_osph	k5, zmm30, [rdx-1024]{1to32}
	vcmpge_osph	k5, zmm30, [rdx-1032]{1to32}
	vcmpgeph	k5, zmm30, zmm29
	vcmpgeph	k5{k7}, zmm30, zmm29
	vcmpgeph	k5, zmm30, zmm29, {sae}
	vcmpgeph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpgeph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpgeph	k5, zmm30, [rcx]{1to32}
	vcmpgeph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpgeph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpgeph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpgeph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpgeph	k5, zmm30, [rdx+1016]{1to32}
	vcmpgeph	k5, zmm30, [rdx+1024]{1to32}
	vcmpgeph	k5, zmm30, [rdx-1024]{1to32}
	vcmpgeph	k5, zmm30, [rdx-1032]{1to32}
	vcmpgt_osph	k5, zmm30, zmm29
	vcmpgt_osph	k5{k7}, zmm30, zmm29
	vcmpgt_osph	k5, zmm30, zmm29, {sae}
	vcmpgt_osph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpgt_osph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpgt_osph	k5, zmm30, [rcx]{1to32}
	vcmpgt_osph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpgt_osph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpgt_osph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpgt_osph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpgt_osph	k5, zmm30, [rdx+1016]{1to32}
	vcmpgt_osph	k5, zmm30, [rdx+1024]{1to32}
	vcmpgt_osph	k5, zmm30, [rdx-1024]{1to32}
	vcmpgt_osph	k5, zmm30, [rdx-1032]{1to32}
	vcmpgtph	k5, zmm30, zmm29
	vcmpgtph	k5{k7}, zmm30, zmm29
	vcmpgtph	k5, zmm30, zmm29, {sae}
	vcmpgtph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpgtph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpgtph	k5, zmm30, [rcx]{1to32}
	vcmpgtph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpgtph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpgtph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpgtph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpgtph	k5, zmm30, [rdx+1016]{1to32}
	vcmpgtph	k5, zmm30, [rdx+1024]{1to32}
	vcmpgtph	k5, zmm30, [rdx-1024]{1to32}
	vcmpgtph	k5, zmm30, [rdx-1032]{1to32}
	vcmptrue_uqph	k5, zmm30, zmm29
	vcmptrue_uqph	k5{k7}, zmm30, zmm29
	vcmptrue_uqph	k5, zmm30, zmm29, {sae}
	vcmptrue_uqph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmptrue_uqph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmptrue_uqph	k5, zmm30, [rcx]{1to32}
	vcmptrue_uqph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmptrue_uqph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmptrue_uqph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmptrue_uqph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmptrue_uqph	k5, zmm30, [rdx+1016]{1to32}
	vcmptrue_uqph	k5, zmm30, [rdx+1024]{1to32}
	vcmptrue_uqph	k5, zmm30, [rdx-1024]{1to32}
	vcmptrue_uqph	k5, zmm30, [rdx-1032]{1to32}
	vcmptrueph	k5, zmm30, zmm29
	vcmptrueph	k5{k7}, zmm30, zmm29
	vcmptrueph	k5, zmm30, zmm29, {sae}
	vcmptrueph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmptrueph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmptrueph	k5, zmm30, [rcx]{1to32}
	vcmptrueph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmptrueph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmptrueph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmptrueph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmptrueph	k5, zmm30, [rdx+1016]{1to32}
	vcmptrueph	k5, zmm30, [rdx+1024]{1to32}
	vcmptrueph	k5, zmm30, [rdx-1024]{1to32}
	vcmptrueph	k5, zmm30, [rdx-1032]{1to32}
	vcmpeq_osph	k5, zmm30, zmm29
	vcmpeq_osph	k5{k7}, zmm30, zmm29
	vcmpeq_osph	k5, zmm30, zmm29, {sae}
	vcmpeq_osph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpeq_osph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpeq_osph	k5, zmm30, [rcx]{1to32}
	vcmpeq_osph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpeq_osph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpeq_osph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpeq_osph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpeq_osph	k5, zmm30, [rdx+1016]{1to32}
	vcmpeq_osph	k5, zmm30, [rdx+1024]{1to32}
	vcmpeq_osph	k5, zmm30, [rdx-1024]{1to32}
	vcmpeq_osph	k5, zmm30, [rdx-1032]{1to32}
	vcmplt_oqph	k5, zmm30, zmm29
	vcmplt_oqph	k5{k7}, zmm30, zmm29
	vcmplt_oqph	k5, zmm30, zmm29, {sae}
	vcmplt_oqph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmplt_oqph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmplt_oqph	k5, zmm30, [rcx]{1to32}
	vcmplt_oqph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmplt_oqph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmplt_oqph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmplt_oqph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmplt_oqph	k5, zmm30, [rdx+1016]{1to32}
	vcmplt_oqph	k5, zmm30, [rdx+1024]{1to32}
	vcmplt_oqph	k5, zmm30, [rdx-1024]{1to32}
	vcmplt_oqph	k5, zmm30, [rdx-1032]{1to32}
	vcmple_oqph	k5, zmm30, zmm29
	vcmple_oqph	k5{k7}, zmm30, zmm29
	vcmple_oqph	k5, zmm30, zmm29, {sae}
	vcmple_oqph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmple_oqph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmple_oqph	k5, zmm30, [rcx]{1to32}
	vcmple_oqph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmple_oqph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmple_oqph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmple_oqph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmple_oqph	k5, zmm30, [rdx+1016]{1to32}
	vcmple_oqph	k5, zmm30, [rdx+1024]{1to32}
	vcmple_oqph	k5, zmm30, [rdx-1024]{1to32}
	vcmple_oqph	k5, zmm30, [rdx-1032]{1to32}
	vcmpunord_sph	k5, zmm30, zmm29
	vcmpunord_sph	k5{k7}, zmm30, zmm29
	vcmpunord_sph	k5, zmm30, zmm29, {sae}
	vcmpunord_sph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpunord_sph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpunord_sph	k5, zmm30, [rcx]{1to32}
	vcmpunord_sph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpunord_sph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpunord_sph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpunord_sph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpunord_sph	k5, zmm30, [rdx+1016]{1to32}
	vcmpunord_sph	k5, zmm30, [rdx+1024]{1to32}
	vcmpunord_sph	k5, zmm30, [rdx-1024]{1to32}
	vcmpunord_sph	k5, zmm30, [rdx-1032]{1to32}
	vcmpneq_usph	k5, zmm30, zmm29
	vcmpneq_usph	k5{k7}, zmm30, zmm29
	vcmpneq_usph	k5, zmm30, zmm29, {sae}
	vcmpneq_usph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpneq_usph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpneq_usph	k5, zmm30, [rcx]{1to32}
	vcmpneq_usph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpneq_usph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpneq_usph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpneq_usph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpneq_usph	k5, zmm30, [rdx+1016]{1to32}
	vcmpneq_usph	k5, zmm30, [rdx+1024]{1to32}
	vcmpneq_usph	k5, zmm30, [rdx-1024]{1to32}
	vcmpneq_usph	k5, zmm30, [rdx-1032]{1to32}
	vcmpnlt_uqph	k5, zmm30, zmm29
	vcmpnlt_uqph	k5{k7}, zmm30, zmm29
	vcmpnlt_uqph	k5, zmm30, zmm29, {sae}
	vcmpnlt_uqph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpnlt_uqph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpnlt_uqph	k5, zmm30, [rcx]{1to32}
	vcmpnlt_uqph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpnlt_uqph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpnlt_uqph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpnlt_uqph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpnlt_uqph	k5, zmm30, [rdx+1016]{1to32}
	vcmpnlt_uqph	k5, zmm30, [rdx+1024]{1to32}
	vcmpnlt_uqph	k5, zmm30, [rdx-1024]{1to32}
	vcmpnlt_uqph	k5, zmm30, [rdx-1032]{1to32}
	vcmpnle_uqph	k5, zmm30, zmm29
	vcmpnle_uqph	k5{k7}, zmm30, zmm29
	vcmpnle_uqph	k5, zmm30, zmm29, {sae}
	vcmpnle_uqph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpnle_uqph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpnle_uqph	k5, zmm30, [rcx]{1to32}
	vcmpnle_uqph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpnle_uqph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpnle_uqph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpnle_uqph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpnle_uqph	k5, zmm30, [rdx+1016]{1to32}
	vcmpnle_uqph	k5, zmm30, [rdx+1024]{1to32}
	vcmpnle_uqph	k5, zmm30, [rdx-1024]{1to32}
	vcmpnle_uqph	k5, zmm30, [rdx-1032]{1to32}
	vcmpord_sph	k5, zmm30, zmm29
	vcmpord_sph	k5{k7}, zmm30, zmm29
	vcmpord_sph	k5, zmm30, zmm29, {sae}
	vcmpord_sph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpord_sph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpord_sph	k5, zmm30, [rcx]{1to32}
	vcmpord_sph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpord_sph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpord_sph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpord_sph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpord_sph	k5, zmm30, [rdx+1016]{1to32}
	vcmpord_sph	k5, zmm30, [rdx+1024]{1to32}
	vcmpord_sph	k5, zmm30, [rdx-1024]{1to32}
	vcmpord_sph	k5, zmm30, [rdx-1032]{1to32}
	vcmpeq_usph	k5, zmm30, zmm29
	vcmpeq_usph	k5{k7}, zmm30, zmm29
	vcmpeq_usph	k5, zmm30, zmm29, {sae}
	vcmpeq_usph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpeq_usph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpeq_usph	k5, zmm30, [rcx]{1to32}
	vcmpeq_usph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpeq_usph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpeq_usph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpeq_usph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpeq_usph	k5, zmm30, [rdx+1016]{1to32}
	vcmpeq_usph	k5, zmm30, [rdx+1024]{1to32}
	vcmpeq_usph	k5, zmm30, [rdx-1024]{1to32}
	vcmpeq_usph	k5, zmm30, [rdx-1032]{1to32}
	vcmpnge_uqph	k5, zmm30, zmm29
	vcmpnge_uqph	k5{k7}, zmm30, zmm29
	vcmpnge_uqph	k5, zmm30, zmm29, {sae}
	vcmpnge_uqph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpnge_uqph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpnge_uqph	k5, zmm30, [rcx]{1to32}
	vcmpnge_uqph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpnge_uqph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpnge_uqph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpnge_uqph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpnge_uqph	k5, zmm30, [rdx+1016]{1to32}
	vcmpnge_uqph	k5, zmm30, [rdx+1024]{1to32}
	vcmpnge_uqph	k5, zmm30, [rdx-1024]{1to32}
	vcmpnge_uqph	k5, zmm30, [rdx-1032]{1to32}
	vcmpngt_uqph	k5, zmm30, zmm29
	vcmpngt_uqph	k5{k7}, zmm30, zmm29
	vcmpngt_uqph	k5, zmm30, zmm29, {sae}
	vcmpngt_uqph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpngt_uqph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpngt_uqph	k5, zmm30, [rcx]{1to32}
	vcmpngt_uqph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpngt_uqph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpngt_uqph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpngt_uqph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpngt_uqph	k5, zmm30, [rdx+1016]{1to32}
	vcmpngt_uqph	k5, zmm30, [rdx+1024]{1to32}
	vcmpngt_uqph	k5, zmm30, [rdx-1024]{1to32}
	vcmpngt_uqph	k5, zmm30, [rdx-1032]{1to32}
	vcmpfalse_osph	k5, zmm30, zmm29
	vcmpfalse_osph	k5{k7}, zmm30, zmm29
	vcmpfalse_osph	k5, zmm30, zmm29, {sae}
	vcmpfalse_osph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpfalse_osph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpfalse_osph	k5, zmm30, [rcx]{1to32}
	vcmpfalse_osph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpfalse_osph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpfalse_osph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpfalse_osph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpfalse_osph	k5, zmm30, [rdx+1016]{1to32}
	vcmpfalse_osph	k5, zmm30, [rdx+1024]{1to32}
	vcmpfalse_osph	k5, zmm30, [rdx-1024]{1to32}
	vcmpfalse_osph	k5, zmm30, [rdx-1032]{1to32}
	vcmpneq_osph	k5, zmm30, zmm29
	vcmpneq_osph	k5{k7}, zmm30, zmm29
	vcmpneq_osph	k5, zmm30, zmm29, {sae}
	vcmpneq_osph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpneq_osph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpneq_osph	k5, zmm30, [rcx]{1to32}
	vcmpneq_osph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpneq_osph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpneq_osph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpneq_osph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpneq_osph	k5, zmm30, [rdx+1016]{1to32}
	vcmpneq_osph	k5, zmm30, [rdx+1024]{1to32}
	vcmpneq_osph	k5, zmm30, [rdx-1024]{1to32}
	vcmpneq_osph	k5, zmm30, [rdx-1032]{1to32}
	vcmpge_oqph	k5, zmm30, zmm29
	vcmpge_oqph	k5{k7}, zmm30, zmm29
	vcmpge_oqph	k5, zmm30, zmm29, {sae}
	vcmpge_oqph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpge_oqph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpge_oqph	k5, zmm30, [rcx]{1to32}
	vcmpge_oqph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpge_oqph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpge_oqph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpge_oqph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpge_oqph	k5, zmm30, [rdx+1016]{1to32}
	vcmpge_oqph	k5, zmm30, [rdx+1024]{1to32}
	vcmpge_oqph	k5, zmm30, [rdx-1024]{1to32}
	vcmpge_oqph	k5, zmm30, [rdx-1032]{1to32}
	vcmpgt_oqph	k5, zmm30, zmm29
	vcmpgt_oqph	k5{k7}, zmm30, zmm29
	vcmpgt_oqph	k5, zmm30, zmm29, {sae}
	vcmpgt_oqph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmpgt_oqph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmpgt_oqph	k5, zmm30, [rcx]{1to32}
	vcmpgt_oqph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmpgt_oqph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmpgt_oqph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmpgt_oqph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmpgt_oqph	k5, zmm30, [rdx+1016]{1to32}
	vcmpgt_oqph	k5, zmm30, [rdx+1024]{1to32}
	vcmpgt_oqph	k5, zmm30, [rdx-1024]{1to32}
	vcmpgt_oqph	k5, zmm30, [rdx-1032]{1to32}
	vcmptrue_usph	k5, zmm30, zmm29
	vcmptrue_usph	k5{k7}, zmm30, zmm29
	vcmptrue_usph	k5, zmm30, zmm29, {sae}
	vcmptrue_usph	k5, zmm30, ZMMWORD PTR [rcx]
	vcmptrue_usph	k5, zmm30, ZMMWORD PTR [rax+r14*8+0x1234]
	vcmptrue_usph	k5, zmm30, [rcx]{1to32}
	vcmptrue_usph	k5, zmm30, ZMMWORD PTR [rdx+8128]
	vcmptrue_usph	k5, zmm30, ZMMWORD PTR [rdx+8192]
	vcmptrue_usph	k5, zmm30, ZMMWORD PTR [rdx-8192]
	vcmptrue_usph	k5, zmm30, ZMMWORD PTR [rdx-8256]
	vcmptrue_usph	k5, zmm30, [rdx+1016]{1to32}
	vcmptrue_usph	k5, zmm30, [rdx+1024]{1to32}
	vcmptrue_usph	k5, zmm30, [rdx-1024]{1to32}
	vcmptrue_usph	k5, zmm30, [rdx-1032]{1to32}
	vcmpeq_oqsh	k5{k7}, xmm29, xmm28
	vcmpeq_oqsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpeq_oqsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpeq_oqsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpeq_oqsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpeq_oqsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpeq_oqsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpeq_oqsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpeqsh	k5{k7}, xmm29, xmm28
	vcmpeqsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpeqsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpeqsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpeqsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpeqsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpeqsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpeqsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmplt_ossh	k5{k7}, xmm29, xmm28
	vcmplt_ossh	k5{k7}, xmm29, xmm28, {sae}
	vcmplt_ossh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmplt_ossh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmplt_ossh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmplt_ossh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmplt_ossh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmplt_ossh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpltsh	k5{k7}, xmm29, xmm28
	vcmpltsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpltsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpltsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpltsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpltsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpltsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpltsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmple_ossh	k5{k7}, xmm29, xmm28
	vcmple_ossh	k5{k7}, xmm29, xmm28, {sae}
	vcmple_ossh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmple_ossh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmple_ossh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmple_ossh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmple_ossh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmple_ossh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmplesh	k5{k7}, xmm29, xmm28
	vcmplesh	k5{k7}, xmm29, xmm28, {sae}
	vcmplesh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmplesh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmplesh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmplesh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmplesh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmplesh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpunord_qsh	k5{k7}, xmm29, xmm28
	vcmpunord_qsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpunord_qsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpunord_qsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpunord_qsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpunord_qsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpunord_qsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpunord_qsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpunordsh	k5{k7}, xmm29, xmm28
	vcmpunordsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpunordsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpunordsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpunordsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpunordsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpunordsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpunordsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpneq_uqsh	k5{k7}, xmm29, xmm28
	vcmpneq_uqsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpneq_uqsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpneq_uqsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpneq_uqsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpneq_uqsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpneq_uqsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpneq_uqsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpneqsh	k5{k7}, xmm29, xmm28
	vcmpneqsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpneqsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpneqsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpneqsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpneqsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpneqsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpneqsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpnlt_ussh	k5{k7}, xmm29, xmm28
	vcmpnlt_ussh	k5{k7}, xmm29, xmm28, {sae}
	vcmpnlt_ussh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpnlt_ussh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpnlt_ussh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpnlt_ussh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpnlt_ussh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpnlt_ussh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpnltsh	k5{k7}, xmm29, xmm28
	vcmpnltsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpnltsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpnltsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpnltsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpnltsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpnltsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpnltsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpnle_ussh	k5{k7}, xmm29, xmm28
	vcmpnle_ussh	k5{k7}, xmm29, xmm28, {sae}
	vcmpnle_ussh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpnle_ussh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpnle_ussh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpnle_ussh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpnle_ussh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpnle_ussh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpnlesh	k5{k7}, xmm29, xmm28
	vcmpnlesh	k5{k7}, xmm29, xmm28, {sae}
	vcmpnlesh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpnlesh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpnlesh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpnlesh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpnlesh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpnlesh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpord_qsh	k5{k7}, xmm29, xmm28
	vcmpord_qsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpord_qsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpord_qsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpord_qsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpord_qsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpord_qsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpord_qsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpordsh	k5{k7}, xmm29, xmm28
	vcmpordsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpordsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpordsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpordsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpordsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpordsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpordsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpeq_uqsh	k5{k7}, xmm29, xmm28
	vcmpeq_uqsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpeq_uqsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpeq_uqsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpeq_uqsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpeq_uqsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpeq_uqsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpeq_uqsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpnge_ussh	k5{k7}, xmm29, xmm28
	vcmpnge_ussh	k5{k7}, xmm29, xmm28, {sae}
	vcmpnge_ussh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpnge_ussh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpnge_ussh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpnge_ussh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpnge_ussh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpnge_ussh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpngesh	k5{k7}, xmm29, xmm28
	vcmpngesh	k5{k7}, xmm29, xmm28, {sae}
	vcmpngesh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpngesh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpngesh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpngesh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpngesh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpngesh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpngt_ussh	k5{k7}, xmm29, xmm28
	vcmpngt_ussh	k5{k7}, xmm29, xmm28, {sae}
	vcmpngt_ussh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpngt_ussh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpngt_ussh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpngt_ussh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpngt_ussh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpngt_ussh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpngtsh	k5{k7}, xmm29, xmm28
	vcmpngtsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpngtsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpngtsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpngtsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpngtsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpngtsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpngtsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpfalse_oqsh	k5{k7}, xmm29, xmm28
	vcmpfalse_oqsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpfalse_oqsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpfalse_oqsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpfalse_oqsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpfalse_oqsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpfalse_oqsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpfalse_oqsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpfalsesh	k5{k7}, xmm29, xmm28
	vcmpfalsesh	k5{k7}, xmm29, xmm28, {sae}
	vcmpfalsesh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpfalsesh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpfalsesh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpfalsesh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpfalsesh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpfalsesh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpneq_oqsh	k5{k7}, xmm29, xmm28
	vcmpneq_oqsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpneq_oqsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpneq_oqsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpneq_oqsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpneq_oqsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpneq_oqsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpneq_oqsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpge_ossh	k5{k7}, xmm29, xmm28
	vcmpge_ossh	k5{k7}, xmm29, xmm28, {sae}
	vcmpge_ossh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpge_ossh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpge_ossh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpge_ossh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpge_ossh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpge_ossh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpgesh	k5{k7}, xmm29, xmm28
	vcmpgesh	k5{k7}, xmm29, xmm28, {sae}
	vcmpgesh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpgesh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpgesh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpgesh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpgesh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpgesh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpgt_ossh	k5{k7}, xmm29, xmm28
	vcmpgt_ossh	k5{k7}, xmm29, xmm28, {sae}
	vcmpgt_ossh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpgt_ossh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpgt_ossh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpgt_ossh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpgt_ossh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpgt_ossh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpgtsh	k5{k7}, xmm29, xmm28
	vcmpgtsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpgtsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpgtsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpgtsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpgtsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpgtsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpgtsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmptrue_uqsh	k5{k7}, xmm29, xmm28
	vcmptrue_uqsh	k5{k7}, xmm29, xmm28, {sae}
	vcmptrue_uqsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmptrue_uqsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmptrue_uqsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmptrue_uqsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmptrue_uqsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmptrue_uqsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmptruesh	k5{k7}, xmm29, xmm28
	vcmptruesh	k5{k7}, xmm29, xmm28, {sae}
	vcmptruesh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmptruesh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmptruesh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmptruesh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmptruesh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmptruesh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpeq_ossh	k5{k7}, xmm29, xmm28
	vcmpeq_ossh	k5{k7}, xmm29, xmm28, {sae}
	vcmpeq_ossh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpeq_ossh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpeq_ossh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpeq_ossh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpeq_ossh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpeq_ossh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmplt_oqsh	k5{k7}, xmm29, xmm28
	vcmplt_oqsh	k5{k7}, xmm29, xmm28, {sae}
	vcmplt_oqsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmplt_oqsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmplt_oqsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmplt_oqsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmplt_oqsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmplt_oqsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmple_oqsh	k5{k7}, xmm29, xmm28
	vcmple_oqsh	k5{k7}, xmm29, xmm28, {sae}
	vcmple_oqsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmple_oqsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmple_oqsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmple_oqsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmple_oqsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmple_oqsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpunord_ssh	k5{k7}, xmm29, xmm28
	vcmpunord_ssh	k5{k7}, xmm29, xmm28, {sae}
	vcmpunord_ssh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpunord_ssh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpunord_ssh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpunord_ssh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpunord_ssh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpunord_ssh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpneq_ussh	k5{k7}, xmm29, xmm28
	vcmpneq_ussh	k5{k7}, xmm29, xmm28, {sae}
	vcmpneq_ussh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpneq_ussh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpneq_ussh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpneq_ussh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpneq_ussh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpneq_ussh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpnlt_uqsh	k5{k7}, xmm29, xmm28
	vcmpnlt_uqsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpnlt_uqsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpnlt_uqsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpnlt_uqsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpnlt_uqsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpnlt_uqsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpnlt_uqsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpnle_uqsh	k5{k7}, xmm29, xmm28
	vcmpnle_uqsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpnle_uqsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpnle_uqsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpnle_uqsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpnle_uqsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpnle_uqsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpnle_uqsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpord_ssh	k5{k7}, xmm29, xmm28
	vcmpord_ssh	k5{k7}, xmm29, xmm28, {sae}
	vcmpord_ssh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpord_ssh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpord_ssh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpord_ssh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpord_ssh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpord_ssh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpeq_ussh	k5{k7}, xmm29, xmm28
	vcmpeq_ussh	k5{k7}, xmm29, xmm28, {sae}
	vcmpeq_ussh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpeq_ussh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpeq_ussh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpeq_ussh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpeq_ussh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpeq_ussh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpnge_uqsh	k5{k7}, xmm29, xmm28
	vcmpnge_uqsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpnge_uqsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpnge_uqsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpnge_uqsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpnge_uqsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpnge_uqsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpnge_uqsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpngt_uqsh	k5{k7}, xmm29, xmm28
	vcmpngt_uqsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpngt_uqsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpngt_uqsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpngt_uqsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpngt_uqsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpngt_uqsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpngt_uqsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpfalse_ossh	k5{k7}, xmm29, xmm28
	vcmpfalse_ossh	k5{k7}, xmm29, xmm28, {sae}
	vcmpfalse_ossh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpfalse_ossh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpfalse_ossh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpfalse_ossh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpfalse_ossh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpfalse_ossh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpneq_ossh	k5{k7}, xmm29, xmm28
	vcmpneq_ossh	k5{k7}, xmm29, xmm28, {sae}
	vcmpneq_ossh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpneq_ossh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpneq_ossh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpneq_ossh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpneq_ossh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpneq_ossh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpge_oqsh	k5{k7}, xmm29, xmm28
	vcmpge_oqsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpge_oqsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpge_oqsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpge_oqsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpge_oqsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpge_oqsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpge_oqsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmpgt_oqsh	k5{k7}, xmm29, xmm28
	vcmpgt_oqsh	k5{k7}, xmm29, xmm28, {sae}
	vcmpgt_oqsh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmpgt_oqsh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmpgt_oqsh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmpgt_oqsh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmpgt_oqsh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmpgt_oqsh	k5{k7}, xmm29, WORD PTR [rdx-1032]
	vcmptrue_ussh	k5{k7}, xmm29, xmm28
	vcmptrue_ussh	k5{k7}, xmm29, xmm28, {sae}
	vcmptrue_ussh	k5{k7}, xmm29, WORD PTR [rcx]
	vcmptrue_ussh	k5{k7}, xmm29, WORD PTR [rax+r14*8+0x1234]
	vcmptrue_ussh	k5{k7}, xmm29, WORD PTR [rdx+1016]
	vcmptrue_ussh	k5{k7}, xmm29, WORD PTR [rdx+1024]
	vcmptrue_ussh	k5{k7}, xmm29, WORD PTR [rdx-1024]
	vcmptrue_ussh	k5{k7}, xmm29, WORD PTR [rdx-1032]
