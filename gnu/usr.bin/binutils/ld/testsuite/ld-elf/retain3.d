#name: SHF_GNU_RETAIN 3 (keep sections referenced by retained sections)
#source: retain3.s
#ld: -e _start --gc-sections
#notarget: ![supports_gnu_osabi] ![check_gc_sections_available]
#DUMPPROG: nm

#...
[0-9a-f]+ . bar
#...
[0-9a-f]+ . foo
#pass
