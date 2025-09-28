# I noticed we don't ship any contrib and/or example scripts using the
# zcurses module, and also that the builtin tetris is sort of boring, so
# I figured I'd port it to curses. It works pretty well, but I noticed
# two problems with the zcurses module in the process:
# 
# 1. the HAVE_USE_DEFAULT_COLORS define seems to never be defined?
# 
# 2a. resizing the window causes 'zcurses input' to wait forever for a
# key, even with a timeout defined.
# 
# Bart says:
# >This probably has something to do with the special-casing around wgetch()
# >for signals handled by the "trap" command.  See the big comment in
# >Src/Modules/curses.c lines 1073-1103.
# 
# >It may be problematic to mix curses with the generic signal handling in
# >the main shell.  We may need to swap in a SIGWINCH handler wrapper while
# >the curses UI is active, and restore the main handler when leaving it.
# 
# 2b. resizing the window doesn't cause an event while running the
# program, but if i resize before starting(?) i get an event RESIZE on
# my first input call.
# 
# Bart says:
# >There's probably some state that needs to be cleared on entry to
# >zccmd_input() so that curses doesn't see something left over from the
# >previous signal.  Unfortunately I don't know what that would be.

if (( $LINES < 22 || $COLUMNS < 46 )); then
  echo >&2 'terminal needs to be at least 22 lines and 46 columns'
  return
fi

emulate -L zsh

typeset -a tetris_shapes
tetris_shapes=(
  0x0f00 0x4444 0x0f00 0x4444
  0x4e00 0x4c40 0x0e40 0x4640
  0x6600 0x6600 0x6600 0x6600
  0x4620 0x6c00 0x4620 0x6c00
  0x2640 0x6300 0x2640 0x6300
  0x6440 0x8e00 0x44c0 0x0e20
  0xc440 0x0e80 0x4460 0x2e00
)
typeset -A tetris_rotations
tetris_rotations=(
  0x0f00 0x4444 0x4444 0x0f00
  0x4e00 0x4c40 0x4c40 0x0e40 0x0e40 0x4640 0x4640 0x4e00
  0x6600 0x6600
  0x4620 0x6c00 0x6c00 0x4620
  0x2640 0x6300 0x6300 0x2640
  0x6440 0x8e00 0x8e00 0x44c0 0x44c0 0x0e20 0x0e20 0x6440
  0xc440 0x0e80 0x0e80 0x4460 0x4460 0x2e00 0x2e00 0xc440
)
local tetris_vsz=20 tetris_hsz=11
local tetris_blankline=${(l:11:: :)}
local tetris_blankboard=${(j::):-${(l:11:: :)}${(s: :)^${(l:20:: :)}}}

local tetris_board=$tetris_blankboard
local tetris_score=0
local tetris_lines=0

local tetris_{block{,_next,_x,_y},i}

