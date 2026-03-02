# Throw an exception.
# The first argument is a string giving the exception.  Other arguments
# are ignored.
#
# This is designed to be called somewhere inside a "try-block", i.e.
# some code of the form:
#   {
#     # try-block
#   } always {
#     # always-block
#   }
# although as normal with exceptions it might be hidden deep inside
# other code.  Note, however, that it must be code running within the
# current shell; with shells, unlike other languages, it is quite easy
# to miss points at which the shell forks.
#
# If there is nothing to catch an exception, this behaves like any
# other shell error, aborting to the command prompt or abandoning a
# script.

# The following must not be local.
typeset -g EXCEPTION="$1"
readonly THROW
if (( TRY_BLOCK_ERROR == 0 )); then
  # We are throwing an exception from the middle of an always-block.
  # We can do this by restoring the error status from the try-block.
  (( TRY_BLOCK_ERROR = 1 ))
fi
# Raise an error, but don't show an error message.
THROW= 2>/dev/null
