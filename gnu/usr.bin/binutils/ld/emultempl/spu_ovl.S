/* Overlay manager for SPU.

   Copyright (C) 2006-2023 Free Software Foundation, Inc.

   This file is part of the GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* MFC DMA defn's.  */
#define MFC_GET_CMD		0x40
#define MFC_MAX_DMA_SIZE	0x4000
#define MFC_TAG_UPDATE_ALL	2
#define MFC_TAG_ID		0

/* Register usage.  */
#define reserved1	$75
#define parm		$75
#define tab1		reserved1
#define tab2		reserved1
#define vma		reserved1
#define oldvma		reserved1
#define newmask		reserved1
#define map		reserved1

#define reserved2	$76
#define off1		reserved2
#define off2		reserved2
#define present1	reserved2
#define present2	reserved2
#define sz		reserved2
#define cmp		reserved2
#define add64		reserved2
#define cgbits		reserved2
#define off3		reserved2
#define off4		reserved2
#define addr4		reserved2
#define off5		reserved2
#define tagstat		reserved2

#define reserved3	$77
#define size1		reserved3
#define size2		reserved3
#define rv3		reserved3
#define ealo		reserved3
#define cmd		reserved3
#define off64		reserved3
#define tab3		reserved3
#define tab4		reserved3
#define tab5		reserved3

#define reserved4	$78
#define ovl		reserved4
#define rv2		reserved4
#define rv5		reserved4
#define cgshuf		reserved4
#define newovl		reserved4
#define irqtmp1		reserved4
#define irqtmp2		reserved4

#define reserved5	$79
#define target		reserved5

#define save1		$74
#define rv4		save1
#define rv7		save1
#define tagid		save1
#define maxsize		save1
#define pbyte		save1
#define pbit		save1

#define save2		$73
#define cur		save2
#define rv6		save2
#define osize		save2
#define zovl		save2
#define oldovl		save2
#define newvma		save2

#define save3		$72
#define rv1		save3
#define ea64		save3
#define buf3		save3
#define genwi		save3
#define newmap		save3
#define oldmask		save3

#define save4		$71
#define irq_stat	save4

	.text
	.align	4
	.type	__rv_pattern, @object
	.size	__rv_pattern, 16
__rv_pattern:
	.word	0x00010203, 0x10111213, 0x80808080, 0x80808080

	.type	__cg_pattern, @object
	.size	__cg_pattern, 16
__cg_pattern:
	.word	0x04050607, 0x80808080, 0x80808080, 0x80808080

	.type	__ovly_current, @object
	.size	__ovly_current, 16
__ovly_current:
	.space	16

/*
 * __ovly_return - stub for returning from overlay functions.
 *
 * On entry the four slots of $lr are:
 *   __ovly_return, prev ovl index, caller return addr, undefined.
 *
 * Load the previous overlay and jump to the caller return address.
 * Updates __ovly_current.
 */
	.align	4
	.global	__ovly_return
	.type	__ovly_return, @function
__ovly_return:
	ila	tab1, _ovly_table - 16				# 0,2	0
	shlqbyi	ovl, $lr, 4					# 1,4	0
#nop
	shlqbyi	target, $lr, 8					# 1,4	1
#nop; lnop
#nop; lnop
	shli	off1, ovl, 4					# 0,4	4
#lnop
#nop
	hbr	ovly_ret9, target				# 1,15	5
#nop; lnop
#nop; lnop
#nop
	lqx	vma, tab1, off1					# 1,6	8
#ifdef OVLY_IRQ_SAVE
	nop
	stqd	save4, -64($sp)					# 1,6	9
#else
#nop; lnop
#endif
#nop; lnop
#nop; lnop
#nop; lnop
#nop; lnop
#nop
	rotqbyi	size1, vma, 4					# 1,4	14
#nop
	stqd	save3, -48($sp)					# 1,6	15
#nop
	stqd	save2, -32($sp)					# 1,6	16
#nop
	stqd	save1, -16($sp)					# 1,6	17
	andi	present1, size1, 1				# 0,2	18
	stqr	ovl, __ovly_current				# 1,6	18
#nop; lnop
#nop
	brz	present1, do_load				# 1,4	20
ovly_ret9:
#nop
	bi	target						# 1,4	21

/*
 * __ovly_load - copy an overlay partion to local store.
 *
 * On entry $75 points to a word consisting of the overlay index in
 * the top 14 bits, and the target address in the bottom 18 bits.
 *
 * Sets up $lr to return via __ovly_return.  If $lr is already set
 * to return via __ovly_return, don't change it.  In that case we
 * have a tail call from one overlay function to another.
 * Updates __ovly_current.
 */
	.align  3
	.global	__ovly_load
	.type	__ovly_load, @function
__ovly_load:
#if OVL_STUB_SIZE == 8
########
#nop
	lqd	target, 0(parm)					# 1,6	-11
#nop; lnop
#nop; lnop
#nop; lnop
#nop; lnop
#nop; lnop
#nop
	rotqby	target, target, parm				# 1,4	-5
	ila	tab2, _ovly_table - 16				# 0,2	-4
	stqd	save3, -48($sp)					# 1,6	-4
