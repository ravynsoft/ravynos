#source: pcgp-relax-02.s
#as:
#ld: --relax --relax-gp
#objdump: -d

.*:[ 	]+file format .*


Disassembly of section .text:

[0-9a-f]+ <_start>:
.*:[ 	]+[0-9a-f]+[ 	]+auipc[ 	]+a1.*
.*:[ 	]+[0-9a-f]+[ 	]+addi?[ 	]+a0,gp.*<data_a>
.*:[ 	]+[0-9a-f]+[ 	]+addi?[ 	]+a1,a1.*<data_b>
#pass
