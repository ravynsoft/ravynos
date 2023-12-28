.*: Assembler messages:
.*:4: Error: .*
.*:5: Error: .*
.*:7: Error: .*
.*:8: Error: .*
.*:10: Error: .*
.*:11: Error: .*
.*:13: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\# Invalid Intel MCU instructions / directives
[ 	]*2[ 	]+\.text
[ 	]*3[ 	]+
[ 	]*4[ 	]+fnstsw
[ 	]*5[ 	]+fstsw	%ax
[ 	]*6[ 	]+
[ 	]*7[ 	]+cmove	%eax,%ebx
[ 	]*8[ 	]+nopw	\(%eax\)
[ 	]*9[ 	]+
[ 	]*10[ 	]+movq	%xmm1, \(%eax\)
[ 	]*11[ 	]+movnti	%eax, \(%eax\)
[ 	]*12[ 	]+
[ 	]*13[ 	]+\.arch generic32
