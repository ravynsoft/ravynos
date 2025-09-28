#name: SHF_GNU_RETAIN 6b (pull section out of lib required by SHF_GNU_RETAIN section)
#source: retain6main.s
#ld: --gc-sections -e _start -u bar -Ltmpdir -lretain6
#notarget: ![supports_gnu_osabi] ![check_gc_sections_available]
#DUMPPROG: nm

#failif
#...
[0-9a-f]+ . .*discard.*
#...
