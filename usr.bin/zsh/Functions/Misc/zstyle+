# This makes defining styles a bit simpler by using a single `+' as a
# special token that allows one to append a context name to the
# previously used context name. Like this:
#
#   zstyle+ ':foo:bar' style1 value1 \
#         + ':baz'     style2 value2 \
#         + ':frob'    style3 value3
#
# This defines style1 with value1 for the context :foo:bar as usual.
# But it also defines styles2 with value2 for the context :foo:bar:baz
# and style3 with value3 for :foo:bar:frob.
# Of course, any of the sub-contexts after the plus signs may be 
# empty strings to re-use the previous context unchanged.
#
# If you don't want to change all your calls to `zstyle' to use
# `zstyle+' you can use an alias `alias zstyle=zstyle+' and make sure
# the completion functions are autoloaded without alias expansion (the
# -U option to the autoload builtin). The completion system normally
# loads its functions with without alias expansion.

case "$1" in
-*) zstyle "$@";;

*)  setopt localoptions noksharrays
    integer i
    local context="$1"
    1=''
    for ((i=2; $#; ++i)); do
      if [[ $i -gt $# || "$argv[i]" == '+' ]]; then
        zstyle "$context${(@)argv[1,i-1]}"
        shift "i > $# ? $# : i"  # Stupid shift error on i > $#
  	i=1
      fi
    done;;
esac
