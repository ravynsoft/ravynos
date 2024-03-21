#source: property-x86-shstk.s
#as: --32 -mx86-used-note=yes
#ld: -r -m elf_i386 -z cet-report=error -z shstk
#error: .*: error: missing IBT property
