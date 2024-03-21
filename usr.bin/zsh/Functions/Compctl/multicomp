# multicomp() {
# Completes all manner of files given prefixes for each path segment.
# e.g. s/z/s -> src/zsh-2.4/src
#
# Usage: e.g.
# compctl -D -f + -U -Q -S '' -K multicomp
#
# Will expand glob patterns already in the word, but use complete-word,
# not TAB (expand-or-complete), or you will get ordinary glob expansion.
# Requires the -U option to compctl.
# Menucompletion is highly recommended for ambiguous matches.
# Liable to screw up escaped metacharacters royally.
# $fignore is not used: feel free to add your own bit.

emulate -R zsh				# Requires zsh 3.0-pre4 or later
local pref head sofar origtop newtop globdir="(-/)" wild
setopt localoptions nullglob rcexpandparam globdots
unsetopt markdirs globsubst shwordsplit nounset

pref="${1}$2"
# Hack to allow programmable completion to select multicomp after a :
# (e.g.
# compctl -D -f -x 's[:]' -U -Q -S '' -K multicomp
# )
pref="${pref#:}"

sofar=('')
reply=('')

if [[ "$pref" = \~* ]]; then
  # If the string started with ~, save the head and what it will become.
  origtop="${pref%%/*}"
  eval "newtop=$origtop"
  # Save the expansion as the bit matched already
  sofar=($newtop)
  pref="${pref#$origtop}"
fi

while [[ -n "$pref" ]]; do
  [[ "$pref" = /* ]] && sofar=(${sofar}/) && pref="${pref#/}"
  head="${pref%%/*}"
  pref="${pref#$head}"
  [[ -z "$pref" ]] && globdir=
  # if path segment contains wildcards, don't add another.
  if [[ "$head" = *[\[\(\*\?\$\~]* ]]; then
    wild=$head
  else
    # Simulate case-insensitive globbing for ASCII characters
    wild="[${(j(][))${(s())head:l}}]*"	# :gs/a/[a]/ etc.
    # The following could all be one expansion, but for readability:
    wild=$wild:gs/a/aA/:gs/b/bB/:gs/c/cC/:gs/d/dD/:gs/e/eE/:gs/f/fF/
    wild=$wild:gs/g/gG/:gs/h/hH/:gs/i/iI/:gs/j/jJ/:gs/k/kK/:gs/l/lL/
    wild=$wild:gs/m/mM/:gs/n/nN/:gs/o/oO/:gs/p/pP/:gs/q/qQ/:gs/r/rR/
    wild=$wild:gs/s/sS/:gs/t/tT/:gs/u/uU/:gs/v/vV/:gs/w/wW/:gs/x/xX/
    wild=$wild:gs/y/yY/:gs/z/zZ/:gs/-/_/:gs/_/-_/:gs/[]//

    # Expand on both sides of '.' (except when leading) as for '/'
    wild="${${wild:gs/[.]/*.*/}#\*}"
  fi

  reply=(${sofar}"${wild}${globdir}")
  reply=(${~reply})

  [[ -z $reply[1] ]] && reply=() && break
  [[ -n $pref ]] && sofar=($reply)
done

# Restore ~'s in front if there were any.
# There had better not be anything funny in $newtop.
[[ -n "$origtop" ]] && reply=("$origtop"${reply#$newtop})

# }
