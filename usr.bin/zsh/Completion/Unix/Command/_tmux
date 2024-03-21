#compdef tmux

# tmux <http://tmux.github.io> completion for zsh <http://zsh.sf.net>.
#
# Configuration:
#
# - On some OSs, the directory for tmux's server sockets may not be
#   the default (which is /tmp/tmux-<uid>/), but say
#   /var/run/tmux/tmux-<uid>, in which case the completion for
#   'tmux -L <tab>' will not be able to find the sockets in the default
#   location (debian does this, for instance); tell _tmux the right place
#   to look:
#       % zstyle ':completion:*:*:tmux:*:sockets' socketdir "/var/run/tmux/tmux-${UID}"
#
# - tmux knows a *lot* of sub-commands, hence 'tmux <tab>' returns a lot
#   of possible completions. _tmux knows about all commands and their aliases.
#   By default, both are offered. If you do not care about the aliases, do this:
#       % zstyle ':completion:*:*:tmux:*:subcommands' mode 'commands'
#
#   The same can be done to only return aliases by setting the above style
#   to 'aliases'. The default value is 'both' (but actually every value
#   different from 'commands' and 'aliases' will have the same effect).
#
#   'lsw' is an alias for 'list-windows' for example; note that not all
#   commands have aliases. So probably, either the default 'both' or
#   'commands' makes most sense for this style.
#
# - For finer grained control over what is suggested as possible completions,
#   the 'ignored-patterns' style may be used; suppose you think that only
#   '*-window' or '*-windows' are worth being completed. You would get that
#   behaviour like this:
#       % zstyle ':completion:*:*:tmux:*:subcommands' ignored-patterns '^*-window(|s)'
#
#   Some tmux commands currently do not work if called from a shell prompt,
#   so it would make sense to ignore them per default (at the time of writing,
#   those commands are choose-{session,client,window}, confirm-before and
#   find-window. This would ignore them:
#       % zstyle ':completion:*:*:tmux:*:subcommands' ignored-patterns \
#                'choose-*' 'confirm-before' 'find-window'
#
# The configuration for subcommand completions may be done in
# this context: ':completion:*:*:tmux-<sub-command>:*:*'
#
# TODO:
#
#   - Implement __tmux-style (possibly using existing helpers like
#     __tmux-attributes and __tmux-colours)
#   - in _tmux-list-panes, use __tmux-windows or __tmux-sessions
#     depending on -s is among the sub-commands current command line.

# Global variables; setup the first time _tmux is called.
# For $_tmux_commands[] generation, see the very end of this file.
typeset -ga _tmux_commands=() _tmux_aliases=()
typeset -gA _tmux_aliasmap

_tmux_aliasmap=(
    # clients and sessions
    attach      attach-session
    detach      detach-client
    has         has-session
    lockc       lock-client
    locks       lock-session
    lsc         list-clients
    lscm        list-commands
    ls          list-sessions
    new         new-session
    refresh     refresh-client
    rename      rename-session
    showmsgs    show-messages
    source      source-file
    start       start-server
    suspendc    suspend-client
    switchc     switch-client

    # windows and panes
    breakp      break-pane
    capturep    capture-pane
    displayp    display-panes
    findw       find-window
    joinp       join-pane
    killp       kill-pane
    killw       kill-window
    last        last-window
    lastp       last-pane
    linkw       link-window
    lsp         list-panes
    lsw         list-windows
    movep       move-pane
    movew       move-window
    neww        new-window
    nextl       next-layout
    next        next-window
    pipep       pipe-pane
    prev        previous-window
    prevl       previous-layout
    renamew     rename-window
    resizep     resize-pane
    resizew     resize-window
    respawnp    respawn-pane
    respawnw    respawn-window
    rotatew     rotate-window
    selectl     select-layout
    selectp     select-pane
    selectw     select-window
    splitw      split-window
    swapp       swap-pane
    swapw       swap-window
    unlinkw     unlink-window

    # key bindings
    bind        bind-key
    lsk         list-keys
    send        send-keys
    unbind      unbind-key

    # options
    set         set-option
    setw        set-window-option
    show        show-options
    showw       show-window-options

    # environment
    setenv      set-environment
    showenv     show-environment

    # status line
    confirm     confirm-before
    menu        display-menu
    display     display-message
    popup       display-popup

    # buffers
    clearhist   clear-history
    deleteb     delete-buffer
    lsb         list-buffers
    loadb       load-buffer
    pasteb      paste-buffer
    saveb       save-buffer
    setb        set-buffer
    showb       show-buffer

    # miscellaneous
    if          if-shell
    lock        lock-server
    run         run-shell
    info        server-info
    wait        wait-for
)

# --- Sub-command functions ---
# These *must* be called _tmux-*(); The description generation relies on
# them being named that way. *No* other functions may match that pattern.
# Other utility functions should be named __tmux-*() (see below).
#
# Another thing, the description generation needs, is handling of
# $tmux_describe: If that parameter is non-empty, the sub-command function
# should only print a description of the sub-command it handles and return
# immediately after doing so.
#
# To add support for a new sub-command, you only have to add a new
# _tmux-<foo>() function below (preferably alphabetically sorted), that
# behaves like described above; and add a alias->command pair in the
# _tmux_aliasmap associative array above (if the command in fact has an
# alias). The rest should just work[tm].

_tmux-attach-session() {
  [[ -n ${tmux_describe} ]] && print "attach or switch to a session" && return

  _arguments -s \
    '-c+[specify working directory for the session]:directory:_directories' \
    '-d[detach other clients attached to target session]' \
    '-f+[set client flags]: :_tmux_client_flags' \
    '-r[put the client into read-only mode]' \
    '-t+[specify target session]:target session: __tmux-sessions-separately' \
    "-E[don't apply update-environment option]" \
    '-x[with -d, send SIGHUP to the parent of the attached client]'
}

_tmux-bind-key() {
  [[ -n ${tmux_describe} ]] && print "bind a key to a command" && return
  _arguments -s -A "-*" \
    '-n[make the binding work without the need for the prefix key]' \
    '-r[the key may repeat]' \
    '-N+[attach a note to the key]:note' \
    '-T+[specify key table for the binding]:key table' \
    '1:key' \
    '*:::template:= _tmux'
}

_tmux-break-pane() {
  [[ -n ${tmux_describe} ]] && print "break a pane from an existing into a new window" && return
  _arguments -s \
    '(-b)-a[move window to next index after]' \
    '(-a)-b[move window to next index before]' \
    "-d[don't make the new window become the active one]" \
    '-F+[specify output format]:format:__tmux-formats' \
    '-P[print information of new window after it has been created]' \
    '-n+[specify window name]:name' \
    '-s+[specify source pane]:pane:__tmux-panes' \
    '-t+[specify destination window]:pane:__tmux-panes'
}

_tmux-capture-pane() {
  [[ -n ${tmux_describe} ]] && print "capture the contents of a pane to a buffer" && return
  _arguments -s \
    '-a[use alternate screen]' \
    '(-p)-b+[choose target buffer]:target buffer:__tmux-buffers' \
    '-C[escape non-printable characters as octal \\ooo]' \
    '-e[include escape sequences for attributes etc]' \
    '-E[specify last line to capture]:line number (- means last line)' \
    '(-N)-J[join wrapped lines and preserve trailing space]' \
    '(-J)-N[preserve trailing space]' \
    '-q[ignore errors when trying to access alternate screen]' \
    '(-b)-p[print data to stdout]' \
    '-P[only capture beginnings of as-yet incomplete escape sequences]' \
    '-S[specify start line to capture]:first line (- means start of scrollback)' \
    '-t+[choose source pane]:source pane:__tmux-panes'
}

_tmux-choose-buffer() {
  [[ -n ${tmux_describe} ]] && print "put a pane into buffer choice mode" && return
  _arguments -s \
    '-N[start without the preview]' \
    '-Z[zoom the pane]' \
    '-r[reverse sort order]' \
    '-F+[specify format for each list item]:format:__tmux-formats' \
    '-f+[filter items]:filter format:__tmux-formats' \
    '-K+[specify format for each shortcut key]:format:__tmux-formats' \
    '-O+[initial sort order]:order:(time name size)' \
    '-t+[specify target window]:session:__tmux-windows' \
    '*:::template:= _tmux'
}

_tmux-choose-client() {
  [[ -n ${tmux_describe} ]] && print "put a window into client choice mode" && return
  _arguments -s \
    '-N[start without the preview]' \
    '-Z[zoom the pane]' \
    '-r[reverse sort order]' \
    '-F+[specify format for each list item]:format:__tmux-formats' \
    '-f+[filter items]:filter format:__tmux-formats' \
    '-K+[specify format for each shortcut key]:format:__tmux-formats' \
    '-O+[initial sort order]:order:(time name size)' \
    '-t+[specify target window]:session:__tmux-windows' \
    '*:::template:= _tmux'
}

