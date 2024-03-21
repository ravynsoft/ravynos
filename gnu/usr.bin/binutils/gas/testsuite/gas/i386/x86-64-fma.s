# Check 64bit FMA instructions

	.allow_index_reg
	.text
_start:

# Tests for op ymm/mem256, ymm, ymm
	vfmadd132pd %ymm4,%ymm6,%ymm2
	vfmadd132pd (%rcx),%ymm6,%ymm2
	vfmadd132ps %ymm4,%ymm6,%ymm2
	vfmadd132ps (%rcx),%ymm6,%ymm2
	vfmadd213pd %ymm4,%ymm6,%ymm2
	vfmadd213pd (%rcx),%ymm6,%ymm2
	vfmadd213ps %ymm4,%ymm6,%ymm2
	vfmadd213ps (%rcx),%ymm6,%ymm2
	vfmadd231pd %ymm4,%ymm6,%ymm2
	vfmadd231pd (%rcx),%ymm6,%ymm2
	vfmadd231ps %ymm4,%ymm6,%ymm2
	vfmadd231ps (%rcx),%ymm6,%ymm2
	vfmaddsub132pd %ymm4,%ymm6,%ymm2
	vfmaddsub132pd (%rcx),%ymm6,%ymm2
	vfmaddsub132ps %ymm4,%ymm6,%ymm2
	vfmaddsub132ps (%rcx),%ymm6,%ymm2
	vfmaddsub213pd %ymm4,%ymm6,%ymm2
	vfmaddsub213pd (%rcx),%ymm6,%ymm2
	vfmaddsub213ps %ymm4,%ymm6,%ymm2
	vfmaddsub213ps (%rcx),%ymm6,%ymm2
	vfmaddsub231pd %ymm4,%ymm6,%ymm2
	vfmaddsub231pd (%rcx),%ymm6,%ymm2
	vfmaddsub231ps %ymm4,%ymm6,%ymm2
	vfmaddsub231ps (%rcx),%ymm6,%ymm2
	vfmsubadd132pd %ymm4,%ymm6,%ymm2
	vfmsubadd132pd (%rcx),%ymm6,%ymm2
	vfmsubadd132ps %ymm4,%ymm6,%ymm2
	vfmsubadd132ps (%rcx),%ymm6,%ymm2
	vfmsubadd213pd %ymm4,%ymm6,%ymm2
	vfmsubadd213pd (%rcx),%ymm6,%ymm2
	vfmsubadd213ps %ymm4,%ymm6,%ymm2
	vfmsubadd213ps (%rcx),%ymm6,%ymm2
	vfmsubadd231pd %ymm4,%ymm6,%ymm2
	vfmsubadd231pd (%rcx),%ymm6,%ymm2
	vfmsubadd231ps %ymm4,%ymm6,%ymm2
	vfmsubadd231ps (%rcx),%ymm6,%ymm2
	vfmsub132pd %ymm4,%ymm6,%ymm2
	vfmsub132pd (%rcx),%ymm6,%ymm2
	vfmsub132ps %ymm4,%ymm6,%ymm2
	vfmsub132ps (%rcx),%ymm6,%ymm2
	vfmsub213pd %ymm4,%ymm6,%ymm2
	vfmsub213pd (%rcx),%ymm6,%ymm2
	vfmsub213ps %ymm4,%ymm6,%ymm2
	vfmsub213ps (%rcx),%ymm6,%ymm2
	vfmsub231pd %ymm4,%ymm6,%ymm2
	vfmsub231pd (%rcx),%ymm6,%ymm2
	vfmsub231ps %ymm4,%ymm6,%ymm2
	vfmsub231ps (%rcx),%ymm6,%ymm2
	vfnmadd132pd %ymm4,%ymm6,%ymm2
	vfnmadd132pd (%rcx),%ymm6,%ymm2
	vfnmadd132ps %ymm4,%ymm6,%ymm2
	vfnmadd132ps (%rcx),%ymm6,%ymm2
	vfnmadd213pd %ymm4,%ymm6,%ymm2
	vfnmadd213pd (%rcx),%ymm6,%ymm2
	vfnmadd213ps %ymm4,%ymm6,%ymm2
	vfnmadd213ps (%rcx),%ymm6,%ymm2
	vfnmadd231pd %ymm4,%ymm6,%ymm2
	vfnmadd231pd (%rcx),%ymm6,%ymm2
	vfnmadd231ps %ymm4,%ymm6,%ymm2
	vfnmadd231ps (%rcx),%ymm6,%ymm2
	vfnmsub132pd %ymm4,%ymm6,%ymm2
	vfnmsub132pd (%rcx),%ymm6,%ymm2
	vfnmsub132ps %ymm4,%ymm6,%ymm2
	vfnmsub132ps (%rcx),%ymm6,%ymm2
	vfnmsub213pd %ymm4,%ymm6,%ymm2
	vfnmsub213pd (%rcx),%ymm6,%ymm2
	vfnmsub213ps %ymm4,%ymm6,%ymm2
	vfnmsub213ps (%rcx),%ymm6,%ymm2
	vfnmsub231pd %ymm4,%ymm6,%ymm2
	vfnmsub231pd (%rcx),%ymm6,%ymm2
	vfnmsub231ps %ymm4,%ymm6,%ymm2
	vfnmsub231ps (%rcx),%ymm6,%ymm2

