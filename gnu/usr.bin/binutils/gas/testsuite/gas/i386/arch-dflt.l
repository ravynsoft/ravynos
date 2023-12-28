.*: Assembler messages:
.*:3: Error:.*`cmovl'.*
.*:9: Error:.*`cmovg'.*
GAS LISTING .*


[ 	]*[0-9]*[ 	]+\.text
[ 	]*[0-9]*[ 	]+start:
[ 	]*[0-9]*[ 	]+cmovl	%eax, %ecx
[ 	]*[0-9]*[ 	]*
[ 	]*[0-9]*[ 	]+\.arch default
[ 	]*[0-9]*[ 	]+\?\?\?\? 0F4DC8[ 	]+cmovnl	%eax, %ecx
[ 	]*[0-9]*[ 	]*
[ 	]*[0-9]*[ 	]+\.arch generic32
[ 	]*[0-9]*[ 	]+cmovg	%eax, %ecx
[ 	]*[0-9]*[ 	]*
[ 	]*[0-9]*[ 	]+\.arch default
[ 	]*[0-9]*[ 	]+\?\?\?\? 0F4EC8[ 	]+cmovng	%eax, %ecx
#pass
