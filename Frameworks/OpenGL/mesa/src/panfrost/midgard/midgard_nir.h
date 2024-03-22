#include <stdbool.h>
#include "nir.h"

bool midgard_nir_lower_algebraic_early(nir_shader *shader);
bool midgard_nir_lower_algebraic_late(nir_shader *shader);
bool midgard_nir_cancel_inot(nir_shader *shader);
void midgard_nir_type_csel(nir_shader *shader);
bool midgard_nir_lower_image_bitsize(nir_shader *shader);
