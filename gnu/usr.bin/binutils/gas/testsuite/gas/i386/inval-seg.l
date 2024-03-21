.*: Assembler messages:
.*:3: Error: .*
.*:4: Error: .*
.*:7: Error: .*
.*:8: Error: .*
.*:9: Error: .*
.*:10: Error: .*
.*:11: Error: .*
GAS LISTING .*


[ 	]*[1-9][0-9]*[ 	]*\.text
[ 	]*[1-9][0-9]*[ 	]*# All the following should be illegal
[ 	]*[1-9][0-9]*[ 	]*movl	%ds,\(%eax\)
[ 	]*[1-9][0-9]*[ 	]*movl	\(%eax\),%ds
[ 	]*[1-9][0-9]*[ 	]*
[ 	]*[1-9][0-9]*[ 	]*\.intel_syntax noprefix
[ 	]*[1-9][0-9]*[ 	]*mov	eax, es:foo:\[eax\]
[ 	]*[1-9][0-9]*[ 	]*mov	eax, es:fs:foo:\[eax\]
[ 	]*[1-9][0-9]*[ 	]*mov	eax, fs:foo:bar:\[eax\]
[ 	]*[1-9][0-9]*[ 	]*mov	eax, fs:foo:gs:\[eax\]
[ 	]*[1-9][0-9]*[ 	]*mov	eax, bar:gs:\[eax\]
