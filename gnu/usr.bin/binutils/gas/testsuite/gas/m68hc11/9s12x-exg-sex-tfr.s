# Test for correct generation of 9s12x specific insns.

	.sect .text
;;
;; Test all s12x extended forms of exg,tfr,sex where supported
;;
;; presently tmp register and h/l forms not supported in gas
;exg
;; none of shaded area is actually supported
    exg     a,a
    exg     b,a
;    exg     ccrh,a
;    exg     tmp3h,a
    exg     d,a
;    exg     xh,a
;    exg     yh,a
;    exg     sph,a
    exg     a,b
    exg     b,b
;    exg     ccrl,b
;    exg     tmp3l,b
    exg     d,b
;    exg     xl,b
;    exg     yl,b
;    exg     spl,b
;    exg     a,ccrh
;    exg     b,ccrl
    exg     ccr,ccr
;    exg     tmp3,ccr
    exg     d,ccr
    exg     x,ccr
    exg     y,ccr
    exg     sp,ccr
;    exg     a,tmp2h
;    exg     b,tmp2l
;    exg     ccr,tmp2
;    exg     tmp3,tmp2
;    exg     d,tmp1
;    exg     x,tmp2
;    exg     y,tmp2
;    exg     sp,tmp2
    exg     a,d
    exg     b,d
    exg     ccr,d
;    exg     tmp1,d
    exg     d,d
    exg     x,d
    exg     y,d
    exg     sp,d
;    exg     a,xh
;    exg     b,xl
    exg     ccr,x
;    exg     tmp3,x
    exg     d,x
    exg     x,x
    exg     y,x
    exg     sp,x
;    exg     a,yh
;    exg     b,yl
    exg     ccr,y
;    exg     tmp3,y
    exg     d,y
    exg     x,y
    exg     y,y
    exg     sp,y
;    exg     a,sph
;    exg     b,spl
    exg     ccr,sp
;    exg     tmp3,sp
    exg     d,sp
    exg     x,sp
    exg     y,sp
    exg     sp,sp

;sex
	sex		a,d
	sex		b,d
	sex		d,x ; new
	sex		d,y ; new

;tfr
	tfr		a,a
	tfr		b,a
;	tfr		tmp3h,a
	tfr		d,a
;	tfr		xh,a
;	tfr		yh,a
;	tfr		sph,a
	tfr		a,b
	tfr		b,b
;	tfr		ccrl,b
;	tfr		tmp3l,b
	tfr		d,b
;	tfr		xl,b
;	tfr		yl,b
;	tfr		spl,b
;	tfr		a,ccrh
;	tfr		b,ccrl
;	tfr		ccrw,ccrw
;	tfr		tmp3,ccrw
;	tfr		d,ccrw
;	tfr		x,ccrw
;	tfr		y,ccrw
;	tfr		sp,ccrw
;	tfr		a,tmp2h
;	tfr		b,tmp2l
;	tfr		ccrw,tmp
;	tfr		tmp3,tmp2
;	tfr		d,tmp1
;	tfr		x,tmp2
;	tfr		y,tmp2
;	tfr		sp,tmp2
;sex
;sex
;	tfr		ccrw,d
;	tfr		tmp1,d
	tfr		d,d
	tfr		x,d
	tfr		y,d
	tfr		sp,d
;	tfr		a,xh
;	tfr		b,xl
;	tfr		ccrw,x
;	tfr		tmp3,x
;sex
	tfr		x,x
	tfr		y,x
	tfr		sp,x
;	tfr		a,yh
;	tfr		b,yl
;	tfr		ccrw,y
;	tfr		tmp3,y
;sex
	tfr		x,y
	tfr		y,y
	tfr		sp,y
;	tfr		a,sph
;	tfr		b,spl
;	tfr		ccrw,xp
;	tfr		tmp3,sp
	tfr		d,sp
	tfr		x,sp
	tfr		y,sp
	tfr		sp,sp

