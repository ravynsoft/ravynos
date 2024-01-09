# Add `autoload promptnl' to your .zshrc, and include a call to promptnl
# near the end of your precmd function.
#
# When promptnl runs, it asks the terminal to send back the current
# position of the cursor.  If the cursor is in column 1, it does nothing;
# otherwise it prints a newline.  Thus you get a newline exactly when one
# is needed.
#
# Of course this can make it appear that `print -n' and friends have
# failed to suppress the final newline; so promptnl outputs the value
# of the EOLMARK parameter before the newline, with prompt sequences
# expanded.  So you can for example use EOLMARK='%B!%b' to put a bold
# exclamation point at the end of the actual output.

# There's another way to accomplish the equivalent, without reading the
# cursor position from the terminal.  Skip to the end of the file to see
# that other way.

emulate -L zsh

# VT100 and ANSI terminals will report the cursor position when sent
# the sequence ESC [ 6 n -- it comes back as ESC [ column ; line R
# with of course no trailing newline.  Column and line are 1-based.

local RECV='' SEND='\e[6n' REPLY=X

# If you are on a very slow tty, you may need to increase WAIT here.
integer WAIT=1

# Make sure there's no typeahead, or it'll confuse things.  Remove
# this block entirely to use this function in 3.0.x at your own risk.
while read -t -k 1
do
    RECV=$RECV$REPLY
done
if [[ -n $RECV ]]
then
    print -z -r -- $RECV
    RECV=''
    REPLY=X
fi

# This is annoying, but zsh immediately resets it properly, so ...
stty -echo

# Output the SEND sequence and read back into RECV.  In case this is
# not a terminal that understands SEND, do a non-blocking read and
# retry for at most WAIT seconds before giving up.  Requires 3.1.9.
# For 3.0.x, remove "-t" but don't call this on the wrong terminal!

print -n $SEND

integer N=$SECONDS
while [[ $REPLY != R ]] && ((SECONDS - N <= WAIT))
do
    if read -t -k 1
    then
	((N=SECONDS))
	RECV=$RECV$REPLY
    fi
done

# If the cursor is not in the first column, emit EOLMARK and newline.

(( ${${${RECV#*\;}%R}:-0} > 1 )) && print -P -- $EOLMARK

return 0

# OK, now here's the other way.  Works on any auto-margin terminal, which
# includes most terminals that respond to ESC [ 6 n as far as I know.  It
# prints a line of spaces exactly as wide as the terminal, then prints a
# carriage return.  If there are any characters already on the line, this
# will cause the line to wrap, otherwise it won't.

: setopt nopromptcr
: PS1="%{${(pl:COLUMNS+1:: ::\r:)}%}$PS1"

# On a very slow connection, you might be able to see the spaces getting
# drawn and then overwritten, so reading the cursor position might work
# better in that circumstance because it transmits fewer characters.  It
# also doesn't work if you resize the terminal.

# To get the EOLMARK behavior, simply adjust the COLUMNS+1 expression to
# account for the width of the mark, and include it.  For example:

: setopt nopromptcr
: PS1="%{%S<EOL>%s${(pl:COLUMNS-4:: ::\r:)}%}$PS1"

# The important bit is that the total width of the string inside %{...%}
# has to be COLUMNS+1, where the extra character is the \r.  However, I
# recommend using a one-character EOLMARK to avoid having the line wrap
# in the middle of the marker string:

setopt nopromptcr
PS1="%{%S#%s${(pl:COLUMNS:: ::\r:)}%}$PS1"
