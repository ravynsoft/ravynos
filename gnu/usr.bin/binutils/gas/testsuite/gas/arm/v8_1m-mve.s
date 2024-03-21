.syntax unified

vstmia r0,{d0-d3}
vldmia    r7!, {d3-d4}

vpop {d0-d2}
vpush {d0-d2}
vpop {d2-d5}
vpush {d1-d6}

vpst
vstrwt.u32 q1, [q0, #-4]

vstr FPSCR, [r2] @ Accepts offset variant without immediate

vstr d0,[r0]
vldr d0,[r0]
vstr s0,[r0]
vldr s0,[r0]

vstr d15,[r1]
vldr d15,[r1]
vstr s31,[r1]
vldr s31,[r1]
	
vpush {s0-s31}		// -> false error, is a valid command
vpush {s0-s15}		// OK
vpop {s0-s15}		// OK
vpop {s0-s31}		// -> false error, is a valid command
