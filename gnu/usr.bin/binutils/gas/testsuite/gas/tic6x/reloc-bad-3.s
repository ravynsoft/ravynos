# Test relocation overflow and insufficiently divisible values.  Note
# that divisibility checks for constant values are only applicable to
# load and store offsets, not ADDA, because constant values are
# encoded literally for ADDA, and divisbility checks for offsets from
# symbols are only applicable with REL relocations.
.data
t0:
	.short b65535-b0
	.short b65536-b0
	.short b0-b32768
	.short b32767-b65536
	.byte b255-b0
	.byte b256-b0
	.byte b0-b128
	.byte b127-b256
.text
.nocmp
.globl f
f:
	addab .D1X b14,b32767-b0,a5
	addab .D1X b14,b32768-b0,a5
	addab .D1X b14,b127-b128,a5
	addah .D1X b14,b32767-b0,a5
	addah .D1X b14,b32768-b0,a5
	addah .D1X b14,b127-b128,a5
	addaw .D1X b14,b32767-b0,a5
	addaw .D1X b14,b32768-b0,a5
	addaw .D1X b14,b127-b128,a5
	addk .S1 b32767-b0,a9
	addk .S1 b0-b32768,a9
	addk .S1 b32768-b0,a9
	addk .S1 b32767-b65536,a9
	mvk .S1 b32767-b0,a9
	mvk .S1 b0-b32768,a9
	mvk .S1 b32768-b0,a9
	mvk .S1 b32767-b65536,a9
	ldb .D2T2 *+b14(b32767-b0),b1
	ldb .D2T2 *+b14(b32768-b0),b1
	ldb .D2T2 *+b14(b32767-b32768),b1
	ldbu .D2T2 *+b14(b32767-b0),b1
	ldbu .D2T2 *+b14(b32768-b0),b1
	ldbu .D2T2 *+b14(b32767-b32768),b1
	ldh .D2T2 *+b14(h32767-h0),b1
	ldh .D2T2 *+b14(h32768-h0),b1
	ldh .D2T2 *+b14(h32767-h32768),b1
	ldh .D2T2 *+b14(b32768-b32767),b1
	ldhu .D2T2 *+b14(h32767-h0),b1
	ldhu .D2T2 *+b14(h32768-h0),b1
	ldhu .D2T2 *+b14(h32767-h32768),b1
	ldhu .D2T2 *+b14(b32768-b32767),b1
	ldw .D2T2 *+b14(w32767-w0),b1
	ldw .D2T2 *+b14(w32768-w0),b1
	ldw .D2T2 *+b14(w32767-w32768),b1
	ldw .D2T2 *+b14(h32768-h32767),b1
	stb .D2T2 b1,*+b14(b32767-b0)
	stb .D2T2 b1,*+b14(b32768-b0)
	stb .D2T2 b1,*+b14(b32767-b32768)
	sth .D2T2 b1,*+b14(h32767-h0)
	sth .D2T2 b1,*+b14(h32768-h0)
	sth .D2T2 b1,*+b14(h32767-h32768)
	sth .D2T2 b1,*+b14(b32768-b32767)
	stw .D2T2 b1,*+b14(w32767-w0)
	stw .D2T2 b1,*+b14(w32768-w0)
	stw .D2T2 b1,*+b14(w32767-w32768)
	stw .D2T2 b1,*+b14(h32768-h32767)
b0:
	.space 127
b127:
	.space 1
b128:
	.space 127
b255:
	.space 1
b256:
	.space 32511
b32767:
	.space 1
b32768:
	.space 32767
b65535:
	.space 1
b65536:
	.word 0
h0:
	.space 65534
h32767:
	.space 2
h32768:
	.word 0
w0:
	.space 131068
w32767:
	.space 4
w32768:
	.word 0
