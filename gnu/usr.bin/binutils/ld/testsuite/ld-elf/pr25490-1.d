#ld: --gc-sections -e _start
#target: [check_gc_sections_available]
#error: .*\(__patchable_function_entries\): error: need linked-to section for --gc-sections
