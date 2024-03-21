#objdump: -t
#name: .file file names ordering

.*: .*

SYMBOL TABLE:
#...
0+ l[ ]*df \*ABS\*[ 	]+0+ file-2\.s
#...
0+ l[ ]*\.text[ 	]+0+ local1
0+ l[ ]*df \*ABS\*[ 	]+0+ aux-1\.s
0+ l[ ]*\.text[ 	]+0+ local2
0+ l[ ]*df \*ABS\*[ 	]+0+ aux-2\.s
0+ l[ ]*\.text[ 	]+0+ local3
#pass
