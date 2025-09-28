#name: SHF_GNU_RETAIN set with numeric flag value in .section
#source: section23.s
#target: [supports_gnu_osabi]
#readelf: -h -S --wide

#...
 +OS/ABI: +UNIX - (GNU|FreeBSD)
#...
  \[..\] .data.retain_var[ 	]+PROGBITS[ 	]+[0-9a-f]+ [0-9a-f]+ [0-9a-f]+ 00 WAR.*
#pass
