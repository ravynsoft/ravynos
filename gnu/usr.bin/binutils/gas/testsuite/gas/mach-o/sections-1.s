# This is the list of section switch data that GCC currently (4.7.0) emits
# for the Darwin/Mach-O target.
	.text
	.data
	.section __TEXT,__textcoal_nt,coalesced,pure_instructions
	.section __TEXT,__text_hot,regular,pure_instructions
	.section __TEXT,__text_cold,regular,pure_instructions
	.section __TEXT,__text_startup,regular,pure_instructions
	.section __TEXT,__text_exit,regular,pure_instructions
	.section __TEXT,__text_hot_coal,coalesced,pure_instructions
	.section __TEXT,__text_cold_coal,coalesced,pure_instructions
	.section __TEXT,__text_stt_coal,coalesced,pure_instructions
	.section __TEXT,__text_exit_coal,coalesced,pure_instructions
	.const
	.section __TEXT,__const_coal,coalesced
	.section	__DATA,__zobj_const
	.static_data
	.section __DATA,__datacoal_nt,coalesced
	.section	__DATA,__zobj_data
	.section	__DATA,__zobj_bss
	.const_data
	.section __DATA,__const_coal,coalesced
	.section	__DATA,__zobj_cnst_data
	.cstring
	.literal4
	.literal8
	.literal16
	.section __DATA, __cfstring
	.mod_init_func
	.mod_term_func
	.constructor
	.destructor
	.dyld
# CFstring
	.section	__DATA,__cfstring
# OBJC
	.objc_class 
	.objc_meta_class 
	.objc_cat_cls_meth 
	.objc_cat_inst_meth 
	.objc_protocol 
	.objc_string_object 
	.objc_cls_meth 
	.objc_inst_meth 
	.objc_cls_refs 
	.objc_message_refs 
	.objc_symbols 
	.objc_category 
	.objc_class_vars 
	.objc_instance_vars 
	.objc_module_info 
	.objc_class_names 
	.objc_meth_var_types 
	.objc_meth_var_names 
	.objc_selector_strs
	.section __OBJC, __sel_fixup, regular, no_dead_strip
	.section __OBJC, __image_info, regular, no_dead_strip
# OBJC 1
	.section __OBJC, __class_ext, regular, no_dead_strip
	.section __OBJC, __property, regular, no_dead_strip
	.section __OBJC, __protocol_ext, regular, no_dead_strip
# OBJC 2
	.section __DATA, __objc_classrefs, regular, no_dead_strip
	.section __DATA, __objc_classlist, regular, no_dead_strip
	.section __DATA, __objc_catlist, regular, no_dead_strip
	.section __DATA, __objc_selrefs, literal_pointers, no_dead_strip
	.section __DATA, __objc_nlclslist, regular, no_dead_strip
	.section __DATA, __objc_nlcatlist, regular, no_dead_strip
	.section __DATA, __objc_protolist, regular, no_dead_strip
	.section __DATA, __objc_protorefs, regular, no_dead_strip
	.section __DATA, __objc_superrefs, regular, no_dead_strip
	.section __DATA, __objc_imageinfo, regular, no_dead_strip
	.section __DATA, __objc_stringobj, regular, no_dead_strip
# DWARF debug
	.section __DWARF,__debug_frame,regular,debug
	.section __DWARF,__debug_info,regular,debug
	.section __DWARF,__debug_abbrev,regular,debug
	.section __DWARF,__debug_aranges,regular,debug
	.section __DWARF,__debug_macinfo,regular,debug
	.section __DWARF,__debug_line,regular,debug
	.section __DWARF,__debug_loc,regular,debug
	.section __DWARF,__debug_pubnames,regular,debug
	.section __DWARF,__debug_pubtypes,regular,debug
	.section __DWARF,__debug_str,regular,debug
	.section __DWARF,__debug_ranges,regular,debug
	.section __DWARF,__debug_macro,regular,debug
# end of base sections for GCC support
