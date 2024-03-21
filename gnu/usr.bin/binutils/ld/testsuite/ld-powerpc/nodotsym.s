 .section .opd,"aw"
 .p2align 3
 .global foo
 .type foo,@function
foo:
 .quad .L.foo,.TOC.@tocbase,0

 .text
.L.foo:
 blr
 .size foo,.-.L.foo
