#name: --gc-sections with multiple debug sections for a function section
#as: -gdwarf-sections
#ld: --gc-sections -e _start
#nm: -B

#...
[0-9a-f]+ N +debug_aranges_main
[0-9a-f]+ N +debug_info_main
[0-9a-f]+ N +debug_line_main
#...
