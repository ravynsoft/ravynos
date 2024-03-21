#source: vaddr.s
#name: non-zero p_vaddr
#ld: -pie -Ttext-segment 0x7000000 -z max-page-size=0x200000
#readelf: -h

ELF Header:
#...
  Type:                              EXEC \(Executable file\)
#pass
