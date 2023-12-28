#source: start.s
#as: --64 -defsym __64_bit__=1 -mx86-used-note=no
#ld: -r -melf_x86_64 -z cet-report=warning
#warning: .*: warning: missing IBT and SHSTK properties
#readelf: -n