_tmux-choose-tree() {
  [[ -n ${tmux_describe} ]] && print "put a window into tree choice mode" && return
  _arguments -s \
    '-G[include all sessions in any session groups in the tree rather than only the first]' \
    '-N[start without the preview]' \
    '-Z[zoom the pane]' \
    '-r[reverse sort order]' \
    '-F+[specify format for each list item]:format:__tmux-formats' \
    '-f+[filter items]:filter format:__tmux-formats' \
    '-K+[specify format for each shortcut key]:format:__tmux-formats' \
    '-O+[initial sort order]:order:(time name size)' \
    '-s[choose among sessions]' \
    '-t+[specify target window]:session:__tmux-windows' \
    '-w[choose among windows]' \
    '*:::template:= _tmux'
}

_tmux-clear-history() {
  [[ -n ${tmux_describe} ]] && print "remove and clear history for a pane" && return
  _arguments '-t+[specify target pane]:pane:__tmux-panes'
}

_tmux-clock-mode() {
  [[ -n ${tmux_describe} ]] && print "enter clock mode" && return
  _arguments '-t+[specify target pane]:pane:__tmux-panes'
}

_tmux-command-prompt() {
  [[ -n ${tmux_describe} ]] && print "open the tmux command prompt in a client" && return
  _arguments -s \
    '-1[only accept one key press]' \
    '-k[only accept one key press and translate it to a key name]' \
    '-N[accept only numeric key presses]' \
    '-i[execute the command every time the prompt input changes]' \
    '-I+[specify list of initial inputs]:initial-text (comma-separated list)' \
    '-p+[specify list of prompts]:prompts (comma-separated list)' \
    '-t+[specify target client]:client:__tmux-clients' \
    '(-W)-T[prompt is for a target - tab complete as appropriate]' \
    '(-T)-W[prompt is for a window - tab complete as appropriate]' \
    '*:::template:= _tmux'
}

_tmux-confirm-before() {
  [[ -n ${tmux_describe} ]] && print "run a command but ask for confirmation before" && return
  _arguments -s \
    '-p+[specify prompt]:prompt string' \
    '-t+[specify target client]:client:__tmux-clients' \
    '*:::command:= _tmux'
}

_tmux-copy-mode() {
  [[ -n ${tmux_describe} ]] && print "enter copy mode" && return
  _arguments -s \
    '-s+[specify source pane]:pane:__tmux-panes' \
    '-t+[specify target pane]:pane:__tmux-panes' \
    '-e[scrolling to the bottom should exit copy mode]' \
    '-H[hide the position indicator in the top right]' \
    '-q[cancel copy mode and any other modes]' \
    '-u[scroll up one page]' \
    '-M[begin a mouse drag]'
}

_tmux-customize-mode() {
  [[ -n ${tmux_describe} ]] && print "enter customize mode" && return
  _arguments -s \
    '-F+[specify format for each item in the tree]:format:__tmux-formats' \
    '-f+[specify initial filter]:filter:__tmux-formats' \
    '-N[start without the option information]' \
    '-t+[specify target pane]:pane:__tmux-panes' \
    '-Z[zoom the pane]'
}

_tmux-delete-buffer() {
  [[ -n ${tmux_describe} ]] && print "delete a paste buffer" && return
  _arguments '-b+[specify target buffer name]:buffer:__tmux-buffers'
}

_tmux-detach-client() {
  [[ -n ${tmux_describe} ]] && print "detach a client from the server" && return
  _arguments -s \
    '-a[kill all clients except for the named by -t]' \
    '-P[send SIGHUP to parent process]' \
    '-E+[run specified shell command to replace the client]:shell command:_cmdstring' \
    '-s+[specify target session and kill its clients]:session:__tmux-sessions-attached' \
    '-t+[specify target client]:client:__tmux-clients'
}

_tmux-display-menu() {
  [[ -n ${tmux_describe} ]] && print "display a menu" && return
  local curcontext="$curcontext" ret=1
  local -a state line expl
  _arguments -C -s -S -A "-*" \
    '-c+[specify target client]:client:__tmux-clients' \
    "-O[don't close menu if mouse is released without making a selection]" \
    '-t+[specify target pane]:pane:__tmux-panes' \
    '-T+[specify title]:title' \
    '-x+[specify horizontal position]: : _alternative "columns\: \:_guard \[0-9\]\# column" "positions\:position\:((R\:right P\:bottom M\:mouse W\:window))"' \
    '-y+[specify vertical position]: : _alternative "rows\: \:_guard \[0-9\]\# row" "positions\:position\:((P\:left M\:mouse S\:status\ line))"' \
    '*::: :->options' && ret=0

  if [[ -n $state ]]; then
    case $(( CURRENT % 3 )) in
      1) _message -e menu-options 'menu option' ;;
      2) _message -e keys 'shortcut key' ;;
      0)
	compset -q
	words=( menu "$words[@]" )
	(( CURRENT++ ))
	_tmux && ret=0
      ;;
    esac
  fi

  return ret
}

_tmux-display-message() {
  [[ -n ${tmux_describe} ]] && print "display a message in the status line" && return
  _arguments -s -S -A "-*" \
    '(-p -F :)-a[list the format variables and their values]' \
    '-I[forward any input read from stdin to the target pane]' \
    '-N[ignore key presses and only close after the delay]' \
    '-c+[specify target client]:client:__tmux-clients' \
    '-d+[time to display message]:delay (milliseconds)' \
    '(-a)-p[print message to stdout]' \
    '-t+[specify target pane]:pane:__tmux-panes' \
    '(-a)-F+[specify output format]:format:__tmux-formats' \
    '(-a)-v[print verbose logging as the format is parsed]' \
    ':message:__tmux-formats'
}

_tmux-display-panes() {
  [[ -n ${tmux_describe} ]] && print "display an indicator for each visible pane" && return
  _arguments -S \
    "-b[don't block other commands until indicator is closed]" \
    '-d+[time to show indicator for]:duration (ms)' \
    '-t+[specify target client]:client:__tmux-clients' \
    '*:::command:= _tmux'
}

_tmux-display-popup() {
  [[ -n ${tmux_describe} ]] && print "display a popup box over a pane" && return
  _arguments -s \
    '-C[close any popup on the client]' \
    '-c+[specify target client]:client:__tmux-clients' \
    '-d+[specify working directory for the command]:directory:_directories' \
    '-E[close the popup when the command exits]' \
    '-w+[specify width]:width' \
    '-h+[specify height]:height' \
    '-t+[specify target pane]:pane:__tmux-panes' \
    '-x+[specify horizontal position]:position' \
    '-y+[specify vertical position]:position' \
    ':shell command:_cmdstring'
}

_tmux-find-window() {
  [[ -n ${tmux_describe} ]] && print "search for a pattern in windows" && return
  _arguments -s \
    '-i[ignore case]' \
    '-r[use regular expression matching]' \
    '-C[match visible contents]' \
    '-N[match window name]' \
    '-T[match window title]' \
    '-t+[specify target window]:window:__tmux-windows' \
    '-Z[zoom the pane]' \
    ':window search pattern'
}

_tmux-has-session() {
  [[ -n ${tmux_describe} ]] && print "check and report if a session exists on the server" && return
  _arguments '-t+[specify target session]:session:__tmux-sessions'
}

_tmux-if-shell() {
  [[ -n ${tmux_describe} ]] && print "execute a tmux command if a shell-command succeeded" && return
  local curcontext="$curcontext" state line ret=1
  _arguments -C -s \
    '-b[run shell command in background]' \
    "-F[don't execute shell command but use it as a string-value]" \
    '-t+[specify target pane]:pane:__tmux-panes' \
    '1:shell command:_cmdstring' \
    '2:tmux command (success):->tmuxcmd' \
    '3:tmux command (failure):->tmuxcmd' && ret=0
  if [[ -n $state ]]; then
    compset -q
    _tmux && ret=0
  fi
}

_tmux-join-pane() {
  [[ -n ${tmux_describe} ]] && print "split a pane and move an existing one into the new space" && return
  # -p removed from documentation but still works, or use -l and append %
  _arguments -s \
    '-b[join source pane left of or above target pane]' \
    "-d[don't make the new window become the active one]" \
    '(-l)-f[span the full size]' \
    '-h[split horizontally]' \
    '-v[split vertically]' \
    "(-p)-l[define new pane's size]:size" \
    "!(-f -l)-p+:size (percentage)" \
    '-s+[specify source pane]:pane:__tmux-panes' \
    '-t+[specify target pane]:pane:__tmux-panes'
}

_tmux-kill-pane() {
  [[ -n ${tmux_describe} ]] && print "destroy a given pane" && return
  _arguments -s \
    '-a[kill all panes except the one specified by -t]' \
    '-t+[specify target pane]:pane:__tmux-panes'
}

_tmux-kill-server() {
  [[ -n ${tmux_describe} ]] && print "kill clients, sessions and server" && return
  __tmux-nothing-else
}

_tmux-kill-session() {
  [[ -n ${tmux_describe} ]] && print "destroy a given session" && return
  _arguments -s \
    '-a[kill all session except the one specified by -t]' \
    '-t+[specify target session]:session:__tmux-sessions' \
    '-C[clear alerts (bell, activity, silence) in all windows linked to the session]'
}

_tmux-kill-window() {
  [[ -n ${tmux_describe} ]] && print "destroy a given window" && return
  _arguments -s \
    '-a[kill all windows except the one specified by -t]' \
    '-t+[specify target window]:window:__tmux-windows'
}

