#name: Glink test 1 (error) (32-bit)
#source: aix-glink-1.s
#as: -a32
#ld: -b32 -bM:SRE -bnogc
#error: undefined reference to `\.ext'
