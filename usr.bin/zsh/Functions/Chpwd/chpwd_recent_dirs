# Save the current directory in the stack of most recently used directories.

emulate -L zsh
setopt extendedglob

local -aU reply
integer changed

autoload -Uz chpwd_recent_filehandler chpwd_recent_add

# Don't save if this is not interactive, or is in a subshell.
# Don't save if this is not being called directly from the top level
# of the shell via a straightforward sequence of shell functions.
# So this is called
#  - on any straightforward cd or pushd (including AUTO_CD)
#  - any time cd or pushd is called from a function invoked directly
#    or indirectly from the command line, e.g. if pushd is a function
#    fixing the order of directories that got broken years ago
# but it is not called any time
#  - the shell is not interactive
#  - we forked
#  - we are being eval'd, including for some special purpose such
#    as a style
#  - we are not called from the top-level command loop, for example
#    we are in a completion function (which is called from zle
#    when the main top-level command interpreter isn't running)
#  - obviously, when cd -q is in use (that's what the option is for).
#
# For compatibility with older shells, skip this test if $ZSH_EVAL_CONTEXT
# isn't set.  This will never be the case inside a shell function when
# the variable is implemented.
if [[ ! -o interactive  || $ZSH_SUBSHELL -ne 0 || \
  ( -n $ZSH_EVAL_CONTEXT && \
  $ZSH_EVAL_CONTEXT != toplevel(:[a-z]#func|)# ) ]]; then
  return
fi

chpwd_recent_filehandler

if [[ $reply[1] != $PWD ]]; then
  chpwd_recent_add $PWD && changed=1

  (( changed )) && chpwd_recent_filehandler $reply
fi
