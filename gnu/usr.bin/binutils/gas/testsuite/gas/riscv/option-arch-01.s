.attribute arch, "rv64ic"
add	a0, a0, a1
.option push
.option arch, +d2p0, -c, +xvendor1p0
add	a0, a0, a1
frcsr	a0	# Should add mapping symbol with ISA here, and then dump it to frcsr.
.option push
.option arch, +m3p0, +d3p0
.option pop
.option pop
