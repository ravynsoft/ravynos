#source: property-x86-ibt.s
#as: --32 -mx86-used-note=yes
#ld: -r -m elf_i386 -z cet-report=error -z ibt
#error: .*: error: missing SHSTK property
