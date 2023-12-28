 .data
foodata: .word 42
 .text
footext:
	.text
	.global jmp
jmp:
	jmp 2
	jmp 8192
	jmp 4096
	jmp 4094
	jmp 2
	jmp 2960
	jmp 2128
	jmp 2926
	.text
	.global call
call:
	call 4
	call 8192
	call 4096
	call 4094
	call 2
	call 7384
	call 7998
	call 5074
	.text
	.global sb
sb:
	sb 1,1
	sb 25,7
	sb 25,4
	sb 25,3
	sb 1,1
	sb 24,7
	sb 16,1
	sb 12,6
	.text
	.global snb
snb:
	snb 1,1
	snb 11,7
	snb 56,4
	snb 25,3
	snb 1,1
	snb 41,5
	snb 62,1
	snb 43,1
	.text
	.global setb
setb:
	setb 1,1
	setb 11,7
	setb 56,4
	setb 25,3
	setb 1,1
	setb 23,1
	setb 25,6
	setb 28,3
	.text
	.global clrb
clrb:
	clrb 1,1
	clrb 11,7
	clrb 56,4
	clrb 25,3
	clrb 1,1
	clrb 36,7
	clrb 15,3
	clrb 18,5
	.text
	.global xorw_l
xorw_l:
	xor W,#0
	xor W,#25
	xor W,#12
	xor W,#123
	xor W,#1
	xor W,#20
	xor W,#122
	xor W,#15
	.text
	.global andw_l
andw_l:
	and W,#0
	and W,#25
	and W,#12
	and W,#12
	and W,#1
	and W,#18
	and W,#29
	and W,#14
	.text
	.global orw_l
orw_l:
	or W,#0
	or W,#25
	or W,#12
	or W,#12
	or W,#1
	or W,#32
	or W,#14
	or W,#33
	.text
	.global addw_l
addw_l:
	add W,#0
	add W,#25
	add W,#12
	add W,#12
	add W,#1
	add W,#21
	add W,#24
	add W,#47
	.text
	.global subw_l
subw_l:
	sub W,#0
	sub W,#25
	sub W,#212
	sub W,#12
	sub W,#1
	sub W,#112
	sub W,#84
	sub W,#225
	.text
	.global cmpw_l
cmpw_l:
	cmp W,#0
	cmp W,#25
	cmp W,#12
	cmp W,#12
	cmp W,#1
	cmp W,#11
	cmp W,#13
	cmp W,#19
	.text
	.global retw_l
retw_l:
	retw #0
	retw #25
	retw #122
	retw #12
	retw #1
	retw #201
	retw #14
	retw #20
	.text
	.global csew_l
csew_l:
	cse W,#0
	cse W,#25
	cse W,#121
	cse W,#122
	cse W,#1
	cse W,#12
	cse W,#231
	cse W,#21
	.text
	.global csnew_l
csnew_l:
	csne W,#0
	csne W,#25
	csne W,#122
	csne W,#12
	csne W,#1
	csne W,#22
	csne W,#112
	csne W,#22
	.text
	.global push_l
push_l:
	push #0
	push #25
	push #112
	push #12
	push #1
	push #18
	push #15
	push #122
	.text
	.global mulsw_l
mulsw_l:
	muls W,#0
	muls W,#25
	muls W,#12
	muls W,#12
	muls W,#1
	muls W,#23
	muls W,#21
	muls W,#18
	.text
	.global muluw_l
muluw_l:
	mulu W,#0
	mulu W,#25
	mulu W,#12
	mulu W,#12
	mulu W,#1
	mulu W,#15
	mulu W,#21
	mulu W,#23
	.text
	.global loadl_l
loadl_l:
	loadl #0
	loadl #25
	loadl #12
	loadl #12
	loadl #1
	loadl #16
	loadl #16
	loadl #21
	.text
	.global loadh_l
loadh_l:
	loadh #0
	loadh #25
	loadh #12
	loadh #12
	loadh #1
	loadh #17
	loadh #24
	loadh #24
	.text
	.global loadl_a
loadl_a:
	loadl 1
	loadl 25
	loadl 12
	loadl 12
	loadl 1
	loadl 76
	loadl 20
	loadl 52
	.text
	.global loadh_a
loadh_a:
	loadh 1
	loadh 25
	loadh 12
	loadh 12
	loadh 1
	loadh 57
	loadh 56
	loadh 59
	.text
	.global addcfr_w
addcfr_w:
	addc 1,W
	addc 11,W
	addc 56,W
	addc 25,W
	addc 100,W
	addc 34,W
	addc 50,W
	addc 24,W
	.text
	.global addcw_fr
addcw_fr:
	addc W,1
	addc W,11
	addc W,26
	addc W,25
	addc W,10
	addc W,27
	addc W,111
	addc W,22
	.text
	.global incsnz_fr
