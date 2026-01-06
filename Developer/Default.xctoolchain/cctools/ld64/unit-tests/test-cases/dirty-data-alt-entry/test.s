




    .data
_foo:  .quad 0,0,0

# note: the .alt_entry means sym2 needs to stay pinned to sym1
    .globl _sym1
    .globl _sym2
    .alt_entry _sym2

_sym1:  .quad 0,0
_sym2:  .quad 0,0

    .globl _sym3
_sym3:  .quad 0,0


    .subsections_via_symbols


