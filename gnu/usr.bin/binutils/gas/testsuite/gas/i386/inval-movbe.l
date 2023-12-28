.*: Assembler messages:
.*:4: Error: .*
.*:5: Error: .*
.*:6: Error: .*
.*:7: Error: .*
.*:10: Error: .*
.*:11: Error: .*
.*:12: Error: .*
.*:13: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\# Check illegal movbe
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+foo:
[ 	]*4[ 	]+movbe	\(%ecx\),%bl
[ 	]*5[ 	]+movbe	%ecx,%ebx
[ 	]*6[ 	]+movbe	%bx,%ecx
[ 	]*7[ 	]+movbe	%bl,\(%ecx\)
[ 	]*8[ 	]+
[ 	]*9[ 	]+\.intel_syntax noprefix
[ 	]*10[ 	]+movbe bl, byte ptr \[ecx\]
[ 	]*11[ 	]+movbe ebx, ecx
[ 	]*12[ 	]+movbe ecx, bx
[ 	]*13[ 	]+movbe byte ptr \[ecx\], bl
