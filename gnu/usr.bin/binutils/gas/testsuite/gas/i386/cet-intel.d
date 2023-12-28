#source: cet.s
#as: -J
#objdump: -dw -Mintel
#name: i386 CET (Intel mode)

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	f3 0f ae e9          	incsspd ecx
 +[a-f0-9]+:	f3 0f 1e c9          	rdsspd ecx
 +[a-f0-9]+:	f3 0f 01 ea          	saveprevssp
 +[a-f0-9]+:	f3 0f 01 29          	rstorssp QWORD PTR \[ecx\]
 +[a-f0-9]+:	0f 38 f6 04 02       	wrssd  \[edx\+eax\*1\],eax
 +[a-f0-9]+:	66 0f 38 f5 14 2f    	wrussd \[edi\+ebp\*1\],edx
 +[a-f0-9]+:	f3 0f 01 e8          	setssbsy
 +[a-f0-9]+:	f3 0f ae 34 04       	clrssbsy QWORD PTR \[esp\+eax\*1\]
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	f3 0f 1e fb          	endbr32
 +[a-f0-9]+:	f3 0f ae e9          	incsspd ecx
 +[a-f0-9]+:	f3 0f 1e c9          	rdsspd ecx
 +[a-f0-9]+:	f3 0f 01 ea          	saveprevssp
 +[a-f0-9]+:	f3 0f 01 6c 01 90    	rstorssp QWORD PTR \[ecx\+eax\*1-0x70\]
 +[a-f0-9]+:	0f 38 f6 02          	wrssd  \[edx\],eax
 +[a-f0-9]+:	0f 38 f6 10          	wrssd  \[eax\],edx
 +[a-f0-9]+:	66 0f 38 f5 14 2f    	wrussd \[edi\+ebp\*1\],edx
 +[a-f0-9]+:	66 0f 38 f5 3c 0e    	wrussd \[esi\+ecx\*1\],edi
 +[a-f0-9]+:	f3 0f 01 e8          	setssbsy
 +[a-f0-9]+:	f3 0f ae 34 44       	clrssbsy QWORD PTR \[esp\+eax\*2\]
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	f3 0f 1e fb          	endbr32
 +[a-f0-9]+:	f3 0f ae e9          	incsspd ecx
 +[a-f0-9]+:	f3 0f 1e c9          	rdsspd ecx
 +[a-f0-9]+:	f3 0f 01 ea          	saveprevssp
 +[a-f0-9]+:	67 f3 0f 01 6c 01    	rstorssp QWORD PTR \[si\+0x1\]
 +[a-f0-9]+:	90                   	nop
 +[a-f0-9]+:	67 0f 38 f6 02       	wrssd  \[bp\+si\],eax
 +[a-f0-9]+:	67 0f 38 f6 10       	wrssd  \[bx\+si\],edx
 +[a-f0-9]+:	67 66 0f 38 f5 14    	wrussd \[si\],edx
 +[a-f0-9]+:	2f                   	das
 +[a-f0-9]+:	67 66 0f 38 f5 3c    	wrussd \[si\],edi
 +[a-f0-9]+:	0e                   	push   cs
 +[a-f0-9]+:	f3 0f 01 e8          	setssbsy
 +[a-f0-9]+:	67 f3 0f ae 34       	clrssbsy QWORD PTR \[si\]
 +[a-f0-9]+:	44                   	inc    esp
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	f3 0f 1e fb          	endbr32
#pass
