#source: rgn-at6.s
#ld: -T rgn-at9.t
#objdump: -h --wide
#xfail: rx-*-*
# Test that lma is adjusted in case the section start vma is aligned and
# lma_region != region if requested by script.

#...
.* 0+10000 +0+20080 .*
.* 0+10100 +0+20180 .*
