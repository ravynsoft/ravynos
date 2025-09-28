; Test more error cases for constant ranges.

;  { dg-do assemble { target cris-*-* } }

 .text
start:
 asrq 63,$r0 ; { dg-error "mmediate value not in 5 bit unsigned range: 63" }
 move 65536,$p0 ; { dg-error "mmediate value not in 8 bit range: 65536" }
 move 65536,$p5 ; { dg-error "mmediate value not in 16 bit range: 65536" }
 bdap.b 65536,$r0 ; { dg-error "mmediate value not in 8 bit signed range: 65536" }
