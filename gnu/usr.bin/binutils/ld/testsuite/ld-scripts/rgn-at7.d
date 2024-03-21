#source: rgn-at6.s
#ld: -T rgn-at7.t
#objdump: -h --wide
# Test that lma is only aligned by script when lma_region!=region.

#...
.* 0+10000 +0+20000 .*
.* 0+10100 +0+20010 .*
