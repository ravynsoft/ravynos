#source: aix-tls-section.s
#as: -a64
#ld: -b64 -shared -bE:aix-tls-section.ex
#objdump: -hw
#target: [is_xcoff_format]

.*

Sections\:
.*
  0 \.text         .*  .*  .*  .*  .*  ALLOC, LOAD, CODE
  1 \.tdata        00000008  ffffffffffff8800  ffffffffffff8800  .*  .*  CONTENTS, ALLOC, LOAD, DATA, THREAD_LOCAL
  2 \.tbss         00000008  ffffffffffff8808  ffffffffffff8808  .*  .*  ALLOC, THREAD_LOCAL
  3 \.data         .* .* .*  .*  .*  ALLOC, LOAD, DATA
#...
