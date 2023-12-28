.*: Assembler messages:
.*:4: Error: expecting valid branch instruction after `bnd'
.*:5: Error: expecting valid branch instruction after `bnd'
.*:6: Error: expecting valid branch instruction after `bnd'
.*:7: Error: expecting valid branch instruction after `bnd'
.*:10: Error: expecting valid branch instruction after `bnd'
.*:11: Error: expecting valid branch instruction after `bnd'
.*:12: Error: expecting valid branch instruction after `bnd'
.*:13: Error: expecting valid branch instruction after `bnd'
GAS LISTING .*


[ 	]*1[ 	]+\# MPX instructions
[ 	]*2[ 	]+\.allow_index_reg
[ 	]*3[ 	]+\.text
[ 	]*4[ 	]+\?\?\?\? F24801C3 		bnd add %rax, %rbx  		\# Bad
.*  Error: expecting valid branch instruction after `bnd'
[ 	]*5[ 	]+\?\?\?\? 6766F2AB 		bnd stosw \(%edi\)    		\# Bad
.*  Error: expecting valid branch instruction after `bnd'
[ 	]*6[ 	]+\?\?\?\? F2E2(00|0A)   		bnd loop foo
.*  Error: expecting valid branch instruction after `bnd'
[ 	]*7[ 	]+\?\?\?\? F2E3(00|0D)   		bnd jrcxz foo
.*  Error: expecting valid branch instruction after `bnd'
[ 	]*8[ 	]+
[ 	]*9[ 	]+\.intel_syntax noprefix
[ 	]*10[ 	]+\?\?\?\? F24801C3 		bnd add rbx, rax		\# Bad
.*  Error: expecting valid branch instruction after `bnd'
[ 	]*11[ 	]+\?\?\?\? 6766F2AB 		bnd stos WORD PTR \[edi]		\# Bad
.*  Error: expecting valid branch instruction after `bnd'
[ 	]*12[ 	]+\?\?\?\? F2E2(00|18)   		bnd loop foo
.*  Error: expecting valid branch instruction after `bnd'
[ 	]*13[ 	]+\?\?\?\? F2E3(00|1B)(  |90) 		bnd jrcxz foo
.*  Error: expecting valid branch instruction after `bnd'
#pass
