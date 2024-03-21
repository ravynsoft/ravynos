 .text
 .global x
 .type	x,@function
x:
 move.d expobj2:GOT,$r10
 move.d expobj2:PLT,$r10
 move.d expfn2:GOT,$r10
 move.d expfn2:PLT,$r10
.Lfe1:
 .size	x,.Lfe1-x

