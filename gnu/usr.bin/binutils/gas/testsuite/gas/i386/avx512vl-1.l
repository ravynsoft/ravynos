.*: Assembler messages:
.*:7: Error: .*bad register name.*
.*:8: Error: .*corei7\.avx.*
.*:9: Error: .*corei7\.avx.*
.*:10: Error: .*corei7\.avx.*
.*:15: Error: .*unsupported.*
.*:16: Error: .*unsupported.*
GAS LISTING .*
#...
[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+\.arch corei7
[ 	]*3[ 	]+_start:
[ 	]*4[ 	]+\.arch \.avx
[ 	]*5[ 	]+\?\?\?\? C5F9E711 		vmovntdq	%xmm2, \(%ecx\)
[ 	]*6[ 	]+\?\?\?\? C5FDE711 		vmovntdq	%ymm2, \(%ecx\)
[ 	]*7[ 	]+vmovntdq	%zmm2, \(%ecx\)
[ 	]*8[ 	]+vpternlogq	\$0xab, %xmm6, %xmm2, %xmm0
[ 	]*9[ 	]+vpternlogq	\$0xab, %ymm6, %ymm2, %ymm0
[ 	]*10[ 	]+vpternlogq	\$0xab, %zmm6, %zmm2, %zmm0
[ 	]*11[ 	]+\.arch \.avx512f
[ 	]*12[ 	]+\?\?\?\? C5F9E701 		vmovntdq	%xmm0, \(%ecx\)
[ 	]*13[ 	]+\?\?\?\? C5FDE701 		vmovntdq	%ymm0, \(%ecx\)
[ 	]*14[ 	]+\?\?\?\? 62F17D48 		vmovntdq	%zmm0, \(%ecx\)
[ 	]*14[ 	]+E701
[ 	]*15[ 	]+vpternlogq	\$0xab, %xmm6, %xmm2, %xmm0
[ 	]*16[ 	]+vpternlogq	\$0xab, %ymm6, %ymm2, %ymm0
[ 	]*17[ 	]+\?\?\?\? 62F3ED48 		vpternlogq	\$0xab, %zmm6, %zmm2, %zmm0
[ 	]*17[ 	]+25C6AB
[ 	]*18[ 	]+\.arch \.avx512vl
[ 	]*19[ 	]+\?\?\?\? C5F9E701 		vmovntdq	%xmm0, \(%ecx\)
[ 	]*20[ 	]+\?\?\?\? C5FDE701 		vmovntdq	%ymm0, \(%ecx\)
[ 	]*21[ 	]+\?\?\?\? 62F17D48 		vmovntdq	%zmm0, \(%ecx\)
[ 	]*21[ 	]+E701
[ 	]*22[ 	]+\?\?\?\? 62F3ED08 		vpternlogq	\$0xab, %xmm6, %xmm2, %xmm0
[ 	]*22[ 	]+25C6AB
[ 	]*23[ 	]+\?\?\?\? 62F3ED28 		vpternlogq	\$0xab, %ymm6, %ymm2, %ymm0
[ 	]*23[ 	]+25C6AB
[ 	]*24[ 	]+\?\?\?\? 62F3ED48 		vpternlogq	\$0xab, %zmm6, %zmm2, %zmm0
[ 	]*24[ 	]+25C6AB
