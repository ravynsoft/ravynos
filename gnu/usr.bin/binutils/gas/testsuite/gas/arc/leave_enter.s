	.cpu HS
	leave_s {r13}
	leave_s {r13-r13}
	leave_s {r13-r26,pcl}
	leave_s {r13-r26,fp}
	leave_s {r13-r26,blink}
	leave_s {r13-r26,fp,blink}
	leave_s {r13-r26,fp,pcl}
	leave_s {r13-r26,blink,pcl}
	leave_s {r13-r26,fp,blink,pcl}
	leave_s {r13,blink,pcl}
	leave_s {blink,pcl}
	leave_s {fp}
	leave_s {blink}
	leave_s {pcl}

	ld 	r0,[r1]

	enter_s {r13}
	enter_s {r13-r13}
	enter_s {r13-r26,fp}
	enter_s {r13-r26,blink}
	enter_s {r13-r26,fp,blink}
	enter_s {r13,blink}
	enter_s {blink}
	enter_s {fp, blink}
	enter_s {fp}
