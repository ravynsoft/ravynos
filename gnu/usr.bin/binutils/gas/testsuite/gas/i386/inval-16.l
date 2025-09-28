.*: Assembler messages:
.*:3: Error: .*
.*:4: Error: .*
.*:5: Error: .*
.*:6: Error: .*
.*:7: Error: .*
.*:8: Error: .*
.*:9: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+\.arch i186; \.code16
[ 	]*3[ 	]+vmovapd	%xmm0,%xmm1
[ 	]*4[ 	]+vaddsd	%xmm4, %xmm5, %xmm6\{%k7\}
[ 	]*5[ 	]+vfrczpd	%xmm7,%xmm7
[ 	]*6[ 	]+andn    \(%eax\), %ecx, %ecx
[ 	]*7[ 	]+bzhi    %ecx, \(%eax\), %ecx
[ 	]*8[ 	]+llwpcb	%ecx
[ 	]*9[ 	]+blcfill	%ecx, %ecx
