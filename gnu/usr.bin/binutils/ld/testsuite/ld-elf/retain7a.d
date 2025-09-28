#name: SHF_GNU_RETAIN 7a
#source: retain7.s
#ld: -e _start --gc-sections
# hppa-linux chooses ELFOSABI_GNU regardless of GNU feature use
#notarget: ![supports_gnu_osabi] ![check_gc_sections_available] hppa-*-linux*
#readelf: -h

#failif
ELF Header:
#...
  OS/ABI:                            UNIX - GNU
#pass
