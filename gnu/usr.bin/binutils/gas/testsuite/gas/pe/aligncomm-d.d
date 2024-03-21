#objdump: -s -j .drectve
#name: aligned common D

# Test the aligned form of the .comm pseudo-op.

.*: .*

Contents of section .drectve:
 0000 202d616c 69676e63 6f6d6d3a 225f6822   -aligncomm:"_h"
 0010 2c38202d 616c6967 6e636f6d 6d3a225f  ,8 -aligncomm:"_
 0020 69222c34 202d616c 69676e63 6f6d6d3a  i",4 -aligncomm:
 0030 225f6a22 2c32202d 616c6967 6e636f6d  "_j",2 -aligncom
 0040 6d3a225f 6b222c31 ........ ........  m:"_k",1.*
