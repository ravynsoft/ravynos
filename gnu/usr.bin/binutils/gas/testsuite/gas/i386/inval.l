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
.*:59: Error: .*
.*:60: Error: .*
.*:62: Error: .*
.*:63: Error: .*
.*:64: Error: .*
.*:65: Error: .*
.*:67: Error: .*mov.*
.*:68: Error: .*mov.*
.*:69: Error: .*mov.*
.*:70: Error: .*mov.*
.*:73: Error: .*
.*:74: Error: .*
.*:75: Error: .*
.*:76: Error: .*
.*:77: Error: .*
.*:78: Error: .*
.*:79: Error: .*
.*:80: Error: .*
.*:81: Error: .*
.*:82: Error: .*
.*:83: Error: .*
.*:84: Error: .*
.*:85: Error: .*
.*:86: Error: .*
.*:87: Error: .*
.*:88: Error: .*
.*:90: Error: .*
.*:91: Error: .*
.*:92: Error: .*
.*:93: Error: .*
.*:95: Error: .*
.*:97: Error: .*shl.*
.*:98: Error: .*rol.*
.*:99: Error: .*rcl.*
.*:101: Error: .*extrq.*
.*:102: Error: .*insertq.*
.*:105: Error: .*
.*:107: Error: .*
.*:108: Error: .*
.*:109: Error: .*
.*:111: Error: .*
.*:112: Error: .*
.*:113: Error: .*
.*:115: Error: .*
.*:116: Error: .*
.*:117: Error: .*
.*:119: Error: .*
.*:120: Error: .*
.*:121: Error: .*
.*:122: Error: .*
[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+\.allow_index_reg
[ 	]*3[ 	]+\# All the following should be illegal
[ 	]*4[ 	]+mov	\(%dx\),%al
[ 	]*5[ 	]+mov	\(%eax,%esp,2\),%al
[ 	]*6[ 	]+setae	%eax
[ 	]*7[ 	]+pushb	%ds
[ 	]*8[ 	]+popb	%ds
[ 	]*9[ 	]+pushb	%al
[ 	]*10[ 	]+popb	%al
[ 	]*11[ 	]+pushb	%ah
[ 	]*12[ 	]+popb	%ah
[ 	]*13[ 	]+pushb	%ax
[ 	]*14[ 	]+popb	%ax
[ 	]*15[ 	]+pushb	%eax
[ 	]*16[ 	]+popb	%eax
[ 	]*17[ 	]+movb	%ds,%ax
[ 	]*18[ 	]+movb	%ds,%eax
[ 	]*19[ 	]+movb	%ax,%ds
[ 	]*20[ 	]+movb	%eax,%ds
[ 	]*21[ 	]+movdb	%eax,%mm0
[ 	]*22[ 	]+movqb	0,%mm0
[ 	]*23[ 	]+ldsb	0,%eax
[ 	]*24[ 	]+setnew	0
[ 	]*25[ 	]+movdw	%eax,%mm0
[ 	]*26[ 	]+movqw	0,%mm0
[ 	]*27[ 	]+div	%cx,%al
[ 	]*28[ 	]+div	%cl,%ax
[ 	]*29[ 	]+div	%ecx,%al
[ 	]*30[ 	]+imul	10,%bx,%ecx
[ 	]*31[ 	]+imul	10,%bx,%al
[ 	]*32[ 	]+popab
[ 	]*33[ 	]+stil
[ 	]*34[ 	]+aaab
[ 	]*35[ 	]+cwdel
[ 	]*36[ 	]+cwdw
[ 	]*37[ 	]+callww	0
[ 	]*38[ 	]+foo:	jaw	foo
[ 	]*39[ 	]+jcxzw	foo
[ 	]*40[ 	]+jecxzl	foo
[ 	]*41[ 	]+loopb	foo
[ 	]*42[ 	]+xlatw	%es:%bx
[ 	]*43[ 	]+xlatl	%es:%bx
[ 	]*44[ 	]+intl	2
[ 	]*45[ 	]+int3b
[ 	]*46[ 	]+hltb
[ 	]*47[ 	]+fstb	%st\(0\)
[ 	]*48[ 	]+fcompll	28\(%ebp\)
[ 	]*49[ 	]+fldlw	\(%eax\)
[ 	]*50[ 	]+movl	\$%ebx,%eax
[ 	]*51[ 	]+insertq	\$4,\$2,%xmm2,%ebx
[ 	]*52[ 	]+cvtsi2ssq \(%eax\),%xmm1
[ 	]*53[ 	]+cvtsi2sdq \(%eax\),%xmm1
[ 	]*54[ 	]+fnstsw %eax
[ 	]*55[ 	]+fnstsw %al
[ 	]*56[ 	]+fstsw %eax
[ 	]*57[ 	]+fstsw %al
[ 	]*58[ 	]+
[ 	]*59[ 	]+movnti %ax, \(%eax\)
[ 	]*60[ 	]+movntiw %ax, \(%eax\)
[ 	]*61[ 	]+
[ 	]*62[ 	]+add \(%si,%esi\), %eax
[ 	]*63[ 	]+add \(%esi,%si\), %eax
[ 	]*64[ 	]+add \(%eiz\), %eax
[ 	]*65[ 	]+add \(%eax\), %eiz
[ 	]*66[ 	]+
[ 	]*[1-9][0-9]*[ 	]+mov %cr0, %di
[ 	]*[1-9][0-9]*[ 	]+mov %ax, %cr7
[ 	]*[1-9][0-9]*[ 	]+mov %cr0, %bh
[ 	]*[1-9][0-9]*[ 	]+mov %al, %cr7
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\.intel_syntax noprefix
[ 	]*[1-9][0-9]*[ 	]+cvtsi2ss xmm1,QWORD PTR \[eax\]
[ 	]*[1-9][0-9]*[ 	]+cvtsi2sd xmm1,QWORD PTR \[eax\]
[ 	]*[1-9][0-9]*[ 	]+cvtsi2ssq xmm1,QWORD PTR \[eax\]
[ 	]*[1-9][0-9]*[ 	]+cvtsi2sdq xmm1,QWORD PTR \[eax\]
[ 	]*[1-9][0-9]*[ 	]+movq xmm1, XMMWORD PTR \[esp\]
[ 	]*[1-9][0-9]*[ 	]+movq xmm1, DWORD PTR \[esp\]
[ 	]*[1-9][0-9]*[ 	]+movq xmm1, WORD PTR \[esp\]
[ 	]*[1-9][0-9]*[ 	]+movq xmm1, BYTE PTR \[esp\]
[ 	]*[1-9][0-9]*[ 	]+movq XMMWORD PTR \[esp\],xmm1
[ 	]*[1-9][0-9]*[ 	]+movq DWORD PTR \[esp\],xmm1
[ 	]*[1-9][0-9]*[ 	]+movq WORD PTR \[esp\],xmm1
[ 	]*[1-9][0-9]*[ 	]+movq BYTE PTR \[esp\],xmm1
[ 	]*[1-9][0-9]*[ 	]+fnstsw eax
[ 	]*[1-9][0-9]*[ 	]+fnstsw al
[ 	]*[1-9][0-9]*[ 	]+fstsw eax
[ 	]*[1-9][0-9]*[ 	]+fstsw al
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+movsx ax, \[eax\]
[ 	]*[1-9][0-9]*[ 	]+movsx eax, \[eax\]
[ 	]*[1-9][0-9]*[ 	]+movzx ax, \[eax\]
[ 	]*[1-9][0-9]*[ 	]+movzx eax, \[eax\]
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+movnti word ptr \[eax\], ax
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+shl \[eax\], 1
[ 	]*[1-9][0-9]*[ 	]+rol \[ecx\], 2
[ 	]*[1-9][0-9]*[ 	]+rcl \[edx\], cl
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+extrq 1, 2, xmm3
[ 	]*[1-9][0-9]*[ 	]+insertq 1, 2, xmm3, xmm4
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\.att_syntax prefix
[ 	]*[1-9][0-9]*[ 	]+movsd \(%esi\), %ss:\(%edi\), %ss:\(%eax\)
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+movl	%ds, %ax
[ 	]*[1-9][0-9]*[ 	]+movl	%ax, %ds
[ 	]*[1-9][0-9]*[ 	]+movl	%ax, %bx
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+movw	%ds, %eax
[ 	]*[1-9][0-9]*[ 	]+movw	%eax, %ds
[ 	]*[1-9][0-9]*[ 	]+movw	%eax, %ebx
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+inb	%dx, %ax
[ 	]*[1-9][0-9]*[ 	]+outb	%ax, %dx
[ 	]*[1-9][0-9]*[ 	]+movb	%ax, %bx
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+incl	\(%dx\)
[ 	]*[1-9][0-9]*[ 	]+incw	\(%dx\)
[ 	]*[1-9][0-9]*[ 	]+mov	\(%dx\), %ax
[ 	]*[1-9][0-9]*[ 	]+mov	%ax, \(%dx\)
#pass
