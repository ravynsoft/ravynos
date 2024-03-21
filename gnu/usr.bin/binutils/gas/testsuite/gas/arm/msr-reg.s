@ Check MSR and MRS instruction operand syntax.
@ Also check for MSR/MRS acceptance in ARM/THUMB modes.

.section .text
.syntax unified

	@ Write to Special Register from register
	msr	APSR,r9    @ deprecated usage.
	msr	APSR_g,r9
	msr	APSR_nzcvq,r9
	msr	APSR_nzcvqg,r9

	@ Write to CPSR flags
	msr 	CPSR,r9
	msr 	CPSR_s,r9
	msr	CPSR_f,r9
	msr 	CPSR_c,r9
	msr	CPSR_x,r9

	@ Write to CPSR flag combos
	msr	CPSR_fs, r9   
	msr	CPSR_fx, r9   
	msr	CPSR_fc, r9   
	msr	CPSR_sf, r9   
	msr	CPSR_sx, r9   
	msr	CPSR_sc, r9   
	msr	CPSR_xf, r9   
	msr	CPSR_xs, r9   
	msr	CPSR_xc, r9   
	msr	CPSR_cf, r9   
	msr	CPSR_cs, r9   
	msr	CPSR_cx, r9   
	msr	CPSR_fsx, r9  
	msr	CPSR_fsc, r9  
	msr	CPSR_fxs, r9  
	msr	CPSR_fxc, r9  
	msr	CPSR_fcs, r9  
	msr	CPSR_fcx, r9  
	msr	CPSR_sfx, r9  
	msr	CPSR_sfc, r9  
	msr	CPSR_sxf, r9  
	msr	CPSR_sxc, r9  
	msr	CPSR_scf, r9  
	msr	CPSR_scx, r9  
	msr	CPSR_xfs, r9  
	msr	CPSR_xfc, r9  
	msr	CPSR_xsf, r9  
	msr	CPSR_xsc, r9  
	msr	CPSR_xcf, r9  
	msr	CPSR_xcs, r9  
	msr	CPSR_cfs, r9  
	msr	CPSR_cfx, r9  
	msr	CPSR_csf, r9  
	msr	CPSR_csx, r9  
	msr	CPSR_cxf, r9  
	msr	CPSR_cxs, r9  
	msr	CPSR_fsxc, r9 
	msr	CPSR_fscx, r9 
	msr	CPSR_fxsc, r9 
	msr	CPSR_fxcs, r9 
	msr	CPSR_fcsx, r9 
	msr	CPSR_fcxs, r9 
	msr	CPSR_sfxc, r9 
	msr	CPSR_sfcx, r9 
	msr	CPSR_sxfc, r9 
	msr	CPSR_sxcf, r9 
	msr	CPSR_scfx, r9 
	msr	CPSR_scxf, r9 
	msr	CPSR_xfsc, r9 
	msr	CPSR_xfcs, r9 
	msr	CPSR_xsfc, r9 
	msr	CPSR_xscf, r9 
	msr	CPSR_xcfs, r9 
	msr	CPSR_xcsf, r9 
	msr	CPSR_cfsx, r9 
	msr	CPSR_cfxs, r9 
	msr	CPSR_csfx, r9 
	msr	CPSR_csxf, r9 
	msr	CPSR_cxfs, r9 
	msr	CPSR_cxsf, r9 

	@ Write to SPSR flags
	msr 	SPSR,r9
	msr 	SPSR_s,r9
	msr	SPSR_f,r9
	msr 	SPSR_c,r9
	msr	SPSR_x,r9

	@ Write to Saved status register
	msr	SPSR_fs, r9   
	msr	SPSR_fx, r9   
	msr	SPSR_fc, r9   
	msr	SPSR_sf, r9   
	msr	SPSR_sx, r9   
	msr	SPSR_sc, r9   
	msr	SPSR_xf, r9   
	msr	SPSR_xs, r9   
	msr	SPSR_xc, r9   
	msr	SPSR_cf, r9   
	msr	SPSR_cs, r9   
	msr	SPSR_cx, r9   
	msr	SPSR_fsx, r9  
	msr	SPSR_fsc, r9  
	msr	SPSR_fxs, r9  
	msr	SPSR_fxc, r9  
	msr	SPSR_fcs, r9  
	msr	SPSR_fcx, r9  
	msr	SPSR_sfx, r9  
	msr	SPSR_sfc, r9  
	msr	SPSR_sxf, r9  
	msr	SPSR_sxc, r9  
	msr	SPSR_scf, r9  
	msr	SPSR_scx, r9  
	msr	SPSR_xfs, r9  
	msr	SPSR_xfc, r9  
	msr	SPSR_xsf, r9  
	msr	SPSR_xsc, r9  
	msr	SPSR_xcf, r9  
	msr	SPSR_xcs, r9  
	msr	SPSR_cfs, r9  
	msr	SPSR_cfx, r9  
	msr	SPSR_csf, r9  
	msr	SPSR_csx, r9  
	msr	SPSR_cxf, r9  
	msr	SPSR_cxs, r9  
	msr	SPSR_fsxc, r9 
	msr	SPSR_fscx, r9 
	msr	SPSR_fxsc, r9 
	msr	SPSR_fxcs, r9 
	msr	SPSR_fcsx, r9 
	msr	SPSR_fcxs, r9 
	msr	SPSR_sfxc, r9 
	msr	SPSR_sfcx, r9 
	msr	SPSR_sxfc, r9 
	msr	SPSR_sxcf, r9 
	msr	SPSR_scfx, r9 
	msr	SPSR_scxf, r9 
	msr	SPSR_xfsc, r9 
	msr	SPSR_xfcs, r9 
	msr	SPSR_xsfc, r9 
	msr	SPSR_xscf, r9 
	msr	SPSR_xcfs, r9 
	msr	SPSR_xcsf, r9 
	msr	SPSR_cfsx, r9 
	msr	SPSR_cfxs, r9 
	msr	SPSR_csfx, r9 
	msr	SPSR_csxf, r9 
	msr	SPSR_cxfs, r9 
	msr	SPSR_cxsf, r9 
