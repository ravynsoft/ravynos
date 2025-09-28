.*: Assembler messages:
.*:5: Error: .*
.*:6: Error: .*
.*:10: Error: .*
.*:11: Error: .*
GAS LISTING .*


[ 	]*1[ 	]+\# Test SIZE32 relocation overflow
[ 	]*2[ 	]+\.local	yyy
[ 	]*3[ 	]+\.comm	yyy,80,32
[ 	]*4[ 	]+\.text
[ 	]*5[ 	]+\?\?\?\? B85E0000 		movl	\$xxx@SIZE \+ 100, %eax
[ 	]*5[ 	]+00
[ 	]*6[ 	]+\?\?\?\? B8ECFFFF 		movl	\$yyy@SIZE - 100, %eax
[ 	]*6[ 	]+FF
[ 	]*7[ 	]+\.local	xxx
[ 	]*8[ 	]+\.comm	xxx,4294967290,32
[ 	]*9[ 	]+\.data
[ 	]*10[ 	]+\?\?\?\? 5E000000 		\.long	xxx@SIZE \+ 100
[ 	]*11[ 	]+\?\?\?\? ECFFFFFF 		\.long	yyy@SIZE - 100
.*  Error: symbol size computation overflow
.*  Error: symbol size computation overflow
.*  Error: symbol size computation overflow
.*  Error: symbol size computation overflow
