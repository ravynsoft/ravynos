s/# Old OSF sed blows up if you have a sed command starting with "#"//
s/# Avoid it by putting the comments within real sed commands.//
s/# Fix .comm syntax to be correct for the PA assemblers.//
s/^[	 ]*\.comm \([^,]*\),\(.*\)/\1 .comm \2/
