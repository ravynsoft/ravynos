#ifndef __NOUVEAU_GLDEFS_H__
#define __NOUVEAU_GLDEFS_H__

static inline unsigned
nvgl_blend_func(unsigned factor)
{
	switch (factor) {
	case PIPE_BLENDFACTOR_ZERO:
		return 0x0000;
	case PIPE_BLENDFACTOR_ONE:
		return 0x0001;
	case PIPE_BLENDFACTOR_SRC_COLOR:
		return 0x0300;
	case PIPE_BLENDFACTOR_INV_SRC_COLOR:
		return 0x0301;
	case PIPE_BLENDFACTOR_SRC_ALPHA:
		return 0x0302;
	case PIPE_BLENDFACTOR_INV_SRC_ALPHA:
		return 0x0303;
	case PIPE_BLENDFACTOR_DST_ALPHA:
		return 0x0304;
	case PIPE_BLENDFACTOR_INV_DST_ALPHA:
		return 0x0305;
	case PIPE_BLENDFACTOR_DST_COLOR:
		return 0x0306;
	case PIPE_BLENDFACTOR_INV_DST_COLOR:
		return 0x0307;
	case PIPE_BLENDFACTOR_SRC_ALPHA_SATURATE:
		return 0x0308;
	case PIPE_BLENDFACTOR_CONST_COLOR:
		return 0x8001;
	case PIPE_BLENDFACTOR_INV_CONST_COLOR:
		return 0x8002;
	case PIPE_BLENDFACTOR_CONST_ALPHA:
		return 0x8003;
	case PIPE_BLENDFACTOR_INV_CONST_ALPHA:
		return 0x8004;
	default:
		return 0x0000;
	}
}

static inline unsigned
nvgl_blend_eqn(unsigned func)
{
	switch (func) {
	case PIPE_BLEND_ADD:
		return 0x8006;
	case PIPE_BLEND_MIN:
		return 0x8007;
	case PIPE_BLEND_MAX:
		return 0x8008;
	case PIPE_BLEND_SUBTRACT:
		return 0x800a;
	case PIPE_BLEND_REVERSE_SUBTRACT:
		return 0x800b;
	default:
		return 0x8006;
	}
}

static inline unsigned
nvgl_logicop_func(unsigned func)
{
	switch (func) {
	case PIPE_LOGICOP_CLEAR:
		return 0x1500;
	case PIPE_LOGICOP_NOR:
		return 0x1508;
	case PIPE_LOGICOP_AND_INVERTED:
		return 0x1504;
	case PIPE_LOGICOP_COPY_INVERTED:
		return 0x150c;
	case PIPE_LOGICOP_AND_REVERSE:
		return 0x1502;
	case PIPE_LOGICOP_INVERT:
		return 0x150a;
	case PIPE_LOGICOP_XOR:
		return 0x1506;
	case PIPE_LOGICOP_NAND:
		return 0x150e;
	case PIPE_LOGICOP_AND:
		return 0x1501;
	case PIPE_LOGICOP_EQUIV:
		return 0x1509;
	case PIPE_LOGICOP_NOOP:
		return 0x1505;
	case PIPE_LOGICOP_OR_INVERTED:
		return 0x150d;
	case PIPE_LOGICOP_COPY:
		return 0x1503;
	case PIPE_LOGICOP_OR_REVERSE:
		return 0x150b;
	case PIPE_LOGICOP_OR:
		return 0x1507;
	case PIPE_LOGICOP_SET:
		return 0x150f;
	default:
		return 0x1505;
	}
}

static inline unsigned
nvgl_comparison_op(unsigned op)
{
	switch (op) {
	case PIPE_FUNC_NEVER:
		return 0x0200;
	case PIPE_FUNC_LESS:
		return 0x0201;
	case PIPE_FUNC_EQUAL:
		return 0x0202;
	case PIPE_FUNC_LEQUAL:
		return 0x0203;
	case PIPE_FUNC_GREATER:
		return 0x0204;
	case PIPE_FUNC_NOTEQUAL:
		return 0x0205;
	case PIPE_FUNC_GEQUAL:
		return 0x0206;
	case PIPE_FUNC_ALWAYS:
		return 0x0207;
	default:
		return 0x0207;
	}
}

static inline unsigned
nvgl_polygon_mode(unsigned mode)
{
	switch (mode) {
	case PIPE_POLYGON_MODE_POINT:
		return 0x1b00;
	case PIPE_POLYGON_MODE_LINE:
		return 0x1b01;
	case PIPE_POLYGON_MODE_FILL:
		return 0x1b02;
	default:
		return 0x1b02;
	}
}

static inline unsigned
nvgl_stencil_op(unsigned op)
{
	switch (op) {
	case PIPE_STENCIL_OP_ZERO:
		return 0x0000;
	case PIPE_STENCIL_OP_INVERT:
		return 0x150a;
	case PIPE_STENCIL_OP_KEEP:
		return 0x1e00;
	case PIPE_STENCIL_OP_REPLACE:
		return 0x1e01;
	case PIPE_STENCIL_OP_INCR:
		return 0x1e02;
	case PIPE_STENCIL_OP_DECR:
		return 0x1e03;
	case PIPE_STENCIL_OP_INCR_WRAP:
		return 0x8507;
	case PIPE_STENCIL_OP_DECR_WRAP:
		return 0x8508;
	default:
		return 0x1e00;
	}
}

static inline unsigned
nvgl_primitive(unsigned prim) {
	switch (prim) {
	case MESA_PRIM_POINTS:
		return 0x0001;
	case MESA_PRIM_LINES:
		return 0x0002;
	case MESA_PRIM_LINE_LOOP:
		return 0x0003;
	case MESA_PRIM_LINE_STRIP:
		return 0x0004;
	case MESA_PRIM_TRIANGLES:
		return 0x0005;
	case MESA_PRIM_TRIANGLE_STRIP:
		return 0x0006;
	case MESA_PRIM_TRIANGLE_FAN:
		return 0x0007;
	case MESA_PRIM_QUADS:
		return 0x0008;
	case MESA_PRIM_QUAD_STRIP:
		return 0x0009;
	case MESA_PRIM_POLYGON:
		return 0x000a;
	default:
		return 0;
	}
}

#endif
