#notoc call stubs, pcrel insns
 .text
 .abiversion 2
_start:
 bl f1
 nop
 bl f1@notoc
 bl f2@notoc
 pld 3,ext@got@pcrel
