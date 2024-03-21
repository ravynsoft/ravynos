.syntax unified

.type f, %function
e:
        b       . - 0xfffffe   @ gas mis-assembles as a forward branch
        b       . - 0xfffffc
        b       . + 0x1000002
        b       . + 0x1000004  @ gas mis-assembles as a backward branch
        b.w     . + 0x2000002  @ gas mis-assembles as a backward branch

f:      b       g              @ gas mis-assembles as a backward branch

        .space 0x1fffff0

g:      b       f              @ gas mis-assembles as a forward branch


