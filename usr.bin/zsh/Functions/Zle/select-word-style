emulate -L zsh
setopt extendedglob

local -a word_functions

word_functions=(backward-kill-word backward-word
  capitalize-word down-case-word
  forward-word kill-word
  transpose-words up-case-word)

[[ -z $1 ]] && autoload -Uz read-from-minibuffer

local REPLY detail f wordstyle teststyle

if ! zle -l $word_functions[1]; then
  for f in $word_functions; do
    autoload -Uz $f-match
    zle -N $f $f-match
  done
fi


while true; do
  if [[ -n $WIDGET && -z $1 ]]; then
    read-from-minibuffer -k1 "Word styles (hit return for more detail):
(b)ash (n)ormal (s)hell (w)hitespace (d)efault (q)uit
(B), (N), (S), (W) as above with subword matching
${detail}? " || return 1
  else
    REPLY=$1
  fi

  detail=

  case $REPLY in
    ([bB]*)
    # bash style
    wordstyle=standard
    zstyle ':zle:*' word-chars ''
    zstyle ':zle:*' skip-whitespace-first true
    ;;

    ([nN]*)
    # normal zsh style
    wordstyle=standard
    zstyle ':zle:*' word-chars "$WORDCHARS"
    zstyle ':zle:*' skip-whitespace-first false
    ;;

    ([sS]*)
    # shell command arguments or special tokens
    wordstyle=shell
    zstyle ':zle:*' skip-whitespace-first false
    ;;

    ([wW]*)
    # whitespace-delimited
    wordstyle=space
    zstyle ':zle:*' skip-whitespace-first false
    ;;

    (d*)
    # default: could also return widgets to builtins here
    wordstyle=
    zstyle -d ':zle:*' word-chars
    zstyle -d ':zle:*' skip-whitespace-first
    ;;

    (q*)
    # quit without setting
    return 1
    ;;

    (*)
    detail="\
(b)ash:       Word characters are alphanumerics only
(n)ormal:     Word characters are alphanumerics plus \$WORDCHARS
(s)hell:      Words are command arguments using shell syntax
(w)hitespace: Words are whitespace-delimited
(d)efault:    Use default, no special handling (usually same as \`n')
(q)uit:       Quit without setting a new style
"
    if [[ -z $WIDGET || -n $1 ]]; then
      print "Usage: $0 word-style
where word-style is one of the characters in parentheses:
$detail" >&2
      return 1
    fi
    continue
    ;;
  esac

  if [[ -n $wordstyle ]]; then
    if [[ $REPLY = [[:upper:]]* ]]; then
      wordstyle+=-subword
    fi
    zstyle ':zle:*' word-style $wordstyle
  fi
  return
done
