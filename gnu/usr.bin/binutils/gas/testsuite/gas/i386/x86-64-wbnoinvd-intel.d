#objdump: -dwMintel
#name: i386 WBNOINVD (Intel disassembly)
#source: wbnoinvd.s

.*: +file format .*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:[ 	]*f3 0f 09[ 	]*wbnoinvd
#pass
