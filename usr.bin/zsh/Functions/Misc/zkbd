#!/bin/zsh -f

[[ -o interactive ]] && {
    local -hi ARGC		# local is a no-op outside of a function
    (ARGC=0) 2>/dev/null || {	# so ARGC remains read-only for "source"
        print -u2 ${0}: must be run as a function or shell script, not sourced
        return 1
    }
}

emulate -RL zsh
local zkbd term key seq

zkbd=${ZDOTDIR:-$HOME}/.zkbd
[[ -d $zkbd ]] || mkdir $zkbd || return 1

trap 'unfunction getmbkey getseq; command rm -f $zkbd/$TERM.tmp' 0
trap "return 1" 1 2 15

getmbkey () {
    local k='' i
    for ((i=10; i>0; --i))
    do
	read -t -k 1 k && break
	sleep 1
    done
    [[ -n $k ]] || return 1
    [[ $k = $'\012' || $k = $'\015' || $k = ' ' ]] && return 0
    # We might not be done yet, thanks to multibyte characters
    local mbk=$k
    while read -t -k 1 k
    do
       mbk=$mbk$k 
    done
    print -Rn $mbk
}

getseq () {
    trap "stty ${$(stty -g 2>/dev/null):-echo -raw}" 0 1 2 15
    stty raw -echo
    local k='' seq='' i
    for ((i=10; i>0; --i))
    do
	read -t -k 1 k && break
	sleep 1
    done
    [[ -n $k ]] || return 1
    [[ $k = $'\012' || $k = $'\015' || $k = ' ' ]] && return 0
    seq=$k
    while read -t -k 1 k
    do
       seq=$seq$k 
    done
    print -Rn ${(V)seq}
}

read term"?Enter current terminal type: [$TERM] "
[[ -n $term ]] && TERM=$term
print 'typeset -g -A key\n' > $zkbd/$TERM.tmp || return 1

cat <<\EOF

We will now test some features of your keyboard and terminal.

If you do not press the requested keys within 10 seconds, key reading will
abort.  If your keyboard does not have a requested key, press Space to
skip to the next key.

EOF

local ctrl alt meta

print -n "Hold down Ctrl and press X: "
ctrl=$(getmbkey) || return 1
print

if [[ $ctrl != $'\030' ]]
then
    print "Your keyboard does not have a working Ctrl key?"
    print "Giving up ..."
    return 1
else
    print
fi

print "Your Meta key may have a Microsoft Windows logo on the cap."
print -n "Hold down Meta and press X: "
meta=$(getmbkey) || return 1
print

if [[ $meta == x ]]
then
    print "Your keyboard or terminal does not recognize the Meta key."
    unset meta
elif [[ $meta > $'\177' ]]
then
    print "Your keyboard uses the Meta key to send high-order characters."
else
    unset meta
fi
print

print -n "Hold down Alt and press X: "
alt=$(getmbkey) || return 1
print

if [[ $alt == x ]]
then
    print "Your keyboard or terminal does not recognize the Alt key."
    unset alt
elif [[ $alt == $meta ]]
then
    print "Your keyboard does not distinguish Alt from Meta."
elif [[ $alt > $'\177' ]]
then
    print "Your keyboard uses the Alt key to send high-order characters."
else
    unset alt
fi

if (( $+alt + $+meta == 0 ))
then
    print $'\n---------\n'
    if [[ -o multibyte ]]
    then cat <<EOF
You are using zsh in MULTIBYTE mode to support modern character sets (for
languages other than English).  To use the Meta or Alt keys, you probably
need to revert to single-byte mode with a command such as

    unsetopt MULTIBYTE
EOF
    else cat <<EOF
Your current terminal and keyboard configuration does not appear to use
high-order characters.  You may be able to enable the Meta or Alt keys
with a command such as

    stty pass8
EOF
    fi
    cat <<EOF

If you want to use these extra keys with zsh, try adding the above command
to your ${ZDOTDIR:-$HOME}/.zshrc file.

See also "man stty" or the documentation for your terminal or emulator.
EOF
fi

(( $+alt || $+meta )) && cat <<EOF

---------

