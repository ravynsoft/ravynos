#name: MIPS legacy NaN setting 5
#source: empty.s
#objdump: -p

.*:.*file format.*mips.*
#failif
private flags = [0-9a-f]*[4-7c-f]..: .*[[]nan2008[]].*
#pass
