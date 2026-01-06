
	.section __MYZEROPAGE,_data
_min:	.long 0
	
#if __arm__
	.zerofill __MYZEROPAGE,__zerofill,_padding,536870910
#else	
	.zerofill __MYZEROPAGE,__zerofill,_padding,2147483644
#endif	
	



