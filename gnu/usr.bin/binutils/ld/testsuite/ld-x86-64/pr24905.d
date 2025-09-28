#as: --64
#ld: -shared -melf_x86_64 $srcdir/$subdir/pr24905.t
#nm: -n

#...
[0-9a-f]* t EXTERNAL_SYM
#pass