_tmux-last-pane() {
  [[ -n ${tmux_describe} ]] && print "select the previously selected pane" && return
  _arguments -s \
    '-d[disable input to the pane]' \
    '-e[enable input to the pane]' \
    '-Z[keep window zoomed if it was zoomed]' \
    '-t+[specify session]:session:__tmux-sessions'
}

_tmux-last-window() {
  [[ -n ${tmux_describe} ]] && print "select the previously selected window" && return
  _arguments '-t+[specify session]:session:__tmux-sessions'
}

_tmux-link-window() {
  [[ -n ${tmux_describe} ]] && print "link a window to another" && return
  _arguments -s \
    '(-b)-a[move window to next index after destination window]' \
    '(-a)-b[move window to next index before destination window]' \
    "-d[don't make the new window become the active one]" \
    '-k[kill the target window if it exists]' \
    '-s+[specify source window]:window:__tmux-windows' \
    '-t+[specify destination window]:window:__tmux-windows'
}

_tmux-list-buffers() {
  [[ -n ${tmux_describe} ]] && print "list paste buffers of a session" && return
  _arguments \
    '-F+[specify output format]:format:__tmux-formats' \
    '-f+[filter items]:filter format:__tmux-formats'
}

_tmux-list-clients() {
  [[ -n ${tmux_describe} ]] && print "list clients attached to server" && return
  _arguments -s \
    '-F+[specify output format]:format:__tmux-formats' \
    '-t+[specify session]:session:__tmux-sessions'
}

_tmux-list-commands() {
  [[ -n ${tmux_describe} ]] && print "list supported sub-commands" && return
  _arguments '-F+[specify format]:format:__tmux-formats' '1:sub-command:_tmux'
}

_tmux-list-keys() {
  [[ -n ${tmux_describe} ]] && print "list all key-bindings" && return
  _arguments -s \
    '-1[list only the first matching key]' \
    '-a[list the command for keys that do have a note]' \
    '-N[list only keys with attached notes]' \
    '-P+[specify a prefix to print before each key]:prefix' \
    '-T+[specify key table]:key table'
}

_tmux-list-panes() {
  [[ -n ${tmux_describe} ]] && print "list panes of a window" && return
  local -a args
  if (( ${+words[(r)-*s*]} )); then
    args=( '-t+[specify target session]:session:__tmux-sessions' )
  else
    args=( '-t+[specify target window]:window:__tmux-windows' )
  fi
  _arguments -s $args \
    '-a[list all panes the server possesses]' \
    '-F+[specify output format]:format:__tmux-formats' \
    '-f+[filter items]:filter format:__tmux-formats' \
    '-s[if specified, -t chooses a session]'
}

_tmux-list-sessions() {
  [[ -n ${tmux_describe} ]] && print "list sessions managed by server" && return
  _arguments \
    '-F+[specify output format]:format:__tmux-formats' \
    '-f+[filter items]:filter format:__tmux-formats'
}

_tmux-list-windows() {
  [[ -n ${tmux_describe} ]] && print "list windows of a session" && return
  _arguments -s \
    '-a[list all windows the tmux server possesses]' \
    '-F[specify output format]:format:__tmux-formats' \
    '-f+[filter items]:filter format:__tmux-formats' \
    '-t+[specify session]:session:__tmux-sessions'
}

_tmux-load-buffer() {
  [[ -n ${tmux_describe} ]] && print "load a file into a paste buffer" && return
  _arguments -A "-*" -S \
    '-b+[specify target buffer name]:buffer:__tmux-buffers' \
    '-t+[specify target client]:client:__tmux-clients' \
    '-w[also send the buffer to the clipboard using the xterm escape sequence]' \
    '1:file:_files'
}

_tmux-lock-client() {
  [[ -n ${tmux_describe} ]] && print "lock a client" && return
  _arguments '-t+[specify client]:client:__tmux-clients'
}

_tmux-lock-server() {
  [[ -n ${tmux_describe} ]] && print "lock all clients attached to the server" && return
  __tmux-nothing-else
}

_tmux-lock-session() {
  [[ -n ${tmux_describe} ]] && print "lock all clients attached to a session" && return
  _arguments '-t+[specify session]:session:__tmux-sessions'
}

_tmux-move-pane() {
 _tmux-join-pane "$@"
}

_tmux-move-window() {
  [[ -n ${tmux_describe} ]] && print "move a window to another" && return
  _arguments -s \
    '(-b)-a[move window to next index after destination window]' \
    '(-a)-b[move window to next index before destination window]' \
    "-d[don't make the new window become the active one]" \
    '-k[kill the target window if it exists]' \
    '-s+[specify source window]:window:__tmux-windows' \
    '-r[renumber windows in session in sequential order]' \
    '-t+[specify destination window]:window:__tmux-windows'
}

_tmux-new-session() {
  [[ -n ${tmux_describe} ]] && print "create a new session" && return
  _arguments -s -A "-*" -S \
    '-c+[specify working directory for the session]:directory:_directories' \
    '-A[attach to existing session if it already exists]' \
    "-d[don't attach new session to current terminal]" \
    "-D[with -A, detach other clients attached to session]" \
    "-E[don't apply update-environment option]" \
    '*-e[specify environment variable]:environment variable:_parameters -g "*export*" -qS=' \
    '-F+[specify output format]:format:__tmux-formats' \
    '-f+[specify client flags]: :_tmux_client_flags' \
    '-n+[specify initial window name]:window name' \
    '-P[print information about new session after it is created]' \
    '-s+[name the session]:session name:__tmux-sessions' \
    '-t+[specify target session]:session:__tmux-sessions' \
    '-x[specify width]:width' \
    '-y[specify height]:height' \
    '-X[with -D, send SIGHUP to the parent of the attached client]' \
    '*:: :_cmdambivalent'
}

_tmux-new-window() {
  [[ -n ${tmux_describe} ]] && print "create a new window" && return
  _arguments -s -A "-*" -S \
    '(-b)-a[insert new window at next index after target]' \
    '(-a)-b[insert new window at next index before target]' \
    '-c+[specify working directory for the session]:directory:_directories' \
    '*-e[specify environment variable]:environment variable:_parameters -g "*export*" -qS=' \
    "(-S)-d[don't make the new window become the active one]" \
    '-F+[specify output format]:format:__tmux-formats' \
    '-k[destroy it if the specified window exists]' \
    '-n+[specify a window name]:window name' \
    '-P[print information about new window after it is created]' \
    '(-d)-S[select window if name already exists]' \
    '-t+[specify target window]:window:__tmux-windows' \
    '*:: :_cmdambivalent'
}

_tmux-next-layout() {
  [[ -n ${tmux_describe} ]] && print "move a window to the next layout" && return
  _arguments '-t+[specify target window]:window:__tmux-windows'
}

_tmux-next-window() {
  [[ -n ${tmux_describe} ]] && print "move to the next window in a session" && return
  _arguments -s \
    '-a[move to the next window with an alert]' \
    '-t+[specify target session]:session:__tmux-sessions'
}

_tmux-paste-buffer() {
  [[ -n ${tmux_describe} ]] && print "insert a paste buffer into the window" && return
  _arguments -s \
    '-b+[specify buffer]:source buffer:__tmux-buffers' \
    '-d[remove buffer from stack after pasting]' \
    '-p[use bracketed paste mode if the application requested it]' \
    "-r[don't replace LF with CR when pasting]" \
    '-s+[specify separator]:separator' \
    '-t+[specify target window]:window:__tmux-windows'
}

_tmux-pipe-pane() {
  [[ -n ${tmux_describe} ]] && print "pipe output from a pane to a shell command" && return
  _arguments -s -A "-*" -S \
    '-I[write stdout from command to the pane as if it were typed]' \
    '-O[pipe output from the pane to the command (default unless -I used)]' \
    '-o[only open a pipe if none is currently opened]' \
    '-t+[specify target pane]:pane:__tmux-panes' \
    ':shell command:_cmdstring'
}

_tmux-previous-layout() {
  [[ -n ${tmux_describe} ]] && print "move a window to the previous layout" && return
  _arguments '-t+[specify target window]:window:__tmux-windows'
}

_tmux-previous-window() {
  [[ -n ${tmux_describe} ]] && print "move to the previous window in a session" && return
  _arguments -s \
    '-a[move to the previous window with an alert]' \
    '-t+[specify target session]:session:__tmux-sessions'
}

_tmux-refresh-client() {
  [[ -n ${tmux_describe} ]] && print "refresh a client" && return
  _arguments -s -A "-*" -S \
    '-B+[set a subscription to a format for a control mode client]:subscription' \
    '-A+[allow a control mode client to trigger actions on a pane]:pane:__tmux-panes -P% -S\:' \
    '-C+[set the width and height of a control client]:width,height' \
    '-c[reset so that the position follows the cursor]' \
    '-D[move visible portion of window down]' \
    '-f+[set client flags]:flag:_tmux_client_flags' \
    '-L[move visible portion of window left]' \
    '-l[request clipboard from the client and store it in a new paste buf using xterm(1) escape sequence]' \
    "-S[only update the client's status bar]" \
    '-t+[specify target client]:client:__tmux-clients' \
    '-R[move visible portion of window right]' \
    '-U[move visible portion of window up]' \
    ': :_guard "[0-9]#" "adjustment"'
}

