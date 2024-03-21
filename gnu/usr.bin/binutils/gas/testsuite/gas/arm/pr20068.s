.global main

main:
LDR R0, =0x12345678
@LDR R0, =0x87654321
FLDD D9, =0xfff000000fff
@FLDD D9, =0
@FLDD D9, =0x0
MOV PC, LR
.ltorg
