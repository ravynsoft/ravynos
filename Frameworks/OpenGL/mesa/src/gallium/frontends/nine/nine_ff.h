
#ifndef _NINE_FF_H_
#define _NINE_FF_H_

#include "device9.h"
#include "vertexdeclaration9.h"

bool nine_ff_init(struct NineDevice9 *);
void nine_ff_fini(struct NineDevice9 *);

void nine_ff_update(struct NineDevice9 *);

void
nine_d3d_matrix_matrix_mul(D3DMATRIX *, const D3DMATRIX *, const D3DMATRIX *);

void
nine_d3d_vector4_matrix_mul(D3DVECTOR *, const D3DVECTOR *, const D3DMATRIX *);
void
nine_d3d_vector3_matrix_mul(D3DVECTOR *, const D3DVECTOR *, const D3DMATRIX *);

float
nine_d3d_matrix_det(const D3DMATRIX *);

void
nine_d3d_matrix_inverse(D3DMATRIX *, const D3DMATRIX *);

void
nine_d3d_matrix_transpose(D3DMATRIX *, const D3DMATRIX *);

#define NINED3DTSS_TCI_DISABLE                       0
#define NINED3DTSS_TCI_PASSTHRU                      1
#define NINED3DTSS_TCI_CAMERASPACENORMAL             2
#define NINED3DTSS_TCI_CAMERASPACEPOSITION           3
#define NINED3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR   4
#define NINED3DTSS_TCI_SPHEREMAP                     5

static inline unsigned
nine_decltype_get_dim(BYTE type)
{
    switch (type) {
    case D3DDECLTYPE_FLOAT1: return 1;
    case D3DDECLTYPE_FLOAT2: return 2;
    case D3DDECLTYPE_FLOAT3: return 3;
    case D3DDECLTYPE_FLOAT4: return 4;
    case D3DDECLTYPE_D3DCOLOR: return 1;
    case D3DDECLTYPE_UBYTE4: return 4;
    case D3DDECLTYPE_SHORT2: return 2;
    case D3DDECLTYPE_SHORT4: return 4;
    case D3DDECLTYPE_UBYTE4N: return 4;
    case D3DDECLTYPE_SHORT2N: return 2;
    case D3DDECLTYPE_SHORT4N: return 4;
    case D3DDECLTYPE_USHORT2N: return 2;
    case D3DDECLTYPE_USHORT4N: return 4;
    case D3DDECLTYPE_UDEC3: return 3;
    case D3DDECLTYPE_DEC3N: return 3;
    case D3DDECLTYPE_FLOAT16_2: return 2;
    case D3DDECLTYPE_FLOAT16_4: return 4;
    default:
        assert(!"Implementation error !");
    }
    return 0;
}

static inline uint16_t
nine_ff_get_projected_key(struct nine_context *context, unsigned num_stages)
{
    unsigned s, i;
    uint16_t projected = 0;
    int8_t input_texture_coord[num_stages];
    memset(&input_texture_coord, 0, sizeof(input_texture_coord));

    if (context->vdecl) {
        for (i = 0; i < context->vdecl->nelems; i++) {
            uint16_t usage = context->vdecl->usage_map[i];
            if (usage % NINE_DECLUSAGE_COUNT == NINE_DECLUSAGE_TEXCOORD) {
                s = usage / NINE_DECLUSAGE_COUNT;
                if (s < num_stages)
                    input_texture_coord[s] = nine_decltype_get_dim(context->vdecl->decls[i].Type);
            }
        }
    }

    for (s = 0; s < num_stages; ++s) {
        unsigned gen = (context->ff.tex_stage[s][D3DTSS_TEXCOORDINDEX] >> 16) + 1;
        unsigned dim = context->ff.tex_stage[s][D3DTSS_TEXTURETRANSFORMFLAGS] & 0x7;
        unsigned proj = !!(context->ff.tex_stage[s][D3DTSS_TEXTURETRANSFORMFLAGS] & D3DTTFF_PROJECTED);

        if (!context->vs) {
            if (dim > 4)
                dim = input_texture_coord[s];

            if (!dim && gen == NINED3DTSS_TCI_PASSTHRU)
                dim = input_texture_coord[s];
            else if (!dim)
                dim = 4;

            if (dim == 1) /* NV behaviour */
                proj = 0;
            if (dim > input_texture_coord[s] && gen == NINED3DTSS_TCI_PASSTHRU)
                proj = 0;
        } else {
            dim = 4;
        }
        if (proj)
            projected |= (dim-1) << (2 * s);
    }
    return projected;
}

static inline uint16_t
nine_ff_get_projected_key_ff(struct nine_context *context)
{
    /* 8 stages */
    return nine_ff_get_projected_key(context, 8);
}

static inline uint8_t
nine_ff_get_projected_key_programmable(struct nine_context *context)
{
    /* We only look at the 4 stages because this function
     * is used only for ps 1.1-3, where only the first four
     * slots are available */
    return (uint8_t)nine_ff_get_projected_key(context, 4);
}

#endif /* _NINE_FF_H_ */
