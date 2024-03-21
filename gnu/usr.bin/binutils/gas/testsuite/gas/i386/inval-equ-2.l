.*: Assembler messages:
.*:3: Error: .*
.*:5: Error: .*
.*:8: Error: .*
.*: Error: .*
.*: Error: .*
.*: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\.globl  bar1
[ 	]*2[ 	]+\.set    bar1,\(%eax\+1\)
[ 	]*3[ 	]+\?\?\?\? A1...... 		mov bar1,%eax
[ 	]*3[ 	]+..
[ 	]*4[ 	]+\.set    bar2,\(%eax\+1\)
[ 	]*5[ 	]+\?\?\?\? A1...... 		mov bar2,%eax
[ 	]*5[ 	]+..
[ 	]*6[ 	]+\.globl  bar2
[ 	]*7[ 	]+\.set    bar3,\(%eax\+1\)
[ 	]*8[ 	]+\?\?\?\? A1...... 		mov bar3,%eax
.*  Error: invalid .* relocation against register
.*  Error: invalid .* relocation against register
.*  Error: invalid .* relocation against register
.*  Error: can't make global register symbol `bar1'
.*  Error: can't make global register symbol `bar2'
.*  Error: can't make global register symbol `bar3'
[ 	]*8[ 	]+..
