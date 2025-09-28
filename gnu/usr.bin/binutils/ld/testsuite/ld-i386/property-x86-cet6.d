#source: start.s
#as: --32 -mx86-used-note=no
#ld: -r -m elf_i386 -z cet-report=warning
#warning: .*: warning: missing IBT and SHSTK properties
#readelf: -n
