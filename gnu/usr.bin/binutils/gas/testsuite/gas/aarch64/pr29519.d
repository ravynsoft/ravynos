# Check that AArch64 specific pseudo-ops can be separated by the ; line separator character.
#name: PR29519 (Separating AArch64 pseudo-ops with ;)
#objdump: -rd
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+0 <\.text>:
.*\.word[ 	]+0x0+0
#pass
