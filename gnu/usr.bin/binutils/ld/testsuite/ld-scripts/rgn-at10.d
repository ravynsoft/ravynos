#source: rgn-at10.s
#ld: -T rgn-at10.t
#objdump: -h --wide
#xfail: hppa*64*-*-hpux* v850*-*-*
# Test that lma is adjusted in case the section start vma is aligned and
# lma_region != region if requested by script.  Make sure this works with
# non-load sections.

#...
.* 0+10000 +0+20000 .*
.* 0+10100 +0+20100 .*
.* 0+10100 +0+20100 .*
