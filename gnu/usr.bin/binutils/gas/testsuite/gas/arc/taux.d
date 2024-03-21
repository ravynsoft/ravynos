#nm: --undefined-only
#name: aux register test
#source: taux.s
# Most of the AUX rgisters are defined for all ARC variants besides the
# FPX/FPUDA registers which should end as undefined when assemble generic.
.* U aux_dpfp1h
.* U aux_dpfp1l
.* U aux_dpfp2h
.* U aux_dpfp2l
.* U d1h
.* U d1l
.* U d2h
.* U d2l
.* U dpfp_status
.* U fp_status
#pass
