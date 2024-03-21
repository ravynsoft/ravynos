.*: Assembler messages:
.*:6: Error: .*
.*:7: Error: .*
.*:9: Error: .*
.*:10: Error: .*
.*:12: Error: .*
.*:13: Error: .*
.*:15: Error: .*
.*:16: Error: .*
.*:18: Error: .*
.*:19: Error: .*
.*:21: Error: .*
.*:22: Error: .*
.*:24: Error: .*
.*:25: Error: .*
.*:27: Error: .*
.*:28: Error: .*
.*:30: Error: .*
.*:31: Error: .*
.*:33: Error: .*
.*:34: Error: .*
.*:36: Error: .*
.*:37: Error: .*
.*:39: Error: .*
.*:40: Error: .*
.*:42: Error: .*
.*:43: Error: .*
.*:45: Error: .*
.*:46: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\# Check instructions with optimized encoding
[ 	]*2[ 	]+
[ 	]*3[ 	]+\.include "optimize-6\.s"
[ 	]*1[ 	]+\# Check instructions with optimized encoding
[ 	]*2[ 	]+
[ 	]*3[ 	]+\.allow_index_reg
[ 	]*4[ 	]+\.text
[ 	]*5[ 	]+_start:
[ 	]*6[ 	]+vandnpd %zmm1, %zmm1, %zmm5\{%k7\}
[ 	]*7[ 	]+vandnpd %zmm1, %zmm1, %zmm5
[ 	]*8[ 	]+
[ 	]*9[ 	]+vandnps %zmm1, %zmm1, %zmm5\{%k7\}
[ 	]*10[ 	]+vandnps %zmm1, %zmm1, %zmm5
[ 	]*11[ 	]+
[ 	]*12[ 	]+vpandnd %zmm1, %zmm1, %zmm5\{%k7\}
[ 	]*13[ 	]+vpandnd %zmm1, %zmm1, %zmm5
[ 	]*14[ 	]+
[ 	]*15[ 	]+vpandnq %zmm1, %zmm1, %zmm5\{%k7\}
[ 	]*16[ 	]+vpandnq %zmm1, %zmm1, %zmm5
[ 	]*17[ 	]+
[ 	]*18[ 	]+vxorpd %zmm1, %zmm1, %zmm5\{%k7\}
[ 	]*19[ 	]+vxorpd %zmm1, %zmm1, %zmm5
[ 	]*20[ 	]+
[ 	]*21[ 	]+vxorps %zmm1, %zmm1, %zmm5\{%k7\}
[ 	]*22[ 	]+vxorps %zmm1, %zmm1, %zmm5
[ 	]*23[ 	]+
[ 	]*24[ 	]+vpxord %zmm1, %zmm1, %zmm5\{%k7\}
[ 	]*25[ 	]+vpxord %zmm1, %zmm1, %zmm5
[ 	]*26[ 	]+
[ 	]*27[ 	]+vpxorq %zmm1, %zmm1, %zmm5\{%k7\}
[ 	]*28[ 	]+vpxorq %zmm1, %zmm1, %zmm5
[ 	]*29[ 	]+
[ 	]*30[ 	]+vpsubb %zmm1, %zmm1, %zmm5\{%k7\}
[ 	]*31[ 	]+vpsubb %zmm1, %zmm1, %zmm5
[ 	]*32[ 	]+
[ 	]*33[ 	]+vpsubw %zmm1, %zmm1, %zmm5\{%k7\}
[ 	]*34[ 	]+vpsubw %zmm1, %zmm1, %zmm5
[ 	]*35[ 	]+
[ 	]*36[ 	]+vpsubd %zmm1, %zmm1, %zmm5\{%k7\}
[ 	]*37[ 	]+vpsubd %zmm1, %zmm1, %zmm5
[ 	]*38[ 	]+
[ 	]*39[ 	]+vpsubq %zmm1, %zmm1, %zmm5\{%k7\}
[ 	]*40[ 	]+vpsubq %zmm1, %zmm1, %zmm5
[ 	]*41[ 	]+
[ 	]*42[ 	]+kxord %k1, %k1, %k5
[ 	]*43[ 	]+kxorq %k1, %k1, %k5
[ 	]*44[ 	]+
[ 	]*45[ 	]+kandnd %k1, %k1, %k5
[ 	]*46[ 	]+kandnq %k1, %k1, %k5
#pass
