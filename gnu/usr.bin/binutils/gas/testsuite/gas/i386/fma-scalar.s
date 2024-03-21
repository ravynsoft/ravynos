# Check AVX scalar instructions

	.allow_index_reg
	.text
_start:

# Tests for op xmm/mem64, xmm, xmm
	vfmadd132sd %xmm4,%xmm6,%xmm2
	vfmadd132sd (%ecx),%xmm6,%xmm2
	vfmadd213sd %xmm4,%xmm6,%xmm2
	vfmadd213sd (%ecx),%xmm6,%xmm2
	vfmadd231sd %xmm4,%xmm6,%xmm2
	vfmadd231sd (%ecx),%xmm6,%xmm2
	vfmsub132sd %xmm4,%xmm6,%xmm2
	vfmsub132sd (%ecx),%xmm6,%xmm2
	vfmsub213sd %xmm4,%xmm6,%xmm2
	vfmsub213sd (%ecx),%xmm6,%xmm2
	vfmsub231sd %xmm4,%xmm6,%xmm2
	vfmsub231sd (%ecx),%xmm6,%xmm2
	vfnmadd132sd %xmm4,%xmm6,%xmm2
	vfnmadd132sd (%ecx),%xmm6,%xmm2
	vfnmadd213sd %xmm4,%xmm6,%xmm2
	vfnmadd213sd (%ecx),%xmm6,%xmm2
	vfnmadd231sd %xmm4,%xmm6,%xmm2
	vfnmadd231sd (%ecx),%xmm6,%xmm2
	vfnmsub132sd %xmm4,%xmm6,%xmm2
	vfnmsub132sd (%ecx),%xmm6,%xmm2
	vfnmsub213sd %xmm4,%xmm6,%xmm2
	vfnmsub213sd (%ecx),%xmm6,%xmm2
	vfnmsub231sd %xmm4,%xmm6,%xmm2
	vfnmsub231sd (%ecx),%xmm6,%xmm2

# Tests for op xmm/mem32, xmm, xmm
	vfmadd132ss %xmm4,%xmm6,%xmm2
	vfmadd132ss (%ecx),%xmm6,%xmm2
	vfmadd213ss %xmm4,%xmm6,%xmm2
	vfmadd213ss (%ecx),%xmm6,%xmm2
	vfmadd231ss %xmm4,%xmm6,%xmm2
	vfmadd231ss (%ecx),%xmm6,%xmm2
	vfmsub132ss %xmm4,%xmm6,%xmm2
	vfmsub132ss (%ecx),%xmm6,%xmm2
	vfmsub213ss %xmm4,%xmm6,%xmm2
	vfmsub213ss (%ecx),%xmm6,%xmm2
	vfmsub231ss %xmm4,%xmm6,%xmm2
	vfmsub231ss (%ecx),%xmm6,%xmm2
	vfnmadd132ss %xmm4,%xmm6,%xmm2
	vfnmadd132ss (%ecx),%xmm6,%xmm2
	vfnmadd213ss %xmm4,%xmm6,%xmm2
	vfnmadd213ss (%ecx),%xmm6,%xmm2
	vfnmadd231ss %xmm4,%xmm6,%xmm2
	vfnmadd231ss (%ecx),%xmm6,%xmm2
	vfnmsub132ss %xmm4,%xmm6,%xmm2
	vfnmsub132ss (%ecx),%xmm6,%xmm2
	vfnmsub213ss %xmm4,%xmm6,%xmm2
	vfnmsub213ss (%ecx),%xmm6,%xmm2
	vfnmsub231ss %xmm4,%xmm6,%xmm2
	vfnmsub231ss (%ecx),%xmm6,%xmm2

	.intel_syntax noprefix

