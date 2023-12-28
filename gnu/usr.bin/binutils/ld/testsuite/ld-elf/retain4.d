#name: SHF_GNU_RETAIN 4 (keep orphaned sections when not discarding)
#source: retain4.s
#ld: -e _start --gc-sections --orphan-handling=place
#notarget: ![supports_gnu_osabi] ![check_gc_sections_available]
#DUMPPROG: nm

#...
[0-9a-f]+ . orphaned_fn
#pass
