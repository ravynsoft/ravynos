target:
	th.sfence.vmas	a0, a1
	th.sync
	th.sync.i
	th.sync.is
	th.sync.s
