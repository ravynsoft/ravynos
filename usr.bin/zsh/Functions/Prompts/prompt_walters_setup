prompt_walters_help () {
  cat <<'EOF'
This prompt is color-scheme-able.  You can invoke it thus:

  prompt walters [<color1>]

where the color is for the right-hand prompt.

This prompt was stolen from Colin Walters <walters@debian.org>,
who gives credit to Michel Daenzer <daenzer@debian.org>.
EOF
}

prompt_walters_setup () {

if [[ "$TERM" != "dumb" ]]; then
    PS1='%B%(?..[%?] )%b%n@%U%m%u> '
    RPS1="%F{${1:-green}}%~%f"
else
    PS1="%(?..[%?] )%n@%m:%~> "
fi

  prompt_opts=(cr percent)
}

prompt_walters_setup "$@"
