        .eabi_attribute 6, 0  @Tag_CPU_arch, 0 means pre-v4.
        .file   "f.c"
        .text
        .align  2
        .global foo
        .type   foo, %function
foo:
        bx      lr
