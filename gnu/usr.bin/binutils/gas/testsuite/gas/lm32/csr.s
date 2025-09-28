                
        wcsr    IE, r0
        wcsr    IE, r31
        wcsr    IM, r0
        wcsr    IM, r31
        wcsr    ICC, r0
        wcsr    ICC, r31
        wcsr    DCC, r0
        wcsr    DCC, r31
        
        rcsr    r0, IE
        rcsr    r31, IE
        rcsr    r0, IM
        rcsr    r31, IM
        rcsr    r0, IP
        rcsr    r31, IP
        rcsr    r0, CC
        rcsr    r31, CC
        rcsr    r0, CFG
        rcsr    r31, CFG
