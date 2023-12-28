# Check FMA instructions

	.allow_index_reg
	.text
_start:

# Tests for op ymm/mem256, ymm, ymm
	vfmadd132pd %ymm4,%ymm6,%ymm2
	vfmadd132pd (%ecx),%ymm6,%ymm2
	vfmadd132ps %ymm4,%ymm6,%ymm2
	vfmadd132ps (%ecx),%ymm6,%ymm2
	vfmadd213pd %ymm4,%ymm6,%ymm2
	vfmadd213pd (%ecx),%ymm6,%ymm2
	vfmadd213ps %ymm4,%ymm6,%ymm2
	vfmadd213ps (%ecx),%ymm6,%ymm2
	vfmadd231pd %ymm4,%ymm6,%ymm2
	vfmadd231pd (%ecx),%ymm6,%ymm2
	vfmadd231ps %ymm4,%ymm6,%ymm2
	vfmadd231ps (%ecx),%ymm6,%ymm2
	vfmaddsub132pd %ymm4,%ymm6,%ymm2
	vfmaddsub132pd (%ecx),%ymm6,%ymm2
	vfmaddsub132ps %ymm4,%ymm6,%ymm2
	vfmaddsub132ps (%ecx),%ymm6,%ymm2
	vfmaddsub213pd %ymm4,%ymm6,%ymm2
	vfmaddsub213pd (%ecx),%ymm6,%ymm2
	vfmaddsub213ps %ymm4,%ymm6,%ymm2
	vfmaddsub213ps (%ecx),%ymm6,%ymm2
	vfmaddsub231pd %ymm4,%ymm6,%ymm2
	vfmaddsub231pd (%ecx),%ymm6,%ymm2
	vfmaddsub231ps %ymm4,%ymm6,%ymm2
	vfmaddsub231ps (%ecx),%ymm6,%ymm2
	vfmsubadd132pd %ymm4,%ymm6,%ymm2
	vfmsubadd132pd (%ecx),%ymm6,%ymm2
	vfmsubadd132ps %ymm4,%ymm6,%ymm2
	vfmsubadd132ps (%ecx),%ymm6,%ymm2
	vfmsubadd213pd %ymm4,%ymm6,%ymm2
	vfmsubadd213pd (%ecx),%ymm6,%ymm2
	vfmsubadd213ps %ymm4,%ymm6,%ymm2
	vfmsubadd213ps (%ecx),%ymm6,%ymm2
	vfmsubadd231pd %ymm4,%ymm6,%ymm2
	vfmsubadd231pd (%ecx),%ymm6,%ymm2
	vfmsubadd231ps %ymm4,%ymm6,%ymm2
	vfmsubadd231ps (%ecx),%ymm6,%ymm2
	vfmsub132pd %ymm4,%ymm6,%ymm2
	vfmsub132pd (%ecx),%ymm6,%ymm2
	vfmsub132ps %ymm4,%ymm6,%ymm2
	vfmsub132ps (%ecx),%ymm6,%ymm2
	vfmsub213pd %ymm4,%ymm6,%ymm2
	vfmsub213pd (%ecx),%ymm6,%ymm2
	vfmsub213ps %ymm4,%ymm6,%ymm2
	vfmsub213ps (%ecx),%ymm6,%ymm2
	vfmsub231pd %ymm4,%ymm6,%ymm2
	vfmsub231pd (%ecx),%ymm6,%ymm2
	vfmsub231ps %ymm4,%ymm6,%ymm2
	vfmsub231ps (%ecx),%ymm6,%ymm2
	vfnmadd132pd %ymm4,%ymm6,%ymm2
	vfnmadd132pd (%ecx),%ymm6,%ymm2
	vfnmadd132ps %ymm4,%ymm6,%ymm2
	vfnmadd132ps (%ecx),%ymm6,%ymm2
	vfnmadd213pd %ymm4,%ymm6,%ymm2
	vfnmadd213pd (%ecx),%ymm6,%ymm2
	vfnmadd213ps %ymm4,%ymm6,%ymm2
	vfnmadd213ps (%ecx),%ymm6,%ymm2
	vfnmadd231pd %ymm4,%ymm6,%ymm2
	vfnmadd231pd (%ecx),%ymm6,%ymm2
	vfnmadd231ps %ymm4,%ymm6,%ymm2
	vfnmadd231ps (%ecx),%ymm6,%ymm2
	vfnmsub132pd %ymm4,%ymm6,%ymm2
	vfnmsub132pd (%ecx),%ymm6,%ymm2
	vfnmsub132ps %ymm4,%ymm6,%ymm2
	vfnmsub132ps (%ecx),%ymm6,%ymm2
	vfnmsub213pd %ymm4,%ymm6,%ymm2
	vfnmsub213pd (%ecx),%ymm6,%ymm2
	vfnmsub213ps %ymm4,%ymm6,%ymm2
	vfnmsub213ps (%ecx),%ymm6,%ymm2
	vfnmsub231pd %ymm4,%ymm6,%ymm2
	vfnmsub231pd (%ecx),%ymm6,%ymm2
	vfnmsub231ps %ymm4,%ymm6,%ymm2
	vfnmsub231ps (%ecx),%ymm6,%ymm2

