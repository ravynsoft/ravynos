#name: --gc-sections with __gxx_personality
#ld: --gc-sections -e main -L tmpdir -lpersonality
#nm: -n
#target: [check_as_cfi]

#failif
#...
.*gxx_personality.*
#...
