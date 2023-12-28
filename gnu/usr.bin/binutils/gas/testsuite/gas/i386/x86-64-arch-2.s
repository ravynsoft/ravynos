# Test -march=
	.text
# cmov feature 
cmove	%eax,%ebx
# clflush
clflush (%rax)
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
xsaveopt (%rcx)
# AES
aesenc  (%rcx),%xmm0
# PCLMUL
pclmulqdq $8,%xmm1,%xmm0
# AES + AVX
vaesenc  (%rcx),%xmm0,%xmm2
# PCLMUL + AVX
vpclmulqdq $8,%xmm4,%xmm6,%xmm2
# FMA
vfmadd132pd %xmm4,%xmm6,%xmm2
# MOVBE
movbe   (%rcx),%ebx
# CX16
cmpxchg16b (%rsi)
# EPT
invept  (%rcx),%rbx
# RDTSCP
rdtscp
# 3DNow or PRFCHW
prefetchw   0x1000(,%rsi,2)
# SSE4a
insertq %xmm2,%xmm1
# SVME
vmload
# ABM/LZCNT
lzcnt %ecx,%ebx
# PadLock
xstorerng
# BMI
blsr %ecx,%ebx
# TBM
blcfill %ecx,%ebx
# LAHF/SAHF
lahf
