#objdump: -j ".ARM.extab" -s
#name: Unwind information for Armv8.1-M.Mainline PACBTI extension
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince
# VxWorks needs a special variant of this file.
#skip: *-*-vxworks*

.*:     file format.*

Contents of section .ARM.extab:
 0000 (84b40281 84b4a300 b0a8b400|8102b484 00a3b484 00b4a8b0) 00000000  .*
