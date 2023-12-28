    .data
x:
    .string "foo"
y:
    .string "bar"

    .text
main:
    lddw %r1, y
