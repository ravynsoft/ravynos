function prompt_zefram_precmd {
	local exitstatus=$?
	setopt localoptions noxtrace noksharrays
	psvar=(SIG)
	[[ $exitstatus -gt 128 ]] && psvar[1]=SIG$signals[$exitstatus-127]
	[[ $psvar[1] = SIG ]] && psvar[1]=$exitstatus
	jobs % >/dev/null 2>&1 && psvar[2]=
}

function prompt_zefram_setup {
  PS1='[%(2L.%L/.)'$ZSH_VERSION']%(?..%B{%v}%b)%n%(2v.%B@%b.@)%m:%B%~%b%(!.#.>) '
  PS2='%(4_:... :)%3_> '

  prompt_opts=( cr subst percent )

  add-zsh-hook precmd prompt_zefram_precmd
}

prompt_zefram_setup "$@"
