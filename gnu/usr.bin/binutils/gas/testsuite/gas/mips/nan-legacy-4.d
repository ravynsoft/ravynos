#name: MIPS legacy NaN setting 4
#source: empty.s
#objdump: -p
#as: -mnan=legacy

.*:.*file format.*mips.*
#failif
private flags = [0-9a-f]*[4-7c-f]..: .*[[]nan2008[]].*
#pass
