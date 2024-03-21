#readelf: -h -S --wide
#name: SHF_GNU_RETAIN sections 29
#notarget: ![supports_gnu_osabi]

#...
 +OS/ABI: +UNIX - (GNU|FreeBSD)
#...
  \[..\] \.rodata\.str1\.1 +PROGBITS +[0-9a-f]+ [0-9a-f]+ 0+4 +01 +AMS +0 +0 +1
#...
  \[..\] \.rodata\.str1\.1 +PROGBITS +[0-9a-f]+ [0-9a-f]+ 0+4 +01 +AMSR +0 +0 +1
#pass
