#source: ../../../elf/file.s
#objdump: -t
#name: .file file names

.*: .*

SYMBOL TABLE:
0+ l[ ]*df \*ABS\*[ 	]+0+ ~tilde
#...
0+ l[ ]*df \*ABS\*[ 	]+0+ hash\#
0+ l[ ]*df \*ABS\*[ 	]+0+ lower
0+ l[ ]*df \*ABS\*[ 	]+0+ UPPER
0+ l[ ]*df \*ABS\*[ 	]+0+ :colon
0+ l[ ]*df \*ABS\*[ 	]+0+ /dir/file\.s
0+ l[ ]*df \*ABS\*[ 	]+0+ \[brackets\]
0+ l[ ]*df \*ABS\*[ 	]+0+ \{braces\}
0+ l[ ]*df \*ABS\*[ 	]+0+ slash/data
0+ l[ ]*df \*ABS\*[ 	]+0+ file\.s
#pass
