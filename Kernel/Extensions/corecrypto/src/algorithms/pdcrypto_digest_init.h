//
//  pdcrypto_digest_init.h
//  pdcrypto
//
//  Created by rafirafi on 3/17/16.
//  Copyright (c) 2016 rafirafi. All rights reserved.
//

#ifndef __pdcrypto__pdcrypto_digest_init__
#define __pdcrypto__pdcrypto_digest_init__

#include <corecrypto/ccdigest.h>

void pdcdigest_init(const struct ccdigest_info *di, ccdigest_ctx_t ctx);

#endif /* defined(__pdcrypto__pdcrypto_digest_init__) */
