# Catch an exception.  Returns 0 if the exception in question was caught.
# The first argument gives the exception to catch, which may be a
# pattern.
# This must be within an always-block.  A typical set of handlers looks
# like:
#   {
#     # try block; something here throws exceptions
#   } always {
#      if catch MyExcept; then
#         # Handler code goes here.
#         print Handling exception MyExcept
#      elif catch *; then
#         # This is the way to implement a catch-all.
#         print Handling any other exception
#      fi
#   }
# As with other languages, exceptions do not need to be handled
# within an always block and may propagate to a handler further up the
# call chain.
#
# It is possible to throw an exception from within the handler by
# using "throw".
#
# The shell variable $CAUGHT is set to the last exception caught,
# which is useful if the argument to "catch" was a pattern.
#
# Use "function" keyword in case catch is already an alias.
function catch {
  if [[ $TRY_BLOCK_ERROR -gt 0 && $EXCEPTION = ${~1} ]]; then
    (( TRY_BLOCK_ERROR = 0 ))
    typeset -g CAUGHT="$EXCEPTION"
    unset EXCEPTION
    return 0
  fi

  return 1
}
# Never use globbing with "catch".
alias catch="noglob catch"

catch "$@"
