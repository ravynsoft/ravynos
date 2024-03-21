# pws prompt theme

prompt_pws_help() {
    cat <<'EOF'
Simple prompt which tries to display only the information you need.
- highlighted parenthesised status if last command had non-zero status
- bold + if shell is not at top level (may need tweaking if there
  is another shell in the process history of your terminal)
- number of background jobs in square brackets if non-zero
- time in yellow on black, with Ding! on the hour.
I usually use this in a white on black terminal.
EOF
}

prompt_pws_setup() {
      PS1='%K{white}%F{red}%(?..(%?%))'\
'%K{black}%F{white}%B%(2L.+.)%(1j.[%j].)'\
'%F{yellow}%(t.Ding!.%D{%L:%M})'\
'%f%k%b%# '
}

prompt_pws_setup "$@"
