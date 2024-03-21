# Match words by the style given below.  The matching depends on the
# cursor position.  The matched_words array is set to the matched portions
# separately.  These look like:
#    <stuff-at-start> <word-before-cursor> <whitespace-before-cursor>
#    <whitespace-after-cursor> <word-after-cursor> <whitespace-after-word>
#    <stuff-at-end>
# where the cursor position is always after the third item and `after'
# is to be interpreted as `after or on'.
#
# matched_words may be an associative array, in which case the
# values above are now given by the elements named start, word-before-cursor,
# ws-before-cursor, ws-after-cursor, word-after-cursor, ws-after-word,
# end.  In addition, the element is-word-start is 1 if the cursor
# is on the start of a word; this is non-trivial in the case of subword
# (camel case) matching as there may be no white space to test.
#
# Some of the array elements will be empty; this depends on the style.
# For example
#    foo bar  rod stick
#            ^
# with the cursor where indicated will with typical settings produce the
# elements `foo ', `bar', ` ', ` ', `rod', ` ' and `stick'.
#
# The style word-style can be set to indicate what a word is.
# The three possibilities are:
#
#  shell	Words are shell words, i.e. elements of a command line.
#  whitespace	Words are space delimited words; only space or tab characters
#               are considered to terminated a word.
#  normal       (the default): the usual zle logic is applied, with all
#		alphanumeric characters plus any characters in $WORDCHARS
#		considered parts of a word.  The style word-chars overrides
#		the parameter.  (Any currently undefined value will be
#		treated as `normal', but this should not be relied upon.)
#  specified    Similar to normal, except that only the words given
#               in the string (and not also alphanumeric characters)
#               are to be considered parts of words.
#  unspecified  The negation of `specified': the characters given
#               are those that aren't to be considered parts of a word.
#               They should probably include white space.
#
# In the case of the `normal' or `(un)specified', more control on the
# behaviour can be obtained by setting the style `word-chars' for the
# current context.  The value is used to override $WORDCHARS locally.
# Hence,
#   zstyle ':zle:transpose-words*' word-style normal
#   zstyle ':zle:transpose-words*' word-chars ''
# will force bash-style word recognition, i.e only alphanumeric characters
# are considered parts of a word.  It is up to the function which calls
# match-words-by-style to set the context in the variable curcontext,
# else a default context will be used (not recommended).
#
# You can override the use of word-chars with the style word-class.
# This specifies the same information, but as a character class.
# The surrounding square brackets shouldn't be given, but anything
# which can appear inside is allowed.  For example,
#   zstyle ':zle:*' word-class '-:[:alnum:]'
# is valid.  Note the usual care with `]' , `^' and `-' must be taken if
# they need to appear as individual characters rather than for grouping.
#
# The final style is `skip-chars'.  This is an integer; that many
# characters counting the one under the cursor will be treated as
# whitespace regardless and added to the front of the fourth element of
# matched_words.  The default is zero, i.e. the character under the cursor
# will appear in <whitespace-after-cursor> if it is whitespace, else in
# <word-after-cursor>.  This style is mostly useful for forcing
# transposition to ignore the current character.
#
# The values of the styles can be overridden by options to the function:
#  -w <word-style>
#  -s <skip-chars>
#  -c <word-class>
#  -C <word-chars>

emulate -L zsh
setopt extendedglob

local wordstyle spacepat wordpat1 wordpat2 opt charskip wordchars wordclass
local match mbegin mend pat1 pat2 word1 word2 ws1 ws2 ws3 skip
local nwords MATCH MBEGIN MEND subwordrange

local curcontext=${curcontext:-:zle:match-words-by-style}

autoload -Uz match-word-context
match-word-context

while getopts "w:s:c:C:r:" opt; do
  case $opt in
    (w)
    wordstyle=$OPTARG
    ;;

    (s)
    skip=$OPTARG
    ;;

    (c)
    wordclass=$OPTARG
    ;;

    (C)
    wordchars=$OPTARG
    ;;

    (r)
    subwordrange=$OPTARG
    ;;

    (*)
    return 1
    ;;
  esac
done

[[ -z $wordstyle ]] && zstyle -s $curcontext word-style wordstyle
[[ -z $skip ]] && zstyle -s $curcontext skip-chars skip
[[ -z $skip ]] && skip=0

