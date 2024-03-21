 .data
 .global two
two:
 .long 2

 .section .gnu.linkonce.d.foo, "aw"
 .global foo
# PE signature of the comdat group
foo:
 .global one
one:
 .long 1
