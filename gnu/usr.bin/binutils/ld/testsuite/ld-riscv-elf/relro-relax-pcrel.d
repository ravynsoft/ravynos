#source: relro-relax-pcrel.s
#ld: -zrelro --relax
#objdump: -d -Mno-aliases

.*:[ 	]+file format .*


Disassembly of section .text:

0+[0-9a-f]+ <_start>:
.*:[ 	]+[0-9a-f]+[ 	]+auipc[ 	]+.*
.*:[ 	]+[0-9a-f]+[ 	]+addi[ 	]+.*<SymbolRodata>
