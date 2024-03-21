.*: Assembler messages:
.*:3: Error:.*`cmovl'.*
.*:10: Error:.*`cmovg'.*
.*:17: Error:.*`cmovz'.*
.*:21: Error:.*`\.arch pop'.*`\.code32'.*
.*:28: Error:.*`\.arch pop'.*`\.code16gcc'.*
.*:32: Error:.*\.arch.*empty.*
GAS LISTING .*


[ 	]*[0-9]*[ 	]+\.text
[ 	]*[0-9]*[ 	]+start:
[ 	]*[0-9]*[ 	]+cmovl	%eax, %ecx
[ 	]*[0-9]*[ 	]*
[ 	]*[0-9]*[ 	]+\.arch push
[ 	]*[0-9]*[ 	]+\.arch default
[ 	]*[0-9]*[ 	]+\?\?\?\? 0F4DC8[ 	]+cmovnl	%eax, %ecx
[ 	]*[0-9]*[ 	]*
[ 	]*[0-9]*[ 	]+\.arch pop
[ 	]*[0-9]*[ 	]+cmovg	%eax, %ecx
[ 	]*[0-9]*[ 	]*
[ 	]*[0-9]*[ 	]+\.arch push
[ 	]*[0-9]*[ 	]+\.arch \.cmov
[ 	]*[0-9]*[ 	]+\?\?\?\? 0F4EC8[ 	]+cmovng	%eax, %ecx
[ 	]*[0-9]*[ 	]*
[ 	]*[0-9]*[ 	]+\.arch pop
[ 	]*[0-9]*[ 	]+cmovz	%eax, %ecx
[ 	]*[0-9]*[ 	]*
[ 	]*[0-9]*[ 	]+\.arch push
[ 	]*[0-9]*[ 	]+\.code16
[ 	]*[0-9]*[ 	]+\.arch pop
[ 	]*[0-9]*[ 	]+\.code32
[ 	]*[0-9]*[ 	]+\.arch pop
[ 	]*[0-9]*[ 	]*
[ 	]*[0-9]*[ 	]+\.code16gcc
[ 	]*[0-9]*[ 	]+\.arch push
[ 	]*[0-9]*[ 	]+\.code32
[ 	]*[0-9]*[ 	]+\.arch pop
[ 	]*[0-9]*[ 	]+\.code16gcc
[ 	]*[0-9]*[ 	]+\.arch pop
[ 	]*[0-9]*[ 	]*
[ 	]*[0-9]*[ 	]+\.arch pop
#pass