# Tests for op xmm/mem128, xmm, xmm
	vfmadd132pd %xmm4,%xmm6,%xmm2
	vfmadd132pd (%rcx),%xmm6,%xmm7
	vfmadd132ps %xmm4,%xmm6,%xmm2
	vfmadd132ps (%rcx),%xmm6,%xmm7
	vfmadd213pd %xmm4,%xmm6,%xmm2
	vfmadd213pd (%rcx),%xmm6,%xmm7
	vfmadd213ps %xmm4,%xmm6,%xmm2
	vfmadd213ps (%rcx),%xmm6,%xmm7
	vfmadd231pd %xmm4,%xmm6,%xmm2
	vfmadd231pd (%rcx),%xmm6,%xmm7
	vfmadd231ps %xmm4,%xmm6,%xmm2
	vfmadd231ps (%rcx),%xmm6,%xmm7
	vfmaddsub132pd %xmm4,%xmm6,%xmm2
	vfmaddsub132pd (%rcx),%xmm6,%xmm7
	vfmaddsub132ps %xmm4,%xmm6,%xmm2
	vfmaddsub132ps (%rcx),%xmm6,%xmm7
	vfmaddsub213pd %xmm4,%xmm6,%xmm2
	vfmaddsub213pd (%rcx),%xmm6,%xmm7
	vfmaddsub213ps %xmm4,%xmm6,%xmm2
	vfmaddsub213ps (%rcx),%xmm6,%xmm7
	vfmaddsub231pd %xmm4,%xmm6,%xmm2
	vfmaddsub231pd (%rcx),%xmm6,%xmm7
	vfmaddsub231ps %xmm4,%xmm6,%xmm2
	vfmaddsub231ps (%rcx),%xmm6,%xmm7
	vfmsubadd132pd %xmm4,%xmm6,%xmm2
	vfmsubadd132pd (%rcx),%xmm6,%xmm7
	vfmsubadd132ps %xmm4,%xmm6,%xmm2
	vfmsubadd132ps (%rcx),%xmm6,%xmm7
	vfmsubadd213pd %xmm4,%xmm6,%xmm2
	vfmsubadd213pd (%rcx),%xmm6,%xmm7
	vfmsubadd213ps %xmm4,%xmm6,%xmm2
	vfmsubadd213ps (%rcx),%xmm6,%xmm7
	vfmsubadd231pd %xmm4,%xmm6,%xmm2
	vfmsubadd231pd (%rcx),%xmm6,%xmm7
	vfmsubadd231ps %xmm4,%xmm6,%xmm2
	vfmsubadd231ps (%rcx),%xmm6,%xmm7
	vfmsub132pd %xmm4,%xmm6,%xmm2
	vfmsub132pd (%rcx),%xmm6,%xmm7
	vfmsub132ps %xmm4,%xmm6,%xmm2
	vfmsub132ps (%rcx),%xmm6,%xmm7
	vfmsub213pd %xmm4,%xmm6,%xmm2
	vfmsub213pd (%rcx),%xmm6,%xmm7
	vfmsub213ps %xmm4,%xmm6,%xmm2
	vfmsub213ps (%rcx),%xmm6,%xmm7
	vfmsub231pd %xmm4,%xmm6,%xmm2
	vfmsub231pd (%rcx),%xmm6,%xmm7
	vfmsub231ps %xmm4,%xmm6,%xmm2
	vfmsub231ps (%rcx),%xmm6,%xmm7
	vfnmadd132pd %xmm4,%xmm6,%xmm2
	vfnmadd132pd (%rcx),%xmm6,%xmm7
	vfnmadd132ps %xmm4,%xmm6,%xmm2
	vfnmadd132ps (%rcx),%xmm6,%xmm7
	vfnmadd213pd %xmm4,%xmm6,%xmm2
	vfnmadd213pd (%rcx),%xmm6,%xmm7
	vfnmadd213ps %xmm4,%xmm6,%xmm2
	vfnmadd213ps (%rcx),%xmm6,%xmm7
	vfnmadd231pd %xmm4,%xmm6,%xmm2
	vfnmadd231pd (%rcx),%xmm6,%xmm7
	vfnmadd231ps %xmm4,%xmm6,%xmm2
	vfnmadd231ps (%rcx),%xmm6,%xmm7
	vfnmsub132pd %xmm4,%xmm6,%xmm2
	vfnmsub132pd (%rcx),%xmm6,%xmm7
	vfnmsub132ps %xmm4,%xmm6,%xmm2
	vfnmsub132ps (%rcx),%xmm6,%xmm7
	vfnmsub213pd %xmm4,%xmm6,%xmm2
	vfnmsub213pd (%rcx),%xmm6,%xmm7
	vfnmsub213ps %xmm4,%xmm6,%xmm2
	vfnmsub213ps (%rcx),%xmm6,%xmm7
	vfnmsub231pd %xmm4,%xmm6,%xmm2
	vfnmsub231pd (%rcx),%xmm6,%xmm7
	vfnmsub231ps %xmm4,%xmm6,%xmm2
	vfnmsub231ps (%rcx),%xmm6,%xmm7

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

