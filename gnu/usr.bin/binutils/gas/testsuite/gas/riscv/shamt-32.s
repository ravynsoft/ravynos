	.option arch, -c
	slli	a0, a0, 0
	slli	a0, a0, 31
	slli	a0, a0, 32
	slli	a0, a0, 63

	srli	a0, a0, 0
	srli	a0, a0, 31
	srli	a0, a0, 32
	srli	a0, a0, 63

	srai	a0, a0, 0
	srai	a0, a0, 31
	srai	a0, a0, 32
	srai	a0, a0, 63

	.option arch, +c
	slli	a0, a0, 0
	slli	a0, a0, 31
	slli	a0, a0, 32
	slli	a0, a0, 63

	srli	a0, a0, 0
	srli	a0, a0, 31
	srli	a0, a0, 32
	srli	a0, a0, 63

	srai	a0, a0, 0
	srai	a0, a0, 31
	srai	a0, a0, 32
	srai	a0, a0, 63
