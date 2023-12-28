#objdump: -drw
#name: i386 nops 8

.*: +file format .*

Disassembly of section .text:

0+ <_0f18>:
 +[a-f0-9]+:	0f 18 c0             	nop    %eax
 +[a-f0-9]+:	0f 18 c1             	nop    %ecx
 +[a-f0-9]+:	0f 18 c2             	nop    %edx
 +[a-f0-9]+:	0f 18 c3             	nop    %ebx
 +[a-f0-9]+:	0f 18 c4             	nop    %esp
 +[a-f0-9]+:	0f 18 c5             	nop    %ebp
 +[a-f0-9]+:	0f 18 c6             	nop    %esi
 +[a-f0-9]+:	0f 18 c7             	nop    %edi
 +[a-f0-9]+:	0f 18 c8             	nop    %eax
 +[a-f0-9]+:	0f 18 c9             	nop    %ecx
 +[a-f0-9]+:	0f 18 ca             	nop    %edx
 +[a-f0-9]+:	0f 18 cb             	nop    %ebx
 +[a-f0-9]+:	0f 18 cc             	nop    %esp
 +[a-f0-9]+:	0f 18 cd             	nop    %ebp
 +[a-f0-9]+:	0f 18 ce             	nop    %esi
 +[a-f0-9]+:	0f 18 cf             	nop    %edi
 +[a-f0-9]+:	0f 18 d0             	nop    %eax
 +[a-f0-9]+:	0f 18 d1             	nop    %ecx
 +[a-f0-9]+:	0f 18 d2             	nop    %edx
 +[a-f0-9]+:	0f 18 d3             	nop    %ebx
 +[a-f0-9]+:	0f 18 d4             	nop    %esp
 +[a-f0-9]+:	0f 18 d5             	nop    %ebp
 +[a-f0-9]+:	0f 18 d6             	nop    %esi
 +[a-f0-9]+:	0f 18 d7             	nop    %edi
 +[a-f0-9]+:	0f 18 d8             	nop    %eax
 +[a-f0-9]+:	0f 18 d9             	nop    %ecx
 +[a-f0-9]+:	0f 18 da             	nop    %edx
 +[a-f0-9]+:	0f 18 db             	nop    %ebx
 +[a-f0-9]+:	0f 18 dc             	nop    %esp
 +[a-f0-9]+:	0f 18 dd             	nop    %ebp
 +[a-f0-9]+:	0f 18 de             	nop    %esi
 +[a-f0-9]+:	0f 18 df             	nop    %edi
 +[a-f0-9]+:	0f 18 e0             	nop    %eax
 +[a-f0-9]+:	0f 18 e1             	nop    %ecx
 +[a-f0-9]+:	0f 18 e2             	nop    %edx
 +[a-f0-9]+:	0f 18 e3             	nop    %ebx
 +[a-f0-9]+:	0f 18 e4             	nop    %esp
 +[a-f0-9]+:	0f 18 e5             	nop    %ebp
 +[a-f0-9]+:	0f 18 e6             	nop    %esi
 +[a-f0-9]+:	0f 18 e7             	nop    %edi
 +[a-f0-9]+:	0f 18 e8             	nop    %eax
 +[a-f0-9]+:	0f 18 e9             	nop    %ecx
 +[a-f0-9]+:	0f 18 ea             	nop    %edx
 +[a-f0-9]+:	0f 18 eb             	nop    %ebx
 +[a-f0-9]+:	0f 18 ec             	nop    %esp
 +[a-f0-9]+:	0f 18 ed             	nop    %ebp
 +[a-f0-9]+:	0f 18 ee             	nop    %esi
 +[a-f0-9]+:	0f 18 ef             	nop    %edi
 +[a-f0-9]+:	0f 18 f0             	nop    %eax
 +[a-f0-9]+:	0f 18 f1             	nop    %ecx
 +[a-f0-9]+:	0f 18 f2             	nop    %edx
 +[a-f0-9]+:	0f 18 f3             	nop    %ebx
 +[a-f0-9]+:	0f 18 f4             	nop    %esp
 +[a-f0-9]+:	0f 18 f5             	nop    %ebp
 +[a-f0-9]+:	0f 18 f6             	nop    %esi
 +[a-f0-9]+:	0f 18 f7             	nop    %edi
 +[a-f0-9]+:	0f 18 f8             	nop    %eax
 +[a-f0-9]+:	0f 18 f9             	nop    %ecx
 +[a-f0-9]+:	0f 18 fa             	nop    %edx
 +[a-f0-9]+:	0f 18 fb             	nop    %ebx
 +[a-f0-9]+:	0f 18 fc             	nop    %esp
 +[a-f0-9]+:	0f 18 fd             	nop    %ebp
 +[a-f0-9]+:	0f 18 fe             	nop    %esi
 +[a-f0-9]+:	0f 18 ff             	nop    %edi
 +[a-f0-9]+:	0f 18 00             	prefetchnta \(%eax\)
 +[a-f0-9]+:	0f 18 08             	prefetcht0 \(%eax\)
 +[a-f0-9]+:	0f 18 10             	prefetcht1 \(%eax\)
 +[a-f0-9]+:	0f 18 18             	prefetcht2 \(%eax\)
 +[a-f0-9]+:	0f 18 20             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 18 28             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 18 30             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 18 38             	nopl   \(%eax\)
 +[a-f0-9]+:	66 0f 18 c0          	nop    %ax
 +[a-f0-9]+:	66 0f 18 c1          	nop    %cx
 +[a-f0-9]+:	66 0f 18 c2          	nop    %dx
 +[a-f0-9]+:	66 0f 18 c3          	nop    %bx
 +[a-f0-9]+:	66 0f 18 c4          	nop    %sp
 +[a-f0-9]+:	66 0f 18 c5          	nop    %bp
 +[a-f0-9]+:	66 0f 18 c6          	nop    %si
 +[a-f0-9]+:	66 0f 18 c7          	nop    %di
 +[a-f0-9]+:	66 0f 18 c8          	nop    %ax
 +[a-f0-9]+:	66 0f 18 c9          	nop    %cx
 +[a-f0-9]+:	66 0f 18 ca          	nop    %dx
 +[a-f0-9]+:	66 0f 18 cb          	nop    %bx
 +[a-f0-9]+:	66 0f 18 cc          	nop    %sp
 +[a-f0-9]+:	66 0f 18 cd          	nop    %bp
 +[a-f0-9]+:	66 0f 18 ce          	nop    %si
 +[a-f0-9]+:	66 0f 18 cf          	nop    %di
 +[a-f0-9]+:	66 0f 18 d0          	nop    %ax
 +[a-f0-9]+:	66 0f 18 d1          	nop    %cx
 +[a-f0-9]+:	66 0f 18 d2          	nop    %dx
 +[a-f0-9]+:	66 0f 18 d3          	nop    %bx
 +[a-f0-9]+:	66 0f 18 d4          	nop    %sp
 +[a-f0-9]+:	66 0f 18 d5          	nop    %bp
 +[a-f0-9]+:	66 0f 18 d6          	nop    %si
 +[a-f0-9]+:	66 0f 18 d7          	nop    %di
 +[a-f0-9]+:	66 0f 18 d8          	nop    %ax
 +[a-f0-9]+:	66 0f 18 d9          	nop    %cx
 +[a-f0-9]+:	66 0f 18 da          	nop    %dx
 +[a-f0-9]+:	66 0f 18 db          	nop    %bx
 +[a-f0-9]+:	66 0f 18 dc          	nop    %sp
 +[a-f0-9]+:	66 0f 18 dd          	nop    %bp
 +[a-f0-9]+:	66 0f 18 de          	nop    %si
 +[a-f0-9]+:	66 0f 18 df          	nop    %di
 +[a-f0-9]+:	66 0f 18 e0          	nop    %ax
 +[a-f0-9]+:	66 0f 18 e1          	nop    %cx
 +[a-f0-9]+:	66 0f 18 e2          	nop    %dx
 +[a-f0-9]+:	66 0f 18 e3          	nop    %bx
 +[a-f0-9]+:	66 0f 18 e4          	nop    %sp
 +[a-f0-9]+:	66 0f 18 e5          	nop    %bp
 +[a-f0-9]+:	66 0f 18 e6          	nop    %si
 +[a-f0-9]+:	66 0f 18 e7          	nop    %di
 +[a-f0-9]+:	66 0f 18 e8          	nop    %ax
 +[a-f0-9]+:	66 0f 18 e9          	nop    %cx
 +[a-f0-9]+:	66 0f 18 ea          	nop    %dx
 +[a-f0-9]+:	66 0f 18 eb          	nop    %bx
 +[a-f0-9]+:	66 0f 18 ec          	nop    %sp
 +[a-f0-9]+:	66 0f 18 ed          	nop    %bp
 +[a-f0-9]+:	66 0f 18 ee          	nop    %si
 +[a-f0-9]+:	66 0f 18 ef          	nop    %di
 +[a-f0-9]+:	66 0f 18 f0          	nop    %ax
 +[a-f0-9]+:	66 0f 18 f1          	nop    %cx
 +[a-f0-9]+:	66 0f 18 f2          	nop    %dx
 +[a-f0-9]+:	66 0f 18 f3          	nop    %bx
 +[a-f0-9]+:	66 0f 18 f4          	nop    %sp
 +[a-f0-9]+:	66 0f 18 f5          	nop    %bp
 +[a-f0-9]+:	66 0f 18 f6          	nop    %si
 +[a-f0-9]+:	66 0f 18 f7          	nop    %di
 +[a-f0-9]+:	66 0f 18 f8          	nop    %ax
 +[a-f0-9]+:	66 0f 18 f9          	nop    %cx
 +[a-f0-9]+:	66 0f 18 fa          	nop    %dx
 +[a-f0-9]+:	66 0f 18 fb          	nop    %bx
 +[a-f0-9]+:	66 0f 18 fc          	nop    %sp
 +[a-f0-9]+:	66 0f 18 fd          	nop    %bp
 +[a-f0-9]+:	66 0f 18 fe          	nop    %si
 +[a-f0-9]+:	66 0f 18 ff          	nop    %di
 +[a-f0-9]+:	66 0f 18 00          	data16 prefetchnta \(%eax\)
 +[a-f0-9]+:	66 0f 18 08          	data16 prefetcht0 \(%eax\)
 +[a-f0-9]+:	66 0f 18 10          	data16 prefetcht1 \(%eax\)
 +[a-f0-9]+:	66 0f 18 18          	data16 prefetcht2 \(%eax\)
 +[a-f0-9]+:	66 0f 18 20          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 18 28          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 18 30          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 18 38          	nopw   \(%eax\)
 +[a-f0-9]+:	f3 0f 18 c0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 18 c1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 18 c2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 18 c3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 18 c4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 18 c5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 18 c6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 18 c7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 18 c8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 18 c9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 18 ca          	repz nop %edx
 +[a-f0-9]+:	f3 0f 18 cb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 18 cc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 18 cd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 18 ce          	repz nop %esi
 +[a-f0-9]+:	f3 0f 18 cf          	repz nop %edi
 +[a-f0-9]+:	f3 0f 18 d0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 18 d1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 18 d2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 18 d3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 18 d4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 18 d5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 18 d6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 18 d7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 18 d8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 18 d9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 18 da          	repz nop %edx
 +[a-f0-9]+:	f3 0f 18 db          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 18 dc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 18 dd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 18 de          	repz nop %esi
 +[a-f0-9]+:	f3 0f 18 df          	repz nop %edi
 +[a-f0-9]+:	f3 0f 18 e0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 18 e1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 18 e2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 18 e3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 18 e4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 18 e5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 18 e6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 18 e7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 18 e8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 18 e9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 18 ea          	repz nop %edx
 +[a-f0-9]+:	f3 0f 18 eb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 18 ec          	repz nop %esp
 +[a-f0-9]+:	f3 0f 18 ed          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 18 ee          	repz nop %esi
 +[a-f0-9]+:	f3 0f 18 ef          	repz nop %edi
 +[a-f0-9]+:	f3 0f 18 f0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 18 f1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 18 f2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 18 f3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 18 f4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 18 f5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 18 f6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 18 f7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 18 f8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 18 f9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 18 fa          	repz nop %edx
 +[a-f0-9]+:	f3 0f 18 fb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 18 fc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 18 fd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 18 fe          	repz nop %esi
 +[a-f0-9]+:	f3 0f 18 ff          	repz nop %edi
 +[a-f0-9]+:	f3 0f 18 00          	repz prefetchnta \(%eax\)
 +[a-f0-9]+:	f3 0f 18 08          	repz prefetcht0 \(%eax\)
 +[a-f0-9]+:	f3 0f 18 10          	repz prefetcht1 \(%eax\)
 +[a-f0-9]+:	f3 0f 18 18          	repz prefetcht2 \(%eax\)
 +[a-f0-9]+:	f3 0f 18 20          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 18 28          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 18 30          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 18 38          	repz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 18 c0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 18 c1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 18 c2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 18 c3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 18 c4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 18 c5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 18 c6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 18 c7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 18 c8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 18 c9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 18 ca          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 18 cb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 18 cc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 18 cd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 18 ce          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 18 cf          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 18 d0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 18 d1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 18 d2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 18 d3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 18 d4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 18 d5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 18 d6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 18 d7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 18 d8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 18 d9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 18 da          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 18 db          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 18 dc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 18 dd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 18 de          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 18 df          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 18 e0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 18 e1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 18 e2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 18 e3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 18 e4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 18 e5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 18 e6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 18 e7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 18 e8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 18 e9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 18 ea          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 18 eb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 18 ec          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 18 ed          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 18 ee          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 18 ef          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 18 f0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 18 f1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 18 f2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 18 f3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 18 f4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 18 f5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 18 f6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 18 f7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 18 f8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 18 f9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 18 fa          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 18 fb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 18 fc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 18 fd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 18 fe          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 18 ff          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 18 00          	repnz prefetchnta \(%eax\)
 +[a-f0-9]+:	f2 0f 18 08          	repnz prefetcht0 \(%eax\)
 +[a-f0-9]+:	f2 0f 18 10          	repnz prefetcht1 \(%eax\)
 +[a-f0-9]+:	f2 0f 18 18          	repnz prefetcht2 \(%eax\)
 +[a-f0-9]+:	f2 0f 18 20          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 18 28          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 18 30          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 18 38          	repnz nopl \(%eax\)

