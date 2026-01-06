/**
 Copyright (c) 2015-present, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the BSD-style license found in the
 LICENSE file in the root directory of this source tree. An additional grant
 of patent rights can be found in the PATENTS file in the same directory.
 */

#include <car/car_format.h>

const char *const car_header_variable = "CARHEADER";
const char *const car_key_format_variable = "KEYFORMAT";
const char *const car_facet_keys_variable = "FACETKEYS";
const char *const car_renditions_variable = "RENDITIONS";

const char *const car_extended_metadata_variable = "EXTENDED_METADATA";
const char *const car_bitmap_keys_variable = "BITMAPKEYS";
const char *const car_globals_variable = "CARGLOBALS";
const char *const car_external_keys_variable = "EXTERNAL_KEYS";

const char *const car_part_info_variable = "PART_INFO";
const char *const car_element_info_variable = "ELEMENT_INFO";
const char *const car_colors_variable = "COLORS";
const char *const car_fonts_variable = "FONTS";
const char *const car_font_sizes_variable = "FONTSIZES";
const char *const car_glyphs_variable = "GLYPHS";
const char *const car_bezels_variable = "BEZELS";

const char *const car_attribute_identifier_names[_car_attribute_identifier_count] = {
    [car_attribute_identifier_element] = "element",
    [car_attribute_identifier_part] = "part",
    [car_attribute_identifier_size] = "size",
    [car_attribute_identifier_direction] = "direction",
    [car_attribute_identifier_value] = "value",
    [car_attribute_identifier_dimension1] = "dimension1",
    [car_attribute_identifier_dimension2] = "dimension2",
    [car_attribute_identifier_state] = "state",
    [car_attribute_identifier_layer] = "layer",
    [car_attribute_identifier_scale] = "scale",
    [car_attribute_identifier_presentation_state] = "presentation_state",
    [car_attribute_identifier_idiom] = "idiom",
    [car_attribute_identifier_subtype] = "subtype",
    [car_attribute_identifier_identifier] = "identifier",
    [car_attribute_identifier_previous_value] = "previous_value",
    [car_attribute_identifier_previous_state] = "previous_state",
    [car_attribute_identifier_size_class_horizontal] = "size_class_horizontal",
    [car_attribute_identifier_size_class_vertical] = "size_class_vertical",
    [car_attribute_identifier_memory_class] = "memory_class",
    [car_attribute_identifier_graphics_class] = "graphics_class",
    [car_attribute_identifier_display_gamut] = "display_gamut",
    [car_attribute_identifier_deployment_target] = "deployment_target",
};

