#name: --set-section-flags test 1 (sections)
#ld: -Tflags1.ld --no-warn-rwx-segments
#objcopy_linked_file: --set-section-flags .post_text_reserve=contents,alloc,load,readonly,code
#readelf: -S --wide

#...
Section Headers:
#...
  \[[ 0-9]+\] \.text.*[ \t]+PROGBITS[ \t0-9a-f]+AX.*
  \[[ 0-9]+\] \.post_text_reserve.*[ \t]+PROGBITS[ \t0-9a-f]+AX.*
#pass
