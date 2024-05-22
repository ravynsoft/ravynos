#!/bin/bash
# fontconfig/test/run-test.sh
#
# Copyright © 2000 Keith Packard
#
# Permission to use, copy, modify, distribute, and sell this software and its
# documentation for any purpose is hereby granted without fee, provided that
# the above copyright notice appear in all copies and that both that
# copyright notice and this permission notice appear in supporting
# documentation, and that the name of the author(s) not be used in
# advertising or publicity pertaining to distribution of the software without
# specific, written prior permission.  The authors make no
# representations about the suitability of this software for any purpose.  It
# is provided "as is" without express or implied warranty.
#
# THE AUTHOR(S) DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
# EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
# DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
# TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE.
set -e

: "${TMPDIR=/tmp}"

case "$OSTYPE" in
    msys ) MyPWD=$(pwd -W) ;;  # On Msys/MinGW, returns a MS Windows style path.
    *    ) MyPWD=$(pwd)    ;;  # On any other platforms, returns a Unix style path.
esac

TESTDIR=${srcdir-"$MyPWD"}
BUILDTESTDIR=${builddir-"$MyPWD"}

BASEDIR=$(mktemp -d "$TMPDIR"/fontconfig.XXXXXXXX)
FONTDIR="$BASEDIR"/fonts
CACHEDIR="$BASEDIR"/cache.dir
EXPECTED=${EXPECTED-"out.expected"}

FCLIST="$LOG_COMPILER ../fc-list/fc-list$EXEEXT"
FCCACHE="$LOG_COMPILER ../fc-cache/fc-cache$EXEEXT"

if [ -x "$(command -v bwrap)" ]; then
    BWRAP="$(command -v bwrap)"
fi

FONT1=$TESTDIR/4x6.pcf
FONT2=$TESTDIR/8x16.pcf
TEST=""

clean_exit() {
    rc=$?
    trap - INT TERM ABRT EXIT
    if [ "x$TEST" != "x" ]; then
        echo "Aborting from '$TEST' with the exit code $rc"
    fi
    exit $rc
}
trap clean_exit INT TERM ABRT EXIT

check () {
    {
	$FCLIST - family pixelsize | sort;
	echo "=";
	$FCLIST - family pixelsize | sort;
	echo "=";
	$FCLIST - family pixelsize | sort;
    } > out
  tr -d '\015' <out >out.tmp; mv out.tmp out
  if cmp out "$BUILDTESTDIR"/"$EXPECTED" > /dev/null ; then : ; else
    echo "*** Test failed: $TEST"
    echo "*** output is in 'out', expected output in '$EXPECTED'"
    exit 1
  fi
  rm -f out
}

prep() {
  rm -rf "$CACHEDIR"
  rm -rf "$FONTDIR"
  mkdir "$FONTDIR"
}

dotest () {
  TEST=$1
  test x"$VERBOSE" = x || echo "Running: $TEST"
}

sed "s!@FONTDIR@!$FONTDIR!
s!@REMAPDIR@!!
s!@CACHEDIR@!$CACHEDIR!" < "$TESTDIR"/fonts.conf.in > fonts.conf

FONTCONFIG_FILE="$MyPWD"/fonts.conf
export FONTCONFIG_FILE

