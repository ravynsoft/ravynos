#objdump: -dwMintel
#name: x86-64 stack-related opcodes (Intel mode)
#source: x86-64-stack.s

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	50                   	push   rax
[ 	]*[a-f0-9]+:	66 50                	push   ax
[ 	]*[a-f0-9]+:	48 50                	rex.W push rax
[ 	]*[a-f0-9]+:	66 48 50             	data16 rex.W push rax
[ 	]*[a-f0-9]+:	58                   	pop    rax
[ 	]*[a-f0-9]+:	66 58                	pop    ax
[ 	]*[a-f0-9]+:	48 58                	rex.W pop rax
[ 	]*[a-f0-9]+:	66 48 58             	data16 rex.W pop rax
[ 	]*[a-f0-9]+:	8f c0                	pop    rax
[ 	]*[a-f0-9]+:	66 8f c0             	pop    ax
[ 	]*[a-f0-9]+:	48 8f c0             	rex.W pop rax
[ 	]*[a-f0-9]+:	66 48 8f c0          	data16 rex.W pop rax
[ 	]*[a-f0-9]+:	8f 00                	pop    QWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	66 8f 00             	pop    WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 8f 00             	rex.W pop QWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	66 48 8f 00          	data16 rex.W pop QWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	ff d0                	call   rax
[ 	]*[a-f0-9]+:	66 ff d0             	call   ax
[ 	]*[a-f0-9]+:	48 ff d0             	rex.W call rax
[ 	]*[a-f0-9]+:	66 48 ff d0          	data16 rex.W call rax
[ 	]*[a-f0-9]+:	ff 10                	call   QWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	66 ff 10             	call   WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 ff 10             	rex.W call QWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	66 48 ff 10          	data16 rex.W call QWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	ff e0                	jmp    rax
[ 	]*[a-f0-9]+:	66 ff e0             	jmp    ax
[ 	]*[a-f0-9]+:	48 ff e0             	rex.W jmp rax
[ 	]*[a-f0-9]+:	66 48 ff e0          	data16 rex.W jmp rax
[ 	]*[a-f0-9]+:	ff 20                	jmp    QWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	66 ff 20             	jmp    WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 ff 20             	rex.W jmp QWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	66 48 ff 20          	data16 rex.W jmp QWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	ff f0                	push   rax
[ 	]*[a-f0-9]+:	66 ff f0             	push   ax
[ 	]*[a-f0-9]+:	48 ff f0             	rex.W push rax
[ 	]*[a-f0-9]+:	66 48 ff f0          	data16 rex.W push rax
[ 	]*[a-f0-9]+:	ff 30                	push   QWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	66 ff 30             	push   WORD PTR \[rax\]
[ 	]*[a-f0-9]+:	48 ff 30             	rex.W push QWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	66 48 ff 30          	data16 rex.W push QWORD PTR \[rax\]
[ 	]*[a-f0-9]+:	6a ff                	push   0xffffffffffffffff
[ 	]*[a-f0-9]+:	66 6a ff             	pushw  0xffff
[ 	]*[a-f0-9]+:	48 6a ff             	rex.W push 0xffffffffffffffff
[ 	]*[a-f0-9]+:	66 48 6a ff          	data16 rex.W push 0xffffffffffffffff
[ 	]*[a-f0-9]+:	68 02 03 04 05       	push   0x5040302
[ 	]*[a-f0-9]+:	66 68 02 03          	pushw  0x302
[ 	]*[a-f0-9]+:	04 05                	add    al,0x5
[ 	]*[a-f0-9]+:	48 68 02 03 04 05    	rex\.W push 0x5040302
[ 	]*[a-f0-9]+:	66 48 68 02 03 04 05 	data16 rex\.W push 0x5040302
[ 	]*[a-f0-9]+:	0f a8                	push   gs
[ 	]*[a-f0-9]+:	66 0f a8             	pushw  gs
[ 	]*[a-f0-9]+:	48 0f a8             	rex.W push gs
[ 	]*[a-f0-9]+:	66 48 0f a8          	data16 rex.W push gs
[ 	]*[a-f0-9]+:	41 0f a8             	rex.B push gs
[ 	]*[a-f0-9]+:	66 41 0f a8          	rex.B pushw gs
[ 	]*[a-f0-9]+:	48                   	rex.W
[ 	]*[a-f0-9]+:	41 0f a8             	rex.B push gs
[ 	]*[a-f0-9]+:	66 48                	data16 rex.W
[ 	]*[a-f0-9]+:	41 0f a8             	rex.B push gs
[ 	]*[a-f0-9]+:	90                   	nop
#pass
