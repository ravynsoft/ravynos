#name: MIPS 2008 NaN setting 3
#source: nan-2008-override.s
#objdump: -p

.*:.*file format.*mips.*
private flags = [0-9a-f]*[4-7c-f]..: .*[[,]nan2008[],].*
#pass
