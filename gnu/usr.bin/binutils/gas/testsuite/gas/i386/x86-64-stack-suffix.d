#objdump: -dwMsuffix
#name: x86-64 stack-related opcodes (with suffixes)
#source: x86-64-stack.s

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	50                   	pushq  %rax
[ 	]*[a-f0-9]+:	66 50                	pushw  %ax
[ 	]*[a-f0-9]+:	48 50                	rex.W pushq %rax
[ 	]*[a-f0-9]+:	66 48 50             	data16 rex.W pushq %rax
[ 	]*[a-f0-9]+:	58                   	popq   %rax
[ 	]*[a-f0-9]+:	66 58                	popw   %ax
[ 	]*[a-f0-9]+:	48 58                	rex.W popq %rax
[ 	]*[a-f0-9]+:	66 48 58             	data16 rex.W popq %rax
[ 	]*[a-f0-9]+:	8f c0                	popq   %rax
[ 	]*[a-f0-9]+:	66 8f c0             	popw   %ax
[ 	]*[a-f0-9]+:	48 8f c0             	rex.W popq %rax
[ 	]*[a-f0-9]+:	66 48 8f c0          	data16 rex.W popq %rax
[ 	]*[a-f0-9]+:	8f 00                	popq   \(%rax\)
[ 	]*[a-f0-9]+:	66 8f 00             	popw   \(%rax\)
[ 	]*[a-f0-9]+:	48 8f 00             	rex.W popq \(%rax\)
[ 	]*[a-f0-9]+:	66 48 8f 00          	data16 rex.W popq \(%rax\)
[ 	]*[a-f0-9]+:	ff d0                	callq  \*%rax
[ 	]*[a-f0-9]+:	66 ff d0             	callw  \*%ax
[ 	]*[a-f0-9]+:	48 ff d0             	rex.W callq \*%rax
[ 	]*[a-f0-9]+:	66 48 ff d0          	data16 rex.W callq \*%rax
[ 	]*[a-f0-9]+:	ff 10                	callq  \*\(%rax\)
[ 	]*[a-f0-9]+:	66 ff 10             	callw  \*\(%rax\)
[ 	]*[a-f0-9]+:	48 ff 10             	rex.W callq \*\(%rax\)
[ 	]*[a-f0-9]+:	66 48 ff 10          	data16 rex.W callq \*\(%rax\)
[ 	]*[a-f0-9]+:	ff e0                	jmpq   \*%rax
[ 	]*[a-f0-9]+:	66 ff e0             	jmpw   \*%ax
[ 	]*[a-f0-9]+:	48 ff e0             	rex.W jmpq \*%rax
[ 	]*[a-f0-9]+:	66 48 ff e0          	data16 rex.W jmpq \*%rax
[ 	]*[a-f0-9]+:	ff 20                	jmpq   \*\(%rax\)
[ 	]*[a-f0-9]+:	66 ff 20             	jmpw   \*\(%rax\)
[ 	]*[a-f0-9]+:	48 ff 20             	rex.W jmpq \*\(%rax\)
[ 	]*[a-f0-9]+:	66 48 ff 20          	data16 rex.W jmpq \*\(%rax\)
[ 	]*[a-f0-9]+:	ff f0                	pushq  %rax
[ 	]*[a-f0-9]+:	66 ff f0             	pushw  %ax
[ 	]*[a-f0-9]+:	48 ff f0             	rex.W pushq %rax
[ 	]*[a-f0-9]+:	66 48 ff f0          	data16 rex.W pushq %rax
[ 	]*[a-f0-9]+:	ff 30                	pushq  \(%rax\)
[ 	]*[a-f0-9]+:	66 ff 30             	pushw  \(%rax\)
[ 	]*[a-f0-9]+:	48 ff 30             	rex.W pushq \(%rax\)
[ 	]*[a-f0-9]+:	66 48 ff 30          	data16 rex.W pushq \(%rax\)
[ 	]*[a-f0-9]+:	6a ff                	pushq  \$0xffffffffffffffff
[ 	]*[a-f0-9]+:	66 6a ff             	pushw  \$0xffff
[ 	]*[a-f0-9]+:	48 6a ff             	rex.W pushq \$0xffffffffffffffff
[ 	]*[a-f0-9]+:	66 48 6a ff          	data16 rex.W pushq \$0xffffffffffffffff
[ 	]*[a-f0-9]+:	68 02 03 04 05       	pushq  \$0x5040302
[ 	]*[a-f0-9]+:	66 68 02 03          	pushw  \$0x302
[ 	]*[a-f0-9]+:	04 05                	addb   \$0x5,%al
[ 	]*[a-f0-9]+:	48 68 02 03 04 05    	rex\.W pushq \$0x5040302
[ 	]*[a-f0-9]+:	66 48 68 02 03 04 05 	data16 rex\.W pushq \$0x5040302
[ 	]*[a-f0-9]+:	0f a8                	pushq  %gs
[ 	]*[a-f0-9]+:	66 0f a8             	pushw  %gs
[ 	]*[a-f0-9]+:	48 0f a8             	rex.W pushq %gs
[ 	]*[a-f0-9]+:	66 48 0f a8          	data16 rex.W pushq %gs
[ 	]*[a-f0-9]+:	41 0f a8             	rex.B pushq %gs
[ 	]*[a-f0-9]+:	66 41 0f a8          	rex.B pushw %gs
[ 	]*[a-f0-9]+:	48                   	rex.W
[ 	]*[a-f0-9]+:	41 0f a8             	rex.B pushq %gs
[ 	]*[a-f0-9]+:	66 48                	data16 rex.W
[ 	]*[a-f0-9]+:	41 0f a8             	rex.B pushq %gs
[ 	]*[a-f0-9]+:	90                   	nop
#pass
