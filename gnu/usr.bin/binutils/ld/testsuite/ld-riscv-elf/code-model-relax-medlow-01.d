#source: code-model.s
#as: -march=rv64i -mabi=lp64 --defsym __medlow__=1
#ld: -Tcode-model-01.ld -melf64lriscv --relax
#objdump: -d -Mno-aliases

.*:[ 	]+file format .*


Disassembly of section .text:

0+10000 <_start>:
[ 	]+[0-9a-f]+:[ 	]+[0-9a-f]+[ 	]+addi[ 	]+t0,gp,\-[0-9]+ # [0-9a-f]+ <symbolL>
[ 	]+[0-9a-f]+:[ 	]+[0-9a-f]+[ 	]+addi[ 	]+t0,gp,\-[0-9]+ # [0-9a-f]+ <symbolG>
