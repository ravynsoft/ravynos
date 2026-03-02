# Restrict the start of the editable line to the region between cursor
# and mark (including the character at the end).  Can be bound used as
# a zle widget, or called as a function from another widget.
#
# Optionally accepts exactly two arguments, which are used instead of
# $CURSOR and $MARK as limits to the range.
#
# Upon exit, $MARK is always the start of the edited range and $CURSOR
# the end of the range, even if they began in the opposite order.
#
# Other options:
#   -p pretext   show "pretext" instead of the buffer text before the region.
#   -P posttext  show "posttext" instead of the buffer text after the region.
#                Either or both may be empty.
#   -n           Only replace the text before or after the region with
#                the -p or -P options if the text was not empty.
#   -l lbufvar   $lbufvar is assigned the value of $LBUFFER and
#   -r rbufvar   $rbufvar is assigned the value of $RBUFFER
#                from any recursive edit (i.e. not with -S or -R).  Neither
#                lbufvar nor rbufvar may begin with the prefix "_ntr_".
#   -S statevar
#   -R statevar
#      Save or restore the state in/from the parameter named statevar.  In
#      either case no recursive editing takes place; this will typically be
#      done within the calling function between calls with -S and -R.  The
#      statevar may not begin with the prefix "_ntr_" which is reserved for
#      parameters within narrow-to-region.

# set the minimum of options to avoid changing behaviour away from
# user preferences from within recursive-edit
setopt localoptions noshwordsplit noksharrays

local _ntr_newbuf _ntr_lbuf_return _ntr_rbuf_return
local _ntr_predisplay=$PREDISPLAY _ntr_postdisplay=$POSTDISPLAY
integer _ntr_savelim=UNDO_LIMIT_NO _ntr_changeno _ntr_histno=HISTNO
integer _ntr_start _ntr_end _ntr_swap _ntr_cursor=$CURSOR _ntr_mark=$MARK
integer _ntr_stat

local _ntr_opt _ntr_pretext _ntr_posttext _ntr_usepretext _ntr_useposttext
local _ntr_nonempty _ntr_save _ntr_restore _ntr_lbuffer _ntr_rbuffer

while getopts "l:np:P:r:R:S:" _ntr_opt; do
  case $_ntr_opt in
    (l) _ntr_lbuf_return=$OPTARG
	;;
    (n) _ntr_nonempty=1
	;;
    (p) _ntr_pretext=$OPTARG _ntr_usepretext=1
	;;
    (P) _ntr_posttext=$OPTARG _ntr_useposttext=1
	;;
    (r) _ntr_rbuf_return=$OPTARG
	;;
    (R) _ntr_restore=$OPTARG
        ;;
    (S)	_ntr_save=$OPTARG
	;;
    (*) [[ $_ntr_opt != '?' ]] && print "$0: unhandled option: $_ntr_opt" >&2
	return 1
	;;
  esac
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

if [[ $_ntr_restore = _ntr_* || $_ntr_save = _ntr_* ||
      $_ntr_lbuf_return = _ntr_* || $_ntr_rbuf_return = _ntr_* ]]; then
  zle -M "$0: _ntr_ prefix is reserved" >&2
  return 1
fi

if [[ -n $_ntr_save || -z $_ntr_restore ]]; then

  if (( $# )); then
    if (( $# != 2 )); then
      zle -M "$0: supply zero or two arguments" >&2
      return 1
    fi
    _ntr_start=$1
    _ntr_end=$2
  else
    _ntr_start=$MARK
    _ntr_end=$CURSOR
  fi

  if (( _ntr_start > _ntr_end )); then
    _ntr_swap=_ntr_start
    _ntr_start=_ntr_end
    _ntr_end=_ntr_swap
  fi

  (( _ntr_cursor -= _ntr_start, _ntr_mark -= _ntr_start ))
  
  _ntr_lbuffer=${BUFFER[1,_ntr_start]}
  if [[ -z $_ntr_usepretext || ( -n $_ntr_nonempty && -z $_ntr_lbuffer ) ]]
  then
    _ntr_pretext=$_ntr_lbuffer
  fi
  _ntr_rbuffer=${BUFFER[_ntr_end+1,-1]}
  if [[ -z $_ntr_useposttext || ( -n $_ntr_nonempty && -z $_ntr_rbuffer ) ]]
  then
    _ntr_posttext=$_ntr_rbuffer
  fi
  _ntr_changeno=$UNDO_CHANGE_NO
  PREDISPLAY="$_ntr_predisplay$_ntr_pretext"
  POSTDISPLAY="$_ntr_posttext$_ntr_postdisplay"

  if [[ -n $_ntr_save ]]; then
    builtin typeset -ga $_ntr_save
    set -A $_ntr_save "${_ntr_predisplay}" "${_ntr_postdisplay}" \
	"${_ntr_savelim}" "${_ntr_changeno}" \
	"${_ntr_start}" "${_ntr_end}" "${_ntr_histno}" || return 1
  fi

  BUFFER=${BUFFER[_ntr_start+1,_ntr_end]}
  CURSOR=$_ntr_cursor
  MARK=$_ntr_mark
  zle split-undo
  UNDO_LIMIT_NO=$UNDO_CHANGE_NO
fi

if [[ -z $_ntr_save && -z $_ntr_restore ]]; then
  zle recursive-edit
  _ntr_stat=$?

  [[ -n $_ntr_lbuf_return ]] &&
    builtin typeset -g ${_ntr_lbuf_return}="${LBUFFER}"
  [[ -n $_ntr_rbuf_return ]] &&
    builtin typeset -g ${_ntr_rbuf_return}="${RBUFFER}"
fi

if [[ -n $_ntr_restore || -z $_ntr_save ]]; then
  if [[ -n $_ntr_restore ]]; then
    if ! { _ntr_predisplay="${${(@P)_ntr_restore}[1]}"
           _ntr_postdisplay="${${(@P)_ntr_restore}[2]}"
	   _ntr_savelim="${${(@P)_ntr_restore}[3]}"
	   _ntr_changeno="${${(@P)_ntr_restore}[4]}"
	   _ntr_start="${${(@P)_ntr_restore}[5]}"
	   _ntr_end="${${(@P)_ntr_restore}[6]}"
	   _ntr_histno="${${(@P)_ntr_restore}[7]}" }; then
      zle -M Failed. >&2
      return 1
    fi
  fi

  _ntr_newbuf="$BUFFER"
  HISTNO=_ntr_histno
  zle undo $_ntr_changeno
  PREDISPLAY=$_ntr_predisplay
  POSTDISPLAY=$_ntr_postdisplay
  BUFFER[_ntr_start+1,_ntr_end]="$_ntr_newbuf"
  (( MARK = _ntr_start, CURSOR = _ntr_start + ${#_ntr_newbuf} ))
  UNDO_LIMIT_NO=_ntr_savelim
fi

return $_ntr_stat
