#source: start.s
#as: --x32 -mx86-used-note=no
#ld: -r -m elf32_x86_64 -z cet-report=warning
#warning: .*: warning: missing IBT and SHSTK properties
#readelf: -n
