.*: Assembler messages:
.*:26: Error: .*unsupported masking.*
.*:27: Error: .*unsupported masking.*
.*:29: Error: .*unsupported instruction.*
.*:30: Error: .*unsupported instruction.*
.*:32: Error: .*unsupported instruction.*
.*:33: Error: .*unsupported instruction.*
.*:36: Error: .*unsupported masking.*
.*:37: Error: .*unsupported masking.*
.*:39: Error: .*unsupported instruction.*
.*:40: Error: .*unsupported instruction.*
.*:43: Error: .*unsupported instruction.*
.*:44: Error: .*unsupported instruction.*
GAS LISTING .*
#...
[ 	]*1[ 	]+\# Test \.arch \.noavx512vl
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+\?\?\?\? 62F27D4F 		vpabsb %zmm5, %zmm6\{%k7\}		\# AVX512BW
[ 	]*3[ 	]+1CF5
[ 	]*4[ 	]+\?\?\?\? 62F27D0F 		vpabsb %xmm5, %xmm6\{%k7\}		\# AVX512BW \+ AVX512VL
[ 	]*4[ 	]+1CF5
[ 	]*5[ 	]+\?\?\?\? 62F27D2F 		vpabsb %ymm5, %ymm6\{%k7\}		\# AVX512BW \+ AVX512VL
[ 	]*5[ 	]+1CF5
[ 	]*6[ 	]+\?\?\?\? 62F27D48 		vpconflictd %zmm5, %zmm6		\# AVX412CD
[ 	]*6[ 	]+C4F5
[ 	]*7[ 	]+\?\?\?\? 62F27D08 		vpconflictd %xmm5, %xmm6		\# AVX412CD \+ AVX512VL
[ 	]*7[ 	]+C4F5
[ 	]*8[ 	]+\?\?\?\? 62F27D28 		vpconflictd %ymm5, %ymm6		\# AVX412CD \+ AVX512VL
[ 	]*8[ 	]+C4F5
[ 	]*9[ 	]+\?\?\?\? 62F1FD4F 		vcvtpd2qq \(%ecx\), %zmm6\{%k7\}		\# AVX512DQ
[ 	]*9[ 	]+7B31
[ 	]*10[ 	]+\?\?\?\? 62F1FD0F 		vcvtpd2qq \(%ecx\), %xmm6\{%k7\}		\# AVX512DQ \+ AVX512VL
[ 	]*10[ 	]+7B31
[ 	]*11[ 	]+\?\?\?\? 62F1FD2F 		vcvtpd2qq \(%ecx\), %ymm6\{%k7\}		\# AVX512DQ \+ AVX512VL
[ 	]*11[ 	]+7B31
[ 	]*12[ 	]+\?\?\?\? 62F27D4F 		vexp2ps %zmm5, %zmm6\{%k7\}		\# AVX512ER
[ 	]*12[ 	]+C8F5
[ 	]*13[ 	]+\?\?\?\? 62F1D54F 		vaddpd %zmm4, %zmm5, %zmm6\{%k7\}		\# AVX512F
[ 	]*13[ 	]+58F4
[ 	]*14[ 	]+\?\?\?\? 62F1D50F 		vaddpd %xmm4, %xmm5, %xmm6\{%k7\}		\# AVX512F \+ AVX512VL
[ 	]*14[ 	]+58F4
[ 	]*15[ 	]+\?\?\?\? 62F1D52F 		vaddpd %ymm4, %ymm5, %ymm6\{%k7\}		\# AVX512F \+ AVX512VL
[ 	]*15[ 	]+58F4
[ 	]*16[ 	]+\?\?\?\? 62F2D54F 		vpmadd52luq %zmm4, %zmm5, %zmm6\{%k7\}	\# AVX512IFMA
[ 	]*16[ 	]+B4F4
[ 	]*17[ 	]+\?\?\?\? 62F2D50F 		vpmadd52luq %xmm4, %xmm5, %xmm6\{%k7\}	\# AVX512IFMA \+ AVX512VL
[ 	]*17[ 	]+B4F4
[ 	]*18[ 	]+\?\?\?\? 62F2D52F 		vpmadd52luq %ymm4, %ymm5, %ymm6\{%k7\}	\# AVX512IFMA \+ AVX512VL
[ 	]*18[ 	]+B4F4
[ 	]*19[ 	]+\?\?\?\? 62F2FD49 		vgatherpf0dpd 23\(%ebp,%ymm7,8\)\{%k1\}	\# AVX512PF
[ 	]*19[ 	]+C68CFD17 
[ 	]*19[ 	]+000000
[ 	]*20[ 	]+\?\?\?\? 62F2554F 		vpermb %zmm4, %zmm5, %zmm6\{%k7\}		\# AVX512VBMI
[ 	]*20[ 	]+8DF4
[ 	]*21[ 	]+\?\?\?\? 62F2550F 		vpermb %xmm4, %xmm5, %xmm6\{%k7\}		\# AVX512VBMI \+ AVX512VL
[ 	]*21[ 	]+8DF4
[ 	]*22[ 	]+\?\?\?\? 62F2552F 		vpermb %ymm4, %ymm5, %ymm6\{%k7\}		\# AVX512VBMI \+ AVX512VL
[ 	]*22[ 	]+8DF4
[ 	]*23[ 	]+
[ 	]*24[ 	]+\.arch \.noavx512vl
[ 	]*25[ 	]+\?\?\?\? 62F27D4F 		vpabsb %zmm5, %zmm6\{%k7\}		\# AVX512BW
[ 	]*25[ 	]+1CF5
[ 	]*26[ 	]+vpabsb %xmm5, %xmm6\{%k7\}		\# AVX512BW \+ AVX512VL
[ 	]*27[ 	]+vpabsb %ymm5, %ymm6\{%k7\}		\# AVX512BW \+ AVX512VL
[ 	]*28[ 	]+\?\?\?\? 62F27D48 		vpconflictd %zmm5, %zmm6		\# AVX412CD
[ 	]*28[ 	]+C4F5
[ 	]*29[ 	]+vpconflictd %xmm5, %xmm6		\# AVX412CD \+ AVX512VL
[ 	]*30[ 	]+vpconflictd %ymm5, %ymm6		\# AVX412CD \+ AVX512VL
[ 	]*31[ 	]+\?\?\?\? 62F1FD4F 		vcvtpd2qq \(%ecx\), %zmm6\{%k7\}		\# AVX512DQ
[ 	]*31[ 	]+7B31
[ 	]*32[ 	]+vcvtpd2qq \(%ecx\), %xmm6\{%k7\}		\# AVX512DQ \+ AVX512VL
[ 	]*33[ 	]+vcvtpd2qq \(%ecx\), %ymm6\{%k7\}		\# AVX512DQ \+ AVX512VL
GAS LISTING .*