0[a-f0-9]+ <_0f19>:
 +[a-f0-9]+:	0f 19 c0             	nop    %eax
 +[a-f0-9]+:	0f 19 c1             	nop    %ecx
 +[a-f0-9]+:	0f 19 c2             	nop    %edx
 +[a-f0-9]+:	0f 19 c3             	nop    %ebx
 +[a-f0-9]+:	0f 19 c4             	nop    %esp
 +[a-f0-9]+:	0f 19 c5             	nop    %ebp
 +[a-f0-9]+:	0f 19 c6             	nop    %esi
 +[a-f0-9]+:	0f 19 c7             	nop    %edi
 +[a-f0-9]+:	0f 19 c8             	nop    %eax
 +[a-f0-9]+:	0f 19 c9             	nop    %ecx
 +[a-f0-9]+:	0f 19 ca             	nop    %edx
 +[a-f0-9]+:	0f 19 cb             	nop    %ebx
 +[a-f0-9]+:	0f 19 cc             	nop    %esp
 +[a-f0-9]+:	0f 19 cd             	nop    %ebp
 +[a-f0-9]+:	0f 19 ce             	nop    %esi
 +[a-f0-9]+:	0f 19 cf             	nop    %edi
 +[a-f0-9]+:	0f 19 d0             	nop    %eax
 +[a-f0-9]+:	0f 19 d1             	nop    %ecx
 +[a-f0-9]+:	0f 19 d2             	nop    %edx
 +[a-f0-9]+:	0f 19 d3             	nop    %ebx
 +[a-f0-9]+:	0f 19 d4             	nop    %esp
 +[a-f0-9]+:	0f 19 d5             	nop    %ebp
 +[a-f0-9]+:	0f 19 d6             	nop    %esi
 +[a-f0-9]+:	0f 19 d7             	nop    %edi
 +[a-f0-9]+:	0f 19 d8             	nop    %eax
 +[a-f0-9]+:	0f 19 d9             	nop    %ecx
 +[a-f0-9]+:	0f 19 da             	nop    %edx
 +[a-f0-9]+:	0f 19 db             	nop    %ebx
 +[a-f0-9]+:	0f 19 dc             	nop    %esp
 +[a-f0-9]+:	0f 19 dd             	nop    %ebp
 +[a-f0-9]+:	0f 19 de             	nop    %esi
 +[a-f0-9]+:	0f 19 df             	nop    %edi
 +[a-f0-9]+:	0f 19 e0             	nop    %eax
 +[a-f0-9]+:	0f 19 e1             	nop    %ecx
 +[a-f0-9]+:	0f 19 e2             	nop    %edx
 +[a-f0-9]+:	0f 19 e3             	nop    %ebx
 +[a-f0-9]+:	0f 19 e4             	nop    %esp
 +[a-f0-9]+:	0f 19 e5             	nop    %ebp
 +[a-f0-9]+:	0f 19 e6             	nop    %esi
 +[a-f0-9]+:	0f 19 e7             	nop    %edi
 +[a-f0-9]+:	0f 19 e8             	nop    %eax
 +[a-f0-9]+:	0f 19 e9             	nop    %ecx
 +[a-f0-9]+:	0f 19 ea             	nop    %edx
 +[a-f0-9]+:	0f 19 eb             	nop    %ebx
 +[a-f0-9]+:	0f 19 ec             	nop    %esp
 +[a-f0-9]+:	0f 19 ed             	nop    %ebp
 +[a-f0-9]+:	0f 19 ee             	nop    %esi
 +[a-f0-9]+:	0f 19 ef             	nop    %edi
 +[a-f0-9]+:	0f 19 f0             	nop    %eax
 +[a-f0-9]+:	0f 19 f1             	nop    %ecx
 +[a-f0-9]+:	0f 19 f2             	nop    %edx
 +[a-f0-9]+:	0f 19 f3             	nop    %ebx
 +[a-f0-9]+:	0f 19 f4             	nop    %esp
 +[a-f0-9]+:	0f 19 f5             	nop    %ebp
 +[a-f0-9]+:	0f 19 f6             	nop    %esi
 +[a-f0-9]+:	0f 19 f7             	nop    %edi
 +[a-f0-9]+:	0f 19 f8             	nop    %eax
 +[a-f0-9]+:	0f 19 f9             	nop    %ecx
 +[a-f0-9]+:	0f 19 fa             	nop    %edx
 +[a-f0-9]+:	0f 19 fb             	nop    %ebx
 +[a-f0-9]+:	0f 19 fc             	nop    %esp
 +[a-f0-9]+:	0f 19 fd             	nop    %ebp
 +[a-f0-9]+:	0f 19 fe             	nop    %esi
 +[a-f0-9]+:	0f 19 ff             	nop    %edi
 +[a-f0-9]+:	0f 19 00             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 19 08             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 19 10             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 19 18             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 19 20             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 19 28             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 19 30             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 19 38             	nopl   \(%eax\)
 +[a-f0-9]+:	66 0f 19 c0          	nop    %ax
 +[a-f0-9]+:	66 0f 19 c1          	nop    %cx
 +[a-f0-9]+:	66 0f 19 c2          	nop    %dx
 +[a-f0-9]+:	66 0f 19 c3          	nop    %bx
 +[a-f0-9]+:	66 0f 19 c4          	nop    %sp
 +[a-f0-9]+:	66 0f 19 c5          	nop    %bp
 +[a-f0-9]+:	66 0f 19 c6          	nop    %si
 +[a-f0-9]+:	66 0f 19 c7          	nop    %di
 +[a-f0-9]+:	66 0f 19 c8          	nop    %ax
 +[a-f0-9]+:	66 0f 19 c9          	nop    %cx
 +[a-f0-9]+:	66 0f 19 ca          	nop    %dx
 +[a-f0-9]+:	66 0f 19 cb          	nop    %bx
 +[a-f0-9]+:	66 0f 19 cc          	nop    %sp
 +[a-f0-9]+:	66 0f 19 cd          	nop    %bp
 +[a-f0-9]+:	66 0f 19 ce          	nop    %si
 +[a-f0-9]+:	66 0f 19 cf          	nop    %di
 +[a-f0-9]+:	66 0f 19 d0          	nop    %ax
 +[a-f0-9]+:	66 0f 19 d1          	nop    %cx
 +[a-f0-9]+:	66 0f 19 d2          	nop    %dx
 +[a-f0-9]+:	66 0f 19 d3          	nop    %bx
 +[a-f0-9]+:	66 0f 19 d4          	nop    %sp
 +[a-f0-9]+:	66 0f 19 d5          	nop    %bp
 +[a-f0-9]+:	66 0f 19 d6          	nop    %si
 +[a-f0-9]+:	66 0f 19 d7          	nop    %di
 +[a-f0-9]+:	66 0f 19 d8          	nop    %ax
 +[a-f0-9]+:	66 0f 19 d9          	nop    %cx
 +[a-f0-9]+:	66 0f 19 da          	nop    %dx
 +[a-f0-9]+:	66 0f 19 db          	nop    %bx
 +[a-f0-9]+:	66 0f 19 dc          	nop    %sp
 +[a-f0-9]+:	66 0f 19 dd          	nop    %bp
 +[a-f0-9]+:	66 0f 19 de          	nop    %si
 +[a-f0-9]+:	66 0f 19 df          	nop    %di
 +[a-f0-9]+:	66 0f 19 e0          	nop    %ax
 +[a-f0-9]+:	66 0f 19 e1          	nop    %cx
 +[a-f0-9]+:	66 0f 19 e2          	nop    %dx
 +[a-f0-9]+:	66 0f 19 e3          	nop    %bx
 +[a-f0-9]+:	66 0f 19 e4          	nop    %sp
 +[a-f0-9]+:	66 0f 19 e5          	nop    %bp
 +[a-f0-9]+:	66 0f 19 e6          	nop    %si
 +[a-f0-9]+:	66 0f 19 e7          	nop    %di
 +[a-f0-9]+:	66 0f 19 e8          	nop    %ax
 +[a-f0-9]+:	66 0f 19 e9          	nop    %cx
 +[a-f0-9]+:	66 0f 19 ea          	nop    %dx
 +[a-f0-9]+:	66 0f 19 eb          	nop    %bx
 +[a-f0-9]+:	66 0f 19 ec          	nop    %sp
 +[a-f0-9]+:	66 0f 19 ed          	nop    %bp
 +[a-f0-9]+:	66 0f 19 ee          	nop    %si
 +[a-f0-9]+:	66 0f 19 ef          	nop    %di
 +[a-f0-9]+:	66 0f 19 f0          	nop    %ax
 +[a-f0-9]+:	66 0f 19 f1          	nop    %cx
 +[a-f0-9]+:	66 0f 19 f2          	nop    %dx
 +[a-f0-9]+:	66 0f 19 f3          	nop    %bx
 +[a-f0-9]+:	66 0f 19 f4          	nop    %sp
 +[a-f0-9]+:	66 0f 19 f5          	nop    %bp
 +[a-f0-9]+:	66 0f 19 f6          	nop    %si
 +[a-f0-9]+:	66 0f 19 f7          	nop    %di
 +[a-f0-9]+:	66 0f 19 f8          	nop    %ax
 +[a-f0-9]+:	66 0f 19 f9          	nop    %cx
 +[a-f0-9]+:	66 0f 19 fa          	nop    %dx
 +[a-f0-9]+:	66 0f 19 fb          	nop    %bx
 +[a-f0-9]+:	66 0f 19 fc          	nop    %sp
 +[a-f0-9]+:	66 0f 19 fd          	nop    %bp
 +[a-f0-9]+:	66 0f 19 fe          	nop    %si
 +[a-f0-9]+:	66 0f 19 ff          	nop    %di
 +[a-f0-9]+:	66 0f 19 00          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 19 08          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 19 10          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 19 18          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 19 20          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 19 28          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 19 30          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 19 38          	nopw   \(%eax\)
 +[a-f0-9]+:	f3 0f 19 c0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 19 c1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 19 c2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 19 c3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 19 c4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 19 c5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 19 c6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 19 c7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 19 c8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 19 c9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 19 ca          	repz nop %edx
 +[a-f0-9]+:	f3 0f 19 cb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 19 cc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 19 cd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 19 ce          	repz nop %esi
 +[a-f0-9]+:	f3 0f 19 cf          	repz nop %edi
 +[a-f0-9]+:	f3 0f 19 d0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 19 d1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 19 d2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 19 d3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 19 d4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 19 d5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 19 d6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 19 d7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 19 d8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 19 d9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 19 da          	repz nop %edx
 +[a-f0-9]+:	f3 0f 19 db          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 19 dc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 19 dd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 19 de          	repz nop %esi
 +[a-f0-9]+:	f3 0f 19 df          	repz nop %edi
 +[a-f0-9]+:	f3 0f 19 e0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 19 e1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 19 e2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 19 e3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 19 e4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 19 e5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 19 e6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 19 e7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 19 e8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 19 e9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 19 ea          	repz nop %edx
 +[a-f0-9]+:	f3 0f 19 eb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 19 ec          	repz nop %esp
 +[a-f0-9]+:	f3 0f 19 ed          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 19 ee          	repz nop %esi
 +[a-f0-9]+:	f3 0f 19 ef          	repz nop %edi
 +[a-f0-9]+:	f3 0f 19 f0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 19 f1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 19 f2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 19 f3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 19 f4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 19 f5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 19 f6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 19 f7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 19 f8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 19 f9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 19 fa          	repz nop %edx
 +[a-f0-9]+:	f3 0f 19 fb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 19 fc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 19 fd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 19 fe          	repz nop %esi
 +[a-f0-9]+:	f3 0f 19 ff          	repz nop %edi
 +[a-f0-9]+:	f3 0f 19 00          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 19 08          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 19 10          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 19 18          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 19 20          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 19 28          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 19 30          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 19 38          	repz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 19 c0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 19 c1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 19 c2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 19 c3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 19 c4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 19 c5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 19 c6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 19 c7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 19 c8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 19 c9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 19 ca          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 19 cb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 19 cc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 19 cd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 19 ce          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 19 cf          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 19 d0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 19 d1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 19 d2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 19 d3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 19 d4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 19 d5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 19 d6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 19 d7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 19 d8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 19 d9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 19 da          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 19 db          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 19 dc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 19 dd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 19 de          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 19 df          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 19 e0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 19 e1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 19 e2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 19 e3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 19 e4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 19 e5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 19 e6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 19 e7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 19 e8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 19 e9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 19 ea          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 19 eb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 19 ec          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 19 ed          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 19 ee          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 19 ef          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 19 f0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 19 f1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 19 f2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 19 f3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 19 f4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 19 f5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 19 f6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 19 f7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 19 f8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 19 f9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 19 fa          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 19 fb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 19 fc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 19 fd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 19 fe          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 19 ff          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 19 00          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 19 08          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 19 10          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 19 18          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 19 20          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 19 28          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 19 30          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 19 38          	repnz nopl \(%eax\)

