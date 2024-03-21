## zsh_directory_name_generic
#
# This function is useful as a hook function for the zsh_directory_name
# facility.
#
# See the zsh-contrib manual page for more.

emulate -L zsh
setopt extendedglob
local -a match mbegin mend

# The variable containing the top level mapping.
local _zdn_topvar

zmodload -i zsh/parameter
zstyle -s ":zdn:${funcstack[2]}:" mapping _zdn_topvar || _zdn_topvar=zdn_top

if (( ! ${(P)#_zdn_topvar} )); then
  print -r -- "$0: $_zdn_topvar is not set" >&2
  return 1
fi

local _zdn_var=$_zdn_topvar
local -A _zdn_assoc

if [[ $1 = n ]]; then
  # Turning a name into a directory.
  local _zdn_name=$2
  local -a _zdn_words
  local _zdn_dir _zdn_cpt

  _zdn_words=(${(s.:.)_zdn_name})
  while (( ${#_zdn_words} )); do
    if [[ -z ${_zdn_var} ]]; then
      print -r -- "$0: too many components in directory name \`$_zdn_name'" >&2
      return 1
    fi

    # Subscripting (P)_zdn_var directly seems not to work.
    _zdn_assoc=(${(Pkv)_zdn_var})
    _zdn_cpt=${_zdn_assoc[${_zdn_words[1]}]}
    shift _zdn_words

    if [[ -z $_zdn_cpt ]]; then
      # If top level component, just try another expansion
      if [[ $_zdn_var != $_zdn_topvar ]]; then
	# Committed to this expansion, so report failure.
	print -r -- "$0: no expansion for directory name \`$_zdn_name'" >&2
      fi
      return 1
    fi
    if [[ $_zdn_cpt = (#b)(*)/:([[:IDENT:]]##) ]]; then
      _zdn_cpt=$match[1]
      _zdn_var=$match[2]
    else
      # may be empty
      _zdn_var=${${_zdn_assoc[:default:]}##*/:}
    fi
    _zdn_dir=${_zdn_dir:+$_zdn_dir/}$_zdn_cpt
  done
  if (( ${#_zdn_dir} )); then
    typeset -ag reply
    reply=($_zdn_dir)
    return 0
  fi
elif [[ $1 = d ]]; then
  # Turning a directory into a name.
  local _zdn_dir=$2
  local _zdn_rest=$_zdn_dir
  local -a _zdn_cpts
  local _zdn_pref _zdn_pref_raw _zdn_matched _zdn_cpt _zdn_name
  local _zdn_pref_matched _zdn_rest_matched
  integer _zdn_matchlen _zdn_len1

  while [[ -n $_zdn_var && -n $_zdn_rest ]]; do
    _zdn_matchlen=0
    _zdn_assoc=(${(Pkv)_zdn_var})
    _zdn_cpts=(${(Ov)_zdn_assoc})
    _zdn_cpt=''
    for _zdn_pref_raw in $_zdn_cpts; do
      _zdn_pref=${_zdn_pref_raw%/:*}
      [[ -z $_zdn_pref ]] && continue
      if [[ $_zdn_rest = $_zdn_pref(#b)(/|)(*) ]]; then
        _zdn_len1=${#_zdn_pref}
	if (( _zdn_len1 > _zdn_matchlen )); then
	  _zdn_matchlen=$_zdn_len1
	  _zdn_cpt=${(k)_zdn_assoc[(r)$_zdn_pref_raw]}
	  # if we matched a /, too, add it...
	  _zdn_pref_matched=$_zdn_pref$match[1]
	  _zdn_rest_matched=$match[2]
	fi
      fi
    done
    if (( _zdn_matchlen )); then
      _zdn_matched+=$_zdn_pref_matched
      _zdn_rest=$_zdn_rest_matched
    fi
    if [[ -n $_zdn_cpt ]]; then
      _zdn_name+=${_zdn_name:+${_zdh_name}:}$_zdn_cpt
      if [[ ${_zdn_assoc[$_zdn_cpt]} = (#b)*/:([[:IDENT:]]##) ]]; then
	_zdn_var=$match[1]
      else
	_zdn_var=${${_zdn_assoc[:default:]}##*/:}
      fi
    else
      break
    fi
  done
  if [[ -n $_zdn_name ]]; then
    # matched something, so report that.
    integer _zdn_len=${#_zdn_matched}
    [[ $_zdn_matched[-1] = / ]] && (( _zdn_len-- ))
    typeset -ag reply
    reply=($_zdn_name $_zdn_len)
    return 0
  fi
  # else let someone else have a go.
elif [[ $1 = c ]]; then
  # Completion

  if [[ -n $SUFFIX ]]; then
    _message "Can't complete in the middle of a dynamic directory name"
  else
    local -a _zdn_cpts
    local _zdn_word _zdn_cpt _zdn_desc _zdn_sofar expl

    while [[ -n ${_zdn_var} && ${PREFIX} = (#b)([^:]##):* ]]; do
      _zdn_word=$match[1]
      compset -P '[^:]##:'
      _zdn_assoc=(${(Pkv)_zdn_var})
      _zdn_cpt=${_zdn_assoc[$_zdn_word]}
      # We only complete at the end so must match here
      [[ -z $_zdn_cpt ]] && return 1
      if [[ $_zdn_cpt = (#b)(*)/:([[:IDENT:]]##) ]]; then
	_zdn_cpt=$match[1]
	_zdn_var=$match[2]
      else
	_zdn_var=${${_zdn_assoc[:default:]}##*/:}
      fi
      _zdn_sofar+=${_zdn_sofar:+${_zdn_sofar}/}$_zdn_cpt
    done
    if [[ -n $_zdn_var ]]; then
      _zdn_assoc=(${(Pkv)_zdn_var})
      local -a _zdn_cpts
      for _zdn_cpt _zdn_desc in ${(kv)_zdn_assoc}; do
	[[ $_zdn_cpt = :* ]] && continue
	_zdn_cpts+=(${_zdn_cpt}:${_zdn_desc%/:[[:IDENT:]]##})
      done
      _describe -t dirnames "directory name under ${_zdn_sofar%%/}" \
	_zdn_cpts -S: -r ':]'
      return
    fi
  fi
fi

# Failed
return 1
## end