# Tests for op ymm/mem256, ymm, ymm
	vfmadd132pd ymm2,ymm6,ymm4
	vfmadd132pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfmadd132pd ymm2,ymm6,[rcx]
	vfmadd132ps ymm2,ymm6,ymm4
	vfmadd132ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfmadd132ps ymm2,ymm6,[rcx]
	vfmadd213pd ymm2,ymm6,ymm4
	vfmadd213pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfmadd213pd ymm2,ymm6,[rcx]
	vfmadd213ps ymm2,ymm6,ymm4
	vfmadd213ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfmadd213ps ymm2,ymm6,[rcx]
	vfmadd231pd ymm2,ymm6,ymm4
	vfmadd231pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfmadd231pd ymm2,ymm6,[rcx]
	vfmadd231ps ymm2,ymm6,ymm4
	vfmadd231ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfmadd231ps ymm2,ymm6,[rcx]
	vfmaddsub132pd ymm2,ymm6,ymm4
	vfmaddsub132pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfmaddsub132pd ymm2,ymm6,[rcx]
	vfmaddsub132ps ymm2,ymm6,ymm4
	vfmaddsub132ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfmaddsub132ps ymm2,ymm6,[rcx]
	vfmaddsub213pd ymm2,ymm6,ymm4
	vfmaddsub213pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfmaddsub213pd ymm2,ymm6,[rcx]
	vfmaddsub213ps ymm2,ymm6,ymm4
	vfmaddsub213ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfmaddsub213ps ymm2,ymm6,[rcx]
	vfmaddsub231pd ymm2,ymm6,ymm4
	vfmaddsub231pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfmaddsub231pd ymm2,ymm6,[rcx]
	vfmaddsub231ps ymm2,ymm6,ymm4
	vfmaddsub231ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfmaddsub231ps ymm2,ymm6,[rcx]
	vfmsubadd132pd ymm2,ymm6,ymm4
	vfmsubadd132pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfmsubadd132pd ymm2,ymm6,[rcx]
	vfmsubadd132ps ymm2,ymm6,ymm4
	vfmsubadd132ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfmsubadd132ps ymm2,ymm6,[rcx]
	vfmsubadd213pd ymm2,ymm6,ymm4
	vfmsubadd213pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfmsubadd213pd ymm2,ymm6,[rcx]
	vfmsubadd213ps ymm2,ymm6,ymm4
	vfmsubadd213ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfmsubadd213ps ymm2,ymm6,[rcx]
	vfmsubadd231pd ymm2,ymm6,ymm4
	vfmsubadd231pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfmsubadd231pd ymm2,ymm6,[rcx]
	vfmsubadd231ps ymm2,ymm6,ymm4
	vfmsubadd231ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfmsubadd231ps ymm2,ymm6,[rcx]
	vfmsub132pd ymm2,ymm6,ymm4
	vfmsub132pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfmsub132pd ymm2,ymm6,[rcx]
	vfmsub132ps ymm2,ymm6,ymm4
	vfmsub132ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfmsub132ps ymm2,ymm6,[rcx]
	vfmsub213pd ymm2,ymm6,ymm4
	vfmsub213pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfmsub213pd ymm2,ymm6,[rcx]
	vfmsub213ps ymm2,ymm6,ymm4
	vfmsub213ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfmsub213ps ymm2,ymm6,[rcx]
	vfmsub231pd ymm2,ymm6,ymm4
	vfmsub231pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfmsub231pd ymm2,ymm6,[rcx]
	vfmsub231ps ymm2,ymm6,ymm4
	vfmsub231ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfmsub231ps ymm2,ymm6,[rcx]
	vfnmadd132pd ymm2,ymm6,ymm4
	vfnmadd132pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfnmadd132pd ymm2,ymm6,[rcx]
	vfnmadd132ps ymm2,ymm6,ymm4
	vfnmadd132ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfnmadd132ps ymm2,ymm6,[rcx]
	vfnmadd213pd ymm2,ymm6,ymm4
	vfnmadd213pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfnmadd213pd ymm2,ymm6,[rcx]
	vfnmadd213ps ymm2,ymm6,ymm4
	vfnmadd213ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfnmadd213ps ymm2,ymm6,[rcx]
	vfnmadd231pd ymm2,ymm6,ymm4
	vfnmadd231pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfnmadd231pd ymm2,ymm6,[rcx]
	vfnmadd231ps ymm2,ymm6,ymm4
	vfnmadd231ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfnmadd231ps ymm2,ymm6,[rcx]
	vfnmsub132pd ymm2,ymm6,ymm4
	vfnmsub132pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfnmsub132pd ymm2,ymm6,[rcx]
	vfnmsub132ps ymm2,ymm6,ymm4
	vfnmsub132ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfnmsub132ps ymm2,ymm6,[rcx]
	vfnmsub213pd ymm2,ymm6,ymm4
	vfnmsub213pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfnmsub213pd ymm2,ymm6,[rcx]
	vfnmsub213ps ymm2,ymm6,ymm4
	vfnmsub213ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfnmsub213ps ymm2,ymm6,[rcx]
	vfnmsub231pd ymm2,ymm6,ymm4
	vfnmsub231pd ymm2,ymm6,YMMWORD PTR [rcx]
	vfnmsub231pd ymm2,ymm6,[rcx]
	vfnmsub231ps ymm2,ymm6,ymm4
	vfnmsub231ps ymm2,ymm6,YMMWORD PTR [rcx]
	vfnmsub231ps ymm2,ymm6,[rcx]

