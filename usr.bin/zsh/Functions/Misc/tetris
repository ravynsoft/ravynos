# Someone once accused zsh of not being as complete as Emacs, because it
# lacks Tetris and an adventure game.
#
# autoload -Uz tetris
# zle -N tetris
# bindkey '...' tetris

emulate -L zsh

tetris_hsz=11
tetris_vsz=20
typeset -ga tetris_shapes
tetris_shapes=(
	0x0f00 0x4444 0x0f00 0x4444
	0x4e00 0x4c40 0x0e40 0x4640
	0x6600 0x6600 0x6600 0x6600
	0x4620 0x6c00 0x4620 0x6c00
	0x2640 0x6300 0x2640 0x6300
	0x6440 0x8e00 0x44c0 0x0e20
	0xc440 0x0e80 0x4460 0x2e00
)
typeset -gA tetris_rotations
tetris_rotations=(
	0x0f00 0x4444 0x4444 0x0f00
	0x4e00 0x4c40 0x4c40 0x0e40 0x0e40 0x4640 0x4640 0x4e00
	0x6600 0x6600
	0x4620 0x6c00 0x6c00 0x4620
	0x2640 0x6300 0x6300 0x2640
	0x6440 0x8e00 0x8e00 0x44c0 0x44c0 0x0e20 0x0e20 0x6440
	0xc440 0x0e80 0x0e80 0x4460 0x4460 0x2e00 0x2e00 0xc440
)

tetris_blankline=
for ((tetris_i=tetris_hsz; tetris_i--; )); do
	tetris_blankline="$tetris_blankline "
done
tetris_blankboard=
for ((tetris_i=tetris_vsz; tetris_i--; )); do
	tetris_blankboard="$tetris_blankboard$tetris_blankline"
done

bindkey -N tetris
bindkey -R -M tetris '\000-\377' tetris-timeout
for ((tetris_i=256; tetris_i--; )); do
	bindkey -M tetris 'T\'$(([##8]tetris_i)) tetris-timeout
done
bindkey -M tetris Ta tetris-left
bindkey -M tetris Tj tetris-left
bindkey -M tetris Ts tetris-rotate
bindkey -M tetris Tk tetris-rotate
bindkey -M tetris Td tetris-right
bindkey -M tetris Tl tetris-right
bindkey -M tetris 'T ' tetris-drop
bindkey -M tetris Tq tetris-quit

unset tetris_board tetris_score

zle -N tetris
function tetris {
	emulate -L zsh
	if ! zle; then
		print -u2 "Use M-x tetris RET to play tetris."
		return 2
	fi
	tetris_saved_state="BUFFER=${BUFFER:q};CURSOR=${CURSOR:q};MARK=${MARK:q};zle -K ${KEYMAP:q}"
	tetris_speed=$((100.0/KEYTIMEOUT))
	zle -K tetris
	if [[ ${tetris_board+set} == set ]]; then
		tetris-timeout
	else
		tetris_board=$tetris_blankboard
		tetris_score=0
		tetris-new-block
	fi
}

function tetris-new-block {
	emulate -L zsh
	tetris_block=$tetris_shapes[1+RANDOM%$#tetris_shapes]
	tetris_block_y=0
	tetris_block_x=4
	if ! tetris-block-fits; then
		tetris-place-block "#"
		tetris-render-screen
		unset tetris_board tetris_score
		tetris-quit
		return
	fi
	tetris-place-block "*"
	tetris-timed-move
}

zle -N tetris-left
function tetris-left {
	emulate -L zsh
	tetris-place-block " "
	(( tetris_block_x-- ))
	tetris-block-fits || (( tetris_block_x++ ))
	tetris-place-block "*"
	tetris-timeout
}

zle -N tetris-right
function tetris-right {
	emulate -L zsh
	tetris-place-block " "
	(( tetris_block_x++ ))
	tetris-block-fits || (( tetris_block_x-- ))
	tetris-place-block "*"
	tetris-timeout
}

zle -N tetris-rotate
function tetris-rotate {
	emulate -L zsh
	tetris-place-block " "
	local save_block=$tetris_block
	tetris_block=$tetris_rotations[$tetris_block]
	tetris-block-fits || tetris_block=$save_block
	tetris-place-block "*"
	tetris-timeout
}

zle -N tetris-drop
function tetris-drop {
	emulate -L zsh
	tetris-place-block " "
	((tetris_block_y++))
	while tetris-block-fits; do
		((tetris_block_y++))
	done
	((tetris_block_y--))
	tetris-block-dropped
}

zle -N tetris-timeout
function tetris-timeout {
	emulate -L zsh
	tetris-place-block " "
	((tetris_block_y++))
	if tetris-block-fits; then
		tetris-place-block "*"
		tetris-timed-move
		return
	fi
	((tetris_block_y--))
	tetris-block-dropped
}

function tetris-block-dropped {
	emulate -L zsh
	tetris-place-block "O"
	local fl=${tetris_blankline// /O} i=$((tetris_block_y*tetris_hsz)) y
	for ((y=0; y!=4; y++)); do
		if [[ $tetris_board[i+1,i+tetris_hsz] == $fl ]]; then
			tetris_board[i+1,i+tetris_hsz]=
			tetris_board=$tetris_blankline$tetris_board
			((tetris_score++))
		fi
		((i += tetris_hsz))
	done
	tetris-new-block
}

function tetris-block-fits {
	emulate -L zsh
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

function tetris-place-block {
	emulate -L zsh
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

function tetris-timed-move {
	emulate -L zsh
	tetris-render-screen
	LBUFFER=
	RBUFFER=$'\n'$tetris_screen
	zle -R
	zle -U T
}

function tetris-render-screen {
	emulate -L zsh
	setopt extendedglob
	local s i extras
	extras=(
		"Score: $tetris_score"
		""
		"Game parameters: ${tetris_hsz}x$tetris_vsz, ${tetris_speed}Hz"
		""
		"Keys:   left: a j"
		"      rotate: s k"
		"       right: d l"
		"        drop: space"
		"        quit: q"
	)
	for ((i=0; i!=tetris_vsz; i++)); do
		s="$s|${${${${${tetris_board[1+i*tetris_hsz,(i+1)*tetris_hsz]}//O/()}//\*/**}// /  }//\#/##}|"${extras[1]+   $extras[1]}$'\n'
		extras[1]=()
	done
	s="$s+${tetris_blankline// /--}+"
	tetris_screen=$s
}

zle -N tetris-quit
function tetris-quit {
	emulate -L zsh
	if [[ ! -o always_last_prompt ]]; then
		BUFFER=
		zle -M $tetris_screen
	fi
	eval $tetris_saved_state
	if [[ -o always_last_prompt ]]; then
		zle -M $tetris_screen
	fi
}

tetris "$@"