# Tests for op xmm/mem64, xmm, xmm
	vfmadd132sd xmm2,xmm6,xmm4
	vfmadd132sd xmm2,xmm6,QWORD PTR [ecx]
	vfmadd132sd xmm2,xmm6,[ecx]
	vfmadd213sd xmm2,xmm6,xmm4
	vfmadd213sd xmm2,xmm6,QWORD PTR [ecx]
	vfmadd213sd xmm2,xmm6,[ecx]
	vfmadd231sd xmm2,xmm6,xmm4
	vfmadd231sd xmm2,xmm6,QWORD PTR [ecx]
	vfmadd231sd xmm2,xmm6,[ecx]
	vfmsub132sd xmm2,xmm6,xmm4
	vfmsub132sd xmm2,xmm6,QWORD PTR [ecx]
	vfmsub132sd xmm2,xmm6,[ecx]
	vfmsub213sd xmm2,xmm6,xmm4
	vfmsub213sd xmm2,xmm6,QWORD PTR [ecx]
	vfmsub213sd xmm2,xmm6,[ecx]
	vfmsub231sd xmm2,xmm6,xmm4
	vfmsub231sd xmm2,xmm6,QWORD PTR [ecx]
	vfmsub231sd xmm2,xmm6,[ecx]
	vfnmadd132sd xmm2,xmm6,xmm4
	vfnmadd132sd xmm2,xmm6,QWORD PTR [ecx]
	vfnmadd132sd xmm2,xmm6,[ecx]
	vfnmadd213sd xmm2,xmm6,xmm4
	vfnmadd213sd xmm2,xmm6,QWORD PTR [ecx]
	vfnmadd213sd xmm2,xmm6,[ecx]
	vfnmadd231sd xmm2,xmm6,xmm4
	vfnmadd231sd xmm2,xmm6,QWORD PTR [ecx]
	vfnmadd231sd xmm2,xmm6,[ecx]
	vfnmsub132sd xmm2,xmm6,xmm4
	vfnmsub132sd xmm2,xmm6,QWORD PTR [ecx]
	vfnmsub132sd xmm2,xmm6,[ecx]
	vfnmsub213sd xmm2,xmm6,xmm4
	vfnmsub213sd xmm2,xmm6,QWORD PTR [ecx]
	vfnmsub213sd xmm2,xmm6,[ecx]
	vfnmsub231sd xmm2,xmm6,xmm4
	vfnmsub231sd xmm2,xmm6,QWORD PTR [ecx]
	vfnmsub231sd xmm2,xmm6,[ecx]

# Tests for op xmm/mem32, xmm, xmm
	vfmadd132ss xmm2,xmm6,xmm4
	vfmadd132ss xmm2,xmm6,DWORD PTR [ecx]
	vfmadd132ss xmm2,xmm6,[ecx]
	vfmadd213ss xmm2,xmm6,xmm4
	vfmadd213ss xmm2,xmm6,DWORD PTR [ecx]
	vfmadd213ss xmm2,xmm6,[ecx]
	vfmadd231ss xmm2,xmm6,xmm4
	vfmadd231ss xmm2,xmm6,DWORD PTR [ecx]
	vfmadd231ss xmm2,xmm6,[ecx]
	vfmsub132ss xmm2,xmm6,xmm4
	vfmsub132ss xmm2,xmm6,DWORD PTR [ecx]
	vfmsub132ss xmm2,xmm6,[ecx]
	vfmsub213ss xmm2,xmm6,xmm4
	vfmsub213ss xmm2,xmm6,DWORD PTR [ecx]
	vfmsub213ss xmm2,xmm6,[ecx]
	vfmsub231ss xmm2,xmm6,xmm4
	vfmsub231ss xmm2,xmm6,DWORD PTR [ecx]
	vfmsub231ss xmm2,xmm6,[ecx]
	vfnmadd132ss xmm2,xmm6,xmm4
	vfnmadd132ss xmm2,xmm6,DWORD PTR [ecx]
	vfnmadd132ss xmm2,xmm6,[ecx]
	vfnmadd213ss xmm2,xmm6,xmm4
	vfnmadd213ss xmm2,xmm6,DWORD PTR [ecx]
	vfnmadd213ss xmm2,xmm6,[ecx]
	vfnmadd231ss xmm2,xmm6,xmm4
	vfnmadd231ss xmm2,xmm6,DWORD PTR [ecx]
	vfnmadd231ss xmm2,xmm6,[ecx]
	vfnmsub132ss xmm2,xmm6,xmm4
	vfnmsub132ss xmm2,xmm6,DWORD PTR [ecx]
	vfnmsub132ss xmm2,xmm6,[ecx]
	vfnmsub213ss xmm2,xmm6,xmm4
	vfnmsub213ss xmm2,xmm6,DWORD PTR [ecx]
	vfnmsub213ss xmm2,xmm6,[ecx]
	vfnmsub231ss xmm2,xmm6,xmm4
	vfnmsub231ss xmm2,xmm6,DWORD PTR [ecx]
	vfnmsub231ss xmm2,xmm6,[ecx]
