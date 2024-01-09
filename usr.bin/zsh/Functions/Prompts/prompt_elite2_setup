# Created by icetrey <trey@imagin.net>
# Added by Spidey 08/06
# Converted to zsh prompt theme by <adam@spiers.net>

prompt_elite2_help () {
  cat <<EOH
This prompt is color-scheme-able.  You can invoke it thus:

  prompt elite2 [<text-color> [<parentheses-color>]]

The default colors are both cyan.  This theme works best with a dark
background.

Recommended fonts for this theme: either UTF-8, or nexus or vga or similar.
If you don't have any of these, the 8-bit characters will probably look
stupid.
EOH
}

prompt_elite2_setup () {
  local text_col=${1:-'cyan'}
  local parens_col=${2:-$text_col}

  local -A schars
  autoload -Uz prompt_special_chars
  prompt_special_chars

  local text="%b%F{$text_col}"
  local parens="%B%F{$parens_col}"
  local punct="%B%F{black}"
  local reset="%b%f"

  local lpar="$parens($text"
  local rpar="$parens)$text"

  PS1="$punct$schars[332]$text$schars[304]$lpar%n$punct@$text%m$rpar$schars[304]$lpar%!$punct/$text%y$rpar$schars[304]$lpar%D{%I:%M%P}$punct:$text%D{%m/%d/%y}$rpar$schars[304]$punct-$reset$prompt_newline$punct$schars[300]$text$schars[304]$lpar%#$punct:$text%~$rpar$schars[304]$punct-$reset " 

  PS2="$parens$schars[304]$text$schars[304]$punct-$reset "

  prompt_opts=(cr subst percent)
}

prompt_elite2_preview () {
  local color colors
  colors=(red yellow green blue magenta)

  if (( ! $#* )); then
    for (( i = 1; i <= $#colors; i++ )); do
      color=$colors[$i]
      prompt_preview_theme elite2 $color
      (( i < $#colors )) && print
    done
  else
    prompt_preview_theme elite2 "$@"
  fi
}

prompt_elite2_setup "$@"
