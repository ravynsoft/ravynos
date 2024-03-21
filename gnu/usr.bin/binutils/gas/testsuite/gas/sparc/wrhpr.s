# Test wrpr
	.text
	wrhpr %g1, %g2, %hpstate
	wrhpr %g1, %hpstate
	wrhpr %g1,666, %hpstate
	wrhpr 666, %g1, %hpstate
	wrhpr 666, %hpstate
	wrhpr %g1, %g2, %htstate
	wrhpr %g1, %htstate
	wrhpr %g1,666, %htstate
	wrhpr 666, %g1, %htstate
	wrhpr 666, %htstate
	wrhpr %g1, %g2, %hintp
	wrhpr %g1, %hintp
	wrhpr %g1,666, %hintp
	wrhpr 666, %g1, %hintp
	wrhpr 666, %hintp
	wrhpr %g1, %g2, %htba
	wrhpr %g1, %htba
	wrhpr %g1,666, %htba
	wrhpr 666, %g1, %htba
	wrhpr 666, %htba
	wrhpr %g1, %g2, %hmcdper
	wrhpr %g1, %hmcdper
	wrhpr %g1,666, %hmcdper
	wrhpr 666, %g1, %hmcdper
	wrhpr 666, %hmcdper
	wrhpr %g1, %g2, %hmcddfr
	wrhpr %g1, %hmcddfr
	wrhpr %g1,666, %hmcddfr
	wrhpr 666, %g1, %hmcddfr
	wrhpr 666, %hmcddfr
	wrhpr %g1, %g2, %hva_mask_nz
	wrhpr %g1, %hva_mask_nz
	wrhpr %g1,666, %hva_mask_nz
	wrhpr 666, %g1, %hva_mask_nz
	wrhpr 666, %hva_mask_nz
	wrhpr %g1, %g2, %hstick_offset
	wrhpr %g1, %hstick_offset
	wrhpr %g1,666, %hstick_offset
	wrhpr 666, %g1, %hstick_offset
	wrhpr 666, %hstick_offset
	wrhpr %g1, %g2, %hstick_enable
	wrhpr %g1, %hstick_enable
	wrhpr %g1,666, %hstick_enable
	wrhpr 666, %g1, %hstick_enable
	wrhpr 666, %hstick_enable
	wrhpr %g1, %g2, %hstick_cmpr
	wrhpr %g1, %hstick_cmpr
	wrhpr %g1,666, %hstick_cmpr
	wrhpr 666, %g1, %hstick_cmpr
	wrhpr 666, %hstick_cmpr
