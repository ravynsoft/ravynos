#name: missing entry symbol
#ld: -pie
#warning: .*: warning: cannot find entry symbol .*
#nm: -n

#...
[0-9a-f]+ T +foo
#...
