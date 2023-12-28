.globl _start, foo, bar

.section ".text.start", "ax"
_start:
    bl foo
    .space 0x7000000

.section ".text.bar", "ax"
bar:
    .space 0x1000000
    .size bar, .-bar

.section ".text.foo", "ax"
foo:
    b _start
