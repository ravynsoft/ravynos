        .section        scnfoo,"aw",%progbits
        .zero 0x10

        .globl  bar
        .data
        .align 8
        .type   bar, %object
        .size   bar, 8
bar:
  .ifdef UNDERSCORE
        .dc.a   ___stop_scnfoo
  .else
        .dc.a   __stop_scnfoo
  .endif
        .dc.a  .sizeof. (scnfoo)
