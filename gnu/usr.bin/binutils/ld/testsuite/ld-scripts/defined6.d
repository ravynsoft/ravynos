#source: defined6.s
#ld: -T defined6.t
#nm: -B

# Check that DEFINED works with various symbol types
#...
0+1 A common
0+1 A common_post
0+1 A common_pre
0+1 A defined
0+1 A defined_post
0+1 A defined_pre
0+1 A undef
0+1 A undef_post
0+0 A undef_pre
0+1 A undefweak
0+1 A undefweak_post
0+0 A undefweak_pre
0+1 A weak
0+1 A weak_post
0+1 A weak_pre
