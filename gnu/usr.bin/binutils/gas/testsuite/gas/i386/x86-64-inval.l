.*: Assembler messages:
.*:4: Error: .*
.*:5: Error: .*
.*:6: Error: .*
.*:7: Error: .*
.*:8: Error: .*
.*:9: Error: .*
.*:10: Error: .*
.*:11: Error: .*
.*:12: Error: .*
.*:13: Error: .*
.*:14: Error: .*
.*:15: Error: .*
.*:16: Error: .*
.*:17: Error: .*
.*:18: Error: .*
.*:19: Error: .*
.*:20: Error: .*
.*:21: Error: .*
.*:22: Error: .*
.*:23: Error: .*
.*:24: Error: .*
.*:25: Error: .*
.*:26: Error: .*
.*:27: Error: .*
.*:28: Error: .*
.*:29: Error: .*
.*:30: Error: .*
.*:31: Error: .*
.*:32: Error: .*
.*:33: Error: .*
.*:34: Error: .*
.*:35: Error: .*
.*:36: Error: .*
.*:37: Error: .*
.*:38: Error: .*
.*:39: Error: .*
.*:40: Error: .*
.*:41: Error: .*
.*:42: Error: .*
.*:43: Error: .*
.*:44: Error: .*
.*:45: Error: .*
.*:46: Error: .*
.*:47: Error: .*
.*:48: Error: .*
.*:49: Error: .*
.*:50: Error: .*
.*:51: Error: .*
.*:52: Error: .*
.*:53: Error: .*
.*:54: Error: .*
.*:55: Error: .*
.*:56: Error: .*
.*:57: Error: .*
.*:58: Error: .*
.*:59: Error: .*
.*:60: Error: .*
.*:61: Error: .*
.*:62: Error: .*
.*:63: Error: .*
.*:64: Error: .*
.*:65: Error: .*
.*:67: Error: .*
.*:68: Error: .*
.*:70: Error: .*
.*:71: Error: .*
.*:72: Error: .*
.*:73: Error: .*
.*:74: Error: .*
.*:75: Error: .*
.*:76: Error: .*
.*:77: Error: .*
.*:78: Error: .*
.*:79: Error: .*
.*:80: Error: .*
.*:81: Error: .*
.*:84: Error: .*
.*:85: Error: .*
.*:86: Error: .*
.*:87: Error: .*
.*:88: Error: .*
.*:89: Error: .*
.*:90: Error: .*
.*:91: Error: .*
.*:92: Error: .*
.*:93: Error: .*
.*:94: Error: .*
.*:95: Error: .*
.*:96: Error: .*
.*:97: Error: .*
.*:98: Error: .*
.*:99: Error: .*
.*:100: Error: .*
.*:101: Error: .*
.*:102: Error: .*
.*:103: Error: .*
.*:104: Error: .*
.*:105: Error: .*
.*:106: Error: .*
.*:107: Error: .*
.*:108: Error: .*
.*:109: Error: .*
.*:110: Error: .*
.*:111: Error: .*
.*:112: Error: .*
.*:113: Error: .*
.*:114: Error: .*
.*:115: Error: .*
.*:116: Error: .*
.*:117: Error: .*
.*:118: Error: .*
.*:121: Error: .*
.*:123: Error: .*
.*:124: Error: .*
.*:125: Error: .*
.*:126: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+\.allow_index_reg
[ 	]*3[ 	]+\# All the following should be illegal for x86-64
[ 	]*4[ 	]+aaa		\# illegal
[ 	]*5[ 	]+aad		\# illegal
[ 	]*6[ 	]+aam		\# illegal
[ 	]*7[ 	]+aas		\# illegal
[ 	]*8[ 	]+arpl %ax,%ax	\# illegal
[ 	]*9[ 	]+bound %eax,\(%rax\) \# illegal
[ 	]*10[ 	]+calll \*%eax	\# 32-bit data size not allowed
[ 	]*11[ 	]+calll \*\(%ax\)	\# 32-bit data size not allowed
[ 	]*12[ 	]+calll \*\(%eax\)	\# 32-bit data size not allowed
[ 	]*13[ 	]+calll \*\(%r8\)	\# 32-bit data size not allowed
[ 	]*14[ 	]+calll \*\(%rax\)	\# 32-bit data size not allowed
[ 	]*15[ 	]+callq \*\(%ax\)	\# no 16-bit addressing
[ 	]*16[ 	]+daa		\# illegal
[ 	]*17[ 	]+das		\# illegal
[ 	]*18[ 	]+enterl \$0,\$0	\# can't have 32-bit stack operands
[ 	]*19[ 	]+into		\# illegal
[ 	]*20[ 	]+foo:	jcxz foo	\# No prefix exists to select CX as a counter
[ 	]*21[ 	]+jmpl \*%eax	\# 32-bit data size not allowed
[ 	]*22[ 	]+jmpl \*\(%ax\)	\# 32-bit data size not allowed
[ 	]*23[ 	]+jmpl \*\(%eax\)	\# 32-bit data size not allowed
[ 	]*24[ 	]+jmpl \*\(%r8\)	\# 32-bit data size not allowed
[ 	]*25[ 	]+jmpl \*\(%rax\)	\# 32-bit data size not allowed
[ 	]*26[ 	]+jmpq \*\(%ax\)	\# no 16-bit addressing
[ 	]*27[ 	]+lcalll \$0,\$0	\# illegal
[ 	]*28[ 	]+lcallq \$0,\$0	\# illegal
[ 	]*29[ 	]+ldsl %eax,\(%rax\) \# illegal
[ 	]*30[ 	]+ldsq %rax,\(%rax\) \# illegal
[ 	]*31[ 	]+lesl %eax,\(%rax\) \# illegal
[ 	]*32[ 	]+lesq %rax,\(%rax\) \# illegal
[ 	]*33[ 	]+ljmpl \$0,\$0	\# illegal
[ 	]*34[ 	]+ljmpq \$0,\$0	\# illegal
[ 	]*35[ 	]+ljmpq \*\(%rax\)	\# 64-bit data size not allowed
[ 	]*36[ 	]+loopw foo	\# No prefix exists to select CX as a counter
[ 	]*37[ 	]+loopew foo	\# No prefix exists to select CX as a counter
[ 	]*38[ 	]+loopnew foo	\# No prefix exists to select CX as a counter
[ 	]*39[ 	]+loopnzw foo	\# No prefix exists to select CX as a counter
[ 	]*40[ 	]+loopzw foo	\# No prefix exists to select CX as a counter
[ 	]*41[ 	]+leavel		\# can't have 32-bit stack operands
[ 	]*42[ 	]+pop %ds		\# illegal
[ 	]*43[ 	]+pop %es		\# illegal
[ 	]*44[ 	]+pop %ss		\# illegal
[ 	]*45[ 	]+popa		\# illegal
[ 	]*46[ 	]+popl %eax	\# can't have 32-bit stack operands
[ 	]*47[ 	]+push %cs	\# illegal
[ 	]*48[ 	]+push %ds	\# illegal
[ 	]*49[ 	]+push %es	\# illegal
[ 	]*50[ 	]+push %ss	\# illegal
[ 	]*51[ 	]+pusha		\# illegal
[ 	]*52[ 	]+pushl %eax	\# can't have 32-bit stack operands
[ 	]*53[ 	]+pushfl		\# can't have 32-bit stack operands
[ 	]*54[ 	]+popfl		\# can't have 32-bit stack operands
[ 	]*55[ 	]+retl		\# can't have 32-bit stack operands
[ 	]*56[ 	]+insertq \$4,\$2,%xmm2,%ebx \# The last operand must be XMM register\.
[ 	]*57[ 	]+fnstsw %eax
GAS LISTING .*


