#/bin/sh
set -euo pipefail

XKBCONFIGROOT='/usr/share/X11/xkb'
XLOCALEDIR='/usr/share/X11/locale'

if [ ! -d test/data ]; then
    echo "Run this from the top source dir"
    exit 1
fi

for file in \
    symbols/terminate \
    symbols/in \
    symbols/keypad \
    symbols/altwin \
    symbols/ctrl \
    symbols/eurosign \
    symbols/inet \
    symbols/shift \
    symbols/pc \
    symbols/ca \
    symbols/cz \
    symbols/srvr_ctrl \
    symbols/capslock \
    symbols/latin \
    symbols/level5 \
    symbols/macintosh_vndr/apple \
    symbols/macintosh_vndr/us \
    symbols/us \
    symbols/nbsp \
    symbols/il \
    symbols/group \
    symbols/compose \
    symbols/level3 \
    symbols/ru \
    symbols/rupeesign \
    symbols/kpdl \
    symbols/de \
    symbols/ch \
    symbols/empty \
    keycodes/xfree86 \
    keycodes/aliases \
    keycodes/evdev \
    keycodes/empty \
    types/complete \
    types/pc \
    types/basic \
    types/iso9995 \
    types/level5 \
    types/numpad \
    types/extra \
    types/mousekeys \
    compat/complete \
    compat/lednum \
    compat/pc \
    compat/ledscroll \
    compat/basic \
    compat/misc \
    compat/iso9995 \
    compat/accessx \
    compat/xfree86 \
    compat/level5 \
    compat/caps \
    compat/ledcaps \
    compat/mousekeys \
    rules/base \
    rules/evdev \
; do
    cp "$XKBCONFIGROOT/$file" "test/data/$file"
done

for file in \
    compose.dir \
    locale.alias \
    locale.dir \
    en_US.UTF-8/Compose \
; do
    cp "$XLOCALEDIR/$file" "test/data/locale/$file"
done
