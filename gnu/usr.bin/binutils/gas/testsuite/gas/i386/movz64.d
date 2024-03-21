#objdump: -dw
#source: movz.s
#name: x86 mov with zero-extend (64-bit object)

.*: +file format .*

Disassembly of section .text:

0+ <movz>:
[ 	]*[a-f0-9]+:	66 0f b6 c0 *	movzbw %al,%ax
[ 	]*[a-f0-9]+:	67 66 0f b6 00 *	movzbw \(%eax\),%ax
[ 	]*[a-f0-9]+:	0f b6 c0 *	movzbl %al,%eax
[ 	]*[a-f0-9]+:	67 0f b6 00 *	movzbl \(%eax\),%eax
[ 	]*[a-f0-9]+:	48 0f b6 c0 *	movzbq %al,%rax
[ 	]*[a-f0-9]+:	48 0f b6 00 *	movzbq \(%rax\),%rax
[ 	]*[a-f0-9]+:	66 0f b6 c0 *	movzbw %al,%ax
[ 	]*[a-f0-9]+:	67 66 0f b6 00 *	movzbw \(%eax\),%ax
[ 	]*[a-f0-9]+:	0f b6 c0 *	movzbl %al,%eax
[ 	]*[a-f0-9]+:	67 0f b6 00 *	movzbl \(%eax\),%eax
[ 	]*[a-f0-9]+:	48 0f b6 c0 *	movzbq %al,%rax
[ 	]*[a-f0-9]+:	48 0f b6 00 *	movzbq \(%rax\),%rax
[ 	]*[a-f0-9]+:	0f b7 c0 *	movzwl %ax,%eax
[ 	]*[a-f0-9]+:	67 0f b7 00 *	movzwl \(%eax\),%eax
[ 	]*[a-f0-9]+:	48 0f b7 c0 *	movzwq %ax,%rax
[ 	]*[a-f0-9]+:	48 0f b7 00 *	movzwq \(%rax\),%rax
[ 	]*[a-f0-9]+:	0f b7 c0 *	movzwl %ax,%eax
[ 	]*[a-f0-9]+:	67 0f b7 00 *	movzwl \(%eax\),%eax
[ 	]*[a-f0-9]+:	48 0f b7 c0 *	movzwq %ax,%rax
[ 	]*[a-f0-9]+:	48 0f b7 00 *	movzwq \(%rax\),%rax
#pass
