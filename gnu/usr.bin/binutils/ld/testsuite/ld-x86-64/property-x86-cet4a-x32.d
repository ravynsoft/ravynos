#source: property-x86-shstk.s
#as: --x32 -mx86-used-note=yes
#ld: -r -m elf32_x86_64 -z cet-report=error
#error: .*: error: missing IBT property
