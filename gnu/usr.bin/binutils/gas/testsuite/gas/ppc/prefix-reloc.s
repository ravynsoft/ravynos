 .text
 pli 9,ext@ha
 sldi 9,9,34
 paddi 9,9,ext@l
 pld 3,ext@pcrel
 pld 4,ext@got@pcrel
 pld 5,ext@plt@pcrel
0: pld 6,ext-0b(0),1
 pld 7,ext(0),0
# The following insn will need an alignment nop, testing the behaviour
# of "dot" in the expression.  Don't stupidly edit this file and lose
# the nop.
 pld 8,ext-.(0),1
