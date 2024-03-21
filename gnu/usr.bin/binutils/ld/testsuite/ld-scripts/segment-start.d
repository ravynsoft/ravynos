#name: SEGMENT_START expression not absolute
#source: segment-start.s
#ld: -e 0 -u __executable_start -T segment-start.ld
#nm: -B
#xfail: mmix-*-* pdp11-*-*
#xfail: c54x*-*-*coff* tic54x-*-*coff*
# XFAIL targets that are not expected to handle SEGMENT_START correctly.

# Make sure `__executable_start' is regular:
#
# 10000000 T __executable_start
#
# not absolute:
#
# 10000000 A __executable_start

#...
0*10000000 T __executable_start
#pass