0[a-f0-9]+ <_0f1a>:
 +[a-f0-9]+:	0f 1a c0             	nop    %eax
 +[a-f0-9]+:	0f 1a c1             	nop    %ecx
 +[a-f0-9]+:	0f 1a c2             	nop    %edx
 +[a-f0-9]+:	0f 1a c3             	nop    %ebx
 +[a-f0-9]+:	0f 1a c4             	nop    %esp
 +[a-f0-9]+:	0f 1a c5             	nop    %ebp
 +[a-f0-9]+:	0f 1a c6             	nop    %esi
 +[a-f0-9]+:	0f 1a c7             	nop    %edi
 +[a-f0-9]+:	0f 1a c8             	nop    %eax
 +[a-f0-9]+:	0f 1a c9             	nop    %ecx
 +[a-f0-9]+:	0f 1a ca             	nop    %edx
 +[a-f0-9]+:	0f 1a cb             	nop    %ebx
 +[a-f0-9]+:	0f 1a cc             	nop    %esp
 +[a-f0-9]+:	0f 1a cd             	nop    %ebp
 +[a-f0-9]+:	0f 1a ce             	nop    %esi
 +[a-f0-9]+:	0f 1a cf             	nop    %edi
 +[a-f0-9]+:	0f 1a d0             	nop    %eax
 +[a-f0-9]+:	0f 1a d1             	nop    %ecx
 +[a-f0-9]+:	0f 1a d2             	nop    %edx
 +[a-f0-9]+:	0f 1a d3             	nop    %ebx
 +[a-f0-9]+:	0f 1a d4             	nop    %esp
 +[a-f0-9]+:	0f 1a d5             	nop    %ebp
 +[a-f0-9]+:	0f 1a d6             	nop    %esi
 +[a-f0-9]+:	0f 1a d7             	nop    %edi
 +[a-f0-9]+:	0f 1a d8             	nop    %eax
 +[a-f0-9]+:	0f 1a d9             	nop    %ecx
 +[a-f0-9]+:	0f 1a da             	nop    %edx
 +[a-f0-9]+:	0f 1a db             	nop    %ebx
 +[a-f0-9]+:	0f 1a dc             	nop    %esp
 +[a-f0-9]+:	0f 1a dd             	nop    %ebp
 +[a-f0-9]+:	0f 1a de             	nop    %esi
 +[a-f0-9]+:	0f 1a df             	nop    %edi
 +[a-f0-9]+:	0f 1a e0             	nop    %eax
 +[a-f0-9]+:	0f 1a e1             	nop    %ecx
 +[a-f0-9]+:	0f 1a e2             	nop    %edx
 +[a-f0-9]+:	0f 1a e3             	nop    %ebx
 +[a-f0-9]+:	0f 1a e4             	nop    %esp
 +[a-f0-9]+:	0f 1a e5             	nop    %ebp
 +[a-f0-9]+:	0f 1a e6             	nop    %esi
 +[a-f0-9]+:	0f 1a e7             	nop    %edi
 +[a-f0-9]+:	0f 1a e8             	nop    %eax
 +[a-f0-9]+:	0f 1a e9             	nop    %ecx
 +[a-f0-9]+:	0f 1a ea             	nop    %edx
 +[a-f0-9]+:	0f 1a eb             	nop    %ebx
 +[a-f0-9]+:	0f 1a ec             	nop    %esp
 +[a-f0-9]+:	0f 1a ed             	nop    %ebp
 +[a-f0-9]+:	0f 1a ee             	nop    %esi
 +[a-f0-9]+:	0f 1a ef             	nop    %edi
 +[a-f0-9]+:	0f 1a f0             	nop    %eax
 +[a-f0-9]+:	0f 1a f1             	nop    %ecx
 +[a-f0-9]+:	0f 1a f2             	nop    %edx
 +[a-f0-9]+:	0f 1a f3             	nop    %ebx
 +[a-f0-9]+:	0f 1a f4             	nop    %esp
 +[a-f0-9]+:	0f 1a f5             	nop    %ebp
 +[a-f0-9]+:	0f 1a f6             	nop    %esi
 +[a-f0-9]+:	0f 1a f7             	nop    %edi
 +[a-f0-9]+:	0f 1a f8             	nop    %eax
 +[a-f0-9]+:	0f 1a f9             	nop    %ecx
 +[a-f0-9]+:	0f 1a fa             	nop    %edx
 +[a-f0-9]+:	0f 1a fb             	nop    %ebx
 +[a-f0-9]+:	0f 1a fc             	nop    %esp
 +[a-f0-9]+:	0f 1a fd             	nop    %ebp
 +[a-f0-9]+:	0f 1a fe             	nop    %esi
 +[a-f0-9]+:	0f 1a ff             	nop    %edi
 +[a-f0-9]+:	0f 1a 00             	bndldx \(%eax\),%bnd0
 +[a-f0-9]+:	0f 1a 08             	bndldx \(%eax\),%bnd1
 +[a-f0-9]+:	0f 1a 10             	bndldx \(%eax\),%bnd2
 +[a-f0-9]+:	0f 1a 18             	bndldx \(%eax\),%bnd3
 +[a-f0-9]+:	0f 1a 20             	bndldx \(%eax\),\(bad\)
 +[a-f0-9]+:	0f 1a 28             	bndldx \(%eax\),\(bad\)
 +[a-f0-9]+:	0f 1a 30             	bndldx \(%eax\),\(bad\)
 +[a-f0-9]+:	0f 1a 38             	bndldx \(%eax\),\(bad\)
 +[a-f0-9]+:	66 0f 1a c0          	bndmov %bnd0,%bnd0
 +[a-f0-9]+:	66 0f 1a c1          	bndmov %bnd1,%bnd0
 +[a-f0-9]+:	66 0f 1a c2          	bndmov %bnd2,%bnd0
 +[a-f0-9]+:	66 0f 1a c3          	bndmov %bnd3,%bnd0
 +[a-f0-9]+:	66 0f 1a c4          	bndmov \(bad\),%bnd0
 +[a-f0-9]+:	66 0f 1a c5          	bndmov \(bad\),%bnd0
 +[a-f0-9]+:	66 0f 1a c6          	bndmov \(bad\),%bnd0
 +[a-f0-9]+:	66 0f 1a c7          	bndmov \(bad\),%bnd0
 +[a-f0-9]+:	66 0f 1a c8          	bndmov %bnd0,%bnd1
 +[a-f0-9]+:	66 0f 1a c9          	bndmov %bnd1,%bnd1
 +[a-f0-9]+:	66 0f 1a ca          	bndmov %bnd2,%bnd1
 +[a-f0-9]+:	66 0f 1a cb          	bndmov %bnd3,%bnd1
 +[a-f0-9]+:	66 0f 1a cc          	bndmov \(bad\),%bnd1
 +[a-f0-9]+:	66 0f 1a cd          	bndmov \(bad\),%bnd1
 +[a-f0-9]+:	66 0f 1a ce          	bndmov \(bad\),%bnd1
 +[a-f0-9]+:	66 0f 1a cf          	bndmov \(bad\),%bnd1
 +[a-f0-9]+:	66 0f 1a d0          	bndmov %bnd0,%bnd2
 +[a-f0-9]+:	66 0f 1a d1          	bndmov %bnd1,%bnd2
 +[a-f0-9]+:	66 0f 1a d2          	bndmov %bnd2,%bnd2
 +[a-f0-9]+:	66 0f 1a d3          	bndmov %bnd3,%bnd2
 +[a-f0-9]+:	66 0f 1a d4          	bndmov \(bad\),%bnd2
 +[a-f0-9]+:	66 0f 1a d5          	bndmov \(bad\),%bnd2
 +[a-f0-9]+:	66 0f 1a d6          	bndmov \(bad\),%bnd2
 +[a-f0-9]+:	66 0f 1a d7          	bndmov \(bad\),%bnd2
 +[a-f0-9]+:	66 0f 1a d8          	bndmov %bnd0,%bnd3
 +[a-f0-9]+:	66 0f 1a d9          	bndmov %bnd1,%bnd3
 +[a-f0-9]+:	66 0f 1a da          	bndmov %bnd2,%bnd3
 +[a-f0-9]+:	66 0f 1a db          	bndmov %bnd3,%bnd3
 +[a-f0-9]+:	66 0f 1a dc          	bndmov \(bad\),%bnd3
 +[a-f0-9]+:	66 0f 1a dd          	bndmov \(bad\),%bnd3
 +[a-f0-9]+:	66 0f 1a de          	bndmov \(bad\),%bnd3
 +[a-f0-9]+:	66 0f 1a df          	bndmov \(bad\),%bnd3
 +[a-f0-9]+:	66 0f 1a e0          	bndmov %bnd0,\(bad\)
 +[a-f0-9]+:	66 0f 1a e1          	bndmov %bnd1,\(bad\)
 +[a-f0-9]+:	66 0f 1a e2          	bndmov %bnd2,\(bad\)
 +[a-f0-9]+:	66 0f 1a e3          	bndmov %bnd3,\(bad\)
 +[a-f0-9]+:	66 0f 1a e4          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1a e5          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1a e6          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1a e7          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1a e8          	bndmov %bnd0,\(bad\)
 +[a-f0-9]+:	66 0f 1a e9          	bndmov %bnd1,\(bad\)
 +[a-f0-9]+:	66 0f 1a ea          	bndmov %bnd2,\(bad\)
 +[a-f0-9]+:	66 0f 1a eb          	bndmov %bnd3,\(bad\)
 +[a-f0-9]+:	66 0f 1a ec          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1a ed          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1a ee          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1a ef          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1a f0          	bndmov %bnd0,\(bad\)
 +[a-f0-9]+:	66 0f 1a f1          	bndmov %bnd1,\(bad\)
 +[a-f0-9]+:	66 0f 1a f2          	bndmov %bnd2,\(bad\)
 +[a-f0-9]+:	66 0f 1a f3          	bndmov %bnd3,\(bad\)
 +[a-f0-9]+:	66 0f 1a f4          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1a f5          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1a f6          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1a f7          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1a f8          	bndmov %bnd0,\(bad\)
 +[a-f0-9]+:	66 0f 1a f9          	bndmov %bnd1,\(bad\)
 +[a-f0-9]+:	66 0f 1a fa          	bndmov %bnd2,\(bad\)
 +[a-f0-9]+:	66 0f 1a fb          	bndmov %bnd3,\(bad\)
 +[a-f0-9]+:	66 0f 1a fc          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1a fd          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1a fe          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1a ff          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1a 00          	bndmov \(%eax\),%bnd0
 +[a-f0-9]+:	66 0f 1a 08          	bndmov \(%eax\),%bnd1
 +[a-f0-9]+:	66 0f 1a 10          	bndmov \(%eax\),%bnd2
 +[a-f0-9]+:	66 0f 1a 18          	bndmov \(%eax\),%bnd3
 +[a-f0-9]+:	66 0f 1a 20          	bndmov \(%eax\),\(bad\)
 +[a-f0-9]+:	66 0f 1a 28          	bndmov \(%eax\),\(bad\)
 +[a-f0-9]+:	66 0f 1a 30          	bndmov \(%eax\),\(bad\)
 +[a-f0-9]+:	66 0f 1a 38          	bndmov \(%eax\),\(bad\)
 +[a-f0-9]+:	f3 0f 1a c0          	bndcl  %eax,%bnd0
 +[a-f0-9]+:	f3 0f 1a c1          	bndcl  %ecx,%bnd0
 +[a-f0-9]+:	f3 0f 1a c2          	bndcl  %edx,%bnd0
 +[a-f0-9]+:	f3 0f 1a c3          	bndcl  %ebx,%bnd0
 +[a-f0-9]+:	f3 0f 1a c4          	bndcl  %esp,%bnd0
 +[a-f0-9]+:	f3 0f 1a c5          	bndcl  %ebp,%bnd0
 +[a-f0-9]+:	f3 0f 1a c6          	bndcl  %esi,%bnd0
 +[a-f0-9]+:	f3 0f 1a c7          	bndcl  %edi,%bnd0
 +[a-f0-9]+:	f3 0f 1a c8          	bndcl  %eax,%bnd1
 +[a-f0-9]+:	f3 0f 1a c9          	bndcl  %ecx,%bnd1
 +[a-f0-9]+:	f3 0f 1a ca          	bndcl  %edx,%bnd1
 +[a-f0-9]+:	f3 0f 1a cb          	bndcl  %ebx,%bnd1
 +[a-f0-9]+:	f3 0f 1a cc          	bndcl  %esp,%bnd1
 +[a-f0-9]+:	f3 0f 1a cd          	bndcl  %ebp,%bnd1
 +[a-f0-9]+:	f3 0f 1a ce          	bndcl  %esi,%bnd1
 +[a-f0-9]+:	f3 0f 1a cf          	bndcl  %edi,%bnd1
 +[a-f0-9]+:	f3 0f 1a d0          	bndcl  %eax,%bnd2
 +[a-f0-9]+:	f3 0f 1a d1          	bndcl  %ecx,%bnd2
 +[a-f0-9]+:	f3 0f 1a d2          	bndcl  %edx,%bnd2
 +[a-f0-9]+:	f3 0f 1a d3          	bndcl  %ebx,%bnd2
 +[a-f0-9]+:	f3 0f 1a d4          	bndcl  %esp,%bnd2
 +[a-f0-9]+:	f3 0f 1a d5          	bndcl  %ebp,%bnd2
 +[a-f0-9]+:	f3 0f 1a d6          	bndcl  %esi,%bnd2
 +[a-f0-9]+:	f3 0f 1a d7          	bndcl  %edi,%bnd2
 +[a-f0-9]+:	f3 0f 1a d8          	bndcl  %eax,%bnd3
 +[a-f0-9]+:	f3 0f 1a d9          	bndcl  %ecx,%bnd3
 +[a-f0-9]+:	f3 0f 1a da          	bndcl  %edx,%bnd3
 +[a-f0-9]+:	f3 0f 1a db          	bndcl  %ebx,%bnd3
 +[a-f0-9]+:	f3 0f 1a dc          	bndcl  %esp,%bnd3
 +[a-f0-9]+:	f3 0f 1a dd          	bndcl  %ebp,%bnd3
 +[a-f0-9]+:	f3 0f 1a de          	bndcl  %esi,%bnd3
 +[a-f0-9]+:	f3 0f 1a df          	bndcl  %edi,%bnd3
 +[a-f0-9]+:	f3 0f 1a e0          	bndcl  %eax,\(bad\)
 +[a-f0-9]+:	f3 0f 1a e1          	bndcl  %ecx,\(bad\)
 +[a-f0-9]+:	f3 0f 1a e2          	bndcl  %edx,\(bad\)
 +[a-f0-9]+:	f3 0f 1a e3          	bndcl  %ebx,\(bad\)
 +[a-f0-9]+:	f3 0f 1a e4          	bndcl  %esp,\(bad\)
 +[a-f0-9]+:	f3 0f 1a e5          	bndcl  %ebp,\(bad\)
 +[a-f0-9]+:	f3 0f 1a e6          	bndcl  %esi,\(bad\)
 +[a-f0-9]+:	f3 0f 1a e7          	bndcl  %edi,\(bad\)
 +[a-f0-9]+:	f3 0f 1a e8          	bndcl  %eax,\(bad\)
 +[a-f0-9]+:	f3 0f 1a e9          	bndcl  %ecx,\(bad\)
 +[a-f0-9]+:	f3 0f 1a ea          	bndcl  %edx,\(bad\)
 +[a-f0-9]+:	f3 0f 1a eb          	bndcl  %ebx,\(bad\)
 +[a-f0-9]+:	f3 0f 1a ec          	bndcl  %esp,\(bad\)
 +[a-f0-9]+:	f3 0f 1a ed          	bndcl  %ebp,\(bad\)
 +[a-f0-9]+:	f3 0f 1a ee          	bndcl  %esi,\(bad\)
 +[a-f0-9]+:	f3 0f 1a ef          	bndcl  %edi,\(bad\)
 +[a-f0-9]+:	f3 0f 1a f0          	bndcl  %eax,\(bad\)
 +[a-f0-9]+:	f3 0f 1a f1          	bndcl  %ecx,\(bad\)
 +[a-f0-9]+:	f3 0f 1a f2          	bndcl  %edx,\(bad\)
 +[a-f0-9]+:	f3 0f 1a f3          	bndcl  %ebx,\(bad\)
 +[a-f0-9]+:	f3 0f 1a f4          	bndcl  %esp,\(bad\)
 +[a-f0-9]+:	f3 0f 1a f5          	bndcl  %ebp,\(bad\)
 +[a-f0-9]+:	f3 0f 1a f6          	bndcl  %esi,\(bad\)
 +[a-f0-9]+:	f3 0f 1a f7          	bndcl  %edi,\(bad\)
 +[a-f0-9]+:	f3 0f 1a f8          	bndcl  %eax,\(bad\)
 +[a-f0-9]+:	f3 0f 1a f9          	bndcl  %ecx,\(bad\)
 +[a-f0-9]+:	f3 0f 1a fa          	bndcl  %edx,\(bad\)
 +[a-f0-9]+:	f3 0f 1a fb          	bndcl  %ebx,\(bad\)
 +[a-f0-9]+:	f3 0f 1a fc          	bndcl  %esp,\(bad\)
 +[a-f0-9]+:	f3 0f 1a fd          	bndcl  %ebp,\(bad\)
 +[a-f0-9]+:	f3 0f 1a fe          	bndcl  %esi,\(bad\)
 +[a-f0-9]+:	f3 0f 1a ff          	bndcl  %edi,\(bad\)
 +[a-f0-9]+:	f3 0f 1a 00          	bndcl  \(%eax\),%bnd0
 +[a-f0-9]+:	f3 0f 1a 08          	bndcl  \(%eax\),%bnd1
 +[a-f0-9]+:	f3 0f 1a 10          	bndcl  \(%eax\),%bnd2
 +[a-f0-9]+:	f3 0f 1a 18          	bndcl  \(%eax\),%bnd3
 +[a-f0-9]+:	f3 0f 1a 20          	bndcl  \(%eax\),\(bad\)
 +[a-f0-9]+:	f3 0f 1a 28          	bndcl  \(%eax\),\(bad\)
 +[a-f0-9]+:	f3 0f 1a 30          	bndcl  \(%eax\),\(bad\)
 +[a-f0-9]+:	f3 0f 1a 38          	bndcl  \(%eax\),\(bad\)
 +[a-f0-9]+:	f2 0f 1a c0          	bndcu  %eax,%bnd0
 +[a-f0-9]+:	f2 0f 1a c1          	bndcu  %ecx,%bnd0
 +[a-f0-9]+:	f2 0f 1a c2          	bndcu  %edx,%bnd0
 +[a-f0-9]+:	f2 0f 1a c3          	bndcu  %ebx,%bnd0
 +[a-f0-9]+:	f2 0f 1a c4          	bndcu  %esp,%bnd0
 +[a-f0-9]+:	f2 0f 1a c5          	bndcu  %ebp,%bnd0
 +[a-f0-9]+:	f2 0f 1a c6          	bndcu  %esi,%bnd0
 +[a-f0-9]+:	f2 0f 1a c7          	bndcu  %edi,%bnd0
 +[a-f0-9]+:	f2 0f 1a c8          	bndcu  %eax,%bnd1
 +[a-f0-9]+:	f2 0f 1a c9          	bndcu  %ecx,%bnd1
 +[a-f0-9]+:	f2 0f 1a ca          	bndcu  %edx,%bnd1
 +[a-f0-9]+:	f2 0f 1a cb          	bndcu  %ebx,%bnd1
 +[a-f0-9]+:	f2 0f 1a cc          	bndcu  %esp,%bnd1
 +[a-f0-9]+:	f2 0f 1a cd          	bndcu  %ebp,%bnd1
 +[a-f0-9]+:	f2 0f 1a ce          	bndcu  %esi,%bnd1
 +[a-f0-9]+:	f2 0f 1a cf          	bndcu  %edi,%bnd1
 +[a-f0-9]+:	f2 0f 1a d0          	bndcu  %eax,%bnd2
 +[a-f0-9]+:	f2 0f 1a d1          	bndcu  %ecx,%bnd2
 +[a-f0-9]+:	f2 0f 1a d2          	bndcu  %edx,%bnd2
 +[a-f0-9]+:	f2 0f 1a d3          	bndcu  %ebx,%bnd2
 +[a-f0-9]+:	f2 0f 1a d4          	bndcu  %esp,%bnd2
 +[a-f0-9]+:	f2 0f 1a d5          	bndcu  %ebp,%bnd2
 +[a-f0-9]+:	f2 0f 1a d6          	bndcu  %esi,%bnd2
 +[a-f0-9]+:	f2 0f 1a d7          	bndcu  %edi,%bnd2
 +[a-f0-9]+:	f2 0f 1a d8          	bndcu  %eax,%bnd3
 +[a-f0-9]+:	f2 0f 1a d9          	bndcu  %ecx,%bnd3
 +[a-f0-9]+:	f2 0f 1a da          	bndcu  %edx,%bnd3
 +[a-f0-9]+:	f2 0f 1a db          	bndcu  %ebx,%bnd3
 +[a-f0-9]+:	f2 0f 1a dc          	bndcu  %esp,%bnd3
 +[a-f0-9]+:	f2 0f 1a dd          	bndcu  %ebp,%bnd3
 +[a-f0-9]+:	f2 0f 1a de          	bndcu  %esi,%bnd3
 +[a-f0-9]+:	f2 0f 1a df          	bndcu  %edi,%bnd3
 +[a-f0-9]+:	f2 0f 1a e0          	bndcu  %eax,\(bad\)
 +[a-f0-9]+:	f2 0f 1a e1          	bndcu  %ecx,\(bad\)
 +[a-f0-9]+:	f2 0f 1a e2          	bndcu  %edx,\(bad\)
 +[a-f0-9]+:	f2 0f 1a e3          	bndcu  %ebx,\(bad\)
 +[a-f0-9]+:	f2 0f 1a e4          	bndcu  %esp,\(bad\)
 +[a-f0-9]+:	f2 0f 1a e5          	bndcu  %ebp,\(bad\)
 +[a-f0-9]+:	f2 0f 1a e6          	bndcu  %esi,\(bad\)
 +[a-f0-9]+:	f2 0f 1a e7          	bndcu  %edi,\(bad\)
 +[a-f0-9]+:	f2 0f 1a e8          	bndcu  %eax,\(bad\)
 +[a-f0-9]+:	f2 0f 1a e9          	bndcu  %ecx,\(bad\)
 +[a-f0-9]+:	f2 0f 1a ea          	bndcu  %edx,\(bad\)
 +[a-f0-9]+:	f2 0f 1a eb          	bndcu  %ebx,\(bad\)
 +[a-f0-9]+:	f2 0f 1a ec          	bndcu  %esp,\(bad\)
 +[a-f0-9]+:	f2 0f 1a ed          	bndcu  %ebp,\(bad\)
 +[a-f0-9]+:	f2 0f 1a ee          	bndcu  %esi,\(bad\)
 +[a-f0-9]+:	f2 0f 1a ef          	bndcu  %edi,\(bad\)
 +[a-f0-9]+:	f2 0f 1a f0          	bndcu  %eax,\(bad\)
 +[a-f0-9]+:	f2 0f 1a f1          	bndcu  %ecx,\(bad\)
 +[a-f0-9]+:	f2 0f 1a f2          	bndcu  %edx,\(bad\)
 +[a-f0-9]+:	f2 0f 1a f3          	bndcu  %ebx,\(bad\)
 +[a-f0-9]+:	f2 0f 1a f4          	bndcu  %esp,\(bad\)
 +[a-f0-9]+:	f2 0f 1a f5          	bndcu  %ebp,\(bad\)
 +[a-f0-9]+:	f2 0f 1a f6          	bndcu  %esi,\(bad\)
 +[a-f0-9]+:	f2 0f 1a f7          	bndcu  %edi,\(bad\)
 +[a-f0-9]+:	f2 0f 1a f8          	bndcu  %eax,\(bad\)
 +[a-f0-9]+:	f2 0f 1a f9          	bndcu  %ecx,\(bad\)
 +[a-f0-9]+:	f2 0f 1a fa          	bndcu  %edx,\(bad\)
 +[a-f0-9]+:	f2 0f 1a fb          	bndcu  %ebx,\(bad\)
 +[a-f0-9]+:	f2 0f 1a fc          	bndcu  %esp,\(bad\)
 +[a-f0-9]+:	f2 0f 1a fd          	bndcu  %ebp,\(bad\)
 +[a-f0-9]+:	f2 0f 1a fe          	bndcu  %esi,\(bad\)
 +[a-f0-9]+:	f2 0f 1a ff          	bndcu  %edi,\(bad\)
 +[a-f0-9]+:	f2 0f 1a 00          	bndcu  \(%eax\),%bnd0
 +[a-f0-9]+:	f2 0f 1a 08          	bndcu  \(%eax\),%bnd1
 +[a-f0-9]+:	f2 0f 1a 10          	bndcu  \(%eax\),%bnd2
 +[a-f0-9]+:	f2 0f 1a 18          	bndcu  \(%eax\),%bnd3
 +[a-f0-9]+:	f2 0f 1a 20          	bndcu  \(%eax\),\(bad\)
 +[a-f0-9]+:	f2 0f 1a 28          	bndcu  \(%eax\),\(bad\)
 +[a-f0-9]+:	f2 0f 1a 30          	bndcu  \(%eax\),\(bad\)
 +[a-f0-9]+:	f2 0f 1a 38          	bndcu  \(%eax\),\(bad\)

