	.syntax unified
	.text

.macro	entry	name, vis, entry_fct
	.align	2
	.\vis	\name
	.\vis	__acle_se_\name
	.thumb
	.thumb_func
	.type	\name, %function
	.type	__acle_se_\name, %function
\name:
.ifnb \entry_fct
	\entry_fct
.endif
__acle_se_\name:
	nop
	.size	\name, .-\name
	.size	__acle_se_\name, .-__acle_se_\name
.endm

	@ Valid setups for veneer generation
.if (VER >= 2)
	entry exported_entry_veneer1, global
.endif
.if (VER != 4)
	entry exported_entry_veneer2, global
.else
	entry exported_entry_veneer2, weak
.endif
.if (VER != 2)
	entry exported_entry_veneer3, global
.endif
.if (VER > 1)
	entry exported_entry_veneer4, global
.endif

	@ Valid setup for entry function without veneer generation
	entry exported_entry_fct1, global, sg
.if (VER != 4)
	entry exported_entry_fct2, global, sg
.else
	@ Invalid setup for entry function without veneer generation
	entry exported_entry_fct2, global, nop
.endif

	@ Normal symbol not exported to SG import library
	.align	2
	.global	not_exported_fct1
	.type	not_exported_fct1, %function
not_exported_fct1:
	nop
	.size	not_exported_fct1, .-not_exported_fct1

.ifdef CHECK_ERRORS
	@ Invalid setups for export to SG import library
	.align	2
	.global	__acle_se_not_exported_fct2
	.type	__acle_se_not_exported_fct2, %function
__acle_se_not_exported_fct2:
	nop
	.size	__acle_se_not_exported_fct2, .-__acle_se_not_exported_fct2

	.align	2
	.global	__acle_se_not_exported_pseudoentry_var
	.global	not_exported_pseudoentry_var
	.data
	.type	__acle_se_not_exported_pseudoentry_var, %object
	.type	not_exported_pseudoentry_var, %object
	.size	not_exported_pseudoentry_var, 4
	.size	__acle_se_not_exported_pseudoentry_var, 4
__acle_se_not_exported_pseudoentry_var:
not_exported_pseudoentry_var:
	.word	42
.endif
