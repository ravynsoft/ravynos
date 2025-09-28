#source: ../x86-64-ept.s
#objdump: -dw
#name: x86-64 (ILP32) EPT

.*:     file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	66 0f 38 80 19       	invept \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	66 44 0f 38 80 19    	invept \(%rcx\),%r11
[ 	]*[a-f0-9]+:	66 0f 38 81 19       	invvpid \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	66 44 0f 38 81 19    	invvpid \(%rcx\),%r11
[ 	]*[a-f0-9]+:	66 0f 38 80 19       	invept \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	66 44 0f 38 80 19    	invept \(%rcx\),%r11
[ 	]*[a-f0-9]+:	66 0f 38 81 19       	invvpid \(%rcx\),%rbx
[ 	]*[a-f0-9]+:	66 44 0f 38 81 19    	invvpid \(%rcx\),%r11
#pass
