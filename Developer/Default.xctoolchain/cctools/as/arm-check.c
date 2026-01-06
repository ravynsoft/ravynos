#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "opcode/arm.h"

static void do_noargs(void) {}
static void do_arit(void) {}
static void do_t_arit3c(void) {}
static void do_t_add_sub(void) {}
static void do_t_arit3(void) {}
static void do_cmp(void) {}
static void do_t_mvn_tst(void) {}
static void do_t_mov_cmp(void) {}
static void do_mov(void) {}
static void do_ldst(void) {}
static void do_t_ldst(void) {}
static void do_ldmstm(void) {}
static void do_t_ldmstm(void) {}
static void do_swi(void) {}
static void do_t_swi(void) {}
static void do_branch(void) {}
static void do_t_branch(void) {}
static void do_bl(void) {}
static void do_t_branch23(void) {}
static void do_adr(void) {}
static void do_t_adr(void) {}
static void do_adrl(void) {}
static void do_nop(void) {}
static void do_t_nop(void) {}
static void do_shift(void) {}
static void do_t_shift(void) {}
static void do_rd_rn(void) {}
static void do_t_neg(void) {}
static void do_push_pop(void) {}
static void do_t_push_pop(void) {}
static void do_t_rsb(void) {}
static void do_rd_rm(void) {}
static void do_t_rd_rm(void) {}
static void do_t_cpy(void) {}
static void do_ldstt(void) {}
static void do_t_ldstt(void) {}
static void do_mul(void) {}
static void do_t_mul(void) {}
static void do_mlas(void) {}
static void do_t_mla(void) {}
static void do_cdp(void) {}
static void do_lstc(void) {}
static void do_co_reg(void) {}
static void do_rd_rm_rn(void) {}
static void do_mrs(void) {}
static void do_t_mrs(void) {}
static void do_msr(void) {}
static void do_t_msr(void) {}
static void do_mull(void) {}
static void do_t_mull(void) {}
static void do_ldstv4(void) {}
static void do_bx(void) {}
static void do_t_bx(void) {}
static void do_blx(void) {}
static void do_t_blx(void) {}
static void do_bkpt(void) {}
static void do_t_bkpt(void) {}
static void do_t_clz(void) {}
static void do_smla(void) {}
static void do_smlal(void) {}
static void do_t_mlal(void) {}
static void do_smul(void) {}
static void do_t_simd(void) {}
static void do_t_rd_rm_rn(void) {}
static void do_pld(void) {}
static void do_t_pld(void) {}
static void do_ldrd(void) {}
static void do_t_ldstd(void) {}
static void do_co_reg2c(void) {}
static void do_bxj(void) {}
static void do_t_bxj(void) {}
static void do_cpsi(void) {}
static void do_t_cpsi(void) {}
static void do_t_rev(void) {}
static void do_sxth(void) {}
static void do_t_sxth(void) {}
static void do_setend(void) {}
static void do_t_setend(void) {}
static void do_ldrex(void) {}
static void do_t_ldrex(void) {}
static void do_strex(void) {}
static void do_t_strex(void) {}
static void do_ssat(void) {}
static void do_t_ssat(void) {}
static void do_usat(void) {}
static void do_t_usat(void) {}
static void do_imm0(void) {}
static void do_t_cps(void) {}
static void do_pkhbt(void) {}
static void do_t_pkhbt(void) {}
static void do_pkhtb(void) {}
static void do_t_pkhtb(void) {}
static void do_rd_rn_rm(void) {}
static void do_rfe(void) {}
static void do_sxtah(void) {}
static void do_t_sxtah(void) {}
static void do_srs(void) {}
static void do_ssat16(void) {}
static void do_t_ssat16(void) {}
static void do_usat16(void) {}
static void do_t_usat16(void) {}
static void do_t_hint(void) {}
static void do_ldrexd(void) {}
static void do_t_ldrexd(void) {}
static void do_strexd(void) {}
static void do_t_strexd(void) {}
static void do_rm_rd_rn(void) {}
static void do_smc(void) {}
static void do_t_smc(void) {}
static void do_bfc(void) {}
static void do_t_bfc(void) {}
static void do_bfi(void) {}
static void do_t_bfi(void) {}
static void do_bfx(void) {}
static void do_t_bfx(void) {}
static void do_mov16(void) {}
static void do_t_mov16(void) {}
static void do_t_rbit(void) {}
static void do_ldsttv4(void) {}
static void do_t_cbz(void) {}
static void do_it(void) {}
static void do_t_it(void) {}
static void do_t_add_sub_w(void) {}
static void do_t_tb(void) {}
static void do_t_orn(void) {}
static void do_t_div(void) {}
static void do_pli(void) {}
static void do_dbg(void) {}
static void do_t_dbg(void) {}
static void do_barrier(void) {}
static void do_t_barrier(void) {}
static void do_rd(void) {}
static void do_rd_cpaddr(void) {}
static void do_fpa_cmp(void) {}
static void do_rn_rd(void) {}
static void do_fpa_ldmstm(void) {}
static void do_vfp_sp_monadic(void) {}
static void do_vfp_reg_from_sp(void) {}
static void do_vfp_sp_from_reg(void) {}
static void do_vmrs(void) {}
static void do_vfp_sp_ldst(void) {}
static void do_vfp_sp_ldstmia(void) {}
static void do_vfp_sp_ldstmdb(void) {}
static void do_vfp_xp_ldstmia(void) {}
static void do_vfp_xp_ldstmdb(void) {}
static void do_vfp_sp_dyadic(void) {}
static void do_vfp_sp_compare_z(void) {}
static void do_vfp_dp_rd_rm(void) {}
static void do_vfp_dp_sp_cvt(void) {}
static void do_vfp_sp_dp_cvt(void) {}
static void do_vfp_dp_rn_rd(void) {}
static void do_vfp_dp_rd_rn(void) {}
static void do_vfp_sp_hp_cvt(void) {}
static void do_vfp_hp_sp_cvt(void) {}
static void do_vfp_t_sp_hp_cvt(void) {}
static void do_vfp_b_sp_hp_cvt(void) {}
static void do_vfp_t_hp_sp_cvt(void) {}
static void do_vfp_b_hp_sp_cvt(void) {}
static void do_vfp_dp_ldst(void) {}
static void do_vfp_dp_ldstmia(void) {}
static void do_vfp_dp_ldstmdb(void) {}
static void do_vfp_dp_rd_rn_rm(void) {}
static void do_vfp_dp_rd(void) {}
static void do_vfp_sp2_from_reg2(void) {}
static void do_vfp_reg2_from_sp2(void) {}
static void do_vfp_dp_rm_rd_rn(void) {}
static void do_vfp_nsyn_sqrt(void) {}
static void do_vfp_nsyn_div(void) {}
static void do_vfp_nsyn_nmul(void) {}
static void do_vfp_nsyn_cmp(void) {}
static void do_vfp_nsyn_push(void) {}
static void do_vfp_nsyn_pop(void) {}
static void do_vfp_nsyn_cvtr(void) {}
static void do_neon_mul(void) {}
static void do_neon_mac_maybe_scalar(void) {}
static void do_neon_addsub_if_i(void) {}
static void do_neon_abs_neg(void) {}
static void do_neon_ldm_stm(void) {}
static void do_neon_ldr_str(void) {}
static void do_neon_cvt(void) {}
static void do_neon_cvtt(void) {}
static void do_neon_cvtb(void) {}
static void do_neon_mov(void) {}
static void do_neon_dyadic_i_su(void) {}
static void do_neon_dyadic_i64_su(void) {}
static void do_neon_rshl(void) {}
static void do_neon_shl_imm(void) {}
static void do_neon_qshl_imm(void) {}
static void do_neon_logic(void) {}
static void do_neon_bitfield(void) {}
static void do_neon_dyadic_if_su(void) {}
static void do_neon_cmp(void) {}
static void do_neon_cmp_inv(void) {}
static void do_neon_ceq(void) {}
static void do_neon_dyadic_if_su_d(void) {}
static void do_neon_dyadic_if_i_d(void) {}
static void do_neon_tst(void) {}
static void do_neon_qdmulh(void) {}
static void do_neon_fcmp_absolute(void) {}
static void do_neon_fcmp_absolute_inv(void) {}
static void do_neon_step(void) {}
static void do_neon_rshift_round_imm(void) {}
static void do_neon_sli(void) {}
static void do_neon_sri(void) {}
static void do_neon_qshlu_imm(void) {}
static void do_neon_rshift_sat_narrow(void) {}
static void do_neon_rshift_sat_narrow_u(void) {}
static void do_neon_rshift_narrow(void) {}
static void do_neon_shll(void) {}
static void do_neon_mvn(void) {}
static void do_neon_abal(void) {}
static void do_neon_dyadic_long(void) {}
static void do_neon_mac_maybe_scalar_long(void) {}
static void do_neon_dyadic_wide(void) {}
static void do_neon_dyadic_narrow(void) {}
static void do_neon_mul_sat_scalar_long(void) {}
static void do_neon_vmull(void) {}
static void do_neon_ext(void) {}
static void do_neon_rev(void) {}
static void do_neon_dup(void) {}
static void do_neon_movl(void) {}
static void do_neon_movn(void) {}
static void do_neon_qmovn(void) {}
static void do_neon_qmovun(void) {}
static void do_neon_zip_uzp(void) {}
static void do_neon_sat_abs_neg(void) {}
static void do_neon_pair_long(void) {}
static void do_neon_recip_est(void) {}
static void do_neon_cls(void) {}
static void do_neon_clz(void) {}
static void do_neon_cnt(void) {}
static void do_neon_swp(void) {}
static void do_neon_trn(void) {}
static void do_neon_tbl_tbx(void) {}
static void do_neon_ldx_stx(void) {}
static void do_vfp_sp_const(void) {}
static void do_vfp_dp_const(void) {}
static void do_vfp_sp_conv_16(void) {}
static void do_vfp_dp_conv_16(void) {}
static void do_vfp_sp_conv_32(void) {}
static void do_vfp_dp_conv_32(void) {}
static void do_xsc_mia(void) {}
static void do_xsc_mar(void) {}
static void do_xsc_mra(void) {}
static void do_iwmmxt_tandorc(void) {}
static void do_iwmmxt_textrc(void) {}
static void do_iwmmxt_textrm(void) {}
static void do_iwmmxt_tinsr(void) {}
static void do_iwmmxt_tmia(void) {}
static void do_iwmmxt_waligni(void) {}
static void do_iwmmxt_wldstbh(void) {}
static void do_iwmmxt_wldstw(void) {}
static void do_iwmmxt_wldstd(void) {}
static void do_iwmmxt_wmov(void) {}
static void do_iwmmxt_wrwrwr_or_imm5(void) {}
static void do_iwmmxt_wshufh(void) {}
static void do_iwmmxt_wzero(void) {}
static void do_iwmmxt_wmerge(void) {}
static void do_mav_dspsc(void) {}
static void do_mav_triple(void) {}
static void do_mav_shift(void) {}
static void do_mav_quad(void) {}

