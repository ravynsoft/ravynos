	psel	0, pn0, p0.b[w12, 0]
	psel	pn0, 0, p0.b[w12, 0]
	psel	pn0, pn0, 0

	psel	pn0, p0, p0.b[w12, 0]
	psel	pn, pn0, p0.b[w12, 0]
	psel	p0, p0, pn0.b[w12, 0]
	psel	pn0, pn0, pn0.b[w12, 0]
	psel	pn0, pn0, p0.b[w11, 0]
	psel	pn0, pn0, p0.b[w16, 0]
	psel	pn0, pn0, p0.b[w12, -1]
	psel	pn0, pn0, p0.b[w12, 16]
