.macro mknops nr_bytes
    .nops \nr_bytes, 9
.endm

.macro ALTERNATIVE
.L\@_orig_s:
.L\@_orig_e:
     mknops (-(((.L\@_repl_e\()1 - .L\@_repl_s\()1) - (.L\@_orig_e - .L\@_orig_s)) > 0) * ((.L\@_repl_e\()1 - .L\@_repl_s\()1) - (.L\@_orig_e - .L\@_orig_s)))
.L\@_orig_p:

    .section .discard, "a", @progbits
    .byte (.L\@_orig_p - .L\@_orig_s)
    .byte 0xff + (.L\@_repl_e\()1 - .L\@_repl_s\()1) - (.L\@_orig_p - .L\@_orig_s)

    .section .altinstr_replacement, "ax", @progbits
.L\@_repl_s\()1:
.L\@_fill_rsb_loop:
    jnz .L\@_fill_rsb_loop
    mov %eax, %esp
.L\@_repl_e\()1:
.endm

	.text
_start:
ALTERNATIVE