#define INSNS_TABLE_ONLY
#include "arm.c"

static
void
print_insns(
const struct asm_opcode *insns)
{
    unsigned int j, r;

	printf("\t%s\t", insns->template);
	r = 1;
	for (j = 0; j < 8 && insns->operands[j] != OP_stop; j++) {
	    switch(insns->operands[j]){
	    case OP_RR:
	    case OP_oRR:
	    case OP_SH:
	    case OP_SHG:
	    case OP_RRnpc:
		printf("r%d", r++);
		break;
	    default:
		printf("UNKNOWN_OP_%d", insns->operands[j]);
	    }
	    if(insns->operands[j+1] != OP_stop)
		printf(",");
	}
	printf("\n");
}

int
main(
int argc,
char **argv)
{
    unsigned int i, len;
    char *p;

	printf("arm:\n");
	for(i = 1; i < sizeof (insns) / sizeof (struct asm_opcode); i++){
	    /*
	     * This hack is looking for arm ALU instructions that set the
	     * condition codes.  And the matching instruction that does not
	     * set them.  Those instructions are listed in the table after
	     * each other with the 's' suffix one second.  Bit 20 is the s-bit
	     * that indicated it sets the condition codes or not.
	     */
	    p = strrchr(insns[i].template, 's');
	    len = p - insns[i].template;
	    if(p != NULL && p[1] == '\0' &&
	       (insns[i].avalue & 0x100000) == 0x100000 &&
	       strncmp(insns[i].template, insns[i-1].template, len) == 0 &&
	       (insns[i-1].avalue & 0x100000) == 0){
		print_insns(insns + i - 1);
		print_insns(insns + i);
	    }
	}
	return(0);
}


