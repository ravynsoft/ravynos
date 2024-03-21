// Test file for AArch64 GAS -- basic integer instructions

func:
          lsl      x1, x2, x3
          lsl      x1, x2, #0
          lsl      x1, x2, #1

          extr	   x1, x2, x3, #1
          extr     x1, x2, x3, #63

          extr     x1, x2, x3, #0
          extr     w1, w2, w3, #31

          CSET     x1, eq
          CSETM    x1, eq

          subs     w1,w1,#0
          cmp      w1,#0

          neg      w1,w2
          sub      w1,w2,#0

          cmp      x1,#0
          subs     x1,x1,#0

          orr      w1,wzr,#15
          mov      x1,x2

          ldr      w1, sp
          ldr      w1, =sp
          ldr      x1, =sp
sp:       .word    0x12345678

          ret      x30
          ret
          ret      x2

          add      sp,x1,x2

          add      x5,x5,#0x7, lsl #12

          add      x1,x2,x3, lsr #1
          add      x5,x5,#0x7

          subs     w1,w1,#1

          movz     x2,#0x64
          movz     x2,#0x64, lsl #0
          movz     x2,#:abs_g0:0x64
          movz     x2,#0x64, lsl #16
          movz     x2,#:abs_g1:(0x64 << 16)
          movz     x2,#0x64, lsl #32
          movz     x2,#:abs_g2:(0x64 << 32)
          movz     x2,#0x64, lsl #48
          movz     x2,#:abs_g3:(0x64 << 48)
          movz     w1,#0x64
          movz     w1,#0x64, lsl #0
          movz     w1,#0x64, lsl #16

          and      x1,x2,x3
          and      w30,w10,w15
          and      w1,w2,#1

          and      x1,x2,x3, lsr #1

          orr      w1,w1,#1
          orr      w1,w1,#1
          orr      x1,x1,#1
          and      x1,x2,#0xf
          and      w1,w2,#0xf
          and      x1,x2,#0x80000000
          and      w1,w2,#0x80000000
          and      x1,x2,#0x800000000

          // 00010010000101000000010100000011
          //    1   2   1   4   0   5   0   3
          and      x5,x4,#0xf

          bic      w1,w2,w3
          bic      x1,x2,x3

1:        b.ne     1b
          b        1b
          b        2f
2:        b.eq     1b


3:        bne      3b
          b        3b
          b        4f
4:        beq      3b

          br       x2

          bcs      4b
          bcc      4b

          .if 0

          lsl      x1, #0, #1

          ext      x1, x2, x3, #64
          ext      w1, w2, w3, #63
          ext      w1, w2, w3, #32

          mov      w1,#10
          neg      w1,#1

          ldm      {x1},[sp]
          ldm      {x1-x2},[sp]
          ldm      {x1,x2,x3,x4},sp
          ldm      {x1-x3},[x1,w2]

          subs     #0,#1

          add      x5,x5,#0x7, lsl #1
          add      x5,x5,#0x7, lsr #1

          movz     x0,#0x64, lsl #1
          movz     x0,#0x64, lsl #2
          movz     x0,#0x64, lsl #3
          movz     x0,#0x64, lsl #4
          movz     x0,#0x64, lsl #64
          movz     w1,#0x64, lsl #32
          movz     w1,#0x64, lsl #48

          orr      #0,w1
          and      sp,x1,x2
          and      x1,sp,x2
          and      x1,x2,sp

          and      w1,#0,x2
          and      x1,#0,w2

          and      x1,x2,w3
          and      x1,w2,x3
          and      x1,w2,w3
          and      w1,x2,x3
          and      w1,x2,w3
          and      w1,w2,x3
          and      w1,w2,w3

          and      x1,x2,#0
          and      w1,w2,#0x800000000
          bic      x1,x2,#1

          br       w2
          br       sp
          .endif

          .equ     sh,2
