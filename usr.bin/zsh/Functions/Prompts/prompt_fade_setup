# Generic colour fade-bar prompt theme from bashprompt
# Created by Jim Foltz <aa204@acorn.net>
# Changed by Spidey 08/06
# Converted to zsh prompt theme by <adam@spiers.net>

prompt_fade_help () {
  cat <<EOH
This prompt is color-scheme-able.  You can invoke it thus:

  prompt fade [<fade-bar-and-cwd> [<userhost> [<date>]]] 

where the parameters are the colors for the fade-bar and current
working directory, user@host text, and date text respectively.  The
default colors are green, white, and white.  This theme works best
with a dark background.

Recommended fonts for this theme: either UTF-8, or nexus or vga or similar.
If you don't have any of these, the 8-bit characters will probably look
stupid.
EOH
}

# emacs shell-script mode gets confused with ' in heredoc above

prompt_fade_setup () {
  local fadebar_cwd=${1:-'green'}
  local userhost=${2:-'white'}
  local date=${3:-'white'}

  local -A schars
  autoload -Uz prompt_special_chars
  prompt_special_chars

  PS1="%F{$fadebar_cwd}%B%K{$fadebar_cwd}$schars[333]$schars[262]$schars[261]$schars[260]%F{$userhost}%K{$fadebar_cwd}%B%n@%m%b%F{$fadebar_cwd}%K{black}$schars[333]$schars[262]$schars[261]$schars[260]%F{$date}%K{black}%B %D{%a %b %d} %D{%I:%M:%S%P} $prompt_newline%F{$fadebar_cwd}%K{black}%B%~/%b%k%f "
  PS2="%F{$fadebar_cwd}%K{black}$schars[333]$schars[262]$schars[261]$schars[260]%f%k>"

  prompt_opts=(cr subst percent)
}

prompt_fade_preview () {
  local color colors
  colors=(red yellow green blue magenta)

  if (( ! $#* )); then
    for (( i = 1; i <= $#colors; i++ )); do
      color=$colors[$i]
      prompt_preview_theme fade $color
      print
    done
    prompt_preview_theme fade white grey blue
  else
    prompt_preview_theme fade "$@"
  fi
}

prompt_fade_setup "$@"
