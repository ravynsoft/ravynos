#name: Glink test 3 (error) (64-bit)
#source: aix-glink-3.s
#as: -a64
#ld: -b64 -bnoautoimp tmpdir/aix64-glink-3b.so
#error: undefined reference to `\.g'
