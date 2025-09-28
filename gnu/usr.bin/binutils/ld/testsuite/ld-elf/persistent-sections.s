.section	.persistent,"aw"
.word 1

.section	.persistent.var_persistent,"aw"
.word 2

.section	.gnu.linkonce.p.var_persistent2,"aw"
.word 3

.text
.global _start
_start:
.word 0