incsnz_fr:
	incsnz 3
	incsnz 11
	incsnz 56
	incsnz 25
	incsnz 1
	incsnz 50
	incsnz 37
	incsnz 43
	.text
	.global incsnzw_fr
incsnzw_fr:
	incsnz W,1
	incsnz W,11
	incsnz W,26
	incsnz W,25
	incsnz W,1
	incsnz W,33
	incsnz W,29
	incsnz W,24
	.text
	.global mulsw_fr
mulsw_fr:
	muls W,1
	muls W,11
	muls W,26
	muls W,25
	muls W,1
	muls W,23
	muls W,13
	muls W,37
	.text
	.global muluw_fr
muluw_fr:
	mulu W,1
	mulu W,11
	mulu W,26
	mulu W,25
	mulu W,1
	mulu W,21
	mulu W,21
	mulu W,34
	.text
	.global decsnz_fr
decsnz_fr:
	decsnz 1
	decsnz 11
	decsnz 56
	decsnz 25
	decsnz 1
	decsnz 43
	decsnz 6
	decsnz 30
	.text
	.global decsnzw_fr
decsnzw_fr:
	decsnz W,1
	decsnz W,11
	decsnz W,26
	decsnz W,25
	decsnz W,1
	decsnz W,24
	decsnz W,58
	decsnz W,20
	.text
	.global subcw_fr
subcw_fr:
	subc W,1
	subc W,11
	subc W,26
	subc W,25
	subc W,1
	subc W,43
	subc W,13
	subc W,33
	.text
	.global subcfr_w
subcfr_w:
	subc 1,W
	subc 11,W
	subc 56,W
	subc 25,W
	subc 1,W
	subc 15,W
	subc 21,W
	subc 43,W
	.text
	.global pop_fr
pop_fr:
	pop 1
	pop 11
	pop 56
	pop 25
	pop 1
	pop 35
	pop 10
	pop 13
	.text
	.global push_fr
push_fr:
	push 1
	push 11
	push 56
	push 25
	push 1
	push 26
	push 13
	push 13
	.text
	.global csew_fr
csew_fr:
	cse W,1
	cse W,11
	cse W,26
	cse W,25
	cse W,1
	cse W,27
	cse W,15
	cse W,87
	.text
	.global csnew_fr
csnew_fr:
	csne W,2
	csne W,11
	csne W,26
	csne W,25
	csne W,1
	csne W,39
	csne W,17
	csne W,43
	.text
	.global incsz_fr
incsz_fr:
	incsz 1
	incsz 11
	incsz 56
	incsz 25
	incsz 1
	incsz 45
	incsz 24
	incsz 77
	.text
	.global incszw_fr
incszw_fr:
	incsz W,1
	incsz W,11
	incsz W,26
	incsz W,25
	incsz W,1
	incsz W,77
	incsz W,11
	incsz W,98
	.text
	.global swap_fr
swap_fr:
	swap 1
	swap 11
	swap 56
	swap 25
	swap 2
	swap 33
	swap 24
	swap 51
	.text
	.global swapw_fr
swapw_fr:
	swap W,1
	swap W,11
	swap W,26
	swap W,25
	swap W,1
	swap W,43
	swap W,32
	swap W,17
	.text
	.global rl_fr
rl_fr:
	rl 2
	rl 11
	rl 56
	rl 25
	rl 1
	rl 30
	rl 34
	rl 43
	.text
	.global rlw_fr
rlw_fr:
	rl W,2
	rl W,11
	rl W,26
	rl W,25
	rl W,1
	rl W,14
	rl W,24
	rl W,27
	.text
	.global rr_fr
rr_fr:
	rr 1
	rr 11
	rr 56
	rr 25
	rr 1
	rr 43
	rr 25
	rr 16
	.text
	.global rrw_fr
rrw_fr:
	rr W,1
	rr W,11
	rr W,26
	rr W,25
	rr W,1
	rr W,16
	rr W,72
	rr W,17
	.text
	.global decsz_fr
decsz_fr:
	decsz 1
	decsz 11
	decsz 56
	decsz 25
	decsz 1
	decsz 78
	decsz 29
	decsz 16
	.text
	.global decszw_fr
decszw_fr:
	decsz W,1
	decsz W,11
	decsz W,26
	decsz W,25
	decsz W,1
	decsz W,26
	decsz W,22
	decsz W,4
	.text
	.global inc_fr
inc_fr:
	inc 1
	inc 11
	inc 56
	inc 25
	inc 1
	inc 43
	inc 43
	inc 83
	.text
	.global incw_fr
incw_fr:
	inc W,1
	inc W,11
	inc W,26
	inc W,25
	inc W,1
	inc W,43
	inc W,30
	inc W,33
	.text
	.global not_fr
