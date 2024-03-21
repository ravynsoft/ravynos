#name: Glink test 3 (error) (32-bit)
#source: aix-glink-3.s
#as: -a32
#ld: -b32 -bnoautoimp tmpdir/aix-glink-3b.so
#error: undefined reference to `\.g'
