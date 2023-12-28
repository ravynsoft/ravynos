        .section ".text", "ax",@progbits
        .global _start, dest, end_label
        .global func_1, func_2, func_3

_start:
local_start:
        nop
        nop
        nop
        nop
        nop
        .type   _start, @function
        .size   _start, .-_start

        .type   local_start, @function
        .size   local_start, .-local_start

func_1:
local_func_1:
        nop
        nop
        nop
        nop
        nop
        .type   func_1, @function
        .size   func_1, .-func_1

        .type   local_func_1, @function
        .size   local_func_1, .-local_func_1

func_2:
local_func_2:
        nop
        jmp     dest
        jmp     dest
        jmp     dest
        nop
        .type   func_2, @function
        .size   func_2, .-func_2

        .type   local_func_2, @function
        .size   local_func_2, .-local_func_2

func_3:
local_func_3:
        nop
        nop
        nop
        nop
        nop
        .type   func_3, @function
        .size   func_3, .-func_3

        .type   local_func_3, @function
        .size   local_func_3, .-local_func_3

dest:
        nop
        nop
        nop
        nop
        nop

end_label:
local_end_label:
