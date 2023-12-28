 .data
 .global four
four:
 .long 4

 .section .gnu.linkonce.d.foo, "aw"
 .global foo
# PE signature of the comdat group
foo:
 .global oney
oney:
 .long 1
