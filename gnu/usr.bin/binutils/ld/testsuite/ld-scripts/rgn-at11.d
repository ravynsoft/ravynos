#source: rgn-at11.s
#ld: -T rgn-at11.t
#objdump: -h --wide
#xfail: hppa*64*-*-hpux* v850*-*-*
# Test that lma is not adjusted in case the section start vma is aligned and
# lma_region != region if not requested by script.

#...
.* 0+10000 +0+20000 .*
.* 0+10100 +0+20010 .*
.* 0+10100 +0+20010 .*
