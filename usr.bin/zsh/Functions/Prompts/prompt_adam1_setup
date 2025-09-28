# adam1 prompt theme

prompt_adam1_help () {
  cat <<'EOF'
This prompt is color-scheme-able.  You can invoke it thus:

  prompt adam1 [<color1> [<color2> [<color3>]]]

where the colors are for the user@host background, current working
directory, and current working directory if the prompt is split over
two lines respectively.  The default colors are blue, cyan and green.
This theme works best with a dark background.

Recommended fonts for this theme: nexus or vga or similar.  If you
don't have any of these, then specify the `plain' option to use 7-bit
replacements for the 8-bit characters.
EOF
}

prompt_adam1_setup () {
  setopt localoptions nowarncreateglobal
  prompt_adam1_color1=${1:-'blue'}
  prompt_adam1_color2=${2:-'cyan'}
  prompt_adam1_color3=${3:-'green'}

  base_prompt="%K{$prompt_adam1_color1}%n@%m%k "
  post_prompt="%b%f%k"

  setopt localoptions extendedglob
  base_prompt_no_color="${base_prompt//(%K{[^\\\}]#\}|%k)/}"
  post_prompt_no_color="${post_prompt//(%K{[^\\\}]#\}|%k)/}"

  add-zsh-hook precmd prompt_adam1_precmd
}

prompt_adam1_precmd () {
  setopt localoptions noxtrace nowarncreateglobal
  local base_prompt_expanded_no_color base_prompt_etc
  local prompt_length space_left

  base_prompt_expanded_no_color=$(print -P "$base_prompt_no_color")
  base_prompt_etc=$(print -P "$base_prompt%(4~|...|)%3~")
  prompt_length=${#base_prompt_etc}
  if [[ $prompt_length -lt 40 ]]; then
    path_prompt="%B%F{$prompt_adam1_color2}%(4~|...|)%3~%F{white}"
  else
    space_left=$(( $COLUMNS - $#base_prompt_expanded_no_color - 2 ))
    path_prompt="%B%F{$prompt_adam1_color3}%${space_left}<...<%~$prompt_newline%F{white}"
  fi
  PS1="$base_prompt$path_prompt %# $post_prompt"
  PS2="$base_prompt$path_prompt %_> $post_prompt"
  PS3="$base_prompt$path_prompt ?# $post_prompt"
}

prompt_adam1_setup "$@"