not_fr:
	not 1
	not 11
	not 56
	not 25
	not 1
	not 43
	not 14
	not 43
	.text
	.global notw_fr
notw_fr:
	not W,1
	not W,11
	not W,26
	not W,25
	not W,1
	not W,84
	not W,43
	not W,50
	.text
	.global test_fr
test_fr:
	test 2
	test 11
	test 56
	test 215
	test 1
	test 43
	test 24
	test 25
	.text
	.global movw_l
movw_l:
	mov W,#0
	mov W,#25
	mov W,#12
	mov W,#12
	mov W,#1
	mov W,#14
	mov W,#11
	mov W,#66
	.text
	.global movfr_w
movfr_w:
	mov 1,W
	mov 11,W
	mov 56,W
	mov 25,W
	mov 1,W
	mov 36,W
	mov 86,W
	mov 18,W
	.text
	.global movw_fr
movw_fr:
	mov W,1
	mov W,11
	mov W,26
	mov W,25
	mov W,1
	mov W,12
	mov W,43
	mov W,23
	.text
	.global addfr_w
addfr_w:
	add 10,W
	add 11,W
	add 56,W
	add 215,W
	add 1,W
	add 43,W
	add 25,W
	add 39,W
	.text
	.global addw_fr
addw_fr:
	add W,1
	add W,11
	add W,26
	add W,25
	add W,1
	add W,19
	add W,91
	add W,25
	.text
	.global xorfr_w
xorfr_w:
	xor 1,W
	xor 11,W
	xor 56,W
	xor 25,W
	xor 2,W
	xor 31,W
	xor 22,W
	xor 43,W
	.text
	.global xorw_fr
xorw_fr:
	xor W,2
	xor W,11
	xor W,26
	xor W,25
	xor W,1
	xor W,14
	xor W,10
	xor W,21
	.text
	.global andfr_w
andfr_w:
	and 1,W
	and 11,W
	and 56,W
	and 25,W
	and 1,W
	and 28,W
	and 37,W
	and 24,W
	.text
	.global andw_fr
andw_fr:
	and W,1
	and W,11
	and W,26
	and W,25
	and W,1
	and W,21
	and W,40
	and W,43
	.text
	.global orfr_w
orfr_w:
	or 1,W
	or 11,W
	or 56,W
	or 25,W
	or 1,W
	or 58,W
	or 29,W
	or 10,W
	.text
	.global orw_fr
orw_fr:
	or W,1
	or W,11
	or W,26
	or W,25
	or W,1
	or W,11
	or W,24
	or W,59
	.text
	.global dec_fr
dec_fr:
	dec 2
	dec 51
	dec 26
	dec 25
	dec 1
	dec 76
	dec 32
	dec 17
	.text
	.global decw_fr
decw_fr:
	dec W,2
	dec W,51
	dec W,56
	dec W,25
	dec W,1
	dec W,1
	dec W,68
	dec W,7
	.text
	.global subfr_w
subfr_w:
	sub 2,W
	sub 11,W
	sub 15,W
	sub 25,W
	sub 1,W
	sub 40,W
	sub 55,W
	sub 17,W
	.text
	.global subw_fr
subw_fr:
	sub W,1
	sub W,21
	sub W,25
	sub W,25
	sub W,1
	sub W,17
	sub W,16
	sub W,18
	.text
	.global clr_fr
clr_fr:
	clr 10
	clr 11
	clr 25
	clr 25
	clr 1
	clr 24
	clr 215
	clr 23
	.text
	.global cmpw_fr
cmpw_fr:
	cmp W,1
	cmp W,21
	cmp W,25
	cmp W,25
	cmp W,1
	cmp W,18
	cmp W,20
	cmp W,16
	.text
	.global speed
speed:
	speed #0
	speed #25
	speed #12
	speed #12
	speed #1
	speed #14
	speed #18
	speed #97
	.text
	.global ireadi
ireadi:
	ireadi
	.text
	.global iwritei
iwritei:
	iwritei
	.text
	.global fread
fread:
	fread
	.text
	.global fwrite
fwrite:
	fwrite
	.text
	.global iread
iread:
	iread
	.text
	.global iwrite
iwrite:
	iwrite
	.text
	.global page
page:
	page 2
	page 8
	page 14
	page 10
	page 12
	page 0
	page 4
	page 6
	.text
	.global system
system:
	system
	.text
	.global reti
reti:
	reti #0
	reti #1
	reti #2
	reti #3
	reti #4
	reti #5
	reti #6
	reti #7
	.text
	.global ret
ret:
	ret
	.text
	.global int
int:
	int
	.text
	.global breakx
breakx:
	breakx
	.text
	.global cwdt
cwdt:
	cwdt
	.text
	.global ferase
ferase:
	ferase
	.text
	.global retnp
retnp:
	retnp
	.text
	.global break
break:
	break
	.text
	.global nop
nop:
	nop
