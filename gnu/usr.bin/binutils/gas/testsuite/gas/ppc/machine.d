#objdump: -s -j .text
#name: PowerPC .machine test
#notarget: *-*-pe *-*-winnt* *-*-cygwin*

.*

Contents of section \.text:
 0000 (7c11eba6|a6eb117c) (7c100ba6|a60b107c) (4c000066|6600004c) (00000200|00020000) .*
 0010 (44000002|02000044) (4c0000a4|a400004c) (7c000224|2402007c) (4e800020|2000804e) .*
 0020 (7c11eba6|a6eb117c) .*
