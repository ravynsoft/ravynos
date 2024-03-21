#as: -mfix-loongson2f-nop
#objdump: -M reg-names=numeric -dr
#name: ST Microelectronics Loongson-2F workarounds of nop issue

.*:     file format .*

Disassembly of section .text:

0+000000 <loongson2f_nop_insn>:
   0:	00200825 	move	\$1,\$1
   4:	00200825 	move	\$1,\$1
   8:	00200825 	move	\$1,\$1
   c:	00200825 	move	\$1,\$1
  10:	00200825 	move	\$1,\$1
  14:	00200825 	move	\$1,\$1
  18:	00200825 	move	\$1,\$1
  1c:	00200825 	move	\$1,\$1
