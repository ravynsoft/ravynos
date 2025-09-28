# Functions to make it easier to type URLs as command line arguments.  As
# you type, the input character is analyzed and, if it may need quoting,
# the current word is checked for a URI scheme.  If one is found and the
# current word is not already in quotes, a backslash is inserted before
# the input character.

# Setup:
#       autoload -Uz url-quote-magic
#       zle -N self-insert url-quote-magic

# As of zsh-5.1, the following may also be necessary in order to apply
# quoting to copy-pasted URLs:
#       autoload -Uz bracketed-paste-magic
#       zle -N bracketed-paste bracketed-paste-magic
# See also backward-extend-paste in bracketed-paste-magic source file.

# A number of zstyles may be set to control the quoting behavior.
#
# url-metas
#  This style is looked up in the context :url-quote-magic:SCHEME (where
#  SCHEME is that of the current URL, e.g. "ftp").  The value is a string
#  listing the characters to be treated as globbing metacharacters when
#  appearing in a URL using that scheme.  The default is to quote all zsh
#  extended globbing characters, excluding '<' and '>' but including
#  braces (as in brace expansion).  See also url-seps.
#
# url-seps
#  Like url-metas, but lists characters that should be considered command
#  separators, redirections, history references, etc.  The default is to
#  quote the standard set of shell separators, excluding those that
#  overlap with the extended globbing characters, but including '<' and
#  '>' and the first character of $histchars.
#
# url-globbers
#  This style is looked up in the context :url-quote-magic.  The values
#  form a list of command names that are expected to do their own globbing
#  on the URL string.  This implies that they are aliased to use the
#  "noglob" modifier.  When the first word on the line matches one of the
#  values AND the URL refers to a local file (see url-local-schema), only
#  the url-seps characters are quoted; the url-metas are left alone,
#  allowing them to affect command-line parsing, completion, etc.  The
#  default values are a literal "noglob" plus (when the zsh/parameter
#  module is available) any commands aliased to the helper function
#  "urlglobber" or its alias "globurl".
#
# url-local-schema
#  This style is always looked up in the context :urlglobber, even though
#  it is used by both url-quote-magic and urlglobber.  The values form
#  a list of URI schema that should be treated as referring to local files
#  by their real local path names, as opposed to files which are specified
#  relative to a web-server-defined document root.  The defaults are "ftp"
#  and "file".
#
# url-other-schema
#  Like url-local-schema, but lists all other URI schema upon which
#  urlglobber and url-quote-magic should act.  If the URI on the
#  command line does not have a scheme appearing either in this list or in
#  url-local-schema, it is not magically quoted.  The default values are
#  "http", "https", and "ftp".  When a scheme appears both here and in
#  url-local-schema, it is quoted differently depending on whether the
#  command name appears in url-globbers.

# TODO:
#	Add a style for "document root" for globbing local URLs.
#	Turn this on at colon, and off again at space or accept.
#       Use compsys for nested quoting analysis and command parsing.

# Establish default values for styles, but only if not already set
local -a reply match mbegin mend

zstyle -m ':url-quote-magic:\*' url-metas '*' ||
    zstyle ':url-quote-magic:*' url-metas '*?[]^(|)~#{}='

zstyle -m ':url-quote-magic:\*' url-seps '*' ||
    zstyle -e ':url-quote-magic:*' url-seps 'reply=(";&<>${histchars[1]}")'

zstyle -m :url-quote-magic url-globbers '*' ||
    zstyle -e :url-quote-magic url-globbers \
	'zmodload -i zsh/parameter;
	 reply=( noglob
		 ${(k)galiases[(R)(* |)(noglob|urlglobber|globurl) *]:-}
		 ${(k)aliases[(R)(* |)(noglob|urlglobber|globurl) *]:-} )'

zstyle -m ':urlglobber' url-local-schema '*' ||
    zstyle ':urlglobber' url-local-schema ftp file

zstyle -m ':urlglobber' url-other-schema '*' ||
    zstyle ':urlglobber' url-other-schema http https ftp

# Define the "urlglobber" helper function and shorthand "globurl" alias

function urlglobber {
    local -a args globbed localschema otherschema reply
    local arg command="$1"
    shift
    zstyle -s :urlglobber url-local-schema localschema '|'
    zstyle -s :urlglobber url-other-schema otherschema '|'
    for arg
    do
	case "${arg}" in
	((${~localschema}):/(|/localhost)/*)
	    globbed=( ${~${arg##ftp://(localhost|)}} )
	    args[$#args+1]=( "${(M)arg##(${~localchema})://(localhost|)}${(@)^globbed}" )
	    ;;
	((${~otherschema}):*) args[${#args}+1]="$arg";;
	(*) args[${#args}+1]=( ${~arg} );;
	esac
    done
    "$command" "${(@)args}"
}
alias globurl='noglob urlglobber '

# Finally, define (and execute if necessary) the function we really want

function url-quote-magic {
    setopt localoptions noksharrays extendedglob
    local qkey="${(q)KEYS}"
    local -a reply match mbegin mend
    if [[ "$KEYS" != "$qkey" ]]
    then
	local lbuf="$LBUFFER$qkey"
	if [[ "${(Q)LBUFFER}$KEYS" == "${(Q)lbuf}" ]]
	then
	    local -a words
	    words=("${(@Q)${(z)lbuf}}")
	    local urlseps urlmetas urlglobbers localschema otherschema
	    if [[ "$words[-1]" == (#b)([^:]##):* ]]
	    then
		zstyle -s ":url-quote-magic:$match[1]" url-seps urlseps ''
		zstyle -s ":url-quote-magic:$match[1]" url-metas urlmetas ''
	    fi
	    zstyle -s :url-quote-magic url-globbers urlglobbers '|'
	    zstyle -s :urlglobber url-other-schema otherschema '|'
	    if [[ "$words[1]" == ${~urlglobbers} ]]
	    then
		zstyle -s :urlglobber url-local-schema localschema '|'
	    else
		localschema=' '
	    fi
	    case "$words[-1]" in
	    (*[\'\"]*) ;;
	    ((${~localschema}):/(|/localhost)/*)
		[[ "$urlseps" == *"$KEYS"* ]] &&
		    LBUFFER="$LBUFFER\\" ;;
	    ((${~otherschema}):*)
		[[ "$urlseps$urlmetas" == *"$KEYS"* ]] &&
		    LBUFFER="$LBUFFER\\" ;;
	    esac
	fi
    fi
    zle .self-insert
}

# Handle zsh autoloading conventions

[[ -o kshautoload ]] || url-quote-magic "$@"
