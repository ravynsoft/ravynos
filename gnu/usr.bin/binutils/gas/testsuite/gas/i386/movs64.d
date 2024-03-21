#objdump: -dw
#source: movs.s
#name: x86 mov with sign-extend (64-bit object)

.*: +file format .*

Disassembly of section .text:

0+ <movs>:
[ 	]*[a-f0-9]+:	66 0f be c0 *	movsbw %al,%ax
[ 	]*[a-f0-9]+:	67 66 0f be 00 *	movsbw \(%eax\),%ax
[ 	]*[a-f0-9]+:	0f be c0 *	movsbl %al,%eax
[ 	]*[a-f0-9]+:	67 0f be 00 *	movsbl \(%eax\),%eax
[ 	]*[a-f0-9]+:	48 0f be c0 *	movsbq %al,%rax
[ 	]*[a-f0-9]+:	48 0f be 00 *	movsbq \(%rax\),%rax
[ 	]*[a-f0-9]+:	66 0f be c0 *	movsbw %al,%ax
[ 	]*[a-f0-9]+:	67 66 0f be 00 *	movsbw \(%eax\),%ax
[ 	]*[a-f0-9]+:	0f be c0 *	movsbl %al,%eax
[ 	]*[a-f0-9]+:	67 0f be 00 *	movsbl \(%eax\),%eax
[ 	]*[a-f0-9]+:	48 0f be c0 *	movsbq %al,%rax
[ 	]*[a-f0-9]+:	48 0f be 00 *	movsbq \(%rax\),%rax
[ 	]*[a-f0-9]+:	0f bf c0 *	movswl %ax,%eax
[ 	]*[a-f0-9]+:	67 0f bf 00 *	movswl \(%eax\),%eax
[ 	]*[a-f0-9]+:	48 0f bf c0 *	movswq %ax,%rax
[ 	]*[a-f0-9]+:	48 0f bf 00 *	movswq \(%rax\),%rax
[ 	]*[a-f0-9]+:	0f bf c0 *	movswl %ax,%eax
[ 	]*[a-f0-9]+:	67 0f bf 00 *	movswl \(%eax\),%eax
[ 	]*[a-f0-9]+:	48 0f bf c0 *	movswq %ax,%rax
[ 	]*[a-f0-9]+:	48 0f bf 00 *	movswq \(%rax\),%rax
#pass
