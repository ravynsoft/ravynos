/**************************************************************************
 * 
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
 *
 **************************************************************************/


#ifndef ST_CB_DRAWTEX_H
#define ST_CB_DRAWTEX_H


struct st_context;

void st_DrawTex(struct gl_context *ctx, GLfloat x, GLfloat y, GLfloat z,
                GLfloat width, GLfloat height);

extern void
st_destroy_drawtex(struct st_context *st);

#endif /* ST_CB_DRAWTEX_H */
