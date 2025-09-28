.*: Assembler messages:
.*:6: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:7: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:8: Error: `\(%rip\)' cannot be used here
.*:9: Error: .*
.*:12: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:13: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:15: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:16: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:19: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:20: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:21: Error: .* `bndcl'
.*:22: Error: .* `bndcl'
.*:25: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:26: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:27: Error: .* `bndcu'
.*:28: Error: .* `bndcu'
.*:31: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:32: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:33: Error: .* `bndcn'
.*:34: Error: .* `bndcn'
.*:37: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:38: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:39: Warning: register scaling is being ignored here
.*:40: Error: `base\(%rip\)' cannot be used here
.*:41: Error: .*
.*:44: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:45: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:46: Warning: register scaling is being ignored here
.*:47: Error: `base\(%rip\)' cannot be used here
.*:48: Error: .*
.*:51: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:52: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:53: Error: `\[rip\]' cannot be used here
.*:54: Error: .*
.*:55: Error: `\[rax\+rsp\]' is not a valid base/index expression
.*:58: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:59: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:61: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:62: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:65: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:66: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:67: Error: .* `bndcl'
.*:68: Error: .* `bndcl'
.*:71: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:72: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:73: Error: .* `bndcu'
.*:74: Error: .* `bndcu'
.*:77: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:78: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:79: Error: .* `bndcn'
.*:80: Error: .* `bndcn'
.*:83: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:84: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:85: Warning: register scaling is being ignored here
.*:86: Error: `\[rip\+base\]' cannot be used here
.*:87: Error: .*
.*:88: Error: `\[rax\+rsp\]' is not a valid base/index expression
.*:91: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:92: Error: 32-bit address isn't allowed in 64-bit MPX instructions.
.*:93: Warning: register scaling is being ignored here
.*:94: Error: `\[rip\+base\]' cannot be used here
.*:95: Error: .*
.*:96: Error: `\[rax\+rsp\]' is not a valid base/index expression
GAS LISTING .*


[ 	]*1[ 	]+\# MPX instructions
[ 	]*2[ 	]+\.allow_index_reg
[ 	]*3[ 	]+\.text
[ 	]*4[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\#\#\# bndmk
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67F30F1B 		bndmk \(%eax\), %bnd1
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+08
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67F30F1B 		bndmk 0x3\(%ecx,%ebx,1\), %bnd1
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+4C1903
[ 	]*[1-9][0-9]*[ 	]+bndmk \(%rip\), %bnd3
[ 	]*[1-9][0-9]*[ 	]+bndmk \(%eip\), %bnd2
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\#\#\# bndmov
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 6766410F 		bndmov \(%r8d\), %bnd1
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+1A08
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 6766410F 		bndmov 0x3\(%r9d,%edx,1\), %bnd1
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+1A4C1103 
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67660F1B 		bndmov %bnd1, \(%eax\)
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+08
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67660F1B 		bndmov %bnd1, 0x3\(%ecx,%eax,1\)
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+4C0103
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\#\#\# bndcl
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67F30F1A 		bndcl \(%ecx\), %bnd1
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+09
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67F30F1A 		bndcl 0x3\(%ecx,%eax,1\), %bnd1
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+4C0103
[ 	]*[1-9][0-9]*[ 	]+bndcl %ecx, %bnd1
[ 	]*[1-9][0-9]*[ 	]+bndcl %cx, %bnd1
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\#\#\# bndcu
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67F20F1A 		bndcu \(%ecx\), %bnd1
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+09
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67F20F1A 		bndcu 0x3\(%ecx,%eax,1\), %bnd1
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+4C0103
[ 	]*[1-9][0-9]*[ 	]+bndcu %ecx, %bnd1
[ 	]*[1-9][0-9]*[ 	]+bndcu %cx, %bnd1
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\#\#\# bndcn
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67F20F1B 		bndcn \(%ecx\), %bnd1
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+09
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67F20F1B 		bndcn 0x3\(%ecx,%eax,1\), %bnd1
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+4C0103
[ 	]*[1-9][0-9]*[ 	]+bndcn %ecx, %bnd1
GAS LISTING .*


[ 	]*[1-9][0-9]*[ 	]+bndcn %cx, %bnd1
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\#\#\# bndstx
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 670F1B44 		bndstx %bnd0, 0x3\(%eax,%ebx,1\)
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+1803
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 670F1B53 		bndstx %bnd2, 3\(%ebx,1\)
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+03
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 410F1B0C 		bndstx %bnd1, \(%r15,%rax,2\)
.*  Warning: register scaling is being ignored here
[ 	]*[1-9][0-9]*[ 	]+47
[ 	]*[1-9][0-9]*[ 	]+bndstx %bnd3, base\(%rip\)
[ 	]*[1-9][0-9]*[ 	]+bndstx %bnd1, base\(%eip\)
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\#\#\# bndldx
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 670F1A44 		bndldx 0x3\(%eax,%ebx,1\), %bnd0
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+1803
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 670F1A53 		bndldx 3\(%ebx,1\), %bnd2
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+03
[ 	]*[1-9][0-9]*[ 	]*\?\?\?\? 420F1A1C 		bndldx \(%rax,%r15,4\), %bnd3
.*  Warning: register scaling is being ignored here
[ 	]*[1-9][0-9]*[ 	]*B8
[ 	]*[1-9][0-9]*[ 	]*bndldx base\(%rip\), %bnd1
[ 	]*[1-9][0-9]*[ 	]*bndldx base\(%eip\), %bnd3
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\.intel_syntax noprefix
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67F30F1B 		bndmk bnd1, \[eax\]
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+08
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67F30F1B 		bndmk bnd1, \[edx\+1\*eax\+0x3\]
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+4C0203
[ 	]*[1-9][0-9]*[ 	]*bndmk bnd3, \[rip\]
[ 	]*[1-9][0-9]*[ 	]*bndmk bnd2, \[eip\]
[ 	]*[1-9][0-9]*[ 	]+bndmk bnd2, \[rax\+rsp\]
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\#\#\# bndmov
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67660F1A 		bndmov bnd1, \[eax\]
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+08
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67660F1A 		bndmov bnd1, \[edx\+1\*eax\+0x3\]
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+4C0203
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67660F1B 		bndmov \[eax\], bnd1
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+08
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67660F1B 		bndmov \[edx\+1\*eax\+0x3\], bnd1
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+4C0203
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\#\#\# bndcl
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67F30F1A 		bndcl bnd1, \[eax\]
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
GAS LISTING .*


[ 	]*[1-9][0-9]*[ 	]+08
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67F30F1A 		bndcl bnd1, \[edx\+1\*eax\+0x3\]
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+4C0203
[ 	]*[1-9][0-9]*[ 	]+bndcl bnd1, eax
[ 	]*[1-9][0-9]*[ 	]+bndcl bnd1, dx
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\#\#\# bndcu
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67F20F1A 		bndcu bnd1, \[eax\]
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+08
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67F20F1A 		bndcu bnd1, \[edx\+1\*eax\+0x3\]
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+4C0203
[ 	]*[1-9][0-9]*[ 	]+bndcu bnd1, eax
[ 	]*[1-9][0-9]*[ 	]+bndcu bnd1, dx
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\#\#\# bndcn
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67F20F1B 		bndcn bnd1, \[eax\]
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+08
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 67F20F1B 		bndcn bnd1, \[edx\+1\*eax\+0x3\]
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+4C0203
[ 	]*[1-9][0-9]*[ 	]+bndcn bnd1, eax
[ 	]*[1-9][0-9]*[ 	]+bndcn bnd1, dx
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\#\#\# bndstx
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 670F1B44 		bndstx \[eax\+ebx\*1\+0x3\], bnd0
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+1803
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 670F1B14 		bndstx \[1\*ebx\+3\], bnd2
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+1D030000 
[ 	]*[1-9][0-9]*[ 	]+00
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 410F1B14 		bndstx \[r8\+rdi\*4\], bnd2
.*  Warning: register scaling is being ignored here
[ 	]*[1-9][0-9]*[ 	]+B8
[ 	]*[1-9][0-9]*[ 	]+bndstx \[rip\+base\], bnd1
[ 	]*[1-9][0-9]*[ 	]+bndstx \[eip\+base\], bnd3
[ 	]*[1-9][0-9]*[ 	]+bndstx \[rax\+rsp\], bnd3
[ 	]*[1-9][0-9]*[ 	]+
[ 	]*[1-9][0-9]*[ 	]+\#\#\# bndldx
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 670F1A44 		bndldx bnd0, \[eax\+ebx\*1\+0x3\]
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+1803
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 670F1A14 		bndldx bnd2, \[1\*ebx\+3\]
.*  Error: 32-bit address isn't allowed in 64-bit MPX instructions\.
[ 	]*[1-9][0-9]*[ 	]+1D030000 
[ 	]*[1-9][0-9]*[ 	]+00
[ 	]*[1-9][0-9]*[ 	]+\?\?\?\? 420F1A14 		bndldx bnd2, \[rdi\+r8\*8\]
.*  Warning: register scaling is being ignored here
[ 	]*[1-9][0-9]*[ 	]+C7
[ 	]*[1-9][0-9]*[ 	]+bndldx bnd1, \[rip\+base\]
[ 	]*[1-9][0-9]*[ 	]+bndldx bnd3, \[eip\+base\]
[ 	]*[1-9][0-9]*[ 	]+bndldx bnd3, \[rax\+rsp\]
#pass