dotest "Basic check"
prep
cp "$FONT1" "$FONT2" "$FONTDIR"
if [ -n "${SOURCE_DATE_EPOCH:-}" ] && [ ${#SOURCE_DATE_EPOCH} -gt 0 ]; then
    touch -m -t "$(date -d @"${SOURCE_DATE_EPOCH}" +%y%m%d%H%M.%S)" "$FONTDIR"
fi
check

dotest "With a subdir"
prep
cp "$FONT1" "$FONT2" "$FONTDIR"
if [ -n "${SOURCE_DATE_EPOCH:-}" ] && [ ${#SOURCE_DATE_EPOCH} -gt 0 ]; then
    touch -m -t "$(date -d @"${SOURCE_DATE_EPOCH}" +%y%m%d%H%M.%S)" "$FONTDIR"
fi
$FCCACHE "$FONTDIR"
check

dotest "Subdir with a cache file"
prep
mkdir "$FONTDIR"/a
cp "$FONT1" "$FONT2" "$FONTDIR"/a
if [ -n "${SOURCE_DATE_EPOCH:-}" ] && [ ${#SOURCE_DATE_EPOCH} -gt 0 ]; then
    touch -m -t "$(date -d @"${SOURCE_DATE_EPOCH}" +%y%m%d%H%M.%S)" "$FONTDIR"/a
fi
$FCCACHE "$FONTDIR"/a
check

dotest "Complicated directory structure"
prep
mkdir "$FONTDIR"/a
mkdir "$FONTDIR"/a/a
mkdir "$FONTDIR"/b
mkdir "$FONTDIR"/b/a
cp "$FONT1" "$FONTDIR"/a
if [ -n "${SOURCE_DATE_EPOCH:-}" ] && [ ${#SOURCE_DATE_EPOCH} -gt 0 ]; then
    touch -m -t "$(date -d @"${SOURCE_DATE_EPOCH}" +%y%m%d%H%M.%S)" "$FONTDIR"/a
fi
cp "$FONT2" "$FONTDIR"/b/a
if [ -n "${SOURCE_DATE_EPOCH:-}" ] && [ ${#SOURCE_DATE_EPOCH} -gt 0 ]; then
    touch -m -t "$(date -d @"${SOURCE_DATE_EPOCH}" +%y%m%d%H%M.%S)" "$FONTDIR"/b/a
fi
check

dotest "Subdir with an out-of-date cache file"
prep
mkdir "$FONTDIR"/a
$FCCACHE "$FONTDIR"/a
sleep 1
cp "$FONT1" "$FONT2" "$FONTDIR"/a
check

dotest "Dir with an out-of-date cache file"
prep
cp "$FONT1" "$FONTDIR"
$FCCACHE "$FONTDIR"
sleep 1
mkdir "$FONTDIR"/a
cp "$FONT2" "$FONTDIR"/a
check

dotest "Keep mtime of the font directory"
prep
cp "$FONT1" "$FONTDIR"
touch -d @0 "$FONTDIR"
stat -c '%y' "$FONTDIR" > out1
$FCCACHE "$FONTDIR"
stat -c '%y' "$FONTDIR" > out2
if cmp out1 out2 > /dev/null ; then : ; else
    echo "*** Test failed: $TEST"
    echo "mtime was modified"
    exit 1
fi

if [ x"$BWRAP" != "x" ] && [ "x$EXEEXT" = "x" ]; then
dotest "Basic functionality with the bind-mounted cache dir"
prep
cp "$FONT1" "$FONT2" "$FONTDIR"
if [ -n "${SOURCE_DATE_EPOCH:-}" ] && [ ${#SOURCE_DATE_EPOCH} -gt 0 ]; then
    touch -m -t "$(date -d @"${SOURCE_DATE_EPOCH}" +%y%m%d%H%M.%S)" "$FONTDIR"
fi
$FCCACHE "$FONTDIR"
sleep 1
ls -l "$CACHEDIR" > out1
TESTTMPDIR=$(mktemp -d "$TMPDIR"/fontconfig.XXXXXXXX)
# Once font dir is remapped, we could use $FONTDIR as different one in theory.
# but we don't use it here and to avoid duplicate entries, set the non-existing
# directory here.
sed "s!@FONTDIR@!$FONTDIR/a!
s!@REMAPDIR@!<remap-dir as-path="'"'"$FONTDIR"'"'">$TESTTMPDIR/fonts</remap-dir>!
s!@CACHEDIR@!$TESTTMPDIR/cache.dir!" < "$TESTDIR"/fonts.conf.in > bind-fonts.conf
$BWRAP --bind / / --bind "$CACHEDIR" "$TESTTMPDIR"/cache.dir --bind "$FONTDIR" "$TESTTMPDIR"/fonts --bind .. "$TESTTMPDIR"/build --dev-bind /dev /dev --setenv FONTCONFIG_FILE "$TESTTMPDIR"/build/test/bind-fonts.conf "$TESTTMPDIR"/build/fc-match/fc-match"$EXEEXT" -f "%{file}\n" ":foundry=Misc" > xxx
$BWRAP --bind / / --bind "$CACHEDIR" "$TESTTMPDIR"/cache.dir --bind "$FONTDIR" "$TESTTMPDIR"/fonts --bind .. "$TESTTMPDIR"/build --dev-bind /dev /dev --setenv FONTCONFIG_FILE "$TESTTMPDIR"/build/test/bind-fonts.conf "$TESTTMPDIR"/build/test/test-bz106618"$EXEEXT" | sort > flist1
$BWRAP --bind / / --bind "$CACHEDIR" "$TESTTMPDIR"/cache.dir --bind "$FONTDIR" "$TESTTMPDIR"/fonts --bind .. "$TESTTMPDIR"/build --dev-bind /dev /dev find "$TESTTMPDIR"/fonts/ -type f -name '*.pcf' | sort > flist2
ls -l "$CACHEDIR" > out2
if cmp out1 out2 > /dev/null ; then : ; else
  echo "*** Test failed: $TEST"
  echo "cache was created/updated."
  echo "Before:"
  cat out1
  echo "After:"
  cat out2
  exit 1
fi
if [ x"$(cat xxx)" != "x$TESTTMPDIR/fonts/4x6.pcf" ]; then
  echo "*** Test failed: $TEST"
  echo "file property doesn't point to the new place: $TESTTMPDIR/fonts/4x6.pcf"
  exit 1
fi
if cmp flist1 flist2 > /dev/null ; then : ; else
  echo "*** Test failed: $TEST"
  echo "file properties doesn't point to the new places"
  echo "Expected result:"
  cat flist2
  echo "Actual result:"
  cat flist1
  exit 1
fi
rm -rf "$TESTTMPDIR" out1 out2 xxx flist1 flist2 bind-fonts.conf

dotest "Different directory content between host and sandbox"
prep
cp "$FONT1" "$FONTDIR"
if [ -n "${SOURCE_DATE_EPOCH:-}" ] && [ ${#SOURCE_DATE_EPOCH} -gt 0 ]; then
    touch -m -t "$(date -d @"${SOURCE_DATE_EPOCH}" +%y%m%d%H%M.%S)" "$FONTDIR"
fi
$FCCACHE "$FONTDIR"
sleep 1
ls -1 --color=no "$CACHEDIR"/*cache*> out1
stat -c '%n %s %y %z' "$(cat out1)" > stat1
TESTTMPDIR=$(mktemp -d "$TMPDIR"/fontconfig.XXXXXXXX)
TESTTMP2DIR=$(mktemp -d "$TMPDIR"/fontconfig.XXXXXXXX)
cp "$FONT2" "$TESTTMP2DIR"
if [ -n "${SOURCE_DATE_EPOCH:-}" ] && [ ${#SOURCE_DATE_EPOCH} -gt 0 ]; then
    touch -m -t "$(date -d @"${SOURCE_DATE_EPOCH}" +%y%m%d%H%M.%S)" "$TESTTMP2DIR"
fi
sed "s!@FONTDIR@!$TESTTMPDIR/fonts</dir><dir salt="'"'"salt-to-make-different"'"'">$FONTDIR!
s!@REMAPDIR@!<remap-dir as-path="'"'"$FONTDIR"'"'">$TESTTMPDIR/fonts</remap-dir>!
s!@CACHEDIR@!$TESTTMPDIR/cache.dir!" < "$TESTDIR"/fonts.conf.in > bind-fonts.conf
$BWRAP --bind / / --bind "$CACHEDIR" "$TESTTMPDIR"/cache.dir --bind "$FONTDIR" "$TESTTMPDIR"/fonts --bind "$TESTTMP2DIR" "$FONTDIR" --bind .. "$TESTTMPDIR"/build --dev-bind /dev /dev --setenv FONTCONFIG_FILE "$TESTTMPDIR"/build/test/bind-fonts.conf "$TESTTMPDIR"/build/fc-match/fc-match"$EXEEXT" -f "%{file}\n" ":foundry=Misc" > xxx
$BWRAP --bind / / --bind "$CACHEDIR" "$TESTTMPDIR"/cache.dir --bind "$FONTDIR" "$TESTTMPDIR"/fonts --bind "$TESTTMP2DIR" "$FONTDIR" --bind .. "$TESTTMPDIR"/build --dev-bind /dev /dev --setenv FONTCONFIG_FILE "$TESTTMPDIR"/build/test/bind-fonts.conf "$TESTTMPDIR"/build/test/test-bz106618"$EXEEXT" | sort > flist1
$BWRAP --bind / / --bind "$CACHEDIR" "$TESTTMPDIR"/cache.dir --bind "$FONTDIR" "$TESTTMPDIR"/fonts --bind "$TESTTMP2DIR" "$FONTDIR" --bind .. "$TESTTMPDIR"/build --dev-bind /dev /dev find "$TESTTMPDIR"/fonts/ -type f -name '*.pcf' | sort > flist2
ls -1 --color=no "$CACHEDIR"/*cache* > out2
stat -c '%n %s %y %z' "$(cat out1)" > stat2
if cmp stat1 stat2 > /dev/null ; then : ; else
  echo "*** Test failed: $TEST"
  echo "cache was created/updated."
  cat stat1 stat2
  exit 1
fi
if grep -v -- "$(cat out1)" out2 > /dev/null ; then : ; else
  echo "*** Test failed: $TEST"
  echo "cache wasn't created for dir inside sandbox."
  cat out1 out2
  exit 1
fi
if [ x"$(cat xxx)" != "x$TESTTMPDIR/fonts/4x6.pcf" ]; then
  echo "*** Test failed: $TEST"
  echo "file property doesn't point to the new place: $TESTTMPDIR/fonts/4x6.pcf"
  exit 1
fi
if cmp flist1 flist2 > /dev/null ; then
  echo "*** Test failed: $TEST"
  echo "Missing fonts should be available on sandbox"
  echo "Expected result:"
  cat flist2
  echo "Actual result:"
  cat flist1
  exit 1
fi
rm -rf "$TESTTMPDIR" "$TESTTMP2DIR" out1 out2 xxx flist1 flist2 stat1 stat2 bind-fonts.conf

dotest "Check consistency of MD5 in cache name"
prep
mkdir -p "$FONTDIR"/sub
cp "$FONT1" "$FONTDIR"/sub
if [ -n "${SOURCE_DATE_EPOCH:-}" ] && [ ${#SOURCE_DATE_EPOCH} -gt 0 ]; then
    touch -m -t "$(date -d @"${SOURCE_DATE_EPOCH}" +%y%m%d%H%M.%S)" "$FONTDIR"/sub
fi
$FCCACHE "$FONTDIR"
sleep 1
(cd "$CACHEDIR"; ls -1 --color=no ./*cache*) > out1
TESTTMPDIR=$(mktemp -d "$TMPDIR"/fontconfig.XXXXXXXX)
mkdir -p "$TESTTMPDIR"/cache.dir
# Once font dir is remapped, we could use $FONTDIR as different one in theory.
# but we don't use it here and to avoid duplicate entries, set the non-existing
# directory here.
sed "s!@FONTDIR@!$FONTDIR/a!
s!@REMAPDIR@!<remap-dir as-path="'"'"$FONTDIR"'"'">$TESTTMPDIR/fonts</remap-dir>!
s!@CACHEDIR@!$TESTTMPDIR/cache.dir!" < "$TESTDIR"/fonts.conf.in > bind-fonts.conf
$BWRAP --bind / / --bind "$FONTDIR" "$TESTTMPDIR"/fonts --bind .. "$TESTTMPDIR"/build --dev-bind /dev /dev --setenv FONTCONFIG_FILE "$TESTTMPDIR"/build/test/bind-fonts.conf "$TESTTMPDIR"/build/fc-cache/fc-cache"$EXEEXT" "$TESTTMPDIR"/fonts
(cd "$TESTTMPDIR"/cache.dir; ls -1 --color=no ./*cache*) > out2
if cmp out1 out2 > /dev/null ; then : ; else
    echo "*** Test failed: $TEST"
    echo "cache was created unexpectedly."
    echo "Before:"
    cat out1
    echo "After:"
    cat out2
    exit 1
fi
rm -rf "$TESTTMPDIR" out1 out2 bind-fonts.conf

dotest "Fallback to uuid"
prep
cp "$FONT1" "$FONTDIR"
if [ -n "${SOURCE_DATE_EPOCH:-}" ] && [ ${#SOURCE_DATE_EPOCH} -gt 0 ]; then
    touch -m -t "$(date -d @"${SOURCE_DATE_EPOCH}" +%y%m%d%H%M.%S)" "$FONTDIR"
fi
touch -d @"$(stat -c %Y "$FONTDIR")" "$FONTDIR"
$FCCACHE "$FONTDIR"
sleep 1
_cache=$(ls -1 --color=no "$CACHEDIR"/*cache*)
_mtime=$(stat -c %Y "$FONTDIR")
_uuid=$(uuidgen)
_newcache=$(echo "$_cache" | sed "s/\([0-9a-f]*\)\(\-.*\)/$_uuid\2/")
mv "$_cache" "$_newcache"
echo "$_uuid" > "$FONTDIR"/.uuid
touch -d @"$_mtime" "$FONTDIR"
(cd "$CACHEDIR"; ls -1 --color=no ./*cache*) > out1
TESTTMPDIR=$(mktemp -d "$TMPDIR"/fontconfig.XXXXXXXX)
mkdir -p "$TESTTMPDIR"/cache.dir
sed "s!@FONTDIR@!$TESTTMPDIR/fonts!
s!@REMAPDIR@!<remap-dir as-path="'"'"$FONTDIR"'"'">$TESTTMPDIR/fonts</remap-dir>!
s!@CACHEDIR@!$TESTTMPDIR/cache.dir!" < "$TESTDIR"/fonts.conf.in > bind-fonts.conf
$BWRAP --bind / / --bind "$CACHEDIR" "$TESTTMPDIR"/cache.dir --bind "$FONTDIR" "$TESTTMPDIR"/fonts --bind .. "$TESTTMPDIR"/build --dev-bind /dev /dev --setenv FONTCONFIG_FILE "$TESTTMPDIR"/build/test/bind-fonts.conf "$TESTTMPDIR"/build/fc-match/fc-match"$EXEEXT" -f ""
(cd "$CACHEDIR"; ls -1 --color=no ./*cache*) > out2
if cmp out1 out2 > /dev/null ; then : ; else
    echo "*** Test failed: $TEST"
    echo "cache was created unexpectedly."
    echo "Before:"
    cat out1
    echo "After:"
    cat out2
    exit 1
fi
rm -rf "$TESTTMPDIR" out1 out2 bind-fonts.conf

else
    echo "No bubblewrap installed. skipping..."
fi # if [ x"$BWRAP" != "x" -a "x$EXEEXT" = "x" ]

if [ "x$EXEEXT" = "x" ]; then
dotest "sysroot option"
prep
mkdir -p "$MyPWD"/sysroot/"$FONTDIR"
mkdir -p "$MyPWD"/sysroot/"$CACHEDIR"
mkdir -p "$MyPWD"/sysroot/"$MyPWD"
cp "$FONT1" "$MyPWD"/sysroot/"$FONTDIR"
if [ -n "${SOURCE_DATE_EPOCH:-}" ] && [ ${#SOURCE_DATE_EPOCH} -gt 0 ]; then
    touch -m -t "$(date -d @"${SOURCE_DATE_EPOCH}" +%y%m%d%H%M.%S)" "$MyPWD"/sysroot/"$FONTDIR"
fi
cp "$MyPWD"/fonts.conf "$MyPWD"/sysroot/"$MyPWD"/fonts.conf
$FCCACHE -y "$MyPWD"/sysroot

dotest "creating cache file on sysroot"
md5=$(printf "%s" "$FONTDIR" | md5sum | sed 's/ .*$//')
echo "checking for cache file $md5"
if ! ls "$MyPWD/sysroot/$CACHEDIR/$md5"*; then
  echo "*** Test failed: $TEST"
  echo "No cache for $FONTDIR ($md5)"
  ls "$MyPWD"/sysroot/"$CACHEDIR"
  exit 1
fi

rm -rf "$MyPWD"/sysroot

dotest "read newer caches when multiple places are allowed to store"
prep
cp "$FONT1" "$FONT2" "$FONTDIR"
if [ -n "${SOURCE_DATE_EPOCH:-}" ] && [ ${#SOURCE_DATE_EPOCH} -gt 0 ]; then
    touch -m -t "$(date -d @"${SOURCE_DATE_EPOCH}" +%y%m%d%H%M.%S)" "$FONTDIR"
fi
MYCACHEBASEDIR=$(mktemp -d "$TMPDIR"/fontconfig.XXXXXXXX)
MYCACHEDIR="$MYCACHEBASEDIR"/cache.dir
MYOWNCACHEDIR="$MYCACHEBASEDIR"/owncache.dir
MYCONFIG=$(mktemp "$TMPDIR"/fontconfig.XXXXXXXX)

mkdir -p "$MYCACHEDIR"
mkdir -p "$MYOWNCACHEDIR"

sed "s!@FONTDIR@!$FONTDIR!
s!@REMAPDIR@!!
s!@CACHEDIR@!$MYCACHEDIR!" < "$TESTDIR"/fonts.conf.in > my-fonts.conf

FONTCONFIG_FILE="$MyPWD"/my-fonts.conf $FCCACHE "$FONTDIR"

sleep 1
cat<<EOF>"$MYCONFIG"
<fontconfig>
  <match target="scan">
    <test name="file"><string>$FONTDIR/4x6.pcf</string></test>
    <edit name="pixelsize"><int>8</int></edit>
  </match>
</fontconfig>
EOF
sed "s!@FONTDIR@!$FONTDIR!
s!@REMAPDIR@!<include ignore_missing=\"yes\">$MYCONFIG</include>!
s!@CACHEDIR@!$MYOWNCACHEDIR!" < "$TESTDIR"/fonts.conf.in > my-fonts.conf

if [ -n "${SOURCE_DATE_EPOCH:-}" ]; then
  old_epoch=${SOURCE_DATE_EPOCH}
  SOURCE_DATE_EPOCH=$(("$SOURCE_DATE_EPOCH" + 1))
fi
FONTCONFIG_FILE="$MyPWD"/my-fonts.conf $FCCACHE -f "$FONTDIR"
if [ -n "${SOURCE_DATE_EPOCH:-}" ]; then
  SOURCE_DATE_EPOCH=${old_epoch}
fi

sed "s!@FONTDIR@!$FONTDIR!
s!@REMAPDIR@!<include ignore_missing=\"yes\">$MYCONFIG</include>!
s!@CACHEDIR@!$MYCACHEDIR</cachedir><cachedir>$MYOWNCACHEDIR!" < "$TESTDIR"/fonts.conf.in > my-fonts.conf

{
    FONTCONFIG_FILE="$MyPWD"/my-fonts.conf $FCLIST - family pixelsize | sort;
    echo "=";
    FONTCONFIG_FILE="$MyPWD"/my-fonts.conf $FCLIST - family pixelsize | sort;
    echo "=";
    FONTCONFIG_FILE="$MyPWD"/my-fonts.conf $FCLIST - family pixelsize | sort;
} > my-out
tr -d '\015' <my-out >my-out.tmp; mv my-out.tmp my-out
sed -e 's/pixelsize=6/pixelsize=8/g' "$BUILDTESTDIR"/"$EXPECTED" > my-out.expected

if cmp my-out my-out.expected > /dev/null ; then : ; else
    echo "*** Test failed: $TEST"
    echo "*** output is in 'my-out', expected output in 'my-out.expected'"
    echo "Actual Result"
    cat my-out
    echo "Expected Result"
    cat my-out.expected
    exit 1
fi

rm -rf "$MYCACHEBASEDIR" "$MYCONFIG" my-fonts.conf my-out my-out.expected

fi # if [ "x$EXEEXT" = "x" ]

if [ -x "$BUILDTESTDIR"/test-crbug1004254 ]; then
    dotest "MT-safe global config"
    prep
    curl -s -o "$FONTDIR"/noto.zip https://noto-website-2.storage.googleapis.com/pkgs/NotoSans-hinted.zip
    (cd "$FONTDIR"; unzip noto.zip)
    if [ -n "${SOURCE_DATE_EPOCH:-}" ] && [ ${#SOURCE_DATE_EPOCH} -gt 0 ]; then
	touch -m -t "$(date -d @"${SOURCE_DATE_EPOCH}" +%y%m%d%H%M.%S)" "$FONTDIR"
    fi
    "$BUILDTESTDIR"/test-crbug1004254
else
    echo "No test-crbug1004254: skipped"
fi

if [ "x$EXEEXT" = "x" ]; then

dotest "empty XDG_CACHE_HOME"
prep
export XDG_CACHE_HOME=""
export old_HOME="$HOME"
export temp_HOME=$(mktemp -d "$TMPDIR"/fontconfig.XXXXXXXX)
export HOME="$temp_HOME"
cp "$FONT1" "$FONT2" "$FONTDIR"
if [ -n "${SOURCE_DATE_EPOCH:-}" ] && [ ${#SOURCE_DATE_EPOCH} -gt 0 ]; then
    touch -m -t "$(date -d @"${SOURCE_DATE_EPOCH}" +%y%m%d%H%M.%S)" "$FONTDIR"
fi
echo "<fontconfig><dir>$FONTDIR</dir><cachedir prefix=\"xdg\">fontconfig</cachedir></fontconfig>" > my-fonts.conf
FONTCONFIG_FILE="$MyPWD"/my-fonts.conf $FCCACHE "$FONTDIR" || :
if [ -d "$HOME"/.cache ] && [ -d "$HOME"/.cache/fontconfig ]; then : ; else
  echo "*** Test failed: $TEST"
  echo "No \$HOME/.cache/fontconfig directory"
  ls -a "$HOME"
  ls -a "$HOME"/.cache
  exit 1
fi

export HOME="$old_HOME"
rm -rf "$temp_HOME" my-fonts.conf
unset XDG_CACHE_HOME
unset old_HOME
unset temp_HOME

fi # if [ "x$EXEEXT" = "x" ]

rm -rf "$FONTDIR" "$CACHEFILE" "$CACHEDIR" "$BASEDIR" "$FONTCONFIG_FILE" out

TEST=""
