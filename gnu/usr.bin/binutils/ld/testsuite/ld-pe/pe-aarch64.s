# A little test to ensure pe-aarch64 is working in LD.
# Currently, the poor pe-aarch64 implementation in binutils
# couldn't do anything useful, hence, this test is rather short

.section .text

_start:
    mov x1, 20
    b foo
foo:
    ret
