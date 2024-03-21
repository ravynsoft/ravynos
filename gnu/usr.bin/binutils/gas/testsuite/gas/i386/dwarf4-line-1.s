	.file	"foo.c"
	.text
bar:
#APP
# 82 "foo.h" 1
	nop
# 0 "" 2
#NO_APP
	ret
foo:
	.file 1 "foo.c"
	nop
	.file 2 "foo.h"
	ret
