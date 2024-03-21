#name: SHF_GNU_RETAIN 7b
#source: retain7.s
#ld: -r
#notarget: ![supports_gnu_osabi] ![check_gc_sections_available] tic6x-*-*
#readelf: -h

ELF Header:
#...
  OS/ABI:                            UNIX - (GNU|FreeBSD)
#pass
