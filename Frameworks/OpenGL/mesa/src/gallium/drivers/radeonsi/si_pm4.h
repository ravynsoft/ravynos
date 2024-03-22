/*
 * Copyright 2012 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef SI_PM4_H
#define SI_PM4_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* forward definitions */
struct si_screen;
struct si_context;

/* State atoms are callbacks which write a sequence of packets into a GPU
 * command buffer (AKA indirect buffer, AKA IB, AKA command stream, AKA CS).
 */
struct si_atom {
   /* The index is only used by si_pm4_emit_state. Non-pm4 atoms don't use it. */
   void (*emit)(struct si_context *ctx, unsigned index);
};

struct si_pm4_state {
   struct si_screen *screen;

   /* PKT3_SET_*_REG handling */
   uint16_t last_reg;   /* register offset in dwords */
   uint16_t last_pm4;
   uint16_t ndw;        /* number of dwords in pm4 */
   uint8_t last_opcode;
   uint8_t last_idx;
   bool is_compute_queue;
   bool packed_is_padded; /* whether SET_*_REG_PAIRS_PACKED is padded to an even number of regs */

   /* For shader states only */
   struct si_atom atom;

   /* commands for the DE */
   uint16_t max_dw;

   /* Used by SQTT to override the shader address */
   uint32_t spi_shader_pgm_lo_reg;

   /* This must be the last field because the array can continue after the structure. */
   uint32_t pm4[64];
};

void si_pm4_cmd_add(struct si_pm4_state *state, uint32_t dw);
void si_pm4_set_reg(struct si_pm4_state *state, unsigned reg, uint32_t val);
void si_pm4_set_reg_idx3(struct si_pm4_state *state, unsigned reg, uint32_t val);
void si_pm4_finalize(struct si_pm4_state *state);

void si_pm4_clear_state(struct si_pm4_state *state, struct si_screen *sscreen,
                        bool is_compute_queue);
void si_pm4_free_state(struct si_context *sctx, struct si_pm4_state *state, unsigned idx);

void si_pm4_emit_commands(struct si_context *sctx, struct si_pm4_state *state);
void si_pm4_emit_state(struct si_context *sctx, unsigned index);
void si_pm4_emit_shader(struct si_context *sctx, unsigned index);
void si_pm4_reset_emitted(struct si_context *sctx);
struct si_pm4_state *si_pm4_create_sized(struct si_screen *sscreen, unsigned max_dw,
                                         bool is_compute_queue);
struct si_pm4_state *si_pm4_clone(struct si_pm4_state *orig);

#ifdef __cplusplus
}
#endif

#endif
