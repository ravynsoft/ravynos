	.text
	.org 0
;;; input
	in a,(0x76)
	in a,(c)
	in b,(c)
	in c,(c)
	in d,(c)
	in e,(c)
	in h,(c)
	in l,(c)

;;; output
	out (0x76),a
	out (c),a
	out (c),b
	out (c),c
	out (c),d
	out (c),e
	out (c),h
	out (c),l
	
	