_tmux-rename-session() {
  [[ -n ${tmux_describe} ]] && print "rename a session" && return
  _arguments -s -A "-*" -S \
    '-t+[specify target session]:session:__tmux-sessions' \
    ':new session name'
}

_tmux-rename-window() {
  [[ -n ${tmux_describe} ]] && print "rename a window" && return
  _arguments -s -A "-*" -S \
    '-t+[specify target window]:window:__tmux-windows' \
    ':new window name'
}

_tmux-resize-pane() {
  [[ -n ${tmux_describe} ]] && print "resize a pane" && return
  _arguments -s -A "-*" -S \
    '-D[resize downward]' \
    '-L[resize to the left]' \
    '-M[begin mouse resizing]' \
    '-R[resize to the right]' \
    '-U[resize upward]' \
    '-t+[specify target pane]:pane:__tmux-panes' \
    '-T[trim lines below the cursor position and moves lines out of the history to replace them]' \
    '-x+[specify width]:width' \
    '-y+[specify height]:height' \
    '-Z[toggle zoom of pane]' \
    ': :_guard "[0-9]#" "adjustment"'
}

_tmux-resize-window() {
  [[ -n ${tmux_describe} ]] && print "resize a window" && return
  _arguments -s -A "-*" -S : \
    '-A[set size of largest session containing the window]' \
    '-a[set size of smallest session containing the window]' \
    '-D[resize downward]' \
    '-L[resize to the left]' \
    '-R[resize to the right]' \
    '-U[resize upward]' \
    '-t+[specify target pane]:pane:__tmux-windows' \
    '-x+[specify width]:width' \
    '-y+[specify height]:height' \
    ': :_guard "[0-9]#" "adjustment"'
}

_tmux-respawn-pane() {
  [[ -n ${tmux_describe} ]] && print "reuse a pane in which a command has exited" && return
  _arguments -s -A "-*" -S \
    '-c+[specify a new working directory for the pane]:directory:_directories' \
    '*-e[specify environment variable]:environment variable:_parameters -g "*export*" -qS=' \
    '-k[kill window if it is in use]' \
    '-t+[specify target pane]:pane:__tmux-pane' \
    ':command:_cmdambivalent'
}

_tmux-respawn-window() {
  [[ -n ${tmux_describe} ]] && print "reuse a window in which a command has exited" && return
  _arguments -s -A "-*" -S \
    '-c+[specify a new working directory for the window]:directory:_directories' \
    '*-e[specify environment variable]:environment variable:_parameters -g "*export*" -qS=' \
    '-k[kill window if it is in use]' \
    '-t+[specify target window]:window:__tmux-windows' \
    ':command:_cmdambivalent'
}

_tmux-rotate-window() {
  [[ -n ${tmux_describe} ]] && print "rotate positions of panes in a window" && return
  _arguments -s \
    '-D[rotate downward]' \
    '-U[rotate upward]' \
    '-Z[keep the window zoomed if it was zoomed]' \
    '-t+[specify target window]:window:__tmux-windows'
}

_tmux-run-shell() {
  [[ -n ${tmux_describe} ]] && print "execute a command without creating a new window" && return
  local curcontext="$curcontext" ret=1
  local -a state line expl
  _arguments -C -s -A "-*" -S \
    '-b[run command in background]' \
    '(1)-C[run a tmux command]' \
    '-d+[specify delay before starting the command]:delay (seconds)' \
    '-t+[specify target pane]:pane:__tmux-panes' \
    '(2)1:command:_cmdstring' \
    '2:tmux command:->tmux-commands' && ret=0

  if [[ -n $state ]]; then
    compset -q
    words=( run "$words[@]" )
    (( CURRENT++ ))
    _tmux && ret=0
  fi
}

_tmux-save-buffer() {
  [[ -n ${tmux_describe} ]] && print "save a paste buffer to a file" && return
  _arguments -s \
    '-a[append to rather than overwriting file]' \
    '-b+[specify a target buffer index]:buffer:__tmux-buffers'
}

_tmux-select-layout() {
  [[ -n ${tmux_describe} ]] && print "choose a layout for a pane" && return
  _arguments -s -A "-*" -S \
    '-E[spread the current pane and any panes next to it out evenly]' \
    '-n[behave like next-layout]' \
    '-o[revert to previous layout]' \
    '-p[behave like previous-layout]' \
    '-t+[specify a target pane]:target pane:__tmux-panes' \
    ':layout:(even-horizontal even-vertical main-horizontal main-vertical tiled)'
}

_tmux-select-pane() {
  [[ -n ${tmux_describe} ]] && print "make a pane the active one in the window" && return
  # -P and -g have been removed from the documentation in tmux 3 but still work
  _arguments -s \
    '-D[move to the pane below target]' \
    '-d[disable input to the pane]' \
    '-e[enable input to the pane]' \
    '-g[show current pane style]' \
    '-l[behave like last-pane]' \
    '-L[move to the pane left of target]' \
    '-M[clear marked pane]' \
    '-m[set marked pane]' \
    '-R[move to the pane right of target]' \
    '-U[move to the pane above target]' \
    '-Z[keep the window zoomed if it was zoomed]' \
    '-P+[set pane style]:style:__tmux-style' \
    '-T+[set the pane title]:title' \
    '-t+[specify target pane]:pane:__tmux-panes'
}

_tmux-select-window() {
  [[ -n ${tmux_describe} ]] && print "select a window" && return
  _arguments -s \
    '-l[behave like last-window]' \
    '-n[behave like next-window]' \
    '-p[behave like previous-window]' \
    '-T[if selected window is the current behave like last-window]' \
    '-t+[specify target window]:window:__tmux-windows'
}

_tmux-send-keys() {
  [[ -n ${tmux_describe} ]] && print "send key(s) to a window" && return
  _arguments -s -A "-*" -S \
    '(-H)-l[disable key name lookup and send data literally]' \
    '-F[expand formats in arguments where appropriate]' \
    '(-l)-H[interpret key as hexadecimal number for an ASCII character]' \
    '-R[reset terminal state]' \
    '-M[pass through a mouse event]' \
    '-X[send a command into copy mode]' \
    '-N+[specify repeat count]:repeat count' \
    '-t+[specify target pane]:pane:__tmux-panes' \
    '*:key'
}

_tmux-send-prefix() {
  [[ -n ${tmux_describe} ]] && print "send the prefix key to a window" && return
  _arguments -s \
    '-2[send secondary prefix key]' \
    '-t+[specify target pane]:pane:__tmux-panes'
}

_tmux-server-info() {
  [[ -n ${tmux_describe} ]] && print "show server information" && return
  __tmux-nothing-else
}

_tmux-set-buffer() {
  [[ -n ${tmux_describe} ]] && print "set contents of a paster buffer" && return
  _arguments -s -A "-*" -S \
    '-a[append to rather than overwriting target buffer]' \
    '-b+[specify target buffer index]:pane:__tmux-buffers' \
    '-n+[specify new buffer name]:buffer-name' \
    '-t+[specify target client]:client:__tmux-clients' \
    '-w[also send the buffer to the clipboard using the xterm escape sequence]' \
    ':data'
}

_tmux-set-environment() {
  [[ -n ${tmux_describe} ]] && print "(un)set an environment variable" && return
  local mode=session action=add
  local curcontext="$curcontext" state line ret=1
  typeset -A opt_args
  _arguments -C -s -A "-*" -S : \
    '-F[expand value as a format]' \
    '(-t)-g[modify global environment]' \
    '-h[mark the variable as hidden]' \
    '(-u)-r[remove variable before starting new processes]' \
    '(-r)-u[unset a variable]' \
    '(-g)-t[specify target session]:target session:__tmux-sessions' \
    ': :->name' '(-u -r)2: :->value' && ret=0

  if (( ${+opt_args[-g]} )); then
      mode=global
  fi
  if (( ${+opt_args[-u]} )); then
      action=unset
  fi
  if (( ${+opt_args[-r]} )); then
      action=remove
  fi

  # TODO: the exclusion "(-g -r)2:" doesn't work, so simulate it here
  if [[ $action == (remove|unset) ]] && [[ $state == value ]]; then
      __tmux-nothing-else
  else
      __tmux-environment-variables $mode $state $action && ret=0
  fi
  return ret
}

_tmux-set-option() {
  [[ -n ${tmux_describe} ]] && print "set a session option" && return
  local mode=session
  local curcontext="$curcontext" state line ret=1
  typeset -A opt_args
  _arguments -C -s : \
    '-a[append to string options]' \
    '-F[expand formats in the option value]' \
    '-g[set a global session option]' \
    '-o[prevent setting of an option that is already set]' \
    '-q[suppress errors about unknown or ambiguous options]' \
    '-u[unset a non-global option]' \
    '-U[unset a pane option across all panes in the window]' \
    '(-w -s)-p[change pane (no session) options]' \
    '(-p -s)-w[change window (not session) options]' \
    '(-p -w)-s[change server (not session) options]' \
    '-t+[specify target session]:target session:__tmux-sessions' \
    '*:: :->name_or_value' && ret=0

  if (( ${+opt_args[-w]} )); then
    mode=window
  elif (( ${+opt_args[-s]} )); then
    mode=server
  fi
  __tmux-options-complete ${mode} ${state} && ret=0
  return ret
}