#nop
	stqd	save2, -32($sp)					# 1,6	-3
#nop
	stqd	save1, -16($sp)					# 1,6	-2
	rotmi	ovl, target, -18				# 0,4	-1
	hbr	ovly_load9, target				# 1,15	-1
	ila	rv1, __ovly_return				# 0,2	0
#lnop
#nop; lnop
#nop
	lqr	cur, __ovly_current				# 1,6	2
	shli	off2, ovl, 4					# 0,4	3
	stqr	ovl, __ovly_current				# 1,6	3
	ceq	rv2, $lr, rv1					# 0,2	4
	lqr	rv3, __rv_pattern				# 1,6	4
#nop; lnop
#nop; lnop
#nop
	lqx	vma, tab2, off2					# 1,6	7
########
#else /* OVL_STUB_SIZE == 16 */
########
	ila	tab2, _ovly_table - 16				# 0,2	0
	stqd	save3, -48($sp)					# 1,6	0
	ila	rv1, __ovly_return				# 0,2	1
	stqd	save2, -32($sp)					# 1,6	1
	shli	off2, ovl, 4					# 0,4	2
	lqr	cur, __ovly_current				# 1,6	2
	nop
	stqr	ovl, __ovly_current				# 1,6	3
	ceq	rv2, $lr, rv1					# 0,2	4
	lqr	rv3, __rv_pattern				# 1,6	4
#nop
	hbr	ovly_load9, target				# 1,15	5
#nop
	lqx	vma, tab2, off2					# 1,6	6
#nop
	stqd	save1, -16($sp)					# 1,6	7
########
#endif

#nop; lnop
#nop; lnop
#nop
	shufb	rv4, rv1, cur, rv3				# 1,4	10
#nop
	fsmb	rv5, rv2					# 1,4	11
#nop
	rotqmbyi rv6, $lr, -8					# 1,4	12
#nop
	rotqbyi	size2, vma, 4					# 1,4	13
#nop
	lqd	save3, -48($sp)					# 1,6	14
#nop; lnop
	or	rv7, rv4, rv6					# 0,2	16
	lqd	save2, -32($sp)					# 1,6	16
	andi	present2, size2, 1				# 0,2	17
#ifdef OVLY_IRQ_SAVE
	stqd	save4, -64($sp)					# 1,6	17
#else
	lnop							# 1,0	17
#endif
	selb	$lr, rv7, $lr, rv5				# 0,2	18
	lqd	save1, -16($sp)					# 1,6	18
#nop
	brz	present2, do_load				# 1,4	19
ovly_load9:
#nop
	bi	target						# 1,4	20

/* If we get here, we are about to load a new overlay.
 * "vma" contains the relevant entry from _ovly_table[].
 *	extern struct {
 *		u32 vma;
 *		u32 size;
 *		u32 file_offset;
 *		u32 buf;
 *	} _ovly_table[];
 */
	.align  3
	.global	__ovly_load_event
	.type	__ovly_load_event, @function
__ovly_load_event:
do_load:
#ifdef OVLY_IRQ_SAVE
	ila	irqtmp1, do_load10				# 0,2	-5
	rotqbyi	sz, vma, 8					# 1,4	-5
#nop
	rdch	irq_stat, $SPU_RdMachStat			# 1,6	-4
#nop
	bid	irqtmp1						# 1,4	-3
do_load10:
	nop
#else
#nop
	rotqbyi	sz, vma, 8					# 1,4	0
#endif
	rotqbyi	osize, vma, 4					# 1,4	1
#nop
	lqa	ea64, _EAR_					# 1,6	2
#nop
	lqr	cgshuf, __cg_pattern				# 1,6	3

/* We could predict the branch at the end of this loop by adding a few
   instructions, and there are plenty of free cycles to do so without
   impacting loop execution time.  However, it doesn't make a great
   deal of sense since we need to wait for the dma to complete anyway.  */
__ovly_xfer_loop:
#nop
	rotqmbyi off64, sz, -4					# 1,4	4
#nop; lnop
#nop; lnop
#nop; lnop
	cg	cgbits, ea64, off64				# 0,2	8
#lnop
#nop; lnop
#nop
	shufb	add64, cgbits, cgbits, cgshuf			# 1,4	10
#nop; lnop
#nop; lnop
#nop; lnop
	addx	add64, ea64, off64				# 0,2	14
#lnop
	ila	maxsize, MFC_MAX_DMA_SIZE			# 0,2	15
	lnop
	ori	ea64, add64, 0					# 0,2	16
	rotqbyi	ealo, add64, 4					# 1,4	16
	cgt	cmp, osize, maxsize				# 0,2	17
	wrch	$MFC_LSA, vma					# 1,6	17
