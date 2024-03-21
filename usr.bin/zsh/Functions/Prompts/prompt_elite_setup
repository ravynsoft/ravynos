# Created by KrON from windowmaker on IRC
# Changed by Spidey 08/06
# Converted to zsh prompt theme by <adam@spiers.net>

prompt_elite_help () {
  cat <<EOH
This prompt is color-scheme-able.  You can invoke it thus:

  prompt elite [<text-color> [<punctuation-color>]]

The default colors are red and blue respectively.  This theme is
intended for use with a black background.

Recommended fonts for this theme: either UTF-8, or nexus or vga or similar.
If you don't have any of these, the 8-bit characters will probably look
stupid.
EOH
}

prompt_elite_setup () {
  local text=${1:-'red'}
  local punctuation=${2:-'blue'}

  local -A schars
  autoload -Uz prompt_special_chars
  prompt_special_chars

  PS1="%F{$text}$schars[332]$schars[304]%F{$punctuation}(%F{$text}%n%F{$punctuation}@%F{$text}%m%F{$punctuation})%F{$text}-%F{$punctuation}(%F{$text}%D{%I:%M%P}%F{$punctuation}-:-%F{$text}%D{%m}%F{$punctuation}%F{$text}/%D{%d}%F{$punctuation})%F{$text}$schars[304]-%F{$punctuation}$schars[371]%F{$text}-$schars[371]$schars[371]%F{$punctuation}$schars[372]$prompt_newline%F{$text}$schars[300]$schars[304]%F{$punctuation}(%F{$text}%1~%F{$punctuation})%F{$text}$schars[304]$schars[371]%F{$punctuation}$schars[372]%f"
  PS2="> "

  prompt_opts=(cr subst percent)
}

prompt_elite_preview () {
  if (( ! $#* )); then
    prompt_preview_theme elite
    print
    prompt_preview_theme elite green yellow
  else
    prompt_preview_theme elite "$@"
  fi
}

prompt_elite_setup "$@"
