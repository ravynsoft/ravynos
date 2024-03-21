#source: code-model.s
#as: -march=rv64i -mabi=lp64 --defsym __medany__=1
#ld: -Tcode-model-02.ld -melf64lriscv --no-relax
#objdump: -d -Mno-aliases

.*:[ 	]+file format .*


Disassembly of section .text:

0+800000000 <_start>:
[ 	]+[0-9a-f]+:[ 	]+[0-9a-f]+[ 	]+lui[ 	]+t0,0x[0-9a-f]+
[ 	]+[0-9a-f]+:[ 	]+[0-9a-f]+[ 	]+ld[ 	]+t0,[0-9]+\(t0\) # [0-9a-f]+ <_GLOBAL_OFFSET_TABLE_\+0x[0-9a-f]+>
[ 	]+[0-9a-f]+:[ 	]+[0-9a-f]+[ 	]+lui[ 	]+t0,0x[0-9a-f]+
[ 	]+[0-9a-f]+:[ 	]+[0-9a-f]+[ 	]+addi[ 	]+t0,t0,[0-9]+ # [0-9a-f]+ <symbolL>
[ 	]+[0-9a-f]+:[ 	]+[0-9a-f]+[ 	]+lui[ 	]+t0,0x[0-9a-f]+
[ 	]+[0-9a-f]+:[ 	]+[0-9a-f]+[ 	]+addi[ 	]+t0,t0,[0-9]+ # [0-9a-f]+ <symbolG>
