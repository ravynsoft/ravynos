#
# .zprofile - Zed Shell startup script for login shells

EDITOR=vi;   	export EDITOR
PAGER=less;  	export PAGER

# set ENV to a file invoked each time sh is started for interactive use.
ENV=$HOME/.zshrc; export ENV

# Let zsh(1) know it's at home, despite /home being a symlink.
if [ "$PWD" != "$HOME" ] && [ "$PWD" -ef "$HOME" ] ; then cd ; fi

if [ ! -n "${TERM}" ]; then TERM="xterm-color"; fi
export TERM

# Query terminal size; useful for serial lines.
if [ -x /usr/bin/resizewin ] ; then /usr/bin/resizewin -z ; fi

PROMPT='%B%F{white}[%F{cyan}%m%F{white}F{cyan}%n%F{white} %b%~%B]%#%b%f '
export PROMPT
