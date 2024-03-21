#as: -J -march=iamcu
#objdump: -dw

.*: +file format elf32-iamcu.*

Disassembly of section .text:

0+ <.text>:
[ 	]*[a-f0-9]+:	66 0f be f0          	movsbw %al,%si
[ 	]*[a-f0-9]+:	0f be f0             	movsbl %al,%esi
[ 	]*[a-f0-9]+:	0f bf f0             	movswl %ax,%esi
[ 	]*[a-f0-9]+:	66 0f be 10          	movsbw \(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f be 10          	movsbw \(%eax\),%dx
[ 	]*[a-f0-9]+:	0f be 10             	movsbl \(%eax\),%edx
[ 	]*[a-f0-9]+:	0f bf 10             	movswl \(%eax\),%edx
[ 	]*[a-f0-9]+:	0f be 10             	movsbl \(%eax\),%edx
[ 	]*[a-f0-9]+:	66 0f be 10          	movsbw \(%eax\),%dx
[ 	]*[a-f0-9]+:	0f bf 10             	movswl \(%eax\),%edx
[ 	]*[a-f0-9]+:	66 0f b6 f0          	movzbw %al,%si
[ 	]*[a-f0-9]+:	0f b6 f0             	movzbl %al,%esi
[ 	]*[a-f0-9]+:	0f b7 f0             	movzwl %ax,%esi
[ 	]*[a-f0-9]+:	66 0f b6 10          	movzbw \(%eax\),%dx
[ 	]*[a-f0-9]+:	66 0f b6 10          	movzbw \(%eax\),%dx
[ 	]*[a-f0-9]+:	0f b6 10             	movzbl \(%eax\),%edx
[ 	]*[a-f0-9]+:	0f b7 10             	movzwl \(%eax\),%edx
[ 	]*[a-f0-9]+:	0f b6 10             	movzbl \(%eax\),%edx
[ 	]*[a-f0-9]+:	66 0f b6 10          	movzbw \(%eax\),%dx
[ 	]*[a-f0-9]+:	0f b6 10             	movzbl \(%eax\),%edx
[ 	]*[a-f0-9]+:	66 0f b6 10          	movzbw \(%eax\),%dx
[ 	]*[a-f0-9]+:	0f b7 10             	movzwl \(%eax\),%edx
[ 	]*[a-f0-9]+:	66 0f be f0          	movsbw %al,%si
[ 	]*[a-f0-9]+:	0f be f0             	movsbl %al,%esi
[ 	]*[a-f0-9]+:	0f bf f0             	movswl %ax,%esi
[ 	]*[a-f0-9]+:	0f be 10             	movsbl \(%eax\),%edx
[ 	]*[a-f0-9]+:	66 0f be 10          	movsbw \(%eax\),%dx
[ 	]*[a-f0-9]+:	0f bf 10             	movswl \(%eax\),%edx
[ 	]*[a-f0-9]+:	66 0f b6 f0          	movzbw %al,%si
[ 	]*[a-f0-9]+:	0f b6 f0             	movzbl %al,%esi
[ 	]*[a-f0-9]+:	0f b7 f0             	movzwl %ax,%esi
[ 	]*[a-f0-9]+:	0f b6 10             	movzbl \(%eax\),%edx
[ 	]*[a-f0-9]+:	66 0f b6 10          	movzbw \(%eax\),%dx
[ 	]*[a-f0-9]+:	0f b7 10             	movzwl \(%eax\),%edx
[ 	]*[a-f0-9]+:	66 0f be 00          	movsbw \(%eax\),%ax
[ 	]*[a-f0-9]+:	0f be 00             	movsbl \(%eax\),%eax
[ 	]*[a-f0-9]+:	0f bf 00             	movswl \(%eax\),%eax
[ 	]*[a-f0-9]+:	66 0f b6 00          	movzbw \(%eax\),%ax
[ 	]*[a-f0-9]+:	0f b6 00             	movzbl \(%eax\),%eax
[ 	]*[a-f0-9]+:	0f b7 00             	movzwl \(%eax\),%eax
#pass
