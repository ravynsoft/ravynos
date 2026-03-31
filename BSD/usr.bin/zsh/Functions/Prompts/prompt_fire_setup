# Fire prompt theme from bashprompt
# Inspired by Raster (Carsten Haitzler of Red Hat Advanced Development Labs)
# Created by BadlandZ
# Changed by Spidey 08/06
# Converted to zsh prompt theme by <adam@spiers.net>

prompt_fire_help () {
  cat <<EOH
This prompt is color-scheme-able.  You can invoke it thus:

  prompt fire [<fire1> [<fire2> [<fire3> [<userhost> [<date> [<cwd>]]]]]]

where the parameters are the three fire colors, and the colors for the
user@host text, date text, and current working directory respectively.
The default colors are yellow, yellow, red, white, white, and yellow.
This theme works best with a dark background.

Recommended fonts for this theme: either UTF-8, or nexus or vga or similar.
If you don't have any of these, the 8-bit characters will probably look
stupid.
EOH
}

prompt_fire_setup () {
  local fire1=${1:-'yellow'}
  local fire2=${2:-'yellow'}
  local fire3=${3:-'red'}
  local userhost=${4:-'white'}
  local date=${5:-'white'}
  local cwd=${6:-'yellow'}

  local -a schars
  autoload -Uz prompt_special_chars
  prompt_special_chars

  local GRAD1="%{$schars[333]$schars[262]$schars[261]$schars[260]%}"
  local GRAD2="%{$schars[260]$schars[261]$schars[262]$schars[333]%}"
  local COLOR1="%B%F{$fire1}%K{$fire2}"
  local COLOR2="%B%F{$userhost}%K{$fire2}"
  local COLOR3="%b%F{$fire3}%K{$fire2}"
  local COLOR4="%b%F{$fire3}%K{black}"
  local COLOR5="%B%F{$cwd}%K{black}"
  local COLOR6="%B%F{$date}%K{black}"
  local GRAD0="%b%f%k"

  PS1=$COLOR1$GRAD1$COLOR2'%n@%m'$COLOR3$GRAD2$COLOR4$GRAD1$COLOR6' %D{%a %b %d} %D{%I:%M:%S%P} '$prompt_newline$COLOR5'%~/'$GRAD0' '
  PS2=$COLOR1$GRAD1$COLOR3$GRAD2$COLOR4$GRAD1$COLOR5'>'$GRAD0' '

  prompt_opts=(cr subst percent)
}

prompt_fire_preview () {
  if (( ! $#* )); then
    prompt_preview_theme fire
    print
    prompt_preview_theme fire red magenta blue white white white 
  else
    prompt_preview_theme fire "$@"
  fi
}

prompt_fire_setup "$@"
