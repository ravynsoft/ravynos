#source: peseh-x64-4.s
#objdump: -h -j .xdata\$_ZN5VBase1fEv
#name: PE x64 SEH test sections flags xdata

.*: .*

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  4 .xdata\$_ZN5VBase1fEv 00000008  0000000000000000  0000000000000000  000007a4  2\*\*2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA, LINK_ONCE_DISCARD
