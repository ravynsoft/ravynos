# Back end for replace-string; can be called as a widget to repeat
# the previous replacement.  _replace_string_src and _replace_string_rep
# are global.

# When called from replace-string, we need to use the widget
# name passed to decide whether to do pattern matching: the widget
# may since have been overwritten.
local MATCH MBEGIN MEND curwidget=${1:-$WIDGET}
local -a match mbegin mend

if [[ -z $_replace_string_src ]]; then
  zle -M "No string to replace."
  return 1
fi

if [[ $curwidget = *(pattern|regex)* ]]; then
    local rep2
    # The following horror is so that an & preceded by an even
    # number of backslashes is active, without stripping backslashes,
    # while preceded by an odd number of backslashes is inactive,
    # with one backslash being stripped.  A similar logic applies
    # to \digit.
    local rep=$_replace_string_rep
    while [[ $rep = (#b)([^\\]#)(\\\\)#(\\|)(\&|\\<->|\\\{<->\})(*) ]]; do
	if [[ -n $match[3] ]]; then
	    # Expression is quoted, strip quotes
	    rep2="${match[1]}${match[2]}${match[4]}"
	else
	    rep2+="${match[1]}${match[2]}"
	    if [[ $match[4] = \& ]]; then
		rep2+='${MATCH}'
	    elif [[ $match[4] = \\\{* ]]; then
		rep2+='${match['${match[4][3,-2]}']}'
	    else
		rep2+='${match['${match[4][2,-1]}']}'
	    fi
	fi
	rep=${match[5]}
    done
    rep2+=$rep
    if [[ $curwidget = *regex* ]]; then
      autoload -Uz regexp-replace
      integer ret=1
      regexp-replace LBUFFER $_replace_string_src $rep2 && ret=0
      regexp-replace RBUFFER $_replace_string_src $rep2 && ret=0
      return ret
    else
      LBUFFER=${LBUFFER//(#bm)$~_replace_string_src/${(e)rep2}}
      RBUFFER=${RBUFFER//(#bm)$~_replace_string_src/${(e)rep2}}
    fi
else
    LBUFFER=${LBUFFER//$_replace_string_src/$_replace_string_rep}
    RBUFFER=${RBUFFER//$_replace_string_src/$_replace_string_rep}
fi
