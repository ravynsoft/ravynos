.equ EXIT_SUCCESS, 0
.equ STDOUT, 1
.equ SYS_exit, 93
.equ SYS_write, 64

.section .rodata
msg:
    .string "hello, world!\n"
    len = . - msg

.text
    .globl  _start
_start:
    li.w        $a2, len
    la.local    $a1, msg
    li.w        $a0, STDOUT
    li.w        $a7, SYS_write
    syscall     0x0

    li.w        $a0, EXIT_SUCCESS
    li.w        $a7, SYS_exit
    syscall     0x0
