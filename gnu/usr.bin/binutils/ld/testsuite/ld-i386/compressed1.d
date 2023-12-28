#name: undefined symbol with compressed debug sections
#as: --32
#ld: -e foo -melf_i386 --noinhibit-exec
#warning: .*/compressed1.c:13:\(.*\): undefined reference to .bar.
#nm: -n

#failif
#...
[ \t]+U bar
#...
