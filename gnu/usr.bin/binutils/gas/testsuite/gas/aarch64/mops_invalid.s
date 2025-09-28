	.arch	armv8.8-a

	cpyfp	x0, [x1]!, x2!
	cpyfp	x0!, [x1]!, x2!
	cpyfp	[x0], [x1]!, x2!
	cpyfp	[x0, #0]!, [x1]!, x2!
	cpyfp	[x0, xzr]!, [x1]!, x2!

	cpyfp	[x1]!, x0, x2!
	cpyfp	[x1]!, x0!, x2!
	cpyfp	[x1]!, [x0], x2!
	cpyfp	[x1]!, [x0, #0]!, x2!
	cpyfp	[x1]!, [x0, xzr]!, x2!

	cpyfp	[x0]!, [x1]!, x2
	cpyfp	[x0]!, [x1]!, !x2
	cpyfp	[x0]!, [x1]!, [x2]
	cpyfp	[x0]!, [x1]!, [x2]!

	cpyfp	[x31]!, [x0]!, x1!
	cpyfp	[sp]!, [x0]!, x1!
	cpyfp	[zr]!, [x0]!, x1!
	cpyfp	[w30]!, [x0]!, x1!
	cpyfp	[w0]!, [x1]!, x2!
	cpyfp	[wsp]!, [x0]!, x1!
	cpyfp	[wzr]!, [x0]!, x1!
	cpyfp	[b0]!, [x1]!, x2!
	cpyfp	[h0]!, [x1]!, x2!
	cpyfp	[s0]!, [x1]!, x2!
	cpyfp	[d0]!, [x1]!, x2!
	cpyfp	[q0]!, [x1]!, x2!
	cpyfp	[v0]!, [x1]!, x2!
	cpyfp	[v0.2d]!, [x1]!, x2!
	cpyfp	[z0]!, [x1]!, x2!
	cpyfp	[z0.d]!, [x1]!, x2!
	cpyfp	[p0]!, [x1]!, x2!
	cpyfp	[p0.d]!, [x1]!, x2!
	cpyfp	[foo]!, [x1]!, x2!

	cpyfp	[x0]!, [x31]!, x1!
	cpyfp	[x0]!, [sp]!, x1!
	cpyfp	[x0]!, [zr]!, x1!
	cpyfp	[x0]!, [w30]!, x1!
	cpyfp	[x1]!, [w0]!, x2!
	cpyfp	[x0]!, [wsp]!, x1!
	cpyfp	[x0]!, [wzr]!, x1!
	cpyfp	[x1]!, [foo]!, x2!

	cpyfp	[x0]!, [x1]!, x31!
	cpyfp	[x0]!, [x1]!, sp!
	cpyfp	[x0]!, [x1]!, zr!
	cpyfp	[x0]!, [x1]!, w30!
	cpyfp	[x1]!, [x2]!, w0!
	cpyfp	[x0]!, [x1]!, wsp!
	cpyfp	[x0]!, [x1]!, wzr!
	cpyfp	[x1]!, [x2]!, foo!

	cpyfp	[x0]!, [x0]!, x1!
	cpyfp	[x10]!, [x1]!, x10!
	cpyfp	[x1]!, [x30]!, x30!

	setp	x0, x1!, x2
	setp	x0!, x1!, x2
	setp	[x0], x1!, x2
	setp	[x0, #0]!, x1!, x2
	setp	[x0, xzr]!, x1!, x2

	setp	[x31]!, x0!, x1
	setp	[sp]!, x0!, x1
	setp	[zr]!, x0!, x1
	setp	[w30]!, x0!, x1
	setp	[w0]!, x1!, x2
	setp	[wsp]!, x0!, x1
	setp	[wzr]!, x0!, x1
	setp	[foo]!, x1!, x2

	setp	[x0]!, x31!, x1
	setp	[x0]!, sp!, x1
	setp	[x0]!, zr!, x1
	setp	[x0]!, w30!, x1
	setp	[x1]!, w0!, x2
	setp	[x0]!, wsp!, x1
	setp	[x0]!, wzr!, x1
	setp	[x1]!, foo!, x2

	setp	[x30]!, x0!, sp
	setp	[x30]!, x0!, wsp
	setp	[x30]!, x0!, wzr

	setp	[x0]!, x0!, x1
	setp	[x10]!, x1!, x10
	setp	[x1]!, x30!, x30

	.arch	armv8.7-a

	cpyfp [x0]!, [x1]!, x2!
	cpyfm [x0]!, [x1]!, x2!
	cpyfe [x0]!, [x1]!, x2!
	cpyfprn [x0]!, [x1]!, x2!
	cpyfmrn [x0]!, [x1]!, x2!
	cpyfern [x0]!, [x1]!, x2!
	cpyfpwn [x0]!, [x1]!, x2!
	cpyfmwn [x0]!, [x1]!, x2!
	cpyfewn [x0]!, [x1]!, x2!
	cpyfpn [x0]!, [x1]!, x2!
	cpyfmn [x0]!, [x1]!, x2!
	cpyfen [x0]!, [x1]!, x2!
	cpyfprt [x0]!, [x1]!, x2!
	cpyfmrt [x0]!, [x1]!, x2!
	cpyfert [x0]!, [x1]!, x2!
	cpyfprtrn [x0]!, [x1]!, x2!
	cpyfmrtrn [x0]!, [x1]!, x2!
	cpyfertrn [x0]!, [x1]!, x2!
	cpyfprtwn [x0]!, [x1]!, x2!
	cpyfmrtwn [x0]!, [x1]!, x2!
	cpyfertwn [x0]!, [x1]!, x2!
	cpyfprtn [x0]!, [x1]!, x2!
	cpyfmrtn [x0]!, [x1]!, x2!
	cpyfertn [x0]!, [x1]!, x2!
	cpyfpwt [x0]!, [x1]!, x2!
	cpyfmwt [x0]!, [x1]!, x2!
	cpyfewt [x0]!, [x1]!, x2!
	cpyfpwtrn [x0]!, [x1]!, x2!
	cpyfmwtrn [x0]!, [x1]!, x2!
	cpyfewtrn [x0]!, [x1]!, x2!
	cpyfpwtwn [x0]!, [x1]!, x2!
	cpyfmwtwn [x0]!, [x1]!, x2!
	cpyfewtwn [x0]!, [x1]!, x2!
	cpyfpwtn [x0]!, [x1]!, x2!
	cpyfmwtn [x0]!, [x1]!, x2!
	cpyfewtn [x0]!, [x1]!, x2!
	cpyfpt [x0]!, [x1]!, x2!
	cpyfmt [x0]!, [x1]!, x2!
	cpyfet [x0]!, [x1]!, x2!
	cpyfptrn [x0]!, [x1]!, x2!
	cpyfmtrn [x0]!, [x1]!, x2!
	cpyfetrn [x0]!, [x1]!, x2!
	cpyfptwn [x0]!, [x1]!, x2!
	cpyfmtwn [x0]!, [x1]!, x2!
	cpyfetwn [x0]!, [x1]!, x2!
	cpyfptn [x0]!, [x1]!, x2!
	cpyfmtn [x0]!, [x1]!, x2!
	cpyfetn [x0]!, [x1]!, x2!

	cpyp [x0]!, [x1]!, x2!
	cpym [x0]!, [x1]!, x2!
	cpye [x0]!, [x1]!, x2!
	cpyprn [x0]!, [x1]!, x2!
	cpymrn [x0]!, [x1]!, x2!
	cpyern [x0]!, [x1]!, x2!
	cpypwn [x0]!, [x1]!, x2!
	cpymwn [x0]!, [x1]!, x2!
	cpyewn [x0]!, [x1]!, x2!
	cpypn [x0]!, [x1]!, x2!
	cpymn [x0]!, [x1]!, x2!
	cpyen [x0]!, [x1]!, x2!
	cpyprt [x0]!, [x1]!, x2!
	cpymrt [x0]!, [x1]!, x2!
	cpyert [x0]!, [x1]!, x2!
	cpyprtrn [x0]!, [x1]!, x2!
	cpymrtrn [x0]!, [x1]!, x2!
	cpyertrn [x0]!, [x1]!, x2!
	cpyprtwn [x0]!, [x1]!, x2!
	cpymrtwn [x0]!, [x1]!, x2!
	cpyertwn [x0]!, [x1]!, x2!
	cpyprtn [x0]!, [x1]!, x2!
	cpymrtn [x0]!, [x1]!, x2!
	cpyertn [x0]!, [x1]!, x2!
	cpypwt [x0]!, [x1]!, x2!
	cpymwt [x0]!, [x1]!, x2!
	cpyewt [x0]!, [x1]!, x2!
	cpypwtrn [x0]!, [x1]!, x2!
	cpymwtrn [x0]!, [x1]!, x2!
	cpyewtrn [x0]!, [x1]!, x2!
	cpypwtwn [x0]!, [x1]!, x2!
	cpymwtwn [x0]!, [x1]!, x2!
	cpyewtwn [x0]!, [x1]!, x2!
	cpypwtn [x0]!, [x1]!, x2!
	cpymwtn [x0]!, [x1]!, x2!
	cpyewtn [x0]!, [x1]!, x2!
	cpypt [x0]!, [x1]!, x2!
	cpymt [x0]!, [x1]!, x2!
	cpyet [x0]!, [x1]!, x2!
	cpyptrn [x0]!, [x1]!, x2!
	cpymtrn [x0]!, [x1]!, x2!
	cpyetrn [x0]!, [x1]!, x2!
	cpyptwn [x0]!, [x1]!, x2!
	cpymtwn [x0]!, [x1]!, x2!
	cpyetwn [x0]!, [x1]!, x2!
	cpyptn [x0]!, [x1]!, x2!
	cpymtn [x0]!, [x1]!, x2!
	cpyetn [x0]!, [x1]!, x2!

	setp [x0]!, x1!, x2
	setm [x0]!, x1!, x2
	sete [x0]!, x1!, x2
	setpt [x0]!, x1!, x2
	setmt [x0]!, x1!, x2
	setet [x0]!, x1!, x2
	setpn [x0]!, x1!, x2
	setmn [x0]!, x1!, x2
	seten [x0]!, x1!, x2
	setptn [x0]!, x1!, x2
	setmtn [x0]!, x1!, x2
	setetn [x0]!, x1!, x2

	setgp [x0]!, x1!, x2
	setgm [x0]!, x1!, x2
	setge [x0]!, x1!, x2
	setgpt [x0]!, x1!, x2
	setgmt [x0]!, x1!, x2
	setget [x0]!, x1!, x2
	setgpn [x0]!, x1!, x2
	setgmn [x0]!, x1!, x2
	setgen [x0]!, x1!, x2
	setgptn [x0]!, x1!, x2
	setgmtn [x0]!, x1!, x2
	setgetn [x0]!, x1!, x2

	.arch	armv8.7-a+mops

	setgp [x0]!, x1!, x2
	setgm [x0]!, x1!, x2
	setge [x0]!, x1!, x2
	setgpt [x0]!, x1!, x2
	setgmt [x0]!, x1!, x2
	setget [x0]!, x1!, x2
	setgpn [x0]!, x1!, x2
	setgmn [x0]!, x1!, x2
	setgen [x0]!, x1!, x2
	setgptn [x0]!, x1!, x2
	setgmtn [x0]!, x1!, x2
	setgetn [x0]!, x1!, x2

	.arch	armv8.7-a+memtag

	setgp [x0]!, x1!, x2
	setgm [x0]!, x1!, x2
	setge [x0]!, x1!, x2
	setgpt [x0]!, x1!, x2
	setgmt [x0]!, x1!, x2
	setget [x0]!, x1!, x2
	setgpn [x0]!, x1!, x2
	setgmn [x0]!, x1!, x2
	setgen [x0]!, x1!, x2
	setgptn [x0]!, x1!, x2
	setgmtn [x0]!, x1!, x2
	setgetn [x0]!, x1!, x2
