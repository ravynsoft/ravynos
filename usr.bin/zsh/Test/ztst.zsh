#!/bin/zsh -f
# The line above is just for convenience.  Normally tests will be run using
# a specified version of zsh.  With dynamic loading, any required libraries
# must already have been installed in that case.
#
# Takes one argument: the name of the test file.  Currently only one such
# file will be processed each time ztst.zsh is run.  This is slower, but
# much safer in terms of preserving the correct status.
# To avoid namespace pollution, all functions and parameters used
# only by the script begin with ZTST_.
#
# Options (without arguments) may precede the test file argument; these
# are interpreted as shell options to set.  -x is probably the most useful.

# Produce verbose messages if non-zero.
# If 1, produce reports of tests executed; if 2, also report on progress.
# Defined in such a way that any value from the environment is used.
: ${ZTST_verbose:=0}

# If non-zero, continue the tests even after a test fails.
: ${ZTST_continue:=0}

# We require all options to be reset, not just emulation options.
# Unfortunately, due to the crud which may be in /etc/zshenv this might
# still not be good enough.  Maybe we should trick it somehow.
emulate -R zsh

# Ensure the locale does not screw up sorting.  Don't supply a locale
# unless there's one set, to minimise problems.
[[ -n $LC_ALL ]] && LC_ALL=C
[[ -n $LC_CTYPE ]] && LC_CTYPE=C
[[ -n $LC_COLLATE ]] && LC_COLLATE=C
[[ -n $LC_NUMERIC ]] && LC_NUMERIC=C
[[ -n $LC_MESSAGES ]] && LC_MESSAGES=C
[[ -n $LANG ]] && LANG=C
# Test file may (or may not) set LANG to other locales. In either case,
# LANG must be passed to child zsh.
export LANG

# Don't propagate variables that are set by default in the shell.
typeset +x WORDCHARS

# Set the module load path to correspond to this build of zsh.
# This Modules directory should have been created by "make check".
[[ -d Modules/zsh ]] && module_path=( $PWD/Modules )

# We need to be able to save and restore the options used in the test.
# We use the $options variable of the parameter module for this.
zmodload zsh/parameter

# Note that both the following are regular arrays, since we only use them
# in whole array assignments to/from $options.
# Options set in test code (i.e. by default all standard options)
ZTST_testopts=(${(kv)options})

setopt extendedglob nonomatch
while [[ $1 = [-+]* ]]; do
  set $1
  shift
done
# Options set in main script
ZTST_mainopts=(${(kv)options})

# We run in the current directory, so remember it.
ZTST_testdir=$PWD
ZTST_testname=$1

integer ZTST_testfailed=0

