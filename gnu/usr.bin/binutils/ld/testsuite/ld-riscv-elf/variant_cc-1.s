.text

.variant_cc cc_global_default_def
.variant_cc cc_global_default_undef
.variant_cc cc_global_default_ifunc
.variant_cc cc_global_hidden_def
.variant_cc cc_global_hidden_ifunc
.variant_cc cc_local
.variant_cc cc_local_ifunc

.global cc_global_default_def
.global cc_global_default_undef
.global cc_global_default_ifunc
.global cc_global_hidden_def
.global cc_global_hidden_ifunc
.global nocc_global_default_def
.global nocc_global_default_undef
.global nocc_global_default_ifunc
.global nocc_global_hidden_def
.global nocc_global_hidden_ifunc

.hidden cc_global_hidden_def
.hidden cc_global_hidden_ifunc
.hidden nocc_global_hidden_def
.hidden nocc_global_hidden_ifunc

.type cc_global_default_ifunc, %gnu_indirect_function
.type cc_global_hidden_ifunc, %gnu_indirect_function
.type cc_local_ifunc, %gnu_indirect_function
.type nocc_global_default_ifunc, %gnu_indirect_function
.type nocc_global_hidden_ifunc, %gnu_indirect_function
.type nocc_local_ifunc, %gnu_indirect_function

cc_global_default_def:
# cc_global_default_undef:
cc_global_hidden_def:
cc_global_default_ifunc:
cc_global_hidden_ifunc:
cc_local:
cc_local_ifunc:
nocc_global_default_def:
# nocc_global_default_undef:
nocc_global_hidden_def:
nocc_global_default_ifunc:
nocc_global_hidden_ifunc:
nocc_local:
nocc_local_ifunc:
	call cc_global_default_def
	call cc_global_default_undef
	call cc_global_hidden_def
	call cc_global_default_ifunc
	call cc_global_hidden_ifunc
	call cc_local
	call cc_local_ifunc
	call nocc_global_default_def
	call nocc_global_default_undef
	call nocc_global_hidden_def
	call nocc_global_default_ifunc
	call nocc_global_hidden_ifunc
	call nocc_local
	call nocc_local_ifunc
