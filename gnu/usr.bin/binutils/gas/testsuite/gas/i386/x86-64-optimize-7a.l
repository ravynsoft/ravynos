.*: Assembler messages:
.*:6: Error: .*
.*:7: Error: .*
.*:8: Error: .*
.*:9: Error: .*
.*:11: Error: .*
.*:12: Error: .*
.*:13: Error: .*
.*:14: Error: .*
.*:16: Error: .*
.*:17: Error: .*
.*:18: Error: .*
.*:19: Error: .*
.*:21: Error: .*
.*:22: Error: .*
.*:23: Error: .*
.*:24: Error: .*
.*:26: Error: .*
.*:27: Error: .*
.*:28: Error: .*
.*:29: Error: .*
.*:31: Error: .*
.*:32: Error: .*
.*:33: Error: .*
.*:34: Error: .*
.*:36: Error: .*
.*:37: Error: .*
.*:38: Error: .*
.*:39: Error: .*
.*:41: Error: .*
.*:42: Error: .*
.*:43: Error: .*
.*:44: Error: .*
.*:46: Error: .*
.*:47: Error: .*
.*:48: Error: .*
.*:49: Error: .*
.*:51: Error: .*
.*:52: Error: .*
.*:53: Error: .*
.*:54: Error: .*
.*:56: Error: .*
.*:57: Error: .*
.*:58: Error: .*
.*:59: Error: .*
.*:61: Error: .*
.*:62: Error: .*
.*:63: Error: .*
.*:64: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\# Check 64bit instructions with optimized encoding
[ 	]*2[ 	]+
[ 	]*3[ 	]+\.include "x86-64-optimize-7\.s"
[ 	]*1[ 	]+\# Check 64bit instructions with optimized encoding
[ 	]*2[ 	]+
[ 	]*3[ 	]+\.allow_index_reg
[ 	]*4[ 	]+\.text
[ 	]*5[ 	]+_start:
[ 	]*6[ 	]+vandnpd %zmm1, %zmm1, %zmm15\{%k7\}
[ 	]*7[ 	]+vandnpd %zmm1, %zmm1, %zmm15
[ 	]*8[ 	]+vandnpd %zmm1, %zmm1, %zmm16
[ 	]*9[ 	]+vandnpd %zmm17, %zmm17, %zmm1
[ 	]*10[ 	]+
[ 	]*11[ 	]+vandnps %zmm1, %zmm1, %zmm15\{%k7\}
[ 	]*12[ 	]+vandnps %zmm1, %zmm1, %zmm15
[ 	]*13[ 	]+vandnps %zmm1, %zmm1, %zmm16
[ 	]*14[ 	]+vandnps %zmm17, %zmm17, %zmm1
[ 	]*15[ 	]+
[ 	]*16[ 	]+vpandnd %zmm1, %zmm1, %zmm15\{%k7\}
[ 	]*17[ 	]+vpandnd %zmm1, %zmm1, %zmm15
[ 	]*18[ 	]+vpandnd %zmm1, %zmm1, %zmm16
[ 	]*19[ 	]+vpandnd %zmm17, %zmm17, %zmm1
[ 	]*20[ 	]+
[ 	]*21[ 	]+vpandnq %zmm1, %zmm1, %zmm15\{%k7\}
[ 	]*22[ 	]+vpandnq %zmm1, %zmm1, %zmm15
[ 	]*23[ 	]+vpandnq %zmm1, %zmm1, %zmm16
[ 	]*24[ 	]+vpandnq %zmm17, %zmm17, %zmm1
[ 	]*25[ 	]+
[ 	]*26[ 	]+vxorpd %zmm1, %zmm1, %zmm15\{%k7\}
[ 	]*27[ 	]+vxorpd %zmm1, %zmm1, %zmm15
[ 	]*28[ 	]+vxorpd %zmm1, %zmm1, %zmm16
[ 	]*29[ 	]+vxorpd %zmm17, %zmm17, %zmm1
[ 	]*30[ 	]+
[ 	]*31[ 	]+vxorps %zmm1, %zmm1, %zmm15\{%k7\}
[ 	]*32[ 	]+vxorps %zmm1, %zmm1, %zmm15
[ 	]*33[ 	]+vxorps %zmm1, %zmm1, %zmm16
[ 	]*34[ 	]+vxorps %zmm17, %zmm17, %zmm1
[ 	]*35[ 	]+
[ 	]*36[ 	]+vpxord %zmm1, %zmm1, %zmm15\{%k7\}
[ 	]*37[ 	]+vpxord %zmm1, %zmm1, %zmm15
[ 	]*38[ 	]+vpxord %zmm1, %zmm1, %zmm16
[ 	]*39[ 	]+vpxord %zmm17, %zmm17, %zmm1
[ 	]*40[ 	]+
[ 	]*41[ 	]+vpxorq %zmm1, %zmm1, %zmm15\{%k7\}
[ 	]*42[ 	]+vpxorq %zmm1, %zmm1, %zmm15
[ 	]*43[ 	]+vpxorq %zmm1, %zmm1, %zmm16
[ 	]*44[ 	]+vpxorq %zmm17, %zmm17, %zmm1
[ 	]*45[ 	]+
[ 	]*46[ 	]+vpsubb %zmm1, %zmm1, %zmm15\{%k7\}
[ 	]*47[ 	]+vpsubb %zmm1, %zmm1, %zmm15
[ 	]*48[ 	]+vpsubb %zmm1, %zmm1, %zmm16
[ 	]*49[ 	]+vpsubb %zmm17, %zmm17, %zmm1
[ 	]*50[ 	]+
[ 	]*51[ 	]+vpsubw %zmm1, %zmm1, %zmm15\{%k7\}
[ 	]*52[ 	]+vpsubw %zmm1, %zmm1, %zmm15
[ 	]*53[ 	]+vpsubw %zmm1, %zmm1, %zmm16
[ 	]*54[ 	]+vpsubw %zmm17, %zmm17, %zmm1
GAS LISTING .*


[ 	]*55[ 	]+
[ 	]*56[ 	]+vpsubd %zmm1, %zmm1, %zmm15\{%k7\}
[ 	]*57[ 	]+vpsubd %zmm1, %zmm1, %zmm15
[ 	]*58[ 	]+vpsubd %zmm1, %zmm1, %zmm16
[ 	]*59[ 	]+vpsubd %zmm17, %zmm17, %zmm1
[ 	]*60[ 	]+
[ 	]*61[ 	]+vpsubq %zmm1, %zmm1, %zmm15\{%k7\}
[ 	]*62[ 	]+vpsubq %zmm1, %zmm1, %zmm15
[ 	]*63[ 	]+vpsubq %zmm1, %zmm1, %zmm16
[ 	]*64[ 	]+vpsubq %zmm17, %zmm17, %zmm1
#pass
