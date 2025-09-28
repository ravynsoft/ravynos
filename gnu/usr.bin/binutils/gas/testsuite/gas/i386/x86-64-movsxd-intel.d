#source: x86-64-movsxd.s
#as:
#objdump: -dw -Mintel
#name: x86-64 movsxd (AMD64) (Intel mode)

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
 +[a-f0-9]+:	48 63 c8             	movsxd rcx,eax
 +[a-f0-9]+:	48 63 08             	movsxd rcx,DWORD PTR \[rax\]
 +[a-f0-9]+:	63 c8                	movsxd ecx,eax
 +[a-f0-9]+:	63 08                	movsxd ecx,DWORD PTR \[rax\]
 +[a-f0-9]+:	66 63 c8             	movsxd cx,eax
 +[a-f0-9]+:	66 63 08             	movsxd cx,DWORD PTR \[rax\]
 +[a-f0-9]+:	48 63 c8             	movsxd rcx,eax
 +[a-f0-9]+:	48 63 08             	movsxd rcx,DWORD PTR \[rax\]
 +[a-f0-9]+:	48 63 08             	movsxd rcx,DWORD PTR \[rax\]
 +[a-f0-9]+:	63 c8                	movsxd ecx,eax
 +[a-f0-9]+:	63 08                	movsxd ecx,DWORD PTR \[rax\]
 +[a-f0-9]+:	63 08                	movsxd ecx,DWORD PTR \[rax\]
 +[a-f0-9]+:	66 63 c8             	movsxd cx,eax
 +[a-f0-9]+:	66 63 08             	movsxd cx,DWORD PTR \[rax\]
 +[a-f0-9]+:	66 63 08             	movsxd cx,DWORD PTR \[rax\]
#pass
