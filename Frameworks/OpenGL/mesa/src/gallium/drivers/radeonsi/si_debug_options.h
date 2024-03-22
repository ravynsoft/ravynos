OPT_BOOL(inline_uniforms, true, "Optimize shaders by replacing uniforms with literals")
OPT_BOOL(aux_debug, false, "Generate ddebug_dumps for the auxiliary context")
OPT_BOOL(sync_compile, false, "Always compile synchronously (will cause stalls)")
OPT_BOOL(dump_shader_binary, false, "Dump shader binary as part of ddebug_dumps")
OPT_BOOL(debug_disassembly, false,
         "Report shader disassembly as part of driver debug messages (for shader db)")
OPT_BOOL(halt_shaders, false, "Halt shaders at the start (will hang)")
OPT_BOOL(vs_fetch_always_opencode, false,
         "Always open code vertex fetches (less efficient, purely for testing)")
OPT_BOOL(no_infinite_interp, false, "Kill PS with infinite interp coeff")
OPT_BOOL(clamp_div_by_zero, false, "Clamp div by zero (x / 0 becomes FLT_MAX instead of NaN)")
OPT_BOOL(vrs2x2, false, "Enable 2x2 coarse shading for non-GUI elements")
OPT_BOOL(enable_sam, false, "Enable Smart Access Memory with Above 4G Decoding for unvalidated platforms.")
OPT_BOOL(disable_sam, false, "Disable Smart Access Memory.")
OPT_BOOL(fp16, false, "Enable FP16 for mediump.")
/* for Maya */
OPT_INT(tc_max_cpu_storage_size, 2500, "Enable the CPU storage for pipelined buffer uploads in TC.")
/* for Creo and Snx */
OPT_INT(max_vram_map_size, 8196, "Maximum size of a buffer in VRAM to map directly. Bigger buffers use staging uploads.")
OPT_BOOL(force_use_fma32, false, "Force use fma32 instruction for GPU family newer than gfx9")
OPT_BOOL(dcc_msaa, false, "Enable DCC for MSAA")
OPT_BOOL(zerovram, false, "Zero all VRAM allocations")
OPT_BOOL(clear_lds, false, "Clear LDS at the end of shaders. Might decrease performance.")

#undef OPT_BOOL
#undef OPT_INT
