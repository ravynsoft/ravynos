#!/bin/sh
#
# Test sudoers_locale early Defaults
#

: ${VISUDO=visudo}

LANG=C; export LANG
LC_NUMERIC=fr_FR.UTF-8; export LC_NUMERIC

# First check that visudo supports non-C locales
# Note that older versions of sudo did not set the locale
# until sudoers was read so this check will fail on them.
$VISUDO -csf - >/dev/null 2>&1 <<-EOF
	Defaults    sudoers_locale = fr_FR.UTF-8
	Defaults    passwd_timeout = "2,5"
	EOF

# Now make sure we can set passwd_timeout to a floating point value
# using a non-C locale.
if [ $? -eq 0 ]; then
    $VISUDO -csf - <<-EOF
	Defaults    passwd_timeout = "2.5"
	Defaults    sudoers_locale = fr_FR.UTF-8
	EOF
else
    # No support for LC_NUMERIC?
    echo "parse error in stdin near line 1"
    echo 'visudo: stdin:1: value "2.5" is invalid for option "passwd_timeout"' 1>&2
fi

exit 0
