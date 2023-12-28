.*: Assembler messages:
.*:16: Error: .*
.*:18: Error: .*
.*:20: Error: .*
.*:22: Error: .*
.*:24: Error: .*
.*:26: Error: .*
.*:28: Error: .*
.*:30: Error: .*
.*:32: Error: .*
.*:34: Error: .*
.*:36: Error: .*
.*:38: Error: .*
.*:40: Error: .*
.*:42: Error: .*
.*:44: Error: .*
.*:48: Error: .*
.*:50: Error: .*
.*:52: Error: .*
.*:54: Error: .*
.*:56: Error: .*
.*:58: Error: .*
.*:60: Error: .*
.*:62: Error: .*
.*:64: Error: .*
.*:66: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\.include "x86-64-arch-2\.s"
[ 	]*1[ 	]+\# Test -march=
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+\# cmov feature 
[ 	]*4[ 	]+\?\?\?\? 0F44D8   	cmove	%eax,%ebx
[ 	]*5[ 	]+\# clflush
[ 	]*6[ 	]+\?\?\?\? 0FAE38   	clflush \(%rax\)
[ 	]*7[ 	]+\# SYSCALL
[ 	]*8[ 	]+\?\?\?\? 0F05     	syscall
[ 	]*9[ 	]+\# MMX
[ 	]*10[ 	]+\?\?\?\? 0FFCDC   	paddb %mm4,%mm3
[ 	]*11[ 	]+\# SSE
[ 	]*12[ 	]+\?\?\?\? F30F58DC 	addss %xmm4,%xmm3
[ 	]*13[ 	]+\# SSE2
[ 	]*14[ 	]+\?\?\?\? F20F58DC 	addsd %xmm4,%xmm3
[ 	]*15[ 	]+\# SSE3
[ 	]*16[ 	]+addsubpd %xmm4,%xmm3
[ 	]*17[ 	]+\# SSSE3
[ 	]*18[ 	]+phaddw %xmm4,%xmm3
[ 	]*19[ 	]+\# SSE4\.1
[ 	]*20[ 	]+phminposuw  %xmm1,%xmm3
[ 	]*21[ 	]+\# SSE4\.2
[ 	]*22[ 	]+crc32   %ecx,%ebx
[ 	]*23[ 	]+\# AVX
[ 	]*24[ 	]+vzeroall
[ 	]*25[ 	]+\# VMX
[ 	]*26[ 	]+vmxoff
[ 	]*27[ 	]+\# SMX
[ 	]*28[ 	]+getsec
[ 	]*29[ 	]+\# Xsave
[ 	]*30[ 	]+xgetbv
[ 	]*31[ 	]+\# Xsaveopt
[ 	]*32[ 	]+xsaveopt \(%rcx\)
[ 	]*33[ 	]+\# AES
[ 	]*34[ 	]+aesenc  \(%rcx\),%xmm0
[ 	]*35[ 	]+\# PCLMUL
[ 	]*36[ 	]+pclmulqdq \$8,%xmm1,%xmm0
[ 	]*37[ 	]+\# AES \+ AVX
[ 	]*38[ 	]+vaesenc  \(%rcx\),%xmm0,%xmm2
[ 	]*39[ 	]+\# PCLMUL \+ AVX
[ 	]*40[ 	]+vpclmulqdq \$8,%xmm4,%xmm6,%xmm2
[ 	]*41[ 	]+\# FMA
[ 	]*42[ 	]+vfmadd132pd %xmm4,%xmm6,%xmm2
[ 	]*43[ 	]+\# MOVBE
[ 	]*44[ 	]+movbe   \(%rcx\),%ebx
[ 	]*45[ 	]+\# CX16
[ 	]*46[ 	]+\?\?\?\? 480FC70E 	cmpxchg16b \(%rsi\)
[ 	]*47[ 	]+\# EPT
[ 	]*48[ 	]+invept  \(%rcx\),%rbx
[ 	]*49[ 	]+\# RDTSCP
[ 	]*50[ 	]+rdtscp
[ 	]*51[ 	]+\# 3DNow or PRFCHW
[ 	]*52[ 	]+prefetchw   0x1000\(,%rsi,2\)
[ 	]*53[ 	]+\# SSE4a
[ 	]*54[ 	]+insertq %xmm2,%xmm1
[ 	]*55[ 	]+\# SVME
[ 	]*56[ 	]+vmload
GAS LISTING .*


[ 	]*57[ 	]+\# ABM/LZCNT
[ 	]*58[ 	]+lzcnt %ecx,%ebx
[ 	]*59[ 	]+\# PadLock
[ 	]*60[ 	]+xstorerng
[ 	]*61[ 	]+\# BMI
[ 	]*62[ 	]+blsr %ecx,%ebx
[ 	]*63[ 	]+\# TBM
[ 	]*64[ 	]+blcfill %ecx,%ebx
[ 	]*65[ 	]+\# LAHF/SAHF
[ 	]*66[ 	]+lahf
