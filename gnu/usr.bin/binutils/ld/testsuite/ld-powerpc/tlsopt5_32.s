 .globl _start
_start:
 stwu 1,-16(1)
 mflr 0
 bcl 20,31,.L2
.L2:
 stw 30,8(1)
 mflr 30
 addis 30,30,_GLOBAL_OFFSET_TABLE_-.L2@ha
 stw 0,20(1)
 addi 30,30,_GLOBAL_OFFSET_TABLE_-.L2@l
 addi 3,30,gd@got@tlsgd
 bl __tls_get_addr(gd@tlsgd)@plt
 lwz 0,20(1)
 lwz 30,8(1)
 mtlr 0
 addi 1,1,16
 blr
