# Test for correct generation of 9s12x specific moves

	.sect .text
;;
;; Test all s12x extended forms of movb, movw
;; page 273 et seq in S12XCPUV2
;;
v1=4
v2=68
v3=88
v4=0x89
v5=0xfe
v6=0x80
a1=0x1234
a2=0x3456
a3=0x8123
a4=0xc567
a5=0x2987
a6=0x1009

;movb
    movb     #v1, a1
    movb     #v2, 0,x
    movb     #v3, -254,y
    movb     #v4, a1,sp
    movb     #v5, [d,x]
    movb     #v6, [a2,sp]

    movb     a1, a2
    movb     a2, 1,x
    movb     a3, 255,y
    movb     a4, a1,sp
    movb     a5, [d,y]
    movb     a6, [a3,sp]

    movb     1,x+, a1
    movb     2,-x, 15,x
    movb     7,sp+, 253,y
    movb     6,-sp, a2,sp
    movb     -15,y, [d,x]
    movb     13,sp, [a5,sp]

    movb     [d,x], a1
    movb     [d,y], 14,x
    movb     [d,sp], 253,y
    movb     [d,pc], a2,sp
    movb     [d,x], [d,x]
    movb     [d,y], [a5,sp]

    movb     [a1,x], a2
    movb     [a2,y], 13,x
    movb     [a3,sp], 251,y
    movb     [a4,pc], a3,sp
    movb     [a5,x], [d,pc]
    movb     [a6,y], [a5,sp]

;movw
    movw     #a1, a1
    movw     #a2, 0,x
    movw     #a3, -254,y
    movw     #a4, a1,sp
    movw     #a5, [d,x]
    movw     #a6, [a2,sp]

    movw     a1, a2
    movw     a2, 1,x
    movw     a3, 255,y
    movw     a4, a1,sp
    movw     a5, [d,y]
    movw     a6, [a3,sp]

    movw     1,x+, a1
    movw     2,-x, 15,x
    movw     7,sp+, 253,y
    movw     6,-sp, a2,sp
    movw     -15,y, [d,x]
    movw     13,sp, [a5,sp]

    movw     [d,x], a1
    movw     [d,y], 14,x
    movw     [d,sp], 253,y
    movw     [d,pc], a2,sp
    movw     [d,x], [d,x]
    movw     [d,y], [a5,sp]

    movw     [a1,x], a2
    movw     [a2,y], 13,x
    movw     [a3,sp], 251,y
    movw     [a4,pc], a3,sp
    movw     [a5,x], [d,pc]
    movw     [a6,y], [a5,sp]
