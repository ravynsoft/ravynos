#notoc call stubs, no pcrel insns
 .text
 .abiversion 2
 .machine power10
_start:
 bl f1
 nop
 bl f1@notoc
 bl f2@notoc
