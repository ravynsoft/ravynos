# adam2 prompt theme

prompt_adam2_help () {
  cat <<'EOF'
This prompt is color-scheme-able.  You can invoke it thus:

  prompt adam2 [ 8bit ] [<color1> [<color2> [<color3>] [<color4>]]

where the colors are for the hyphens, current directory, user@host,
and user input bits respectively.  The default colors are cyan, green,
cyan, and white.  This theme works best with a dark background.

If you have either UTF-8 or the `nexus' or `vga' console fonts or similar,
you can specify the `8bit' option to use 8-bit replacements for the
7-bit characters.

And you probably thought adam1 was overkill ...
EOF
}

prompt_adam2_setup () {
  # Some can't be local
  setopt localoptions nowarncreateglobal
  local prompt_gfx_tlc prompt_gfx_mlc prompt_gfx_blc

  if [[ $1 == '8bit' ]]; then
    shift
    if [[ ${LC_ALL:-${LC_CTYPE:-$LANG}} = *UTF-8* ]]; then
      prompt_gfx_tlc=$'\xe2\x94\x8c'
      prompt_gfx_mlc=$'\xe2\x94\x9c'
      prompt_gfx_blc=$'\xe2\x94\x94'
      prompt_gfx_hyphen=$'\xe2\x94\x80'
    else
      prompt_gfx_tlc=$'\xda'
      prompt_gfx_mlc=$'\xc3'
      prompt_gfx_blc=$'\xc0'
      prompt_gfx_hyphen=$'\xc4'
    fi
  else
    prompt_gfx_tlc='.'
    prompt_gfx_mlc='|'
    prompt_gfx_blc='\`'
    prompt_gfx_hyphen='-'
  fi

  # Colour scheme
  prompt_adam2_color1=${1:-'cyan'}    # hyphens
  prompt_adam2_color2=${2:-'green'}   # current directory
  prompt_adam2_color3=${3:-'cyan'}    # user@host
  prompt_adam2_color4=${4:-'white'}   # user input

  local prompt_gfx_bbox 
  prompt_gfx_tbox="%B%F{$prompt_adam2_color1}${prompt_gfx_tlc}%b%F{$prompt_adam2_color1}${prompt_gfx_hyphen}"
  prompt_gfx_bbox="%B%F{$prompt_adam2_color1}${prompt_gfx_blc}${prompt_gfx_hyphen}%b%F{$prompt_adam2_color1}"

  # This is a cute hack.  Well I like it, anyway.
  prompt_gfx_bbox_to_mbox=$'%{\e[A\r'"%}%B%F{$prompt_adam2_color1}${prompt_gfx_mlc}%b%F{$prompt_adam2_color1}${prompt_gfx_hyphen}%{"$'\e[B%}'

  prompt_l_paren="%B%F{black}("
  prompt_r_paren="%B%F{black})"

  prompt_user_host="%b%F{$prompt_adam2_color3}%n%B%F{$prompt_adam2_color3}@%b%F{$prompt_adam2_color3}%m"

  prompt_line_1a="$prompt_gfx_tbox$prompt_l_paren%B%F{$prompt_adam2_color2}%~$prompt_r_paren%b%F{$prompt_adam2_color1}"
  prompt_line_1b="$prompt_l_paren$prompt_user_host$prompt_r_paren%b%F{$prompt_adam2_color1}${prompt_gfx_hyphen}"

  prompt_line_2="$prompt_gfx_bbox${prompt_gfx_hyphen}%B%F{white}"

  prompt_char="%(!.#.>)"

  prompt_opts=(cr subst percent)

  add-zsh-hook precmd prompt_adam2_precmd
}

prompt_adam2_precmd() {
  setopt localoptions extendedglob noxtrace nowarncreateglobal
  local prompt_line_1

  prompt_adam2_choose_prompt

  PS1="$prompt_line_1$prompt_newline$prompt_line_2%B%F{white}$prompt_char %b%f%k"
  PS2="$prompt_line_2$prompt_gfx_bbox_to_mbox%B%F{white}%_> %b%f%k"
  PS3="$prompt_line_2$prompt_gfx_bbox_to_mbox%B%F{white}?# %b%f%k"
  zle_highlight[(r)default:*]="default:fg=$prompt_adam2_color4,bold"
}

prompt_adam2_choose_prompt () {
  local prompt_line_1a_width=${#${(S%%)prompt_line_1a//(\%([KF1]|)\{*\}|\%[Bbkf])}}
  local prompt_line_1b_width=${#${(S%%)prompt_line_1b//(\%([KF1]|)\{*\}|\%[Bbkf])}}

  local prompt_padding_size=$(( COLUMNS
                                  - prompt_line_1a_width
                                  - prompt_line_1b_width ))

  # Try to fit in long path and user@host.
  if (( prompt_padding_size > 0 )); then
    local prompt_padding
    eval "prompt_padding=\${(l:${prompt_padding_size}::${prompt_gfx_hyphen}:)_empty_zz}"
    prompt_line_1="$prompt_line_1a$prompt_padding$prompt_line_1b"
    return
  fi

  prompt_padding_size=$(( COLUMNS - prompt_line_1a_width ))

  # Didn't fit; try to fit in just long path.
  if (( prompt_padding_size > 0 )); then
    local prompt_padding
    eval "prompt_padding=\${(l:${prompt_padding_size}::${prompt_gfx_hyphen}:)_empty_zz}"
    prompt_line_1="$prompt_line_1a$prompt_padding"
    return
  fi

  # Still didn't fit; truncate 
  local prompt_pwd_size=$(( COLUMNS - 5 ))
  prompt_line_1="$prompt_gfx_tbox$prompt_l_paren%B%F{$prompt_adam2_color2}%$prompt_pwd_size<...<%~%<<$prompt_r_paren%b%F{$prompt_adam2_color1}$prompt_gfx_hyphen"
}

prompt_adam2_setup "$@"
