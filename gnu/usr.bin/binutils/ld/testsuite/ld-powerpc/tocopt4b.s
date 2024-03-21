 .section .toc,"aw"
 .globl xt
xt:
 .quad x
#
# This testcase will fail with a warning "xt defined on removed toc entry"
# if a large-toc access like the following is added to this file, because
# toc analysis only considers toc accesses from the current object file.
# The small-toc access from tocopt4a.s doesn't cause xt entry to be marked
# !can_optimize.  The testcase only passes because toc analysis considers
# *no* access from the current file as being sufficiently unusual to
# warrant keeping the toc entry.  So, if you use global symbols on toc
# entries, don't mix code models.
#
# .text
# addis 9,2,xt@toc@ha
# ld 9,xt@toc@l(9)