# Tests for op xmm/mem128, xmm, xmm
	vfmadd132pd %xmm4,%xmm6,%xmm2
	vfmadd132pd (%ecx),%xmm6,%xmm7
	vfmadd132ps %xmm4,%xmm6,%xmm2
	vfmadd132ps (%ecx),%xmm6,%xmm7
	vfmadd213pd %xmm4,%xmm6,%xmm2
	vfmadd213pd (%ecx),%xmm6,%xmm7
	vfmadd213ps %xmm4,%xmm6,%xmm2
	vfmadd213ps (%ecx),%xmm6,%xmm7
	vfmadd231pd %xmm4,%xmm6,%xmm2
	vfmadd231pd (%ecx),%xmm6,%xmm7
	vfmadd231ps %xmm4,%xmm6,%xmm2
	vfmadd231ps (%ecx),%xmm6,%xmm7
	vfmaddsub132pd %xmm4,%xmm6,%xmm2
	vfmaddsub132pd (%ecx),%xmm6,%xmm7
	vfmaddsub132ps %xmm4,%xmm6,%xmm2
	vfmaddsub132ps (%ecx),%xmm6,%xmm7
	vfmaddsub213pd %xmm4,%xmm6,%xmm2
	vfmaddsub213pd (%ecx),%xmm6,%xmm7
	vfmaddsub213ps %xmm4,%xmm6,%xmm2
	vfmaddsub213ps (%ecx),%xmm6,%xmm7
	vfmaddsub231pd %xmm4,%xmm6,%xmm2
	vfmaddsub231pd (%ecx),%xmm6,%xmm7
	vfmaddsub231ps %xmm4,%xmm6,%xmm2
	vfmaddsub231ps (%ecx),%xmm6,%xmm7
	vfmsubadd132pd %xmm4,%xmm6,%xmm2
	vfmsubadd132pd (%ecx),%xmm6,%xmm7
	vfmsubadd132ps %xmm4,%xmm6,%xmm2
	vfmsubadd132ps (%ecx),%xmm6,%xmm7
	vfmsubadd213pd %xmm4,%xmm6,%xmm2
	vfmsubadd213pd (%ecx),%xmm6,%xmm7
	vfmsubadd213ps %xmm4,%xmm6,%xmm2
	vfmsubadd213ps (%ecx),%xmm6,%xmm7
	vfmsubadd231pd %xmm4,%xmm6,%xmm2
	vfmsubadd231pd (%ecx),%xmm6,%xmm7
	vfmsubadd231ps %xmm4,%xmm6,%xmm2
	vfmsubadd231ps (%ecx),%xmm6,%xmm7
	vfmsub132pd %xmm4,%xmm6,%xmm2
	vfmsub132pd (%ecx),%xmm6,%xmm7
	vfmsub132ps %xmm4,%xmm6,%xmm2
	vfmsub132ps (%ecx),%xmm6,%xmm7
	vfmsub213pd %xmm4,%xmm6,%xmm2
	vfmsub213pd (%ecx),%xmm6,%xmm7
	vfmsub213ps %xmm4,%xmm6,%xmm2
	vfmsub213ps (%ecx),%xmm6,%xmm7
	vfmsub231pd %xmm4,%xmm6,%xmm2
	vfmsub231pd (%ecx),%xmm6,%xmm7
	vfmsub231ps %xmm4,%xmm6,%xmm2
	vfmsub231ps (%ecx),%xmm6,%xmm7
	vfnmadd132pd %xmm4,%xmm6,%xmm2
	vfnmadd132pd (%ecx),%xmm6,%xmm7
	vfnmadd132ps %xmm4,%xmm6,%xmm2
	vfnmadd132ps (%ecx),%xmm6,%xmm7
	vfnmadd213pd %xmm4,%xmm6,%xmm2
	vfnmadd213pd (%ecx),%xmm6,%xmm7
	vfnmadd213ps %xmm4,%xmm6,%xmm2
	vfnmadd213ps (%ecx),%xmm6,%xmm7
	vfnmadd231pd %xmm4,%xmm6,%xmm2
	vfnmadd231pd (%ecx),%xmm6,%xmm7
	vfnmadd231ps %xmm4,%xmm6,%xmm2
	vfnmadd231ps (%ecx),%xmm6,%xmm7
	vfnmsub132pd %xmm4,%xmm6,%xmm2
	vfnmsub132pd (%ecx),%xmm6,%xmm7
	vfnmsub132ps %xmm4,%xmm6,%xmm2
	vfnmsub132ps (%ecx),%xmm6,%xmm7
	vfnmsub213pd %xmm4,%xmm6,%xmm2
	vfnmsub213pd (%ecx),%xmm6,%xmm7
	vfnmsub213ps %xmm4,%xmm6,%xmm2
	vfnmsub213ps (%ecx),%xmm6,%xmm7
	vfnmsub231pd %xmm4,%xmm6,%xmm2
	vfnmsub231pd (%ecx),%xmm6,%xmm7
	vfnmsub231ps %xmm4,%xmm6,%xmm2
	vfnmsub231ps (%ecx),%xmm6,%xmm7

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