# Tests for op xmm/mem128, xmm, xmm
	vfmadd132pd xmm2,xmm6,xmm4
	vfmadd132pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfmadd132pd xmm7,xmm6,[rcx]
	vfmadd132ps xmm2,xmm6,xmm4
	vfmadd132ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfmadd132ps xmm7,xmm6,[rcx]
	vfmadd213pd xmm2,xmm6,xmm4
	vfmadd213pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfmadd213pd xmm7,xmm6,[rcx]
	vfmadd213ps xmm2,xmm6,xmm4
	vfmadd213ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfmadd213ps xmm7,xmm6,[rcx]
	vfmadd231pd xmm2,xmm6,xmm4
	vfmadd231pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfmadd231pd xmm7,xmm6,[rcx]
	vfmadd231ps xmm2,xmm6,xmm4
	vfmadd231ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfmadd231ps xmm7,xmm6,[rcx]
	vfmaddsub132pd xmm2,xmm6,xmm4
	vfmaddsub132pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfmaddsub132pd xmm7,xmm6,[rcx]
	vfmaddsub132ps xmm2,xmm6,xmm4
	vfmaddsub132ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfmaddsub132ps xmm7,xmm6,[rcx]
	vfmaddsub213pd xmm2,xmm6,xmm4
	vfmaddsub213pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfmaddsub213pd xmm7,xmm6,[rcx]
	vfmaddsub213ps xmm2,xmm6,xmm4
	vfmaddsub213ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfmaddsub213ps xmm7,xmm6,[rcx]
	vfmaddsub231pd xmm2,xmm6,xmm4
	vfmaddsub231pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfmaddsub231pd xmm7,xmm6,[rcx]
	vfmaddsub231ps xmm2,xmm6,xmm4
	vfmaddsub231ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfmaddsub231ps xmm7,xmm6,[rcx]
	vfmsubadd132pd xmm2,xmm6,xmm4
	vfmsubadd132pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfmsubadd132pd xmm7,xmm6,[rcx]
	vfmsubadd132ps xmm2,xmm6,xmm4
	vfmsubadd132ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfmsubadd132ps xmm7,xmm6,[rcx]
	vfmsubadd213pd xmm2,xmm6,xmm4
	vfmsubadd213pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfmsubadd213pd xmm7,xmm6,[rcx]
	vfmsubadd213ps xmm2,xmm6,xmm4
	vfmsubadd213ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfmsubadd213ps xmm7,xmm6,[rcx]
	vfmsubadd231pd xmm2,xmm6,xmm4
	vfmsubadd231pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfmsubadd231pd xmm7,xmm6,[rcx]
	vfmsubadd231ps xmm2,xmm6,xmm4
	vfmsubadd231ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfmsubadd231ps xmm7,xmm6,[rcx]
	vfmsub132pd xmm2,xmm6,xmm4
	vfmsub132pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfmsub132pd xmm7,xmm6,[rcx]
	vfmsub132ps xmm2,xmm6,xmm4
	vfmsub132ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfmsub132ps xmm7,xmm6,[rcx]
	vfmsub213pd xmm2,xmm6,xmm4
	vfmsub213pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfmsub213pd xmm7,xmm6,[rcx]
	vfmsub213ps xmm2,xmm6,xmm4
	vfmsub213ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfmsub213ps xmm7,xmm6,[rcx]
	vfmsub231pd xmm2,xmm6,xmm4
	vfmsub231pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfmsub231pd xmm7,xmm6,[rcx]
	vfmsub231ps xmm2,xmm6,xmm4
	vfmsub231ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfmsub231ps xmm7,xmm6,[rcx]
	vfnmadd132pd xmm2,xmm6,xmm4
	vfnmadd132pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfnmadd132pd xmm7,xmm6,[rcx]
	vfnmadd132ps xmm2,xmm6,xmm4
	vfnmadd132ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfnmadd132ps xmm7,xmm6,[rcx]
	vfnmadd213pd xmm2,xmm6,xmm4
	vfnmadd213pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfnmadd213pd xmm7,xmm6,[rcx]
	vfnmadd213ps xmm2,xmm6,xmm4
	vfnmadd213ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfnmadd213ps xmm7,xmm6,[rcx]
	vfnmadd231pd xmm2,xmm6,xmm4
	vfnmadd231pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfnmadd231pd xmm7,xmm6,[rcx]
	vfnmadd231ps xmm2,xmm6,xmm4
	vfnmadd231ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfnmadd231ps xmm7,xmm6,[rcx]
	vfnmsub132pd xmm2,xmm6,xmm4
	vfnmsub132pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfnmsub132pd xmm7,xmm6,[rcx]
	vfnmsub132ps xmm2,xmm6,xmm4
	vfnmsub132ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfnmsub132ps xmm7,xmm6,[rcx]
	vfnmsub213pd xmm2,xmm6,xmm4
	vfnmsub213pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfnmsub213pd xmm7,xmm6,[rcx]
	vfnmsub213ps xmm2,xmm6,xmm4
	vfnmsub213ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfnmsub213ps xmm7,xmm6,[rcx]
	vfnmsub231pd xmm2,xmm6,xmm4
	vfnmsub231pd xmm7,xmm6,XMMWORD PTR [rcx]
	vfnmsub231pd xmm7,xmm6,[rcx]
	vfnmsub231ps xmm2,xmm6,xmm4
	vfnmsub231ps xmm7,xmm6,XMMWORD PTR [rcx]
	vfnmsub231ps xmm7,xmm6,[rcx]

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
