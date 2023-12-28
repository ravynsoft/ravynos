#name: MIPS 2008 NaN setting 2
#source: nan-2008.s
#objdump: -p
#as: -mnan=legacy

.*:.*file format.*mips.*
private flags = [0-9a-f]*[4-7c-f]..: .*[[,]nan2008[],].*
#pass
