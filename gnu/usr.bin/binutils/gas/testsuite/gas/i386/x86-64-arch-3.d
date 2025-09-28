#as: -march=generic64+smap+adx+rdseed+clzero+sha+xsavec+xsaves+clflushopt+mwaitx+rdpid+clwb+wbnoinvd+rdpru+mcommit
#objdump: -dw
#name: x86-64 arch 3

.*:     file format .*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	0f 01 ca             	clac
[ 	]*[a-f0-9]+:	0f 01 cb             	stac
[ 	]*[a-f0-9]+:	66 0f 38 f6 ca       	adcx   %edx,%ecx
[ 	]*[a-f0-9]+:	f3 0f 38 f6 ca       	adox   %edx,%ecx
[ 	]*[a-f0-9]+:	0f c7 f8             	rdseed %eax
[ 	]*[a-f0-9]+:	0f 01 fc             	clzero
[ 	]*[a-f0-9]+:	0f 01 fc             	clzero
[ 	]*[a-f0-9]+:	67 0f 01 fc          	addr32 clzero
[ 	]*[a-f0-9]+:	44 0f 38 c8 00       	sha1nexte \(%rax\),%xmm8
[ 	]*[a-f0-9]+:	48 0f c7 21          	xsavec64 \(%rcx\)
[ 	]*[a-f0-9]+:	48 0f c7 29          	xsaves64 \(%rcx\)
[ 	]*[a-f0-9]+:	66 0f ae 39          	clflushopt \(%rcx\)
[ 	]*[a-f0-9]+:	0f 01 fa             	monitorx %rax,%ecx,%edx
[ 	]*[a-f0-9]+:	67 0f 01 fa          	monitorx %eax,%ecx,%edx
[ 	]*[a-f0-9]+:	0f 01 fa             	monitorx %rax,%ecx,%edx
[ 	]*[a-f0-9]+:	67 0f 01 fa          	monitorx %eax,%ecx,%edx
[ 	]*[a-f0-9]+:	0f 01 fa             	monitorx %rax,%ecx,%edx
[ 	]*[a-f0-9]+:	0f 01 fb             	mwaitx %eax,%ecx,%ebx
[ 	]*[a-f0-9]+:	0f 01 fb             	mwaitx %eax,%ecx,%ebx
[ 	]*[a-f0-9]+:	0f 01 fb             	mwaitx %eax,%ecx,%ebx
[ 	]*[a-f0-9]+:[ 	]*66 0f ae 31[ 	]*clwb   \(%rcx\)
[ 	]*[a-f0-9]+:[ 	]*66 42 0f ae b4 f0 23 01 00 00[ 	]*clwb   0x123\(%rax,%r14,8\)
[ 	]*[a-f0-9]+:[ 	]*f3 0f 01 fa[ 	]*mcommit
[ 	]*[a-f0-9]+:[ 	]*f3 0f c7 f8[ 	]*rdpid  %rax
[ 	]*[a-f0-9]+:[ 	]*f3 41 0f c7 fa[ 	]*rdpid  %r10
[ 	]*[a-f0-9]+:[ 	]*0f 01 fd[ 	]*rdpru
[ 	]*[a-f0-9]+:[ 	]*f3 0f 09[ 	]*wbnoinvd
#pass