0[a-f0-9]+ <_0f1b>:
 +[a-f0-9]+:	0f 1b c0             	nop    %eax
 +[a-f0-9]+:	0f 1b c1             	nop    %ecx
 +[a-f0-9]+:	0f 1b c2             	nop    %edx
 +[a-f0-9]+:	0f 1b c3             	nop    %ebx
 +[a-f0-9]+:	0f 1b c4             	nop    %esp
 +[a-f0-9]+:	0f 1b c5             	nop    %ebp
 +[a-f0-9]+:	0f 1b c6             	nop    %esi
 +[a-f0-9]+:	0f 1b c7             	nop    %edi
 +[a-f0-9]+:	0f 1b c8             	nop    %eax
 +[a-f0-9]+:	0f 1b c9             	nop    %ecx
 +[a-f0-9]+:	0f 1b ca             	nop    %edx
 +[a-f0-9]+:	0f 1b cb             	nop    %ebx
 +[a-f0-9]+:	0f 1b cc             	nop    %esp
 +[a-f0-9]+:	0f 1b cd             	nop    %ebp
 +[a-f0-9]+:	0f 1b ce             	nop    %esi
 +[a-f0-9]+:	0f 1b cf             	nop    %edi
 +[a-f0-9]+:	0f 1b d0             	nop    %eax
 +[a-f0-9]+:	0f 1b d1             	nop    %ecx
 +[a-f0-9]+:	0f 1b d2             	nop    %edx
 +[a-f0-9]+:	0f 1b d3             	nop    %ebx
 +[a-f0-9]+:	0f 1b d4             	nop    %esp
 +[a-f0-9]+:	0f 1b d5             	nop    %ebp
 +[a-f0-9]+:	0f 1b d6             	nop    %esi
 +[a-f0-9]+:	0f 1b d7             	nop    %edi
 +[a-f0-9]+:	0f 1b d8             	nop    %eax
 +[a-f0-9]+:	0f 1b d9             	nop    %ecx
 +[a-f0-9]+:	0f 1b da             	nop    %edx
 +[a-f0-9]+:	0f 1b db             	nop    %ebx
 +[a-f0-9]+:	0f 1b dc             	nop    %esp
 +[a-f0-9]+:	0f 1b dd             	nop    %ebp
 +[a-f0-9]+:	0f 1b de             	nop    %esi
 +[a-f0-9]+:	0f 1b df             	nop    %edi
 +[a-f0-9]+:	0f 1b e0             	nop    %eax
 +[a-f0-9]+:	0f 1b e1             	nop    %ecx
 +[a-f0-9]+:	0f 1b e2             	nop    %edx
 +[a-f0-9]+:	0f 1b e3             	nop    %ebx
 +[a-f0-9]+:	0f 1b e4             	nop    %esp
 +[a-f0-9]+:	0f 1b e5             	nop    %ebp
 +[a-f0-9]+:	0f 1b e6             	nop    %esi
 +[a-f0-9]+:	0f 1b e7             	nop    %edi
 +[a-f0-9]+:	0f 1b e8             	nop    %eax
 +[a-f0-9]+:	0f 1b e9             	nop    %ecx
 +[a-f0-9]+:	0f 1b ea             	nop    %edx
 +[a-f0-9]+:	0f 1b eb             	nop    %ebx
 +[a-f0-9]+:	0f 1b ec             	nop    %esp
 +[a-f0-9]+:	0f 1b ed             	nop    %ebp
 +[a-f0-9]+:	0f 1b ee             	nop    %esi
 +[a-f0-9]+:	0f 1b ef             	nop    %edi
 +[a-f0-9]+:	0f 1b f0             	nop    %eax
 +[a-f0-9]+:	0f 1b f1             	nop    %ecx
 +[a-f0-9]+:	0f 1b f2             	nop    %edx
 +[a-f0-9]+:	0f 1b f3             	nop    %ebx
 +[a-f0-9]+:	0f 1b f4             	nop    %esp
 +[a-f0-9]+:	0f 1b f5             	nop    %ebp
 +[a-f0-9]+:	0f 1b f6             	nop    %esi
 +[a-f0-9]+:	0f 1b f7             	nop    %edi
 +[a-f0-9]+:	0f 1b f8             	nop    %eax
 +[a-f0-9]+:	0f 1b f9             	nop    %ecx
 +[a-f0-9]+:	0f 1b fa             	nop    %edx
 +[a-f0-9]+:	0f 1b fb             	nop    %ebx
 +[a-f0-9]+:	0f 1b fc             	nop    %esp
 +[a-f0-9]+:	0f 1b fd             	nop    %ebp
 +[a-f0-9]+:	0f 1b fe             	nop    %esi
 +[a-f0-9]+:	0f 1b ff             	nop    %edi
 +[a-f0-9]+:	0f 1b 00             	bndstx %bnd0,\(%eax\)
 +[a-f0-9]+:	0f 1b 08             	bndstx %bnd1,\(%eax\)
 +[a-f0-9]+:	0f 1b 10             	bndstx %bnd2,\(%eax\)
 +[a-f0-9]+:	0f 1b 18             	bndstx %bnd3,\(%eax\)
 +[a-f0-9]+:	0f 1b 20             	bndstx \(bad\),\(%eax\)
 +[a-f0-9]+:	0f 1b 28             	bndstx \(bad\),\(%eax\)
 +[a-f0-9]+:	0f 1b 30             	bndstx \(bad\),\(%eax\)
 +[a-f0-9]+:	0f 1b 38             	bndstx \(bad\),\(%eax\)
 +[a-f0-9]+:	66 0f 1b c0          	bndmov %bnd0,%bnd0
 +[a-f0-9]+:	66 0f 1b c1          	bndmov %bnd0,%bnd1
 +[a-f0-9]+:	66 0f 1b c2          	bndmov %bnd0,%bnd2
 +[a-f0-9]+:	66 0f 1b c3          	bndmov %bnd0,%bnd3
 +[a-f0-9]+:	66 0f 1b c4          	bndmov %bnd0,\(bad\)
 +[a-f0-9]+:	66 0f 1b c5          	bndmov %bnd0,\(bad\)
 +[a-f0-9]+:	66 0f 1b c6          	bndmov %bnd0,\(bad\)
 +[a-f0-9]+:	66 0f 1b c7          	bndmov %bnd0,\(bad\)
 +[a-f0-9]+:	66 0f 1b c8          	bndmov %bnd1,%bnd0
 +[a-f0-9]+:	66 0f 1b c9          	bndmov %bnd1,%bnd1
 +[a-f0-9]+:	66 0f 1b ca          	bndmov %bnd1,%bnd2
 +[a-f0-9]+:	66 0f 1b cb          	bndmov %bnd1,%bnd3
 +[a-f0-9]+:	66 0f 1b cc          	bndmov %bnd1,\(bad\)
 +[a-f0-9]+:	66 0f 1b cd          	bndmov %bnd1,\(bad\)
 +[a-f0-9]+:	66 0f 1b ce          	bndmov %bnd1,\(bad\)
 +[a-f0-9]+:	66 0f 1b cf          	bndmov %bnd1,\(bad\)
 +[a-f0-9]+:	66 0f 1b d0          	bndmov %bnd2,%bnd0
 +[a-f0-9]+:	66 0f 1b d1          	bndmov %bnd2,%bnd1
 +[a-f0-9]+:	66 0f 1b d2          	bndmov %bnd2,%bnd2
 +[a-f0-9]+:	66 0f 1b d3          	bndmov %bnd2,%bnd3
 +[a-f0-9]+:	66 0f 1b d4          	bndmov %bnd2,\(bad\)
 +[a-f0-9]+:	66 0f 1b d5          	bndmov %bnd2,\(bad\)
 +[a-f0-9]+:	66 0f 1b d6          	bndmov %bnd2,\(bad\)
 +[a-f0-9]+:	66 0f 1b d7          	bndmov %bnd2,\(bad\)
 +[a-f0-9]+:	66 0f 1b d8          	bndmov %bnd3,%bnd0
 +[a-f0-9]+:	66 0f 1b d9          	bndmov %bnd3,%bnd1
 +[a-f0-9]+:	66 0f 1b da          	bndmov %bnd3,%bnd2
 +[a-f0-9]+:	66 0f 1b db          	bndmov %bnd3,%bnd3
 +[a-f0-9]+:	66 0f 1b dc          	bndmov %bnd3,\(bad\)
 +[a-f0-9]+:	66 0f 1b dd          	bndmov %bnd3,\(bad\)
 +[a-f0-9]+:	66 0f 1b de          	bndmov %bnd3,\(bad\)
 +[a-f0-9]+:	66 0f 1b df          	bndmov %bnd3,\(bad\)
 +[a-f0-9]+:	66 0f 1b e0          	bndmov \(bad\),%bnd0
 +[a-f0-9]+:	66 0f 1b e1          	bndmov \(bad\),%bnd1
 +[a-f0-9]+:	66 0f 1b e2          	bndmov \(bad\),%bnd2
 +[a-f0-9]+:	66 0f 1b e3          	bndmov \(bad\),%bnd3
 +[a-f0-9]+:	66 0f 1b e4          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1b e5          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1b e6          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1b e7          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1b e8          	bndmov \(bad\),%bnd0
 +[a-f0-9]+:	66 0f 1b e9          	bndmov \(bad\),%bnd1
 +[a-f0-9]+:	66 0f 1b ea          	bndmov \(bad\),%bnd2
 +[a-f0-9]+:	66 0f 1b eb          	bndmov \(bad\),%bnd3
 +[a-f0-9]+:	66 0f 1b ec          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1b ed          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1b ee          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1b ef          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1b f0          	bndmov \(bad\),%bnd0
 +[a-f0-9]+:	66 0f 1b f1          	bndmov \(bad\),%bnd1
 +[a-f0-9]+:	66 0f 1b f2          	bndmov \(bad\),%bnd2
 +[a-f0-9]+:	66 0f 1b f3          	bndmov \(bad\),%bnd3
 +[a-f0-9]+:	66 0f 1b f4          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1b f5          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1b f6          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1b f7          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1b f8          	bndmov \(bad\),%bnd0
 +[a-f0-9]+:	66 0f 1b f9          	bndmov \(bad\),%bnd1
 +[a-f0-9]+:	66 0f 1b fa          	bndmov \(bad\),%bnd2
 +[a-f0-9]+:	66 0f 1b fb          	bndmov \(bad\),%bnd3
 +[a-f0-9]+:	66 0f 1b fc          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1b fd          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1b fe          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1b ff          	bndmov \(bad\),\(bad\)
 +[a-f0-9]+:	66 0f 1b 00          	bndmov %bnd0,\(%eax\)
 +[a-f0-9]+:	66 0f 1b 08          	bndmov %bnd1,\(%eax\)
 +[a-f0-9]+:	66 0f 1b 10          	bndmov %bnd2,\(%eax\)
 +[a-f0-9]+:	66 0f 1b 18          	bndmov %bnd3,\(%eax\)
 +[a-f0-9]+:	66 0f 1b 20          	bndmov \(bad\),\(%eax\)
 +[a-f0-9]+:	66 0f 1b 28          	bndmov \(bad\),\(%eax\)
 +[a-f0-9]+:	66 0f 1b 30          	bndmov \(bad\),\(%eax\)
 +[a-f0-9]+:	66 0f 1b 38          	bndmov \(bad\),\(%eax\)
 +[a-f0-9]+:	f3 0f 1b c0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1b c1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1b c2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1b c3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1b c4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1b c5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1b c6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1b c7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1b c8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1b c9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1b ca          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1b cb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1b cc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1b cd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1b ce          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1b cf          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1b d0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1b d1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1b d2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1b d3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1b d4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1b d5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1b d6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1b d7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1b d8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1b d9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1b da          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1b db          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1b dc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1b dd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1b de          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1b df          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1b e0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1b e1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1b e2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1b e3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1b e4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1b e5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1b e6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1b e7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1b e8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1b e9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1b ea          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1b eb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1b ec          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1b ed          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1b ee          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1b ef          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1b f0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1b f1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1b f2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1b f3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1b f4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1b f5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1b f6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1b f7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1b f8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1b f9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1b fa          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1b fb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1b fc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1b fd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1b fe          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1b ff          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1b 00          	bndmk  \(%eax\),%bnd0
 +[a-f0-9]+:	f3 0f 1b 08          	bndmk  \(%eax\),%bnd1
 +[a-f0-9]+:	f3 0f 1b 10          	bndmk  \(%eax\),%bnd2
 +[a-f0-9]+:	f3 0f 1b 18          	bndmk  \(%eax\),%bnd3
 +[a-f0-9]+:	f3 0f 1b 20          	bndmk  \(%eax\),\(bad\)
 +[a-f0-9]+:	f3 0f 1b 28          	bndmk  \(%eax\),\(bad\)
 +[a-f0-9]+:	f3 0f 1b 30          	bndmk  \(%eax\),\(bad\)
 +[a-f0-9]+:	f3 0f 1b 38          	bndmk  \(%eax\),\(bad\)
 +[a-f0-9]+:	f2 0f 1b c0          	bndcn  %eax,%bnd0
 +[a-f0-9]+:	f2 0f 1b c1          	bndcn  %ecx,%bnd0
 +[a-f0-9]+:	f2 0f 1b c2          	bndcn  %edx,%bnd0
 +[a-f0-9]+:	f2 0f 1b c3          	bndcn  %ebx,%bnd0
 +[a-f0-9]+:	f2 0f 1b c4          	bndcn  %esp,%bnd0
 +[a-f0-9]+:	f2 0f 1b c5          	bndcn  %ebp,%bnd0
 +[a-f0-9]+:	f2 0f 1b c6          	bndcn  %esi,%bnd0
 +[a-f0-9]+:	f2 0f 1b c7          	bndcn  %edi,%bnd0
 +[a-f0-9]+:	f2 0f 1b c8          	bndcn  %eax,%bnd1
 +[a-f0-9]+:	f2 0f 1b c9          	bndcn  %ecx,%bnd1
 +[a-f0-9]+:	f2 0f 1b ca          	bndcn  %edx,%bnd1
 +[a-f0-9]+:	f2 0f 1b cb          	bndcn  %ebx,%bnd1
 +[a-f0-9]+:	f2 0f 1b cc          	bndcn  %esp,%bnd1
 +[a-f0-9]+:	f2 0f 1b cd          	bndcn  %ebp,%bnd1
 +[a-f0-9]+:	f2 0f 1b ce          	bndcn  %esi,%bnd1
 +[a-f0-9]+:	f2 0f 1b cf          	bndcn  %edi,%bnd1
 +[a-f0-9]+:	f2 0f 1b d0          	bndcn  %eax,%bnd2
 +[a-f0-9]+:	f2 0f 1b d1          	bndcn  %ecx,%bnd2
 +[a-f0-9]+:	f2 0f 1b d2          	bndcn  %edx,%bnd2
 +[a-f0-9]+:	f2 0f 1b d3          	bndcn  %ebx,%bnd2
 +[a-f0-9]+:	f2 0f 1b d4          	bndcn  %esp,%bnd2
 +[a-f0-9]+:	f2 0f 1b d5          	bndcn  %ebp,%bnd2
 +[a-f0-9]+:	f2 0f 1b d6          	bndcn  %esi,%bnd2
 +[a-f0-9]+:	f2 0f 1b d7          	bndcn  %edi,%bnd2
 +[a-f0-9]+:	f2 0f 1b d8          	bndcn  %eax,%bnd3
 +[a-f0-9]+:	f2 0f 1b d9          	bndcn  %ecx,%bnd3
 +[a-f0-9]+:	f2 0f 1b da          	bndcn  %edx,%bnd3
 +[a-f0-9]+:	f2 0f 1b db          	bndcn  %ebx,%bnd3
 +[a-f0-9]+:	f2 0f 1b dc          	bndcn  %esp,%bnd3
 +[a-f0-9]+:	f2 0f 1b dd          	bndcn  %ebp,%bnd3
 +[a-f0-9]+:	f2 0f 1b de          	bndcn  %esi,%bnd3
 +[a-f0-9]+:	f2 0f 1b df          	bndcn  %edi,%bnd3
 +[a-f0-9]+:	f2 0f 1b e0          	bndcn  %eax,\(bad\)
 +[a-f0-9]+:	f2 0f 1b e1          	bndcn  %ecx,\(bad\)
 +[a-f0-9]+:	f2 0f 1b e2          	bndcn  %edx,\(bad\)
 +[a-f0-9]+:	f2 0f 1b e3          	bndcn  %ebx,\(bad\)
 +[a-f0-9]+:	f2 0f 1b e4          	bndcn  %esp,\(bad\)
 +[a-f0-9]+:	f2 0f 1b e5          	bndcn  %ebp,\(bad\)
 +[a-f0-9]+:	f2 0f 1b e6          	bndcn  %esi,\(bad\)
 +[a-f0-9]+:	f2 0f 1b e7          	bndcn  %edi,\(bad\)
 +[a-f0-9]+:	f2 0f 1b e8          	bndcn  %eax,\(bad\)
 +[a-f0-9]+:	f2 0f 1b e9          	bndcn  %ecx,\(bad\)
 +[a-f0-9]+:	f2 0f 1b ea          	bndcn  %edx,\(bad\)
 +[a-f0-9]+:	f2 0f 1b eb          	bndcn  %ebx,\(bad\)
 +[a-f0-9]+:	f2 0f 1b ec          	bndcn  %esp,\(bad\)
 +[a-f0-9]+:	f2 0f 1b ed          	bndcn  %ebp,\(bad\)
 +[a-f0-9]+:	f2 0f 1b ee          	bndcn  %esi,\(bad\)
 +[a-f0-9]+:	f2 0f 1b ef          	bndcn  %edi,\(bad\)
 +[a-f0-9]+:	f2 0f 1b f0          	bndcn  %eax,\(bad\)
 +[a-f0-9]+:	f2 0f 1b f1          	bndcn  %ecx,\(bad\)
 +[a-f0-9]+:	f2 0f 1b f2          	bndcn  %edx,\(bad\)
 +[a-f0-9]+:	f2 0f 1b f3          	bndcn  %ebx,\(bad\)
 +[a-f0-9]+:	f2 0f 1b f4          	bndcn  %esp,\(bad\)
 +[a-f0-9]+:	f2 0f 1b f5          	bndcn  %ebp,\(bad\)
 +[a-f0-9]+:	f2 0f 1b f6          	bndcn  %esi,\(bad\)
 +[a-f0-9]+:	f2 0f 1b f7          	bndcn  %edi,\(bad\)
 +[a-f0-9]+:	f2 0f 1b f8          	bndcn  %eax,\(bad\)
 +[a-f0-9]+:	f2 0f 1b f9          	bndcn  %ecx,\(bad\)
 +[a-f0-9]+:	f2 0f 1b fa          	bndcn  %edx,\(bad\)
 +[a-f0-9]+:	f2 0f 1b fb          	bndcn  %ebx,\(bad\)
 +[a-f0-9]+:	f2 0f 1b fc          	bndcn  %esp,\(bad\)
 +[a-f0-9]+:	f2 0f 1b fd          	bndcn  %ebp,\(bad\)
 +[a-f0-9]+:	f2 0f 1b fe          	bndcn  %esi,\(bad\)
 +[a-f0-9]+:	f2 0f 1b ff          	bndcn  %edi,\(bad\)
 +[a-f0-9]+:	f2 0f 1b 00          	bndcn  \(%eax\),%bnd0
 +[a-f0-9]+:	f2 0f 1b 08          	bndcn  \(%eax\),%bnd1
 +[a-f0-9]+:	f2 0f 1b 10          	bndcn  \(%eax\),%bnd2
 +[a-f0-9]+:	f2 0f 1b 18          	bndcn  \(%eax\),%bnd3
 +[a-f0-9]+:	f2 0f 1b 20          	bndcn  \(%eax\),\(bad\)
 +[a-f0-9]+:	f2 0f 1b 28          	bndcn  \(%eax\),\(bad\)
 +[a-f0-9]+:	f2 0f 1b 30          	bndcn  \(%eax\),\(bad\)
 +[a-f0-9]+:	f2 0f 1b 38          	bndcn  \(%eax\),\(bad\)

