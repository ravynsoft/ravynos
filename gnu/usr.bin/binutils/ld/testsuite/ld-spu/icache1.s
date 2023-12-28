 .text
 .globl _start
 .type _start,@function
_start:
 ilhu $3,f5@h
 iohl $3,f5@l
 br f1

 .data
 .word f1, f2, f3, f4

 .section ".f1.part1","ax",@progbits
 .globl f1
 .type f1,@function
f1:
 nop
 stqd $0,16($1)
 ai $1,$1,-64
 stqd $1,0($1)
 brsl $0,f2
 brsl $0,f3
 .fill 800
 br .Lf1.part2
 .size f1,.-f1

 .section ".f1.part2","ax",@progbits
.Lf1.part2:
 ai $1,$1,64
 lqd $0,16($1)
 bi $0
 .fill 800
 .size .Lf1.part2,.-.Lf1.part2

 .section ".f2.part1","ax",@progbits
 .globl f2
 .type f2,@function
f2:
 ai $1,$1,-128
 stqd $1,0($1)
 .fill 512
 ai $1,$1,128
 bi $0
 .size f2,.-f2

 .section ".f3.part1","ax",@progbits
 .type f3,@function
f3:
 .fill 256
 bi $0
 .size f3,.-f3

 .section ".f4.part1","ax",@progbits
 .type f4,@function
f4:
 stqd $(0),16($1)
 stqd $1,-512($1)
 ai $1,$1,-512
 brasl $0,f2
 .fill 800
 br .Lf4.part2
 .size f4,.-f4

 .section ".f4.part2","ax",@progbits
.Lf4.part2:
#alloca
 ilhu $3,-4000@h
 iohl $3,-4000@l
 a $4,$1,$3
 lnop
 ori $1,$4,0
 stqd $4,0($4)
 brsl $0,f5
#recursion
 brsl $0,f4
 lqd $1,0($1)
 .fill 800
 br .Lf4.part3
 .size .Lf4.part2,.-.Lf4.part2

 .section ".f4.part3","ax",@progbits
.Lf4.part3:
#recursion
 brasl $0,f4
 brsl $0,f4
 .fill 800
 br .Lf4.part4
 .size .Lf4.part3,.-.Lf4.part3

 .section ".f4.part4","ax",@progbits
.Lf4.part4:
 .fill 800
 ila $3,512
 a $1,$1,$3
 lqd $0,16($1)
#sibling call
 bra f5
 .size .Lf4.part4,.-.Lf4.part4

 .section ".f5.part1","ax",@progbits
 .type f5,@function
f5:
 stqd $(0),16($1)
 stqd $1,-512($1)
 ai $1,$1,-512
 brsl $0,f5
 .fill 800
 ila $3,512
 a $1,$1,$3
 lqd $0,16($1)
 bi $0
 .size f5,.-f5
