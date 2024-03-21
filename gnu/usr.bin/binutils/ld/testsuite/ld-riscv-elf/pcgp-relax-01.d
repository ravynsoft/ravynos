#source: pcgp-relax-01.s
#ld: --relax
#objdump: -d -Mno-aliases

.*:[ 	]+file format .*


Disassembly of section \.text:

0+[0-9a-f]+ <_start>:
.*:[ 	]+[0-9a-f]+[ 	]+addi[ 	]+a0,a0,[0-9]+
.*:[ 	]+[0-9a-f]+[ 	]+jal[ 	]+ra,[0-9a-f]+ <_start>
.*:[ 	]+[0-9a-f]+[ 	]+addi[ 	]+a1,gp,\-[0-9]+ # [0-9a-f]+ <data_g>
.*:[ 	]+[0-9a-f]+[ 	]+addi[ 	]+a2,gp,\-[0-9]+ # [0-9a-f]+ <data_g>
.*:[ 	]+[0-9a-f]+[ 	]+addi[ 	]+a3,tp,0 # 0 <data_t>
.*:[ 	]+[0-9a-f]+[ 	]+auipc[ 	]+a0,0x[0-9a-f]+
