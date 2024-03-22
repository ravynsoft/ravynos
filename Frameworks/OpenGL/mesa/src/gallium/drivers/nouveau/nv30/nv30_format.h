#ifndef __NV30_FORMAT_H__
#define __NV30_FORMAT_H__

struct nv30_format_info {
   unsigned bindings;
};

struct nv30_format {
   unsigned hw;
};

struct nv30_vtxfmt {
   unsigned hw;
};

struct nv30_texfmt {
   unsigned nv30;
   unsigned nv30_rect;
   unsigned nv40;
   struct {
      unsigned src;
      unsigned cmp;
   } swz[6];
   unsigned swizzle;
   unsigned filter;
   unsigned wrap;
};

extern const struct nv30_format_info nv30_format_info_table[];
static inline const struct nv30_format_info *
nv30_format_info(struct pipe_screen *pscreen, enum pipe_format format)
{
   return &nv30_format_info_table[format];
}

extern const struct nv30_format nv30_format_table[];
static inline const struct nv30_format *
nv30_format(struct pipe_screen *pscreen, enum pipe_format format)
{
   return &nv30_format_table[format];
}

extern const struct nv30_vtxfmt nv30_vtxfmt_table[];
static inline const struct nv30_vtxfmt *
nv30_vtxfmt(struct pipe_screen *pscreen, enum pipe_format format)
{
   return &nv30_vtxfmt_table[format];
}

extern const struct nv30_texfmt nv30_texfmt_table[];
static inline const struct nv30_texfmt *
nv30_texfmt(struct pipe_screen *pscreen, enum pipe_format format)
{
   return &nv30_texfmt_table[format];
}

#endif
