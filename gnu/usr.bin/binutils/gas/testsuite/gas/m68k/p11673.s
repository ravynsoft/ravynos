# The following addressing mode is allowed on isab and higher
# with moveb and movew, but it is forbidden with movel.
        movel   #1,%a0@(2)
