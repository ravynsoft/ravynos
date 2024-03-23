
/* Generated data (by glib-mkenums) */

#include "config.h"

#include "cairo-gobject.h"

GType
cairo_gobject_status_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_STATUS_SUCCESS, "CAIRO_STATUS_SUCCESS", "success" },
          { CAIRO_STATUS_NO_MEMORY, "CAIRO_STATUS_NO_MEMORY", "no-memory" },
          { CAIRO_STATUS_INVALID_RESTORE, "CAIRO_STATUS_INVALID_RESTORE", "invalid-restore" },
          { CAIRO_STATUS_INVALID_POP_GROUP, "CAIRO_STATUS_INVALID_POP_GROUP", "invalid-pop-group" },
          { CAIRO_STATUS_NO_CURRENT_POINT, "CAIRO_STATUS_NO_CURRENT_POINT", "no-current-point" },
          { CAIRO_STATUS_INVALID_MATRIX, "CAIRO_STATUS_INVALID_MATRIX", "invalid-matrix" },
          { CAIRO_STATUS_INVALID_STATUS, "CAIRO_STATUS_INVALID_STATUS", "invalid-status" },
          { CAIRO_STATUS_NULL_POINTER, "CAIRO_STATUS_NULL_POINTER", "null-pointer" },
          { CAIRO_STATUS_INVALID_STRING, "CAIRO_STATUS_INVALID_STRING", "invalid-string" },
          { CAIRO_STATUS_INVALID_PATH_DATA, "CAIRO_STATUS_INVALID_PATH_DATA", "invalid-path-data" },
          { CAIRO_STATUS_READ_ERROR, "CAIRO_STATUS_READ_ERROR", "read-error" },
          { CAIRO_STATUS_WRITE_ERROR, "CAIRO_STATUS_WRITE_ERROR", "write-error" },
          { CAIRO_STATUS_SURFACE_FINISHED, "CAIRO_STATUS_SURFACE_FINISHED", "surface-finished" },
          { CAIRO_STATUS_SURFACE_TYPE_MISMATCH, "CAIRO_STATUS_SURFACE_TYPE_MISMATCH", "surface-type-mismatch" },
          { CAIRO_STATUS_PATTERN_TYPE_MISMATCH, "CAIRO_STATUS_PATTERN_TYPE_MISMATCH", "pattern-type-mismatch" },
          { CAIRO_STATUS_INVALID_CONTENT, "CAIRO_STATUS_INVALID_CONTENT", "invalid-content" },
          { CAIRO_STATUS_INVALID_FORMAT, "CAIRO_STATUS_INVALID_FORMAT", "invalid-format" },
          { CAIRO_STATUS_INVALID_VISUAL, "CAIRO_STATUS_INVALID_VISUAL", "invalid-visual" },
          { CAIRO_STATUS_FILE_NOT_FOUND, "CAIRO_STATUS_FILE_NOT_FOUND", "file-not-found" },
          { CAIRO_STATUS_INVALID_DASH, "CAIRO_STATUS_INVALID_DASH", "invalid-dash" },
          { CAIRO_STATUS_INVALID_DSC_COMMENT, "CAIRO_STATUS_INVALID_DSC_COMMENT", "invalid-dsc-comment" },
          { CAIRO_STATUS_INVALID_INDEX, "CAIRO_STATUS_INVALID_INDEX", "invalid-index" },
          { CAIRO_STATUS_CLIP_NOT_REPRESENTABLE, "CAIRO_STATUS_CLIP_NOT_REPRESENTABLE", "clip-not-representable" },
          { CAIRO_STATUS_TEMP_FILE_ERROR, "CAIRO_STATUS_TEMP_FILE_ERROR", "temp-file-error" },
          { CAIRO_STATUS_INVALID_STRIDE, "CAIRO_STATUS_INVALID_STRIDE", "invalid-stride" },
          { CAIRO_STATUS_FONT_TYPE_MISMATCH, "CAIRO_STATUS_FONT_TYPE_MISMATCH", "font-type-mismatch" },
          { CAIRO_STATUS_USER_FONT_IMMUTABLE, "CAIRO_STATUS_USER_FONT_IMMUTABLE", "user-font-immutable" },
          { CAIRO_STATUS_USER_FONT_ERROR, "CAIRO_STATUS_USER_FONT_ERROR", "user-font-error" },
          { CAIRO_STATUS_NEGATIVE_COUNT, "CAIRO_STATUS_NEGATIVE_COUNT", "negative-count" },
          { CAIRO_STATUS_INVALID_CLUSTERS, "CAIRO_STATUS_INVALID_CLUSTERS", "invalid-clusters" },
          { CAIRO_STATUS_INVALID_SLANT, "CAIRO_STATUS_INVALID_SLANT", "invalid-slant" },
          { CAIRO_STATUS_INVALID_WEIGHT, "CAIRO_STATUS_INVALID_WEIGHT", "invalid-weight" },
          { CAIRO_STATUS_INVALID_SIZE, "CAIRO_STATUS_INVALID_SIZE", "invalid-size" },
          { CAIRO_STATUS_USER_FONT_NOT_IMPLEMENTED, "CAIRO_STATUS_USER_FONT_NOT_IMPLEMENTED", "user-font-not-implemented" },
          { CAIRO_STATUS_DEVICE_TYPE_MISMATCH, "CAIRO_STATUS_DEVICE_TYPE_MISMATCH", "device-type-mismatch" },
          { CAIRO_STATUS_DEVICE_ERROR, "CAIRO_STATUS_DEVICE_ERROR", "device-error" },
          { CAIRO_STATUS_INVALID_MESH_CONSTRUCTION, "CAIRO_STATUS_INVALID_MESH_CONSTRUCTION", "invalid-mesh-construction" },
	  { CAIRO_STATUS_DEVICE_FINISHED, "CAIRO_STATUS_DEVICE_FINISHED", "device-finished" },
	  { CAIRO_STATUS_JBIG2_GLOBAL_MISSING, "CAIRO_STATUS_JBIG2_GLOBAL_MISSING", "jbig2-global_missing" },
	  { CAIRO_STATUS_PNG_ERROR, "CAIRO_STATUS_PNG_ERROR", "png-error" },
	  { CAIRO_STATUS_FREETYPE_ERROR, "CAIRO_STATUS_FREETYPE_ERROR", "freetype-error" },
	  { CAIRO_STATUS_LAST_STATUS, "CAIRO_STATUS_LAST_STATUS", "last-status" },
	  { CAIRO_STATUS_WIN32_GDI_ERROR, "CAIRO_STATUS_WIN32_GDI_ERROR", "win32-gdi-error" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_status_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_content_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_CONTENT_COLOR, "CAIRO_CONTENT_COLOR", "color" },
          { CAIRO_CONTENT_ALPHA, "CAIRO_CONTENT_ALPHA", "alpha" },
          { CAIRO_CONTENT_COLOR_ALPHA, "CAIRO_CONTENT_COLOR_ALPHA", "color-alpha" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_content_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_operator_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_OPERATOR_CLEAR, "CAIRO_OPERATOR_CLEAR", "clear" },
          { CAIRO_OPERATOR_SOURCE, "CAIRO_OPERATOR_SOURCE", "source" },
          { CAIRO_OPERATOR_OVER, "CAIRO_OPERATOR_OVER", "over" },
          { CAIRO_OPERATOR_IN, "CAIRO_OPERATOR_IN", "in" },
          { CAIRO_OPERATOR_OUT, "CAIRO_OPERATOR_OUT", "out" },
          { CAIRO_OPERATOR_ATOP, "CAIRO_OPERATOR_ATOP", "atop" },
          { CAIRO_OPERATOR_DEST, "CAIRO_OPERATOR_DEST", "dest" },
          { CAIRO_OPERATOR_DEST_OVER, "CAIRO_OPERATOR_DEST_OVER", "dest-over" },
          { CAIRO_OPERATOR_DEST_IN, "CAIRO_OPERATOR_DEST_IN", "dest-in" },
          { CAIRO_OPERATOR_DEST_OUT, "CAIRO_OPERATOR_DEST_OUT", "dest-out" },
          { CAIRO_OPERATOR_DEST_ATOP, "CAIRO_OPERATOR_DEST_ATOP", "dest-atop" },
          { CAIRO_OPERATOR_XOR, "CAIRO_OPERATOR_XOR", "xor" },
          { CAIRO_OPERATOR_ADD, "CAIRO_OPERATOR_ADD", "add" },
          { CAIRO_OPERATOR_SATURATE, "CAIRO_OPERATOR_SATURATE", "saturate" },
          { CAIRO_OPERATOR_MULTIPLY, "CAIRO_OPERATOR_MULTIPLY", "multiply" },
          { CAIRO_OPERATOR_SCREEN, "CAIRO_OPERATOR_SCREEN", "screen" },
          { CAIRO_OPERATOR_OVERLAY, "CAIRO_OPERATOR_OVERLAY", "overlay" },
          { CAIRO_OPERATOR_DARKEN, "CAIRO_OPERATOR_DARKEN", "darken" },
          { CAIRO_OPERATOR_LIGHTEN, "CAIRO_OPERATOR_LIGHTEN", "lighten" },
          { CAIRO_OPERATOR_COLOR_DODGE, "CAIRO_OPERATOR_COLOR_DODGE", "color-dodge" },
          { CAIRO_OPERATOR_COLOR_BURN, "CAIRO_OPERATOR_COLOR_BURN", "color-burn" },
          { CAIRO_OPERATOR_HARD_LIGHT, "CAIRO_OPERATOR_HARD_LIGHT", "hard-light" },
          { CAIRO_OPERATOR_SOFT_LIGHT, "CAIRO_OPERATOR_SOFT_LIGHT", "soft-light" },
          { CAIRO_OPERATOR_DIFFERENCE, "CAIRO_OPERATOR_DIFFERENCE", "difference" },
          { CAIRO_OPERATOR_EXCLUSION, "CAIRO_OPERATOR_EXCLUSION", "exclusion" },
          { CAIRO_OPERATOR_HSL_HUE, "CAIRO_OPERATOR_HSL_HUE", "hsl-hue" },
          { CAIRO_OPERATOR_HSL_SATURATION, "CAIRO_OPERATOR_HSL_SATURATION", "hsl-saturation" },
          { CAIRO_OPERATOR_HSL_COLOR, "CAIRO_OPERATOR_HSL_COLOR", "hsl-color" },
          { CAIRO_OPERATOR_HSL_LUMINOSITY, "CAIRO_OPERATOR_HSL_LUMINOSITY", "hsl-luminosity" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_operator_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_antialias_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_ANTIALIAS_DEFAULT, "CAIRO_ANTIALIAS_DEFAULT", "default" },

          { CAIRO_ANTIALIAS_NONE, "CAIRO_ANTIALIAS_NONE", "none" },
          { CAIRO_ANTIALIAS_GRAY, "CAIRO_ANTIALIAS_GRAY", "gray" },
          { CAIRO_ANTIALIAS_SUBPIXEL, "CAIRO_ANTIALIAS_SUBPIXEL", "subpixel" },

          { CAIRO_ANTIALIAS_FAST, "CAIRO_ANTIALIAS_FAST", "fast" },
          { CAIRO_ANTIALIAS_GOOD, "CAIRO_ANTIALIAS_GOOD", "good" },
          { CAIRO_ANTIALIAS_BEST, "CAIRO_ANTIALIAS_BEST", "best" },

          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_antialias_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_fill_rule_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_FILL_RULE_WINDING, "CAIRO_FILL_RULE_WINDING", "winding" },
          { CAIRO_FILL_RULE_EVEN_ODD, "CAIRO_FILL_RULE_EVEN_ODD", "even-odd" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_fill_rule_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_line_cap_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_LINE_CAP_BUTT, "CAIRO_LINE_CAP_BUTT", "butt" },
          { CAIRO_LINE_CAP_ROUND, "CAIRO_LINE_CAP_ROUND", "round" },
          { CAIRO_LINE_CAP_SQUARE, "CAIRO_LINE_CAP_SQUARE", "square" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_line_cap_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_line_join_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_LINE_JOIN_MITER, "CAIRO_LINE_JOIN_MITER", "miter" },
          { CAIRO_LINE_JOIN_ROUND, "CAIRO_LINE_JOIN_ROUND", "round" },
          { CAIRO_LINE_JOIN_BEVEL, "CAIRO_LINE_JOIN_BEVEL", "bevel" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_line_join_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_text_cluster_flags_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_TEXT_CLUSTER_FLAG_BACKWARD, "CAIRO_TEXT_CLUSTER_FLAG_BACKWARD", "backward" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_text_cluster_flags_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_font_slant_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_FONT_SLANT_NORMAL, "CAIRO_FONT_SLANT_NORMAL", "normal" },
          { CAIRO_FONT_SLANT_ITALIC, "CAIRO_FONT_SLANT_ITALIC", "italic" },
          { CAIRO_FONT_SLANT_OBLIQUE, "CAIRO_FONT_SLANT_OBLIQUE", "oblique" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_font_slant_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_font_weight_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_FONT_WEIGHT_NORMAL, "CAIRO_FONT_WEIGHT_NORMAL", "normal" },
          { CAIRO_FONT_WEIGHT_BOLD, "CAIRO_FONT_WEIGHT_BOLD", "bold" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_font_weight_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_subpixel_order_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_SUBPIXEL_ORDER_DEFAULT, "CAIRO_SUBPIXEL_ORDER_DEFAULT", "default" },
          { CAIRO_SUBPIXEL_ORDER_RGB, "CAIRO_SUBPIXEL_ORDER_RGB", "rgb" },
          { CAIRO_SUBPIXEL_ORDER_BGR, "CAIRO_SUBPIXEL_ORDER_BGR", "bgr" },
          { CAIRO_SUBPIXEL_ORDER_VRGB, "CAIRO_SUBPIXEL_ORDER_VRGB", "vrgb" },
          { CAIRO_SUBPIXEL_ORDER_VBGR, "CAIRO_SUBPIXEL_ORDER_VBGR", "vbgr" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_subpixel_order_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_hint_style_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_HINT_STYLE_DEFAULT, "CAIRO_HINT_STYLE_DEFAULT", "default" },
          { CAIRO_HINT_STYLE_NONE, "CAIRO_HINT_STYLE_NONE", "none" },
          { CAIRO_HINT_STYLE_SLIGHT, "CAIRO_HINT_STYLE_SLIGHT", "slight" },
          { CAIRO_HINT_STYLE_MEDIUM, "CAIRO_HINT_STYLE_MEDIUM", "medium" },
          { CAIRO_HINT_STYLE_FULL, "CAIRO_HINT_STYLE_FULL", "full" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_hint_style_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_hint_metrics_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_HINT_METRICS_DEFAULT, "CAIRO_HINT_METRICS_DEFAULT", "default" },
          { CAIRO_HINT_METRICS_OFF, "CAIRO_HINT_METRICS_OFF", "off" },
          { CAIRO_HINT_METRICS_ON, "CAIRO_HINT_METRICS_ON", "on" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_hint_metrics_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_font_type_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_FONT_TYPE_TOY, "CAIRO_FONT_TYPE_TOY", "toy" },
          { CAIRO_FONT_TYPE_FT, "CAIRO_FONT_TYPE_FT", "ft" },
          { CAIRO_FONT_TYPE_WIN32, "CAIRO_FONT_TYPE_WIN32", "win32" },
          { CAIRO_FONT_TYPE_QUARTZ, "CAIRO_FONT_TYPE_QUARTZ", "quartz" },
          { CAIRO_FONT_TYPE_USER, "CAIRO_FONT_TYPE_USER", "user" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_font_type_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_path_data_type_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_PATH_MOVE_TO, "CAIRO_PATH_MOVE_TO", "move-to" },
          { CAIRO_PATH_LINE_TO, "CAIRO_PATH_LINE_TO", "line-to" },
          { CAIRO_PATH_CURVE_TO, "CAIRO_PATH_CURVE_TO", "curve-to" },
          { CAIRO_PATH_CLOSE_PATH, "CAIRO_PATH_CLOSE_PATH", "close-path" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_path_data_type_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_device_type_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_DEVICE_TYPE_DRM, "CAIRO_DEVICE_TYPE_DRM", "drm" },
          { CAIRO_DEVICE_TYPE_GL, "CAIRO_DEVICE_TYPE_GL", "gl" },
          { CAIRO_DEVICE_TYPE_SCRIPT, "CAIRO_DEVICE_TYPE_SCRIPT", "script" },
          { CAIRO_DEVICE_TYPE_XCB, "CAIRO_DEVICE_TYPE_XCB", "xcb" },
          { CAIRO_DEVICE_TYPE_XLIB, "CAIRO_DEVICE_TYPE_XLIB", "xlib" },
          { CAIRO_DEVICE_TYPE_XML, "CAIRO_DEVICE_TYPE_XML", "xml" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_device_type_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_surface_type_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_SURFACE_TYPE_IMAGE, "CAIRO_SURFACE_TYPE_IMAGE", "image" },
          { CAIRO_SURFACE_TYPE_PDF, "CAIRO_SURFACE_TYPE_PDF", "pdf" },
          { CAIRO_SURFACE_TYPE_PS, "CAIRO_SURFACE_TYPE_PS", "ps" },
          { CAIRO_SURFACE_TYPE_XLIB, "CAIRO_SURFACE_TYPE_XLIB", "xlib" },
          { CAIRO_SURFACE_TYPE_XCB, "CAIRO_SURFACE_TYPE_XCB", "xcb" },
          { CAIRO_SURFACE_TYPE_GLITZ, "CAIRO_SURFACE_TYPE_GLITZ", "glitz" },
          { CAIRO_SURFACE_TYPE_QUARTZ, "CAIRO_SURFACE_TYPE_QUARTZ", "quartz" },
          { CAIRO_SURFACE_TYPE_WIN32, "CAIRO_SURFACE_TYPE_WIN32", "win32" },
          { CAIRO_SURFACE_TYPE_BEOS, "CAIRO_SURFACE_TYPE_BEOS", "beos" },
          { CAIRO_SURFACE_TYPE_DIRECTFB, "CAIRO_SURFACE_TYPE_DIRECTFB", "directfb" },
          { CAIRO_SURFACE_TYPE_SVG, "CAIRO_SURFACE_TYPE_SVG", "svg" },
          { CAIRO_SURFACE_TYPE_OS2, "CAIRO_SURFACE_TYPE_OS2", "os2" },
          { CAIRO_SURFACE_TYPE_WIN32_PRINTING, "CAIRO_SURFACE_TYPE_WIN32_PRINTING", "win32-printing" },
          { CAIRO_SURFACE_TYPE_QUARTZ_IMAGE, "CAIRO_SURFACE_TYPE_QUARTZ_IMAGE", "quartz-image" },
          { CAIRO_SURFACE_TYPE_SCRIPT, "CAIRO_SURFACE_TYPE_SCRIPT", "script" },
          { CAIRO_SURFACE_TYPE_QT, "CAIRO_SURFACE_TYPE_QT", "qt" },
          { CAIRO_SURFACE_TYPE_RECORDING, "CAIRO_SURFACE_TYPE_RECORDING", "recording" },
          { CAIRO_SURFACE_TYPE_VG, "CAIRO_SURFACE_TYPE_VG", "vg" },
          { CAIRO_SURFACE_TYPE_GL, "CAIRO_SURFACE_TYPE_GL", "gl" },
          { CAIRO_SURFACE_TYPE_DRM, "CAIRO_SURFACE_TYPE_DRM", "drm" },
          { CAIRO_SURFACE_TYPE_TEE, "CAIRO_SURFACE_TYPE_TEE", "tee" },
          { CAIRO_SURFACE_TYPE_XML, "CAIRO_SURFACE_TYPE_XML", "xml" },
          { CAIRO_SURFACE_TYPE_SKIA, "CAIRO_SURFACE_TYPE_SKIA", "skia" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_surface_type_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_format_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_FORMAT_INVALID, "CAIRO_FORMAT_INVALID", "invalid" },
          { CAIRO_FORMAT_ARGB32, "CAIRO_FORMAT_ARGB32", "argb32" },
          { CAIRO_FORMAT_RGB24, "CAIRO_FORMAT_RGB24", "rgb24" },
          { CAIRO_FORMAT_A8, "CAIRO_FORMAT_A8", "a8" },
          { CAIRO_FORMAT_A1, "CAIRO_FORMAT_A1", "a1" },
          { CAIRO_FORMAT_RGB16_565, "CAIRO_FORMAT_RGB16_565", "rgb16-565" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_format_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_pattern_type_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_PATTERN_TYPE_SOLID, "CAIRO_PATTERN_TYPE_SOLID", "solid" },
          { CAIRO_PATTERN_TYPE_SURFACE, "CAIRO_PATTERN_TYPE_SURFACE", "surface" },
          { CAIRO_PATTERN_TYPE_LINEAR, "CAIRO_PATTERN_TYPE_LINEAR", "linear" },
          { CAIRO_PATTERN_TYPE_RADIAL, "CAIRO_PATTERN_TYPE_RADIAL", "radial" },
          { CAIRO_PATTERN_TYPE_MESH, "CAIRO_PATTERN_TYPE_MESH", "mesh" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_pattern_type_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_extend_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_EXTEND_NONE, "CAIRO_EXTEND_NONE", "none" },
          { CAIRO_EXTEND_REPEAT, "CAIRO_EXTEND_REPEAT", "repeat" },
          { CAIRO_EXTEND_REFLECT, "CAIRO_EXTEND_REFLECT", "reflect" },
          { CAIRO_EXTEND_PAD, "CAIRO_EXTEND_PAD", "pad" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_extend_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_filter_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_FILTER_FAST, "CAIRO_FILTER_FAST", "fast" },
          { CAIRO_FILTER_GOOD, "CAIRO_FILTER_GOOD", "good" },
          { CAIRO_FILTER_BEST, "CAIRO_FILTER_BEST", "best" },
          { CAIRO_FILTER_NEAREST, "CAIRO_FILTER_NEAREST", "nearest" },
          { CAIRO_FILTER_BILINEAR, "CAIRO_FILTER_BILINEAR", "bilinear" },
          { CAIRO_FILTER_GAUSSIAN, "CAIRO_FILTER_GAUSSIAN", "gaussian" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_filter_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}

GType
cairo_gobject_region_overlap_get_type (void)
{
   static gsize type_ret = 0;
   if (g_once_init_enter (&type_ret)) {
      static const GEnumValue values[] = {
          { CAIRO_REGION_OVERLAP_IN, "CAIRO_REGION_OVERLAP_IN", "in" },
          { CAIRO_REGION_OVERLAP_OUT, "CAIRO_REGION_OVERLAP_OUT", "out" },
          { CAIRO_REGION_OVERLAP_PART, "CAIRO_REGION_OVERLAP_PART", "part" },
          { 0, NULL, NULL }
      };
      GType type = g_enum_register_static (g_intern_static_string ("cairo_region_overlap_t"), values);

      g_once_init_leave (&type_ret, type);
   }
   return type_ret;
}


/* Generated data ends here */

