#objdump: --section-headers
#as: --generate-missing-build-notes=no
# Minimum code alignment should be set.
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*: +file format.*aarch64.*

Sections:
Idx Name[ ]+Size[ ]+VMA[ ]+LMA[ ]+File off[ ]+Algn
  0 \.text         .*  .*  .*  .*  2\*\*2
                  .*CODE.*
  1 \.data         .*  .*  .* .*  2\*\*0
                  .*DATA.*
  2 \.bss          .*  .*  .*  .*  2\*\*0
.*
