.globl cc
.globl dd

.text
cc:
    csrr a0, sie
    csrr a1, sie
dd:
    ret
