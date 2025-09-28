## vim:ft=zsh
## perforce support by: Phil Pennock
## Distributed under the same BSD-ish license as zsh itself.

# If use-server is true in the :vcs_info:p4:... context, contact the
# server to decide whether the directory is handled by Perforce.  This can
# cause a delay if the network times out, in particular if looking up the
# server name failed.  Hence this is not the default.  If a timeout
# occurred, the server:port pair is added to the associative array
# vcs_info_p4_dead_servers and the server is never contacted again.  The
# array must be edited by hand to remove it.
#
# If use-server is false or not set, the function looks to see if there is
# a file $P4CONFIG somewhere above in the hierarchy.  This is far from
# foolproof; in fact it relies on you using the particular working practice
# of having such files in all client root directories and nowhere above.


(( ${+functions[VCS_INFO_p4_get_server]} )) ||
VCS_INFO_p4_get_server() {
  emulate -L zsh
  setopt extendedglob

  local -a settings
  settings=(${(f)"$(${vcs_comm[cmd]} set)"})
  serverport=${${settings[(r)P4PORT=*]##P4PORT=}%% *}
  case $serverport in
    (''|:)
    serverport=perforce:1666
    ;;

    (:*)
    serverport=perforce${serverport}
    ;;

    (*:)
    serverport=${serverport}1666
    ;;

    (<->)
    serverport=perforce:${serverport}
    ;;
  esac
}


VCS_INFO_detect_p4() {
  local serverport p4where

  if zstyle -t ":vcs_info:p4:${usercontext}:${rrn}" use-server; then
    # Use "p4 where" to decide whether the path is under the
    # client workspace.
    if (( ${#vcs_info_p4_dead_servers} )); then
      # See if the server is in the list of defunct servers
      VCS_INFO_p4_get_server
      [[ -n $vcs_info_p4_dead_servers[$serverport] ]] && return 1
    fi
    if p4where="$(${vcs_comm[cmd]} where 2>&1)"; then
      return 0
    fi
    if [[ $p4where = *"Connect to server failed"* ]]; then
      # If the connection failed, mark the server as defunct.
      # Otherwise it worked but we weren't within a client.
      typeset -gA vcs_info_p4_dead_servers
      [[ -z $serverport ]] && VCS_INFO_p4_get_server
      vcs_info_p4_dead_servers[$serverport]=1
    fi
    return 1
  else
    [[ -n ${P4CONFIG} ]] || return 1
    VCS_INFO_check_com ${vcs_comm[cmd]} || return 1
    vcs_comm[detect_need_file]="${P4CONFIG}"
    VCS_INFO_bydir_detect .
    return $?
  fi
}

VCS_INFO_detect_p4 "$@"
