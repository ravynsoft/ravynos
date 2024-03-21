.*: Assembler messages:
.*:2: Error: invalid instruction suffix for `call'
.*:3: Error: invalid instruction suffix for `call'
.*:4: Error: operand (size|type) mismatch for `jmp'
.*:5: Error: invalid instruction suffix for `jmp'
.*:6: Error: invalid instruction suffix for `jmp'
.*:7: Error: invalid instruction suffix for `ret'
.*:8: Error: invalid instruction suffix for `ret'
.*:11: Error: operand (size|type) mismatch for `call'
.*:12: Error: invalid instruction suffix for `call'
.*:13: Error: invalid instruction suffix for `call'
.*:14: Error: operand (size|type) mismatch for `call'
.*:15: Error: operand (size|type) mismatch for `jmp'
.*:16: Error: invalid instruction suffix for `jmp'
.*:17: Error: invalid instruction suffix for `jmp'
.*:18: Error: operand (size|type) mismatch for `jmp'
.*:19: Error: invalid instruction suffix for `ret'
.*:20: Error: invalid instruction suffix for `ret'
GAS LISTING .*
#...
[ 	]*1[ 	]+\.text
[ 	]*2[ 	]+callw	\*%ax
[ 	]*3[ 	]+callw	\*\(%rax\)
[ 	]*4[ 	]+jmp	\*%ax
[ 	]*5[ 	]+jmpw	\*%ax
[ 	]*6[ 	]+jmpw	\*\(%rax\)
[ 	]*7[ 	]+retw
[ 	]*8[ 	]+retw	\$8
[ 	]*9[ 	]+
[ 	]*10[ 	]+\.intel_syntax noprefix
[ 	]*11[ 	]+call	ax
[ 	]*12[ 	]+callw	ax
[ 	]*13[ 	]+callw	\[rax\]
[ 	]*14[ 	]+call	WORD PTR \[rax\]
[ 	]*15[ 	]+jmp	ax
[ 	]*16[ 	]+jmpw	ax
[ 	]*17[ 	]+jmpw	\[rax\]
[ 	]*18[ 	]+jmp	WORD PTR \[rax\]
[ 	]*19[ 	]+retw
[ 	]*20[ 	]+retw	8
#pass
