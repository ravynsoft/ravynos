# Check 64bit AVX scalar instructions

	.allow_index_reg
	.text
_start:

# Tests for op xmm/mem64, xmm, xmm
	vfmadd132sd %xmm4,%xmm6,%xmm2
	vfmadd132sd (%rcx),%xmm6,%xmm2
	vfmadd213sd %xmm4,%xmm6,%xmm2
	vfmadd213sd (%rcx),%xmm6,%xmm2
	vfmadd231sd %xmm4,%xmm6,%xmm2
	vfmadd231sd (%rcx),%xmm6,%xmm2
	vfmsub132sd %xmm4,%xmm6,%xmm2
	vfmsub132sd (%rcx),%xmm6,%xmm2
	vfmsub213sd %xmm4,%xmm6,%xmm2
	vfmsub213sd (%rcx),%xmm6,%xmm2
	vfmsub231sd %xmm4,%xmm6,%xmm2
	vfmsub231sd (%rcx),%xmm6,%xmm2
	vfnmadd132sd %xmm4,%xmm6,%xmm2
	vfnmadd132sd (%rcx),%xmm6,%xmm2
	vfnmadd213sd %xmm4,%xmm6,%xmm2
	vfnmadd213sd (%rcx),%xmm6,%xmm2
	vfnmadd231sd %xmm4,%xmm6,%xmm2
	vfnmadd231sd (%rcx),%xmm6,%xmm2
	vfnmsub132sd %xmm4,%xmm6,%xmm2
	vfnmsub132sd (%rcx),%xmm6,%xmm2
	vfnmsub213sd %xmm4,%xmm6,%xmm2
	vfnmsub213sd (%rcx),%xmm6,%xmm2
	vfnmsub231sd %xmm4,%xmm6,%xmm2
	vfnmsub231sd (%rcx),%xmm6,%xmm2

# Tests for op xmm/mem32, xmm, xmm
	vfmadd132ss %xmm4,%xmm6,%xmm2
	vfmadd132ss (%rcx),%xmm6,%xmm2
	vfmadd213ss %xmm4,%xmm6,%xmm2
	vfmadd213ss (%rcx),%xmm6,%xmm2
	vfmadd231ss %xmm4,%xmm6,%xmm2
	vfmadd231ss (%rcx),%xmm6,%xmm2
	vfmsub132ss %xmm4,%xmm6,%xmm2
	vfmsub132ss (%rcx),%xmm6,%xmm2
	vfmsub213ss %xmm4,%xmm6,%xmm2
	vfmsub213ss (%rcx),%xmm6,%xmm2
	vfmsub231ss %xmm4,%xmm6,%xmm2
	vfmsub231ss (%rcx),%xmm6,%xmm2
	vfnmadd132ss %xmm4,%xmm6,%xmm2
	vfnmadd132ss (%rcx),%xmm6,%xmm2
	vfnmadd213ss %xmm4,%xmm6,%xmm2
	vfnmadd213ss (%rcx),%xmm6,%xmm2
	vfnmadd231ss %xmm4,%xmm6,%xmm2
	vfnmadd231ss (%rcx),%xmm6,%xmm2
	vfnmsub132ss %xmm4,%xmm6,%xmm2
	vfnmsub132ss (%rcx),%xmm6,%xmm2
	vfnmsub213ss %xmm4,%xmm6,%xmm2
	vfnmsub213ss (%rcx),%xmm6,%xmm2
	vfnmsub231ss %xmm4,%xmm6,%xmm2
	vfnmsub231ss (%rcx),%xmm6,%xmm2

	.intel_syntax noprefix

