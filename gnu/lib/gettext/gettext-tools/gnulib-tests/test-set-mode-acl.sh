#!/bin/sh

# Show all commands when run with environment variable VERBOSE=yes.
test -z "$VERBOSE" || set -x

test "$USE_ACL" = 0 &&
  {
    echo "Skipping test: insufficient ACL support"
    exit 77
  }

# func_tmpdir
# creates a temporary directory.
# Sets variable
# - tmp             pathname of freshly created temporary directory
func_tmpdir ()
{
  # Use the environment variable TMPDIR, falling back to /tmp. This allows
  # users to specify a different temporary directory, for example, if their
  # /tmp is filled up or too small.
  : "${TMPDIR=/tmp}"
  {
    # Use the mktemp program if available. If not available, hide the error
    # message.
    tmp=`(umask 077 && mktemp -d "$TMPDIR/glXXXXXX") 2>/dev/null` &&
    test -n "$tmp" && test -d "$tmp"
  } ||
  {
    # Use a simple mkdir command. It is guaranteed to fail if the directory
    # already exists.  $RANDOM is bash specific and expands to empty in shells
    # other than bash, ksh and zsh.  Its use does not increase security;
    # rather, it minimizes the probability of failure in a very cluttered /tmp
    # directory.
    tmp=$TMPDIR/gl$$-$RANDOM
    (umask 077 && mkdir "$tmp")
  } ||
  {
    echo "$0: cannot create a temporary directory in $TMPDIR" >&2
    exit 1
  }
}

