//
//  systemx.h
//  hfs
//
//  Created by Chris Suter on 8/12/15.
//
//

#ifndef systemx_c
#define systemx_c

__BEGIN_DECLS

#define SYSTEMX_QUIET		((void *)1)

int __attribute__((sentinel)) systemx(const char *prog, ...);

__END_DECLS

#endif /* systemx_c */
