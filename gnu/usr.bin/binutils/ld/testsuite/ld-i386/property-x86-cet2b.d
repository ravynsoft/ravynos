#source: property-x86-empty.s
#source: property-x86-ibt.s
#source: property-x86-shstk.s
#source: property-x86-3.s
#as: --32 -mx86-used-note=yes
#ld: -r -m elf_i386 -z cet-report=error
#error: .*: error: missing IBT and SHSTK properties.*: error: missing SHSTK property.*: error: missing IBT property.*: error: missing IBT and SHSTK properties
