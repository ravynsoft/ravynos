
#include "Number.h"

#define LOTS_O_PROBES  { NUMBER_HIT(1); NUMBER_HIT(2); NUMBER_HIT(3); NUMBER_HIT(4); }


inline void foo() { 
	LOTS_O_PROBES 
}


