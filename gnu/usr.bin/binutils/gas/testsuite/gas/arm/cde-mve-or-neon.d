#name: Custom Datapath Extension (CDE)
#source: cde-mve-or-neon.s
#as: -mno-warn-deprecated -march=armv8-m.main+cdecp0+cdecp7+fp -I$srcdir/$subdir
#as: -mno-warn-deprecated -march=armv8-m.main+cdecp0+cdecp1+cdecp2+cdecp3+cdecp4+cdecp5+cdecp6+cdecp7+fp -I$srcdir/$subdir
#objdump: -M force-thumb -dr --show-raw-insn -marmv8-m.main -M coproc0=cde -M coproc7=cde
#...
00000000 <\.text>:
 *[0-9a-f]+:	ec20 0000 	vcx1	p0, s0, #0
 *[0-9a-f]+:	ec2f 0000 	vcx1	p0, s0, #1920
 *[0-9a-f]+:	ec20 0080 	vcx1	p0, s0, #64
 *[0-9a-f]+:	ec20 003f 	vcx1	p0, s0, #63
 *[0-9a-f]+:	ec20 0700 	vcx1	p7, s0, #0
 *[0-9a-f]+:	ec60 0000 	vcx1	p0, s1, #0
 *[0-9a-f]+:	ec20 f000 	vcx1	p0, s30, #0
 *[0-9a-f]+:	ed20 0000 	vcx1	p0, d0, #0
 *[0-9a-f]+:	ed2f 0000 	vcx1	p0, d0, #1920
 *[0-9a-f]+:	ed20 0080 	vcx1	p0, d0, #64
 *[0-9a-f]+:	ed20 003f 	vcx1	p0, d0, #63
 *[0-9a-f]+:	ed20 0700 	vcx1	p7, d0, #0
 *[0-9a-f]+:	ed20 f000 	vcx1	p0, d15, #0
 *[0-9a-f]+:	fc20 0000 	vcx1a	p0, s0, #0
 *[0-9a-f]+:	fc2f 0000 	vcx1a	p0, s0, #1920
 *[0-9a-f]+:	fc20 0080 	vcx1a	p0, s0, #64
 *[0-9a-f]+:	fc20 003f 	vcx1a	p0, s0, #63
 *[0-9a-f]+:	fc20 0700 	vcx1a	p7, s0, #0
 *[0-9a-f]+:	fc60 0000 	vcx1a	p0, s1, #0
 *[0-9a-f]+:	fc20 f000 	vcx1a	p0, s30, #0
 *[0-9a-f]+:	fd20 0000 	vcx1a	p0, d0, #0
 *[0-9a-f]+:	fd2f 0000 	vcx1a	p0, d0, #1920
 *[0-9a-f]+:	fd20 0080 	vcx1a	p0, d0, #64
 *[0-9a-f]+:	fd20 003f 	vcx1a	p0, d0, #63
 *[0-9a-f]+:	fd20 0700 	vcx1a	p7, d0, #0
 *[0-9a-f]+:	fd20 f000 	vcx1a	p0, d15, #0
 *[0-9a-f]+:	ec30 0000 	vcx2	p0, s0, s0, #0
 *[0-9a-f]+:	ec3f 0000 	vcx2	p0, s0, s0, #60
 *[0-9a-f]+:	ec30 0080 	vcx2	p0, s0, s0, #2
 *[0-9a-f]+:	ec30 0010 	vcx2	p0, s0, s0, #1
 *[0-9a-f]+:	ec30 0700 	vcx2	p7, s0, s0, #0
 *[0-9a-f]+:	ec70 0000 	vcx2	p0, s1, s0, #0
 *[0-9a-f]+:	ec30 f000 	vcx2	p0, s30, s0, #0
 *[0-9a-f]+:	ec30 0020 	vcx2	p0, s0, s1, #0
 *[0-9a-f]+:	ec30 000f 	vcx2	p0, s0, s30, #0
 *[0-9a-f]+:	ed30 0000 	vcx2	p0, d0, d0, #0
 *[0-9a-f]+:	ed3f 0000 	vcx2	p0, d0, d0, #60
 *[0-9a-f]+:	ed30 0080 	vcx2	p0, d0, d0, #2
 *[0-9a-f]+:	ed30 0010 	vcx2	p0, d0, d0, #1
 *[0-9a-f]+:	ed30 0700 	vcx2	p7, d0, d0, #0
 *[0-9a-f]+:	ed30 f000 	vcx2	p0, d15, d0, #0
 *[0-9a-f]+:	ed30 000f 	vcx2	p0, d0, d15, #0
 *[0-9a-f]+:	fc30 0000 	vcx2a	p0, s0, s0, #0
 *[0-9a-f]+:	fc3f 0000 	vcx2a	p0, s0, s0, #60
 *[0-9a-f]+:	fc30 0080 	vcx2a	p0, s0, s0, #2
 *[0-9a-f]+:	fc30 0010 	vcx2a	p0, s0, s0, #1
 *[0-9a-f]+:	fc30 0700 	vcx2a	p7, s0, s0, #0
 *[0-9a-f]+:	fc70 0000 	vcx2a	p0, s1, s0, #0
 *[0-9a-f]+:	fc30 f000 	vcx2a	p0, s30, s0, #0
 *[0-9a-f]+:	fc30 0020 	vcx2a	p0, s0, s1, #0
 *[0-9a-f]+:	fc30 000f 	vcx2a	p0, s0, s30, #0
 *[0-9a-f]+:	fd30 0000 	vcx2a	p0, d0, d0, #0
 *[0-9a-f]+:	fd3f 0000 	vcx2a	p0, d0, d0, #60
 *[0-9a-f]+:	fd30 0080 	vcx2a	p0, d0, d0, #2
 *[0-9a-f]+:	fd30 0010 	vcx2a	p0, d0, d0, #1
 *[0-9a-f]+:	fd30 0700 	vcx2a	p7, d0, d0, #0
 *[0-9a-f]+:	fd30 f000 	vcx2a	p0, d15, d0, #0
 *[0-9a-f]+:	fd30 000f 	vcx2a	p0, d0, d15, #0
 *[0-9a-f]+:	ec80 0000 	vcx3	p0, s0, s0, s0, #0
 *[0-9a-f]+:	ecb0 0000 	vcx3	p0, s0, s0, s0, #6
 *[0-9a-f]+:	ec80 0010 	vcx3	p0, s0, s0, s0, #1
 *[0-9a-f]+:	ec80 0700 	vcx3	p7, s0, s0, s0, #0
 *[0-9a-f]+:	ecc0 0000 	vcx3	p0, s1, s0, s0, #0
 *[0-9a-f]+:	ec80 f000 	vcx3	p0, s30, s0, s0, #0
 *[0-9a-f]+:	ec80 0080 	vcx3	p0, s0, s1, s0, #0
 *[0-9a-f]+:	ec8f 0000 	vcx3	p0, s0, s30, s0, #0
 *[0-9a-f]+:	ec80 0020 	vcx3	p0, s0, s0, s1, #0
 *[0-9a-f]+:	ec80 000f 	vcx3	p0, s0, s0, s30, #0
 *[0-9a-f]+:	ed80 0000 	vcx3	p0, d0, d0, d0, #0
 *[0-9a-f]+:	edb0 0000 	vcx3	p0, d0, d0, d0, #6
 *[0-9a-f]+:	ed80 0010 	vcx3	p0, d0, d0, d0, #1
 *[0-9a-f]+:	ed80 0700 	vcx3	p7, d0, d0, d0, #0
 *[0-9a-f]+:	ed80 f000 	vcx3	p0, d15, d0, d0, #0
 *[0-9a-f]+:	ed8f 0000 	vcx3	p0, d0, d15, d0, #0
 *[0-9a-f]+:	ed80 000f 	vcx3	p0, d0, d0, d15, #0
 *[0-9a-f]+:	fc80 0000 	vcx3a	p0, s0, s0, s0, #0
 *[0-9a-f]+:	fcb0 0000 	vcx3a	p0, s0, s0, s0, #6
 *[0-9a-f]+:	fc80 0010 	vcx3a	p0, s0, s0, s0, #1
 *[0-9a-f]+:	fc80 0700 	vcx3a	p7, s0, s0, s0, #0
 *[0-9a-f]+:	fcc0 0000 	vcx3a	p0, s1, s0, s0, #0
 *[0-9a-f]+:	fc80 f000 	vcx3a	p0, s30, s0, s0, #0
 *[0-9a-f]+:	fc80 0080 	vcx3a	p0, s0, s1, s0, #0
 *[0-9a-f]+:	fc8f 0000 	vcx3a	p0, s0, s30, s0, #0
 *[0-9a-f]+:	fc80 0020 	vcx3a	p0, s0, s0, s1, #0
 *[0-9a-f]+:	fc80 000f 	vcx3a	p0, s0, s0, s30, #0
 *[0-9a-f]+:	fd80 0000 	vcx3a	p0, d0, d0, d0, #0
 *[0-9a-f]+:	fdb0 0000 	vcx3a	p0, d0, d0, d0, #6
 *[0-9a-f]+:	fd80 0010 	vcx3a	p0, d0, d0, d0, #1
 *[0-9a-f]+:	fd80 0700 	vcx3a	p7, d0, d0, d0, #0
 *[0-9a-f]+:	fd80 f000 	vcx3a	p0, d15, d0, d0, #0
 *[0-9a-f]+:	fd8f 0000 	vcx3a	p0, d0, d15, d0, #0
 *[0-9a-f]+:	fd80 000f 	vcx3a	p0, d0, d0, d15, #0
