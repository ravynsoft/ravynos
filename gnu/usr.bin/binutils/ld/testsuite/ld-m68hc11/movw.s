;;; Bug. movw used R_M68HC12_16B which was 1 byte misaligned.
	.sect .text
	.globl _start
_start:
start:
    movw    gp_max_on,x, gp_clk,x
    movw    0x22,sp, gp_clk,y
    movw    gp_clk,x, 0x12,sp
    movw    0x1001,x, 0x2002,y
    movw    small_off,sp, gp_max_on,y
    tfr     x,y
    rts

