#include <stddef.h>
#include <corecrypto/cc.h>
#include <corecrypto/ccdigest.h>
#include "cc_abort.h"

void pdcdigest_fn(const struct ccdigest_info *di, unsigned long len, const void *data, void *digest) {
	ccdigest_di_decl(di, ctx);
	ccdigest_init(di, ctx);
	ccdigest_update(di, ctx, len, data);
	ccdigest_final(di, ctx, (unsigned char *)digest);
	ccdigest_di_clear(di, ctx);
}
