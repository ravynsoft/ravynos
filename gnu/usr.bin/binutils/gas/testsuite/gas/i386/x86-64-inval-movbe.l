.*: Assembler messages:
.*:4: Error: .*
.*:5: Error: .*
.*:6: Error: .*
.*:7: Error: .*
.*:8: Error: .*
.*:11: Error: .*
.*:12: Error: .*
.*:13: Error: .*
.*:14: Error: .*
.*:15: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\# Check illegal movbe in 64bit mode\.
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+foo:
[ 	]*4[ 	]+movbe	\(%rcx\),%bl
[ 	]*5[ 	]+movbe	%ecx,%ebx
[ 	]*6[ 	]+movbe	%bx,%rcx
[ 	]*7[ 	]+movbe	%rbx,%rcx
[ 	]*8[ 	]+movbe	%bl,\(%rcx\)
[ 	]*9[ 	]+
[ 	]*10[ 	]+\.intel_syntax noprefix
[ 	]*11[ 	]+movbe bl, byte ptr \[rcx\]
[ 	]*12[ 	]+movbe ebx, ecx
[ 	]*13[ 	]+movbe rcx, bx
[ 	]*14[ 	]+movbe rcx, rbx
[ 	]*15[ 	]+movbe byte ptr \[rcx\], bl
