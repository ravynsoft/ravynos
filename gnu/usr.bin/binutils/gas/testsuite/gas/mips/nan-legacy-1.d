#name: MIPS legacy NaN setting 1
#source: nan-legacy.s
#objdump: -p

.*:.*file format.*mips.*
#failif
private flags = [0-9a-f]*[4-7c-f]..: .*[[]nan2008[]].*
#pass
