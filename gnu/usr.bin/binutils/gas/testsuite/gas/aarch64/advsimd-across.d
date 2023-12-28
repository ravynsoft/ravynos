#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	0e303be7 	saddlv	h7, v31.8b
   4:	4e303be7 	saddlv	h7, v31.16b
   8:	0e703be7 	saddlv	s7, v31.4h
   c:	4e703be7 	saddlv	s7, v31.8h
  10:	4eb03be7 	saddlv	d7, v31.4s
  14:	2e303be7 	uaddlv	h7, v31.8b
  18:	6e303be7 	uaddlv	h7, v31.16b
  1c:	2e703be7 	uaddlv	s7, v31.4h
  20:	6e703be7 	uaddlv	s7, v31.8h
  24:	6eb03be7 	uaddlv	d7, v31.4s
  28:	0e30abe7 	smaxv	b7, v31.8b
  2c:	4e30abe7 	smaxv	b7, v31.16b
  30:	0e70abe7 	smaxv	h7, v31.4h
  34:	4e70abe7 	smaxv	h7, v31.8h
  38:	4eb0abe7 	smaxv	s7, v31.4s
  3c:	2e30abe7 	umaxv	b7, v31.8b
  40:	6e30abe7 	umaxv	b7, v31.16b
  44:	2e70abe7 	umaxv	h7, v31.4h
  48:	6e70abe7 	umaxv	h7, v31.8h
  4c:	6eb0abe7 	umaxv	s7, v31.4s
  50:	0e31abe7 	sminv	b7, v31.8b
  54:	4e31abe7 	sminv	b7, v31.16b
  58:	0e71abe7 	sminv	h7, v31.4h
  5c:	4e71abe7 	sminv	h7, v31.8h
  60:	4eb1abe7 	sminv	s7, v31.4s
  64:	2e31abe7 	uminv	b7, v31.8b
  68:	6e31abe7 	uminv	b7, v31.16b
  6c:	2e71abe7 	uminv	h7, v31.4h
  70:	6e71abe7 	uminv	h7, v31.8h
  74:	6eb1abe7 	uminv	s7, v31.4s
  78:	0e31bbe7 	addv	b7, v31.8b
  7c:	4e31bbe7 	addv	b7, v31.16b
  80:	0e71bbe7 	addv	h7, v31.4h
  84:	4e71bbe7 	addv	h7, v31.8h
  88:	4eb1bbe7 	addv	s7, v31.4s
  8c:	6e30cbe7 	fmaxnmv	s7, v31.4s
  90:	6eb0cbe7 	fminnmv	s7, v31.4s
  94:	6e30fbe7 	fmaxv	s7, v31.4s
  98:	6eb0fbe7 	fminv	s7, v31.4s
