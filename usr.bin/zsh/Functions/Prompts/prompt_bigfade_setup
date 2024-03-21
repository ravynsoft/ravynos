# Generic large colour fade-bar prompt theme from bashprompt
# Created by James Manning <jmm@raleigh.ibm.com>
# Changed by Spidey 08/06
# Converted to zsh prompt theme by <adam@spiers.net>

prompt_bigfade_help () {
  cat <<EOH
This prompt is color-scheme-able.  You can invoke it thus:

  prompt bigfade [<fade-bar> [<userhost> [<date> [<cwd>]]]]

where the parameters are the colors for the fade-bar, user@host text,
date text, and current working directory respectively.  The default
colors are blue, white, white, and yellow.  This theme works best with
a dark background.


Recommended fonts for this theme: either UTF-8, or nexus or vga or similar.
If you don't have any of these, the 8-bit characters will probably look
stupid.
EOH
}

prompt_bigfade_setup () {
  local fadebar=${1:-'blue'}
  local userhost=${2:-'white'}
  local date=${3:-'white'}
  local cwd=${4:-'yellow'}

  local -A schars
  autoload -Uz prompt_special_chars
  prompt_special_chars

  PS1="%B%F{$fadebar}$schars[333]$schars[262]$schars[261]$schars[260]%B%F{$userhost}%K{$fadebar}%n@%m%b%k%f%F{$fadebar}%K{black}$schars[260]$schars[261]$schars[262]$schars[333]%b%f%k%F{$fadebar}%K{black}$schars[333]$schars[262]$schars[261]$schars[260]%B%F{$date}%K{black} %D{%a %b %d} %D{%I:%M:%S%P}$prompt_newline%B%F{$cwd}%K{black}%d>%b%f%k "
  PS2="%B%F{$fadebar}$schars[333]$schars[262]$schars[261]$schars[260]%b%F{$fadebar}%K{black}$schars[260]$schars[261]$schars[262]$schars[333]%F{$fadebar}%K{black}$schars[333]$schars[262]$schars[261]$schars[260]%B%F{$fadebar}>%b%f%k "

  prompt_opts=(cr subst percent)
}

prompt_bigfade_preview () {
  if (( ! $#* )); then
    prompt_preview_theme bigfade
    print
    prompt_preview_theme bigfade red white grey white
  else
    prompt_preview_theme bigfade "$@"
  fi
}

prompt_bigfade_setup "$@"
