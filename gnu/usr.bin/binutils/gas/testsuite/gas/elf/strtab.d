#readelf: -W -x .strtab
#name: .strtab section

#failif
#...
 +0x[0-9 ]+.*\.xxxx\..*
#...
