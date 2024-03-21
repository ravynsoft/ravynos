#source: property-x86-ibt.s
#as: --64 -defsym __64_bit__=1 -mx86-used-note=yes
#ld: -r -melf_x86_64 -z cet-report=error
#error: .*: error: missing SHSTK property
