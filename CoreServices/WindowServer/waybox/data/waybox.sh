#!/bin/sh

if test -z "$XDG_CONFIG_HOME";
then
    XDG_CONFIG_HOME=${XDG_CONFIG_HOME:-$HOME/.config}
    export XDG_CONFIG_HOME
fi

WB_USER_CONF_DIR=$XDG_CONFIG_HOME/waybox
WB_SYS_CONF_DIR=@sysconfdir@/xdg/waybox
# Seemingly, Openbox hard-coded ~/.config/openbox rather than using
# $XDG_CONFIG_HOME. Older versions of Openbox used ~/.openbox and would
# continue to use it if available.
test -d ~/.openbox && OB_USER_CONF_DIR=~/.openbox || OB_USER_CONF_DIR=~/.config/openbox
OB_SYS_CONF_DIR=@sysconfdir@/xdg/openbox

_()
{
    if which gettext.sh >/dev/null 2>&1;
    then
        . gettext.sh
        TEXTDOMAIN=@package@
        export TEXTDOMAIN
        TEXTDOMAINDIR=@localedir@
        export TEXTDOMAINDIR
        eval_gettext "$1"
    else
        printf "$1"
    fi
}

# Load the environment variables
if test -f $WB_USER_CONF_DIR/environment;
then
    . $WB_USER_CONF_DIR/environment;
elif test -f $WB_SYS_CONF_DIR/environment;
then
    . $WB_SYS_CONF_DIR/environment
elif test -f $OB_USER_CONF_DIR/environment;
then
    _ "WARNING: Using files from Openbox. These may not work correctly."
    . $OB_USER_CONF_DIR/environment;
elif test -f $OB_SYS_CONF_DIR/environment;
then
    _ "WARNING: Using files from Openbox. These may not work correctly."
    . $OB_SYS_CONF_DIR/environment;
fi

# Get the autostart script to use
#
# Openbox calls the autostart script from the XDG autostart script, so we don't
# need to run the autostart script twice if only the Openbox scripts are
# available
if test -x $WB_USER_CONF_DIR/autostart;
then
    WB_AUTOSTART=$WB_USER_CONF_DIR/autostart;
elif test -x $WB_SYS_CONF_DIR/autostart;
then
    WB_AUTOSTART=$WB_SYS_CONF_DIR/autostart
fi

# And the XDG autostart script
if test -x $WB_USER_CONF_DIR/xdg-autostart;
then
    WB_XDG_AUTOSTART=$WB_USER_CONF_DIR/xdg-autostart;
elif test -x $WB_SYS_CONF_DIR/xdg-autostart;
then
    WB_XDG_AUTOSTART=$WB_SYS_CONF_DIR/xdg-autostart
elif test -x "@libexecdir@/openbox-autostart";
then
    _ "WARNING: Using files from Openbox. These may not work correctly."
    WB_XDG_AUTOSTART="@libexecdir@/openbox-autostart OPENBOX";
fi

if test -f $WB_USER_CONF_DIR/rc.xml;
then
    WB_RC_XML=$WB_USER_CONF_DIR/rc.xml
elif test -f $WB_SYS_CONF_DIR/rc.xml;
then
    WB_RC_XML=$WB_SYS_CONF_DIR/rc.xml
elif test -f $OB_USER_CONF_DIR/rc.xml;
then
    _ "WARNING: Using files from Openbox. These may not work correctly."
    WB_RC_XML=$OB_USER_CONF_DIR/rc.xml
elif test -f $OB_SYS_CONF_DIR/rc.xml;
then
    _ "WARNING: Using files from Openbox. These may not work correctly."
    WB_RC_XML=$OB_SYS_CONF_DIR/rc.xml;
else
    _ "ERROR: No configuration file found." >&2
    exit 1
fi
export WB_RC_XML

if which dbus-launch >/dev/null 2>&1;
then
    DBUS_LANCH="dbus-launch --exit-with-session"
fi

# No need to export these to Waybox
unset TEXTDOMAIN TEXTDOMAINDIR
$DBUS_LAUNCH @libexecdir@/waybox --startup "${WB_AUTOSTART:-true}; ${WB_XDG_AUTOSTART:-true}" "$@"
