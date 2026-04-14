//
//  pdcrypto_digest_update.h
//  pdcrypto
//
//  Created by rafirafi on 3/17/16.
//  Copyright (c) 2016 rafirafi. All rights reserved.
//

#ifndef __pdcrypto__pdcrypto_digest_update__
#define __pdcrypto__pdcrypto_digest_update__

#include <corecrypto/ccdigest.h>

void pdcdigest_update(const struct ccdigest_info *di, ccdigest_ctx_t ctx,
                     unsigned long len, const void *data);

#endif /* defined(__pdcrypto__pdcrypto_digest_update__) */
