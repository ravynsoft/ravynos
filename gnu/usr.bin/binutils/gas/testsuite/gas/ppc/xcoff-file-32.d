#as: -a32
#source: xcoff-file.s
#objdump: -t
#name: XCOFF file test (32-bit)

.*

SYMBOL TABLE:
\[  0\].*\(scl 103\) \(nx 4\) .* file.s
File 
File ftype 1 fname "A long string"
File ftype 2 fname "short"
File ftype 128 fname "Another long string inside the strign table."
