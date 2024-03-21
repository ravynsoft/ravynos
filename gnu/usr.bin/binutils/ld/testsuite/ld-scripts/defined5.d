#ld: -Tdefined5.t
#warning: .*multiple definition of `defined'.*
#nm: -B

# Check that a script can override an object file symbol, if multiple
# definitions are allowed.  See pr12356.
#...
0+1000 D defined
#pass
