.globl align2

.text
.balign 4
align2:
    csrr a0, sie
.fill 0xfffb6
