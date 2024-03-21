        .section        scnfoo,"aw",%progbits
        .zero 0x10

        .globl  bar
        .data
        .type   bar, %object
bar:
  .ifdef UNDERSCORE
        .dc.a   ___start_scnfoo
  .else
        .dc.a   __start_scnfoo
  .endif
