zmodload -i zsh/parameter || return 1

prompt_bart_help () {
    setopt localoptions nocshnullcmd noshnullcmd
    [[ $ZSH_VERSION < 4.2.2 ]] &&
	print 'Requires ZSH_VERSION 4.2.2'$'\n'
    <<-\EOF
	This prompt gives the effect of left and right prompts on the upper
	line of a two-line prompt.  It also illustrates including history
	text in the prompt.  The lower line is initialized from the last
	(or only) line of whatever prompt was previously in effect.

	    prompt bart [on|off] [color...]

	You may provide up to five colors, though only three colors (red,
	blue, and the default foreground) are used if no arguments are
	given.  The defaults look best on a light background.

	No background colors or hardwired cursor motion escapes are used,
	and it is not necessary to setopt promptsubst.

	The "off" token temporarily disables the theme; "on" restores it.
	Note, this does NOT fully reset to the original prompt state, it
	only hides/reveals the extra lines above the command line and
	removes	the supporting hooks.
	EOF
    [[ $(read -sek 1 "?${(%):-%S[press return]%s}") == [Qq] ]] &&
	print -nP '\r%E' && return
    <<-\EOF

	The "upper left prompt" looks like:
	    machine [last command] /current/working/dir
	The first three color arguments apply to these components.  The
	last command is shown in standout mode if the exit status was
	nonzero, or underlined if the job was stopped.

	If the last command is too wide to fit without truncating the
	current directory, it is put on a middle line by itself.  The
	current	directory uses %~, so namedir abbreviation applies.

	The "upper right prompt" looks like:
	    date time
	The fourth color is used for the date, and the first again for the
	time.  As with RPS1, first the date and then the time disappear as
	the upper left prompt grows too wide.  The clock is not live; it
	changes only after each command, as does the rest of the prompt.
	EOF
    [[ $(read -sek 1 "?${(%):-%S[press return]%s}") == [Qq] ]] &&
	print -nP '\r%E' && return
    <<-\EOF

	When RPS1 (RPROMPT) is set before this prompt is selected and a
	fifth color is specified, that color is turned on before RPS1 is
	displayed and reset after it.  Other color changes within RPS1, if
	any, remain in effect.  This also applies to RPS2 (RPROMPT2).
	If a fifth color is specified and there is no RPS2, PS2 (PROMPT2)
	is colored and moved to RPS2.  Changes to RPS1/RPS2 are currently
	not reverted when the theme is switched off.  These work best with
	the TRANSIENT_RPROMPT option, which must be set separately.

	This theme hijacks psvar[7] through [9] to avoid having to reset
	the entire PS1 string on every command.  It uses TRAPWINCH to set
	the position of the upper right prompt on a window resize, so the
	prompt may not match the window width until the next command.

	When setopt nopromptcr is in effect, an ANSI terminal protocol
	exchange is attempted in order to determine the current cursor
	column, and the width of the upper prompt is adjusted accordingly.
	If your terminal is not ANSI compliant, this may cause unexpected
	behavior, and in any case it may consume typeahead.  (Recommended
	workaround is to setopt promptcr.)
	EOF
    [[ $(read -sek 1 "?${(%):-%S[done]%s}") == $'\n' ]] ||
	print -nP '\n%E'
}

integer -g PSCOL=1
typeset -g PSCMD=

