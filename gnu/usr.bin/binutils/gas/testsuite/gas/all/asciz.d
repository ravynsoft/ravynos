#objdump: -s -j .data -j "\$DATA\$"
#name: Generation of NUL terminated strings
# The TIC4x and TIC5x assemblers do not support the concatenation of space separated strings.
#xfail: tic4*-* tic5*-*

.*: +file format .*

Contents of section (\.data|\$DATA\$):
.*ab\.cd\..*
#pass
