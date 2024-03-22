#include "nir.h"

struct rusticl_lower_state {
    size_t base_global_invoc_id_loc;
    size_t const_buf_loc;
    size_t printf_buf_loc;
    size_t format_arr_loc;
    size_t order_arr_loc;
    size_t work_dim_loc;
};

bool rusticl_lower_intrinsics(nir_shader *nir, struct rusticl_lower_state *state);
bool rusticl_lower_inputs(nir_shader *nir);
