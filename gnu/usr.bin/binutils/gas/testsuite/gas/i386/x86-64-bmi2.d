#as:
#objdump: -dw
#name: x86-64 BMI2 insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	c4 e3 7b f0 d8 07    	rorx   \$0x7,%eax,%ebx
[ 	]*[a-f0-9]+:	c4 e3 7b f0 19 07    	rorx   \$0x7,\(%rcx\),%ebx
[ 	]*[a-f0-9]+:	c4 43 7b f0 f9 07    	rorx   \$0x7,%r9d,%r15d
[ 	]*[a-f0-9]+:	c4 63 7b f0 39 07    	rorx   \$0x7,\(%rcx\),%r15d
[ 	]*[a-f0-9]+:	c4 e2 63 f6 f0       	mulx   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 63 f6 31       	mulx   \(%rcx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 42 03 f6 d1       	mulx   %r9d,%r15d,%r10d
[ 	]*[a-f0-9]+:	c4 62 03 f6 11       	mulx   \(%rcx\),%r15d,%r10d
[ 	]*[a-f0-9]+:	c4 e2 63 f5 f0       	pdep   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 63 f5 31       	pdep   \(%rcx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 42 03 f5 d1       	pdep   %r9d,%r15d,%r10d
[ 	]*[a-f0-9]+:	c4 62 03 f5 11       	pdep   \(%rcx\),%r15d,%r10d
[ 	]*[a-f0-9]+:	c4 e2 62 f5 f0       	pext   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 62 f5 31       	pext   \(%rcx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 42 02 f5 d1       	pext   %r9d,%r15d,%r10d
[ 	]*[a-f0-9]+:	c4 62 02 f5 11       	pext   \(%rcx\),%r15d,%r10d
[ 	]*[a-f0-9]+:	c4 e2 78 f5 f3       	bzhi   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 60 f5 31       	bzhi   %ebx,\(%rcx\),%esi
[ 	]*[a-f0-9]+:	c4 42 30 f5 d7       	bzhi   %r9d,%r15d,%r10d
[ 	]*[a-f0-9]+:	c4 62 30 f5 11       	bzhi   %r9d,\(%rcx\),%r10d
[ 	]*[a-f0-9]+:	c4 e2 7a f7 f3       	sarx   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 62 f7 31       	sarx   %ebx,\(%rcx\),%esi
[ 	]*[a-f0-9]+:	c4 42 32 f7 d7       	sarx   %r9d,%r15d,%r10d
[ 	]*[a-f0-9]+:	c4 62 32 f7 11       	sarx   %r9d,\(%rcx\),%r10d
[ 	]*[a-f0-9]+:	c4 e2 79 f7 f3       	shlx   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 61 f7 31       	shlx   %ebx,\(%rcx\),%esi
[ 	]*[a-f0-9]+:	c4 42 31 f7 d7       	shlx   %r9d,%r15d,%r10d
[ 	]*[a-f0-9]+:	c4 62 31 f7 11       	shlx   %r9d,\(%rcx\),%r10d
[ 	]*[a-f0-9]+:	c4 e2 7b f7 f3       	shrx   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 63 f7 31       	shrx   %ebx,\(%rcx\),%esi
[ 	]*[a-f0-9]+:	c4 42 33 f7 d7       	shrx   %r9d,%r15d,%r10d
[ 	]*[a-f0-9]+:	c4 62 33 f7 11       	shrx   %r9d,\(%rcx\),%r10d
[ 	]*[a-f0-9]+:	c4 e3 fb f0 d8 07    	rorx   \$0x7,%rax,%rbx
[ 	]*[a-f0-9]+:	c4 e3 fb f0 19 07    	rorx   \$0x7,\(%rcx\),%rbx
[ 	]*[a-f0-9]+:	c4 43 fb f0 f9 07    	rorx   \$0x7,%r9,%r15
[ 	]*[a-f0-9]+:	c4 63 fb f0 39 07    	rorx   \$0x7,\(%rcx\),%r15
[ 	]*[a-f0-9]+:	c4 e2 e3 f6 f0       	mulx   %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 e3 f6 31       	mulx   \(%rcx\),%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 42 83 f6 d1       	mulx   %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 83 f6 11       	mulx   \(%rcx\),%r15,%r10
[ 	]*[a-f0-9]+:	c4 e2 e3 f5 f0       	pdep   %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 e3 f5 31       	pdep   \(%rcx\),%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 42 83 f5 d1       	pdep   %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 83 f5 11       	pdep   \(%rcx\),%r15,%r10
[ 	]*[a-f0-9]+:	c4 e2 e2 f5 f0       	pext   %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 e2 f5 31       	pext   \(%rcx\),%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 42 82 f5 d1       	pext   %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 82 f5 11       	pext   \(%rcx\),%r15,%r10
[ 	]*[a-f0-9]+:	c4 e2 f8 f5 f3       	bzhi   %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 f8 f5 31       	bzhi   %rax,\(%rcx\),%rsi
[ 	]*[a-f0-9]+:	c4 42 b0 f5 d7       	bzhi   %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 b0 f5 11       	bzhi   %r9,\(%rcx\),%r10
[ 	]*[a-f0-9]+:	c4 e2 fa f7 f3       	sarx   %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 fa f7 31       	sarx   %rax,\(%rcx\),%rsi
[ 	]*[a-f0-9]+:	c4 42 b2 f7 d7       	sarx   %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 b2 f7 11       	sarx   %r9,\(%rcx\),%r10
[ 	]*[a-f0-9]+:	c4 e2 f9 f7 f3       	shlx   %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 f9 f7 31       	shlx   %rax,\(%rcx\),%rsi
[ 	]*[a-f0-9]+:	c4 42 b1 f7 d7       	shlx   %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 b1 f7 11       	shlx   %r9,\(%rcx\),%r10
[ 	]*[a-f0-9]+:	c4 e2 fb f7 f3       	shrx   %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 fb f7 31       	shrx   %rax,\(%rcx\),%rsi
[ 	]*[a-f0-9]+:	c4 42 b3 f7 d7       	shrx   %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 b3 f7 11       	shrx   %r9,\(%rcx\),%r10
[ 	]*[a-f0-9]+:	c4 e3 7b f0 d8 07    	rorx   \$0x7,%eax,%ebx
[ 	]*[a-f0-9]+:	c4 e3 7b f0 19 07    	rorx   \$0x7,\(%rcx\),%ebx
[ 	]*[a-f0-9]+:	c4 43 7b f0 d1 07    	rorx   \$0x7,%r9d,%r10d
[ 	]*[a-f0-9]+:	c4 63 7b f0 11 07    	rorx   \$0x7,\(%rcx\),%r10d
[ 	]*[a-f0-9]+:	c4 e3 7b f0 19 07    	rorx   \$0x7,\(%rcx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 63 f6 f0       	mulx   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 63 f6 31       	mulx   \(%rcx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 42 2b f6 f9       	mulx   %r9d,%r10d,%r15d
[ 	]*[a-f0-9]+:	c4 62 2b f6 39       	mulx   \(%rcx\),%r10d,%r15d
[ 	]*[a-f0-9]+:	c4 e2 63 f6 31       	mulx   \(%rcx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 63 f5 f0       	pdep   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 63 f5 31       	pdep   \(%rcx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 42 2b f5 f9       	pdep   %r9d,%r10d,%r15d
[ 	]*[a-f0-9]+:	c4 62 2b f5 39       	pdep   \(%rcx\),%r10d,%r15d
[ 	]*[a-f0-9]+:	c4 e2 63 f5 31       	pdep   \(%rcx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 62 f5 f0       	pext   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 62 f5 31       	pext   \(%rcx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 42 2a f5 f9       	pext   %r9d,%r10d,%r15d
[ 	]*[a-f0-9]+:	c4 62 2a f5 39       	pext   \(%rcx\),%r10d,%r15d
[ 	]*[a-f0-9]+:	c4 e2 62 f5 31       	pext   \(%rcx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 78 f5 f3       	bzhi   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 60 f5 31       	bzhi   %ebx,\(%rcx\),%esi
[ 	]*[a-f0-9]+:	c4 42 30 f5 fa       	bzhi   %r9d,%r10d,%r15d
[ 	]*[a-f0-9]+:	c4 62 30 f5 39       	bzhi   %r9d,\(%rcx\),%r15d
[ 	]*[a-f0-9]+:	c4 e2 60 f5 31       	bzhi   %ebx,\(%rcx\),%esi
[ 	]*[a-f0-9]+:	c4 e2 7a f7 f3       	sarx   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 62 f7 31       	sarx   %ebx,\(%rcx\),%esi
[ 	]*[a-f0-9]+:	c4 42 32 f7 fa       	sarx   %r9d,%r10d,%r15d
[ 	]*[a-f0-9]+:	c4 62 32 f7 39       	sarx   %r9d,\(%rcx\),%r15d
[ 	]*[a-f0-9]+:	c4 e2 62 f7 31       	sarx   %ebx,\(%rcx\),%esi
[ 	]*[a-f0-9]+:	c4 e2 79 f7 f3       	shlx   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 61 f7 31       	shlx   %ebx,\(%rcx\),%esi
[ 	]*[a-f0-9]+:	c4 42 31 f7 fa       	shlx   %r9d,%r10d,%r15d
[ 	]*[a-f0-9]+:	c4 62 31 f7 39       	shlx   %r9d,\(%rcx\),%r15d
[ 	]*[a-f0-9]+:	c4 e2 61 f7 31       	shlx   %ebx,\(%rcx\),%esi
[ 	]*[a-f0-9]+:	c4 e2 7b f7 f3       	shrx   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 63 f7 31       	shrx   %ebx,\(%rcx\),%esi
[ 	]*[a-f0-9]+:	c4 42 33 f7 fa       	shrx   %r9d,%r10d,%r15d
[ 	]*[a-f0-9]+:	c4 62 33 f7 39       	shrx   %r9d,\(%rcx\),%r15d
[ 	]*[a-f0-9]+:	c4 e2 63 f7 31       	shrx   %ebx,\(%rcx\),%esi
[ 	]*[a-f0-9]+:	c4 e3 fb f0 d8 07    	rorx   \$0x7,%rax,%rbx
[ 	]*[a-f0-9]+:	c4 e3 fb f0 19 07    	rorx   \$0x7,\(%rcx\),%rbx
[ 	]*[a-f0-9]+:	c4 43 fb f0 f9 07    	rorx   \$0x7,%r9,%r15
[ 	]*[a-f0-9]+:	c4 63 fb f0 39 07    	rorx   \$0x7,\(%rcx\),%r15
[ 	]*[a-f0-9]+:	c4 e3 fb f0 19 07    	rorx   \$0x7,\(%rcx\),%rbx
[ 	]*[a-f0-9]+:	c4 e2 e3 f6 f0       	mulx   %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 e3 f6 31       	mulx   \(%rcx\),%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 42 83 f6 d1       	mulx   %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 83 f6 11       	mulx   \(%rcx\),%r15,%r10
[ 	]*[a-f0-9]+:	c4 e2 e3 f6 31       	mulx   \(%rcx\),%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 e3 f5 f0       	pdep   %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 e3 f5 31       	pdep   \(%rcx\),%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 42 83 f5 d1       	pdep   %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 83 f5 11       	pdep   \(%rcx\),%r15,%r10
[ 	]*[a-f0-9]+:	c4 e2 e3 f5 31       	pdep   \(%rcx\),%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 e2 f5 f0       	pext   %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 e2 f5 31       	pext   \(%rcx\),%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 42 82 f5 d1       	pext   %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 82 f5 11       	pext   \(%rcx\),%r15,%r10
[ 	]*[a-f0-9]+:	c4 e2 e2 f5 31       	pext   \(%rcx\),%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 f8 f5 f3       	bzhi   %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 f8 f5 31       	bzhi   %rax,\(%rcx\),%rsi
[ 	]*[a-f0-9]+:	c4 42 b0 f5 d7       	bzhi   %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 b0 f5 11       	bzhi   %r9,\(%rcx\),%r10
[ 	]*[a-f0-9]+:	c4 e2 f8 f5 31       	bzhi   %rax,\(%rcx\),%rsi
[ 	]*[a-f0-9]+:	c4 e2 fa f7 f3       	sarx   %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 fa f7 31       	sarx   %rax,\(%rcx\),%rsi
[ 	]*[a-f0-9]+:	c4 42 b2 f7 d7       	sarx   %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 b2 f7 11       	sarx   %r9,\(%rcx\),%r10
[ 	]*[a-f0-9]+:	c4 e2 fa f7 31       	sarx   %rax,\(%rcx\),%rsi
[ 	]*[a-f0-9]+:	c4 e2 f9 f7 f3       	shlx   %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 f9 f7 31       	shlx   %rax,\(%rcx\),%rsi
[ 	]*[a-f0-9]+:	c4 42 b1 f7 d7       	shlx   %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 b1 f7 11       	shlx   %r9,\(%rcx\),%r10
[ 	]*[a-f0-9]+:	c4 e2 f9 f7 31       	shlx   %rax,\(%rcx\),%rsi
[ 	]*[a-f0-9]+:	c4 e2 fb f7 f3       	shrx   %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 fb f7 31       	shrx   %rax,\(%rcx\),%rsi
[ 	]*[a-f0-9]+:	c4 42 b3 f7 d7       	shrx   %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 b3 f7 11       	shrx   %r9,\(%rcx\),%r10
[ 	]*[a-f0-9]+:	c4 e2 fb f7 31       	shrx   %rax,\(%rcx\),%rsi
#pass
