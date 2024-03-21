.text

.variant_pcs f_spec_global_default_def
.variant_pcs f_spec_global_default_undef
.variant_pcs f_spec_global_hidden_def
.variant_pcs f_spec_local2
.variant_pcs f_spec_global_default_ifunc
.variant_pcs f_spec_global_hidden_ifunc
.variant_pcs f_spec_local2_ifunc

.global f_spec_global_default_def
.global f_spec_global_default_undef
.global f_spec_global_hidden_def
.global f_spec_global_default_ifunc
.global f_spec_global_hidden_ifunc
.global f_base_global_default_def
.global f_base_global_default_undef
.global f_base_global_hidden_def
.global f_base_global_default_ifunc
.global f_base_global_hidden_ifunc

.hidden f_spec_global_hidden_def
.hidden f_spec_global_hidden_ifunc
.hidden f_base_global_hidden_def
.hidden f_base_global_hidden_ifunc

.type f_spec_local2_ifunc, %gnu_indirect_function
.type f_base_local2_ifunc, %gnu_indirect_function

f_spec_local2:
f_base_local2:
f_spec_local2_ifunc:
f_base_local2_ifunc:
	bl f_spec_global_default_def
	bl f_spec_global_default_undef
	bl f_spec_global_hidden_def
	bl f_spec_local2
	bl f_base_global_default_def
	bl f_base_global_default_undef
	bl f_base_global_hidden_def
	bl f_base_local2
	bl f_spec_global_default_ifunc
	bl f_spec_global_hidden_ifunc
	bl f_spec_local2_ifunc
	bl f_base_global_default_ifunc
	bl f_base_global_hidden_ifunc
	bl f_base_local2_ifunc