prompt_bart_preexec () {
    setopt localoptions noxtrace noshwordsplit noksharrays unset
    local -a cmd; cmd=( ${(z)3} )
    if [[ $cmd[1] = fg ]]
    then
	shift cmd
	cmd[1]=${cmd[1]:-%+}
    fi
    if [[ $#cmd -eq 1 && $cmd[1] = %* ]]
    then
	PSCMD=$jobtexts[$cmd[1]]
    elif [[ -o autoresume && -n $jobtexts[%?$2] ]]
    then
	PSCMD=$jobtexts[%?$2]
    else
	# Use history text to avoid alias expansion
	PSCMD=$history[$HISTCMD]
    fi
    return 0
}

prompt_bart_precmd () {
    setopt localoptions noxtrace noksharrays unset
    local zero='%([BSUbfksu]|[FB]{*})' escape colno lineno

    : "${PSCMD:=$history[$[HISTCMD-1]]}"	# Default to history text

    # Using psvar here protects against unwanted promptsubst expansions.

    psvar[7]="$PSCMD"
    psvar[8]=''		# No padding until we compute it
    psvar[9]=()

    # Reset the truncation widths for upcoming computations
    ((PSCOL == 1)) || { PSCOL=1 ; prompt_bart_ps1 }
    if [[ -o promptcr ]]
    then
	# Emulate the 4.3.x promptsp option if it isn't available
	eval '[[ -o promptsp ]] 2>/dev/null' ||
	    print -nP "${(l.COLUMNS.. .)}\e[s${(pl.COLUMNS..\b.)}%E\e[u" >$TTY
    else IFS='[;' read -s -d R escape\?$'\e[6n' lineno PSCOL <$TTY
    fi
    ((PSCOL == 1)) || prompt_bart_ps1
    ((colno = COLUMNS-PSCOL))

    # Compute the size of the upper left prompt and set psvar[9] if needed.
    ((${#${(f)${(%%)${(S)PS1//$~zero/}}}[1]} > colno)) && psvar[9]=''

    # Compute and set the padding between upper left and right prompts.
    (((colno -= ${#${(f)${(%%)${(S)PS1//$~zero/}}}[1]}) > 0)) &&
	psvar[8]=${(l:colno:: :)}
}

prompt_bart_ps1 () {
    setopt localoptions noxtrace noksharrays

    local -ah ps1
    local -h host hist1 hist2 dir space date time rs="%b%f%k"
    local -h eon="%(?.[.%20(?.[%U.%S[))" eoff="%(?.].%20(?.%u].]%s))"

    # Set up the components of the upper line
    host="%{$fg[%m]%}%m$rs"
    hist1="%9(v. . %{$fg[%h]%}$eon%7v$eoff$rs )"
    hist2=$'%9(v.\n'"%{$fg[%h]%}$eon%7v$eoff$rs.)"
    dir="%{$fg[%~]%}%8~$rs"
    space=%8v
    date="%{$fg[%D]%}%D$rs"	# Prefer "%{$fg[%D]%}%W$rs" in the USA?
    time="%{$fg[%@]%}%@$rs"

    # This is just a tad silly ...
    [[ $prompt_theme[1] = oliver ]] && PS1='[%h%0(?..:%?)]%# ' ||
	PS1=${PS1//$prompt_newline/$'\n'}

    # Assemble the new prompt
    ps1=( ${(f)PS1} )		# Split the existing prompt at newlines
    ps1=(
	"%$[COLUMNS-PSCOL]>..>"	# Begin truncation (upper left prompt)
	"$host"
	"$hist1"		# Empty when too wide for one line
	"$dir"
	"%<<"			# End truncation (end upper left prompt)
	"$space"		# Pad line to upper right position
	"%$[COLUMNS-PSCOL-15](l. . $date)"
	"%$[COLUMNS-PSCOL-7](l.. $time)"
	"$hist2"		# Empty when $hist1 is not empty
	$'\n'
	$ps1[-1]		# Keep last line of the existing prompt
	)
    PS1="${(j::)ps1}"
}

prompt_bart_winch () {
    setopt localoptions nolocaltraps noksharrays unset

    # Delete ourself from TRAPWINCH if not using our precmd insert.
    [[ $precmd_functions = *prompt_bart_precmd* ]] && prompt_bart_ps1 ||
	functions[TRAPWINCH]="${functions[TRAPWINCH]//prompt_bart_winch}"
}

prompt_bart_setup () {
    setopt localoptions nolocaltraps noksharrays unset
    typeset -gA fg

    # A few extra niceties ...
    repeat 1; do case "$1:l" in
      (off|disable)
	add-zsh-hook -D precmd "prompt_*_precmd"
	add-zsh-hook -D preexec "prompt_*_preexec"
	functions[TRAPWINCH]="${functions[TRAPWINCH]//prompt_bart_winch}"
	[[ $prompt_theme[1] = bart ]] && PS1=${${(f)PS1}[-1]}
	return 1	# Prevent change of $prompt_theme
	;;
      (on|enable)
	shift
	[[ $prompt_theme[1] = bart ]] && break
	;&
      (*)
	# Use the fg assoc to hold our selected colors ...
	# This used to be provided by the function colors, but is now
	# set directly from here.  There should be no clash if both
	# are in use.
	fg[%m]="%F{${1:-red}}"
	fg[%h]="%F{${2:-blue}}"
	fg[%~]="%F{${3:-default}}"
	fg[%D]="%F{${4:-default}}"
	fg[%@]="%F{${1:-red}}"
	;;
    esac; done

    prompt_bart_ps1

    if (($# > 4))
    then
	# No RPS1 by default because prompt_off_setup doesn't fix it.
	if (($#RPS1))
	then
	    RPS1="%F{$5}$RPS1%f"
	fi
	# RPS2 is less obvious so don't mind that it's not restored.
	if (($#RPS2))
	then
	    RPS2="%F{$5}$RPS2%f"
	else
	    RPS2="%F{$5}<${${PS2//\%_/%^}%> }%f"
	    PS2=''
	fi
    fi
    # Paste our special commands into precmd and TRAPWINCH
    
    add-zsh-hook precmd prompt_bart_precmd
    add-zsh-hook preexec prompt_bart_preexec
    prompt_cleanup \
        'functions[TRAPWINCH]="${functions[TRAPWINCH]//prompt_bart_winch}"'
    functions[TRAPWINCH]="${functions[TRAPWINCH]//prompt_bart_winch}
	prompt_bart_winch"

    return 0
}

prompt_bart_preview () {
    local +h PS1='%# '
    prompt_preview_theme bart "$@"
}

[[ -o kshautoload ]] || prompt_bart_setup "$@"
