#source: aix-tls-section.s
#as: -a32
#ld: -b32 -shared -bE:aix-tls-section.ex
#objdump: -hw
#target: [is_xcoff_format]

.*

Sections:
.*
  0 \.text         .*  .*  .*  .*  .*  ALLOC, LOAD, CODE
  1 \.tdata        00000008  ffff8800  ffff8800  .*  .*  CONTENTS, ALLOC, LOAD, DATA, THREAD_LOCAL
  2 \.tbss         00000008  ffff8808  ffff8808  .*  .*  ALLOC, THREAD_LOCAL
  3 \.data         .*  .*  .*  .*  .*  ALLOC, LOAD, DATA
#...
