;;; TLS tests: check tpoff addendum and resolving local TLS symbols.

	.cpu HS
	.global		a
	.section	.tbss,"awT",@nobits
	.align		4
	.type		a, @object
	.size		a, 60
a:
	.zero		 60

	.type		local_tls_var, @object
	.size		local_tls_var, 4
local_tls_var:
	.zero		4


	.section	.text
	.align 4
foo:
;;; Test if the tpoff addendum is correctly generated.
	add r2,r25,@a@tpoff+48
;;; Test if local TLS symbol is correctly resolved.
	add r0,r0,@local_tls_var@dtpoff@.tbss
