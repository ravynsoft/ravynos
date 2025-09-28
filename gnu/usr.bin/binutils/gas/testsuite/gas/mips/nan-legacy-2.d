#name: MIPS legacy NaN setting 2
#source: nan-legacy.s
#objdump: -p
#as: -mnan=2008

.*:.*file format.*mips.*
#failif
private flags = [0-9a-f]*[4-7c-f]..: .*[[]nan2008[]].*
#pass