#nop; lnop
	selb	sz, osize, maxsize, cmp				# 0,2	19
	wrch	$MFC_EAH, ea64					# 1,6	19
	ila	tagid, MFC_TAG_ID				# 0,2	20
	wrch	$MFC_EAL, ealo					# 1,6	20
	ila	cmd, MFC_GET_CMD				# 0,2	21
	wrch	$MFC_Size, sz					# 1,6	21
	sf	osize, sz, osize				# 0,2	22
	wrch	$MFC_TagId, tagid				# 1,6	22
	a	vma, vma, sz					# 0,2	23
	wrch	$MFC_Cmd, cmd					# 1,6	23
#nop
	brnz	osize, __ovly_xfer_loop				# 1,4	24

/* Now update our data structions while waiting for DMA to complete.
   Low bit of .size needs to be cleared on the _ovly_table entry
   corresponding to the evicted overlay, and set on the entry for the
   newly loaded overlay.  Note that no overlay may in fact be evicted
   as _ovly_buf_table[] starts with all zeros.  Don't zap .size entry
   for zero index!  Also of course update the _ovly_buf_table entry.  */
#nop
	lqr	newovl, __ovly_current				# 1,6	25
#nop; lnop
#nop; lnop
#nop; lnop
#nop; lnop
#nop; lnop
	shli	off3, newovl, 4					# 0,4	31
#lnop
	ila	tab3, _ovly_table - 16				# 0,2	32
#lnop
#nop
	fsmbi	pbyte, 0x100					# 1,4	33
#nop; lnop
#nop
	lqx	vma, tab3, off3					# 1,6	35
#nop; lnop
	andi	pbit, pbyte, 1					# 0,2	37
	lnop
#nop; lnop
#nop; lnop
#nop; lnop
	or	newvma, vma, pbit				# 0,2	41
	rotqbyi	buf3, vma, 12					# 1,4	41
#nop; lnop
#nop
	stqx	newvma, tab3, off3				# 1,6	43
#nop; lnop
	shli	off4, buf3, 2					# 1,4	45
#lnop
	ila	tab4, _ovly_buf_table - 4			# 0,2	46
#lnop
#nop; lnop
#nop; lnop
#nop
	lqx	map, tab4, off4					# 1,6	49
#nop
	cwx	genwi, tab4, off4				# 1,4	50
	a	addr4, tab4, off4				# 0,2	51
#lnop
#nop; lnop
#nop; lnop
#nop; lnop
#nop
	rotqby	oldovl, map, addr4				# 1,4	55
#nop
	shufb	newmap, newovl, map, genwi			# 0,4	56
#if MFC_TAG_ID < 16
	ila	newmask, 1 << MFC_TAG_ID			# 0,2	57
#else
	ilhu	newmask, 1 << (MFC_TAG_ID - 16)			# 0,2	57
#endif
#lnop
#nop; lnop
#nop; lnop
	stqd	newmap, 0(addr4)				# 1,6	60

/* Save app's tagmask, wait for DMA complete, restore mask.  */
	ila	tagstat, MFC_TAG_UPDATE_ALL			# 0,2	61
	rdch	oldmask, $MFC_RdTagMask				# 1,6	61
#nop
	wrch	$MFC_WrTagMask, newmask				# 1,6	62
#nop
	wrch	$MFC_WrTagUpdate, tagstat			# 1,6	63
#nop
	rdch	tagstat, $MFC_RdTagStat				# 1,6	64
#nop
	sync							# 1,4	65
/* Any hint prior to the sync is lost.  A hint here allows the branch
   to complete 15 cycles after the hint.  With no hint the branch will
   take 18 or 19 cycles.  */
	ila	tab5, _ovly_table - 16				# 0,2	66
	hbr	do_load99, target				# 1,15	66
	shli	off5, oldovl, 4					# 0,4	67
	wrch	$MFC_WrTagMask, oldmask				# 1,6	67
	ceqi	zovl, oldovl, 0					# 0,2	68
#lnop
#nop; lnop
#nop
	fsm	zovl, zovl					# 1,4	70
#nop
	lqx	oldvma, tab5, off5				# 1,6	71
#nop
	lqd	save3, -48($sp)					# 1,6	72
#nop; lnop
	andc	pbit, pbit, zovl				# 0,2	74
	lqd	save2, -32($sp)					# 1,6	74
#ifdef OVLY_IRQ_SAVE
	ila	irqtmp2, do_load90				# 0,2	75
#lnop
	andi	irq_stat, irq_stat, 1				# 0,2	76
#lnop
#else
#nop; lnop
#nop; lnop
#endif
	andc	oldvma, oldvma, pbit				# 0,2	77
	lqd	save1, -16($sp)					# 1,6	77
	nop							# 0,0	78
#lnop
#nop
	stqx	oldvma, tab5, off5				# 1,6	79
#nop
#ifdef OVLY_IRQ_SAVE
	binze	irq_stat, irqtmp2				# 1,4	80
do_load90:
#nop
	lqd	save4, -64($sp)					# 1,6	84
#else
#nop; lnop
#endif

	.global	_ovly_debug_event
	.type	_ovly_debug_event, @function
_ovly_debug_event:
	nop
/* Branch to target address. */
do_load99:
	bi	target						# 1,4	81/85

	.size	__ovly_load, . - __ovly_load
