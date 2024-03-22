// DriConf options specific to radeonsi
DRI_CONF_SECTION_PERFORMANCE
DRI_CONF_ADAPTIVE_SYNC(true)
DRI_CONF_MESA_GLTHREAD_DRIVER(true)
DRI_CONF_SECTION_END

DRI_CONF_SECTION_DEBUG
#define OPT_BOOL(name, dflt, description) DRI_CONF_OPT_B(radeonsi_##name, dflt, description)
#define OPT_INT(name, dflt, description) DRI_CONF_OPT_I(radeonsi_##name, dflt, INT_MIN, INT_MAX, description)

#include "si_debug_options.h"
DRI_CONF_SECTION_END
