.*: Assembler messages:
.*:6: Error: expecting valid branch instruction after `bnd'
.*:7: Error: expecting valid branch instruction after `bnd'
.*:8: Error: expecting valid branch instruction after `bnd'
.*:8: Warning: skipping prefixes on `lcall'
.*:9: Error: expecting valid branch instruction after `bnd'
.*:9: Warning: skipping prefixes on `ljmp'
.*:10: Error: expecting valid branch instruction after `bnd'
.*:11: Error: expecting valid branch instruction after `bnd'
.*:14: Error: expecting valid branch instruction after `bnd'
.*:15: Error: expecting valid branch instruction after `bnd'
.*:16: Error: expecting valid branch instruction after `bnd'
.*:16: Warning: skipping prefixes on `lcall'
.*:17: Error: expecting valid branch instruction after `bnd'
.*:17: Warning: skipping prefixes on `ljmp'
.*:18: Error: expecting valid branch instruction after `bnd'
.*:19: Error: expecting valid branch instruction after `bnd'
GAS LISTING .*


[ 	]*1[ 	]+\# MPX instructions
[ 	]*2[ 	]+\.allow_index_reg
[ 	]*3[ 	]+\.text
[ 	]*4[ 	]+\.extern xxx
[ 	]*5[ 	]+foo:
[ 	]*6[ 	]+\?\?\?\? F201C3   		bnd add %eax, %ebx  		\# Bad
.*  Error: expecting valid branch instruction after `bnd'
[ 	]*7[ 	]+\?\?\?\? 66F2AB   		bnd stosw \(%edi\)    		\# Bad
.*  Error: expecting valid branch instruction after `bnd'
[ 	]*8[ 	]+\?\?\?\? 9A000000 		bnd lcall \$0x1234,\$xxx
.*  Error: expecting valid branch instruction after `bnd'
.*  Warning: skipping prefixes on `lcall'
[ 	]*8[ 	]+003412
[ 	]*9[ 	]+\?\?\?\? EA000000 		bnd ljmp \$0x1234,\$xxx
.*  Error: expecting valid branch instruction after `bnd'
.*  Warning: skipping prefixes on `ljmp'
[ 	]*9[ 	]+003412
[ 	]*10[ 	]+\?\?\?\? F2E2E9   		bnd loop foo
.*  Error: expecting valid branch instruction after `bnd'
[ 	]*11[ 	]+\?\?\?\? 67F2E3E5 		bnd jcxz foo
.*  Error: expecting valid branch instruction after `bnd'
[ 	]*12[ 	]+
[ 	]*13[ 	]+\.intel_syntax noprefix
[ 	]*14[ 	]+\?\?\?\? F201C3   		bnd add ebx, eax		\# Bad
.*  Error: expecting valid branch instruction after `bnd'
[ 	]*15[ 	]+\?\?\?\? 66F2AB   		bnd stos WORD PTR\[edi]		\# Bad
.*  Error: expecting valid branch instruction after `bnd'
[ 	]*16[ 	]+\?\?\?\? 9A000000 		bnd lcall 0x1234,xxx
.*  Error: expecting valid branch instruction after `bnd'
.*  Warning: skipping prefixes on `lcall'
[ 	]*16[ 	]+003412
[ 	]*17[ 	]+\?\?\?\? EA000000 		bnd ljmp 0x1234,xxx
.*  Error: expecting valid branch instruction after `bnd'
.*  Warning: skipping prefixes on `ljmp'
[ 	]*17[ 	]+003412
[ 	]*18[ 	]+\?\?\?\? F2E2CE   		bnd loop foo
.*  Error: expecting valid branch instruction after `bnd'
[ 	]*19[ 	]+\?\?\?\? 67F2E3CA 		bnd jcxz foo
.*  Error: expecting valid branch instruction after `bnd'
#pass
