#ld: --warn-unresolved
#warning: \(\.text\+0x8\): warning: undefined reference to `foo'
#objdump: -sj.rel.dyn -sj.got

.*

Contents of section \.got:
 *[^ ]* 00000000 00000000 00000000 00000000  .*