# Tests for op ymm/mem256, ymm, ymm
	vfmadd132pd ymm2,ymm6,ymm4
	vfmadd132pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfmadd132pd ymm2,ymm6,[ecx]
	vfmadd132ps ymm2,ymm6,ymm4
	vfmadd132ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfmadd132ps ymm2,ymm6,[ecx]
	vfmadd213pd ymm2,ymm6,ymm4
	vfmadd213pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfmadd213pd ymm2,ymm6,[ecx]
	vfmadd213ps ymm2,ymm6,ymm4
	vfmadd213ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfmadd213ps ymm2,ymm6,[ecx]
	vfmadd231pd ymm2,ymm6,ymm4
	vfmadd231pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfmadd231pd ymm2,ymm6,[ecx]
	vfmadd231ps ymm2,ymm6,ymm4
	vfmadd231ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfmadd231ps ymm2,ymm6,[ecx]
	vfmaddsub132pd ymm2,ymm6,ymm4
	vfmaddsub132pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfmaddsub132pd ymm2,ymm6,[ecx]
	vfmaddsub132ps ymm2,ymm6,ymm4
	vfmaddsub132ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfmaddsub132ps ymm2,ymm6,[ecx]
	vfmaddsub213pd ymm2,ymm6,ymm4
	vfmaddsub213pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfmaddsub213pd ymm2,ymm6,[ecx]
	vfmaddsub213ps ymm2,ymm6,ymm4
	vfmaddsub213ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfmaddsub213ps ymm2,ymm6,[ecx]
	vfmaddsub231pd ymm2,ymm6,ymm4
	vfmaddsub231pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfmaddsub231pd ymm2,ymm6,[ecx]
	vfmaddsub231ps ymm2,ymm6,ymm4
	vfmaddsub231ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfmaddsub231ps ymm2,ymm6,[ecx]
	vfmsubadd132pd ymm2,ymm6,ymm4
	vfmsubadd132pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfmsubadd132pd ymm2,ymm6,[ecx]
	vfmsubadd132ps ymm2,ymm6,ymm4
	vfmsubadd132ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfmsubadd132ps ymm2,ymm6,[ecx]
	vfmsubadd213pd ymm2,ymm6,ymm4
	vfmsubadd213pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfmsubadd213pd ymm2,ymm6,[ecx]
	vfmsubadd213ps ymm2,ymm6,ymm4
	vfmsubadd213ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfmsubadd213ps ymm2,ymm6,[ecx]
	vfmsubadd231pd ymm2,ymm6,ymm4
	vfmsubadd231pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfmsubadd231pd ymm2,ymm6,[ecx]
	vfmsubadd231ps ymm2,ymm6,ymm4
	vfmsubadd231ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfmsubadd231ps ymm2,ymm6,[ecx]
	vfmsub132pd ymm2,ymm6,ymm4
	vfmsub132pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfmsub132pd ymm2,ymm6,[ecx]
	vfmsub132ps ymm2,ymm6,ymm4
	vfmsub132ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfmsub132ps ymm2,ymm6,[ecx]
	vfmsub213pd ymm2,ymm6,ymm4
	vfmsub213pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfmsub213pd ymm2,ymm6,[ecx]
	vfmsub213ps ymm2,ymm6,ymm4
	vfmsub213ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfmsub213ps ymm2,ymm6,[ecx]
	vfmsub231pd ymm2,ymm6,ymm4
	vfmsub231pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfmsub231pd ymm2,ymm6,[ecx]
	vfmsub231ps ymm2,ymm6,ymm4
	vfmsub231ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfmsub231ps ymm2,ymm6,[ecx]
	vfnmadd132pd ymm2,ymm6,ymm4
	vfnmadd132pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfnmadd132pd ymm2,ymm6,[ecx]
	vfnmadd132ps ymm2,ymm6,ymm4
	vfnmadd132ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfnmadd132ps ymm2,ymm6,[ecx]
	vfnmadd213pd ymm2,ymm6,ymm4
	vfnmadd213pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfnmadd213pd ymm2,ymm6,[ecx]
	vfnmadd213ps ymm2,ymm6,ymm4
	vfnmadd213ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfnmadd213ps ymm2,ymm6,[ecx]
	vfnmadd231pd ymm2,ymm6,ymm4
	vfnmadd231pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfnmadd231pd ymm2,ymm6,[ecx]
	vfnmadd231ps ymm2,ymm6,ymm4
	vfnmadd231ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfnmadd231ps ymm2,ymm6,[ecx]
	vfnmsub132pd ymm2,ymm6,ymm4
	vfnmsub132pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfnmsub132pd ymm2,ymm6,[ecx]
	vfnmsub132ps ymm2,ymm6,ymm4
	vfnmsub132ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfnmsub132ps ymm2,ymm6,[ecx]
	vfnmsub213pd ymm2,ymm6,ymm4
	vfnmsub213pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfnmsub213pd ymm2,ymm6,[ecx]
	vfnmsub213ps ymm2,ymm6,ymm4
	vfnmsub213ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfnmsub213ps ymm2,ymm6,[ecx]
	vfnmsub231pd ymm2,ymm6,ymm4
	vfnmsub231pd ymm2,ymm6,YMMWORD PTR [ecx]
	vfnmsub231pd ymm2,ymm6,[ecx]
	vfnmsub231ps ymm2,ymm6,ymm4
	vfnmsub231ps ymm2,ymm6,YMMWORD PTR [ecx]
	vfnmsub231ps ymm2,ymm6,[ecx]

