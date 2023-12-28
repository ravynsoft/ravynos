.*: Assembler messages:
.*:6: Error: invalid .* `addss' after `repe'
.*:7: Error: invalid .* `addss' after `repne'
.*:8: Error: invalid .* `vaddss' after `repe'
.*:9: Error: invalid .* `vaddss' after `repne'
.*:14: Error: same type of prefix .*
.*:15: Error: same type of prefix .*
.*:18: Error: data size .* `addps'
.*:19: Error: data size .* `addpd'
.*:20: Error: data size .* `vaddps'
.*:21: Error: data size .* `vaddpd'
.*:25: Error: same type of prefix .*
GAS LISTING .*
#...
[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+prefix:
[ 	]*3[ 	]+\?\?\?\? 67E203[ 	]+addr16 loop	\.Lrep_ret
[ 	]*4[ 	]+\?\?\?\? 67E304[ 	]+addr16 jecxz	\.Ldata16
[ 	]*5[ 	]*
[ 	]*6[ 	]+repe addss	%xmm0, %xmm0
[ 	]*7[ 	]+repne addss	%xmm0, %xmm0
[ 	]*8[ 	]+repe vaddss	%xmm0, %xmm0, %xmm0
[ 	]*9[ 	]+repne vaddss	%xmm0, %xmm0, %xmm0
[ 	]*10[ 	]*
[ 	]*11[ 	]+\.Lrep_ret:
[ 	]*12[ 	]+\?\?\?\? F2C3[ 	]+bnd ret
[ 	]*13[ 	]+\?\?\?\? F3C3[ 	]+rep ret
[ 	]*14[ 	]+bnd rep ret
[ 	]*15[ 	]+rep bnd ret
[ 	]*16[ 	]*
[ 	]*17[ 	]+\.Ldata16:
[ 	]*18[ 	]+data16 addps	%xmm0, %xmm0
[ 	]*19[ 	]+data16 addpd	%xmm0, %xmm0
[ 	]*20[ 	]+data16 vaddps	%xmm0, %xmm0, %xmm0
[ 	]*21[ 	]+data16 vaddpd	%xmm0, %xmm0, %xmm0
[ 	]*22[ 	]*
[ 	]*23[ 	]+\.Lsegment:
[ 	]*24[ 	]+\?\?\?\? 368B4500[ 	]+ss mov		%ss:\(%ebp\), %eax
[ 	]*25[ 	]+ss mov		%ds:\(%ebp\), %eax
[ 	]*26[ 	]+\?\?\?\? 3E8B4500[ 	]+ds mov		%ss:\(%ebp\), %eax
[ 	]*27[ 	]+\?\?\?\? 3E8B4500[ 	]+ds mov		%ds:\(%ebp\), %eax
[ 	]*28[ 	]*
#pass
