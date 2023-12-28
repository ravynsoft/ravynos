#source: ../x86-64-io.s
#objdump: -dwMintel
#name: x86-64 (ILP32) rex.W in/out (Intel disassembly)

.*: +file format .*

Disassembly of section .text:

0+000 <_in>:
   0:	48 ed                	rex.W in eax,dx
   2:	66 48 ed             	data16 rex.W in eax,dx

0+005 <_out>:
   5:	48 ef                	rex.W out dx,eax
   7:	66 48 ef             	data16 rex.W out dx,eax

0+00a <_ins>:
   a:	48 6d                	rex.W ins DWORD PTR es:\[rdi\],dx
   c:	66 48 6d             	data16 rex.W ins DWORD PTR es:\[rdi\],dx

0+00f <_outs>:
   f:	48 6f                	rex.W outs dx,DWORD PTR ds:\[rsi\]
  11:	66 48 6f             	data16 rex.W outs dx,DWORD PTR ds:\[rsi\]
#pass
