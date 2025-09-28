#name: MIPS 2008 NaN setting 4
#source: empty.s
#objdump: -p
#as: -mnan=2008

.*:.*file format.*mips.*
private flags = [0-9a-f]*[4-7c-f]..: .*[[,]nan2008[],].*
#pass
