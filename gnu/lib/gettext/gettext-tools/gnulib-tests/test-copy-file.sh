#!/bin/sh

# Show all commands when run with environment variable VERBOSE=yes.
test -z "$VERBOSE" || set -x

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
  rm -f tmpfile[0-9] tmpaclout[0-2]
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

  # Define a function to test for the same ACLs, from the point of view of
  # the programs.
  # func_test_same_acls file1 file2
  case $acl_flavor in
    linux | cygwin | freebsd | solaris)
      func_test_same_acls ()
      {
        getfacl "$1" | sed -e "s/$1/FILENAME/g" > tmpaclout1
        getfacl "$2" | sed -e "s/$2/FILENAME/g" > tmpaclout2
        cmp tmpaclout1 tmpaclout2 > /dev/null
      }
      ;;
    hpux)
      func_test_same_acls ()
      {
        lsacl "$1" | sed -e "s/$1/FILENAME/g" > tmpaclout1
        lsacl "$2" | sed -e "s/$2/FILENAME/g" > tmpaclout2
        cmp tmpaclout1 tmpaclout2 > /dev/null
      }
      ;;
    hpuxjfs)
      func_test_same_acls ()
      {
        { lsacl "$1" | sed -e "s/$1/FILENAME/g" > tmpaclout1
          lsacl "$2" | sed -e "s/$2/FILENAME/g" > tmpaclout2
          cmp tmpaclout1 tmpaclout2 > /dev/null
        } &&
        { getacl "$1" | sed -e "s/$1/FILENAME/g" > tmpaclout1
          getacl "$2" | sed -e "s/$2/FILENAME/g" > tmpaclout2
          cmp tmpaclout1 tmpaclout2 > /dev/null
        }
      }
      ;;
    osf1 | nsk)
      func_test_same_acls ()
      {
        getacl "$1" | sed -e "s/$1/FILENAME/g" > tmpaclout1
        getacl "$2" | sed -e "s/$2/FILENAME/g" > tmpaclout2
        cmp tmpaclout1 tmpaclout2 > /dev/null
      }
      ;;
    aix)
      func_test_same_acls ()
      {
        aclget "$1" > tmpaclout1
        aclget "$2" > tmpaclout2
        cmp tmpaclout1 tmpaclout2 > /dev/null
      }
      ;;
    macosx)
      func_test_same_acls ()
      {
        /bin/ls -le "$1" | sed -e "s/$1/FILENAME/g" > tmpaclout1
        /bin/ls -le "$2" | sed -e "s/$2/FILENAME/g" > tmpaclout2
        cmp tmpaclout1 tmpaclout2 > /dev/null
      }
      ;;
    irix)
      func_test_same_acls ()
      {
        /bin/ls -lD "$1" | sed -e "s/$1/FILENAME/g" > tmpaclout1
        /bin/ls -lD "$2" | sed -e "s/$2/FILENAME/g" > tmpaclout2
        cmp tmpaclout1 tmpaclout2 > /dev/null
      }
      ;;
    none)
      func_test_same_acls ()
      {
        :
      }
      ;;
  esac

  # func_test_copy file1 file2
  # copies file1 to file2 and verifies the permissions and ACLs are the same
  # on both.
  func_test_copy ()
  {
    ${CHECKER} "$builddir"/test-copy-file${EXEEXT} "$1" "$2" || exit 1
    if test "$USE_ACL" != 0; then
      ${CHECKER} "$builddir"/test-sameacls${EXEEXT} "$1" "$2" || exit 1
      func_test_same_acls                           "$1" "$2" || exit 1
    fi
  }

  func_test_copy tmpfile0 tmpfile1

  if test "$USE_ACL" != 0 && test $acl_flavor != none; then
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
        setfacl -m user:$auid:1 tmpfile0

        func_test_copy tmpfile0 tmpfile2

        # Set an ACL for a group.
        setfacl -m group:$agid:4 tmpfile0

        func_test_copy tmpfile0 tmpfile3

        # Set an ACL for other.
        case $acl_flavor in
          freebsd) setfacl -m other::4 tmpfile0 ;;
          solaris) chmod o+r tmpfile0 ;;
          *)       setfacl -m other:4 tmpfile0 ;;
        esac

        func_test_copy tmpfile0 tmpfile4

        # Remove the ACL for the user.
        case $acl_flavor in
          linux)   setfacl -x user:$auid tmpfile0 ;;
          freebsd) setfacl -x user:$auid:1 tmpfile0 ;;
          *)       setfacl -d user:$auid:1 tmpfile0 ;;
        esac

        func_test_copy tmpfile0 tmpfile5

        # Remove the ACL for other.
        case $acl_flavor in
          linux | solaris) ;; # impossible
          freebsd) setfacl -x other::4 tmpfile0 ;;
          *)       setfacl -d other:4 tmpfile0 ;;
        esac

        func_test_copy tmpfile0 tmpfile6

        # Remove the ACL for the group.
        case $acl_flavor in
          linux)   setfacl -x group:$agid tmpfile0 ;;
          freebsd) setfacl -x group:$agid:4 tmpfile0 ;;
          *)       setfacl -d group:$agid:4 tmpfile0 ;;
        esac

        func_test_copy tmpfile0 tmpfile7

        # Delete all optional ACLs.
        case $acl_flavor in
          linux | freebsd)
            setfacl -m user:$auid:1 tmpfile0
            setfacl -b tmpfile0
            ;;
          *)
            setfacl -s user::6,group::0,other:0 tmpfile0 ;;
        esac

        func_test_copy tmpfile0 tmpfile8

        # Copy ACLs from a file that has no ACLs.
        echo > tmpfile9
        chmod a+x tmpfile9
        case $acl_flavor in
          linux)   getfacl tmpfile9 | setfacl --set-file=- tmpfile0 ;;
          freebsd) ;;
          *)       getfacl tmpfile9 | setfacl -f - tmpfile0 ;;
        esac
        rm -f tmpfile9

        func_test_copy tmpfile0 tmpfile9

        ;;

      cygwin)

        # Set an ACL for a group.
        setfacl -m group:0:1 tmpfile0

        func_test_copy tmpfile0 tmpfile2

        # Set an ACL for other.
        setfacl -m other:4 tmpfile0

        func_test_copy tmpfile0 tmpfile4

        # Remove the ACL for the group.
        setfacl -d group:0 tmpfile0

        func_test_copy tmpfile0 tmpfile5

        # Remove the ACL for other.
        setfacl -d other:4 tmpfile0

        func_test_copy tmpfile0 tmpfile6

        # Delete all optional ACLs.
        setfacl -s user::6,group::0,other:0 tmpfile0

        func_test_copy tmpfile0 tmpfile8

        # Copy ACLs from a file that has no ACLs.
        echo > tmpfile9
        chmod a+x tmpfile9
        getfacl tmpfile9 | setfacl -f - tmpfile0
        rm -f tmpfile9

        func_test_copy tmpfile0 tmpfile9

        ;;

      hpux)

        # Set an ACL for a user.
        orig=`lsacl tmpfile0 | sed -e 's/ tmpfile0$//'`
        chacl -r "${orig}($auid.%,--x)" tmpfile0

        func_test_copy tmpfile0 tmpfile2

        # Set an ACL for a group.
        orig=`lsacl tmpfile0 | sed -e 's/ tmpfile0$//'`
        chacl -r "${orig}(%.$agid,r--)" tmpfile0

        func_test_copy tmpfile0 tmpfile3

        # Set an ACL for other.
        orig=`lsacl tmpfile0 | sed -e 's/ tmpfile0$//'`
        chacl -r "${orig}(%.%,r--)" tmpfile0

        func_test_copy tmpfile0 tmpfile4

        # Remove the ACL for the user.
        chacl -d "($auid.%,--x)" tmpfile0

        func_test_copy tmpfile0 tmpfile5

        # Remove the ACL for the group.
        chacl -d "(%.$agid,r--)" tmpfile0

        func_test_copy tmpfile0 tmpfile6

        # Delete all optional ACLs.
        chacl -z tmpfile0

        func_test_copy tmpfile0 tmpfile8

        # Copy ACLs from a file that has no ACLs.
        echo > tmpfile9
        chmod a+x tmpfile9
        orig=`lsacl tmpfile9 | sed -e 's/ tmpfile9$//'`
        rm -f tmpfile9
        chacl -r "${orig}" tmpfile0

        func_test_copy tmpfile0 tmpfile9

        ;;

      hpuxjfs)

        # Set an ACL for a user.
        orig=`lsacl tmpfile0 | sed -e 's/ tmpfile0$//'`
        chacl -r "${orig}($auid.%,--x)" tmpfile0 \
          || setacl -m user:$auid:1 tmpfile0

        func_test_copy tmpfile0 tmpfile2

        # Set an ACL for a group.
        orig=`lsacl tmpfile0 | sed -e 's/ tmpfile0$//'`
        chacl -r "${orig}(%.$agid,r--)" tmpfile0 \
          || setacl -m group:$agid:4 tmpfile0

        func_test_copy tmpfile0 tmpfile3

        # Set an ACL for other.
        orig=`lsacl tmpfile0 | sed -e 's/ tmpfile0$//'`
        chacl -r "${orig}(%.%,r--)" tmpfile0 \
          || setacl -m other:4 tmpfile0

        func_test_copy tmpfile0 tmpfile4

        # Remove the ACL for the user.
        chacl -d "($auid.%,--x)" tmpfile0 \
          || setacl -d user:$auid tmpfile0

        func_test_copy tmpfile0 tmpfile5

        # Remove the ACL for the group.
        chacl -d "(%.$agid,r--)" tmpfile0 \
          || setacl -d group:$agid tmpfile0

        func_test_copy tmpfile0 tmpfile6

        # Delete all optional ACLs.
        chacl -z tmpfile0 \
          || { setacl -m user:$auid:1 tmpfile0
               setacl -s user::6,group::0,class:7,other:0 tmpfile0
             }

        func_test_copy tmpfile0 tmpfile8

        # Copy ACLs from a file that has no ACLs.
        echo > tmpfile9
        chmod a+x tmpfile9
        orig=`lsacl tmpfile9 | sed -e 's/ tmpfile9$//'`
        getacl tmpfile9 > tmpaclout0
        rm -f tmpfile9
        chacl -r "${orig}" tmpfile0 \
          || setacl -f tmpaclout0 tmpfile0

        func_test_copy tmpfile0 tmpfile9

        ;;

      osf1)

        # Set an ACL for a user.
        setacl -u user:$auid:1 tmpfile0

        func_test_copy tmpfile0 tmpfile2

        # Set an ACL for a group.
        setacl -u group:$agid:4 tmpfile0

        func_test_copy tmpfile0 tmpfile3

        # Set an ACL for other.
        setacl -u other::4 tmpfile0

        func_test_copy tmpfile0 tmpfile4

        # Remove the ACL for the user.
        setacl -x user:$auid:1 tmpfile0

        func_test_copy tmpfile0 tmpfile5

        if false; then # would give an error "can't set ACL: Invalid argument"
          # Remove the ACL for other.
          setacl -x other::4 tmpfile0

          func_test_copy tmpfile0 tmpfile6
        fi

        # Remove the ACL for the group.
        setacl -x group:$agid:4 tmpfile0

        func_test_copy tmpfile0 tmpfile7

        # Delete all optional ACLs.
        setacl -u user:$auid:1 tmpfile0
        setacl -b tmpfile0

        func_test_copy tmpfile0 tmpfile8

        # Copy ACLs from a file that has no ACLs.
        echo > tmpfile9
        chmod a+x tmpfile9
        getacl tmpfile9 > tmpaclout0
        setacl -b -U tmpaclout0 tmpfile0
        rm -f tmpfile9

        func_test_copy tmpfile0 tmpfile9

        ;;

      nsk)

        # Set an ACL for a user.
        setacl -m user:$auid:1 tmpfile0

        func_test_copy tmpfile0 tmpfile2

        # Set an ACL for a group.
        setacl -m group:$agid:4 tmpfile0

        func_test_copy tmpfile0 tmpfile3

        # Set an ACL for other.
        setacl -m other:4 tmpfile0

        func_test_copy tmpfile0 tmpfile4

        # Remove the ACL for the user.
        setacl -d user:$auid tmpfile0

        func_test_copy tmpfile0 tmpfile5

        # Remove the ACL for the group.
        setacl -d group:$agid tmpfile0

        func_test_copy tmpfile0 tmpfile6

        # Delete all optional ACLs.
        setacl -m user:$auid:1 tmpfile0
        setacl -s user::6,group::0,class:7,other:0 tmpfile0

        func_test_copy tmpfile0 tmpfile8

        # Copy ACLs from a file that has no ACLs.
        echo > tmpfile9
        chmod a+x tmpfile9
        getacl tmpfile9 > tmpaclout0
        setacl -f tmpaclout0 tmpfile0
        rm -f tmpfile9

        func_test_copy tmpfile0 tmpfile9

        ;;

      aix)

        # Set an ACL for a user.
        { aclget tmpfile0 | sed -e 's/disabled$/enabled/'; echo "        permit --x u:$auid"; } | aclput tmpfile0

        func_test_copy tmpfile0 tmpfile2

        # Set an ACL for a group.
        { aclget tmpfile0 | sed -e 's/disabled$/enabled/'; echo "        permit r-- g:$agid"; } | aclput tmpfile0

        func_test_copy tmpfile0 tmpfile3

        # Set an ACL for other.
        chmod o+r tmpfile0

        func_test_copy tmpfile0 tmpfile4

        # Remove the ACL for the user.
        aclget tmpfile0 | grep -v ' u:[^ ]*$' | aclput tmpfile0

        func_test_copy tmpfile0 tmpfile5

        # Remove the ACL for the group.
        aclget tmpfile0 | grep -v ' g:[^ ]*$' | aclput tmpfile0

        func_test_copy tmpfile0 tmpfile7

        # Delete all optional ACLs.
        aclget tmpfile0 | sed -e 's/enabled$/disabled/' | sed -e '/disabled$/q' | aclput tmpfile0

        func_test_copy tmpfile0 tmpfile8

        # Copy ACLs from a file that has no ACLs.
        echo > tmpfile9
        chmod a+x tmpfile9
        aclget tmpfile9 | aclput tmpfile0
        rm -f tmpfile9

        func_test_copy tmpfile0 tmpfile9

        ;;

      macosx)

        # Set an ACL for a user.
        /bin/chmod +a "user:daemon allow execute" tmpfile0

        func_test_copy tmpfile0 tmpfile2

        # Set an ACL for a group.
        /bin/chmod +a "group:daemon allow read" tmpfile0

        func_test_copy tmpfile0 tmpfile3

        # Set an ACL for other.
        chmod o+r tmpfile0

        func_test_copy tmpfile0 tmpfile4

        # Remove the ACL for the user.
        /bin/chmod -a "user:daemon allow execute" tmpfile0

        func_test_copy tmpfile0 tmpfile5

        # Remove the ACL for the group.
        /bin/chmod -a "group:daemon allow read" tmpfile0

        func_test_copy tmpfile0 tmpfile7

        # Delete all optional ACLs.
        /bin/chmod -N tmpfile0

        func_test_copy tmpfile0 tmpfile8

        # Copy ACLs from a file that has no ACLs.
        echo > tmpfile9
        chmod a+x tmpfile9
        { /bin/ls -le tmpfile9 | sed -n -e 's/^ [0-9][0-9]*: //p'; echo; } | /bin/chmod -E tmpfile0
        rm -f tmpfile9

        func_test_copy tmpfile0 tmpfile9

        ;;

      irix)

        # Set an ACL for a user.
        /sbin/chacl user::rw-,group::---,other::---,user:$auid:--x tmpfile0

        func_test_copy tmpfile0 tmpfile2

        # Set an ACL for a group.
        /sbin/chacl user::rw-,group::---,other::---,user:$auid:--x,group:$agid:r-- tmpfile0

        func_test_copy tmpfile0 tmpfile3

        # Set an ACL for other.
        /sbin/chacl user::rw-,group::---,user:$auid:--x,group:$agid:r--,other::r-- tmpfile0

        func_test_copy tmpfile0 tmpfile4

        # Remove the ACL for the user.
        /sbin/chacl user::rw-,group::---,group:$agid:r--,other::r-- tmpfile0

        func_test_copy tmpfile0 tmpfile5

        # Remove the ACL for the group.
        /sbin/chacl user::rw-,group::---,other::r-- tmpfile0

        func_test_copy tmpfile0 tmpfile7

        ;;

    esac
  fi

  rm -f tmpfile[0-9] tmpaclout[0-2]
) || exit 1

rm -rf "$tmp"
exit 0
