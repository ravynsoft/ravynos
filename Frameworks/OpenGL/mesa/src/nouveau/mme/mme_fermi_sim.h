/*
 * Copyright © 2022 Mary Guillemard
 * SPDX-License-Identifier: MIT
 */
#ifndef MME_FERMI_SIM_H
#define MME_FERMI_SIM_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mme_fermi_inst;

struct mme_fermi_sim_mem {
   uint64_t addr;
   void *data;
   size_t size;
};

void mme_fermi_sim(uint32_t inst_count, const struct mme_fermi_inst *insts,
                   uint32_t param_count, const uint32_t *params,
                   uint32_t mem_count, struct mme_fermi_sim_mem *mems);

#ifdef __cplusplus
}
#endif

#endif /* MME_FERMI_SIM_H */
