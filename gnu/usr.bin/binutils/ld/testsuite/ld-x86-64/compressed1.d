#name: undefined symbol with compressed debug sections
#as: --64
#ld: -e foo -melf_x86_64 --noinhibit-exec
#warning: .*/compressed1.c:13:\(.*\): undefined reference to .bar.
#nm: -n

#failif
#...
[ \t]+U bar
#...
