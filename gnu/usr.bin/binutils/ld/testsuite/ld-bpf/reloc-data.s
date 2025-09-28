    .data
foo:
    .string "foo"

    .global bar
bar:
    .string "bar"

d64:
    .quad foo
d32:
    .word d64