# Tests for op xmm/mem64, xmm, xmm
	vfmadd132sd xmm2,xmm6,xmm4
	vfmadd132sd xmm2,xmm6,QWORD PTR [rcx]
	vfmadd132sd xmm2,xmm6,[rcx]
	vfmadd213sd xmm2,xmm6,xmm4
	vfmadd213sd xmm2,xmm6,QWORD PTR [rcx]
	vfmadd213sd xmm2,xmm6,[rcx]
	vfmadd231sd xmm2,xmm6,xmm4
	vfmadd231sd xmm2,xmm6,QWORD PTR [rcx]
	vfmadd231sd xmm2,xmm6,[rcx]
	vfmsub132sd xmm2,xmm6,xmm4
	vfmsub132sd xmm2,xmm6,QWORD PTR [rcx]
	vfmsub132sd xmm2,xmm6,[rcx]
	vfmsub213sd xmm2,xmm6,xmm4
	vfmsub213sd xmm2,xmm6,QWORD PTR [rcx]
	vfmsub213sd xmm2,xmm6,[rcx]
	vfmsub231sd xmm2,xmm6,xmm4
	vfmsub231sd xmm2,xmm6,QWORD PTR [rcx]
	vfmsub231sd xmm2,xmm6,[rcx]
	vfnmadd132sd xmm2,xmm6,xmm4
	vfnmadd132sd xmm2,xmm6,QWORD PTR [rcx]
	vfnmadd132sd xmm2,xmm6,[rcx]
	vfnmadd213sd xmm2,xmm6,xmm4
	vfnmadd213sd xmm2,xmm6,QWORD PTR [rcx]
	vfnmadd213sd xmm2,xmm6,[rcx]
	vfnmadd231sd xmm2,xmm6,xmm4
	vfnmadd231sd xmm2,xmm6,QWORD PTR [rcx]
	vfnmadd231sd xmm2,xmm6,[rcx]
	vfnmsub132sd xmm2,xmm6,xmm4
	vfnmsub132sd xmm2,xmm6,QWORD PTR [rcx]
	vfnmsub132sd xmm2,xmm6,[rcx]
	vfnmsub213sd xmm2,xmm6,xmm4
	vfnmsub213sd xmm2,xmm6,QWORD PTR [rcx]
	vfnmsub213sd xmm2,xmm6,[rcx]
	vfnmsub231sd xmm2,xmm6,xmm4
	vfnmsub231sd xmm2,xmm6,QWORD PTR [rcx]
	vfnmsub231sd xmm2,xmm6,[rcx]

# Tests for op xmm/mem32, xmm, xmm
	vfmadd132ss xmm2,xmm6,xmm4
	vfmadd132ss xmm2,xmm6,DWORD PTR [rcx]
	vfmadd132ss xmm2,xmm6,[rcx]
	vfmadd213ss xmm2,xmm6,xmm4
	vfmadd213ss xmm2,xmm6,DWORD PTR [rcx]
	vfmadd213ss xmm2,xmm6,[rcx]
	vfmadd231ss xmm2,xmm6,xmm4
	vfmadd231ss xmm2,xmm6,DWORD PTR [rcx]
	vfmadd231ss xmm2,xmm6,[rcx]
	vfmsub132ss xmm2,xmm6,xmm4
	vfmsub132ss xmm2,xmm6,DWORD PTR [rcx]
	vfmsub132ss xmm2,xmm6,[rcx]
	vfmsub213ss xmm2,xmm6,xmm4
	vfmsub213ss xmm2,xmm6,DWORD PTR [rcx]
	vfmsub213ss xmm2,xmm6,[rcx]
	vfmsub231ss xmm2,xmm6,xmm4
	vfmsub231ss xmm2,xmm6,DWORD PTR [rcx]
	vfmsub231ss xmm2,xmm6,[rcx]
	vfnmadd132ss xmm2,xmm6,xmm4
	vfnmadd132ss xmm2,xmm6,DWORD PTR [rcx]
	vfnmadd132ss xmm2,xmm6,[rcx]
	vfnmadd213ss xmm2,xmm6,xmm4
	vfnmadd213ss xmm2,xmm6,DWORD PTR [rcx]
	vfnmadd213ss xmm2,xmm6,[rcx]
	vfnmadd231ss xmm2,xmm6,xmm4
	vfnmadd231ss xmm2,xmm6,DWORD PTR [rcx]
	vfnmadd231ss xmm2,xmm6,[rcx]
	vfnmsub132ss xmm2,xmm6,xmm4
	vfnmsub132ss xmm2,xmm6,DWORD PTR [rcx]
	vfnmsub132ss xmm2,xmm6,[rcx]
	vfnmsub213ss xmm2,xmm6,xmm4
	vfnmsub213ss xmm2,xmm6,DWORD PTR [rcx]
	vfnmsub213ss xmm2,xmm6,[rcx]
	vfnmsub231ss xmm2,xmm6,xmm4
	vfnmsub231ss xmm2,xmm6,DWORD PTR [rcx]
	vfnmsub231ss xmm2,xmm6,[rcx]
