.syntax unified
.text
@ VMVN with cmode = d
.inst 0xff800d70

@ VMVN with cmode = f, should be undefined
.inst 0xff800f70

@ VBIC
.inst 0xef800e70 @VMOV
.inst 0xef800070 @VMVN
.inst 0xef800270 @VMVN
.inst 0xef800470 @VMVN
.inst 0xef800670 @VMVN
.inst 0xef800870 @VMVN
.inst 0xef800a70 @VMVN
.inst 0xef800c70 @VMVN

@ VMOV
.inst 0xef800150 @VORR
.inst 0xef800350 @VORR
.inst 0xef800550 @VORR
.inst 0xef800950 @VORR
.inst 0xef800b50 @VORR
.inst 0xef800170 @VBIC
.inst 0xef800370 @VBIC
.inst 0xef800570 @VBIC
.inst 0xef800770 @VBIC
.inst 0xef800970 @VBIC
.inst 0xef800b70 @VBIC

@ VMVN same as VMOV -> VBIC
@ VORR
.inst 0xef800050 @VMOV
.inst 0xef800250 @VMOV
.inst 0xef800450 @VMOV
.inst 0xef800650 @VMOV
.inst 0xef800850 @VMOV
.inst 0xef800a50 @VMOV
.inst 0xef800c50 @VMOV
.inst 0xef800e50 @VMOV
.inst 0xef800d50 @VMOV
.inst 0xef800f50 @VMOV


vmvn.i32 q0,#0x80ffff
vmov.i32 q0,#0x80ffff
