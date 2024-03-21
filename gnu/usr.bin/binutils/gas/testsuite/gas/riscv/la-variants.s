# lla is always local, lga is always global, and la depends on pic
	.extern a
.text
	.option nopic
	la a0, a
	lla a1, a
	lga a2, a
	.option pic
	la a3, a
	lla a4, a
	lga a5, a
