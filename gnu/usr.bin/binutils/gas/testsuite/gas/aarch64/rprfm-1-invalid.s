	rprfm	pldkeep, 0, [x0]
	rprfm	pldkeep, x0, 0

	rprfm	pldl1keep, x0, [x0]
	rprfm	#-1, x0, [x0]
	rprfm	#64, x0, [x0]
	rprfm	#1, sp, [x0]
	rprfm	#1, w0, [x0]
	rprfm	#1, x0, [xzr]
