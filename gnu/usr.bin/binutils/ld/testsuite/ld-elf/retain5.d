#name: SHF_GNU_RETAIN 5 (don't pull SHF_GNU_RETAIN section out of lib)
#source: retain5main.s
#ld: --gc-sections -e _start -Ltmpdir -lretain5 -Map=retain5.map
#notarget: ![supports_gnu_osabi] ![check_gc_sections_available]
#map: retain5.map
#DUMPPROG: nm

#failif
#...
[0-9a-f]+ . foo
#...