You may enable keybindings that use the \
${meta:+Meta}${meta:+${alt:+ and }}${alt:+Alt} key${meta:+${alt:+s}} \
by adding

    bindkey -m

to your ${ZDOTDIR:-$HOME}/.zshrc file.

EOF

read -k 1 key"?Press a key to proceed: "
[[ $key != $'\n' ]] && print

cat <<\EOF

---------

You will now be asked to press in turn each of the 12 function keys, then
the Backspace key, the 6 common keypad keys found on typical PC keyboards,
plus the 4 arrow keys, and finally the Menu key (near Ctrl on the right).
If your keyboard does not have the requested key, press Space to skip to
the next key.

Do not type ahead!  Wait at least one second after pressing each key for
zsh to read the entire sequence and prompt for the next key.  If a key
sequence does not echo within 2 seconds after you press it, that key may
not be sending any sequence at all.  In this case zsh is not able to make
use of that key.  Press Space to skip to the next key.

EOF

read -k 1 key"?Press a key when ready to begin: "
[[ $key != $'\n' ]] && print

cat <<\EOF

If you do not press a key within 10 seconds, key reading will abort.
If you make a mistake, stop typing and wait, then run this program again.

EOF

# There are 509 combinations of the following three arrays that represent
# possible keystrokes.  (Actually, Sun keyboards don't have Meta or Menu,
# though some have R{1..12} keys as well, so really there are either 433
# or 517 combinations; but some X11 apps map Shift-F{1..11} to emulate the
# unmodified Sun keys, so really only the 345 PC combinations are usable.
# Let's not even get into distinguishing Left and Right Shift/Alt/Meta.)
# No one would ever want to type them all into this program (would they?),
# so by default ask for the 23 unmodified PC keys.  If you uncomment more,
# you should fix the introductory text above.

local -a pckeys sunkeys modifiers
pckeys=(F{1..12}
        Backspace  Insert  Home   PageUp 
                   Delete  End   PageDown
                            Up
                    Left   Down   Right
        Menu
       )
sunkeys=(Stop  Again
         Props Undo
         Front Copy
         Open  Paste
         Find  Cut
            Help
        )
modifiers=(Shift- # Control- Alt- Meta-
           # Control-Shift- Alt-Shift- Meta-Shift-
           # Control-Alt- Control-Meta- Alt-Meta-
           # Control-Alt-Shift- Control-Meta-Shift-
           # Alt-Meta-Shift- Control-Alt-Meta-Shift-
          )

exec 3>/dev/tty

for key in $pckeys # $^modifiers$^pckeys $sunkeys $^modifiers$^sunkeys
do
    print -u3 -Rn "Press $key: "
    seq="$(getseq)" || return 1
    print "key[$key]='${(q)seq}'"
    print -u3 -R $seq
done >> $zkbd/$TERM.tmp

source $zkbd/$TERM.tmp || return 1
if [[ "${key[Delete]}" == "${key[Backspace]}" ]]
then
    print
    print Warning: Backspace and Delete key both send "${(q)key[Delete]}"
else
    if [[ "${key[Delete]}" != "^?" ]]
    then
	print
        print Warning: Delete key sends "${(q)key[Delete]}" '(not ^?)'
    fi
    if [[ "${key[Backspace]}" != "^H" ]]
    then
	print
        print Warning: Backspace sends "${(q)key[Backspace]}"
    fi
fi

local termID=${${DISPLAY:t}:-$VENDOR-$OSTYPE} termFile=$zkbd/$TERM.tmp
command mv $termFile $zkbd/$TERM-$termID && termFile=$zkbd/$TERM-$termID

cat <<EOF

Parameter assignments for the keys you typed have been written to the file:
$termFile

You may read this file into ${ZDOTDIR:-$HOME}/.zshrc or another startup
file with the "source" or "." commands, then reference the \$key parameter
in bindkey commands, for example like this:

    source ${(D)zkbd}/\$TERM-\${\${DISPLAY:t}:-\$VENDOR-\$OSTYPE}
    [[ -n \${key[Left]} ]] && bindkey "\${key[Left]}" backward-char
    [[ -n \${key[Right]} ]] && bindkey "\${key[Right]}" forward-char
    # etc.

Adjust the name of the file being sourced, as necessary.
EOF
