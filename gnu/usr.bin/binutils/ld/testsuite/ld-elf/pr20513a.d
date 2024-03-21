#source: pr20513a.s
#source: pr20513b.s
#ld:
#readelf: -S --wide

#failif
#...
[ 	]*\[.*\][ 	]+\..text\.exclude[ 	]+.*
#...