# Tests for op xmm/mem128, xmm, xmm
	vfmadd132pd xmm2,xmm6,xmm4
	vfmadd132pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfmadd132pd xmm7,xmm6,[ecx]
	vfmadd132ps xmm2,xmm6,xmm4
	vfmadd132ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfmadd132ps xmm7,xmm6,[ecx]
	vfmadd213pd xmm2,xmm6,xmm4
	vfmadd213pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfmadd213pd xmm7,xmm6,[ecx]
	vfmadd213ps xmm2,xmm6,xmm4
	vfmadd213ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfmadd213ps xmm7,xmm6,[ecx]
	vfmadd231pd xmm2,xmm6,xmm4
	vfmadd231pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfmadd231pd xmm7,xmm6,[ecx]
	vfmadd231ps xmm2,xmm6,xmm4
	vfmadd231ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfmadd231ps xmm7,xmm6,[ecx]
	vfmaddsub132pd xmm2,xmm6,xmm4
	vfmaddsub132pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfmaddsub132pd xmm7,xmm6,[ecx]
	vfmaddsub132ps xmm2,xmm6,xmm4
	vfmaddsub132ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfmaddsub132ps xmm7,xmm6,[ecx]
	vfmaddsub213pd xmm2,xmm6,xmm4
	vfmaddsub213pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfmaddsub213pd xmm7,xmm6,[ecx]
	vfmaddsub213ps xmm2,xmm6,xmm4
	vfmaddsub213ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfmaddsub213ps xmm7,xmm6,[ecx]
	vfmaddsub231pd xmm2,xmm6,xmm4
	vfmaddsub231pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfmaddsub231pd xmm7,xmm6,[ecx]
	vfmaddsub231ps xmm2,xmm6,xmm4
	vfmaddsub231ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfmaddsub231ps xmm7,xmm6,[ecx]
	vfmsubadd132pd xmm2,xmm6,xmm4
	vfmsubadd132pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfmsubadd132pd xmm7,xmm6,[ecx]
	vfmsubadd132ps xmm2,xmm6,xmm4
	vfmsubadd132ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfmsubadd132ps xmm7,xmm6,[ecx]
	vfmsubadd213pd xmm2,xmm6,xmm4
	vfmsubadd213pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfmsubadd213pd xmm7,xmm6,[ecx]
	vfmsubadd213ps xmm2,xmm6,xmm4
	vfmsubadd213ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfmsubadd213ps xmm7,xmm6,[ecx]
	vfmsubadd231pd xmm2,xmm6,xmm4
	vfmsubadd231pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfmsubadd231pd xmm7,xmm6,[ecx]
	vfmsubadd231ps xmm2,xmm6,xmm4
	vfmsubadd231ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfmsubadd231ps xmm7,xmm6,[ecx]
	vfmsub132pd xmm2,xmm6,xmm4
	vfmsub132pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfmsub132pd xmm7,xmm6,[ecx]
	vfmsub132ps xmm2,xmm6,xmm4
	vfmsub132ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfmsub132ps xmm7,xmm6,[ecx]
	vfmsub213pd xmm2,xmm6,xmm4
	vfmsub213pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfmsub213pd xmm7,xmm6,[ecx]
	vfmsub213ps xmm2,xmm6,xmm4
	vfmsub213ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfmsub213ps xmm7,xmm6,[ecx]
	vfmsub231pd xmm2,xmm6,xmm4
	vfmsub231pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfmsub231pd xmm7,xmm6,[ecx]
	vfmsub231ps xmm2,xmm6,xmm4
	vfmsub231ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfmsub231ps xmm7,xmm6,[ecx]
	vfnmadd132pd xmm2,xmm6,xmm4
	vfnmadd132pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfnmadd132pd xmm7,xmm6,[ecx]
	vfnmadd132ps xmm2,xmm6,xmm4
	vfnmadd132ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfnmadd132ps xmm7,xmm6,[ecx]
	vfnmadd213pd xmm2,xmm6,xmm4
	vfnmadd213pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfnmadd213pd xmm7,xmm6,[ecx]
	vfnmadd213ps xmm2,xmm6,xmm4
	vfnmadd213ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfnmadd213ps xmm7,xmm6,[ecx]
	vfnmadd231pd xmm2,xmm6,xmm4
	vfnmadd231pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfnmadd231pd xmm7,xmm6,[ecx]
	vfnmadd231ps xmm2,xmm6,xmm4
	vfnmadd231ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfnmadd231ps xmm7,xmm6,[ecx]
	vfnmsub132pd xmm2,xmm6,xmm4
	vfnmsub132pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfnmsub132pd xmm7,xmm6,[ecx]
	vfnmsub132ps xmm2,xmm6,xmm4
	vfnmsub132ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfnmsub132ps xmm7,xmm6,[ecx]
	vfnmsub213pd xmm2,xmm6,xmm4
	vfnmsub213pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfnmsub213pd xmm7,xmm6,[ecx]
	vfnmsub213ps xmm2,xmm6,xmm4
	vfnmsub213ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfnmsub213ps xmm7,xmm6,[ecx]
	vfnmsub231pd xmm2,xmm6,xmm4
	vfnmsub231pd xmm7,xmm6,XMMWORD PTR [ecx]
	vfnmsub231pd xmm7,xmm6,[ecx]
	vfnmsub231ps xmm2,xmm6,xmm4
	vfnmsub231ps xmm7,xmm6,XMMWORD PTR [ecx]
	vfnmsub231ps xmm7,xmm6,[ecx]

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
