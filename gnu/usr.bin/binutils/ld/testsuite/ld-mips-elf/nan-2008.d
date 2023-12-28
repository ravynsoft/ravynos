#source: nan-2008.s
#source: nan-2008.s
#ld: -r
#objdump: -p

.*:.*file format.*mips.*
private flags = [0-9a-f]*[4-7c-f]..: .*[[,]nan2008[],].*
#pass
