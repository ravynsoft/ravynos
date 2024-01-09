##
# The "keeper" function suite originally appeared in several zsh-users
# posts in the fall of 2004.  It was published in summary form in the
# Shell Corner column on UnixReview.com in January 2005 at the URL
# <http://www.unixreview.com/documents/s=9513/ur0501a/ur0501a.htm>
#
# Article still available on the Wayback Machine:
# <http://web.archive.org/web/20050207041146/http://www.unixreview.com/documents/s=9513/ur0501a/ur0501a.htm>
#
# A few minor edits have been made to those functions for this file.  Key
# bindings are commented out to avoid clashes with any existing bindings.
##

declare -a kept

# The "keep" function accepts a set of file patterns as the positional
# parameters or a series of lines (expected to represent file names) on
# standard input.  It stores the expansion of those patterns, or the input
# lines, in the global variable $kept, and then displays the result
# formatted in columns, similar to an "ls" listing.  Its alias, also named
# "keep", prevents the file patterns from being expanded when the command
# line is executed; they're expanded in the assignment to $kept instead,
# so that the local settings of nonomatch etc. are applied.

function keep {
    setopt localoptions nomarkdirs nonomatch nocshnullglob nullglob
    setopt noksharrays noshwordsplit
    kept=($~*)
    if [[ ! -t 0 ]]; then
        local line
        while read -r line; do
            kept+=( $line )
        done
    fi
    print -Rc - ${^kept%/}(T)
}
alias keep='noglob keep'

# The function "_insert_kept" copies the value of $kept to the cursor
# position.  If a prefix of a name is immediately to the left of the
# cursor, then only the subset of $kept that matches that prefix is
# copied, as is usual for completion.  The examples bind it to two
# different widgets, "insert-kept-result" and "expand-kept-result".  If
# invoked via the "expand-kept-result" widget, it replaces a pattern on
# the command line with the matching words from the $kept array.

_insert_kept() {
    (( $#kept )) || return 1
    local action
    zstyle -s :completion:$curcontext insert-kept action
    if [[ -n $action ]]
    then compstate[insert]=$action
    elif [[ $WIDGET = *expand* ]]
    then compstate[insert]=all
    fi
    if [[ $WIDGET = *expand* ]]
    then compadd -U ${(M)kept:#${~words[CURRENT]}}
    else compadd -a kept
    fi
}

zle -C insert-kept-result complete-word _generic
zstyle ':completion:insert-kept-result:*' completer _insert_kept 
# bindkey '^Xk' insert-kept-result

zle -C expand-kept-result complete-word _generic
zstyle ':completion:expand-kept-result:*' completer _insert_kept
# bindkey '^XK' expand-kept-result

# The "_expand_word_and_keep" function stores the expansions computed by
# the "_expand" completer in the global $kept for later retrieval by
# "_insert_kept".

_expand_word_and_keep() {
    {
        function compadd {
            local -A args
            zparseopts -E -A args J:
            if [[ $args[-J] == all-expansions ]]
            then
                builtin compadd -A kept "$@"
                kept=( ${(Q)${(z)kept}} )
            fi
            builtin compadd "$@"
        }
        _expand_word
    } always {
        unfunction compadd
    }
}

zle -C _expand_word complete-word _expand_word_and_keep

# This style is required to segregate the all-expansions group for
# purposes of _expand_word_and_keep.
zstyle ':completion:expand-word:expand:::all-expansions' group-name ''
