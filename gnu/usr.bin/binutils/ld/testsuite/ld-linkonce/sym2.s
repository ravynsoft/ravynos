 .data
 .global three
three:
 .long 3

 .section .gnu.linkonce.d.foo, "aw"
 .global foo
# PE signature of the comdat group
foo:
 .global onex
onex:
 .long 1
