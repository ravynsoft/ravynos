#as: -mavxscalar=256
#objdump: -dw
#name: i386 VEX non-LIG insns with -mavxscalar=256

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	c5 f9 6e c0          	vmovd  %eax,%xmm0
 +[a-f0-9]+:	c5 f9 6e 00          	vmovd  \(%eax\),%xmm0
 +[a-f0-9]+:	c4 e1 79 6e c0       	vmovd  %eax,%xmm0
 +[a-f0-9]+:	c4 e1 79 6e 00       	vmovd  \(%eax\),%xmm0
 +[a-f0-9]+:	c5 f9 7e c0          	vmovd  %xmm0,%eax
 +[a-f0-9]+:	c5 f9 7e 00          	vmovd  %xmm0,\(%eax\)
 +[a-f0-9]+:	c4 e1 79 7e c0       	vmovd  %xmm0,%eax
 +[a-f0-9]+:	c4 e1 79 7e 00       	vmovd  %xmm0,\(%eax\)
 +[a-f0-9]+:	c5 fa 7e c0          	vmovq  %xmm0,%xmm0
 +[a-f0-9]+:	c5 fa 7e 00          	vmovq  \(%eax\),%xmm0
 +[a-f0-9]+:	c4 e1 7a 7e c0       	vmovq  %xmm0,%xmm0
 +[a-f0-9]+:	c4 e1 7a 7e 00       	vmovq  \(%eax\),%xmm0
 +[a-f0-9]+:	c5 f9 d6 c0          	vmovq  %xmm0,%xmm0
 +[a-f0-9]+:	c5 f9 d6 00          	vmovq  %xmm0,\(%eax\)
 +[a-f0-9]+:	c4 e1 79 d6 c0       	vmovq  %xmm0,%xmm0
 +[a-f0-9]+:	c4 e1 79 d6 00       	vmovq  %xmm0,\(%eax\)
 +[a-f0-9]+:	c4 e3 79 17 c0 00    	vextractps \$0x0,%xmm0,%eax
 +[a-f0-9]+:	c4 e3 79 17 00 00    	vextractps \$0x0,%xmm0,\(%eax\)
 +[a-f0-9]+:	c4 e3 79 14 c0 00    	vpextrb \$0x0,%xmm0,%eax
 +[a-f0-9]+:	c4 e3 79 14 00 00    	vpextrb \$0x0,%xmm0,\(%eax\)
 +[a-f0-9]+:	c5 f9 c5 c0 00       	vpextrw \$0x0,%xmm0,%eax
 +[a-f0-9]+:	c4 e1 79 c5 c0 00    	vpextrw \$0x0,%xmm0,%eax
 +[a-f0-9]+:	c4 e3 79 15 c0 00    	vpextrw \$0x0,%xmm0,%eax
 +[a-f0-9]+:	c4 e3 79 15 00 00    	vpextrw \$0x0,%xmm0,\(%eax\)
 +[a-f0-9]+:	c4 e3 79 16 c0 00    	vpextrd \$0x0,%xmm0,%eax
 +[a-f0-9]+:	c4 e3 79 16 00 00    	vpextrd \$0x0,%xmm0,\(%eax\)
 +[a-f0-9]+:	c4 e3 79 21 c0 00    	vinsertps \$0x0,%xmm0,%xmm0,%xmm0
 +[a-f0-9]+:	c4 e3 79 21 00 00    	vinsertps \$0x0,\(%eax\),%xmm0,%xmm0
 +[a-f0-9]+:	c4 e3 79 20 c0 00    	vpinsrb \$0x0,%eax,%xmm0,%xmm0
 +[a-f0-9]+:	c4 e3 79 20 00 00    	vpinsrb \$0x0,\(%eax\),%xmm0,%xmm0
 +[a-f0-9]+:	c5 f9 c4 c0 00       	vpinsrw \$0x0,%eax,%xmm0,%xmm0
 +[a-f0-9]+:	c5 f9 c4 00 00       	vpinsrw \$0x0,\(%eax\),%xmm0,%xmm0
 +[a-f0-9]+:	c4 e1 79 c4 c0 00    	vpinsrw \$0x0,%eax,%xmm0,%xmm0
 +[a-f0-9]+:	c4 e1 79 c4 00 00    	vpinsrw \$0x0,\(%eax\),%xmm0,%xmm0
 +[a-f0-9]+:	c4 e3 79 22 c0 00    	vpinsrd \$0x0,%eax,%xmm0,%xmm0
 +[a-f0-9]+:	c4 e3 79 22 00 00    	vpinsrd \$0x0,\(%eax\),%xmm0,%xmm0
 +[a-f0-9]+:	c5 f8 ae 10          	vldmxcsr \(%eax\)
 +[a-f0-9]+:	c5 f8 ae 18          	vstmxcsr \(%eax\)
 +[a-f0-9]+:	c4 e1 78 ae 10       	vldmxcsr \(%eax\)
 +[a-f0-9]+:	c4 e1 78 ae 18       	vstmxcsr \(%eax\)
 +[a-f0-9]+:	c4 e2 78 f2 00       	andn   \(%eax\),%eax,%eax
 +[a-f0-9]+:	c4 e2 78 f7 00       	bextr  %eax,\(%eax\),%eax
 +[a-f0-9]+:	c4 e2 78 f3 18       	blsi   \(%eax\),%eax
 +[a-f0-9]+:	c4 e2 78 f3 10       	blsmsk \(%eax\),%eax
 +[a-f0-9]+:	c4 e2 78 f3 08       	blsr   \(%eax\),%eax
 +[a-f0-9]+:	c4 e2 78 f5 00       	bzhi   %eax,\(%eax\),%eax
 +[a-f0-9]+:	c4 e2 7b f6 00       	mulx   \(%eax\),%eax,%eax
 +[a-f0-9]+:	c4 e2 7b f5 00       	pdep   \(%eax\),%eax,%eax
 +[a-f0-9]+:	c4 e2 7a f5 00       	pext   \(%eax\),%eax,%eax
 +[a-f0-9]+:	c4 e3 7b f0 00 00    	rorx   \$0x0,\(%eax\),%eax
 +[a-f0-9]+:	c4 e2 7a f7 00       	sarx   %eax,\(%eax\),%eax
 +[a-f0-9]+:	c4 e2 79 f7 00       	shlx   %eax,\(%eax\),%eax
 +[a-f0-9]+:	c4 e2 7b f7 00       	shrx   %eax,\(%eax\),%eax
 +[a-f0-9]+:	8f ea 78 10 00 00 00 00 00 	bextr  \$0x0,\(%eax\),%eax
 +[a-f0-9]+:	8f e9 78 01 08       	blcfill \(%eax\),%eax
 +[a-f0-9]+:	8f e9 78 02 30       	blci   \(%eax\),%eax
 +[a-f0-9]+:	8f e9 78 01 28       	blcic  \(%eax\),%eax
 +[a-f0-9]+:	8f e9 78 02 08       	blcmsk \(%eax\),%eax
 +[a-f0-9]+:	8f e9 78 01 18       	blcs   \(%eax\),%eax
 +[a-f0-9]+:	8f e9 78 01 10       	blsfill \(%eax\),%eax
 +[a-f0-9]+:	8f e9 78 01 30       	blsic  \(%eax\),%eax
 +[a-f0-9]+:	8f e9 78 01 38       	t1mskc \(%eax\),%eax
 +[a-f0-9]+:	8f e9 78 01 20       	tzmsk  \(%eax\),%eax
#pass