0[a-f0-9]+ <_0f1c>:
 +[a-f0-9]+:	0f 1c c0             	nop    %eax
 +[a-f0-9]+:	0f 1c c1             	nop    %ecx
 +[a-f0-9]+:	0f 1c c2             	nop    %edx
 +[a-f0-9]+:	0f 1c c3             	nop    %ebx
 +[a-f0-9]+:	0f 1c c4             	nop    %esp
 +[a-f0-9]+:	0f 1c c5             	nop    %ebp
 +[a-f0-9]+:	0f 1c c6             	nop    %esi
 +[a-f0-9]+:	0f 1c c7             	nop    %edi
 +[a-f0-9]+:	0f 1c c8             	nop    %eax
 +[a-f0-9]+:	0f 1c c9             	nop    %ecx
 +[a-f0-9]+:	0f 1c ca             	nop    %edx
 +[a-f0-9]+:	0f 1c cb             	nop    %ebx
 +[a-f0-9]+:	0f 1c cc             	nop    %esp
 +[a-f0-9]+:	0f 1c cd             	nop    %ebp
 +[a-f0-9]+:	0f 1c ce             	nop    %esi
 +[a-f0-9]+:	0f 1c cf             	nop    %edi
 +[a-f0-9]+:	0f 1c d0             	nop    %eax
 +[a-f0-9]+:	0f 1c d1             	nop    %ecx
 +[a-f0-9]+:	0f 1c d2             	nop    %edx
 +[a-f0-9]+:	0f 1c d3             	nop    %ebx
 +[a-f0-9]+:	0f 1c d4             	nop    %esp
 +[a-f0-9]+:	0f 1c d5             	nop    %ebp
 +[a-f0-9]+:	0f 1c d6             	nop    %esi
 +[a-f0-9]+:	0f 1c d7             	nop    %edi
 +[a-f0-9]+:	0f 1c d8             	nop    %eax
 +[a-f0-9]+:	0f 1c d9             	nop    %ecx
 +[a-f0-9]+:	0f 1c da             	nop    %edx
 +[a-f0-9]+:	0f 1c db             	nop    %ebx
 +[a-f0-9]+:	0f 1c dc             	nop    %esp
 +[a-f0-9]+:	0f 1c dd             	nop    %ebp
 +[a-f0-9]+:	0f 1c de             	nop    %esi
 +[a-f0-9]+:	0f 1c df             	nop    %edi
 +[a-f0-9]+:	0f 1c e0             	nop    %eax
 +[a-f0-9]+:	0f 1c e1             	nop    %ecx
 +[a-f0-9]+:	0f 1c e2             	nop    %edx
 +[a-f0-9]+:	0f 1c e3             	nop    %ebx
 +[a-f0-9]+:	0f 1c e4             	nop    %esp
 +[a-f0-9]+:	0f 1c e5             	nop    %ebp
 +[a-f0-9]+:	0f 1c e6             	nop    %esi
 +[a-f0-9]+:	0f 1c e7             	nop    %edi
 +[a-f0-9]+:	0f 1c e8             	nop    %eax
 +[a-f0-9]+:	0f 1c e9             	nop    %ecx
 +[a-f0-9]+:	0f 1c ea             	nop    %edx
 +[a-f0-9]+:	0f 1c eb             	nop    %ebx
 +[a-f0-9]+:	0f 1c ec             	nop    %esp
 +[a-f0-9]+:	0f 1c ed             	nop    %ebp
 +[a-f0-9]+:	0f 1c ee             	nop    %esi
 +[a-f0-9]+:	0f 1c ef             	nop    %edi
 +[a-f0-9]+:	0f 1c f0             	nop    %eax
 +[a-f0-9]+:	0f 1c f1             	nop    %ecx
 +[a-f0-9]+:	0f 1c f2             	nop    %edx
 +[a-f0-9]+:	0f 1c f3             	nop    %ebx
 +[a-f0-9]+:	0f 1c f4             	nop    %esp
 +[a-f0-9]+:	0f 1c f5             	nop    %ebp
 +[a-f0-9]+:	0f 1c f6             	nop    %esi
 +[a-f0-9]+:	0f 1c f7             	nop    %edi
 +[a-f0-9]+:	0f 1c f8             	nop    %eax
 +[a-f0-9]+:	0f 1c f9             	nop    %ecx
 +[a-f0-9]+:	0f 1c fa             	nop    %edx
 +[a-f0-9]+:	0f 1c fb             	nop    %ebx
 +[a-f0-9]+:	0f 1c fc             	nop    %esp
 +[a-f0-9]+:	0f 1c fd             	nop    %ebp
 +[a-f0-9]+:	0f 1c fe             	nop    %esi
 +[a-f0-9]+:	0f 1c ff             	nop    %edi
 +[a-f0-9]+:	0f 1c 00             	cldemote \(%eax\)
 +[a-f0-9]+:	0f 1c 08             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1c 10             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1c 18             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1c 20             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1c 28             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1c 30             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1c 38             	nopl   \(%eax\)
 +[a-f0-9]+:	66 0f 1c c0          	nop    %ax
 +[a-f0-9]+:	66 0f 1c c1          	nop    %cx
 +[a-f0-9]+:	66 0f 1c c2          	nop    %dx
 +[a-f0-9]+:	66 0f 1c c3          	nop    %bx
 +[a-f0-9]+:	66 0f 1c c4          	nop    %sp
 +[a-f0-9]+:	66 0f 1c c5          	nop    %bp
 +[a-f0-9]+:	66 0f 1c c6          	nop    %si
 +[a-f0-9]+:	66 0f 1c c7          	nop    %di
 +[a-f0-9]+:	66 0f 1c c8          	nop    %ax
 +[a-f0-9]+:	66 0f 1c c9          	nop    %cx
 +[a-f0-9]+:	66 0f 1c ca          	nop    %dx
 +[a-f0-9]+:	66 0f 1c cb          	nop    %bx
 +[a-f0-9]+:	66 0f 1c cc          	nop    %sp
 +[a-f0-9]+:	66 0f 1c cd          	nop    %bp
 +[a-f0-9]+:	66 0f 1c ce          	nop    %si
 +[a-f0-9]+:	66 0f 1c cf          	nop    %di
 +[a-f0-9]+:	66 0f 1c d0          	nop    %ax
 +[a-f0-9]+:	66 0f 1c d1          	nop    %cx
 +[a-f0-9]+:	66 0f 1c d2          	nop    %dx
 +[a-f0-9]+:	66 0f 1c d3          	nop    %bx
 +[a-f0-9]+:	66 0f 1c d4          	nop    %sp
 +[a-f0-9]+:	66 0f 1c d5          	nop    %bp
 +[a-f0-9]+:	66 0f 1c d6          	nop    %si
 +[a-f0-9]+:	66 0f 1c d7          	nop    %di
 +[a-f0-9]+:	66 0f 1c d8          	nop    %ax
 +[a-f0-9]+:	66 0f 1c d9          	nop    %cx
 +[a-f0-9]+:	66 0f 1c da          	nop    %dx
 +[a-f0-9]+:	66 0f 1c db          	nop    %bx
 +[a-f0-9]+:	66 0f 1c dc          	nop    %sp
 +[a-f0-9]+:	66 0f 1c dd          	nop    %bp
 +[a-f0-9]+:	66 0f 1c de          	nop    %si
 +[a-f0-9]+:	66 0f 1c df          	nop    %di
 +[a-f0-9]+:	66 0f 1c e0          	nop    %ax
 +[a-f0-9]+:	66 0f 1c e1          	nop    %cx
 +[a-f0-9]+:	66 0f 1c e2          	nop    %dx
 +[a-f0-9]+:	66 0f 1c e3          	nop    %bx
 +[a-f0-9]+:	66 0f 1c e4          	nop    %sp
 +[a-f0-9]+:	66 0f 1c e5          	nop    %bp
 +[a-f0-9]+:	66 0f 1c e6          	nop    %si
 +[a-f0-9]+:	66 0f 1c e7          	nop    %di
 +[a-f0-9]+:	66 0f 1c e8          	nop    %ax
 +[a-f0-9]+:	66 0f 1c e9          	nop    %cx
 +[a-f0-9]+:	66 0f 1c ea          	nop    %dx
 +[a-f0-9]+:	66 0f 1c eb          	nop    %bx
 +[a-f0-9]+:	66 0f 1c ec          	nop    %sp
 +[a-f0-9]+:	66 0f 1c ed          	nop    %bp
 +[a-f0-9]+:	66 0f 1c ee          	nop    %si
 +[a-f0-9]+:	66 0f 1c ef          	nop    %di
 +[a-f0-9]+:	66 0f 1c f0          	nop    %ax
 +[a-f0-9]+:	66 0f 1c f1          	nop    %cx
 +[a-f0-9]+:	66 0f 1c f2          	nop    %dx
 +[a-f0-9]+:	66 0f 1c f3          	nop    %bx
 +[a-f0-9]+:	66 0f 1c f4          	nop    %sp
 +[a-f0-9]+:	66 0f 1c f5          	nop    %bp
 +[a-f0-9]+:	66 0f 1c f6          	nop    %si
 +[a-f0-9]+:	66 0f 1c f7          	nop    %di
 +[a-f0-9]+:	66 0f 1c f8          	nop    %ax
 +[a-f0-9]+:	66 0f 1c f9          	nop    %cx
 +[a-f0-9]+:	66 0f 1c fa          	nop    %dx
 +[a-f0-9]+:	66 0f 1c fb          	nop    %bx
 +[a-f0-9]+:	66 0f 1c fc          	nop    %sp
 +[a-f0-9]+:	66 0f 1c fd          	nop    %bp
 +[a-f0-9]+:	66 0f 1c fe          	nop    %si
 +[a-f0-9]+:	66 0f 1c ff          	nop    %di
 +[a-f0-9]+:	66 0f 1c 00          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1c 08          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1c 10          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1c 18          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1c 20          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1c 28          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1c 30          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1c 38          	nopw   \(%eax\)
 +[a-f0-9]+:	f3 0f 1c c0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1c c1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1c c2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1c c3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1c c4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1c c5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1c c6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1c c7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1c c8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1c c9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1c ca          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1c cb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1c cc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1c cd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1c ce          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1c cf          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1c d0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1c d1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1c d2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1c d3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1c d4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1c d5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1c d6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1c d7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1c d8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1c d9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1c da          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1c db          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1c dc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1c dd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1c de          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1c df          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1c e0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1c e1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1c e2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1c e3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1c e4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1c e5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1c e6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1c e7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1c e8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1c e9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1c ea          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1c eb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1c ec          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1c ed          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1c ee          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1c ef          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1c f0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1c f1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1c f2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1c f3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1c f4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1c f5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1c f6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1c f7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1c f8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1c f9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1c fa          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1c fb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1c fc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1c fd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1c fe          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1c ff          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1c 00          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1c 08          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1c 10          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1c 18          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1c 20          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1c 28          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1c 30          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1c 38          	repz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1c c0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1c c1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1c c2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1c c3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1c c4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1c c5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1c c6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1c c7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1c c8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1c c9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1c ca          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1c cb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1c cc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1c cd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1c ce          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1c cf          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1c d0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1c d1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1c d2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1c d3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1c d4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1c d5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1c d6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1c d7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1c d8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1c d9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1c da          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1c db          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1c dc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1c dd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1c de          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1c df          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1c e0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1c e1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1c e2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1c e3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1c e4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1c e5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1c e6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1c e7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1c e8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1c e9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1c ea          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1c eb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1c ec          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1c ed          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1c ee          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1c ef          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1c f0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1c f1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1c f2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1c f3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1c f4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1c f5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1c f6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1c f7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1c f8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1c f9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1c fa          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1c fb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1c fc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1c fd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1c fe          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1c ff          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1c 00          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1c 08          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1c 10          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1c 18          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1c 20          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1c 28          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1c 30          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1c 38          	repnz nopl \(%eax\)

