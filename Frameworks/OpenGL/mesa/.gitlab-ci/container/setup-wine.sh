#!/usr/bin/env bash

export WINEPREFIX="$1"
export WINEDEBUG="-all"

# We don't want crash dialogs
cat >crashdialog.reg <<EOF
Windows Registry Editor Version 5.00

[HKEY_CURRENT_USER\Software\Wine\WineDbg]
"ShowCrashDialog"=dword:00000000

EOF

# Set the wine prefix and disable the crash dialog
wine regedit crashdialog.reg
rm crashdialog.reg

# An immediate wine command may fail with: "${WINEPREFIX}: Not a
# valid wine prefix."  and that is just spit because of checking
# the existance of the system.reg file, which fails.  Just giving
# it a bit more of time for it to be created solves the problem
# ...
while ! test -f  "${WINEPREFIX}/system.reg"; do sleep 1; done
