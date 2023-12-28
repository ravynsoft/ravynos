        .section        scnfoo,"aw",%progbits
        .zero 0x10

        .globl  bar
        .data
        .type   bar, %object
bar:
  .ifdef UNDERSCORE
        .dc.a   ___stop_scnfoo
  .else
        .dc.a   __stop_scnfoo
  .endif
