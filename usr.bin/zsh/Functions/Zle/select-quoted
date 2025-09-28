# Text object for matching characters between a particular delimiter
#
# So for example, given "text", the vi command vi" will select
# all the text between the double quotes
#
# The following is an example of how to enable this:
#     autoload -U select-quoted
#     zle -N select-quoted
#     for m in visual viopp; do
#	for c in {a,i}{\',\",\`}; do
#	  bindkey -M $m $c select-quoted
#	done
#     done

setopt localoptions noksharrays

local matching=${${1:-$KEYS}[2]}
local -i start=CURSOR+2 end=CURSOR+2 found=0 alt=0 count=0

if ((REGION_ACTIVE )); then
  if (( MARK < CURSOR )); then
    start=MARK+2
  else
    end=MARK+2
  fi
fi

[[ $BUFFER[CURSOR+1] = $matching && $BUFFER[CURSOR] != \\ ]] && count=1
while (( (count || ! alt) && --start )) && [[ $BUFFER[start] != $'\n' ]]; do
  if [[ $BUFFER[start] = "$matching" ]]; then
    if [[ $BUFFER[start-1] = \\ ]]; then
      (( start-- ))
    elif (( ! found )); then
      found=start
    else
      (( ! alt )) && alt=start
      (( count && ++count ))
    fi
  fi
done

for (( start=CURSOR+2; ! found && start+1 < $#BUFFER; start++ )); do
  case $BUFFER[start] in
    $'\n') return 1 ;;
    \\) (( start++ )) ;;
    "$matching")
      (( end=start+1, found=start ))
    ;;
  esac
done

[[ $BUFFER[end-1] = \\ ]] && (( end++ ))
until [[ $BUFFER[end] == "$matching" ]]; do
  [[ $BUFFER[end] = \\ ]] && (( end++ ))
  if [[ $BUFFER[end] = $'\n' ]] || (( ++end > $#BUFFER )); then
    end=0
    break
  fi
done

if (( alt && (!end || count == 2) )); then
  end=found
  found=alt
fi
(( end )) || return 1

[[ ${${1:-$KEYS}[1]} = a ]] && (( found-- )) || (( end-- ))
(( REGION_ACTIVE = !!REGION_ACTIVE ))
[[ $KEYMAP = vicmd ]] && (( REGION_ACTIVE && end-- ))
MARK=found
CURSOR=end
