#name: PR 14480 - correct assembly of 'jsr pc, @(r0)'
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+00 <start>:
[ 	]+0:[ 	]+15c0 0014[ 	]+mov[ 	]+\$24, r0
[ 	]+4:[ 	]+09c8[ 	]+jsr[ 	]+pc, \(r0\)
[ 	]+6:[ 	]+09f8 0000[ 	]+jsr[ 	]+pc, \*0\(r0\)
[ 	]+a:[ 	]+09f8 0000[ 	]+jsr[ 	]+pc, \*0\(r0\)
[ 	]+e:[ 	]+09f8 0002[ 	]+jsr[ 	]+pc, \*2\(r0\)
#pass
