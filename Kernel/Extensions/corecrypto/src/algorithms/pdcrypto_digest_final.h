//
//  pd_crypto_digest_final.h
//  pdcrypto
//
//  Created by rafirafi on 3/17/16.
//  Copyright (c) 2016 rafirafi. All rights reserved.
//

#ifndef __pdcrypto__pd_crypto_digest_final__
#define __pdcrypto__pd_crypto_digest_final__

#include <corecrypto/ccdigest.h>

__BEGIN_DECLS
void pdcdigest_final_64le(const struct ccdigest_info *di, ccdigest_ctx_t ctx, unsigned char *digest);

void pdcdigest_final_64be(const struct ccdigest_info *di, ccdigest_ctx_t ctx, unsigned char *digest);
__END_DECLS

#endif /* defined(__pdcrypto__pd_crypto_digest_final__) */
