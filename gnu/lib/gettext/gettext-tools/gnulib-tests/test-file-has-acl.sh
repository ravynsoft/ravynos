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
  rm -f tmpfile[0-9] tmp.err
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
      if (LC_ALL=C setfacl --help | grep ' --test' >/dev/null) 2>/dev/null; then
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

  # func_test_file_has_acl file expected
  # tests the result of the file_has_acl function on file, and checks that it
  # matches the expected value.
  func_test_file_has_acl ()
  {
    res=`${CHECKER} "$builddir"/test-file-has-acl${EXEEXT} "$1"`
    test "$res" = "$2" || {
      echo "file_has_acl(\"$1\") returned $res, expected $2" 1>&2
      exit 1
    }
  }

  # func_test_has_acl file expected
  # tests the result of the file_has_acl function on file, and checks that it
  # matches the expected value, also taking into account the system's 'ls'
  # program.
  case $acl_flavor in
    freebsd | solaris | hpux | macosx)
      case $acl_flavor in
        freebsd | solaris | hpux) acl_ls_option="-ld" ;;
        macosx)                   acl_ls_option="-lde" ;;
      esac
      func_test_has_acl ()
      {
        func_test_file_has_acl "$1" "$2"
        case `/bin/ls $acl_ls_option "$1" | sed 1q` in
          ??????????+*)
            test "$2" = yes || {
              echo "/bin/ls $acl_ls_option $1 shows an ACL, but expected $2" 1>&2
              exit 1
            }
            ;;
          ??????????" "*)
            test "$2" = no || {
              echo "/bin/ls $acl_ls_option $1 shows no ACL, but expected $2" 1>&2
              exit 1
            }
            ;;
        esac
      }
      ;;
    irix)
      func_test_has_acl ()
      {
        func_test_file_has_acl "$1" "$2"
        case `/bin/ls -ldD "$1" | sed 1q` in
          *" []")
            test "$2" = no || {
              echo "/bin/ls -ldD $1 shows no ACL, but expected $2" 1>&2
              exit 1
            }
            ;;
          *)
            test "$2" = yes || {
              echo "/bin/ls -ldD $1 shows an ACL, but expected $2" 1>&2
              exit 1
            }
            ;;
        esac
      }
      ;;
    *)
      func_test_has_acl ()
      {
        func_test_file_has_acl "$1" "$2"
      }
      ;;
  esac

  func_test_has_acl tmpfile0 no

  mkdir tmpdir0
  func_test_has_acl tmpdir0 no

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

    case $acl_flavor in
      linux | freebsd | solaris)

        # Set an ACL for a user.
        if setfacl -m user:$auid:1 tmpfile0; then

          func_test_has_acl tmpfile0 yes

          # Remove the ACL for the user.
          case $acl_flavor in
            linux)   setfacl -x user:$auid tmpfile0 ;;
            freebsd) setfacl -x user:$auid:1 tmpfile0 ;;
            *)       setfacl -d user:$auid:1 tmpfile0 ;;
          esac

          # On Linux and FreeBSD, the ACL for the mask is implicitly added.
          # On Solaris, it is always there.
          case $acl_flavor in
            linux | freebsd) func_test_has_acl tmpfile0 yes ;;
            *)               func_test_has_acl tmpfile0 no ;;
          esac

          # Remove the ACL for the mask, if it was implicitly added.
          case $acl_flavor in
            linux | freebsd) setfacl -x mask: tmpfile0 ;;
            *)               setfacl -d mask: tmpfile0 ;;
          esac

          func_test_has_acl tmpfile0 no

        fi
        ;;

      cygwin)

        # Set an ACL for a group.
        if setfacl -m group:0:1 tmpfile0; then

          func_test_has_acl tmpfile0 yes

          # Remove the ACL for the group.
          setfacl -d group:0 tmpfile0

          func_test_has_acl tmpfile0 no

        fi
        ;;

      hpux | hpuxjfs)

        # Set an ACL for a user.
        orig=`lsacl tmpfile0 | sed -e 's/ tmpfile0$//'`
        if chacl -r "${orig}($auid.%,--x)" tmpfile0; then

          func_test_has_acl tmpfile0 yes

          # Remove the ACL for the user.
          chacl -d "($auid.%,--x)" tmpfile0

          func_test_has_acl tmpfile0 no

        else
          if test $acl_flavor = hpuxjfs; then

            # Set an ACL for a user.
            setacl -m user:$auid:1 tmpfile0

            func_test_has_acl tmpfile0 yes

            # Remove the ACL for the user.
            setacl -d user:$auid tmpfile0

            func_test_has_acl tmpfile0 no

          fi
        fi
        ;;

      osf1)

        # Set an ACL for a user.
        setacl -u user:$auid:1 tmpfile0 2> tmp.err
        cat tmp.err 1>&2
        if grep 'Error:' tmp.err > /dev/null \
           || grep 'Operation not supported' tmp.err > /dev/null; then
          :
        else

          func_test_has_acl tmpfile0 yes

          # Remove the ACL for the user.
          setacl -x user:$auid:1 tmpfile0

          func_test_has_acl tmpfile0 no

        fi
        ;;

      nsk)

        # Set an ACL for a user.
        setacl -m user:$auid:1 tmpfile0

        func_test_has_acl tmpfile0 yes

        # Remove the ACL for the user.
        setacl -d user:$auid tmpfile0

        func_test_has_acl tmpfile0 no

        ;;

      aix)

        # Set an ACL for a user.
        { aclget tmpfile0 | sed -e 's/disabled$/enabled/'; echo "        permit --x u:$auid"; } | aclput tmpfile0
        if aclget tmpfile0 | grep enabled > /dev/null; then

          func_test_has_acl tmpfile0 yes

          # Remove the ACL for the user.
          aclget tmpfile0 | grep -v ' u:[^ ]*$' | aclput tmpfile0

          func_test_has_acl tmpfile0 no

        fi
        ;;

      macosx)

        # Set an ACL for a user.
        /bin/chmod +a "user:daemon allow execute" tmpfile0

        func_test_has_acl tmpfile0 yes

        # Remove the ACL for the user.
        /bin/chmod -a "user:daemon allow execute" tmpfile0

        func_test_has_acl tmpfile0 no

        ;;

      irix)

        # Set an ACL for a user.
        /sbin/chacl user::rw-,group::---,other::---,user:$auid:--x tmpfile0 2> tmp.err
        cat tmp.err 1>&2
        if test -s tmp.err; then :; else

          func_test_has_acl tmpfile0 yes

          # Remove the ACL for the user.
          /sbin/chacl user::rw-,group::---,other::--- tmpfile0

          func_test_has_acl tmpfile0 no

        fi
        ;;

    esac
  fi

  rm -f tmpfile[0-9] tmp.err
  rm -rf tmpdir0
) || exit 1

rm -rf "$tmp"
exit 0
