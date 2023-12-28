#source: property-x86-ibt.s
#as: --x32 -mx86-used-note=yes
#ld: -r -m elf32_x86_64 -z cet-report=error -z ibt
#error: .*: error: missing SHSTK property
