#source: bc.s
#as: -mpower8 --defsym POWER8=1 --defsym AT=1
#objdump: -d -Mpower8

.*:     file format .*

Disassembly of section \.text:

0+ <\.text>:
   0:	(40 00 00 00|00 00 00 40) 	bdnzf   lt,0 .*
   4:	(40 40 00 00|00 00 40 40) 	bdzf    lt,4 .*
   8:	(40 80 00 00|00 00 80 40) 	bge     8 .*
   c:	(40 c0 00 00|00 00 c0 40) 	bge-    c .*
  10:	(40 e0 00 00|00 00 e0 40) 	bge\+    10 .*
  14:	(41 00 00 00|00 00 00 41) 	bdnzt   lt,14 .*
  18:	(41 40 00 00|00 00 40 41) 	bdzt    lt,18 .*
  1c:	(41 80 00 00|00 00 80 41) 	blt     1c .*
  20:	(41 c0 00 00|00 00 c0 41) 	blt-    20 .*
  24:	(41 e0 00 00|00 00 e0 41) 	blt\+    24 .*
  28:	(42 00 00 00|00 00 00 42) 	bdnz    28 .*
  2c:	(42 40 00 00|00 00 40 42) 	bdz     2c .*
  30:	(42 80 00 00|00 00 80 42) 	bc      20,lt,30 .*
  34:	(43 00 00 00|00 00 00 43) 	bdnz-   34 .*
  38:	(43 20 00 00|00 00 20 43) 	bdnz\+   38 .*
  3c:	(43 40 00 00|00 00 40 43) 	bdz-    3c .*
  40:	(43 60 00 00|00 00 60 43) 	bdz\+    40 .*
  44:	(4c 80 04 20|20 04 80 4c) 	bgectr
  48:	(4c c0 04 20|20 04 c0 4c) 	bgectr-
  4c:	(4c e0 04 20|20 04 e0 4c) 	bgectr\+
  50:	(4d 80 04 20|20 04 80 4d) 	bltctr
  54:	(4d c0 04 20|20 04 c0 4d) 	bltctr-
  58:	(4d e0 04 20|20 04 e0 4d) 	bltctr\+
  5c:	(4e 80 04 20|20 04 80 4e) 	bctr
  60:	(4c 00 00 20|20 00 00 4c) 	bdnzflr lt
  64:	(4c 40 00 20|20 00 40 4c) 	bdzflr  lt
  68:	(4c 80 00 20|20 00 80 4c) 	bgelr
  6c:	(4c c0 00 20|20 00 c0 4c) 	bgelr-
  70:	(4c e0 00 20|20 00 e0 4c) 	bgelr\+
  74:	(4d 00 00 20|20 00 00 4d) 	bdnztlr lt
  78:	(4d 40 00 20|20 00 40 4d) 	bdztlr  lt
  7c:	(4d 80 00 20|20 00 80 4d) 	bltlr
  80:	(4d c0 00 20|20 00 c0 4d) 	bltlr-
  84:	(4d e0 00 20|20 00 e0 4d) 	bltlr\+
  88:	(4e 00 00 20|20 00 00 4e) 	bdnzlr
  8c:	(4e 40 00 20|20 00 40 4e) 	bdzlr
  90:	(4e 80 00 20|20 00 80 4e) 	blr
  94:	(4f 00 00 20|20 00 00 4f) 	bdnzlr-
  98:	(4f 20 00 20|20 00 20 4f) 	bdnzlr\+
  9c:	(4f 40 00 20|20 00 40 4f) 	bdzlr-
  a0:	(4f 60 00 20|20 00 60 4f) 	bdzlr\+
  a4:	(4c 00 04 60|60 04 00 4c) 	bdnzftar lt
  a8:	(4c 40 04 60|60 04 40 4c) 	bdzftar lt
  ac:	(4c 80 04 60|60 04 80 4c) 	bgetar
  b0:	(4c c0 04 60|60 04 c0 4c) 	bgetar-
  b4:	(4c e0 04 60|60 04 e0 4c) 	bgetar\+
  b8:	(4d 00 04 60|60 04 00 4d) 	bdnzttar lt
  bc:	(4d 40 04 60|60 04 40 4d) 	bdzttar lt
  c0:	(4d 80 04 60|60 04 80 4d) 	blttar
  c4:	(4d c0 04 60|60 04 c0 4d) 	blttar-
  c8:	(4d e0 04 60|60 04 e0 4d) 	blttar\+
  cc:	(4e 00 04 60|60 04 00 4e) 	bdnztar
  d0:	(4e 40 04 60|60 04 40 4e) 	bdztar
  d4:	(4e 80 04 60|60 04 80 4e) 	btar
  d8:	(4f 00 04 60|60 04 00 4f) 	bdnztar-
  dc:	(4f 20 04 60|60 04 20 4f) 	bdnztar\+
  e0:	(4f 40 04 60|60 04 40 4f) 	bdztar-
  e4:	(4f 60 04 60|60 04 60 4f) 	bdztar\+
#pass
