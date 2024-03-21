#source: code-model.s
#as: -march=rv64i -mabi=lp64 --defsym __medlow__=1 --defsym __undefweak__=1
#ld: -Tcode-model-02.ld -melf64lriscv --no-relax
#objdump: -d -Mno-aliases

.*:[ 	]+file format .*


Disassembly of section .text:

0+800000000 <_start>:
[ 	]+[0-9a-f]+:[ 	]+[0-9a-f]+[ 	]+lui[ 	]+t0,0x0
[ 	]+[0-9a-f]+:[ 	]+[0-9a-f]+[ 	]+addi[ 	]+t0,t0,0 # 0 <.*>
