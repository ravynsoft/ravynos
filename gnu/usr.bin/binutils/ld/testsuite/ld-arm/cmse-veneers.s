	.syntax unified
	.thumb
	.file	"foo.c"
	.text

.macro	decltype	name, type
.ifc \type,object
	.data
.else
	.thumb
	.thumb_func
.endif
	.type	\name, %\type
.endm


.macro	entry	name, type, vis, typespc, visspc, entry_fct
	.align	2
.ifb \visspc
	.\vis	__acle_se_\name
.else
	.\visspc	__acle_se_\name
.endif
	.\vis	\name
	.thumb
	.thumb_func
.ifb \typespc
	decltype	__acle_se_\name, \type
.else
	decltype	__acle_se_\name, \typespc
.endif
	decltype	\name, \type
__acle_se_\name:
	\entry_fct
\name:
.ifc \type,object
	.word 42
.else
	nop
.endif
	.size	\name, .-\name
	.size	__acle_se_\name, .-__acle_se_\name
.endm


.ifndef CHECK_ERRORS
	@ Valid setups for veneer generation
	entry glob_entry_veneer1, function, global
	entry weak_entry_veneer1, function, weak
	entry glob_entry_veneer2, function, global, visspc=weak
	entry weak_entry_veneer2, function, weak, visspc=global

	@ Valid setup for entry function without SG veneer
	entry glob_entry_fct, function, global, entry_fct=nop

	@ Valid setup for entry function with absolute address
	.align 2
	.global	__acle_se_abs_entry_fct
	.global	abs_entry_fct
	.type	__acle_se_abs_entry_fct, %function
	.type	abs_entry_fct, %function
__acle_se_abs_entry_fct = 0x10000
abs_entry_fct = 0x10004
	.size	abs_entry_fct, 0
	.size	__acle_se_abs_entry_fct, 0
.else
	@ Invalid setups for veneer generation (visibility)
	entry loc_entry_veneer1, function, local
	entry loc_entry_veneer2, function, global, visspc=local
	entry loc_entry_veneer3, function, local, visspc=global
	entry loc_entry_veneer4, function, weak, visspc=local
	entry loc_entry_veneer5, function, local, visspc=weak

	@ Invalid setups for veneer generation (absent standard symbol)
	.align	2
	.global	__acle_se_fake_entry_veneer1
	.thumb
	.thumb_func
	.type	__acle_se_fake_entry_veneer1, %function
__acle_se_fake_entry_veneer1:
	nop
	.size	__acle_se_fake_entry_veneer1, .-__acle_se_fake_entry_veneer1

	@ Invalid setups for veneer generation (type)
	entry obj_entry_veneer1, object, global, typespc=function
	entry obj_entry_veneer2, function, global, typespc=object

	@ Invalid setup for veneer generation (sections)
	.section .text.sub1
	.align	2
	.thumb
	.thumb_func
	.global	__acle_se_fake_entry_veneer2
	.type	__acle_se_fake_entry_veneer2, %function
__acle_se_fake_entry_veneer2:
	nop
	.size	__acle_se_fake_entry_veneer2, .-__acle_se_fake_entry_veneer2
	.section .text.sub2
	.align	2
	.thumb
	.thumb_func
	.global	fake_entry_veneer2
	.type	fake_entry_veneer2, %function
fake_entry_veneer2:
	nop
	.size	fake_entry_veneer2, .-fake_entry_veneer2
.endif
