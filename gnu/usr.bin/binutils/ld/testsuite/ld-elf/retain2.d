#name: SHF_GNU_RETAIN 2 (remove SHF_GNU_RETAIN sections by placing in /DISCARD/)
#source: retain1.s
#ld: -e _start -Map=retain2.map --gc-sections --script=retain2.ld
#map: retain2.map
#notarget: ![supports_gnu_osabi] ![check_gc_sections_available]