# This is POSIX nonsense.  Because of the vague feeling someone, somewhere
# may one day need to examine the arguments of "tail" using a standard
# option parser, every Unix user in the world is expected to switch
# to using "tail -n NUM" instead of "tail -NUM".  Older versions of
# tail don't support this.
tail() {
  emulate -L zsh

  if [[ -z $TAIL_SUPPORTS_MINUS_N ]]; then
    local test
    test=$(echo "foo\nbar" | command tail -n 1 2>/dev/null)
    if [[ $test = bar ]]; then
      TAIL_SUPPORTS_MINUS_N=1
    else
      TAIL_SUPPORTS_MINUS_N=0
    fi
  fi

  integer argi=${argv[(i)-<->]}

  if [[ $argi -le $# && $TAIL_SUPPORTS_MINUS_N = 1 ]]; then
    argv[$argi]=(-n ${argv[$argi][2,-1]})
  fi

  command tail "$argv[@]"
}

# The source directory is not necessarily the current directory,
# but if $0 doesn't contain a `/' assume it is.
if [[ $0 = */* ]]; then
  ZTST_srcdir=${0%/*}
else
  ZTST_srcdir=$PWD
fi
[[ $ZTST_srcdir = /* ]] || ZTST_srcdir="$ZTST_testdir/$ZTST_srcdir"

# Set the function autoload paths to correspond to this build of zsh.
fpath=( $ZTST_srcdir/../Functions/*~*/CVS(/)
        $ZTST_srcdir/../Completion
        $ZTST_srcdir/../Completion/*/*~*/CVS(/) )

: ${TMPPREFIX:=/tmp/zsh}
ZTST_tmp=${TMPPREFIX}.ztst.$$
if ! rm -f $ZTST_tmp || ! mkdir -p $ZTST_tmp || ! chmod go-w $ZTST_tmp; then
  print "Can't create $ZTST_tmp for exclusive use." >&2
  exit 1
fi
# Temporary files for redirection inside tests.
ZTST_in=${ZTST_tmp}/ztst.in
# hold the expected output
ZTST_out=${ZTST_tmp}/ztst.out
ZTST_err=${ZTST_tmp}/ztst.err
# hold the actual output from the test
ZTST_tout=${ZTST_tmp}/ztst.tout
ZTST_terr=${ZTST_tmp}/ztst.terr

ZTST_cleanup() {
  cd $ZTST_testdir
  rm -rf $ZTST_testdir/dummy.tmp $ZTST_testdir/*.tmp(N) ${ZTST_tmp}
}

# This cleanup always gets performed, even if we abort.  Later,
# we should try and arrange that any test-specific cleanup
# always gets called as well.
##trap 'print cleaning up...
##ZTST_cleanup' INT QUIT TERM
# Make sure it's clean now.
rm -rf dummy.tmp *.tmp

# Report failure.  Note that all output regarding the tests goes to stdout.
# That saves an unpleasant mixture of stdout and stderr to sort out.
ZTST_testfailed() {
  print -r "Test $ZTST_testname failed: $1"
  if [[ -n $ZTST_message ]]; then
    print -r "Was testing: $ZTST_message"
  fi
  print -r "$ZTST_testname: test failed."
  if [[ -n $ZTST_failmsg ]]; then
    print -r "The following may (or may not) help identifying the cause:
$ZTST_failmsg"
  fi
  ZTST_testfailed=1
  # if called from within ZTST_Test() this will increment ZTST_Test's local
  # ZTST_failures. Otherwise global ZTST_failures will be incremented
  # (but currently its value is not used).
  (( ++ZTST_failures ))
  return 1
}
ZTST_testxpassed() {
  print -r "Test $ZTST_testname was expected to fail, but passed."
  if [[ -n $ZTST_message ]]; then
    print -r "Was testing: $ZTST_message"
  fi
  print -r "$ZTST_testname: test XPassed."
  if [[ -n $ZTST_failmsg ]]; then
    print -r "The following may (or may not) help identifying the cause:
$ZTST_failmsg"
  fi
  ZTST_testfailed=1
  (( ++ZTST_failures ))
  return 1
}

# Print messages if $ZTST_verbose is non-empty
ZTST_verbose() {
  local lev=$1
  shift
  if [[ -n $ZTST_verbose && $ZTST_verbose -ge $lev ]]; then
    print -r -u $ZTST_fd -- $*
  fi
}
ZTST_hashmark() {
  if [[ ZTST_verbose -le 0 && -t $ZTST_fd ]]; then
    print -n -u$ZTST_fd -- ${(pl:SECONDS::\#::\#\r:)}
  fi
  (( SECONDS > COLUMNS+1 && (SECONDS -= COLUMNS) ))
}

if [[ ! -r $ZTST_testname ]]; then
  ZTST_testfailed "can't read test file."
  exit 1
fi

exec {ZTST_fd}>&1
exec {ZTST_input}<$ZTST_testname

# The current line read from the test file.
ZTST_curline=''
# The current section being run
ZTST_cursect=''

# Get a new input line.  Don't mangle spaces; set IFS locally to empty.
# We shall skip comments at this level.
ZTST_getline() {
  local IFS=
  while true; do
    read -u $ZTST_input -r ZTST_curline || return 1
    [[ $ZTST_curline == \#* ]] || return 0
  done
}

# Get the name of the section.  It may already have been read into
# $curline, or we may have to skip some initial comments to find it.
# If argument present, it's OK to skip the reset of the current section,
# so no error if we find garbage.
ZTST_getsect() {
  local match mbegin mend

  while [[ $ZTST_curline != '%'(#b)([[:alnum:]]##)* ]]; do
    ZTST_getline || return 1
    [[ $ZTST_curline = [[:blank:]]# ]] && continue
    if [[ $# -eq 0 && $ZTST_curline != '%'[[:alnum:]]##* ]]; then
      ZTST_testfailed "bad line found before or after section:
$ZTST_curline"
      exit 1
    fi
  done
  # have the next line ready waiting
  ZTST_getline
  ZTST_cursect=${match[1]}
  ZTST_verbose 2 "ZTST_getsect: read section name: $ZTST_cursect"
  return 0
}

# Read in an indented code chunk for execution
ZTST_getchunk() {
  # Code chunks are always separated by blank lines or the
  # end of a section, so if we already have a piece of code,
  # we keep it.  Currently that shouldn't actually happen.
  ZTST_code=''
  # First find the chunk.
  while [[ $ZTST_curline = [[:blank:]]# ]]; do
    ZTST_getline || break
  done
  while [[ $ZTST_curline = [[:blank:]]##[^[:blank:]]* ]]; do
    ZTST_code="${ZTST_code:+${ZTST_code}
}${ZTST_curline}"
    ZTST_getline || break
  done
  ZTST_verbose 2 "ZTST_getchunk: read code chunk:
$ZTST_code"
  [[ -n $ZTST_code ]]
}

# Read in a piece for redirection.
ZTST_getredir() {
  local char=${ZTST_curline[1]} fn
  ZTST_redir=${ZTST_curline[2,-1]}
  while ZTST_getline; do
    [[ $ZTST_curline[1] = $char ]] || break
    ZTST_redir="${ZTST_redir}
${ZTST_curline[2,-1]}"
  done
  ZTST_verbose 2 "ZTST_getredir: read redir for '$char':
$ZTST_redir"

  case $char in
    ('<') fn=$ZTST_in
    ;;
    ('>') fn=$ZTST_out
    ;;
    ('?') fn=$ZTST_err
    ;;
    (*)  ZTST_testfailed "bad redir operator: $char"
    return 1
    ;;
  esac
  if [[ $ZTST_flags = *q* && $char = '<' ]]; then
    # delay substituting output until variables are set
    print -r -- "${(e)ZTST_redir}" >>$fn
  else
    print -r -- "$ZTST_redir" >>$fn
  fi

  return 0
}

# Execute an indented chunk.  Redirections will already have
# been set up, but we need to handle the options.
ZTST_execchunk() {
  setopt localloops # don't let continue & break propagate out
  options=($ZTST_testopts)
  () {
      unsetopt localloops
      eval "$ZTST_code"
  }
  ZTST_status=$?
  # careful... ksh_arrays may be in effect.
  ZTST_testopts=(${(kv)options[*]})
  options=(${ZTST_mainopts[*]})
  ZTST_verbose 2 "ZTST_execchunk: status $ZTST_status"
  return $ZTST_status
}

# Functions for preparation and cleaning.
ZTST_prep ZTST_clean () {
  # Execute indented code chunks. If ZTST_unimplemented is set
  # in any chunk then we will skip the remaining chunks.
  # We ignore return status of chunks when cleaning up.
  while [[ -z "$ZTST_unimplemented" ]] && ZTST_getchunk; do
    ZTST_execchunk >/dev/null || [[ $0 = ZTST_clean ]] || {
      ZTST_testfailed "non-zero status from preparation code:
$ZTST_code"
      return 1
    }
  done
  return 0
}

# diff wrapper
ZTST_diff() {
  emulate -L zsh
  setopt extendedglob

  local diff_out
  integer diff_pat diff_ret

  case $1 in
    (p)
    diff_pat=1
    ;;

    (d)
    ;;

    (*)
    print "Bad ZTST_diff code: d for diff, p for pattern match"
    ;;
  esac
  shift
      
  if (( diff_pat )); then
    local -a diff_lines1 diff_lines2
    integer failed i l
    local p

    diff_lines1=("${(f@)$(<$argv[-2])}")
    diff_lines2=("${(f@)$(<$argv[-1])}")
    if (( ${#diff_lines1} != ${#diff_lines2} )); then
      failed=1
      print -r "Pattern match failed, line mismatch (${#diff_lines1}/${#diff_lines2}):"
    else
      for (( i = 1; i <= ${#diff_lines1}; i++ )); do
	if [[ ${diff_lines2[i]} != ${~diff_lines1[i]} ]]; then
	  failed=1
	  print -r "Pattern match failed, line $i:"
	  break
	fi
      done
    fi
    if (( failed )); then
      for (( l = 1; l <= ${#diff_lines1}; ++l )); do
	if (( l == i )); then
	  p="-"
	else
	  p=" "
	fi
	print -r -- "$p<${diff_lines1[l]}"
      done
      for (( l = 1; l <= ${#diff_lines2}; ++l )); do
	if (( l == i )); then
	  p="+"
	else
	  p=" "
	fi
	print -r -- "$p>${diff_lines2[l]}"
      done
      diff_ret=1
    fi
  else
    diff_out=$(diff -a "$@")
    diff_ret="$?"
    if [[ "$diff_ret" != "0" ]]; then
      print -r -- "$diff_out"
    fi
  fi

  return "$diff_ret"
}

ZTST_test() {
  local last match mbegin mend found substlines
  local diff_out diff_err
  local ZTST_skip
  integer expected_to_fail ZTST_failures

  while true; do
    rm -f $ZTST_in $ZTST_out $ZTST_err
    touch $ZTST_in $ZTST_out $ZTST_err
    ZTST_message=''
    ZTST_failmsg=''
    found=0
    diff_out=d
    diff_err=d

    ZTST_verbose 2 "ZTST_test: looking for new test"

    while true; do
      ZTST_verbose 2 "ZTST_test: examining line:
$ZTST_curline"
      case $ZTST_curline in
	(%*) if [[ $found = 0 ]]; then
	      break 2
	    else
	      last=1
	      break
	    fi
	    ;;
	([[:space:]]#)
	    if [[ $found = 0 ]]; then
	      ZTST_getline || break 2
	      continue
	    else
	      break
	    fi
	    ;;
	([[:space:]]##[^[:space:]]*) ZTST_getchunk
	  if [[ $ZTST_curline == (#b)([-0-9]##)([[:alpha:]]#)(:*)# ]]; then
	    ZTST_xstatus=$match[1]
	    ZTST_flags=$match[2]
	    ZTST_message=${match[3]:+${match[3][2,-1]}}
	  else
	    ZTST_testfailed "expecting test status at:
$ZTST_curline"
	    return 1
	  fi
	  ZTST_getline
	  found=1
	  ;;
	('<'*) ZTST_getredir || return 1
	  found=1
	  ;;
	('*>'*)
	  ZTST_curline=${ZTST_curline[2,-1]}
	  diff_out=p
	  ;&
	('>'*)
	  ZTST_getredir || return 1
	  found=1
	  ;;
	('*?'*)
	  ZTST_curline=${ZTST_curline[2,-1]}
	  diff_err=p
	  ;&
	('?'*)
	  ZTST_getredir || return 1
	  found=1
	  ;;
	('F:'*) ZTST_failmsg="${ZTST_failmsg:+${ZTST_failmsg}
}  ${ZTST_curline[3,-1]}"
	  ZTST_getline
	  found=1
          ;;
	(*) ZTST_testfailed "bad line in test block:
$ZTST_curline"
	  return 1
          ;;
      esac
    done

    # If we found some code to execute...
    if [[ -n $ZTST_code ]]; then
      ZTST_hashmark
      ZTST_verbose 1 "Running test: $ZTST_message"
      ZTST_verbose 2 "ZTST_test: expecting status: $ZTST_xstatus"
      ZTST_verbose 2 "Input: $ZTST_in, output: $ZTST_out, error: $ZTST_terr"

      ZTST_execchunk <$ZTST_in >$ZTST_tout 2>$ZTST_terr

      if [[ -n $ZTST_skip ]]; then
	ZTST_verbose 0 "Test case skipped: $ZTST_skip"
	ZTST_skip=
	if [[ -n $last ]]; then
	  break
	else
	  continue
	fi
      fi

      if [[ $ZTST_flags = *f* ]]; then
        expected_to_fail=1
        ZTST_xfail_diff() { ZTST_diff "$@" > /dev/null }
        ZTST_diff=ZTST_xfail_diff
      else
        expected_to_fail=0
        ZTST_diff=ZTST_diff
      fi

      # First check we got the right status, if specified.
      if [[ $ZTST_xstatus != - && $ZTST_xstatus != $ZTST_status ]]; then
        if (( expected_to_fail )); then
          ZTST_verbose 1 "Test failed, as expected."
          continue
        fi
	ZTST_testfailed "bad status $ZTST_status, expected $ZTST_xstatus from:
$ZTST_code${$(<$ZTST_terr):+
Error output:
$(<$ZTST_terr)}"
        if (( ZTST_continue ));then continue; else return 1; fi
      fi

      ZTST_verbose 2 "ZTST_test: test produced standard output:
$(<$ZTST_tout)
ZTST_test: and standard error:
$(<$ZTST_terr)"

      # Now check output and error.
      if [[ $ZTST_flags = *q* && -s $ZTST_out ]]; then
	substlines="$(<$ZTST_out)"
	rm -rf $ZTST_out
	print -r -- "${(e)substlines}" >$ZTST_out
      fi
      if [[ $ZTST_flags != *d* ]] && ! $ZTST_diff $diff_out -u $ZTST_out $ZTST_tout; then
        if (( expected_to_fail )); then
          ZTST_verbose 1 "Test failed, as expected."
          continue
        fi
	ZTST_testfailed "output differs from expected as shown above for:
$ZTST_code${$(<$ZTST_terr):+
Error output:
$(<$ZTST_terr)}"
        if (( ZTST_continue ));then continue; else return 1; fi
      fi
      if [[ $ZTST_flags = *q* && -s $ZTST_err ]]; then
	substlines="$(<$ZTST_err)"
	rm -rf $ZTST_err
	print -r -- "${(e)substlines}" >$ZTST_err
      fi
      if [[ $ZTST_flags != *D* ]] && ! $ZTST_diff $diff_err -u $ZTST_err $ZTST_terr; then
        if (( expected_to_fail )); then
          ZTST_verbose 1 "Test failed, as expected."
          continue
        fi
	ZTST_testfailed "error output differs from expected as shown above for:
$ZTST_code"
        if (( ZTST_continue ));then continue; else return 1; fi
      fi
      if (( expected_to_fail )); then
        ZTST_testxpassed
        if (( ZTST_continue ));then continue; else return 1; fi
      fi
    fi
    ZTST_verbose 1 "Test successful."
    [[ -n $last ]] && break
  done

  if (( ZTST_failures )); then
    ZTST_verbose 1 "ZTST_test: $ZTST_failures test(s) failed"
  else
    ZTST_verbose 2 "ZTST_test: all tests successful"
  fi

  # reset message to keep ZTST_testfailed output correct
  ZTST_message=''

  return ZTST_failures
}


# Remember which sections we've done.
typeset -A ZTST_sects
ZTST_sects=(prep 0 test 0 clean 0)

print "$ZTST_testname: starting."

# Now go through all the different sections until the end.
# prep section may set ZTST_unimplemented, in this case the actual
# tests will be skipped
ZTST_skipok=
ZTST_unimplemented=
while [[ -z "$ZTST_unimplemented" ]] && ZTST_getsect $ZTST_skipok; do
  case $ZTST_cursect in
    (prep) if (( ${ZTST_sects[prep]} + ${ZTST_sects[test]} + \
	        ${ZTST_sects[clean]} )); then
	    ZTST_testfailed "\`prep' section must come first"
	    break   # skip %test and %clean sections, but run ZTST_cleanup
	  fi
	  ZTST_prep || ZTST_skipok=1
	  ZTST_sects[prep]=1
	  ;;
    (test)
	  if (( ${ZTST_sects[test]} + ${ZTST_sects[clean]} )); then
	    ZTST_testfailed "bad placement of \`test' section"
	    break   # skip %clean section, but run ZTST_cleanup
	  fi
          if [[ -z "$ZTST_skipok" ]]; then  # if no error in %prep
            # careful here: we can't execute ZTST_test before || or &&
            # because that affects the behaviour of traps in the tests.
            ZTST_test
            (( $? )) && ZTST_skipok=1
          fi
	  ZTST_sects[test]=1
	  ;;
    (clean)
	   if (( ${ZTST_sects[test]} == 0 || ${ZTST_sects[clean]} )); then
	     ZTST_testfailed "bad use of \`clean' section"
	   else
	     ZTST_clean
	     ZTST_sects[clean]=1
	   fi
	   ZTST_skipok=
	   ;;
    *) ZTST_testfailed "bad section name: $ZTST_cursect"
       ;;
  esac
done

if [[ -n "$ZTST_unimplemented" ]]; then
  print "$ZTST_testname: skipped ($ZTST_unimplemented)"
  ZTST_testfailed=2
elif (( ! $ZTST_testfailed )); then
  print "$ZTST_testname: all tests successful."
fi
ZTST_cleanup
exit $(( ZTST_testfailed ))
