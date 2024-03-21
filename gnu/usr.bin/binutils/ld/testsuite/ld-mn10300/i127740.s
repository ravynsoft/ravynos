      .section .text
       .global _main
       .global _dummy
_main:
       mov     _g_label,d1     # instruction is changed by relaxations

       .balign 0x100
_dummy:
       .long     _dummy
       ret [],0
       .size   _main, .-_main
       .comm   _g_label,4,4
