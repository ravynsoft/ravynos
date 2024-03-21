.*: Assembler messages:
.*:6: Error: expecting indirect branch instruction after `notrack'
.*:6: Warning: skipping prefixes on `call'
.*:7: Error: expecting indirect branch instruction after `notrack'
.*:9: Error: same type of prefix used twice
.*:10: Error: same type of prefix used twice
.*:13: Error: same type of prefix used twice
.*:14: Error: same type of prefix used twice
GAS LISTING .*


[ 	]*1[ 	]+\# Check 64bit unsupported NOTRACK prefix
[ 	]*2[ 	]+
[ 	]*3[ 	]+\.allow_index_reg
[ 	]*4[ 	]+\.text
[ 	]*5[ 	]+_start:
[ 	]*6[ 	]+\?\?\?\? [0-9A-F]* 		notrack call foo
\*\*\*\*  Error: expecting indirect branch instruction after `notrack'
\*\*\*\*  Warning: skipping prefixes on `call'
[ 	]*6[ 	]+[0-9A-F]*
[ 	]*7[ 	]+\?\?\?\? [0-9A-F]* 		notrack jmp foo
\*\*\*\*  Error: expecting indirect branch instruction after `notrack'
[ 	]*7[ 	]+[0-9A-F]*
[ 	]*8[ 	]+
[ 	]*9[ 	]+fs notrack call \*%rax
[ 	]*10[ 	]+notrack fs call \*%rax
[ 	]*11[ 	]+
[ 	]*12[ 	]+\.intel_syntax noprefix
[ 	]*13[ 	]+fs notrack call rax
[ 	]*14[ 	]+notrack fs call rax
#pass
