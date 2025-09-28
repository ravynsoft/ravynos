#
# We will just reuse the AIX hints since we support only building
# for the PASE and the PASE hints are merged with the AIX hints.
#

case "$PASE" in
'') cat >&4 <<EOF
***
*** This build process only works with the PASE environment (not ILE).
*** You must supply the -DPASE parameter to the Configure script,
*** please read the file README.os400.  Exiting now.
***
EOF
    exit 1
    ;;
*)  cat >&4 <<EOF
***
*** Using the AIX hints file, $src/hints/aix.sh.
***
EOF
    osname=aix
    . $src/hints/aix.sh
    ;;
esac
