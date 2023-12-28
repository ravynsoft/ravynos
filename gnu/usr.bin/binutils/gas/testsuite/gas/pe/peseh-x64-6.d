#source: peseh-x64-4.s
#objdump: -h -j .pdata\$_ZN5VBase1fEv
#name: PE x64 SEH test sections flags pdata

.*: .*

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  5 .pdata\$_ZN5VBase1fEv 0000000c  0000000000000000  0000000000000000  000007ac  2\*\*2
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, DATA, LINK_ONCE_DISCARD