_tmux-set-window-option() {
  [[ -n ${tmux_describe} ]] && print "set a window option" && return
  local curcontext="$curcontext" state line ret=1
  typeset -A opt_args
  _arguments -C -s : \
    '-a[append to string options]' \
    '-F[expand formats in the option value]' \
    '-g[set a global window option]' \
    '-o[prevent setting of an option that is already set]' \
    '-q[suppress errors about unknown or ambiguous options]' \
    '-u[unset a non-global option]' \
    '-t+[specify target window]:target window:__tmux-windows' \
    '*:: :->name_or_value' && ret=0
  __tmux-options-complete window ${state} && ret=0
  return ret
}

_tmux-set-hook() {
  [[ -n ${tmux_describe} ]] && print "set a hook to a command" && return
  _arguments -s \
    '-a[append to hook]' \
    '(-R)-g[add hook to global list]' \
    '(-g -u)-R[run hook immediately]' \
    '(-w)-p[set pane hooks]' \
    '(-p)-w[set window hooks]' \
    '(-R)-u[unset a hook]' \
    '-t+[specify target session]:session:__tmux-sessions' \
    ':hook name:_tmux_hooks' \
    '*:::command:= _tmux'
}

_tmux-show-hooks() {
  [[ -n ${tmux_describe} ]] && print "show the global list of hooks" && return
  _arguments -s -S -A "-*" \
    '-g[show global list of hooks]' \
    '(-w)-p[show pane hooks]' \
    '(-p)-w[show window hooks]' \
    '-t+[specify target session]:session:__tmux-sessions' \
}

_tmux-show-buffer() {
  [[ -n ${tmux_describe} ]] && print "display the contents of a paste buffer" && return
  _arguments '-b[specify target buffer index]:pane:->buffer'
}

_tmux-show-environment() {
  [[ -n ${tmux_describe} ]] && print "display the environment" && return
  local mode=session
  local curcontext="$curcontext" state line ret=1
  typeset -A opt_args
  _arguments -C -A "-*" -s : \
    '(-t)-g[show global environment]' \
    '-h[show hidden variables]' \
    '-s[format output as Bourne shell commands]' \
    '(-g)-t+[specify target session]:target session:__tmux-sessions' \
    '1:: :->name' && ret=0

  if (( ${+opt_args[-g]} )); then
      mode=global
  fi

  __tmux-environment-variables $mode $state show && ret=0
  return ret
}

_tmux-show-messages() {
  [[ -n ${tmux_describe} ]] && print "show client's message log" && return
  _arguments -s \
    '-J[show debugging information about running jobs]' \
    '-T[show debugging information about involved terminals]' \
    '-t+[specify target client]:client:__tmux-clients'
}

_tmux-show-options() {
  [[ -n ${tmux_describe} ]] && print "show session options" && return
  local mode=session
  local curcontext="$curcontext" state line ret=1
  typeset -A opt_args
  _arguments -C -s : \
    '-A[include options inherited from a parent set]' \
    '-H[include hooks]' \
    '-g[show global options]' \
    '-q[suppress errors about unknown or ambiguous options]' \
    '-v[show only the option value, not the name]' \
    '(-s -w)-p[show pane (not session) options]' \
    '(-p -w)-s[show server (not session) options]' \
    '(-p -s)-w[show window (not session) options]' \
    '-t+[specify target session]:target session:__tmux-sessions' \
    '*:: :->name_or_value' && ret=0

  if (( ${+opt_args[-w]} )); then
    mode=window
  elif (( ${+opt_args[-s]} )); then
    mode=server
  fi
  __tmux-options-complete ${mode} ${state} && ret=0
  return ret
}

_tmux-show-window-options() {
  [[ -n ${tmux_describe} ]] && print "show window options" && return
  local curcontext="$curcontext" state line ret=1
  typeset -A opt_args
  _arguments -C -s : \
    '-g[show global options]' \
    '-v[show only the option value, not the name]' \
    '-t+[specify target window]:target window:__tmux-windows' \
    '*:: :->name_or_value' && ret=0
  __tmux-options-complete window ${state} && ret=0
  return ret
}

_tmux-source-file() {
  [[ -n ${tmux_describe} ]] && print "execute tmux commands from a file" && return
  _arguments \
    '-F[expand path as a format]' \
    '-n[file is parsed but no commands are executed]' \
    "-q[don't report error if path doesn't exist]" \
    '-v[show parsed commands and line numbers if possible]' \
    '*:path:_directories'
}

_tmux-split-window() {
  [[ -n ${tmux_describe} ]] && print "splits a pane into two" && return
  # -p removed from documentation but still works, or use -l and append %
  _arguments -s \
    '-b[create new pane left of or above target pane]' \
    "-d[don't make the new window become the active one]" \
    '*-e[specify environment variable]:environment variable:_parameters -g "*export*" -qS=' \
    '-F+[specify output format]:format:__tmux-formats' \
    '(-l)-f[create new pane spanning full window width or height]' \
    '-h[split horizontally]' \
    '-v[split vertically]' \
    "(-p)-l[define new pane's size]:size" \
    "!(-f -l)-p+:size (percentage)" \
    '-t+[specify target pane]:pane:__tmux-panes' \
    '-Z[zoom the pane]' \
    '(:)-I[create empty pane and forward stdin to it]' \
    ':command:_cmdambivalent'
  # Yes, __tmux-panes is correct here. The behaviour was changed
  # in recent tmux versions and makes more sense. Except that
  # changing the command's name might annoy users. So it stays like
  # this.
}

_tmux-start-server() {
  [[ -n ${tmux_describe} ]] && print "start a tmux server" && return
  __tmux-nothing-else
}

_tmux-suspend-client() {
  [[ -n ${tmux_describe} ]] && print "suspend a client" && return
  _arguments '-t+[specify destination client]:client:__tmux-clients'
}

_tmux-swap-pane() {
  [[ -n ${tmux_describe} ]] && print "swap two panes" && return
  _arguments -s \
    '-D[move pane down]' \
    '-U[move pane up]' \
    '-Z[keep the window zoomed if it was zoomed]' \
    "-d[don't change the active pane]" \
    '-s+[specify source pane]:pane:__tmux-panes' \
    '-t+[specify destination pane]:pane:__tmux-panes'
}

_tmux-swap-window() {
  [[ -n ${tmux_describe} ]] && print "swap two windows" && return
  _arguments -s \
    "-d[don't make the new window become the active one]" \
    '-s+[specify source window]:window:__tmux-windows' \
    '-t+[specify destination window]:window:__tmux-windows'
}

_tmux-switch-client() {
  [[ -n ${tmux_describe} ]] && print "switch the client to another session" && return
  _arguments -s \
    '-c+[specify a target client]:client:__tmux-clients' \
    "-E[don't apply update-environment option]" \
    '-l[move client to last session]' \
    '-n[move client to next session]' \
    '-p[move client to previous session]' \
    '-r[toggle read-only flag of client]' \
    '-Z[keep the window zoomed if it was zoomed]' \
    '-t+[specify target window]:window:__tmux-windows'
}

_tmux-unbind-key() {
  [[ -n ${tmux_describe} ]] && print "unbind a key" && return
  local curcontext="$curcontext" state line keytable
  local -a ow

  ow=( "${words[@]}" )
  _arguments -C -s \
    '-a[remove all key bindings]' \
    '-n[remove a non-prefix binding]' \
    '-q[prevent errors being returned]' \
    '-T[specify key table]:key table' \
    '*:: :->boundkeys'

    [[ ${state} != 'boundkeys' ]] && return
    keytable="$(__tmux-get-optarg -t "${ow[@]}")"
    if [[ -n ${keytable} ]]; then
        __tmux-bound-keys -t ${keytable}
        return
    fi
    __tmux-bound-keys
}

_tmux-unlink-window() {
  [[ -n ${tmux_describe} ]] && print "unlink a window" && return
  _arguments -s \
    '-k[kill the window if it is only in one session]' \
    '-t+[specify target window]:target window:__tmux-windows'
}

_tmux-wait-for() {
  [[ -n ${tmux_describe} ]] && print "wait for an event or trigger it" && return
  _arguments -s \
    '-L[lock the named channel]' \
    '-S[send signal to channel]' \
    '-U[unlock the named channel]' \
    ':event channel'
}

# --- Utility functions ---
# They should be called __tmux-*() and kept separate from the
# sub-command functions.

function __tmux-attributes() {
    local -a attr already
    attr=( default bright bold dim underscore blink reverse hidden italics )
    compset -P '*,'
    already=( ${(s<,>)IPREFIX} )
    _describe -t tmux-attribute 'tmux attribute' attr -S, -F already -q
}

