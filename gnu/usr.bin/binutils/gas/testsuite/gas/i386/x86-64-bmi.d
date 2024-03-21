#as:
#objdump: -dw
#name: x86-64 BMI insns

.*: +file format .*


Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	66 f3 0f bc d8       	tzcnt  %ax,%bx
[ 	]*[a-f0-9]+:	66 f3 0f bc 19       	tzcnt  \(%rcx\),%bx
[ 	]*[a-f0-9]+:	66 f3 44 0f bc 39    	tzcnt  \(%rcx\),%r15w
[ 	]*[a-f0-9]+:	c4 e2 60 f2 f0       	andn   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 60 f2 31       	andn   \(%rcx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 42 00 f2 d1       	andn   %r9d,%r15d,%r10d
[ 	]*[a-f0-9]+:	c4 62 00 f2 11       	andn   \(%rcx\),%r15d,%r10d
[ 	]*[a-f0-9]+:	c4 e2 78 f7 f3       	bextr  %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 60 f7 31       	bextr  %ebx,\(%rcx\),%esi
[ 	]*[a-f0-9]+:	c4 42 30 f7 d7       	bextr  %r9d,%r15d,%r10d
[ 	]*[a-f0-9]+:	c4 62 30 f7 11       	bextr  %r9d,\(%rcx\),%r10d
[ 	]*[a-f0-9]+:	f3 0f bc d8          	tzcnt  %eax,%ebx
[ 	]*[a-f0-9]+:	f3 0f bc 19          	tzcnt  \(%rcx\),%ebx
[ 	]*[a-f0-9]+:	f3 44 0f bc 39       	tzcnt  \(%rcx\),%r15d
[ 	]*[a-f0-9]+:	c4 e2 60 f3 d8       	blsi   %eax,%ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f3 19       	blsi   \(%rcx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 00 f3 19       	blsi   \(%rcx\),%r15d
[ 	]*[a-f0-9]+:	c4 e2 60 f3 d0       	blsmsk %eax,%ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f3 11       	blsmsk \(%rcx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 00 f3 11       	blsmsk \(%rcx\),%r15d
[ 	]*[a-f0-9]+:	c4 e2 60 f3 c8       	blsr   %eax,%ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f3 09       	blsr   \(%rcx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 00 f3 09       	blsr   \(%rcx\),%r15d
[ 	]*[a-f0-9]+:	c4 e2 e0 f2 f0       	andn   %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 e0 f2 31       	andn   \(%rcx\),%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 42 80 f2 d1       	andn   %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 80 f2 11       	andn   \(%rcx\),%r15,%r10
[ 	]*[a-f0-9]+:	c4 e2 f8 f7 f3       	bextr  %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 f8 f7 31       	bextr  %rax,\(%rcx\),%rsi
[ 	]*[a-f0-9]+:	c4 42 b0 f7 d7       	bextr  %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 b0 f7 11       	bextr  %r9,\(%rcx\),%r10
[ 	]*[a-f0-9]+:	f3 48 0f bc d8       	tzcnt  %rax,%rbx
[ 	]*[a-f0-9]+:	f3 48 0f bc 19       	tzcnt  \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	f3 4d 0f bc f9       	tzcnt  %r9,%r15
[ 	]*[a-f0-9]+:	f3 4c 0f bc 39       	tzcnt  \(%rcx\),%r15
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 d8       	blsi   %rax,%rbx
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 19       	blsi   \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	c4 c2 80 f3 d9       	blsi   %r9,%r15
[ 	]*[a-f0-9]+:	c4 e2 80 f3 19       	blsi   \(%rcx\),%r15
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 d0       	blsmsk %rax,%rbx
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 11       	blsmsk \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	c4 c2 80 f3 d1       	blsmsk %r9,%r15
[ 	]*[a-f0-9]+:	c4 e2 80 f3 11       	blsmsk \(%rcx\),%r15
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 c8       	blsr   %rax,%rbx
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 09       	blsr   \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	c4 c2 80 f3 c9       	blsr   %r9,%r15
[ 	]*[a-f0-9]+:	c4 e2 80 f3 09       	blsr   \(%rcx\),%r15
[ 	]*[a-f0-9]+:	66 f3 0f bc d8       	tzcnt  %ax,%bx
[ 	]*[a-f0-9]+:	66 f3 0f bc 19       	tzcnt  \(%rcx\),%bx
[ 	]*[a-f0-9]+:	66 f3 44 0f bc 11    	tzcnt  \(%rcx\),%r10w
[ 	]*[a-f0-9]+:	66 f3 0f bc 19       	tzcnt  \(%rcx\),%bx
[ 	]*[a-f0-9]+:	c4 e2 60 f2 f0       	andn   %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 60 f2 31       	andn   \(%rcx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 42 28 f2 f9       	andn   %r9d,%r10d,%r15d
[ 	]*[a-f0-9]+:	c4 62 28 f2 39       	andn   \(%rcx\),%r10d,%r15d
[ 	]*[a-f0-9]+:	c4 e2 60 f2 31       	andn   \(%rcx\),%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 78 f7 f3       	bextr  %eax,%ebx,%esi
[ 	]*[a-f0-9]+:	c4 e2 60 f7 31       	bextr  %ebx,\(%rcx\),%esi
[ 	]*[a-f0-9]+:	c4 42 30 f7 fa       	bextr  %r9d,%r10d,%r15d
[ 	]*[a-f0-9]+:	c4 62 30 f7 39       	bextr  %r9d,\(%rcx\),%r15d
[ 	]*[a-f0-9]+:	c4 e2 60 f7 31       	bextr  %ebx,\(%rcx\),%esi
[ 	]*[a-f0-9]+:	f3 0f bc d8          	tzcnt  %eax,%ebx
[ 	]*[a-f0-9]+:	f3 0f bc 19          	tzcnt  \(%rcx\),%ebx
[ 	]*[a-f0-9]+:	f3 44 0f bc 11       	tzcnt  \(%rcx\),%r10d
[ 	]*[a-f0-9]+:	f3 0f bc 19          	tzcnt  \(%rcx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f3 d8       	blsi   %eax,%ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f3 19       	blsi   \(%rcx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 28 f3 19       	blsi   \(%rcx\),%r10d
[ 	]*[a-f0-9]+:	c4 e2 60 f3 19       	blsi   \(%rcx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f3 d0       	blsmsk %eax,%ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f3 11       	blsmsk \(%rcx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 28 f3 11       	blsmsk \(%rcx\),%r10d
[ 	]*[a-f0-9]+:	c4 e2 60 f3 11       	blsmsk \(%rcx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f3 c8       	blsr   %eax,%ebx
[ 	]*[a-f0-9]+:	c4 e2 60 f3 09       	blsr   \(%rcx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 28 f3 09       	blsr   \(%rcx\),%r10d
[ 	]*[a-f0-9]+:	c4 e2 60 f3 09       	blsr   \(%rcx\),%ebx
[ 	]*[a-f0-9]+:	c4 e2 e0 f2 f0       	andn   %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 e0 f2 31       	andn   \(%rcx\),%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 42 80 f2 d1       	andn   %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 80 f2 11       	andn   \(%rcx\),%r15,%r10
[ 	]*[a-f0-9]+:	c4 e2 e0 f2 31       	andn   \(%rcx\),%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 f8 f7 f3       	bextr  %rax,%rbx,%rsi
[ 	]*[a-f0-9]+:	c4 e2 f8 f7 31       	bextr  %rax,\(%rcx\),%rsi
[ 	]*[a-f0-9]+:	c4 42 b0 f7 d7       	bextr  %r9,%r15,%r10
[ 	]*[a-f0-9]+:	c4 62 b0 f7 11       	bextr  %r9,\(%rcx\),%r10
[ 	]*[a-f0-9]+:	c4 e2 f8 f7 31       	bextr  %rax,\(%rcx\),%rsi
[ 	]*[a-f0-9]+:	f3 48 0f bc d8       	tzcnt  %rax,%rbx
[ 	]*[a-f0-9]+:	f3 48 0f bc 19       	tzcnt  \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	f3 4d 0f bc f9       	tzcnt  %r9,%r15
[ 	]*[a-f0-9]+:	f3 4c 0f bc 39       	tzcnt  \(%rcx\),%r15
[ 	]*[a-f0-9]+:	f3 48 0f bc 19       	tzcnt  \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 d8       	blsi   %rax,%rbx
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 19       	blsi   \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	c4 c2 80 f3 d9       	blsi   %r9,%r15
[ 	]*[a-f0-9]+:	c4 e2 80 f3 19       	blsi   \(%rcx\),%r15
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 19       	blsi   \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 d0       	blsmsk %rax,%rbx
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 11       	blsmsk \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	c4 c2 80 f3 d1       	blsmsk %r9,%r15
[ 	]*[a-f0-9]+:	c4 e2 80 f3 11       	blsmsk \(%rcx\),%r15
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 11       	blsmsk \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 c8       	blsr   %rax,%rbx
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 09       	blsr   \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	c4 c2 80 f3 c9       	blsr   %r9,%r15
[ 	]*[a-f0-9]+:	c4 e2 80 f3 09       	blsr   \(%rcx\),%r15
[ 	]*[a-f0-9]+:	c4 e2 e0 f3 09       	blsr   \(%rcx\),%rbx
#pass
