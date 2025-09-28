# Test -march=
	.text
# cmov feature 
cmove	%eax,%ebx
# clflush
clflush (%eax)
# SYSCALL
syscall
# MMX
paddb %mm4,%mm3
# SSE
addss %xmm4,%xmm3
# SSE2
addsd %xmm4,%xmm3
# SSE3
addsubpd %xmm4,%xmm3
# SSSE3
phaddw %xmm4,%xmm3
# SSE4.1
phminposuw  %xmm1,%xmm3
# SSE4.2
crc32   %ecx,%ebx
# AVX
vzeroall
# VMX
vmxoff
# SMX
getsec
# Xsave
xgetbv
# Xsaveopt
xsaveopt (%ecx)
# AES
aesenc  (%ecx),%xmm0
# PCLMUL
pclmulqdq $8,%xmm1,%xmm0
# AES + AVX
vaesenc  (%ecx),%xmm0,%xmm2
# PCLMUL + AVX
vpclmulqdq $8,%xmm4,%xmm6,%xmm2
# FMA
vfmadd132pd %xmm4,%xmm6,%xmm2
# MOVBE
movbe   (%ecx),%ebx
# EPT
invept  (%ecx),%ebx
# RDTSCP
rdtscp
# 3DNow or PRFCHW
prefetchw 0x1000(,%esi,2)
# SSE4a
insertq %xmm2,%xmm1
# SVME
vmload
# ABM/LZCNT
lzcnt %ecx,%ebx
# PadLock
xstorerng
# nop
nopl (%eax)
# BMI
blsr %ecx,%ebx
# TBM
blcfill %ecx,%ebx
# MONITOR
monitor
