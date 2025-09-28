#objdump: -dw
#name: i386 CET

.*:     file format .*

Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	f3 0f ae e9          	incsspd %ecx
 +[a-f0-9]+:	f3 0f 1e c9          	rdsspd %ecx
 +[a-f0-9]+:	f3 0f 01 ea          	saveprevssp
 +[a-f0-9]+:	f3 0f 01 29          	rstorssp \(%ecx\)
 +[a-f0-9]+:	0f 38 f6 04 02       	wrssd  %eax,\(%edx,%eax,1\)
 +[a-f0-9]+:	66 0f 38 f5 14 2f    	wrussd %edx,\(%edi,%ebp,1\)
 +[a-f0-9]+:	f3 0f 01 e8          	setssbsy
 +[a-f0-9]+:	f3 0f ae 34 04       	clrssbsy \(%esp,%eax,1\)
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	f3 0f 1e fb          	endbr32
 +[a-f0-9]+:	f3 0f ae e9          	incsspd %ecx
 +[a-f0-9]+:	f3 0f 1e c9          	rdsspd %ecx
 +[a-f0-9]+:	f3 0f 01 ea          	saveprevssp
 +[a-f0-9]+:	f3 0f 01 6c 01 90    	rstorssp -0x70\(%ecx,%eax,1\)
 +[a-f0-9]+:	0f 38 f6 02          	wrssd  %eax,\(%edx\)
 +[a-f0-9]+:	0f 38 f6 10          	wrssd  %edx,\(%eax\)
 +[a-f0-9]+:	66 0f 38 f5 14 2f    	wrussd %edx,\(%edi,%ebp,1\)
 +[a-f0-9]+:	66 0f 38 f5 3c 0e    	wrussd %edi,\(%esi,%ecx,1\)
 +[a-f0-9]+:	f3 0f 01 e8          	setssbsy
 +[a-f0-9]+:	f3 0f ae 34 44       	clrssbsy \(%esp,%eax,2\)
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	f3 0f 1e fb          	endbr32
 +[a-f0-9]+:	f3 0f ae e9          	incsspd %ecx
 +[a-f0-9]+:	f3 0f 1e c9          	rdsspd %ecx
 +[a-f0-9]+:	f3 0f 01 ea          	saveprevssp
 +[a-f0-9]+:	67 f3 0f 01 6c 01    	rstorssp 0x1\(%si\)
 +[a-f0-9]+:	90                   	nop
 +[a-f0-9]+:	67 0f 38 f6 02       	wrssd  %eax,\(%bp,%si\)
 +[a-f0-9]+:	67 0f 38 f6 10       	wrssd  %edx,\(%bx,%si\)
 +[a-f0-9]+:	67 66 0f 38 f5 14    	wrussd %edx,\(%si\)
 +[a-f0-9]+:	2f                   	das
 +[a-f0-9]+:	67 66 0f 38 f5 3c    	wrussd %edi,\(%si\)
 +[a-f0-9]+:	0e                   	push   %cs
 +[a-f0-9]+:	f3 0f 01 e8          	setssbsy
 +[a-f0-9]+:	67 f3 0f ae 34       	clrssbsy \(%si\)
 +[a-f0-9]+:	44                   	inc    %esp
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	f3 0f 1e fb          	endbr32
#pass
