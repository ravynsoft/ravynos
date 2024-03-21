# Text object for matching characters between matching pairs of brackets
#
# So for example, given (( i+1 )), the vi command ci( will change
# all the text between matching colons.
#
# The following is an example of how to enable this:
#     autoload -U select-bracketed
#     zle -N select-bracketed
#     for m in visual viopp; do
#	for c in {a,i}${(s..)^:-'()[]{}<>bB'}; do
#	  bindkey -M $m $c select-bracketed
#	done
#     done

setopt localoptions noksharrays

local style=${${1:-$KEYS}[1]} matching="(){}[]<>bbBB"
local -i find=${NUMERIC:-1} idx=${matching[(I)[${${1:-$KEYS}[2]}]]}%9
(( idx )) || return 1 # no corresponding closing bracket
local lmatch=${matching[1 + ((idx-1) & ~1)]}
local rmatch=${matching[1 + ((idx-1) | 1)]}
local -i start=CURSOR+1 end=CURSOR+1 rfind=find

[[ $BUFFER[start] = "$rmatch" ]] && (( start--, end-- ))
if (( REGION_ACTIVE  && MARK != CURSOR)); then
  (( MARK < CURSOR && (start=end=MARK+1) ))
  local -i origstart=start-1
  [[ $style = i ]] && (( origstart-- ))
fi

while (( find )); do
  for (( ; find && start; --start )); do
    case $BUFFER[start] in
      "$lmatch") (( find-- )) ;;
      "$rmatch") (( find++ )) ;;
    esac
  done

  (( find )) && return 1 # opening bracket not found

  while (( rfind && end++ < $#BUFFER )); do
    case $BUFFER[end] in
      "$lmatch") (( rfind++ )) ;;
      "$rmatch") (( rfind-- )) ;;
    esac
  done

  (( rfind )) && return 1 # closing bracket not found

  (( REGION_ACTIVE && MARK != CURSOR && start >= origstart &&
    ( find=rfind=${NUMERIC:-1} ) ))
done

[[ $style = i ]] && (( start++, end-- ))
(( REGION_ACTIVE = !!REGION_ACTIVE ))
[[ $KEYMAP = vicmd ]] && (( REGION_ACTIVE && end-- ))
MARK=$start
CURSOR=$end
