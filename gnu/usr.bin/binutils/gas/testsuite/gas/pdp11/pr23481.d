#name: PR 23481 - correct assembly of '@rN' and '(rN)'
#objdump: -dw

.*: +file format .*

Disassembly of section .text:

0+00 <start>:
[ 	]+0:[ 	]+2009[ 	]+cmp[ 	]+r0, \(r1\)
[ 	]+2:[ 	]+2009[ 	]+cmp[ 	]+r0, \(r1\)
[ 	]+4:[ 	]+2240[ 	]+cmp[ 	]+\(r1\), r0
[ 	]+6:[ 	]+2240[ 	]+cmp[ 	]+\(r1\), r0
[ 	]+8:[ 	]+2249[ 	]+cmp[ 	]+\(r1\), \(r1\)
#pass
