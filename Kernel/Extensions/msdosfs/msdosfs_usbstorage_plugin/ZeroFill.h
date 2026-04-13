//
//  ZeroFill.h
//  livefiles_msdos
//
//  Created by Yakov Ben Zaken on 28/11/2017.
//

#ifndef ZeroFill_h
#define ZeroFill_h

#include "Common.h"

int     ZeroFill_Init       ( void );
void    ZeroFill_DeInit     ( void );
int     ZeroFill_Fill       ( int iFd, uint64_t uOffset, uint32_t uLength );


#endif /* ZeroFill_h */