[ 	]*58[ 	]+fnstsw %al
[ 	]*59[ 	]+fstsw %eax
[ 	]*60[ 	]+fstsw %al
[ 	]*61[ 	]+in \$8,%rax
[ 	]*62[ 	]+out %rax,\$8
[ 	]*63[ 	]+movzxl \(%rax\),%rax
[ 	]*64[ 	]+movnti %ax, \(%rax\)
[ 	]*65[ 	]+movntiw %ax, \(%rax\)
[ 	]*66[ 	]+
[ 	]*67[ 	]+mov 0x80000000\(%rax\),%ebx
[ 	]*68[ 	]+mov 0x80000000,%ebx
[ 	]*69[ 	]+
[ 	]*70[ 	]+add \(%rip,%rsi\), %eax
[ 	]*71[ 	]+add \(%rsi,%rip\), %eax
[ 	]*72[ 	]+add \(,%rip\), %eax
[ 	]*73[ 	]+add \(%eip,%esi\), %eax
[ 	]*74[ 	]+add \(%esi,%eip\), %eax
[ 	]*75[ 	]+add \(,%eip\), %eax
[ 	]*76[ 	]+add \(%rsi,%esi\), %eax
[ 	]*77[ 	]+add \(%esi,%rsi\), %eax
[ 	]*78[ 	]+add \(%eiz\), %eax
[ 	]*79[ 	]+add \(%riz\), %eax
[ 	]*80[ 	]+add \(%rax\), %riz
[ 	]*81[ 	]+add \(%rax\), %eiz
[ 	]*82[ 	]+
[ 	]*83[ 	]+\.intel_syntax noprefix
[ 	]*84[ 	]+cmpxchg16b dword ptr \[rax\] \# Must be oword
[ 	]*85[ 	]+movq xmm1, XMMWORD PTR \[rsp\]
[ 	]*86[ 	]+movq xmm1, DWORD PTR \[rsp\]
[ 	]*87[ 	]+movq xmm1, WORD PTR \[rsp\]
[ 	]*88[ 	]+movq xmm1, BYTE PTR \[rsp\]
[ 	]*89[ 	]+movq XMMWORD PTR \[rsp\],xmm1
[ 	]*90[ 	]+movq DWORD PTR \[rsp\],xmm1
[ 	]*91[ 	]+movq WORD PTR \[rsp\],xmm1
[ 	]*92[ 	]+movq BYTE PTR \[rsp\],xmm1
[ 	]*93[ 	]+fnstsw eax
[ 	]*94[ 	]+fnstsw al
[ 	]*95[ 	]+fstsw eax
[ 	]*96[ 	]+fstsw al
[ 	]*97[ 	]+in rax,8
[ 	]*98[ 	]+out 8,rax
[ 	]*99[ 	]+movsx ax, \[rax\]
[ 	]*100[ 	]+movsx eax, \[rax\]
[ 	]*101[ 	]+movsx rax, \[rax\]
[ 	]*102[ 	]+movzx ax, \[rax\]
[ 	]*103[ 	]+movzx eax, \[rax\]
[ 	]*104[ 	]+movzx rax, \[rax\]
[ 	]*105[ 	]+movnti word ptr \[rax\], ax
[ 	]*106[ 	]+calld eax	\# 32-bit data size not allowed
[ 	]*107[ 	]+calld \[ax\]	\# 32-bit data size not allowed
[ 	]*108[ 	]+calld \[eax\]	\# 32-bit data size not allowed
[ 	]*109[ 	]+calld \[r8\]	\# 32-bit data size not allowed
[ 	]*110[ 	]+calld \[rax\]	\# 32-bit data size not allowed
[ 	]*111[ 	]+callq \[ax\]	\# no 16-bit addressing
[ 	]*112[ 	]+jmpd eax	\# 32-bit data size not allowed
[ 	]*113[ 	]+jmpd \[ax\]	\# 32-bit data size not allowed
[ 	]*114[ 	]+jmpd \[eax\]	\# 32-bit data size not allowed
GAS LISTING .*


[ 	]*115[ 	]+jmpd \[r8\]	\# 32-bit data size not allowed
[ 	]*116[ 	]+jmpd \[rax\]	\# 32-bit data size not allowed
[ 	]*117[ 	]+jmpq \[ax\]	\# no 16-bit addressing
[ 	]*[1-9][0-9]*[ 	]+mov eax,\[rax\+0x876543210\] \# out of range displacement
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\.att_syntax prefix
[ 	]*[1-9][0-9]*[ 	]+movsd \(%rsi\), %ss:\(%rdi\), %ss:\(%rax\)
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+incl	\(%dx\)
[ 	]*[1-9][0-9]*[ 	]+incw	\(%dx\)
[ 	]*[1-9][0-9]*[ 	]+mov	\(%dx\), %ax
[ 	]*[1-9][0-9]*[ 	]+mov	%ax, \(%dx\)
