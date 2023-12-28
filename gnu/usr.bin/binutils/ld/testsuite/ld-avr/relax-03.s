        .section ".text", "ax",@progbits
        .global _start, end_label

_start:
local_start:
        nop
        jmp     end_label
        jmp     end_label
        jmp     end_label
        nop
        .type   _start, @function
        .size   _start, .-_start

        .type   local_start, @function
        .size   local_start, .-local_start

end_label:
local_end_label:
