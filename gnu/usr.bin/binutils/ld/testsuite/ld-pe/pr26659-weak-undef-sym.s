        .text
        .globl  foo
        .def    foo;    .scl    2;      .type   32;     .endef
foo:
        pushq   %rbp
        movq    %rsp, %rbp
        subq    $32, %rsp
        movl    %ecx, 16(%rbp)
        movq    .refptr.bar1(%rip), %rax
        testq   %rax, %rax
        je      .L2
        call    bar1
.L2:
        movq    .refptr.bar2(%rip), %rax
        testq   %rax, %rax
        je      .L3
        call    bar2
.L3:
        movl    16(%rbp), %eax
        imull   %eax, %eax
        addq    $32, %rsp
        popq    %rbp
        ret
        .weak   bar2
        .weak   bar1
        .def    bar1;   .scl    2;      .type   32;     .endef
        .def    bar2;   .scl    2;      .type   32;     .endef
        .section        .rdata$.refptr.bar2, "dr"
        .globl  .refptr.bar2
        .linkonce       discard
.refptr.bar2:
        .quad   bar2
        .section        .rdata$.refptr.bar1, "dr"
        .globl  .refptr.bar1
        .linkonce       discard
.refptr.bar1:
        .quad   bar1

