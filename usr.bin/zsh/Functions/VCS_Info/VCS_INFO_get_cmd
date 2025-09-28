## vim:ft=zsh
# find out if the user wants us to use a special binary.
# the default command name is the same as the backend name.
local cmd
zstyle -s ":vcs_info:${vcs}:${usercontext}:${rrn}" "command" cmd
vcs_comm[cmd]=${cmd:-$vcs}
