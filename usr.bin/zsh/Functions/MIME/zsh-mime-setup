emulate -L zsh
setopt extendedglob cbases

local opt o_verbose o_list i

autoload -Uz zsh-mime-handler

while getopts "flv" opt; do
  case $opt in
    # List: show existing suffixes and their handlers then exit.
    (l)
    o_list=1
    ;;

    # Verbose; print diagnostics to stdout.
    (v)
    o_verbose=1
    ;;

    # Force; discard any existing settings before reading.
    (f)
    unset -m zsh_mime_\*
    ;;

    (*)
    [[ $opt = \? ]] || print -r "Option $opt not handled, complain" >&2
    return 1
    ;;
  esac
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))


if [[ -n $o_list ]]; then
  # List and return.  Remember that suffixes may be overridden by styles.
  # However, we require some sort of standard handler to be present,
  # so we don't need to search styles for suffixes that aren't
  # handled.  Yet.
  local list_word
  local -a handlers
  if (( $# )); then
    handlers=(${(k)zsh_mime_handlers[(I)${(j.|.)*}]})
  else
    handlers=(${(k)zsh_mime_handlers})
  fi
  for suffix in ${(o)handlers}; do
      zstyle -s ":mime:.$suffix:" handler list_word ||
        list_word=${zsh_mime_handlers[$suffix]}
      print ${(r.10.)suffix}$list_word
      zstyle -s ":mime:.$suffix:" flags list_word ||
        list_word=${zsh_mime_flags[$suffix]}
      if [[ -n $list_word ]]; then
	print "  flags: $list_word"
      fi
  done
  return 0
fi


# Handler for each suffix.
(( ${+zsh_mime_handlers} )) || typeset -gA zsh_mime_handlers
# Corresponding flags, if any, for handler
(( ${+zsh_mime_flags} )) || typeset -gA zsh_mime_flags

# Internal maps read from MIME configuration files.
# Note we don't remember the types, just the mappings from suffixes
# to handlers and their flags.
typeset -A suffix_type_map type_handler_map type_flags_map

local -a default_type_files default_cap_files
local -a type_files cap_files array match mbegin mend
local file line type suffix exts elt flags line2
integer ind

default_type_files=(~/.mime.types /etc/mime.types)
default_cap_files=(~/.mailcap /etc/mailcap)

# Customizable list of files to examine.
if zstyle -a :mime: mime-types type_files; then
  while (( (ind = ${type_files[(I)+]}) > 0 )); do
    type_files[$ind]=($default_type_files)
  done
else
  type_files=($default_type_files)
fi

if zstyle -a :mime: mailcap cap_files; then
  while (( (ind = ${cap_files[(I)+]}) > 0 )); do
    cap_files[$ind]=($default_cap_files)
  done
else
  cap_files=($default_cap_files)
fi

{
  mime-setup-add-type() {
    local type suffix
    local -a array

    type=$1
    shift

    while (( $# )); do
	# `.ps' instead of `ps' has been noted
	suffix=${1##.}
	shift

	if [[ -z $suffix_type_map[$suffix] ]]; then
	    [[ -n $o_verbose ]] && 
	    print -r "Adding type $type for $suffix" >&2
	    suffix_type_map[$suffix]=$type
	else
	    # Skip duplicates.
	    array=(${=suffix_type_map[$suffix]})
	    if [[ ${array[(I)$type]} -eq 0 ]]; then
		[[ -n $o_verbose ]] &&
		print -r "Appending type $type for already defined $suffix" >&2
		suffix_type_map[$suffix]+=" $type"
	    fi
	fi
    done
  }

  # Loop through files to find suffixes for MIME types.
  # Earlier entries take precedence, so the files need to be listed
  # with the user's own first.  This also means pre-existing
  # values in suffix_type_map are respected.
  for file in $type_files; do
    [[ -r $file ]] || continue

    # For once we rely on the fact that read handles continuation
    # lines ending in backslashes, i.e. there's no -r.
    while read line; do
      # Skip blank or comment lines.
      [[ $line = [[:space:]]#(\#*|) ]] && continue

      # There are two types of line you find in MIME type files.
      # The original simple sort contains the type name then suffixes
      # separated by whitespace.  However, Netscape insists
      # on adding lines with backslash continuation with
      # key="value" pairs.  So we'd better handle both.
      if [[ $line = *=* ]]; then
        # Gory.
        # This relies on the fact that a typical entry:
        #   type=video/x-mpeg2 desc="MPEG2 Video" exts="mpv2,mp2v"
        # looks like a parameter assignment.  However, we really
        # don't want to be screwed up by future extensions,
        # so we split the elements to an array and pick out the
        # ones we're interested in.
        type= exts=

        # Syntactically split line to preserve quoted words.
        array=(${(z)line})
        for elt in $array; do
          if [[ $elt = (type|exts)=* ]]; then
            eval $elt
          fi
        done

        # Get extensions by splitting on comma
        array=(${(s.,.)exts})

        [[ -n $type ]] && mime-setup-add-type $type $array
      else
        # Simple.
        mime-setup-add-type ${=line}
      fi
    done <$file
  done
} always {
  unfunction mime-setup-add-type >&/dev/null
}

local -a pats_prio o_prios
local o_overwrite sentinel
typeset -A type_prio_flags_map type_prio_src_map type_prio_mprio_map
integer src_id prio mprio

# A list of keywords indicating the methods used to break ties amongst multiple
# entries. The following keywords are accepted:
# files: The order of files read: Entries from files read earlier are preferred
#   (The default value of the variable is a list with this keyword alone)
# priority: The priority flag is matched in the entry. Can be a value from 0 to
# 9. The default priority is 5. Higher priorities are preferred.
# flags: See the mailcap-prio-flags option
# place: Always overrides. Useful for specifying that entries read later are
# preferred.
#
# As the program reads mailcap entries, if it encounters a duplicate
# entry, each of the keywords in the list are checked to see if the new
# entry can override the existing entry. If none of the keywords are able
# to decide whether the new entry should be preferred to the older one, the
# new entry is discarded.
zstyle -a :mime: mailcap-priorities o_prios || o_prios=(files)

# This style is used as an argument for the flags test in mailcap-priorities.
# This is a list of patterns, each of which is tested against the flags for the
# mailcap entry. An match with a pattern ahead in the list is preferred as
# opposed to a match later in the list. An unmatched item is least preferred.
zstyle -a :mime: mailcap-prio-flags pats_prio

# Loop through files to find handlers for types.
((src_id = 0))
for file in $cap_files; do
  [[ -r $file ]] || continue

  ((src_id = src_id + 1))
  # Oh, great.  We need to preserve backslashes inside the line,
  # but need to manage continuation lines.
  while read -r line; do
    # Skip blank or comment lines.
    [[ $line = [[:space:]]#(\#*|) ]] && continue

    while [[ $line = (#b)(*)\\ ]]; do
      line=$match[1]
      read -r line2 || break
      line+=$line2
    done

    # Guess what, this file has a completely different format.
    # See mailcap(4).
    # The biggest unpleasantness here is that the fields are
    # delimited by semicolons, but the command field, which
    # is the one we want to extract, may itself contain backslashed
    # semicolons.
    if [[ $line = (#b)[[:space:]]#([^[:space:]\;]##)[[:space:]]#\;(*) ]]
    then
      # this is the only form we can handle, but there's no point
      # issuing a warning for other forms.
      type=$match[1]
      line=$match[2]
      # See if it has flags after the command.
      if [[ $line = (#b)(([^\;\\]|\\\;|\\[^\;])#)\;(*) ]]; then
        line=$match[1]
        flags=$match[3]
      else
        flags=
      fi
      # Remove quotes from semicolons
      line=${line//\\\;/\;}
      # and remove any surrounding white space --- this might
      # make the handler empty.
      line=${${line##[[:space:]]#}%%[[:space:]]}

      ((prio = 0))
      for i in $pats_prio; do
	  # print -r "Comparing $i with '$flags'" >&2
	[[ $flags = ${~i} ]] && break
	  # print -r "Comparison failed" >&2
	((prio = prio + 1))
      done
      ((mprio=5))
      [[ $flags = (#b)*priority=([0-9])* ]] && mprio=$match[1]
      sentinel=no
      if [[ -n $type_handler_map[$type] ]]; then
	for i in $o_prios; do
	  case $i in
	    (files)
	    if [[ $src_id -lt $type_prio_src_map[$type] ]]; then
	      sentinel=yes; break
	    elif [[ $src_id -gt $type_prio_src_map[$type] ]]; then
	      sentinel=no; break
	    fi
	    ;;
	    (priority)
	    if [[ $mprio -gt $type_prio_mprio_map[$type] ]]; then
	      sentinel=yes; break
	    elif [[ $mprio -lt $type_prio_mprio_map[$type] ]]; then
	      sentinel=no; break
	    fi
	    ;;
	    (flags)
	    if [[ $prio -lt $type_prio_flags_map[$type] ]]; then
	      sentinel=yes; break
	    elif [[ $prio -gt $type_prio_flags_map[$type] ]]; then
	      sentinel=no; break
	    fi
	    ;;
	    (place)
	    sentinel=yes
	    break
	    ;;
	  esac
	done
      else
	sentinel=yes
      fi

      if [[ $sentinel = yes ]]; then
	if [[ -n $o_verbose ]]; then
	  if [[ -n $type_handler_map[$type] ]]; then
	    print -r "Overriding" >&2
	  else
	    print -r "Adding" >&2
	  fi
	  print -r " handler for type $type:" >&2
	  print -r "  $line" >&2
	fi
	type_handler_map[$type]=$line
	type_flags_map[$type]=$flags
	type_prio_src_map[$type]=$src_id
	type_prio_flags_map[$type]=$prio
	type_prio_mprio_map[$type]=$mprio
	if [[ -n $flags && -n $o_verbose ]]; then
	  print -r "  with flags $flags" >&2
	fi
      elif [[ -n $o_verbose ]]; then
	print -r "Skipping handler for already defined type $type:" >&2
	print -r "  $line" >&2
	if [[ -n $flags ]]; then
	  print -r " with flags $flags" >&2
	fi
      fi
    fi
  done <$file
done


# Check for styles which override whatever is in the file.
# We need to make sure there is a handler set up; for some
# uses we may need to defer checking styles until zsh-mime-handler.
# How much we need to do here is a moot point.
zstyle -L | while read line; do
  array=(${(Q)${(z)line}})
  if [[ $array[3] = (handler|flags) && \
        $array[2] = (#b):mime:.([^:]##):(*) ]]; then
    suffix=$match[1]
    # Make sure there is a suffix alias set up for this.
    alias -s $suffix >&/dev/null || alias -s $suffix=zsh-mime-handler
    # Also for upper case variant
    alias -s ${(U)suffix} >&/dev/null || alias -s ${(U)suffix}=zsh-mime-handler
  fi
done

# Now associate the suffixes directly with handlers.
# We just look for the first one with a handler.
# If there is no handler, we don't bother registering an alias
# for the suffix.

for suffix line in ${(kv)suffix_type_map}; do
  # Skip if we already have a handler.
  [[ -n $zsh_mime_handlers[$suffix] ]] && continue

  # Split the space-separated list of types.
  array=(${=line})

  # Find the first type with a handler.
  line2=
  for type in $array; do
    line2=${type_handler_map[$type]}
    [[ -n $line2 ]] && break
  done

  # See if there is a generic type/* handler.
  # TODO: do we need to consider other forms of wildcard?
  if [[ -z $line2 ]]; then
    for type in $array; do
      type="${type%%/*}/*"
      line2=${type_handler_map[$type]}
      [[ -n $line2 ]] && break
    done
  fi

  if [[ -n $line2 ]]; then
    # Found a type with a handler.
    # Install the zsh handler as an alias, but never override
    # existing suffix handling.
    alias -s $suffix >&/dev/null || alias -s $suffix=zsh-mime-handler
    alias -s ${(U)suffix} >&/dev/null || alias -s ${(U)suffix}=zsh-mime-handler

    zsh_mime_handlers[$suffix]=$line2
    zsh_mime_flags[$suffix]=$type_flags_map[$type]
  fi
done

true