func_tmpdir
# builddir may already be set by the script that invokes this one.
case "$builddir" in
  '') builddir=`pwd` ;;
  /* | ?:*) ;;
  *) builddir=`pwd`/$builddir ;;
esac
cd "$builddir" ||
  {
    echo "$0: cannot determine build directory (unreadable parent dir?)" >&2
    exit 1
  }
# Switch to a temporary directory, to increase the likelihood that ACLs are
# supported on the current file system. (/tmp is usually locally mounted,
# whereas the build dir is sometimes NFS-mounted.)
( cd "$tmp"

  # Prepare tmpfile0.
  rm -f tmpfile[0-9]
  echo "Simple contents" > tmpfile0
  chmod 600 tmpfile0

  # Classification of the platform according to the programs available for
  # manipulating ACLs.
  # Possible values are:
  #   linux, cygwin, freebsd, solaris, hpux, hpuxjfs, osf1, aix, macosx, irix, none.
  # TODO: Support also native Windows platforms (mingw).
  acl_flavor=none
  if (getfacl tmpfile0 >/dev/null) 2>/dev/null; then
    # Platforms with the getfacl and setfacl programs.
    # Linux, FreeBSD, Solaris, Cygwin.
    if (setfacl --help >/dev/null) 2>/dev/null; then
      # Linux, Cygwin.
      if (LC_ALL=C setfacl --help | grep ' --set-file' >/dev/null) 2>/dev/null; then
        # Linux.
        acl_flavor=linux
      else
        acl_flavor=cygwin
      fi
    else
      # FreeBSD, Solaris.
      if (LC_ALL=C setfacl 2>&1 | grep '\-x entries' >/dev/null) 2>/dev/null; then
        # FreeBSD.
        acl_flavor=freebsd
      else
        # Solaris.
        acl_flavor=solaris
      fi
    fi
  else
    if (lsacl / >/dev/null) 2>/dev/null; then
      # Platforms with the lsacl and chacl programs.
      # HP-UX, sometimes also IRIX.
      if (getacl tmpfile0 >/dev/null) 2>/dev/null; then
        # HP-UX 11.11 or newer.
        acl_flavor=hpuxjfs
      else
        # HP-UX 11.00.
        acl_flavor=hpux
      fi
    else
      if (getacl tmpfile0 >/dev/null) 2>/dev/null; then
        # Tru64, NonStop Kernel.
        if (getacl -m tmpfile0 >/dev/null) 2>/dev/null; then
          # Tru64.
          acl_flavor=osf1
        else
          # NonStop Kernel.
          acl_flavor=nsk
        fi
      else
        if (aclget tmpfile0 >/dev/null) 2>/dev/null; then
          # AIX.
          acl_flavor=aix
        else
          if (fsaclctl -v >/dev/null) 2>/dev/null; then
            # Mac OS X.
            acl_flavor=macosx
          else
            if test -f /sbin/chacl; then
              # IRIX.
              acl_flavor=irix
            fi
          fi
        fi
      fi
    fi
  fi

  if test $acl_flavor != none; then
    # A POSIX compliant 'id' program.
    if test -f /usr/xpg4/bin/id; then
      ID=/usr/xpg4/bin/id
    else
      ID=id
    fi
    # Use a user and group id different from the current one, to avoid
    # redundant/ambiguous ACLs.
    myuid=`$ID -u`
    mygid=`$ID -g`
    auid=1
    if test "$auid" = "$myuid"; then auid=2; fi
    agid=1
    if test "$agid" = "$mygid"; then agid=2; fi
  fi

  for mode in 700 400 200 100 644 650 605 011 4700 2070; do
    rm -f tmpfile0 tmpfile1 tmpfile2

    # Prepare a file with no ACL.
    echo "Anything" > tmpfile0
    # If a mode is not supported (e.g. 2070 on FreeBSD), we skip testing it.
    if chmod $mode tmpfile0 2>/dev/null; then
      modestring0=`ls -l tmpfile0 | dd ibs=1 count=10 2>/dev/null`

      # Prepare a file with no ACL.
      echo "Some contents" > tmpfile1
      chmod 600 tmpfile1

      # Try to set the ACL to only the given mode.
      ${CHECKER} "$builddir"/test-set-mode-acl${EXEEXT} tmpfile1 $mode
      # Verify that tmpfile1 has no ACL and has the desired mode.
      modestring=`ls -l tmpfile1 | dd ibs=1 count=10 2>/dev/null`
      if test "x$modestring" != "x$modestring0"; then
        echo "mode = $mode: tmpfile1 has wrong mode: $modestring" 1>&2
        exit 1
      fi
      if test `${CHECKER} "$builddir"/test-file-has-acl${EXEEXT} tmpfile1` != no; then
        echo "mode = $mode: tmpfile1 got an ACL" 1>&2
        exit 1
      fi

      if test $acl_flavor != none; then

        # Prepare a file with an ACL.
        echo "Special contents" > tmpfile2
        chmod 600 tmpfile2
        # Set an ACL for a user (or group).
        case $acl_flavor in
          linux | freebsd | solaris)
            setfacl -m user:$auid:1 tmpfile0
            ;;
          cygwin)
            setfacl -m group:0:1 tmpfile0
            ;;
          hpux)
            orig=`lsacl tmpfile0 | sed -e 's/ tmpfile0$//'`
            chacl -r "${orig}($auid.%,--x)" tmpfile0
            ;;
          hpuxjfs)
            orig=`lsacl tmpfile0 | sed -e 's/ tmpfile0$//'`
            chacl -r "${orig}($auid.%,--x)" tmpfile0 \
              || setacl -m user:$auid:1 tmpfile0
            ;;
          osf1)
            setacl -u user:$auid:1 tmpfile0
            ;;
          nsk)
            setacl -m user:$auid:1 tmpfile0
            ;;
          aix)
            { aclget tmpfile0 | sed -e 's/disabled$/enabled/'; echo "        permit --x u:$auid"; } | aclput tmpfile0
            ;;
          macosx)
            /bin/chmod +a "user:daemon allow execute" tmpfile0
            ;;
          irix)
            /sbin/chacl user::rw-,group::---,other::---,user:$auid:--x tmpfile0
            ;;
        esac

        # Try to set the ACL to only the given mode.
        ${CHECKER} "$builddir"/test-set-mode-acl${EXEEXT} tmpfile2 $mode
        # Verify that tmpfile2 has no ACL and has the desired mode.
        modestring=`ls -l tmpfile2 | dd ibs=1 count=10 2>/dev/null`
        if test "x$modestring" != "x$modestring0"; then
          echo "mode = $mode: tmpfile2 has wrong mode: $modestring" 1>&2
          exit 1
        fi
        if test `${CHECKER} "$builddir"/test-file-has-acl${EXEEXT} tmpfile2` != no; then
          echo "mode = $mode: tmpfile2 still has an ACL" 1>&2
          exit 1
        fi
      fi
    fi
  done

  rm -f tmpfile[0-9]
) || exit 1

rm -rf "$tmp"
exit 0