0[a-f0-9]+ <_0f1d>:
 +[a-f0-9]+:	0f 1d c0             	nop    %eax
 +[a-f0-9]+:	0f 1d c1             	nop    %ecx
 +[a-f0-9]+:	0f 1d c2             	nop    %edx
 +[a-f0-9]+:	0f 1d c3             	nop    %ebx
 +[a-f0-9]+:	0f 1d c4             	nop    %esp
 +[a-f0-9]+:	0f 1d c5             	nop    %ebp
 +[a-f0-9]+:	0f 1d c6             	nop    %esi
 +[a-f0-9]+:	0f 1d c7             	nop    %edi
 +[a-f0-9]+:	0f 1d c8             	nop    %eax
 +[a-f0-9]+:	0f 1d c9             	nop    %ecx
 +[a-f0-9]+:	0f 1d ca             	nop    %edx
 +[a-f0-9]+:	0f 1d cb             	nop    %ebx
 +[a-f0-9]+:	0f 1d cc             	nop    %esp
 +[a-f0-9]+:	0f 1d cd             	nop    %ebp
 +[a-f0-9]+:	0f 1d ce             	nop    %esi
 +[a-f0-9]+:	0f 1d cf             	nop    %edi
 +[a-f0-9]+:	0f 1d d0             	nop    %eax
 +[a-f0-9]+:	0f 1d d1             	nop    %ecx
 +[a-f0-9]+:	0f 1d d2             	nop    %edx
 +[a-f0-9]+:	0f 1d d3             	nop    %ebx
 +[a-f0-9]+:	0f 1d d4             	nop    %esp
 +[a-f0-9]+:	0f 1d d5             	nop    %ebp
 +[a-f0-9]+:	0f 1d d6             	nop    %esi
 +[a-f0-9]+:	0f 1d d7             	nop    %edi
 +[a-f0-9]+:	0f 1d d8             	nop    %eax
 +[a-f0-9]+:	0f 1d d9             	nop    %ecx
 +[a-f0-9]+:	0f 1d da             	nop    %edx
 +[a-f0-9]+:	0f 1d db             	nop    %ebx
 +[a-f0-9]+:	0f 1d dc             	nop    %esp
 +[a-f0-9]+:	0f 1d dd             	nop    %ebp
 +[a-f0-9]+:	0f 1d de             	nop    %esi
 +[a-f0-9]+:	0f 1d df             	nop    %edi
 +[a-f0-9]+:	0f 1d e0             	nop    %eax
 +[a-f0-9]+:	0f 1d e1             	nop    %ecx
 +[a-f0-9]+:	0f 1d e2             	nop    %edx
 +[a-f0-9]+:	0f 1d e3             	nop    %ebx
 +[a-f0-9]+:	0f 1d e4             	nop    %esp
 +[a-f0-9]+:	0f 1d e5             	nop    %ebp
 +[a-f0-9]+:	0f 1d e6             	nop    %esi
 +[a-f0-9]+:	0f 1d e7             	nop    %edi
 +[a-f0-9]+:	0f 1d e8             	nop    %eax
 +[a-f0-9]+:	0f 1d e9             	nop    %ecx
 +[a-f0-9]+:	0f 1d ea             	nop    %edx
 +[a-f0-9]+:	0f 1d eb             	nop    %ebx
 +[a-f0-9]+:	0f 1d ec             	nop    %esp
 +[a-f0-9]+:	0f 1d ed             	nop    %ebp
 +[a-f0-9]+:	0f 1d ee             	nop    %esi
 +[a-f0-9]+:	0f 1d ef             	nop    %edi
 +[a-f0-9]+:	0f 1d f0             	nop    %eax
 +[a-f0-9]+:	0f 1d f1             	nop    %ecx
 +[a-f0-9]+:	0f 1d f2             	nop    %edx
 +[a-f0-9]+:	0f 1d f3             	nop    %ebx
 +[a-f0-9]+:	0f 1d f4             	nop    %esp
 +[a-f0-9]+:	0f 1d f5             	nop    %ebp
 +[a-f0-9]+:	0f 1d f6             	nop    %esi
 +[a-f0-9]+:	0f 1d f7             	nop    %edi
 +[a-f0-9]+:	0f 1d f8             	nop    %eax
 +[a-f0-9]+:	0f 1d f9             	nop    %ecx
 +[a-f0-9]+:	0f 1d fa             	nop    %edx
 +[a-f0-9]+:	0f 1d fb             	nop    %ebx
 +[a-f0-9]+:	0f 1d fc             	nop    %esp
 +[a-f0-9]+:	0f 1d fd             	nop    %ebp
 +[a-f0-9]+:	0f 1d fe             	nop    %esi
 +[a-f0-9]+:	0f 1d ff             	nop    %edi
 +[a-f0-9]+:	0f 1d 00             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1d 08             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1d 10             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1d 18             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1d 20             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1d 28             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1d 30             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1d 38             	nopl   \(%eax\)
 +[a-f0-9]+:	66 0f 1d c0          	nop    %ax
 +[a-f0-9]+:	66 0f 1d c1          	nop    %cx
 +[a-f0-9]+:	66 0f 1d c2          	nop    %dx
 +[a-f0-9]+:	66 0f 1d c3          	nop    %bx
 +[a-f0-9]+:	66 0f 1d c4          	nop    %sp
 +[a-f0-9]+:	66 0f 1d c5          	nop    %bp
 +[a-f0-9]+:	66 0f 1d c6          	nop    %si
 +[a-f0-9]+:	66 0f 1d c7          	nop    %di
 +[a-f0-9]+:	66 0f 1d c8          	nop    %ax
 +[a-f0-9]+:	66 0f 1d c9          	nop    %cx
 +[a-f0-9]+:	66 0f 1d ca          	nop    %dx
 +[a-f0-9]+:	66 0f 1d cb          	nop    %bx
 +[a-f0-9]+:	66 0f 1d cc          	nop    %sp
 +[a-f0-9]+:	66 0f 1d cd          	nop    %bp
 +[a-f0-9]+:	66 0f 1d ce          	nop    %si
 +[a-f0-9]+:	66 0f 1d cf          	nop    %di
 +[a-f0-9]+:	66 0f 1d d0          	nop    %ax
 +[a-f0-9]+:	66 0f 1d d1          	nop    %cx
 +[a-f0-9]+:	66 0f 1d d2          	nop    %dx
 +[a-f0-9]+:	66 0f 1d d3          	nop    %bx
 +[a-f0-9]+:	66 0f 1d d4          	nop    %sp
 +[a-f0-9]+:	66 0f 1d d5          	nop    %bp
 +[a-f0-9]+:	66 0f 1d d6          	nop    %si
 +[a-f0-9]+:	66 0f 1d d7          	nop    %di
 +[a-f0-9]+:	66 0f 1d d8          	nop    %ax
 +[a-f0-9]+:	66 0f 1d d9          	nop    %cx
 +[a-f0-9]+:	66 0f 1d da          	nop    %dx
 +[a-f0-9]+:	66 0f 1d db          	nop    %bx
 +[a-f0-9]+:	66 0f 1d dc          	nop    %sp
 +[a-f0-9]+:	66 0f 1d dd          	nop    %bp
 +[a-f0-9]+:	66 0f 1d de          	nop    %si
 +[a-f0-9]+:	66 0f 1d df          	nop    %di
 +[a-f0-9]+:	66 0f 1d e0          	nop    %ax
 +[a-f0-9]+:	66 0f 1d e1          	nop    %cx
 +[a-f0-9]+:	66 0f 1d e2          	nop    %dx
 +[a-f0-9]+:	66 0f 1d e3          	nop    %bx
 +[a-f0-9]+:	66 0f 1d e4          	nop    %sp
 +[a-f0-9]+:	66 0f 1d e5          	nop    %bp
 +[a-f0-9]+:	66 0f 1d e6          	nop    %si
 +[a-f0-9]+:	66 0f 1d e7          	nop    %di
 +[a-f0-9]+:	66 0f 1d e8          	nop    %ax
 +[a-f0-9]+:	66 0f 1d e9          	nop    %cx
 +[a-f0-9]+:	66 0f 1d ea          	nop    %dx
 +[a-f0-9]+:	66 0f 1d eb          	nop    %bx
 +[a-f0-9]+:	66 0f 1d ec          	nop    %sp
 +[a-f0-9]+:	66 0f 1d ed          	nop    %bp
 +[a-f0-9]+:	66 0f 1d ee          	nop    %si
 +[a-f0-9]+:	66 0f 1d ef          	nop    %di
 +[a-f0-9]+:	66 0f 1d f0          	nop    %ax
 +[a-f0-9]+:	66 0f 1d f1          	nop    %cx
 +[a-f0-9]+:	66 0f 1d f2          	nop    %dx
 +[a-f0-9]+:	66 0f 1d f3          	nop    %bx
 +[a-f0-9]+:	66 0f 1d f4          	nop    %sp
 +[a-f0-9]+:	66 0f 1d f5          	nop    %bp
 +[a-f0-9]+:	66 0f 1d f6          	nop    %si
 +[a-f0-9]+:	66 0f 1d f7          	nop    %di
 +[a-f0-9]+:	66 0f 1d f8          	nop    %ax
 +[a-f0-9]+:	66 0f 1d f9          	nop    %cx
 +[a-f0-9]+:	66 0f 1d fa          	nop    %dx
 +[a-f0-9]+:	66 0f 1d fb          	nop    %bx
 +[a-f0-9]+:	66 0f 1d fc          	nop    %sp
 +[a-f0-9]+:	66 0f 1d fd          	nop    %bp
 +[a-f0-9]+:	66 0f 1d fe          	nop    %si
 +[a-f0-9]+:	66 0f 1d ff          	nop    %di
 +[a-f0-9]+:	66 0f 1d 00          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1d 08          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1d 10          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1d 18          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1d 20          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1d 28          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1d 30          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1d 38          	nopw   \(%eax\)
 +[a-f0-9]+:	f3 0f 1d c0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1d c1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1d c2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1d c3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1d c4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1d c5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1d c6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1d c7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1d c8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1d c9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1d ca          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1d cb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1d cc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1d cd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1d ce          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1d cf          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1d d0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1d d1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1d d2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1d d3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1d d4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1d d5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1d d6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1d d7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1d d8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1d d9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1d da          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1d db          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1d dc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1d dd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1d de          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1d df          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1d e0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1d e1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1d e2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1d e3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1d e4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1d e5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1d e6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1d e7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1d e8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1d e9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1d ea          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1d eb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1d ec          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1d ed          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1d ee          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1d ef          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1d f0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1d f1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1d f2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1d f3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1d f4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1d f5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1d f6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1d f7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1d f8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1d f9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1d fa          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1d fb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1d fc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1d fd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1d fe          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1d ff          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1d 00          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1d 08          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1d 10          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1d 18          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1d 20          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1d 28          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1d 30          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1d 38          	repz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1d c0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1d c1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1d c2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1d c3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1d c4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1d c5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1d c6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1d c7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1d c8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1d c9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1d ca          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1d cb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1d cc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1d cd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1d ce          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1d cf          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1d d0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1d d1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1d d2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1d d3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1d d4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1d d5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1d d6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1d d7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1d d8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1d d9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1d da          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1d db          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1d dc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1d dd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1d de          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1d df          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1d e0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1d e1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1d e2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1d e3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1d e4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1d e5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1d e6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1d e7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1d e8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1d e9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1d ea          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1d eb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1d ec          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1d ed          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1d ee          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1d ef          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1d f0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1d f1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1d f2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1d f3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1d f4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1d f5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1d f6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1d f7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1d f8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1d f9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1d fa          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1d fb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1d fc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1d fd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1d fe          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1d ff          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1d 00          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1d 08          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1d 10          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1d 18          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1d 20          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1d 28          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1d 30          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1d 38          	repnz nopl \(%eax\)

