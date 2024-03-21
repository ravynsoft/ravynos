# Test rdasr
	.text
	rd %pcr,%g0
	rd %pic,%g1
	rd %dcr,%g2
	rd %gsr,%g3
	rd %softint,%g4
	rd %tick_cmpr,%g5
	rd %sys_tick,%g5
	rd %sys_tick_cmpr,%g4
	rd %cfr,%g6
	rd %entropy,%g2