[ 	]*34[ 	]+\?\?\?\? 62F27D4F 		vexp2ps %zmm5, %zmm6\{%k7\}		\# AVX512ER
[ 	]*34[ 	]+C8F5
[ 	]*35[ 	]+\?\?\?\? 62F1D54F 		vaddpd %zmm4, %zmm5, %zmm6\{%k7\}		\# AVX512F
[ 	]*35[ 	]+58F4
[ 	]*36[ 	]+vaddpd %xmm4, %xmm5, %xmm6\{%k7\}		\# AVX512F \+ AVX512VL
[ 	]*37[ 	]+vaddpd %ymm4, %ymm5, %ymm6\{%k7\}		\# AVX512F \+ AVX512VL
[ 	]*38[ 	]+\?\?\?\? 62F2D54F 		vpmadd52luq %zmm4, %zmm5, %zmm6\{%k7\}	\# AVX512IFMA
[ 	]*38[ 	]+B4F4
[ 	]*39[ 	]+vpmadd52luq %xmm4, %xmm5, %xmm6\{%k7\}	\# AVX512IFMA \+ AVX512VL
[ 	]*40[ 	]+vpmadd52luq %ymm4, %ymm5, %ymm6\{%k7\}	\# AVX512IFMA \+ AVX512VL
[ 	]*41[ 	]+\?\?\?\? 62F2FD49 		vgatherpf0dpd 23\(%ebp,%ymm7,8\)\{%k1\}	\# AVX512PF
[ 	]*41[ 	]+C68CFD17 
[ 	]*41[ 	]+000000
[ 	]*42[ 	]+\?\?\?\? 62F2554F 		vpermb %zmm4, %zmm5, %zmm6\{%k7\}		\# AVX512VBMI
[ 	]*42[ 	]+8DF4
[ 	]*43[ 	]+vpermb %xmm4, %xmm5, %xmm6\{%k7\}		\# AVX512VBMI \+ AVX512VL
[ 	]*44[ 	]+vpermb %ymm4, %ymm5, %ymm6\{%k7\}		\# AVX512VBMI \+ AVX512VL
[ 	]*45[ 	]+
[ 	]*46[ 	]+\?\?\?\? C4E2791C 		vpabsb %xmm5, %xmm6
[ 	]*46[ 	]+F5
[ 	]*47[ 	]+\?\?\?\? C4E27D1C 		vpabsb %ymm5, %ymm6
[ 	]*47[ 	]+F5
[ 	]*48[ 	]+\?\?\?\? C5D158F4 		vaddpd %xmm4, %xmm5, %xmm6
[ 	]*49[ 	]+\?\?\?\? C5D558F4 		vaddpd %ymm4, %ymm5, %ymm6
[ 	]*50[ 	]+\?\?\?\? 660F381C 		pabsb %xmm5, %xmm6
[ 	]*50[ 	]+F5
[ 	]*51[ 	]+\?\?\?\? 660F58F4 		addpd %xmm4, %xmm6
[ 	]*52[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\.intel_syntax noprefix
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 62F3FD48 		vfpclasspd k0, \[eax], 0
[ 	]*[1-9][0-9]*[ 	]+660000
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 62F37D48 		vfpclassps k0, \[eax\+0x80], 0
[ 	]*[1-9][0-9]*[ 	]+66400200 ?
[ 	]*[1-9][0-9]*[ 	]+
#pass