0[a-f0-9]+ <_0f1e>:
 +[a-f0-9]+:	0f 1e c0             	nop    %eax
 +[a-f0-9]+:	0f 1e c1             	nop    %ecx
 +[a-f0-9]+:	0f 1e c2             	nop    %edx
 +[a-f0-9]+:	0f 1e c3             	nop    %ebx
 +[a-f0-9]+:	0f 1e c4             	nop    %esp
 +[a-f0-9]+:	0f 1e c5             	nop    %ebp
 +[a-f0-9]+:	0f 1e c6             	nop    %esi
 +[a-f0-9]+:	0f 1e c7             	nop    %edi
 +[a-f0-9]+:	0f 1e c8             	nop    %eax
 +[a-f0-9]+:	0f 1e c9             	nop    %ecx
 +[a-f0-9]+:	0f 1e ca             	nop    %edx
 +[a-f0-9]+:	0f 1e cb             	nop    %ebx
 +[a-f0-9]+:	0f 1e cc             	nop    %esp
 +[a-f0-9]+:	0f 1e cd             	nop    %ebp
 +[a-f0-9]+:	0f 1e ce             	nop    %esi
 +[a-f0-9]+:	0f 1e cf             	nop    %edi
 +[a-f0-9]+:	0f 1e d0             	nop    %eax
 +[a-f0-9]+:	0f 1e d1             	nop    %ecx
 +[a-f0-9]+:	0f 1e d2             	nop    %edx
 +[a-f0-9]+:	0f 1e d3             	nop    %ebx
 +[a-f0-9]+:	0f 1e d4             	nop    %esp
 +[a-f0-9]+:	0f 1e d5             	nop    %ebp
 +[a-f0-9]+:	0f 1e d6             	nop    %esi
 +[a-f0-9]+:	0f 1e d7             	nop    %edi
 +[a-f0-9]+:	0f 1e d8             	nop    %eax
 +[a-f0-9]+:	0f 1e d9             	nop    %ecx
 +[a-f0-9]+:	0f 1e da             	nop    %edx
 +[a-f0-9]+:	0f 1e db             	nop    %ebx
 +[a-f0-9]+:	0f 1e dc             	nop    %esp
 +[a-f0-9]+:	0f 1e dd             	nop    %ebp
 +[a-f0-9]+:	0f 1e de             	nop    %esi
 +[a-f0-9]+:	0f 1e df             	nop    %edi
 +[a-f0-9]+:	0f 1e e0             	nop    %eax
 +[a-f0-9]+:	0f 1e e1             	nop    %ecx
 +[a-f0-9]+:	0f 1e e2             	nop    %edx
 +[a-f0-9]+:	0f 1e e3             	nop    %ebx
 +[a-f0-9]+:	0f 1e e4             	nop    %esp
 +[a-f0-9]+:	0f 1e e5             	nop    %ebp
 +[a-f0-9]+:	0f 1e e6             	nop    %esi
 +[a-f0-9]+:	0f 1e e7             	nop    %edi
 +[a-f0-9]+:	0f 1e e8             	nop    %eax
 +[a-f0-9]+:	0f 1e e9             	nop    %ecx
 +[a-f0-9]+:	0f 1e ea             	nop    %edx
 +[a-f0-9]+:	0f 1e eb             	nop    %ebx
 +[a-f0-9]+:	0f 1e ec             	nop    %esp
 +[a-f0-9]+:	0f 1e ed             	nop    %ebp
 +[a-f0-9]+:	0f 1e ee             	nop    %esi
 +[a-f0-9]+:	0f 1e ef             	nop    %edi
 +[a-f0-9]+:	0f 1e f0             	nop    %eax
 +[a-f0-9]+:	0f 1e f1             	nop    %ecx
 +[a-f0-9]+:	0f 1e f2             	nop    %edx
 +[a-f0-9]+:	0f 1e f3             	nop    %ebx
 +[a-f0-9]+:	0f 1e f4             	nop    %esp
 +[a-f0-9]+:	0f 1e f5             	nop    %ebp
 +[a-f0-9]+:	0f 1e f6             	nop    %esi
 +[a-f0-9]+:	0f 1e f7             	nop    %edi
 +[a-f0-9]+:	0f 1e f8             	nop    %eax
 +[a-f0-9]+:	0f 1e f9             	nop    %ecx
 +[a-f0-9]+:	0f 1e fa             	nop    %edx
 +[a-f0-9]+:	0f 1e fb             	nop    %ebx
 +[a-f0-9]+:	0f 1e fc             	nop    %esp
 +[a-f0-9]+:	0f 1e fd             	nop    %ebp
 +[a-f0-9]+:	0f 1e fe             	nop    %esi
 +[a-f0-9]+:	0f 1e ff             	nop    %edi
 +[a-f0-9]+:	0f 1e 00             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1e 08             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1e 10             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1e 18             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1e 20             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1e 28             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1e 30             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1e 38             	nopl   \(%eax\)
 +[a-f0-9]+:	66 0f 1e c0          	nop    %ax
 +[a-f0-9]+:	66 0f 1e c1          	nop    %cx
 +[a-f0-9]+:	66 0f 1e c2          	nop    %dx
 +[a-f0-9]+:	66 0f 1e c3          	nop    %bx
 +[a-f0-9]+:	66 0f 1e c4          	nop    %sp
 +[a-f0-9]+:	66 0f 1e c5          	nop    %bp
 +[a-f0-9]+:	66 0f 1e c6          	nop    %si
 +[a-f0-9]+:	66 0f 1e c7          	nop    %di
 +[a-f0-9]+:	66 0f 1e c8          	nop    %ax
 +[a-f0-9]+:	66 0f 1e c9          	nop    %cx
 +[a-f0-9]+:	66 0f 1e ca          	nop    %dx
 +[a-f0-9]+:	66 0f 1e cb          	nop    %bx
 +[a-f0-9]+:	66 0f 1e cc          	nop    %sp
 +[a-f0-9]+:	66 0f 1e cd          	nop    %bp
 +[a-f0-9]+:	66 0f 1e ce          	nop    %si
 +[a-f0-9]+:	66 0f 1e cf          	nop    %di
 +[a-f0-9]+:	66 0f 1e d0          	nop    %ax
 +[a-f0-9]+:	66 0f 1e d1          	nop    %cx
 +[a-f0-9]+:	66 0f 1e d2          	nop    %dx
 +[a-f0-9]+:	66 0f 1e d3          	nop    %bx
 +[a-f0-9]+:	66 0f 1e d4          	nop    %sp
 +[a-f0-9]+:	66 0f 1e d5          	nop    %bp
 +[a-f0-9]+:	66 0f 1e d6          	nop    %si
 +[a-f0-9]+:	66 0f 1e d7          	nop    %di
 +[a-f0-9]+:	66 0f 1e d8          	nop    %ax
 +[a-f0-9]+:	66 0f 1e d9          	nop    %cx
 +[a-f0-9]+:	66 0f 1e da          	nop    %dx
 +[a-f0-9]+:	66 0f 1e db          	nop    %bx
 +[a-f0-9]+:	66 0f 1e dc          	nop    %sp
 +[a-f0-9]+:	66 0f 1e dd          	nop    %bp
 +[a-f0-9]+:	66 0f 1e de          	nop    %si
 +[a-f0-9]+:	66 0f 1e df          	nop    %di
 +[a-f0-9]+:	66 0f 1e e0          	nop    %ax
 +[a-f0-9]+:	66 0f 1e e1          	nop    %cx
 +[a-f0-9]+:	66 0f 1e e2          	nop    %dx
 +[a-f0-9]+:	66 0f 1e e3          	nop    %bx
 +[a-f0-9]+:	66 0f 1e e4          	nop    %sp
 +[a-f0-9]+:	66 0f 1e e5          	nop    %bp
 +[a-f0-9]+:	66 0f 1e e6          	nop    %si
 +[a-f0-9]+:	66 0f 1e e7          	nop    %di
 +[a-f0-9]+:	66 0f 1e e8          	nop    %ax
 +[a-f0-9]+:	66 0f 1e e9          	nop    %cx
 +[a-f0-9]+:	66 0f 1e ea          	nop    %dx
 +[a-f0-9]+:	66 0f 1e eb          	nop    %bx
 +[a-f0-9]+:	66 0f 1e ec          	nop    %sp
 +[a-f0-9]+:	66 0f 1e ed          	nop    %bp
 +[a-f0-9]+:	66 0f 1e ee          	nop    %si
 +[a-f0-9]+:	66 0f 1e ef          	nop    %di
 +[a-f0-9]+:	66 0f 1e f0          	nop    %ax
 +[a-f0-9]+:	66 0f 1e f1          	nop    %cx
 +[a-f0-9]+:	66 0f 1e f2          	nop    %dx
 +[a-f0-9]+:	66 0f 1e f3          	nop    %bx
 +[a-f0-9]+:	66 0f 1e f4          	nop    %sp
 +[a-f0-9]+:	66 0f 1e f5          	nop    %bp
 +[a-f0-9]+:	66 0f 1e f6          	nop    %si
 +[a-f0-9]+:	66 0f 1e f7          	nop    %di
 +[a-f0-9]+:	66 0f 1e f8          	nop    %ax
 +[a-f0-9]+:	66 0f 1e f9          	nop    %cx
 +[a-f0-9]+:	66 0f 1e fa          	nop    %dx
 +[a-f0-9]+:	66 0f 1e fb          	nop    %bx
 +[a-f0-9]+:	66 0f 1e fc          	nop    %sp
 +[a-f0-9]+:	66 0f 1e fd          	nop    %bp
 +[a-f0-9]+:	66 0f 1e fe          	nop    %si
 +[a-f0-9]+:	66 0f 1e ff          	nop    %di
 +[a-f0-9]+:	66 0f 1e 00          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1e 08          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1e 10          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1e 18          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1e 20          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1e 28          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1e 30          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1e 38          	nopw   \(%eax\)
 +[a-f0-9]+:	f3 0f 1e c0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1e c1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1e c2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1e c3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1e c4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1e c5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1e c6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1e c7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1e c8          	rdsspd %eax
 +[a-f0-9]+:	f3 0f 1e c9          	rdsspd %ecx
 +[a-f0-9]+:	f3 0f 1e ca          	rdsspd %edx
 +[a-f0-9]+:	f3 0f 1e cb          	rdsspd %ebx
 +[a-f0-9]+:	f3 0f 1e cc          	rdsspd %esp
 +[a-f0-9]+:	f3 0f 1e cd          	rdsspd %ebp
 +[a-f0-9]+:	f3 0f 1e ce          	rdsspd %esi
 +[a-f0-9]+:	f3 0f 1e cf          	rdsspd %edi
 +[a-f0-9]+:	f3 0f 1e d0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1e d1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1e d2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1e d3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1e d4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1e d5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1e d6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1e d7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1e d8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1e d9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1e da          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1e db          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1e dc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1e dd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1e de          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1e df          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1e e0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1e e1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1e e2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1e e3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1e e4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1e e5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1e e6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1e e7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1e e8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1e e9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1e ea          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1e eb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1e ec          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1e ed          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1e ee          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1e ef          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1e f0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1e f1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1e f2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1e f3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1e f4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1e f5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1e f6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1e f7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1e f8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1e f9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1e fa          	endbr64
 +[a-f0-9]+:	f3 0f 1e fb          	endbr32
 +[a-f0-9]+:	f3 0f 1e fc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1e fd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1e fe          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1e ff          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1e 00          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1e 08          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1e 10          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1e 18          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1e 20          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1e 28          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1e 30          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1e 38          	repz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1e c0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1e c1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1e c2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1e c3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1e c4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1e c5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1e c6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1e c7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1e c8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1e c9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1e ca          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1e cb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1e cc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1e cd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1e ce          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1e cf          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1e d0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1e d1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1e d2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1e d3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1e d4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1e d5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1e d6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1e d7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1e d8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1e d9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1e da          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1e db          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1e dc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1e dd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1e de          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1e df          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1e e0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1e e1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1e e2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1e e3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1e e4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1e e5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1e e6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1e e7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1e e8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1e e9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1e ea          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1e eb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1e ec          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1e ed          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1e ee          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1e ef          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1e f0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1e f1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1e f2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1e f3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1e f4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1e f5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1e f6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1e f7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1e f8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1e f9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1e fa          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1e fb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1e fc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1e fd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1e fe          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1e ff          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1e 00          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1e 08          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1e 10          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1e 18          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1e 20          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1e 28          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1e 30          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1e 38          	repnz nopl \(%eax\)

