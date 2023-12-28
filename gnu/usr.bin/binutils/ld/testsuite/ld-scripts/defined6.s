 .globl defined
 defined == 1
 .globl undef
 .comm common,1
 .weak weak
 weak == 2
 .weak undefweak
 .data
 .dc.a undefweak
