#source: pr20513b.s
#source: pr20513a.s
#ld:
#readelf: -S --wide

#failif
#...
[ 	]*\[.*\][ 	]+\..text\.exclude[ 	]+.*
#...