0[a-f0-9]+ <_0f1f>:
 +[a-f0-9]+:	0f 1f c0             	nop    %eax
 +[a-f0-9]+:	0f 1f c1             	nop    %ecx
 +[a-f0-9]+:	0f 1f c2             	nop    %edx
 +[a-f0-9]+:	0f 1f c3             	nop    %ebx
 +[a-f0-9]+:	0f 1f c4             	nop    %esp
 +[a-f0-9]+:	0f 1f c5             	nop    %ebp
 +[a-f0-9]+:	0f 1f c6             	nop    %esi
 +[a-f0-9]+:	0f 1f c7             	nop    %edi
 +[a-f0-9]+:	0f 1f c8             	nop    %eax
 +[a-f0-9]+:	0f 1f c9             	nop    %ecx
 +[a-f0-9]+:	0f 1f ca             	nop    %edx
 +[a-f0-9]+:	0f 1f cb             	nop    %ebx
 +[a-f0-9]+:	0f 1f cc             	nop    %esp
 +[a-f0-9]+:	0f 1f cd             	nop    %ebp
 +[a-f0-9]+:	0f 1f ce             	nop    %esi
 +[a-f0-9]+:	0f 1f cf             	nop    %edi
 +[a-f0-9]+:	0f 1f d0             	nop    %eax
 +[a-f0-9]+:	0f 1f d1             	nop    %ecx
 +[a-f0-9]+:	0f 1f d2             	nop    %edx
 +[a-f0-9]+:	0f 1f d3             	nop    %ebx
 +[a-f0-9]+:	0f 1f d4             	nop    %esp
 +[a-f0-9]+:	0f 1f d5             	nop    %ebp
 +[a-f0-9]+:	0f 1f d6             	nop    %esi
 +[a-f0-9]+:	0f 1f d7             	nop    %edi
 +[a-f0-9]+:	0f 1f d8             	nop    %eax
 +[a-f0-9]+:	0f 1f d9             	nop    %ecx
 +[a-f0-9]+:	0f 1f da             	nop    %edx
 +[a-f0-9]+:	0f 1f db             	nop    %ebx
 +[a-f0-9]+:	0f 1f dc             	nop    %esp
 +[a-f0-9]+:	0f 1f dd             	nop    %ebp
 +[a-f0-9]+:	0f 1f de             	nop    %esi
 +[a-f0-9]+:	0f 1f df             	nop    %edi
 +[a-f0-9]+:	0f 1f e0             	nop    %eax
 +[a-f0-9]+:	0f 1f e1             	nop    %ecx
 +[a-f0-9]+:	0f 1f e2             	nop    %edx
 +[a-f0-9]+:	0f 1f e3             	nop    %ebx
 +[a-f0-9]+:	0f 1f e4             	nop    %esp
 +[a-f0-9]+:	0f 1f e5             	nop    %ebp
 +[a-f0-9]+:	0f 1f e6             	nop    %esi
 +[a-f0-9]+:	0f 1f e7             	nop    %edi
 +[a-f0-9]+:	0f 1f e8             	nop    %eax
 +[a-f0-9]+:	0f 1f e9             	nop    %ecx
 +[a-f0-9]+:	0f 1f ea             	nop    %edx
 +[a-f0-9]+:	0f 1f eb             	nop    %ebx
 +[a-f0-9]+:	0f 1f ec             	nop    %esp
 +[a-f0-9]+:	0f 1f ed             	nop    %ebp
 +[a-f0-9]+:	0f 1f ee             	nop    %esi
 +[a-f0-9]+:	0f 1f ef             	nop    %edi
 +[a-f0-9]+:	0f 1f f0             	nop    %eax
 +[a-f0-9]+:	0f 1f f1             	nop    %ecx
 +[a-f0-9]+:	0f 1f f2             	nop    %edx
 +[a-f0-9]+:	0f 1f f3             	nop    %ebx
 +[a-f0-9]+:	0f 1f f4             	nop    %esp
 +[a-f0-9]+:	0f 1f f5             	nop    %ebp
 +[a-f0-9]+:	0f 1f f6             	nop    %esi
 +[a-f0-9]+:	0f 1f f7             	nop    %edi
 +[a-f0-9]+:	0f 1f f8             	nop    %eax
 +[a-f0-9]+:	0f 1f f9             	nop    %ecx
 +[a-f0-9]+:	0f 1f fa             	nop    %edx
 +[a-f0-9]+:	0f 1f fb             	nop    %ebx
 +[a-f0-9]+:	0f 1f fc             	nop    %esp
 +[a-f0-9]+:	0f 1f fd             	nop    %ebp
 +[a-f0-9]+:	0f 1f fe             	nop    %esi
 +[a-f0-9]+:	0f 1f ff             	nop    %edi
 +[a-f0-9]+:	0f 1f 00             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1f 08             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1f 10             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1f 18             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1f 20             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1f 28             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1f 30             	nopl   \(%eax\)
 +[a-f0-9]+:	0f 1f 38             	nopl   \(%eax\)
 +[a-f0-9]+:	66 0f 1f c0          	nop    %ax
 +[a-f0-9]+:	66 0f 1f c1          	nop    %cx
 +[a-f0-9]+:	66 0f 1f c2          	nop    %dx
 +[a-f0-9]+:	66 0f 1f c3          	nop    %bx
 +[a-f0-9]+:	66 0f 1f c4          	nop    %sp
 +[a-f0-9]+:	66 0f 1f c5          	nop    %bp
 +[a-f0-9]+:	66 0f 1f c6          	nop    %si
 +[a-f0-9]+:	66 0f 1f c7          	nop    %di
 +[a-f0-9]+:	66 0f 1f c8          	nop    %ax
 +[a-f0-9]+:	66 0f 1f c9          	nop    %cx
 +[a-f0-9]+:	66 0f 1f ca          	nop    %dx
 +[a-f0-9]+:	66 0f 1f cb          	nop    %bx
 +[a-f0-9]+:	66 0f 1f cc          	nop    %sp
 +[a-f0-9]+:	66 0f 1f cd          	nop    %bp
 +[a-f0-9]+:	66 0f 1f ce          	nop    %si
 +[a-f0-9]+:	66 0f 1f cf          	nop    %di
 +[a-f0-9]+:	66 0f 1f d0          	nop    %ax
 +[a-f0-9]+:	66 0f 1f d1          	nop    %cx
 +[a-f0-9]+:	66 0f 1f d2          	nop    %dx
 +[a-f0-9]+:	66 0f 1f d3          	nop    %bx
 +[a-f0-9]+:	66 0f 1f d4          	nop    %sp
 +[a-f0-9]+:	66 0f 1f d5          	nop    %bp
 +[a-f0-9]+:	66 0f 1f d6          	nop    %si
 +[a-f0-9]+:	66 0f 1f d7          	nop    %di
 +[a-f0-9]+:	66 0f 1f d8          	nop    %ax
 +[a-f0-9]+:	66 0f 1f d9          	nop    %cx
 +[a-f0-9]+:	66 0f 1f da          	nop    %dx
 +[a-f0-9]+:	66 0f 1f db          	nop    %bx
 +[a-f0-9]+:	66 0f 1f dc          	nop    %sp
 +[a-f0-9]+:	66 0f 1f dd          	nop    %bp
 +[a-f0-9]+:	66 0f 1f de          	nop    %si
 +[a-f0-9]+:	66 0f 1f df          	nop    %di
 +[a-f0-9]+:	66 0f 1f e0          	nop    %ax
 +[a-f0-9]+:	66 0f 1f e1          	nop    %cx
 +[a-f0-9]+:	66 0f 1f e2          	nop    %dx
 +[a-f0-9]+:	66 0f 1f e3          	nop    %bx
 +[a-f0-9]+:	66 0f 1f e4          	nop    %sp
 +[a-f0-9]+:	66 0f 1f e5          	nop    %bp
 +[a-f0-9]+:	66 0f 1f e6          	nop    %si
 +[a-f0-9]+:	66 0f 1f e7          	nop    %di
 +[a-f0-9]+:	66 0f 1f e8          	nop    %ax
 +[a-f0-9]+:	66 0f 1f e9          	nop    %cx
 +[a-f0-9]+:	66 0f 1f ea          	nop    %dx
 +[a-f0-9]+:	66 0f 1f eb          	nop    %bx
 +[a-f0-9]+:	66 0f 1f ec          	nop    %sp
 +[a-f0-9]+:	66 0f 1f ed          	nop    %bp
 +[a-f0-9]+:	66 0f 1f ee          	nop    %si
 +[a-f0-9]+:	66 0f 1f ef          	nop    %di
 +[a-f0-9]+:	66 0f 1f f0          	nop    %ax
 +[a-f0-9]+:	66 0f 1f f1          	nop    %cx
 +[a-f0-9]+:	66 0f 1f f2          	nop    %dx
 +[a-f0-9]+:	66 0f 1f f3          	nop    %bx
 +[a-f0-9]+:	66 0f 1f f4          	nop    %sp
 +[a-f0-9]+:	66 0f 1f f5          	nop    %bp
 +[a-f0-9]+:	66 0f 1f f6          	nop    %si
 +[a-f0-9]+:	66 0f 1f f7          	nop    %di
 +[a-f0-9]+:	66 0f 1f f8          	nop    %ax
 +[a-f0-9]+:	66 0f 1f f9          	nop    %cx
 +[a-f0-9]+:	66 0f 1f fa          	nop    %dx
 +[a-f0-9]+:	66 0f 1f fb          	nop    %bx
 +[a-f0-9]+:	66 0f 1f fc          	nop    %sp
 +[a-f0-9]+:	66 0f 1f fd          	nop    %bp
 +[a-f0-9]+:	66 0f 1f fe          	nop    %si
 +[a-f0-9]+:	66 0f 1f ff          	nop    %di
 +[a-f0-9]+:	66 0f 1f 00          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1f 08          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1f 10          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1f 18          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1f 20          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1f 28          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1f 30          	nopw   \(%eax\)
 +[a-f0-9]+:	66 0f 1f 38          	nopw   \(%eax\)
 +[a-f0-9]+:	f3 0f 1f c0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1f c1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1f c2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1f c3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1f c4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1f c5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1f c6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1f c7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1f c8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1f c9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1f ca          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1f cb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1f cc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1f cd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1f ce          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1f cf          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1f d0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1f d1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1f d2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1f d3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1f d4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1f d5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1f d6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1f d7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1f d8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1f d9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1f da          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1f db          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1f dc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1f dd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1f de          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1f df          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1f e0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1f e1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1f e2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1f e3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1f e4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1f e5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1f e6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1f e7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1f e8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1f e9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1f ea          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1f eb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1f ec          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1f ed          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1f ee          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1f ef          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1f f0          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1f f1          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1f f2          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1f f3          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1f f4          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1f f5          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1f f6          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1f f7          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1f f8          	repz nop %eax
 +[a-f0-9]+:	f3 0f 1f f9          	repz nop %ecx
 +[a-f0-9]+:	f3 0f 1f fa          	repz nop %edx
 +[a-f0-9]+:	f3 0f 1f fb          	repz nop %ebx
 +[a-f0-9]+:	f3 0f 1f fc          	repz nop %esp
 +[a-f0-9]+:	f3 0f 1f fd          	repz nop %ebp
 +[a-f0-9]+:	f3 0f 1f fe          	repz nop %esi
 +[a-f0-9]+:	f3 0f 1f ff          	repz nop %edi
 +[a-f0-9]+:	f3 0f 1f 00          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1f 08          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1f 10          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1f 18          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1f 20          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1f 28          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1f 30          	repz nopl \(%eax\)
 +[a-f0-9]+:	f3 0f 1f 38          	repz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1f c0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1f c1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1f c2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1f c3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1f c4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1f c5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1f c6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1f c7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1f c8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1f c9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1f ca          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1f cb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1f cc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1f cd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1f ce          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1f cf          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1f d0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1f d1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1f d2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1f d3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1f d4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1f d5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1f d6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1f d7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1f d8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1f d9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1f da          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1f db          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1f dc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1f dd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1f de          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1f df          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1f e0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1f e1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1f e2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1f e3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1f e4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1f e5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1f e6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1f e7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1f e8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1f e9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1f ea          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1f eb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1f ec          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1f ed          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1f ee          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1f ef          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1f f0          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1f f1          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1f f2          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1f f3          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1f f4          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1f f5          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1f f6          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1f f7          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1f f8          	repnz nop %eax
 +[a-f0-9]+:	f2 0f 1f f9          	repnz nop %ecx
 +[a-f0-9]+:	f2 0f 1f fa          	repnz nop %edx
 +[a-f0-9]+:	f2 0f 1f fb          	repnz nop %ebx
 +[a-f0-9]+:	f2 0f 1f fc          	repnz nop %esp
 +[a-f0-9]+:	f2 0f 1f fd          	repnz nop %ebp
 +[a-f0-9]+:	f2 0f 1f fe          	repnz nop %esi
 +[a-f0-9]+:	f2 0f 1f ff          	repnz nop %edi
 +[a-f0-9]+:	f2 0f 1f 00          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1f 08          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1f 10          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1f 18          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1f 20          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1f 28          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1f 30          	repnz nopl \(%eax\)
 +[a-f0-9]+:	f2 0f 1f 38          	repnz nopl \(%eax\)
#pass
