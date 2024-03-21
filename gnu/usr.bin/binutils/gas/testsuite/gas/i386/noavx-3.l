.*: Assembler messages:
.*:4: Error: .*generic.*
.*:5: Error: .*generic.*
.*:6: Error: .*generic.*
.*:7: Error: .*generic.*
.*:12: Error: .*generic.*
.*:15: Error: .*avx.*
.*:32: Error: .*noavx2.*
.*:35: Error: .*noavx.*
.*:36: Error: .*noavx.*
.*:37: Error: .*noavx.*
.*:38: Error: .*noavx.*
.*:39: Error: .*noavx.*
.*:42: Error: .*noavx.*
.*:43: Error: .*noavx.*
.*:44: Error: .*noavx.*
.*:45: Error: .*noavx.*
GAS LISTING .*
#...
[ 	]*1[ 	]+\# Test \.arch \[\.avxX|\.noavxX\]
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+\.arch generic32
[ 	]*4[ 	]+vcvtph2ps %xmm4,%ymm4
[ 	]*5[ 	]+vfmadd132pd %ymm4,%ymm6,%ymm2
[ 	]*6[ 	]+vfmaddpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*7[ 	]+vfrczpd %xmm7,%xmm7
[ 	]*8[ 	]+\.arch \.mmx
[ 	]*9[ 	]+\.arch \.sse2
[ 	]*10[ 	]+\?\?\?\? 0F77     		emms
[ 	]*11[ 	]+\?\?\?\? 0FAEE8   		lfence
[ 	]*12[ 	]+vzeroupper
[ 	]*13[ 	]+\.arch \.avx
[ 	]*14[ 	]+\?\?\?\? C5F877   		vzeroupper
[ 	]*15[ 	]+vpermpd \$7,%ymm6,%ymm2
[ 	]*16[ 	]+\.arch \.avx2
[ 	]*17[ 	]+\?\?\?\? C4E3FD01 		vpermpd \$7,%ymm6,%ymm2
[ 	]*17[ 	]+D607
[ 	]*18[ 	]+\.arch \.f16c
[ 	]*19[ 	]+\?\?\?\? C4E27D13 		vcvtph2ps %xmm4,%ymm4
[ 	]*19[ 	]+E4
[ 	]*20[ 	]+\.arch \.fma
[ 	]*21[ 	]+\?\?\?\? C4E2CD98 		vfmadd132pd %ymm4,%ymm6,%ymm2
[ 	]*21[ 	]+D4
[ 	]*22[ 	]+\.arch \.fma4
[ 	]*23[ 	]+\?\?\?\? C4E3ED69 		vfmaddpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*23[ 	]+FC60
[ 	]*24[ 	]+\.arch \.xop
[ 	]*25[ 	]+\?\?\?\? 8FE97881 		vfrczpd %xmm7,%xmm7
[ 	]*25[ 	]+FF
[ 	]*26[ 	]+\.arch \.noavx2
[ 	]*27[ 	]+\?\?\?\? C5F877   		vzeroupper
[ 	]*28[ 	]+\?\?\?\? C4E27D13 		vcvtph2ps %xmm4,%ymm4
[ 	]*28[ 	]+E4
[ 	]*29[ 	]+\?\?\?\? C4E2CD98 		vfmadd132pd %ymm4,%ymm6,%ymm2
[ 	]*29[ 	]+D4
[ 	]*30[ 	]+\?\?\?\? C4E3ED69 		vfmaddpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*30[ 	]+FC60
[ 	]*31[ 	]+\?\?\?\? 8FE97881 		vfrczpd %xmm7,%xmm7
[ 	]*31[ 	]+FF
[ 	]*32[ 	]+vpermpd \$7,%ymm6,%ymm2
[ 	]*33[ 	]+\?\?\?\? 0FAEE8   		lfence
[ 	]*34[ 	]+\.arch \.noavx
[ 	]*35[ 	]+vzeroupper
[ 	]*36[ 	]+vcvtph2ps %xmm4,%ymm4
[ 	]*37[ 	]+vfmadd132pd %ymm4,%ymm6,%ymm2
[ 	]*38[ 	]+vfmaddpd %ymm4,%ymm6,%ymm2,%ymm7
[ 	]*39[ 	]+vfrczpd %xmm7,%xmm7
[ 	]*40[ 	]+\?\?\?\? 0F77     		emms
[ 	]*41[ 	]+\?\?\?\? 0FAEE8   		lfence
[ 	]*42[ 	]+vaddps	%xmm6, %xmm5, %xmm4
[ 	]*43[ 	]+vaddps	%ymm6, %ymm5, %ymm4
[ 	]*44[ 	]+vaddps	%zmm6, %zmm5, %zmm4
[ 	]*45[ 	]+vaddss	%xmm6, %xmm5, %xmm4
#pass