function __tetris-next-block {
  tetris_block_next=$tetris_shapes[1+RANDOM%$#tetris_shapes]
}

function __tetris-new-block {
  tetris_block=$tetris_block_next
  __tetris-next-block
  __tetris-draw-next-block
  tetris_block_y=0
  tetris_block_x=4
  if ! __tetris-block-fits; then
    __tetris-game-over
  fi
  __tetris-place-block "*"
}

function __tetris-left {
  __tetris-place-block " "
  (( tetris_block_x-- ))
  __tetris-block-fits || (( tetris_block_x++ ))
  __tetris-place-block "*"
}

function __tetris-right {
  __tetris-place-block " "
  (( tetris_block_x++ ))
  __tetris-block-fits || (( tetris_block_x-- ))
  __tetris-place-block "*"
}

function __tetris-rotate {
  __tetris-place-block " "
  local save_block=$tetris_block
  tetris_block=$tetris_rotations[$tetris_block]
  __tetris-block-fits || tetris_block=$save_block
  __tetris-place-block "*"
}

function __tetris-drop {
  __tetris-place-block " "
  ((tetris_block_y++))
  while __tetris-block-fits; do
    ((tetris_block_y++))
    ((tetris_score+=2))
  done
  ((tetris_block_y--))
  __tetris-block-dropped
}

function __tetris-timeout {
  __tetris-place-block " "
  ((tetris_block_y++))
  if __tetris-block-fits; then
    __tetris-place-block "*"
    return
  fi
  ((tetris_block_y--))
  __tetris-block-dropped
}

function __tetris-block-dropped {
  integer bonus=1
  __tetris-place-block "O"
  local fl=${tetris_blankline// /O} i=$((tetris_block_y*tetris_hsz))
  repeat 4; do
    if [[ $tetris_board[i+1,i+tetris_hsz] == $fl ]]; then
      if (( fancygraphics )); then for char in {7..1}; do
        tetris_board[i+1,i+tetris_hsz]=${tetris_blankline// /$char}
        __tetris-render-screen
        zcurses timeout score 50
        zcurses input score
      done; fi
      tetris_board[i+1,i+tetris_hsz]=
      tetris_board=$tetris_blankline$tetris_board
      ((tetris_score+=100*(bonus++*(tetris_lines/10+10))))
      ((tetris_lines+=1))
      if ((tetris_lines % 10 == 0)); then
        ((timestep = timestep * 0.80))
      fi
    fi
    ((i += tetris_hsz))
  done
  __tetris-new-block
}

function __tetris-block-fits {
  local y x i=$((1+tetris_block_y*tetris_hsz+tetris_block_x)) b=0x8000
  for ((y=0; y!=4; y++)); do
    for ((x=0; x!=4; x++)); do
      if ((tetris_block&b)); then
        ((x+tetris_block_x >= 0)) || return 1
        ((x+tetris_block_x < tetris_hsz)) || return 1
        ((y+tetris_block_y >= 0)) || return 1
        ((y+tetris_block_y < tetris_vsz)) || return 1
        [[ $tetris_board[i] == " " ]] || return 1
      fi
      ((b >>= 1))
      ((i++))
    done
    ((i+=tetris_hsz-4))
  done
  return 0
}

function __tetris-draw-next-block {
  local tetris_preview
  local y x i=1 b=0x8000
  for ((y=0; y!=4; y++)); do
    tetris_preview="    "
    for ((x=0; x!=4; x++)); do
      ((tetris_block_next&b)) && tetris_preview[i]=\*
      ((b >>= 1))
      ((i++))
    done
    i=1
    zcurses move preview $((y+1)) 1
    zcurses string preview ${${${tetris_preview//O/$filled_block}//\*/$active_block}// /  }
  done
}

function __tetris-place-block {
  local y x i=$((1+tetris_block_y*tetris_hsz+tetris_block_x)) b=0x8000
  for ((y=0; y!=4; y++)); do
    for ((x=0; x!=4; x++)); do
      ((tetris_block&b)) && tetris_board[i]=$1
      ((b >>= 1))
      ((i++))
    done
    ((i+=tetris_hsz-4))
  done
}

function __tetris-render-screen {
  local i x piece
  setopt localoptions histsubstpattern extendedglob
  local -a match mbegin mend
  local -A animation
  animation=( 7 ▇▇ 6 ▆▆ 5 ▅▅ 4 ▄▄ 3 ▃▃ 2 ▂▂ 1 ▁▁ )
  for (( i = 0; i < tetris_vsz; i++ )); do
    zcurses move gamearea $(( i + 1 )) 1
    zcurses string gamearea ${${${${${tetris_board[1+i*tetris_hsz,(i+1)*tetris_hsz]}//O/$filled_block}//\*/$active_block}// /  }//(#b)([1-7])/$animation[$match[1]]}
  done

  zcurses clear score
  zcurses move score 1 1
  zcurses string score "Score: $tetris_score"$'\
'" Lines: $tetris_lines"$'\
'" Speed: ${timestep%.*} ms"

  zcurses border gamearea
  zcurses border score
  zcurses border preview
  zcurses refresh gamearea score preview $debug
}

function __tetris-game-over {
  gameover=1
}

function __tetris-new-game {
  gameover=0
  timestep=1000
  tetris_score=0
  tetris_lines=0
  __tetris-next-block
  __tetris-new-block
  __tetris-render-screen
}

function __tetris-game-over-screen {
  __tetris-debug "Died with $tetris_score points!"
  tetris_board=$tetris_blankboard
  local text="You got $tetris_score points!"
  local gameover_height=4 gameover_width=$(( $#text + 2 ))
  zcurses addwin gameover $gameover_height $gameover_width \
                          $(( off_y + (game_height-gameover_height)/2 )) \
                          $(( off_x + (game_width+score_width-gameover_width)/2 ))
  zcurses move gameover 1 1
  zcurses string gameover $text
  text='Play again? [yn]'
  zcurses move gameover 2 $(( (gameover_width - $#text)/2 ))
  zcurses string gameover $text
  zcurses border gameover
  keepplaying=
  until [[ $keepplaying = [ynq] ]]; do
    zcurses input gameover keepplaying
  done
  zcurses delwin gameover
  zcurses refresh stdscr
  zcurses timeout gamearea ${timestep%.*}
  __tetris-new-game
}

function __tetris-debug {
  if [[ -z $debug ]]; then
    return
  fi
  zcurses scroll debug -1
  zcurses move debug 0 0
  zcurses string debug "$1"
}

function __tetris-remove-wins {
  local delwin
  local -a delwins
  delwins=(gamearea score debug gameover help preview)
  for delwin in ${delwins:*zcurses_windows}; do
    zcurses delwin $delwin
  done
}

function __tetris-help {
  local i
  local help_height=9 help_width=23
  zcurses addwin help $help_height $help_width \
                      $(( off_y + (game_height - help_height) / 2 )) \
                      $(( off_x + (game_width + score_width - help_width) / 2 ))
  zcurses move help 1 1
  zcurses string help $'left: h, j, left\
 right: right, n, l\
 rotate: up, c, i\
 soft drop: down, t, k\
 hard drop: space\
 quit: q\
 press space to return'
  zcurses border help
  until [[ $i == [\ q] ]]; do
    zcurses input help i
    if [[ $i == q ]]; then
      keepplaying=n
    fi
  done
  zcurses delwin help
  zcurses refresh stdscr
}

zmodload zsh/curses && {
  zcurses init
  __tetris-remove-wins
  zcurses refresh
  echoti civis
  local debug=
  if (( ${@[(I)--debug|-d]} )); then
    debug=debug
  fi
  local off_x off_y
  local game_height=22   game_width=25
  local score_height=5   score_width=20
  local preview_height=6 preview_width=10
  local filled_block active_block 
  local fancygraphics
  if zmodload zsh/langinfo && [[ $langinfo[CODESET] = UTF-8 ]]; then
    filled_block=██
    active_block=▒▒
    fancygraphics=${@[(I)--silly]}
  else
    filled_block='[]'
    active_block='()'
    fancygraphics=0
  fi
  off_x=$(( (COLUMNS-game_width-score_width-1) / 2 ))
  off_y=$(( (LINES-game_height) / 2 ))
  zcurses clear stdscr redraw
  zcurses refresh stdscr
  zcurses addwin gamearea $game_height $game_width $off_y $off_x
  zcurses scroll gamearea off
  zcurses addwin score $score_height $score_width \
                       $off_y $(( off_x + game_width + 1 ))
  zcurses scroll score off
  zcurses addwin preview $preview_height $preview_width \
                         $(( off_y + score_height )) $(( off_x + game_width + 1 ))
  zcurses scroll preview off
  if [[ -n $debug ]]; then
    zcurses addwin debug $(( game_height - score_height - preview_height - 1 )) \
                         $score_width \
                         $(( off_y + score_height + preview_height ))\
                         $(( off_x + game_width + 1 ))
  fi
  typeset -F SECONDS
  local now prev timestep timeout key kkey keepplaying=y gameover=0
  prev=$SECONDS
  __tetris-new-game
  zcurses timeout gamearea 0
  while [[ $keepplaying == y ]]; do
    if zcurses input gamearea key kkey; then
      __tetris-debug "got input $key$kkey"
      case $key$kkey in
        LEFT|h|j)  __tetris-left;;
        RIGHT|n|l) __tetris-right;;
        UP|c|i)    __tetris-rotate;;
        DOWN|t|k)  __tetris-timeout; ((tetris_score++)); prev=$SECONDS;;
        " ")       __tetris-drop;;
        q)         break;;
        F1|H)      __tetris-help;;
      esac
    else
      __tetris-debug "timed out"
      __tetris-timeout
    fi
    now=$SECONDS
    if (( prev + timestep/1000. < now )); then
      (( prev += timestep/1000. ))
    fi
    timeout=${$(( 1000.*(prev + timestep/1000. - now) + 1 ))%.*}
    if (( timeout < 0 )); then
      __tetris-debug "BUG: timeout < 0"
      timeout=${timestep%.*}
    fi
    zcurses timeout gamearea $timeout
    __tetris-debug "timeout: $timeout"

    __tetris-render-screen
    if [[ $gameover == 1 ]]; then
      __tetris-game-over-screen
    fi
  done
} always {
  __tetris-remove-wins
  echoti cnorm
  zcurses end
}
