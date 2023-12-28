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
[ 	]*5[ 	]+\?\?\?\? C5F9E711 		vmovntdq	%xmm2, \(%rcx\)
[ 	]*6[ 	]+\?\?\?\? C5FDE711 		vmovntdq	%ymm2, \(%rcx\)
[ 	]*7[ 	]+vmovntdq	%zmm2, \(%rcx\)
[ 	]*8[ 	]+vpternlogq	\$0xab, %xmm16, %xmm2, %xmm0
[ 	]*9[ 	]+vpternlogq	\$0xab, %ymm16, %ymm2, %ymm0
[ 	]*10[ 	]+vpternlogq	\$0xab, %zmm16, %zmm2, %zmm0
[ 	]*11[ 	]+\.arch \.avx512f
[ 	]*12[ 	]+\?\?\?\? C5F9E701 		vmovntdq	%xmm0, \(%rcx\)
[ 	]*13[ 	]+\?\?\?\? C5FDE701 		vmovntdq	%ymm0, \(%rcx\)
[ 	]*14[ 	]+\?\?\?\? 62F17D48 		vmovntdq	%zmm0, \(%rcx\)
[ 	]*14[ 	]+E701
[ 	]*15[ 	]+vpternlogq	\$0xab, %xmm16, %xmm2, %xmm0
[ 	]*16[ 	]+vpternlogq	\$0xab, %ymm16, %ymm2, %ymm0
[ 	]*17[ 	]+\?\?\?\? 62B3ED48 		vpternlogq	\$0xab, %zmm16, %zmm2, %zmm0
[ 	]*17[ 	]+25C0AB
[ 	]*18[ 	]+\.arch \.avx512vl
[ 	]*19[ 	]+\?\?\?\? C5F9E701 		vmovntdq	%xmm0, \(%rcx\)
[ 	]*20[ 	]+\?\?\?\? C5FDE701 		vmovntdq	%ymm0, \(%rcx\)
[ 	]*21[ 	]+\?\?\?\? 62F17D48 		vmovntdq	%zmm0, \(%rcx\)
[ 	]*21[ 	]+E701
[ 	]*22[ 	]+\?\?\?\? 62B3ED08 		vpternlogq	\$0xab, %xmm16, %xmm2, %xmm0
[ 	]*22[ 	]+25C0AB
[ 	]*23[ 	]+\?\?\?\? 62B3ED28 		vpternlogq	\$0xab, %ymm16, %ymm2, %ymm0
[ 	]*23[ 	]+25C0AB
[ 	]*24[ 	]+\?\?\?\? 62B3ED48 		vpternlogq	\$0xab, %zmm16, %zmm2, %zmm0
[ 	]*24[ 	]+25C0AB
