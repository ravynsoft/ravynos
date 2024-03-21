.*: Assembler messages:
.*:5: Error: .*
.*:6: Error: .*
.*:7: Error: .*
.*:8: Error: .*
.*:9: Error: .*
.*:10: Error: .*
.*:12: Error: .*
.*:13: Error: .*
.*:15: Error: .*
.*:16: Error: .*
.*:18: Error: .*
.*:19: Error: .*
.*:20: Error: .*
.*:21: Error: .*
.*:24: Error: .*
.*:25: Error: .*
.*:26: Error: .*
.*:27: Error: .*
.*:28: Error: .*
.*:29: Error: .*
.*:31: Error: .*
.*:32: Error: .*
.*:34: Error: .*
.*:35: Error: .*
.*:37: Error: .*
.*:38: Error: .*
.*:39: Error: .*
.*:40: Error: .*
.*:42: Error: .*
.*:43: Error: .*
.*:44: Error: .*
.*:45: Error: .*
.*:46: Error: .*
.*:47: Error: .*
.*:48: Error: .*
.*:51: Error: .*
.*:52: Error: .*
.*:55: Error: .*
.*:56: Error: .*
.*:58: Error: .*
.*:61: Error: .*unsupported broadcast for `vdpbf16ps'
.*:62: Error: .*unsupported broadcast for `vcvtne2ps2bf16'
GAS LISTING .*


[ 	]*1[ 	]+# Check illegal AVX512F instructions
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+\.allow_index_reg
[ 	]*4[ 	]+_start:
[ 	]*5[ 	]+mov \{sae\}, %rax\{%k1\}
[ 	]*6[ 	]+mov \{sae\}, %rax
[ 	]*7[ 	]+mov %rbx, %rax\{%k2\}
[ 	]*8[ 	]+vaddps %zmm3, %zmm1, %zmm2\{z\}\{%k1\}\{z\}
[ 	]*9[ 	]+vaddps %zmm3, %zmm1\{%k3\}, %zmm2\{z\}
[ 	]*10[ 	]+vaddps %zmm3, %zmm1\{%k1\}, %zmm2\{%k2\}
[ 	]*11[ 	]+
[ 	]*12[ 	]+vcvtps2pd \(%rax\), %zmm1\{1to8\}
[ 	]*13[ 	]+vcvtps2pd \(%rax\)\{1to16\}, %zmm1
[ 	]*14[ 	]+
[ 	]*15[ 	]+vcvtps2pd \(%rax\)\{%k1\}, %zmm1
[ 	]*16[ 	]+vcvtps2pd \(%rax\)\{z\}, %zmm1
[ 	]*17[ 	]+
[ 	]*18[ 	]+vgatherqpd \(%rdi\),%zmm6\{%k1\}
[ 	]*19[ 	]+vgatherqpd \(%zmm2\),%zmm6\{%k1\}
[ 	]*20[ 	]+vpscatterdd %zmm6,\(%rdi\)\{%k1\}
[ 	]*21[ 	]+vpscatterdd %zmm6,\(%zmm2\)\{%k1\}
[ 	]*22[ 	]+
[ 	]*23[ 	]+\.intel_syntax noprefix
[ 	]*24[ 	]+mov rax\{k1\}, \{sae\}
[ 	]*25[ 	]+mov rax, \{sae\}
[ 	]*26[ 	]+mov rax\{k2\}, rbx
[ 	]*27[ 	]+vaddps zmm2\{z\}\{k1\}\{z\}, zmm1, zmm3
[ 	]*28[ 	]+vaddps zmm2\{z\}, zmm1\{k3\}, zmm3
[ 	]*29[ 	]+vaddps zmm2\{k2\}, zmm1\{k1\}, zmm3
[ 	]*30[ 	]+
[ 	]*31[ 	]+vcvtps2pd zmm1\{1to8\}, \[rax\]
[ 	]*32[ 	]+vcvtps2pd zmm1, \[rax\]\{1to16\}
[ 	]*33[ 	]+
[ 	]*34[ 	]+vcvtps2pd zmm1, \[rax\]\{k1\}
[ 	]*35[ 	]+vcvtps2pd zmm1, \[rax\]\{z\}
[ 	]*36[ 	]+
[ 	]*37[ 	]+vgatherqpd zmm6\{k1\}, ZMMWORD PTR \[rdi\]
[ 	]*38[ 	]+vgatherqpd zmm6\{k1\}, ZMMWORD PTR \[zmm2\+riz\]
[ 	]*39[ 	]+vpscatterdd ZMMWORD PTR \[rdi\]\{k1\}, zmm6
[ 	]*40[ 	]+vpscatterdd ZMMWORD PTR \[zmm2\+riz\]\{k1\}, zmm6
[ 	]*41[ 	]+
[ 	]*42[ 	]+vaddps zmm2, zmm1, QWORD PTR \[rax\]\{1to8\}
[ 	]*43[ 	]+vaddps zmm2, zmm1, QWORD PTR \[rax\]\{1to16\}
[ 	]*44[ 	]+vaddpd zmm2, zmm1, DWORD PTR \[rax\]\{1to8\}
[ 	]*45[ 	]+vaddpd zmm2, zmm1, DWORD PTR \[rax\]\{1to16\}
[ 	]*46[ 	]+vaddps zmm2, zmm1, ZMMWORD PTR \[rax\]\{1to16\}
[ 	]*47[ 	]+vaddps zmm2, zmm1, DWORD PTR \[rax\]
[ 	]*48[ 	]+vaddpd zmm2, zmm1, QWORD PTR \[rax\]
[ 	]*49[ 	]*
[ 	]*50[ 	]+\.att_syntax prefix
[ 	]*51[ 	]+vaddps %zmm0, %zmm1, %zmm2\{%rcx\}
[ 	]*52[ 	]+vaddps %zmm0, %zmm1, %zmm2\{z\}
[ 	]*53[ 	]*
[ 	]*54[ 	]+\.intel_syntax noprefix
[ 	]*55[ 	]+vaddps zmm2\{rcx\}, zmm1, zmm0
[ 	]*56[ 	]+vaddps zmm2\{z\}, zmm1, zmm0
[ 	]*57[ 	]*
GAS LISTING .*
[ 	]*58[ 	]+vcvtps2qq xmm0, DWORD PTR \[rax\]
[ 	]*59[ 	]*
[ 	]*60[ 	]+\.att_syntax prefix
[ 	]*61[ 	]+vdpbf16ps 8\(%rax\)\{1to8\}, %zmm2, %zmm2
[ 	]*62[ 	]+vcvtne2ps2bf16 8\(%rax\)\{1to8\}, %zmm2, %zmm2
