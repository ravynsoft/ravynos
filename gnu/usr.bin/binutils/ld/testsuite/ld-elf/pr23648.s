 .text
 .global entry, _entry
entry:
_entry:
 .dc.a foo

 .section .text.test0,"ax",%progbits
 .global test0
test0:
 .dc.a 0

 .section .text.test1,"ax",%progbits
 .global test1
test1:
 .dc.a 1
