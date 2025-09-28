#source: ppc476-shared.s
#as: -a32
#ld: -melf32ppc -shared -z common-page-size=0x10000 -z notext --ppc476-workaround -T ppc476-shared.lnk
#objdump: -R
#target: powerpc*-*-*

.*:     file format .*

DYNAMIC RELOCATION RECORDS
OFFSET +TYPE +VALUE
0001000[02] R_PPC_ADDR16_LO   \.text\+0x00050000
0002000[02] R_PPC_ADDR16_LO   \.text\+0x00050000
0003000[02] R_PPC_ADDR16_LO   \.text\+0x00050000
0004000[02] R_PPC_ADDR16_HA   \.text\+0x00050000
0004001[02] R_PPC_ADDR16_HA   \.text\+0x00050000
0004002[02] R_PPC_ADDR16_HA   \.text\+0x00050000