function __tmux-buffers() {
    local expl
    local -a buffers

    if [[ ${(t)bopts} != *array* ]]; then
        local -a bopts; bopts=()
    fi

    buffers=( ${${(f)"$(command tmux 2> /dev/null list-buffers "${bopts[@]}")"}/:[ $'\t']##/:} )
    _describe -t buffers 'buffer' buffers
}

function __tmux-bound-keys() {
    local expl
    local -a keys

    keys=( ${${${${(f)"$(command tmux 2> /dev/null list-keys "$@")"}/:[ $'\t']##/:}/(#s)[ $'\t']##/}/(#s):/\\:} )
    _describe -t keys 'key' keys
}

function __tmux-clients() {
    local expl
    local -a clients
    clients=( ${${(f)"$(command tmux 2> /dev/null list-clients)"}/:[ $'\t']##/:} )
    _describe -t clients 'client' clients
}

function __tmux-environment-variables() {
    local mode="$1" state="$2" action="$3"

    local -a dash_g
    case $mode in
        (global) dash_g=(-g);;
        (session) dash_g=();;
        (*) return 1;; # bug in the caller
    esac

    local hint
    case $action in
        (add|remove) hint=" (or specify a new one)";;
        (unset|show) hint="";;
        (*) return 1;; # bug in the caller
    esac

    case ${state} in
        (name) 
            local -a vars_and_vals=( ${(@f)"$(command tmux 2>/dev/null show-env $dash_g)"} )
            local -a descriptions
            local k_v k v
            for k_v in $vars_and_vals; do
                k=${k_v%%=*}
                if [[ $k == -* ]]; then
                    k=${k#-}
                    v='(remove)'
                else
                    v=${k_v#*=}
                fi
                descriptions+=( "${k//:/\\:}:$v" )
            done
            # TODO: this if/else is because '_describe ${hint:+"-x"}' prints the "No matches" error in addition to the message.
            local msg="${dash_g[1]:+"global "}environment variable${hint}"
            if _describe -t parameters $msg descriptions; then
                :
            elif [[ -n $hint ]]; then
                _message -e $msg
            fi
            ;;
        (value)
            local var_and_val=${(@f)"$(command tmux 2>/dev/null show-env $dash_g -- ${(Q)words[-2]})"}
            # TODO: this if/else is because '_description -x' prints the "No matches" error in addition to the message.
            if [[ -n $var_and_val ]]; then
                local -a expl
                _description -x parameter-values expl "value for ${words[-2]}"
                compadd "$expl[@]" - ${var_and_val#*=}
            else
                _message -e "value for ${words[-2]}"
            fi
            ;;
        (*)
            return 1
            ;;
    esac
}

__tmux-formats() {
  local hash='#' open='{' close='}' paren='(' quest='?'
  local tmux_variables expl
  compquote hash open close paren quest
  compset -p ${#PREFIX%$hash*}
  if compset -P "${(q)hash}${open}"; then
    if compset -P "${(q)quest}"; then
      close=,
    elif ! compset -P "([bdt]|s/[^/]#/[^/]#/|=(-|)<->):"; then
      _describe -t operators operator '(
	\?:conditional
	\=:length\ limit
      )' -S '' -- '(
	m:fnmatch\ comparison
	t:convert\ time\ to\ string
	b:basename
	c:search\ for\ fnmatch\ pattern\ in\ pane\ content
	d:dirname
	\==:comparison
	\!=:comparison
      )' -S : -- '(
	s:substitution
      )' -S / -- '(
	\|\|:either\ of\ two\ conditions
	\&\&:both\ of\ two\ conditions
      )' -S ,
    fi
    tmux_variables=(
      'alternate_on:if pane is in alternate screen'
      'alternate_saved_x:saved cursor X in alternate screen'
      'alternate_saved_y:saved cursor Y in alternate screen'
      'buffer_created:time buffer created'
      'buffer_name:name of buffer'
      'buffer_sample:sample of start of buffer'
      'buffer_size:size of the specified buffer in bytes'
      'client_activity:time client last had activity'
      'client_created:time client created'
      'client_control_mode:1 if client is in control mode'
      'client_discarded:bytes discarded when client behind'
      'client_height:height of client'
      'client_key_table:current key table'
      "client_last_session:name of the client's last session"
      'client_name:name of client'
      'client_pid:PID of client process'
      'client_prefix:1 if prefix key has been pressed'
      'client_readonly:1 if client is readonly'
      "client_session:name of the client's session"
      'client_termname:terminal name of client'
      'client_termtype:terminal type of client'
      'client_tty:pseudo terminal of client'
      'client_utf8:1 if client supports utf8'
      'client_width:width of client'
      'client_written:bytes written to client'
      'command:name of command in use, if any'
      'command_list_name:command name if listing commands'
      'command_list_alias:command alias if listing commands'
      'command_list_usage:command usage if listing commands'
      'cursor_flag:pane cursor flag'
      'cursor_character:character at cursor in pane'
      'cursor_x:cursor X position in pane'
      'cursor_y:cursor Y position in pane'
      'history_bytes:number of bytes in window history'
      'history_limit:maximum window history lines'
      'history_size:size of history in lines'
      'hook:name of running hook, if any'
      'hook_pane:ID of pane where hook was run, if any'
      'hook_session:ID of session where hook was run, if any'
      'hook_session_name:name of session where hook was run, if any'
      'hook_window:ID of window where hook was run, if any'
      'hook_window_name:name of window where hook was run, if any'
      'host:hostname of local host'
      'host_short:hostname of local host (no domain name)'
      'insert_flag:pane insert flag'
      'keypad_cursor_flag:pane keypad cursor flag'
      'keypad_flag:pane keypad flag'
      'line:line number in the list'
      'mouse_any_flag:pane mouse any flag'
      'mouse_button_flag:pane mouse button flag'
      'mouse_standard_flag:pane mouse standard flag'
      'mouse_all_flag:pane mouse all flag'
      'pane_active:1 if active pane'
      'pane_at_bottom:1 if pane is at the bottom of window'
      'pane_at_left:1 if pane is at the left of window'
      'pane_at_right:1 if pane is at the right of window'
      'pane_at_top:1 if pane is at the top of window'
      'pane_bottom:bottom of pane'
      'pane_current_command:current command if available'
      'pane_dead:1 if pane is dead'
      'pane_dead_status:exit status of process in dead pane'
      'pane_format:1 if format is for a pane (not assuming the current)'
      'pane_height:height of pane'
      'pane_id:unique pane ID'
      'pane_in_mode:if pane is in a mode'
      'pane_input_off:if input to pane is disabled'
      'pane_index:index of pane'
      'pane_left:left of pane'
      'pane_mode:name of pane mode, if any.'
      'pane_pid:PID of first process in pane'
      'pane_pipe:1 if pane is being piped'
      'pane_right:right of pane'
      'pane_search_string:last search string in copy mode'
      'pane_start_command:command pane started with'
      'pane_synchronized:if pane is synchronized'
      'pane_tabs:pane tab positions'
      'pane_title:title of pane'
      'pane_top:top of pane'
      'pane_tty:pseudo terminal of pane'
      'pane_width:width of pane'
      'pid:server PID'
      'rectangle_toggle:1 if rectangle selection is activated'
      'scroll_region_lower:bottom of scroll region in pane'
      'scroll_region_upper:top of scroll region in pane'
      'scroll_position:scroll position in copy mode'
      'selection_present:1 if selection started in copy mode'
      'session_alerts:list of window indexes with alerts'
      'session_attached:number of clients session is attached to'
      'session_activity:time of session last activity'
      'session_created:time session created'
      'session_format:1 if format is for a session (not assuming the current)'
      'session_last_attached:time session last attached'
      'session_group:name of session group'
      'session_group_size:size of session group'
      'session_group_list:list of sessions in group'
      'session_grouped:1 if session in a group'
      'session_id:unique session ID'
      'session_many_attached:1 if multiple clients attached'
      'session_name:name of session'
      'session_stack:window indexes in most recent order'
      'session_width:width of session'
      'session_windows:number of windows in session'
      'socket_path:server socket path'
      'start_time:server start time'
      'version:server version'
      'window_activity:time of window last activity'
      'window_activity_flag:1 if window has activity'
      'window_active:1 if window active'
      'window_bell_flag:1 if window has bell'
      'window_flags:window flags'
      'window_format:1 if format is for a window (not assuming the current)'
      'window_height:height of window'
      'window_id:unique window ID'
      'window_index:index of window'
      'window_last_flag:1 if window is the last used'
      'window_layout:window layout description, ignoring zoomed window panes'
      'window_linked:1 if window is linked across sessions'
      'window_name:name of window'
      'window_offset_x:X offset into window if larger than client'
      'window_offset_y:Y offset into window if larger than client'
      'window_panes:number of panes in window'
      'window_silence_flag:1 if window has silence alert'
      'window_stack_index:index in session most recent stack'
      'window_visible_layout:window layout description, respecting zoomed window panes'
      'window_width:width of window'
      'window_zoomed_flag:1 if window is zoomed'
      'wrap_flag:pane wrap flag'
    )
    _describe -t variables variable tmux_variables -S "$close"
  elif compset -P "${(q)hash}${(q)paren}"; then
    compset -S '(\\|)\)*'
    _cmdstring
  elif [[ $PREFIX = ${hash}* ]]; then
    _describe -t variables variable '(
      \#H:local\ hostname
      \#h:short\ local\ hostname
      \#D:pane\ id
      \#P:pane\ index
      \#T:pane\ title
      \#S:session\ name
      \#F:window\ flags
      \#I:window\ index
      \#W:window\ name
    )' -S ''
  else
    _wanted format-specifiers expl 'format specifier' compadd -S '' \#
  fi
}

function __tmux-colours() {
    local -a colnames
    colnames=( default black red green yellow blue magenta cyan white colourN:"replace N by a number between 0 and 255" )
    compset -P 'colour*'
    if [[ -z ${IPREFIX} ]]; then
        _describe -t tmux-colours 'colour' colnames
    else
        _message 'colour number 0..255'
    fi
}

_tmux_hooks() {
  _alternative \
    'hooks:hook name:(alert-activity alert-bell alert-silence client-attached client-detached client-resized client-session-changed pane-died pane-exited pane-set-clipboard session-created session-closed session-renamed window-linked window-renamed window-unlinked)' \
    'post-hooks:command post-hook:compadd - after-${_tmux_aliasmap}'
}

_tmux_client_flags() {
  _values -s , flag active-pane ignore-size no-output \
      'pause-after:time (seconds)' read-only wait-exit
}

function __tmux-get-optarg() {
    local opt="$1"
    local -i i
    shift

    for (( i = 1; i <= $#; i++ )); do
        if [[ ${argv[$i]} == ${opt} ]]; then
            if [[ ${argv[$(( i + 1 ))]} != -* ]]; then
                print -- ${argv[$(( i + 1 ))]}
            fi
            return
        fi
    done
}

__tmux-nothing-else() {
  _message -e "no further arguments"
}

function __tmux-option-guard() {
    local mode opt guard int_guard
    mode="$1"
    opt="$2"
    shift; shift
    local -a options desc
    int_guard='_guard "[0-9]#" "'${opt}': numeric value"'
    if [[ ${mode} == 'session' ]]; then
        options=(
            'activity-action:DESC:any none current other'
            'assume-paste-time:'${int_guard}
            'base-index:'${int_guard}
            'bell-action:DESC:any none current other'
            'default-command:MSG:command string'
            'default-shell:MSG:shell executable'
            'default-size:MSG:XxY'
            'destroy-unattached:DESC:on off'
            'detach-on-destroy:DESC:on off'
            'display-panes-colour:__tmux-colours'
            'display-panes-active-colour:__tmux-colours'
            'display-panes-time:'${int_guard}
            'display-time:'${int_guard}
            'history-limit:'${int_guard}
            'lock-after-time:'${int_guard}
            'lock-command:MSG:command string'
            'message-command-style:__tmux-style'
            'message-style:__tmux-style'
            'mouse:DESC:on off'
            'prefix:MSG:primary prefix key'
            'prefix2:MSG:secondary prefix key'
            'renumber-windows:DESC:on off'
            'repeat-time:'${int_guard}
            'set-titles:DESC:on off'
            'set-titles-string:MSG:title format string'
            'silence-action:DESC:any none current other'
            'status:DESC:on off'
            'status-format:MSG:format string'
            'status-interval:'${int_guard}
            'status-justify:DESC:left centre right'
            'status-keys:DESC:vi emacs'
            'status-left:MSG:format string'
            'status-left-length:'${int_guard}
            'status-left-style:__tmux-style'
            'status-position:DESC:top bottom'
            'status-right:MSG:format string'
            'status-right-length:'${int_guard}
            'status-right-style:__tmux-style'
            'status-style:__tmux-style'
            'update-environment:MSG:string listing env. variables'
            'user-keys:MSG:key'
            'visual-activity:DESC:on off'
            'visual-bell:DESC:on off'
            'visual-silence:DESC:on off'
            'word-separators:MSG:separator string'
        )
    elif [[ ${mode} == 'server' ]]; then
        options=(
            'buffer-limit:'${int_guard}
            'command-alias:MSG:alias'
            'default-terminal:MSG:terminal string'
            'escape-time:'${int_guard}
            'exit-empty:DESC:on off'
            'exit-unattached:DESC:on off'
            'focus-events:DESC:on off'
            'history-file:_path-files -g "*(-.)"'
            'message-limit:'${int_guard}
            'set-clipboard:DESC:on off'
            'terminal-overrides:MSG:overrides string'
        )
    else
        options=(
            'aggressive-resize:DESC:on off'
            'allow-rename:DESC:on off'
            'alternate-screen:DESC:on off'
            'automatic-rename:DESC:on off'
            'automatic-rename-format:DESC:__tmux-format'
            'clock-mode-colour:__tmux-colours'
            'clock-mode-style:DESC:12 24'
            'main-pane-height:'${int_guard}
            'main-pane-width:'${int_guard}
            'mode-keys:DESC:vi emacs'
            'mode-style:__tmux-style'
            'monitor-activity:DESC:on off'
            'monitor-bell:DESC:on off'
            'monitor-silence:DESC:on off'
            'other-pane-height:'${int_guard}
            'other-pane-width:'${int_guard}
            'pane-active-border-style:__tmux-style'
            'pane-base-index:'${int_guard}
            'pane-border-format:MSG:pane border status string'
            'pane-border-status:DESC:off top bottom'
            'pane-border-style:__tmux-style'
            'remain-on-exit:DESC:on off'
            'synchronize-panes:DESC:on off'
            'window-active-style:__tmux-style'
            'window-status-activity-style:__tmux-style'
            'window-status-bell-style:__tmux-style'
            'window-status-current-format:MSG:status format string'
            'window-status-current-style:__tmux-style'
            'window-status-format:MSG:status format string'
            'window-status-last-style:__tmux-style'
            'window-status-separator:MSG:separator string'
            'window-status-style:__tmux-style'
            'window-size:MSG:XxY'
            'window-style:__tmux-style'
            'wrap-search:DESC:on off'
            'xterm-keys:DESC:on off'
        )
    fi

    guard=${(M)options:#$opt:*}
    if [[ -z ${guard} ]]; then
        _message "unknown ${mode} option: ${opt}"
        return
    fi
    guard=${guard#*:}
    case ${guard} in
        ('') ;;
        (MSG:*)
            _message -e ${guard#*:}
            ;;
        (DESC:*)
            eval "desc=( ${guard#*:} )"
            _describe -t "tmux-${mode}-option-value" "${opt}" desc
            ;;
        (*)
            eval ${guard}
            ;;
    esac
}

function __tmux-session-options() {
    local -a tmux_session_options
    tmux_session_options=(
        'activity-action:set action on window activity when monitor-activity is on'
        'assume-paste-time:assume keys are pasted instead of typed if this fast'
        'base-index:define where to start numbering'
        'bell-action:set action on window bell'
        'default-command:default command for new windows'
        'default-shell:default shell executable'
        'default-size:set the default size of windows when the size is not set'
        'destroy-unattached:destroy session if no client is attached'
        'detach-on-destroy:detach client if attached session is destroyed'
        'display-panes-colour:colour used for display-panes'
        'display-panes-active-colour:colour for active pane in display-panes'
        'display-panes-time:time (in msecs) of display-panes output'
        'display-time:time (in msecs) messages are displayed'
        'history-limit:number of copy-mode lines per window'
        'key-table:default key table'
        'lock-after-time:lock sessions after N seconds'
        'lock-command:command to run for locking a client'
        'message-command-style:status line message command style'
        'message-style:status line message style'
        'mouse:enable mouse support'
        'prefix:primary prefix key'
        'prefix2:secondary prefix key'
        'renumber-windows:renumber windows if a window is closed'
        'repeat-time:time for multiple commands without prefix-key presses'
        'set-titles:try to set xterm window titles'
        'set-titles-string:format used by set-titles'
        'silence-action:set action on window silence when monitor-silence is on'
        'status:show or hide the status bar'
        'status-format:specify the format to be used for each line of the status line'
        'status-interval:interval (in seconds) for status bar updates'
        'status-justify:position of the window list in status bar'
        'status-keys:mode to use in status bar modes (vi/emacs)'
        'status-left:format to use left in status bar'
        'status-left-length:maximum length of the left part of the status bar'
        'status-left-style:style of left part of status line'
        'status-position:status line position'
        'status-right:format to use right in status bar'
        'status-right-length:maximum length of the right part of the status bar'
        'status-right-style:style of right part of status line'
        'status-style:style status line'
        "update-environment:list of variables to be copied to a session's environment"
        'user-keys:set list of user-defined key escape sequences'
        'visual-activity:display status line messages upon activity'
        'visual-bell:use visual bell instead of audible'
        'visual-silence:print a message if monitor-silence is on'
        'word-separators:string of characters considered word separators'
    )
    _describe -t tmux-options 'tmux session option' tmux_session_options
}

function __tmux-options-complete() {
    local mode="$1" state="$2"

    case ${state} in
        name_or_value)
            if (( CURRENT == 1 )) && [[ ${mode} == 'session' ]]; then
                __tmux-session-options
            elif (( CURRENT == 1 )) && [[ ${mode} == 'server' ]]; then
                __tmux-server-options
            elif (( CURRENT == 1 )) && [[ ${mode} == 'window' ]]; then
                __tmux-window-options
            elif (( CURRENT == 2 )); then
                __tmux-option-guard ${mode} ${words[1]}
            else
                __tmux-nothing-else
            fi
            ;;
    esac
}

function __tmux-panes() {
    local expl line orig="$IPREFIX"
    local -i num
    local -a panes opts

    compset -P '*.' && opts=( -t "${${IPREFIX%.}#$orig}" )
    num=0
    command tmux 2> /dev/null list-panes "${opts[@]}" | while IFS= read -r line; do
        panes+=( $(( num++ )):${line//:/} )
    done
    _describe -t panes 'pane' panes "$@"
    if [[ ${IPREFIX} != *. ]]; then
        _wanted windows expl 'window' __tmux-windows -S.
    fi
}

function __tmux-server-options() {
    local -a tmux_server_options
    tmux_server_options=(
        'buffer-limit:number of buffers kept per session'
        'command-alias:custom command aliases'
        'default-terminal:default terminal definition string'
        'escape-time:set timeout to detect single escape characters (in msecs)'
        'exit-unattached:make server exit if it has no attached clients'
        'exit-empty:exit when there are no active sessions'
        'focus-events:request focus events from terminal'
        'history-file:tmux command history file name'
        'message-limit:set size of message log per client'
        'set-clipboard:use esc sequences to set terminal clipboard'
        'terminal-overrides:override terminal descriptions'
    )
    _describe -t tmux-server-options 'tmux server option' tmux_server_options
}

function __tmux-sessions() {
    local -a sessions
    sessions=( ${${(f)"$(command tmux 2> /dev/null list-sessions)"}/:[ $'\t']##/:} )
    _describe -t sessions 'session' sessions "$@"
}

function __tmux-sessions-attached() {
    local -a sessions
    sessions=( ${${(f)"$(command tmux 2> /dev/null list-sessions)"}/:[ $'\t']##/:} )
    sessions=( ${(M)sessions:#*"(attached)"} )
    _describe -t sessions 'attached session' sessions "$@"
}

# Complete attached-sessions and detached-sessions as separate tags.
function __tmux-sessions-separately() {
    local ret=1
    local -a sessions detached_sessions attached_sessions
    sessions=( ${${(f)"$(command tmux 2> /dev/null list-sessions)"}/:[ $'\t']##/:} )
    detached_sessions=(    ${sessions:#*"(attached)"} )
    attached_sessions=( ${(M)sessions:#*"(attached)"} )

    # ### This seems to work without a _tags loop but not with it.  I suspect
    # ### that has something to do with _describe doing its own _tags loop.
    _tags detached-sessions attached-sessions
    # Placing detached before attached means the default behaviour of this
    # function better suits its only current caller, _tmux-attach-session().
    _requested detached-sessions && _describe -t detached-sessions 'detached session' detached_sessions "$@" && ret=0
    _requested attached-sessions && _describe -t attached-sessions 'attached session' attached_sessions "$@" && ret=0

    return ret
}

function __tmux-socket-name() {
    local expl sdir
    local curcontext="${curcontext}"
    local -a socks
    zstyle -s ":completion:${curcontext}:sockets" socketdir sdir || sdir="${TMUX_TMPDIR:-/tmp}/tmux-${UID}"
    socks=(${sdir}/*(=:t))
    _wanted socket expl 'socket name' compadd ${expl} -- ${socks}
}

function __tmux-style() {
    _message 'not implemented yet'
}

function __tmux-window-options() {
    local -a tmux_window_options
    tmux_window_options=(
        'aggressive-resize:aggressively resize windows'
        'allow-rename:allow programs to change window titles'
        'alternate-screen:allow alternate screen feature to be used'
        'automatic-rename:attempt to automatically rename windows'
        'automatic-rename-format:format for automatic renames'
        'clock-mode-colour:set clock colour'
        'clock-mode-style:set clock hour format (12/24)'
        'main-pane-height:set height for main-* layouts'
        'main-pane-width:set width for main-* layouts'
        'mode-keys:mode to use in copy and choice modes (vi/emacs)'
        'mode-style:set window modes style'
        'monitor-activity:monitor window activity'
        'monitor-bell:monitor for a bell in the window'
        'monitor-silence:monitor window for inactivity'
        'other-pane-height:height of other panes'
        'other-pane-width:width of other panes'
        'pane-active-border-style:style of border of active pane'
        'pane-base-index:integer at which to start indexing panes'
        'pane-border-format:set pane border format string'
        'pane-border-status:turn border status off or set its position'
        'pane-border-style:style of border pane'
        "remain-on-exit:don't destroy windows after the program exits"
        'synchronize-panes:send input to all panes of a window'
        'window-active-style:style of active window'
        'window-status-activity-style:style of status bar activity tag'
        'window-status-bell-style:style of status bar bell tag'
        'window-status-current-format:set status line format for active window'
        'window-status-current-style:style of current window in status bar'
        'window-status-format:set status line format for all but the active window'
        'window-status-last-style:style of last window in status bar'
        'window-status-separator:separator drawn between windows in status line'
        'window-status-style:general status bar style'
        'window-size:indicate how to automatically size windows'
        'window-style:style of window'
        'wrap-search:search wrap around at the end of a pane'
        'xterm-keys:generate xterm-style function key sequences'
    )
    _describe -t tmux-window-options 'tmux window option' tmux_window_options
}

function __tmux-windows() {
    local expl
    local -a wins opts

    compset -P '*:'
    if [[ -n ${IPREFIX} ]]; then
        opts=( -t "${IPREFIX%:}" )
    else
        opts=( )
    fi
    wins=( ${${(M)${(f)"$(command tmux 2> /dev/null list-windows "${opts[@]}")"}:#<->*}/:[ $'\t']##/:} )
    _describe -t windows 'window' wins "$@"
    if [[ ${IPREFIX} != *: ]]; then
        _wanted sessions expl 'session' __tmux-sessions -S:
    fi
}

# And here is the actual _tmux(), that puts it all together:
_tmux() {
  local curcontext="${curcontext}" state line ret=1
  local mode
  local tmuxcommand
  local tmux_describe

  _arguments -C -s -w \
    '-2[force using 256 colours]' \
    '-c[execute a shell command]:command name:_command_names' \
    '-C[start tmux in control mode. -CC disables echo]' \
    "-D[don't start the tmux server as a daemon]" \
    '-f[specify configuration file]:tmux config file:_files -g "*(-.)"' \
    '-l[behave like a login shell]' \
    '-L[specify socket name]:socket name:__tmux-socket-name' \
    "-N[don't start the server even if the command would normally do so]" \
    '-S[specify socket path]:server socket:_path_files -g "*(=,/)"' \
    '-T+[set terminal features for the client]: : _values -s , 256 clipboard ccolour cstyle extkeys focus margins mouse overline rectfill RGB strikethrough sync title usstyle' \
    '-u[force using UTF-8]' \
    '-v[request verbose logging]' \
    '-V[report tmux version]' \
    '*:: :->subcommand_or_options' && ret=0

  [[ -z $state ]] && return ret

  if (( CURRENT == 1 )); then
    zstyle -s ":completion:${curcontext}:subcommands" mode mode || mode='both'
    if [[ ${mode} == 'commands' ]]; then
      _describe -t subcommands 'tmux command' _tmux_commands && ret=0
    elif [[ ${mode} == 'aliases' ]]; then
      _describe -t subcommands 'tmux alias' _tmux_aliases && ret=0
    else
      _describe -t subcommands 'tmux command or alias' _tmux_commands -- _tmux_aliases && ret=0
    fi
  else
    tmuxcommand="${words[1]}"
    if [[ -n ${_tmux_aliasmap[$tmuxcommand]} ]] ; then
      tmuxcommand="${_tmux_aliasmap[$tmuxcommand]}"
    fi
    if ! (( ${+functions[_tmux-$tmuxcommand]} )); then
      local low high
      low=$_tmux_commands[(i)$tmuxcommand*]
      high=$_tmux_commands[(I)$tmuxcommand*]
      if (( low == high )); then
	tmuxcommand=${_tmux_commands[low]%%:*}
      elif (( low < high )); then
	_message "ambiguous command $tmuxcommand"
      else
	_message "subcommand $tmuxcommand not known"
	_normal && ret=0
      fi
    fi
    curcontext="${curcontext%:*}-${tmuxcommand}:"
    _call_function ret _tmux-${tmuxcommand}
  fi
  return ret
}

# description generation follows; only done on 1st _tmux call.
local f desc
local -A rev
local tmux_describe
tmux_describe='yes, please'
for f in ${(k)_tmux_aliasmap} ; do
  rev+=( ${_tmux_aliasmap[$f]} $f )
done
for f in ${(M)${(k)functions}:#_tmux-*} ; do
  desc="$($f)"
  _tmux_commands+=( "${f#_tmux-}${desc:+:$desc}" )
  [[ -n ${rev[${f#_tmux-}]} ]] && _tmux_aliases+=( "${rev[${f#_tmux-}]}${desc:+:$desc}" )
done

_tmux "$@"
