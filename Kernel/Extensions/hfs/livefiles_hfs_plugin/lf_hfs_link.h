/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_link.h
 *  livefiles_hfs
 *
 *  Created by Or Haimovich on 17/05/2018.
 */

#ifndef lf_hfs_link_h
#define lf_hfs_link_h

#include "lf_hfs_catalog.h"
#include "lf_hfs_cnode.h"
#include "lf_hfs.h"

void    hfs_relorigin(struct cnode *cp, cnid_t parentcnid);
void    hfs_savelinkorigin(cnode_t *cp, cnid_t parentcnid);
void    hfs_privatedir_init(struct hfsmount * hfsmp, enum privdirtype type);
int     hfs_lookup_lastlink (struct hfsmount *hfsmp, cnid_t linkfileid, cnid_t *lastid, struct cat_desc *cdesc);
int     hfs_unlink(struct hfsmount *hfsmp, struct vnode *dvp, struct vnode *vp, struct componentname *cnp, int skip_reserve);
void    hfs_relorigins(struct cnode *cp);
int     hfs_makelink(struct hfsmount *hfsmp, struct vnode *src_vp, struct cnode *cp,struct cnode *dcp, struct componentname *cnp);
cnid_t  hfs_currentparent(cnode_t *cp, bool have_lock);
#endif /* lf_hfs_link_h */
