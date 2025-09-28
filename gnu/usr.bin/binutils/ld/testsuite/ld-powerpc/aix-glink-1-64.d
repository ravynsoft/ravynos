#name: Glink test 1 (error) (64-bit)
#source: aix-glink-1.s
#as: -a64
#ld: -b64 -bM:SRE -bnogc
#error: undefined reference to `\.ext'
