
#ifndef _NINE_DUMP_H_
#define _NINE_DUMP_H_

#include "d3d9types.h"
#include "d3d9caps.h"

const char *nine_D3DDEVTYPE_to_str(D3DDEVTYPE);
const char *nine_D3DQUERYTYPE_to_str(D3DQUERYTYPE);
const char *nine_D3DTSS_to_str(D3DTEXTURESTAGESTATETYPE);
const char *nine_D3DTOP_to_str(D3DTEXTUREOP);
const char *nine_D3DPOOL_to_str(D3DPOOL);
const char *nine_D3DRTYPE_to_str(D3DRESOURCETYPE);
const char *nine_D3DUSAGE_to_str(DWORD);
const char *nine_D3DPRESENTFLAG_to_str(DWORD);
const char *nine_D3DLOCK_to_str(DWORD);
const char *nine_D3DSAMP_to_str(DWORD);

#if defined(DEBUG) || !defined(NDEBUG)

void
nine_dump_D3DADAPTER_IDENTIFIER9(unsigned, const D3DADAPTER_IDENTIFIER9 *);
void
nine_dump_D3DCAPS9(unsigned, const D3DCAPS9 *);
void
nine_dump_D3DLIGHT9(unsigned, const D3DLIGHT9 *);
void
nine_dump_D3DMATERIAL9(unsigned, const D3DMATERIAL9 *);
void
nine_dump_D3DTSS_value(unsigned, D3DTEXTURESTAGESTATETYPE, DWORD);

#else /* !DEBUG && NDEBUG */

static inline void
nine_dump_D3DADAPTER_IDENTIFIER9(unsigned ch, const D3DADAPTER_IDENTIFIER9 *id)
{ }
static inline void
nine_dump_D3DCAPS9(unsigned ch, const D3DCAPS9 *caps)
{ }
static inline void
nine_dump_D3DLIGHT9(unsigned ch, const D3DLIGHT9 *light)
{ }
static inline void
nine_dump_D3DMATERIAL9(unsigned ch, const D3DMATERIAL9 *mat)
{ }
static inline void
nine_dump_D3DTSS_value(unsigned ch, D3DTEXTURESTAGESTATETYPE tss, DWORD value)
{ }

#endif /* DEBUG || !NDEBUG */

#endif /* _NINE_DUMP_H_H_ */