case $wordstyle in
  (*shell*) local bufwords
	  # This splits the line into words as the shell understands them.
	  bufwords=(${(Z:n:)LBUFFER})
	  nwords=${#bufwords}
	  wordpat1="${(q)bufwords[-1]}"

	  # Take substring of RBUFFER to skip over $skip characters
	  # from the cursor position.
	  bufwords=(${(Z:n:)RBUFFER[1+$skip,-1]})
	  wordpat2="${(q)bufwords[1]}"
	  spacepat='[[:space:]]#'

	  # Assume the words are at the top level, i.e. if we are inside
	  # 'something with spaces' then we need to ignore the embedded
	  # spaces and consider the whole word.
	  bufwords=(${(Z:n:)BUFFER})
	  if (( ${#bufwords[$nwords]} > ${#wordpat1} )); then
	    # Yes, we're in the middle of a shell word.
	    # Find out what's in front.
	    eval pat1='${LBUFFER%%(#b)('${wordpat1}')('${spacepat}')}'
	    # Now everything from ${#pat1}+1 is wordy
	    wordpat1=${LBUFFER[${#pat1}+1,-1]}
	    wordpat2=${RBUFFER[1,${#bufwords[$nwords]}-${#wordpat1}+1]}

	    wordpat1=${(q)wordpat1}
	    wordpat2=${(q)wordpat2}
	  fi
	  ;;
  (*space*) spacepat='[[:space:]]#'
           wordpat1='[^[:space:]]##'
	   wordpat2=$wordpat1
	   ;;
  (*) local wc
      # See if there is a character class.
      wc=$wordclass
      if [[ -n $wc ]] || zstyle -s $curcontext word-class wc; then
	# Treat as a character class: do minimal quoting.
	wc=${wc//(#m)[\'\"\`\$\(\)\^]/\\$MATCH}
      else
	# See if there is a local version of $WORDCHARS.
	wc=$wordchars
	if [[ -z $wc ]]; then
	  zstyle -s $curcontext word-chars wc ||
	  wc=$WORDCHARS
	fi
	if [[ $wc = (#b)(?*)-(*) ]]; then
	  # We need to bring any `-' to the front to avoid confusing
	  # character classes... we get away with `]' since in zsh
          # this isn't a pattern character if it's quoted.
	  wc=-$match[1]$match[2]
	fi
	wc="${(q)wc}"
      fi
      # Quote $wc where necessary, because we don't want those
      # characters to be considered as pattern characters later on.
      if [[ $wordstyle = *specified* ]]; then
        if [[ $wordstyle != *unspecified* ]]; then
	  # The given set of characters are the word characters, nothing else
	  wordpat1="[${wc}]##"
	  # anything else is a space.
	  spacepat="[^${wc}]#"
	else
	  # The other way round.
	  wordpat1="[^${wc}]##"
	  spacepat="[${wc}]#"
    	fi
      else
        # Normal: similar, but add alphanumerics.
	wordpat1="[${wc}[:alnum:]]##"
	spacepat="[^${wc}[:alnum:]]#"
      fi
      wordpat2=$wordpat1
      ;;
esac

# The eval makes any special characters in the parameters active.
# In particular, we need the surrounding `[' s to be `real'.
# This is why we quoted the wordpats in the `shell' option, where
# they have to be treated as literal strings at this point.
match=()
eval pat1='${LBUFFER%%(#b)('${wordpat1}')('${spacepat}')}'
word1=$match[1]
ws1=$match[2]

if [[ $wordstyle = *subword* ]]; then
  if [[ -z $subwordrange ]] &&
    ! zstyle -s $curcontext subword-range subwordrange; then
    subwordrange='[:upper:]'
  fi
  # The rule here is that a word boundary may be an upper case letter
  # followed by a lower case letter, or an upper case letter at
  # the start of a group of upper case letters.  To make
  # it easier to be consistent, we just use anything that
  # isn't an upper case character instead of a lower case
  # character.
  # Here the initial "*" will match greedily, so we get the
  # last such match, as we want.
  integer epos
  if [[ $word1 = (#b)(*)([${~subwordrange}][^${~subwordrange}]*) ]]; then
    (( epos = ${#match[1]} ))
  fi
  if [[ $word1 = (#b)(*[^${~subwordrange}])([${~subwordrange}]*) ]]; then
    (( ${#match[1]} > epos ))  &&  (( epos = ${#match[1]} ))
  fi
  if (( epos > 0 )); then
    pat1+=$word1[1,epos]
    word1=$word1[epos+1,-1]
  fi
fi

match=()
charskip=${(l:skip::?:)}

eval pat2='${RBUFFER##(#b)('${charskip}${spacepat}')('\
${wordpat2}')('${spacepat}')}'
if [[ -n $match[2] ]]; then
  ws2=$match[1]
  word2=$match[2]
  ws3=$match[3]
else
  # No more words, so anything left is white space after cursor.
  ws2=$RBUFFER
  pat2=
fi

integer wordstart
[[ ( -n $ws1 || -n $ws2 ) && -n $word2 ]] && wordstart=1
if [[ $wordstyle = *subword* ]]; then
  # Do we have a group of upper case characters at the start
  # of word2 (that don't form the entire word)?
  # Again, rely on greedy matching of first pattern.
  if [[ $word2 = (#b)([${~subwordrange}][${~subwordrange}]##)(*) &&
	  -n $match[2] ]]; then
    # Yes, so the last one is new word boundary.
    (( epos = ${#match[1]} - 1 ))
    # Otherwise, are we in the middle of a word?
    # In other, er, words, we've got something on the left with no
    # white space following and something that doesn't start a word here.
  elif [[ -n $word1 && -z $ws1 && -z $ws2 && \
    $word2 = (#b)([^${~subwordrange}]##)* ]]; then
    (( epos = ${#match[1]} ))
    # Otherwise, do we have upper followed by non-upper not
    # at the start?  Ignore the initial character, we already
    # know it's a word boundary so it can be an upper case character
    # if it wants.
  elif [[ $word2 = (#b)(?[^${~subwordrange}]##)[${~subwordrange}]* ]]; then
    (( epos = ${#match[1]} ))
    (( wordstart = 1 ))
  else
    (( epos = 0 ))
  fi
  if (( epos )); then
    # Careful: if we matched a subword there's no whitespace immediately
    # after the matched word, so ws3 should be empty and any existing
    # value tacked onto pat2.
    pat2="${word2[epos+1,-1]}$ws3$pat2"
    ws3=
    word2=$word2[1,epos]
  fi
fi

# matched_words should be local to caller.
# Just fix type here.
if [[ ${(t)matched_words} = *association* ]]; then
  matched_words=(
    start              "$pat1"
    word-before-cursor "$word1"
    ws-before-cursor   "$ws1"
    ws-after-cursor    "$ws2"
    word-after-cursor  "$word2"
    ws-after-word      "$ws3"
    end                "$pat2"
    is-word-start      $wordstart
  )
else
  matched_words=("$pat1" "$word1" "$ws1" "$ws2" "$word2" "$ws3" "$pat2")
fi
