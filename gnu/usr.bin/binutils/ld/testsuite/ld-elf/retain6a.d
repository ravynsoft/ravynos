#name: SHF_GNU_RETAIN 6a (pull section out of lib required by SHF_GNU_RETAIN section)
#source: retain6main.s
#ld: --gc-sections -e _start -u bar -Ltmpdir -lretain6
#notarget: ![supports_gnu_osabi] ![check_gc_sections_available]
#DUMPPROG: nm

#...
[0-9a-f]+ . bar
#...
[0-9a-f]+ . retain_from_lib
#...
[0-9a-f]+ . retained_var
#pass
