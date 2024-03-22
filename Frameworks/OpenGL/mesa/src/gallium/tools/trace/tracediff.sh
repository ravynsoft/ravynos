#!/usr/bin/env bash
##########################################################################
#
# Copyright 2011 Jose Fonseca
# All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
##########################################################################/

set -e

PROGNAME="$(basename "$0")"
TRACEDUMP="${TRACEDUMP:-$(dirname "$0")/dump.py}"



###
### Helper functions
###
fatal()
{
  echo "ERROR: $1"
  exit 1
}


print_version()
{
  echo "TraceDiff - Compare two Gallium trace files"
  echo "(C) Copyright 2011 Jose Fonseca"
  echo ""
}


print_help()
{
  echo "Usage: ${PROGNAME} [options] <tracefile1> <tracefile2>"
  echo ""
  echo "  -h, --help         display this help and exit"
  echo "  -V, --version      output version information and exit"
  echo ""
  echo "  -m, --meld         use Meld for diffing (default is sdiff)"
  echo ""
  echo "dump.py options:"
  echo "  -N, --named        generate symbolic names for raw pointer values"
  echo "  -M, --method-only  output only call names without arguments"
  echo ""
  echo "sdiff options:"
  echo "  -d, --minimal      try hard to find a smaller set of changes"
  echo ""
}


do_cleanup()
{
  if test -d "$TEMPDIR"; then
    rm -rf "$TEMPDIR"
  fi
}


strip_dump()
{
  INFILE="$1"
  OUTFILE="$2"

  python3 "$TRACEDUMP" --plain --suppress --ignore-junk \
    "${DUMP_ARGS[@]}" "$INFILE" \
  | sed \
    -e 's/\r$//g' \
    -e 's/, /,\n\t/g' \
    -e 's/) = /)\n\t= /' \
  > "$OUTFILE"
}


###
### Main code starts
###
trap do_cleanup HUP INT TERM
DUMP_ARGS=()
SDIFF_ARGS=()
USE_MELD=0

while test -n "$1"
do
  case "$1" in
    --version|-V)
      print_version
      exit 0
      ;;
    --help|-h)
      print_version
      print_help
      exit 0
      ;;
    -N|--named|-M|--method-only)
      DUMP_ARGS+=("$1")
      shift
      ;;
    -d|--minimal)
      SDIFF_ARGS+=("$1")
      shift
      ;;
    -m|--meld)
      USE_MELD=1
      shift
      ;;
    *)
      if test "x$INFILE1" = "x"; then
        INFILE1="$1";
      elif test "x$INFILE2" = "x"; then
        INFILE2="$1";
      else
        fatal "Too many input filenames specified."
      fi
      shift
      ;;
  esac
done


if test "x$INFILE1" = "x" -o "x$INFILE2" = "x"; then
  print_help
  fatal "Not enough input file(s) specified!"
fi


TEMPDIR="$(mktemp -d)"
TEMP1="${TEMPDIR}/1"
TEMP2="${TEMPDIR}/2"

if test $USE_MELD -ne 0; then
  strip_dump "$INFILE1" "$TEMP1" "$@" || fatal "Could not dump '${INFILE1}."
  strip_dump "$INFILE2" "$TEMP2" "$@" || fatal "Could not dump '${INFILE2}."
  meld "$TEMP1" "$TEMP2"
else
  mkfifo "$TEMP1" || fatal "Could not create fifo 1"
  mkfifo "$TEMP2" || fatal "Could not create fifo 2"

  strip_dump "$INFILE1" "$TEMP1" "$@" &
  strip_dump "$INFILE2" "$TEMP2" "$@" &

  sdiff \
    --left-column \
    --width="$(tput cols)" \
    --speed-large-files \
    "${SDIFF_ARGS[@]}" \
    "$TEMP1" "$TEMP2" \
  | less
fi
