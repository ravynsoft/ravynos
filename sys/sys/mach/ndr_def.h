#include <sys/mach/ndr.h>

#if _BYTE_ORDER == _LITTLE_ENDIAN
#define NDR_INT_ENDIAN NDR_INT_LITTLE_ENDIAN
#elif _BYTE_ORDER == _BIG_ENDIAN
#define NDR_INT_ENDIAN NDR_INT_BIG_ENDIAN
#else
#error "bad endian value"
#endif


NDR_record_t NDR_record = {
        0,                      /* mig_reserved */
        0,                      /* mig_reserved */
        0,                      /* mig_reserved */
        NDR_PROTOCOL_2_0,               
        NDR_INT_ENDIAN,
        NDR_CHAR_ASCII,
        NDR_FLOAT_IEEE,
        0,
};
