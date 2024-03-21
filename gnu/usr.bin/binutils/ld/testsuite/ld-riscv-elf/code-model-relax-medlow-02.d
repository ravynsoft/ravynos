#source: code-model.s
#as: -march=rv64i -mabi=lp64 --defsym __medlow__=1
#ld: -Tcode-model-02.ld -melf64lriscv --relax
#objdump: -d -Mno-aliases

.*:[ 	]+file format .*


Disassembly of section .text:

0+800000000 <_start>:
[ 	]+[0-9a-f]+:[ 	]+[0-9a-f]+[ 	]+addi[ 	]+t0,zero,[0-9]+
[ 	]+[0-9a-f]+:[ 	]+[0-9a-f]+[ 	]+addi[ 	]+t0,zero,[0-9]+
