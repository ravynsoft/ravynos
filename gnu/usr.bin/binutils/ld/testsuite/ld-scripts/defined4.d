#ld: -Tdefined4.t
#nm: -B
#source: defined4.s
#xfail: mips*-*-*
# We check that defined and defined1 have the same address.  MIPS targets
# use different address.

# Check that arithmetic on DEFINED works.
#...
0+0 D defined
#...
0+0 D defined1
#pass
