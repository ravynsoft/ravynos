@ Check MSR and MRS instruction operand syntax.
@ Also check for MSR/MRS acceptance in ARM/THUMB modes.

.section .text
.syntax unified

	@ Write to Special Register from Immediate
	@ Write to application status register
	msr	APSR_nzcvq,#0xc0000004
	msr	APSR_g,#0xc0000004
	msr	APSR_nzcvq,#0xc0000004
	msr	APSR_nzcvqg,#0xc0000004

	@ Write to CPSR flags
	msr 	CPSR,#0xc0000004
	msr 	CPSR_s,#0xc0000004
	msr	CPSR_f,#0xc0000004
	msr 	CPSR_c,#0xc0000004
	msr	CPSR_x,#0xc0000004

	@ Write to CPSR flag combos
	msr	CPSR_fs, #0xc0000004   
	msr	CPSR_fx, #0xc0000004   
	msr	CPSR_fc, #0xc0000004   
	msr	CPSR_sf, #0xc0000004   
	msr	CPSR_sx, #0xc0000004   
	msr	CPSR_sc, #0xc0000004   
	msr	CPSR_xf, #0xc0000004   
	msr	CPSR_xs, #0xc0000004   
	msr	CPSR_xc, #0xc0000004   
	msr	CPSR_cf, #0xc0000004   
	msr	CPSR_cs, #0xc0000004   
	msr	CPSR_cx, #0xc0000004   
	msr	CPSR_fsx, #0xc0000004  
	msr	CPSR_fsc, #0xc0000004  
	msr	CPSR_fxs, #0xc0000004  
	msr	CPSR_fxc, #0xc0000004  
	msr	CPSR_fcs, #0xc0000004  
	msr	CPSR_fcx, #0xc0000004  
	msr	CPSR_sfx, #0xc0000004  
	msr	CPSR_sfc, #0xc0000004  
	msr	CPSR_sxf, #0xc0000004  
	msr	CPSR_sxc, #0xc0000004  
	msr	CPSR_scf, #0xc0000004  
	msr	CPSR_scx, #0xc0000004  
	msr	CPSR_xfs, #0xc0000004  
	msr	CPSR_xfc, #0xc0000004  
	msr	CPSR_xsf, #0xc0000004  
	msr	CPSR_xsc, #0xc0000004  
	msr	CPSR_xcf, #0xc0000004  
	msr	CPSR_xcs, #0xc0000004  
	msr	CPSR_cfs, #0xc0000004  
	msr	CPSR_cfx, #0xc0000004  
	msr	CPSR_csf, #0xc0000004  
	msr	CPSR_csx, #0xc0000004  
	msr	CPSR_cxf, #0xc0000004  
	msr	CPSR_cxs, #0xc0000004  
	msr	CPSR_fsxc, #0xc0000004 
	msr	CPSR_fscx, #0xc0000004 
	msr	CPSR_fxsc, #0xc0000004 
	msr	CPSR_fxcs, #0xc0000004 
	msr	CPSR_fcsx, #0xc0000004 
	msr	CPSR_fcxs, #0xc0000004 
	msr	CPSR_sfxc, #0xc0000004 
	msr	CPSR_sfcx, #0xc0000004 
	msr	CPSR_sxfc, #0xc0000004 
	msr	CPSR_sxcf, #0xc0000004 
	msr	CPSR_scfx, #0xc0000004 
	msr	CPSR_scxf, #0xc0000004 
	msr	CPSR_xfsc, #0xc0000004 
	msr	CPSR_xfcs, #0xc0000004 
	msr	CPSR_xsfc, #0xc0000004 
	msr	CPSR_xscf, #0xc0000004 
	msr	CPSR_xcfs, #0xc0000004 
	msr	CPSR_xcsf, #0xc0000004 
	msr	CPSR_cfsx, #0xc0000004 
	msr	CPSR_cfxs, #0xc0000004 
	msr	CPSR_csfx, #0xc0000004 
	msr	CPSR_csxf, #0xc0000004 
	msr	CPSR_cxfs, #0xc0000004 
	msr	CPSR_cxsf, #0xc0000004 

	@ Write to Saved status register
	@ Write to SPSR flags
	msr 	SPSR,   #0xc0000004
	msr 	SPSR_s, #0xc0000004
	msr	SPSR_f, #0xc0000004
	msr 	SPSR_c, #0xc0000004
	msr	SPSR_x, #0xc0000004

	@Write to SPSR flag combos
	msr	SPSR_fs, #0xc0000004   
	msr	SPSR_fx, #0xc0000004   
	msr	SPSR_fc, #0xc0000004   
	msr	SPSR_sf, #0xc0000004   
	msr	SPSR_sx, #0xc0000004   
	msr	SPSR_sc, #0xc0000004   
	msr	SPSR_xf, #0xc0000004   
	msr	SPSR_xs, #0xc0000004   
	msr	SPSR_xc, #0xc0000004   
	msr	SPSR_cf, #0xc0000004   
	msr	SPSR_cs, #0xc0000004   
	msr	SPSR_cx, #0xc0000004   
	msr	SPSR_fsx, #0xc0000004  
	msr	SPSR_fsc, #0xc0000004  
	msr	SPSR_fxs, #0xc0000004  
	msr	SPSR_fxc, #0xc0000004  
	msr	SPSR_fcs, #0xc0000004  
	msr	SPSR_fcx, #0xc0000004  
	msr	SPSR_sfx, #0xc0000004  
	msr	SPSR_sfc, #0xc0000004  
	msr	SPSR_sxf, #0xc0000004  
	msr	SPSR_sxc, #0xc0000004  
	msr	SPSR_scf, #0xc0000004  
	msr	SPSR_scx, #0xc0000004  
	msr	SPSR_xfs, #0xc0000004  
	msr	SPSR_xfc, #0xc0000004  
	msr	SPSR_xsf, #0xc0000004  
	msr	SPSR_xsc, #0xc0000004  
	msr	SPSR_xcf, #0xc0000004  
	msr	SPSR_xcs, #0xc0000004  
	msr	SPSR_cfs, #0xc0000004  
	msr	SPSR_cfx, #0xc0000004  
	msr	SPSR_csf, #0xc0000004  
	msr	SPSR_csx, #0xc0000004  
	msr	SPSR_cxf, #0xc0000004  
	msr	SPSR_cxs, #0xc0000004  
	msr	SPSR_fsxc, #0xc0000004 
	msr	SPSR_fscx, #0xc0000004 
	msr	SPSR_fxsc, #0xc0000004 
	msr	SPSR_fxcs, #0xc0000004 
	msr	SPSR_fcsx, #0xc0000004 
	msr	SPSR_fcxs, #0xc0000004 
	msr	SPSR_sfxc, #0xc0000004 
	msr	SPSR_sfcx, #0xc0000004 
	msr	SPSR_sxfc, #0xc0000004 
	msr	SPSR_sxcf, #0xc0000004 
	msr	SPSR_scfx, #0xc0000004 
	msr	SPSR_scxf, #0xc0000004 
	msr	SPSR_xfsc, #0xc0000004 
	msr	SPSR_xfcs, #0xc0000004 
	msr	SPSR_xsfc, #0xc0000004 
	msr	SPSR_xscf, #0xc0000004 
	msr	SPSR_xcfs, #0xc0000004 
	msr	SPSR_xcsf, #0xc0000004 
	msr	SPSR_cfsx, #0xc0000004 
	msr	SPSR_cfxs, #0xc0000004 
	msr	SPSR_csfx, #0xc0000004 
	msr	SPSR_csxf, #0xc0000004 
	msr	SPSR_cxfs, #0xc0000004 
	msr	SPSR_cxsf, #0xc0000004 


