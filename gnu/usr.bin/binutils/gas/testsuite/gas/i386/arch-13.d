#as: -march=i686+smap+adx+rdseed+clzero+xsavec+xsaves+clflushopt+mwaitx+rdpid+clwb+wbnoinvd+rdpru+mcommit+svme+sev_es
#objdump: -dw
#name: i386 arch 13

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
[ 	]*[a-f0-9]+:	67 0f 01 fc          	addr16 clzero
[ 	]*[a-f0-9]+:	0f c7 21             	xsavec \(%ecx\)
[ 	]*[a-f0-9]+:	0f c7 29             	xsaves \(%ecx\)
[ 	]*[a-f0-9]+:	66 0f ae 39          	clflushopt \(%ecx\)
[ 	]*[a-f0-9]+:	0f 01 fa             	monitorx %eax,%ecx,%edx
[ 	]*[a-f0-9]+:	67 0f 01 fa          	monitorx %ax,%ecx,%edx
[ 	]*[a-f0-9]+:	0f 01 fa             	monitorx %eax,%ecx,%edx
[ 	]*[a-f0-9]+:	0f 01 fb             	mwaitx %eax,%ecx,%ebx
[ 	]*[a-f0-9]+:	0f 01 fb             	mwaitx %eax,%ecx,%ebx
[ 	]*[a-f0-9]+:[ 	]*66 0f ae 31[ 	]*clwb   \(%ecx\)
[ 	]*[a-f0-9]+:[ 	]*66 0f ae b4 f4 c0 1d fe ff[ 	]*clwb   -0x1e240\(%esp,%esi,8\)
[ 	]*[a-f0-9]+:[ 	]*f3 0f 01 fa[ 	]*mcommit
[ 	]*[a-f0-9]+:[ 	]*f3 0f c7 f8[ 	]*rdpid  %eax
[ 	]*[a-f0-9]+:[ 	]*0f 01 fd[ 	]*rdpru
[ 	]*[a-f0-9]+:[ 	]*f3 0f 01 d9[ 	]*vmgexit
[ 	]*[a-f0-9]+:[ 	]*f2 0f 01 d9[ 	]*vmgexit
[ 	]*[a-f0-9]+:[ 	]*f3 0f 09[ 	]*wbnoinvd
#pass
