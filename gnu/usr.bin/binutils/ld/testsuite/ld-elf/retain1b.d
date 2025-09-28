#name: SHF_GNU_RETAIN 1b
#source: retain1.s
#ld: -e _start --gc-sections
#notarget: ![supports_gnu_osabi] ![check_gc_sections_available]
#nm: -n

#failif
#...
[0-9a-f]+ . .*discard.*
#...
