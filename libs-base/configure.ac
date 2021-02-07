# configure.ac for GNU Objective-C library
# Process this file with autoconf to produce a configure script.
#
#   Copyright (C) 1993,1994, 1995, 1996, 1997 Free Software Foundation, Inc.
#
#   Written by:  Andrew Kachites McCallum <mccallum@cs.rochester.edu>
#   Dept. of Computer Science, U. of Rochester, Rochester, NY  14627
#
#   This file is part of the GNU Objective-C library.
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public
#   License as published by the Free Software Foundation; either
#   version 2 of the License, or (at your option) any later version.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public
#   License along with this library; if not, write to the Free
#   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#   Boston, MA 02111 USA

#
# DEPENDENCIES POLICY
#
# Generally the aim in the base library is to minimise dependencies on
# external libraries which may not come as standard on all systems, and
# you should not, as a rule, add such dependencies.
#
# However, in some cases there are large areas of functionality where it
# would just be too much effort to implement things ourselves and it makes
# sense to use external software to do the job.
#
# In such cases this configure.ac script should include options to allow
# people to build with the dependency or to disable those parts of the
# base library which require the external dependency and build without it.
# The configure script should support a --disable-xxx option in such a case,
# where 'xxx' is the name of the dependency.
#
# If a major dependency is NOT specifically disabled, the configuration
# process must fail with an error message ... all builds of gnustep-base
# should support all expected functionality unless the person configuring
# and building is very certain they don't want it.
#
# Where a dependency is introduced this script should do this by setting
# HAVE_XXX to 1 or 0 and calling AC_SUBST(XXX) to substitute the information
# into the make files:
#   config.mak.in: used to build the base library itsself
#   base.make.in: the makefile fragment used to build software which uses base
# These files then define GNUSTEP_BASE_HAVE_XXX to 1 or 0 so that the variable
# may be used to control conditional compilation of particular files.
#
# In addition the values should be substituted into the header file
# Headers/GNUstepBase/GSConfig.h.in so that it can define a
# preprocessor constant of the form GS_USE_XXX to 1 or 0, providing a
# standard mechanism for parts of a particular file to compile to make
# use of the external code or not.
# This file is included by all the base library source code, and may also
# be included by code which uses the base library.
#

builtin(include, config/objc-con-autoload.m4)dnl
builtin(include, config/objc-sys-dynamic.m4)dnl
builtin(include, config/procfs-exe-link.m4)dnl
builtin(include, config/procfs.m4)dnl
builtin(include, config/pathxml.m4)dnl
builtin(include, config/pathtls.m4)dnl
builtin(include, config/codeset.m4)dnl
builtin(include, config/addlibrarypath.m4)dnl
builtin(include, config/pkg.m4)dnl

AC_INIT
AC_PREREQ([2.60])
AC_CONFIG_SRCDIR([Source/NSArray.m])

# If GNUSTEP_MAKEFILES is undefined, try to use gnustep-config to determine it.
if test -z "$GNUSTEP_MAKEFILES"; then
  GNUSTEP_MAKEFILES=`gnustep-config --variable=GNUSTEP_MAKEFILES 2>&5`
fi

if test -z "$GNUSTEP_MAKEFILES"; then
  AC_MSG_ERROR([You must have the gnustep-make package installed and set up the GNUSTEP_MAKEFILES environment variable to contain the path to the makefiles directory before configuring!])
fi

# If LIBRARY_COMBO is undefined, try to use gnustep-config to determine it.
if test -z "$LIBRARY_COMBO"; then
  LIBRARY_COMBO=`gnustep-config --variable=LIBRARY_COMBO 2>&5`
fi
OBJC_RUNTIME_LIB=`echo $LIBRARY_COMBO | tr '-' ' ' | awk '{print $1}'`

if test "$OBJC_RUNTIME_LIB" = "ng" -o "$OBJC_RUNTIME_LIB" = "apple"; then
  nonfragile=yes
  BASE_NONFRAGILE_ABI=1
else
  nonfragile=`gnustep-config --objc-flags | grep _NONFRAGILE_ABI 2>&5`
  if test -z "$nonfragile"; then
    nonfragile=no
    BASE_NONFRAGILE_ABI=0
  else
    nonfragile=yes
    BASE_NONFRAGILE_ABI=1
  fi
fi
AC_SUBST(BASE_NONFRAGILE_ABI)

MAKECPPFLAGS=`gnustep-config --variable=CPPFLAGS`
if test "$CPPFLAGS" = ""; then
  CPPFLAGS=$MAKECPPFLAGS
  export CPPFLAGS
else
  if test "$CPPFLAGS" != "$MAKECPPFLAGS"; then
    AC_MSG_WARN([You are running configure with the preprocessor options ($CPPFLAGS) set to a different value from that used by gnustep-make ($MAKECPPFLAGS).  To avoid conflicts/problems, reconfigure/reinstall gnustep-make to use CPPFLAGS=$CPPFLAGS or run the gnustep-base configure again with your CPPFLAGS environment variable set to $MAKECPPFLAGS])
  fi
fi

MAKELDFLAGS=`gnustep-config --variable=LDFLAGS`
if test "$LDFLAGS" = ""; then
  LDFLAGS=$MAKELDFLAGS
  export LDFLAGS
else
  if test "$LDFLAGS" != "$MAKELDFLAGS"; then
    AC_MSG_WARN([You are running configure with the link options ($LDFLAGS) set to a different value from that used by gnustep-make ($MAKELDFLAGS).  To avoid conflicts/problems, reconfigure/reinstall gnustep-make to use LDFLAGS=$LDFLAGS or run the gnustep-base configure again with your LDFLAGS environment variable set to $MAKELDFLAGS])
  fi
fi

# We shouldn't be loading GNUstep.sh here.  It would load in a lot of
# variables which might get confused with the ones that will be used
# at runtime.  We will load it later once we have determined (and
# saved) the runtime configuration.

# This variable might get temporarily overwritten with the
# GNUSTEP_MAKEFILES of the runtime configuration, make sure we keep
# track of the original one.  CURRENT_GNUSTEP_MAKEFILES is the one
# that we use to locate the actual gnustep-make installation that
# will build the software.
CURRENT_GNUSTEP_MAKEFILES="$GNUSTEP_MAKEFILES"

#--------------------------------------------------------------------
# Use config.guess, config.sub and install-sh provided by gnustep-make
#--------------------------------------------------------------------
AC_CONFIG_AUX_DIR($CURRENT_GNUSTEP_MAKEFILES)

#--------------------------------------------------------------------
# Use a .h file with #define's, instead of -D command-line switches
#--------------------------------------------------------------------
AC_CONFIG_HEADER(Headers/GNUstepBase/config.h)

#--------------------------------------------------------------------
# Determine the host, build, and target systems
#--------------------------------------------------------------------
AC_CANONICAL_TARGET([])

#--------------------------------------------------------------------
# Setup cross-compilation-information
#--------------------------------------------------------------------

AC_ARG_WITH(cross-compilation-info,
[  --with-cross-compilation-info=PATH
				Specify path to the configuration file that
				contains information for the configure script in
				case of cross compilation. This information
				replaces those obtained from running programmes
				during the configuration phase.],
  cross_result="$withval",
  cross_result="no"
)
CROSS_CONFIG="./cross.config"
if test "$cross_result" != "no"
then
 if test -f "$cross_result" && test -r "$cross_result"
  then
    CROSS_CONFIG="$cross_result"
  else
    AC_MSG_ERROR(["Could not load cross-compilation variables from $cross_result"])
  fi
fi

# import information from the cross-config.
. "$CROSS_CONFIG"



#---------------------------------------------------------------------
# Location of the GNUstep.conf config file (--with-config-file)
#---------------------------------------------------------------------
AC_MSG_CHECKING([for GNUstep configuration file to use at runtime])

# This requires gnustep-make > 1.13.0 to work.  For gnustep-make =
# 1.13.0 we would have to parse
# $CURRENT_GNUSTEP_MAKEFILES/$obj_dir/config.make, but $obj_dir is not defined
# yet at this stage in config, not sure if it's worth trying to make
# it work.  For gnustep-make < 1.13.0 we would have to parse
# $CURRENT_GNUSTEP_MAKEFILES/config.make.
GNUSTEP_MAKE_CONFIG=`(grep '^GNUSTEP_CONFIG_FILE *=' $CURRENT_GNUSTEP_MAKEFILES/config-noarch.make | sed -e 's/GNUSTEP_CONFIG_FILE *= *\(.*\)/\1/') 2>&5`

# So, for backwards compatiblity, we try the plain config.make too.
# This should work with gnustep-make < 1.13.0, and with 1.13.0 too if
# they haven't deleted the file.
if test "$GNUSTEP_MAKE_CONFIG"  = ""; then
  GNUSTEP_MAKE_CONFIG=`(grep '^GNUSTEP_CONFIG_FILE *=' $CURRENT_GNUSTEP_MAKEFILES/config.make | sed -e 's/GNUSTEP_CONFIG_FILE *= *\(.*\)/\1/') 2>&5`
fi

AC_ARG_WITH(config-file,
[  --with-config-file=PATH       Specify path to the GNUstep config file.
				This is the location to be used by the base
				library to locate path information at
				application or tool runtime.
				This file might not even exist now; it is
				not read at configure time.  The base library
				will only read it at runtime.
				If unspecified, this uses the same value as
				the GNUstep make package on unix-like systems,
				but uses ./GNUstep.conf on mingw so that
				it is relative to the location of the
				base library DLL.
				If a leading './' is specified, the path
				is taken to be relative to the base library
				linked runtime, not all operating systems
				can support this, so on some platforms you
				may need to specify the location of the
				config file using the GNUSTEP_CONFIG_FILE
				environment variable at runtime.
				If a trailing '/' is specified, the path is
				used for locating domains but no GNUstep
				config file is read at runtime.],
  result="$withval",
  result="no"
)
if test "$result" != "no"
then
  GNUSTEP_TARGET_CONFIG_FILE="$result"
fi
if test x"$GNUSTEP_TARGET_CONFIG_FILE" = x""; then
  case "$target_os" in
    mingw*|windows*)
      GNUSTEP_TARGET_CONFIG_FILE=./GNUstep.conf ;;
    *)
      GNUSTEP_TARGET_CONFIG_FILE="$GNUSTEP_MAKE_CONFIG" ;;
  esac
fi
AC_MSG_RESULT($GNUSTEP_TARGET_CONFIG_FILE)

#-----------------------------------------------------------------
# Whether the GNUstep.conf file path can be set in the environment
# By default this is enabled on unix, but disabled on mswindows
# since the normal setup on mswindows is to have the config file
# located with the base library dll for runtime configuration and
# use the environment variable to control the developer config file
# location (used by gnustep-make when building).
#-----------------------------------------------------------------
case "$target_os" in
    mingw*|windows*)	enable_env_config=no;;
    *)		enable_env_config=yes;;
esac
AC_MSG_CHECKING([whether the GNUstep.conf file path can be set in the environment])
AC_ARG_ENABLE(environment-config-file,
[  --disable-environment-config-file
                                Disables the use of the GNUSTEP_CONFIG_FILE
				environment variable to specify/override
				the location of the GNUstep config file
				at runtime.  This option is occasionally
				useful to disable the environment variable
				for sites which wish to 'lock down' users
				to always work with a specific system-wide
				configuration.  On unix-like systems the
				default is for this option to be enabled.
				It is disabled by default on windows systems
				so that the base library will not use a
				config file intended for the gnustep-make
				system (and containing unix-style paths
				which cannot be used by windows apps).
				Normally this should be left at its default
				setting.],
  ac_cv_environment_config_file=$enableval,
  ac_cv_environment_config_file=$enable_env_config)
if test "$ac_cv_environment_config_file" = "yes"; then
  AC_DEFINE(OPTION_NO_ENVIRONMENT, 0,
    [Enable GNUSTEP_CONFIG_FILE environment variable])
  AC_MSG_RESULT([yes])
else
  AC_DEFINE(OPTION_NO_ENVIRONMENT, 1,
    [Disable GNUSTEP_CONFIG_FILE environment variable])
  AC_MSG_RESULT([no: disabled from the command-line])
fi

#--------------------------------------------------------------------
# We are now trying to determine the default GNUstep paths to be
# used at runtime.  So all GNUSTEP_xxx variables from now on are to be
# considered as 'runtime' ones.  They refer to paths that might not
# make any sense now, but might make sense once gnustep-base is
# installed in its final location.
#---------------------------------------------------------------------

#
# Set 'standard' defaults for values from configuration file.
#
case "$target_os" in
  *)
    GNUSTEP_SYSTEM_ROOT=/usr/GNUstep/System
    GNUSTEP_LOCAL_ROOT=/usr/GNUstep/Local
    GNUSTEP_NETWORK_ROOT=/usr/GNUstep/Local
    GNUSTEP_USER_DEFAULTS_DIR=GNUstep/Defaults
    GNUSTEP_USER_CONFIG_FILE=.GNUstep.conf
    ;;
esac

#---------------------------------------------------------------------
# Now read/import the existing configuration file, if any
#---------------------------------------------------------------------

# Reading/importing an existing configuration file is good as it means
# the built-in default paths in the code will match those of your
# installation (or of the config file you specify).
# It can be annoying in certain cases though; this option lets you
# turn it off.
AC_MSG_CHECKING([if we should import an existing configuration file now])
AC_ARG_ENABLE(importing-config-file,
[  --disable-importing-config-file
                                Disable importing of an existing GNUstep config
				file and use inbuilt defaults instead.],
  ac_cv_importing_config_file=$enableval,
  ac_cv_importing_config_file="yes")
if test "$ac_cv_importing_config_file" = "no"; then
  AC_MSG_RESULT([no: disabled from the command-line])
else
  AC_MSG_RESULT([yes])
fi


if test "$ac_cv_importing_config_file" = "yes" ;
then
  AC_MSG_CHECKING([for default GNUstep configuration file to use])
  AC_ARG_WITH(default-config,
[  --with-default-config=PATH   Specify path to a GNUstep config file to be
				imported at configure time (now) and used to
				provide default values for the base library
				to use at runtime if no GNUstep config file
				is found at runtime.  If this is not specified
				then the path from the gnustep-make package
				is used.],
    result="$withval",
    result="no"
  )
  if test "$result" != "no"
  then
    GNUSTEP_DEFAULT_CONFIG="$result"
  fi
  if test "$GNUSTEP_DEFAULT_CONFIG" = ""; then
    # No file to import has been specified.  We need to read the paths
    # from somewhere though!  GNUstep.sh might not have been sourced,
    # so at this stage we have no knowledge of what the paths must be.
    # The only place that we can read them from is the gnustep-make
    # GNUstep.conf file.
    # So we fall back to useing the make settings for the built-in ones.
    # However, on mingw these will be msys style paths, and we don't
    # want that ... so later on we convert these to portable relative
    # paths based on the directory in which the base library will be
    # installed.
    GNUSTEP_DEFAULT_CONFIG="$GNUSTEP_MAKE_CONFIG"
  fi
  AC_MSG_RESULT($GNUSTEP_DEFAULT_CONFIG)

  #
  # Only try importing if the default config file has been specified.
  #
  if test "$GNUSTEP_DEFAULT_CONFIG" != ""; then
    # Make sure we have a slash in the path so that '.' will source it
    case $GNUSTEP_DEFAULT_CONFIG in
      /*) ;;
      *) GNUSTEP_DEFAULT_CONFIG="./$GNUSTEP_DEFAULT_CONFIG" ;;
    esac
    #
    # Use the default config file to override standard values.
    #
    if test ! -f "$GNUSTEP_DEFAULT_CONFIG"; then
      AC_MSG_RESULT([fail: file "$GNUSTEP_DEFAULT_CONFIG" does not exist])
      # Ohoh ... things are not going well.  We are asked to import
      # a config file that doesn't exist.  So all paths might be unset
      # and who knows what we'll end up hardcoding into gnustep-base.
      # It looks like we need to make sure the user knows what they
      # are doing, as there is a high chance they don't and might end
      # up with a confused/non-working system.  As far as we know, the
      # system might be already screwed.  If they don't want to import
      # a config file (eg, they don't have one and they don't care about
      # the hardcoded paths) they should just say so. ;-)
      AC_MSG_ERROR([Please run configure again with the --disable-importing-config-file option or specifying an alternative file using the --with-default-config= option])
      exit 1
    else
      AC_MSG_RESULT([trying to import "$GNUSTEP_DEFAULT_CONFIG"])
      if test -r "$GNUSTEP_DEFAULT_CONFIG"
      then
        AC_MSG_NOTICE([If this fails, please run configure again with the --disable-importing-config-file option or specifying an alternative file using the --with-default-config= option])
        . "$GNUSTEP_DEFAULT_CONFIG"
      else
        AC_MSG_RESULT([fail: file "$GNUSTEP_DEFAULT_CONFIG" is not readable])
	AC_MSG_ERROR([Unable to import configuration file at $GNUSTEP_DEFAULT_CONFIG.  Please run configure again with a revised/corrected --with-default-config= option or with --disable-importing-config-file.])
	exit 1
      fi
    fi
  fi
fi

# Now we have the problem of what to do if some of the paths were not
# set by GNUstep.conf (eg, old gnustep-make), or if no config file was
# read, or if the config file was corrupt.  We decide that the most
# likely case is an old gnustep-make, so by default we configure the
# other paths basing on GNUSTEP_*_ROOT.

# TODO/FIXME: We should really have better checks once the situation
# has stabilized.  This is a reasonable hack for now.

if test x"$GNUSTEP_MAKEFILES" = x""; then GNUSTEP_MAKEFILES=$GNUSTEP_SYSTEM_ROOT/Library/Makefiles; fi

if test x"$GNUSTEP_SYSTEM_USERS_DIR" = x""; then GNUSTEP_SYSTEM_USERS_DIR=/home; fi
if test x"$GNUSTEP_NETWORK_USERS_DIR" = x""; then GNUSTEP_NETWORK_USERS_DIR=/home; fi
if test x"$GNUSTEP_LOCAL_USERS_DIR" = x""; then GNUSTEP_LOCAL_USERS_DIR=/home; fi

if test x"$GNUSTEP_SYSTEM_APPS" = x""; then GNUSTEP_SYSTEM_APPS=$GNUSTEP_SYSTEM_ROOT/Applications; fi
if test x"$GNUSTEP_SYSTEM_ADMIN_APPS" = x""; then GNUSTEP_SYSTEM_ADMIN_APPS=$GNUSTEP_SYSTEM_ROOT/Applications/Admin; fi
if test x"$GNUSTEP_SYSTEM_WEB_APPS" = x""; then GNUSTEP_SYSTEM_WEB_APPS=$GNUSTEP_SYSTEM_ROOT/WebApplications; fi
if test x"$GNUSTEP_SYSTEM_TOOLS" = x""; then GNUSTEP_SYSTEM_TOOLS=$GNUSTEP_SYSTEM_ROOT/Tools; fi
if test x"$GNUSTEP_SYSTEM_ADMIN_TOOLS" = x""; then GNUSTEP_SYSTEM_ADMIN_TOOLS=$GNUSTEP_SYSTEM_ROOT/Tools/Admin; fi
if test x"$GNUSTEP_SYSTEM_LIBRARY" = x""; then GNUSTEP_SYSTEM_LIBRARY=$GNUSTEP_SYSTEM_ROOT/Library; fi
if test x"$GNUSTEP_SYSTEM_LIBRARIES" = x""; then GNUSTEP_SYSTEM_LIBRARIES=$GNUSTEP_SYSTEM_LIBRARY/Libraries; fi
if test x"$GNUSTEP_SYSTEM_HEADERS" = x""; then GNUSTEP_SYSTEM_HEADERS=$GNUSTEP_SYSTEM_LIBRARY/Headers; fi
if test x"$GNUSTEP_SYSTEM_DOC" = x""; then GNUSTEP_SYSTEM_DOC=$GNUSTEP_SYSTEM_LIBRARY/Documentation; fi
if test x"$GNUSTEP_SYSTEM_DOC_MAN" = x""; then GNUSTEP_SYSTEM_DOC_MAN=$GNUSTEP_SYSTEM_LIBRARY/Documentation/man; fi
if test x"$GNUSTEP_SYSTEM_DOC_INFO" = x""; then GNUSTEP_SYSTEM_DOC_INFO=$GNUSTEP_SYSTEM_LIBRARY/Documentation/info; fi

if test x"$GNUSTEP_NETWORK_APPS" = x""; then GNUSTEP_NETWORK_APPS=$GNUSTEP_NETWORK_ROOT/Applications; fi
if test x"$GNUSTEP_NETWORK_ADMIN_APPS" = x""; then GNUSTEP_NETWORK_ADMIN_APPS=$GNUSTEP_NETWORK_ROOT/Applications/Admin; fi
if test x"$GNUSTEP_NETWORK_WEB_APPS" = x""; then GNUSTEP_NETWORK_APPS=$GNUSTEP_NETWORK_ROOT/WebApplications; fi
if test x"$GNUSTEP_NETWORK_TOOLS" = x""; then GNUSTEP_NETWORK_TOOLS=$GNUSTEP_NETWORK_ROOT/Tools; fi
if test x"$GNUSTEP_NETWORK_ADMIN_TOOLS" = x""; then GNUSTEP_NETWORK_ADMIN_TOOLS=$GNUSTEP_NETWORK_ROOT/Tools/Admin; fi
if test x"$GNUSTEP_NETWORK_LIBRARY" = x""; then GNUSTEP_NETWORK_LIBRARY=$GNUSTEP_NETWORK_ROOT/Library; fi
if test x"$GNUSTEP_NETWORK_LIBRARIES" = x""; then GNUSTEP_NETWORK_LIBRARIES=$GNUSTEP_NETWORK_LIBRARY/Libraries; fi
if test x"$GNUSTEP_NETWORK_HEADERS" = x""; then GNUSTEP_NETWORK_HEADERS=$GNUSTEP_NETWORK_LIBRARY/Headers; fi
if test x"$GNUSTEP_NETWORK_DOC" = x""; then GNUSTEP_NETWORK_DOC=$GNUSTEP_NETWORK_LIBRARY/Documentation; fi
if test x"$GNUSTEP_NETWORK_DOC_MAN" = x""; then GNUSTEP_NETWORK_DOC_MAN=$GNUSTEP_NETWORK_LIBRARY/Documentation/man; fi
if test x"$GNUSTEP_NETWORK_DOC_INFO" = x""; then GNUSTEP_NETWORK_DOC_INFO=$GNUSTEP_NETWORK_LIBRARY/Documentation/info; fi

if test x"$GNUSTEP_LOCAL_APPS" = x""; then GNUSTEP_LOCAL_APPS=$GNUSTEP_LOCAL_ROOT/Applications; fi
if test x"$GNUSTEP_LOCAL_ADMIN_APPS" = x""; then GNUSTEP_LOCAL_ADMIN_APPS=$GNUSTEP_LOCAL_ROOT/Applications/Admin; fi
if test x"$GNUSTEP_LOCAL_WEB_APPS" = x""; then GNUSTEP_LOCAL_APPS=$GNUSTEP_LOCAL_ROOT/WebApplications; fi
if test x"$GNUSTEP_LOCAL_TOOLS" = x""; then GNUSTEP_LOCAL_TOOLS=$GNUSTEP_LOCAL_ROOT/Tools; fi
if test x"$GNUSTEP_LOCAL_ADMIN_TOOLS" = x""; then GNUSTEP_LOCAL_ADMIN_TOOLS=$GNUSTEP_LOCAL_ROOT/Tools/Admin; fi
if test x"$GNUSTEP_LOCAL_LIBRARY" = x""; then GNUSTEP_LOCAL_LIBRARY=$GNUSTEP_LOCAL_ROOT/Library; fi
if test x"$GNUSTEP_LOCAL_LIBRARIES" = x""; then GNUSTEP_LOCAL_LIBRARIES=$GNUSTEP_LOCAL_LIBRARY/Libraries; fi
if test x"$GNUSTEP_LOCAL_HEADERS" = x""; then GNUSTEP_LOCAL_HEADERS=$GNUSTEP_LOCAL_LIBRARY/Headers; fi
if test x"$GNUSTEP_LOCAL_DOC" = x""; then GNUSTEP_LOCAL_DOC=$GNUSTEP_LOCAL_LIBRARY/Documentation; fi
if test x"$GNUSTEP_LOCAL_DOC_MAN" = x""; then GNUSTEP_LOCAL_DOC_MAN=$GNUSTEP_LOCAL_LIBRARY/Documentation/man; fi
if test x"$GNUSTEP_LOCAL_DOC_INFO" = x""; then GNUSTEP_LOCAL_DOC_INFO=$GNUSTEP_LOCAL_LIBRARY/Documentation/info; fi

if test x"$GNUSTEP_USER_DIR_APPS" = x""; then GNUSTEP_USER_DIR_APPS=$GNUSTEP_USER_DIR/Applications; fi
if test x"$GNUSTEP_USER_DIR_ADMIN_APPS" = x""; then GNUSTEP_USER_DIR_ADMIN_APPS=$GNUSTEP_USER_DIR/Applications/Admin; fi
if test x"$GNUSTEP_USER_DIR_WEB_APPS" = x""; then GNUSTEP_USER_DIR_APPS=$GNUSTEP_USER_DIR/WebApplications; fi
if test x"$GNUSTEP_USER_DIR_TOOLS" = x""; then GNUSTEP_USER_DIR_TOOLS=$GNUSTEP_USER_DIR/Tools; fi
if test x"$GNUSTEP_USER_DIR_ADMIN_TOOLS" = x""; then GNUSTEP_USER_DIR_ADMIN_TOOLS=$GNUSTEP_USER_DIR/Tools/Admin; fi
if test x"$GNUSTEP_USER_DIR_LIBRARY" = x""; then GNUSTEP_USER_DIR_LIBRARY=$GNUSTEP_USER_DIR/Library; fi
if test x"$GNUSTEP_USER_DIR_LIBRARIES" = x""; then GNUSTEP_USER_DIR_LIBRARIES=$GNUSTEP_USER_DIR/Libraries; fi
if test x"$GNUSTEP_USER_DIR_HEADERS" = x""; then GNUSTEP_USER_DIR_HEADERS=$GNUSTEP_USER_DIR/Headers; fi
if test x"$GNUSTEP_USER_DIR_DOC" = x""; then GNUSTEP_USER_DIR_DOC=$GNUSTEP_USER_DIR/Documentation; fi
if test x"$GNUSTEP_USER_DIR_DOC_MAN" = x""; then GNUSTEP_USER_DIR_DOC_MAN=$GNUSTEP_USER_DIR/Documentation/man; fi
if test x"$GNUSTEP_USER_DIR_DOC_INFO" = x""; then GNUSTEP_USER_DIR_DOC_INFO=$GNUSTEP_USER_DIR/Documentation/info; fi

AC_MSG_CHECKING([for GNUstep-base installation domain])
AC_ARG_WITH(installation-domain,
[  --with-installation-domain=DOMAIN
				Specify the domain (SYSTEM, LOCAL,
				NETWORK or USER) into which
				gnustep-base will be installed.
				Whenever relative paths are hardcoded
				into gnustep-base (at the moment, this
				happens only on MinGW) this option
				must be used and must match the domain
				where you will be installing
				gnustep-base.
				If this is not specified, the output of
				gnustep-config --installation-domain-for=gnustep-base
				(which should normally be LOCAL) is used.],
  result="$withval",
  result="no"
)
if test "$result" = "no"
then
  # Check if gnustep-config supports the option
  # --installation-domain-for, and use it if available.  That option
  # will automatically use the shell variable GNUSTEP_INSTALLATION_DOMAIN or
  # the installation-domains.conf files (or the LOCAL default) as appropriate.
  # Otherwise, if that option is not available just fall back to
  # using GNUSTEP_INSTALLATION_DOMAIN or (if that was not defiend) LOCAL.
  if (gnustep-config --help | grep installation-domain) >&5 2>&5
  then
    result=`gnustep-config --installation-domain-for=gnustep-base 2>&5`
  else
    AC_MSG_WARN([The 'gnustep-config' script was not found or out of date.])
    result=$GNUSTEP_INSTALLATION_DOMAIN
    if test "$result" = ""
    then
      # This case was added on December 2008 and is only for backwards
      # compatibility with older versions of gnustep-make.
      result=LOCAL
    fi
  fi
fi
case "$result" in
  SYSTEM)
    AC_MSG_RESULT([SYSTEM])
    GNUSTEP_BASE_PATH="$GNUSTEP_SYSTEM_TOOLS";;
  LOCAL)
    AC_MSG_RESULT([LOCAL])
    GNUSTEP_BASE_PATH="$GNUSTEP_LOCAL_TOOLS";;
  NETWORK)
    AC_MSG_RESULT([NETWORK])
    GNUSTEP_BASE_PATH="$GNUSTEP_NETWORK_TOOLS";;
  USER)
    AC_MSG_RESULT([USER])
    GNUSTEP_BASE_PATH="$GNUSTEP_USER_TOOLS";;
  *)
    AC_MSG_ERROR([Unknown installation domain '$result' (it should be SYSTEM, LOCAL, NETWORK or USER).  Please run configure again with the option --with-installation-domain=LOCAL (or whatever domain you want to install into).])
    exit 1;;
esac
# We store GNUSTEP_BASE_DOMAIN in config.mak so that we can check
# at runtime and make sure it is consistent with the
# GNUSTEP_INSTALLATION_DOMAIN that is used at runtime.
GNUSTEP_BASE_DOMAIN=$result
AC_SUBST(GNUSTEP_BASE_DOMAIN)

#
# If we are on mingw, we now want to convert the paths to relative
# paths (relative to libgnustep-base.dll).
#
case "$target_os" in
  mingw*|windows*)
    # TODO: Improve this hack.
    # According to Wikipedia, this is the default for Windows 2000,
    # Windows XP and Windows Server 2003.  For Windows Vista this will
    # change to C:\Users.  The directory name needs to be localized though
    # (and the disk may need changing as well ?).
    GNUSTEP_SYSTEM_USERS_DIR="C:\Documents and Settings"
    GNUSTEP_NETWORK_USERS_DIR="C:\Documents and Settings"
    GNUSTEP_LOCAL_USERS_DIR="C:\Documents and Settings"

    # TODO: It would be nice to use the 'short' output of
    # relative_path.sh, but older versions of relative_path.sh
    # did not support specifying the type of output and
    # would abort if they were given more than 2 parameters,
    # so we can not use the 'short' option if we want gnustep-base
    # to work with older versions of gnustep-make.
    # Once everyone has upgraded to gnustep-make >= 2.0.5 (I'd say
    # two years after it has been released ?), we could switch to the
    # 'short' output though.
    GNUSTEP_SYSTEM_APPS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_SYSTEM_APPS`
    GNUSTEP_SYSTEM_ADMIN_APPS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_SYSTEM_ADMIN_APPS`
    GNUSTEP_SYSTEM_WEB_APPS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_SYSTEM_WEB_APPS`
    GNUSTEP_SYSTEM_TOOLS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_SYSTEM_TOOLS`
    GNUSTEP_SYSTEM_ADMIN_TOOLS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_SYSTEM_ADMIN_TOOLS`
    GNUSTEP_SYSTEM_LIBRARY=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_SYSTEM_LIBRARY`
    GNUSTEP_SYSTEM_LIBRARIES=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_SYSTEM_LIBRARIES`
    GNUSTEP_SYSTEM_HEADERS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_SYSTEM_HEADERS`
    GNUSTEP_SYSTEM_DOC=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_SYSTEM_DOC`
    GNUSTEP_SYSTEM_DOC_MAN=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_SYSTEM_DOC_MAN`
    GNUSTEP_SYSTEM_DOC_INFO=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_SYSTEM_DOC_INFO`

    GNUSTEP_NETWORK_APPS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_NETWORK_APPS`
    GNUSTEP_NETWORK_ADMIN_APPS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_NETWORK_ADMIN_APPS`
    GNUSTEP_NETWORK_WEB_APPS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_NETWORK_WEB_APPS`
    GNUSTEP_NETWORK_TOOLS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_NETWORK_TOOLS`
    GNUSTEP_NETWORK_ADMIN_TOOLS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_NETWORK_ADMIN_TOOLS`
    GNUSTEP_NETWORK_LIBRARY=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_NETWORK_LIBRARY`
    GNUSTEP_NETWORK_LIBRARIES=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_NETWORK_LIBRARIES`
    GNUSTEP_NETWORK_HEADERS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_NETWORK_HEADERS`
    GNUSTEP_NETWORK_DOC=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_NETWORK_DOC`
    GNUSTEP_NETWORK_DOC_MAN=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_NETWORK_DOC_MAN`
    GNUSTEP_NETWORK_DOC_INFO=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_NETWORK_DOC_INFO`

    GNUSTEP_LOCAL_APPS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_LOCAL_APPS`
    GNUSTEP_LOCAL_ADMIN_APPS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_LOCAL_ADMIN_APPS`
    GNUSTEP_LOCAL_WEB_APPS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_LOCAL_WEB_APPS`
    GNUSTEP_LOCAL_TOOLS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_LOCAL_TOOLS`
    GNUSTEP_LOCAL_ADMIN_TOOLS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_LOCAL_ADMIN_TOOLS`
    GNUSTEP_LOCAL_LIBRARY=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_LOCAL_LIBRARY`
    GNUSTEP_LOCAL_LIBRARIES=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_LOCAL_LIBRARIES`
    GNUSTEP_LOCAL_HEADERS=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_LOCAL_HEADERS`
    GNUSTEP_LOCAL_DOC=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_LOCAL_DOC`
    GNUSTEP_LOCAL_DOC_MAN=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_LOCAL_DOC_MAN`
    GNUSTEP_LOCAL_DOC_INFO=`$GNUSTEP_MAKEFILES/relative_path.sh $GNUSTEP_BASE_PATH $GNUSTEP_LOCAL_DOC_INFO`
# It would be nice to now store this stuff into a ./GNUstep.conf file
# installed with gnustep-base.dll.  This would clarify.
    ;;
esac

GNUSTEP_BASE_RELATIVE_PATHS=no

# Now, check if any of the paths is a relative path.
for path in \
 "$GNUSTEP_SYSTEM_APPS" "$GNUSTEP_SYSTEM_ADMIN_APPS" "$GNUSTEP_SYSTEM_WEB_APPS" \
 "$GNUSTEP_SYSTEM_TOOLS" "$GNUSTEP_SYSTEM_ADMIN_TOOLS" \
 "$GNUSTEP_SYSTEM_LIBRARY" "$GNUSTEP_SYSTEM_LIBRARIES" "$GNUSTEP_SYSTEM_HEADERS" \
 "$GNUSTEP_SYSTEM_DOC" "$GNUSTEP_SYSTEM_DOC_MAN" "$GNUSTEP_SYSTEM_DOC_INFO" \
 "$GNUSTEP_NETWORK_APPS" "$GNUSTEP_NETWORK_ADMIN_APPS" "$GNUSTEP_NETWORK_WEB_APPS" \
 "$GNUSTEP_NETWORK_TOOLS" "$GNUSTEP_NETWORK_ADMIN_TOOLS" \
 "$GNUSTEP_NETWORK_LIBRARY" "$GNUSTEP_NETWORK_LIBRARIES" "$GNUSTEP_NETWORK_HEADERS" \
 "$GNUSTEP_NETWORK_DOC" "$GNUSTEP_NETWORK_DOC_MAN" "$GNUSTEP_NETWORK_DOC_INFO" \
 "$GNUSTEP_LOCAL_APPS" "$GNUSTEP_LOCAL_ADMIN_APPS" "$GNUSTEP_LOCAL_WEB_APPS" \
 "$GNUSTEP_LOCAL_TOOLS" "$GNUSTEP_LOCAL_ADMIN_TOOLS" \
 "$GNUSTEP_LOCAL_LIBRARY" "$GNUSTEP_LOCAL_LIBRARIES" "$GNUSTEP_LOCAL_HEADERS" \
 "$GNUSTEP_LOCAL_DOC" "$GNUSTEP_LOCAL_DOC_MAN" "$GNUSTEP_LOCAL_DOC_INFO"; do
case "$path" in
  ./*)  GNUSTEP_BASE_RELATIVE_PATHS=yes ;;
  ../*) GNUSTEP_BASE_RELATIVE_PATHS=yes ;;
esac
done

AC_MSG_CHECKING([if we are hardcoding any relative paths in gnustep-base])
AC_MSG_RESULT($GNUSTEP_BASE_RELATIVE_PATHS)
AC_SUBST(GNUSTEP_BASE_RELATIVE_PATHS)

#
# Now, we want to print out the paths that we're going to hardcode
# into gnustep-base.  This is to help users in finding
# misconfigurations.
#
AC_MSG_NOTICE([We store the following filesystem layout into gnustep-base, to be used when no config file is found])
AC_MSG_CHECKING([for Makefiles directory])
AC_MSG_RESULT($GNUSTEP_MAKEFILES)
AC_MSG_CHECKING([for user defaults directory])
AC_MSG_RESULT($GNUSTEP_USER_DEFAULTS_DIR)
AC_MSG_CHECKING([for user config file])
AC_MSG_RESULT($GNUSTEP_USER_CONFIG_FILE)

AC_MSG_CHECKING([for System Applications directory])
AC_MSG_RESULT($GNUSTEP_SYSTEM_APPS)
AC_MSG_CHECKING([for System Admin Applications directory])
AC_MSG_RESULT($GNUSTEP_SYSTEM_ADMIN_APPS)
AC_MSG_CHECKING([for System Web Applications directory])
AC_MSG_RESULT($GNUSTEP_SYSTEM_WEB_APPS)
AC_MSG_CHECKING([for System Tools directory])
AC_MSG_RESULT($GNUSTEP_SYSTEM_TOOLS)
AC_MSG_CHECKING([for System Admin Tools directory])
AC_MSG_RESULT($GNUSTEP_SYSTEM_ADMIN_TOOLS)
AC_MSG_CHECKING([for System Library directory])
AC_MSG_RESULT($GNUSTEP_SYSTEM_LIBRARY)
AC_MSG_CHECKING([for System Libraries directory])
AC_MSG_RESULT($GNUSTEP_SYSTEM_LIBRARIES)
AC_MSG_CHECKING([for System Headers directory])
AC_MSG_RESULT($GNUSTEP_SYSTEM_HEADERS)
AC_MSG_CHECKING([for System Documentation directory])
AC_MSG_RESULT($GNUSTEP_SYSTEM_DOC)
AC_MSG_CHECKING([for System Info Documentation directory])
AC_MSG_RESULT($GNUSTEP_SYSTEM_DOC_INFO)
AC_MSG_CHECKING([for System Man Documentation directory])
AC_MSG_RESULT($GNUSTEP_SYSTEM_DOC_MAN)

AC_MSG_CHECKING([for Network Applications directory])
AC_MSG_RESULT($GNUSTEP_NETWORK_APPS)
AC_MSG_CHECKING([for Network Admin Applications directory])
AC_MSG_RESULT($GNUSTEP_NETWORK_ADMIN_APPS)
AC_MSG_CHECKING([for Network Web Applications directory])
AC_MSG_RESULT($GNUSTEP_NETWORK_WEB_APPS)
AC_MSG_CHECKING([for Network Tools directory])
AC_MSG_RESULT($GNUSTEP_NETWORK_TOOLS)
AC_MSG_CHECKING([for Network Admin Tools directory])
AC_MSG_RESULT($GNUSTEP_NETWORK_ADMIN_TOOLS)
AC_MSG_CHECKING([for Network Library directory])
AC_MSG_RESULT($GNUSTEP_NETWORK_LIBRARY)
AC_MSG_CHECKING([for Network Libraries directory])
AC_MSG_RESULT($GNUSTEP_NETWORK_LIBRARIES)
AC_MSG_CHECKING([for Network Headers directory])
AC_MSG_RESULT($GNUSTEP_NETWORK_HEADERS)
AC_MSG_CHECKING([for Network Documentation directory])
AC_MSG_RESULT($GNUSTEP_NETWORK_DOC)
AC_MSG_CHECKING([for Network Info Documentation directory])
AC_MSG_RESULT($GNUSTEP_NETWORK_DOC_INFO)
AC_MSG_CHECKING([for Network Man Documentation directory])
AC_MSG_RESULT($GNUSTEP_NETWORK_DOC_MAN)

AC_MSG_CHECKING([for Local Applications directory])
AC_MSG_RESULT($GNUSTEP_LOCAL_APPS)
AC_MSG_CHECKING([for Local Admin Applications directory])
AC_MSG_RESULT($GNUSTEP_LOCAL_ADMIN_APPS)
AC_MSG_CHECKING([for Local Web Applications directory])
AC_MSG_RESULT($GNUSTEP_LOCAL_WEB_APPS)
AC_MSG_CHECKING([for Local Tools directory])
AC_MSG_RESULT($GNUSTEP_LOCAL_TOOLS)
AC_MSG_CHECKING([for Local Admin Tools directory])
AC_MSG_RESULT($GNUSTEP_LOCAL_ADMIN_TOOLS)
AC_MSG_CHECKING([for Local Library directory])
AC_MSG_RESULT($GNUSTEP_LOCAL_LIBRARY)
AC_MSG_CHECKING([for Local Libraries directory])
AC_MSG_RESULT($GNUSTEP_LOCAL_LIBRARIES)
AC_MSG_CHECKING([for Local Headers directory])
AC_MSG_RESULT($GNUSTEP_LOCAL_HEADERS)
AC_MSG_CHECKING([for Local Documentation directory])
AC_MSG_RESULT($GNUSTEP_LOCAL_DOC)
AC_MSG_CHECKING([for Local Info Documentation directory])
AC_MSG_RESULT($GNUSTEP_LOCAL_DOC_INFO)
AC_MSG_CHECKING([for Local Man Documentation directory])
AC_MSG_RESULT($GNUSTEP_LOCAL_DOC_MAN)

AC_MSG_CHECKING([for User Applications directory])
AC_MSG_RESULT($GNUSTEP_USER_DIR_APPS)
AC_MSG_CHECKING([for User Admin Applications directory])
AC_MSG_RESULT($GNUSTEP_USER_DIR_ADMIN_APPS)
AC_MSG_CHECKING([for User Web Applications directory])
AC_MSG_RESULT($GNUSTEP_USER_DIR_WEB_APPS)
AC_MSG_CHECKING([for User Tools directory])
AC_MSG_RESULT($GNUSTEP_USER_DIR_TOOLS)
AC_MSG_CHECKING([for User Admin Tools directory])
AC_MSG_RESULT($GNUSTEP_USER_DIR_ADMIN_TOOLS)
AC_MSG_CHECKING([for User Library directory])
AC_MSG_RESULT($GNUSTEP_USER_DIR_LIBRARY)
AC_MSG_CHECKING([for User Libraries directory])
AC_MSG_RESULT($GNUSTEP_USER_DIR_LIBRARIES)
AC_MSG_CHECKING([for User Headers directory])
AC_MSG_RESULT($GNUSTEP_USER_DIR_HEADERS)
AC_MSG_CHECKING([for User Documentation directory])
AC_MSG_RESULT($GNUSTEP_USER_DIR_DOC)
AC_MSG_CHECKING([for User Info Documentation directory])
AC_MSG_RESULT($GNUSTEP_USER_DIR_DOC_INFO)
AC_MSG_CHECKING([for User Man Documentation directory])
AC_MSG_RESULT($GNUSTEP_USER_DIR_DOC_MAN)

AC_MSG_CHECKING([for System User directory])
AC_MSG_RESULT($GNUSTEP_SYSTEM_USERS_DIR)
AC_MSG_CHECKING([for Network User directory])
AC_MSG_RESULT($GNUSTEP_NETWORK_USERS_DIR)
AC_MSG_CHECKING([for Local User directory])
AC_MSG_RESULT($GNUSTEP_LOCAL_USERS_DIR)

#
# Set the default configuration file values in config.h to be hard-coded
# into NSPathUtilities.m
#
GNUSTEP_MAKEFILES=`echo $GNUSTEP_MAKEFILES|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_MAKEFILES,
  "$GNUSTEP_MAKEFILES",
  [Built in default value for GNUstep Makefiles])
GNUSTEP_USER_DEFAULTS_DIR=`echo $GNUSTEP_USER_DEFAULTS_DIR|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_USER_DEFAULTS_DIR,
  "$GNUSTEP_USER_DEFAULTS_DIR",
  [Built in default value for GNUstep user defaults directory])
GNUSTEP_USER_CONFIG_FILE=`echo $GNUSTEP_USER_CONFIG_FILE|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_USER_CONFIG_FILE,
  "$GNUSTEP_USER_CONFIG_FILE",
  [Built in default value for GNUstep user config file])
GNUSTEP_TARGET_CONFIG_FILE=`echo $GNUSTEP_TARGET_CONFIG_FILE|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_CONFIG_FILE,
  "$GNUSTEP_TARGET_CONFIG_FILE",
  [Built in default value for GNUstep config file])

#
# SYSTEM domain paths
#
GNUSTEP_SYSTEM_APPS=`echo $GNUSTEP_SYSTEM_APPS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_SYSTEM_APPS,
  "$GNUSTEP_SYSTEM_APPS",
  [Built in default value for GNUstep system apps])

GNUSTEP_SYSTEM_ADMIN_APPS=`echo $GNUSTEP_SYSTEM_ADMIN_APPS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_SYSTEM_ADMIN_APPS,
  "$GNUSTEP_SYSTEM_ADMIN_APPS",
  [Built in default value for GNUstep system apps])

GNUSTEP_SYSTEM_WEB_APPS=`echo $GNUSTEP_SYSTEM_WEB_APPS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_SYSTEM_WEB_APPS,
  "$GNUSTEP_SYSTEM_WEB_APPS",
  [Built in default value for GNUstep web apps])

GNUSTEP_SYSTEM_TOOLS=`echo $GNUSTEP_SYSTEM_TOOLS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_SYSTEM_TOOLS,
  "$GNUSTEP_SYSTEM_TOOLS",
  [Built in default value for GNUstep system tools])

GNUSTEP_SYSTEM_ADMIN_TOOLS=`echo $GNUSTEP_SYSTEM_ADMIN_TOOLS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_SYSTEM_ADMIN_TOOLS,
  "$GNUSTEP_SYSTEM_ADMIN_TOOLS",
  [Built in default value for GNUstep system tools])

GNUSTEP_SYSTEM_LIBRARY=`echo $GNUSTEP_SYSTEM_LIBRARY|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_SYSTEM_LIBRARY,
  "$GNUSTEP_SYSTEM_LIBRARY",
  [Built in default value for GNUstep system library])

GNUSTEP_SYSTEM_LIBRARIES=`echo $GNUSTEP_SYSTEM_LIBRARIES|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_SYSTEM_LIBRARIES,
  "$GNUSTEP_SYSTEM_LIBRARIES",
  [Built in default value for GNUstep system libraries])

GNUSTEP_SYSTEM_HEADERS=`echo $GNUSTEP_SYSTEM_HEADERS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_SYSTEM_HEADERS,
  "$GNUSTEP_SYSTEM_HEADERS",
  [Built in default value for GNUstep system headers])

GNUSTEP_SYSTEM_DOC=`echo $GNUSTEP_SYSTEM_DOC|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_SYSTEM_DOC,
  "$GNUSTEP_SYSTEM_DOC",
  [Built in default value for GNUstep system documentation])

GNUSTEP_SYSTEM_DOC_MAN=`echo $GNUSTEP_SYSTEM_DOC_MAN|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_SYSTEM_DOC_MAN,
  "$GNUSTEP_SYSTEM_DOC_MAN",
  [Built in default value for GNUstep system manpages documentation])

GNUSTEP_SYSTEM_DOC_INFO=`echo $GNUSTEP_SYSTEM_DOC_INFO|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_SYSTEM_DOC_INFO,
  "$GNUSTEP_SYSTEM_DOC_INFO",
  [Built in default value for GNUstep system info documentation])

#
# NETWORK domain paths
#
GNUSTEP_NETWORK_APPS=`echo $GNUSTEP_NETWORK_APPS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_NETWORK_APPS,
  "$GNUSTEP_NETWORK_APPS",
  [Built in default value for GNUstep network apps])

GNUSTEP_NETWORK_ADMIN_APPS=`echo $GNUSTEP_NETWORK_ADMIN_APPS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_NETWORK_ADMIN_APPS,
  "$GNUSTEP_NETWORK_ADMIN_APPS",
  [Built in default value for GNUstep network apps])

GNUSTEP_NETWORK_WEB_APPS=`echo $GNUSTEP_NETWORK_WEB_APPS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_NETWORK_WEB_APPS,
  "$GNUSTEP_NETWORK_WEB_APPS",
  [Built in default value for GNUstep network web apps])

GNUSTEP_NETWORK_TOOLS=`echo $GNUSTEP_NETWORK_TOOLS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_NETWORK_TOOLS,
  "$GNUSTEP_NETWORK_TOOLS",
  [Built in default value for GNUstep network tools])

GNUSTEP_NETWORK_ADMIN_TOOLS=`echo $GNUSTEP_NETWORK_ADMIN_TOOLS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_NETWORK_ADMIN_TOOLS,
  "$GNUSTEP_NETWORK_ADMIN_TOOLS",
  [Built in default value for GNUstep system tools])

GNUSTEP_NETWORK_LIBRARY=`echo $GNUSTEP_NETWORK_LIBRARY|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_NETWORK_LIBRARY,
  "$GNUSTEP_NETWORK_LIBRARY",
  [Built in default value for GNUstep network library])

GNUSTEP_NETWORK_LIBRARIES=`echo $GNUSTEP_NETWORK_LIBRARIES|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_NETWORK_LIBRARIES,
  "$GNUSTEP_NETWORK_LIBRARIES",
  [Built in default value for GNUstep network libraries])

GNUSTEP_NETWORK_HEADERS=`echo $GNUSTEP_NETWORK_HEADERS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_NETWORK_HEADERS,
  "$GNUSTEP_NETWORK_HEADERS",
  [Built in default value for GNUstep network headers])

GNUSTEP_NETWORK_DOC=`echo $GNUSTEP_NETWORK_DOC|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_NETWORK_DOC,
  "$GNUSTEP_NETWORK_DOC",
  [Built in default value for GNUstep network documentation])

GNUSTEP_NETWORK_DOC_MAN=`echo $GNUSTEP_NETWORK_DOC_MAN|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_NETWORK_DOC_MAN,
  "$GNUSTEP_NETWORK_DOC_MAN",
  [Built in default value for GNUstep network manpages documentation])

GNUSTEP_NETWORK_DOC_INFO=`echo $GNUSTEP_NETWORK_DOC_INFO|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_NETWORK_DOC_INFO,
  "$GNUSTEP_NETWORK_DOC_INFO",
  [Built in default value for GNUstep network info documentation])

#
# LOCAL domain paths
#
GNUSTEP_LOCAL_APPS=`echo $GNUSTEP_LOCAL_APPS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_LOCAL_APPS,
  "$GNUSTEP_LOCAL_APPS",
  [Built in default value for GNUstep local apps])

GNUSTEP_LOCAL_ADMIN_APPS=`echo $GNUSTEP_LOCAL_ADMIN_APPS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_LOCAL_ADMIN_APPS,
  "$GNUSTEP_LOCAL_ADMIN_APPS",
  [Built in default value for GNUstep local apps])

GNUSTEP_LOCAL_WEB_APPS=`echo $GNUSTEP_LOCAL_WEB_APPS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_LOCAL_WEB_APPS,
  "$GNUSTEP_LOCAL_WEB_APPS",
  [Built in default value for GNUstep local web apps])

GNUSTEP_LOCAL_TOOLS=`echo $GNUSTEP_LOCAL_TOOLS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_LOCAL_TOOLS,
  "$GNUSTEP_LOCAL_TOOLS",
  [Built in default value for GNUstep local tools])

GNUSTEP_LOCAL_ADMIN_TOOLS=`echo $GNUSTEP_LOCAL_ADMIN_TOOLS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_LOCAL_ADMIN_TOOLS,
  "$GNUSTEP_LOCAL_ADMIN_TOOLS",
  [Built in default value for GNUstep local tools])

GNUSTEP_LOCAL_LIBRARY=`echo $GNUSTEP_LOCAL_LIBRARY|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_LOCAL_LIBRARY,
  "$GNUSTEP_LOCAL_LIBRARY",
  [Built in default value for GNUstep local library])

GNUSTEP_LOCAL_LIBRARIES=`echo $GNUSTEP_LOCAL_LIBRARIES|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_LOCAL_LIBRARIES,
  "$GNUSTEP_LOCAL_LIBRARIES",
  [Built in default value for GNUstep local libraries])

GNUSTEP_LOCAL_HEADERS=`echo $GNUSTEP_LOCAL_HEADERS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_LOCAL_HEADERS,
  "$GNUSTEP_LOCAL_HEADERS",
  [Built in default value for GNUstep local headers])

GNUSTEP_LOCAL_DOC=`echo $GNUSTEP_LOCAL_DOC|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_LOCAL_DOC,
  "$GNUSTEP_LOCAL_DOC",
  [Built in default value for GNUstep local documentation])

GNUSTEP_LOCAL_DOC_MAN=`echo $GNUSTEP_LOCAL_DOC_MAN|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_LOCAL_DOC_MAN,
  "$GNUSTEP_LOCAL_DOC_MAN",
  [Built in default value for GNUstep local manpages documentation])

GNUSTEP_LOCAL_DOC_INFO=`echo $GNUSTEP_LOCAL_DOC_INFO|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_LOCAL_DOC_INFO,
  "$GNUSTEP_LOCAL_DOC_INFO",
  [Built in default value for GNUstep local info documentation])

#
# USER_DIR domain paths
#
GNUSTEP_USER_DIR_APPS=`echo $GNUSTEP_USER_DIR_APPS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_USER_DIR_APPS,
  "$GNUSTEP_USER_DIR_APPS",
  [Built in default value for GNUstep user_dir apps])

GNUSTEP_USER_DIR_ADMIN_APPS=`echo $GNUSTEP_USER_DIR_ADMIN_APPS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_USER_DIR_ADMIN_APPS,
  "$GNUSTEP_USER_DIR_ADMIN_APPS",
  [Built in default value for GNUstep user_dir admin apps])

GNUSTEP_USER_DIR_WEB_APPS=`echo $GNUSTEP_USER_DIR_WEB_APPS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_USER_DIR_WEB_APPS,
  "$GNUSTEP_USER_DIR_WEB_APPS",
  [Built in default value for GNUstep user_dir web apps])

GNUSTEP_USER_DIR_TOOLS=`echo $GNUSTEP_USER_DIR_TOOLS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_USER_DIR_TOOLS,
  "$GNUSTEP_USER_DIR_TOOLS",
  [Built in default value for GNUstep user_dir tools])

GNUSTEP_USER_DIR_ADMIN_TOOLS=`echo $GNUSTEP_USER_DIR_ADMIN_TOOLS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_USER_DIR_ADMIN_TOOLS,
  "$GNUSTEP_USER_DIR_ADMIN_TOOLS",
  [Built in default value for GNUstep user_dir tools])

GNUSTEP_USER_DIR_LIBRARY=`echo $GNUSTEP_USER_DIR_LIBRARY|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_USER_DIR_LIBRARY,
  "$GNUSTEP_USER_DIR_LIBRARY",
  [Built in default value for GNUstep user_dir library])

GNUSTEP_USER_DIR_LIBRARIES=`echo $GNUSTEP_USER_DIR_LIBRARIES|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_USER_DIR_LIBRARIES,
  "$GNUSTEP_USER_DIR_LIBRARIES",
  [Built in default value for GNUstep user_dir libraries])

GNUSTEP_USER_DIR_HEADERS=`echo $GNUSTEP_USER_DIR_HEADERS|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_USER_DIR_HEADERS,
  "$GNUSTEP_USER_DIR_HEADERS",
  [Built in default value for GNUstep user_dir headers])

GNUSTEP_USER_DIR_DOC=`echo $GNUSTEP_USER_DIR_DOC|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_USER_DIR_DOC,
  "$GNUSTEP_USER_DIR_DOC",
  [Built in default value for GNUstep user_dir documentation])

GNUSTEP_USER_DIR_DOC_MAN=`echo $GNUSTEP_USER_DIR_DOC_MAN|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_USER_DIR_DOC_MAN,
  "$GNUSTEP_USER_DIR_DOC_MAN",
  [Built in default value for GNUstep user_dir manpages documentation])

GNUSTEP_USER_DIR_DOC_INFO=`echo $GNUSTEP_USER_DIR_DOC_INFO|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_USER_DIR_DOC_INFO,
  "$GNUSTEP_USER_DIR_DOC_INFO",
  [Built in default value for GNUstep user_dir info documentation])

GNUSTEP_TARGET_SYSTEM_USERS_DIR=`echo $GNUSTEP_TARGET_SYSTEM_USERS_DIR|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_SYSTEM_USERS_DIR,
  "$GNUSTEP_TARGET_SYSTEM_USERS_DIR",
  [Built in default value for GNUstep System Users directory])
GNUSTEP_TARGET_NETWORK_USERS_DIR=`echo $GNUSTEP_TARGET_NETWORK_USERS_DIR|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_NETWORK_USERS_DIR,
  "$GNUSTEP_TARGET_NETWORK_USERS_DIR",
  [Built in default value for GNUstep Network Users directory])
GNUSTEP_TARGET_LOCAL_USERS_DIR=`echo $GNUSTEP_TARGET_LOCAL_USERS_DIR|sed -e 's/\\\\/\\\\\\\\/g'`
AC_DEFINE_UNQUOTED(GNUSTEP_TARGET_LOCAL_USERS_DIR,
  "$GNUSTEP_TARGET_LOCAL_USERS_DIR",
  [Built in default value for GNUstep Local Users directory])

#
# Now load the values to be used in locating libraries etc used when
# building the base library ... as supplied by the gnustep-make package
#
# It looks like we ought to source the whole GNUstep.sh here, and even
# ask it to output all variables!  That way we have access to (eg)
# GNUSTEP_SYSTEM_HEADERS below.
#
# We need to unset any values that we really need, or existing settings
# would be used by GNUstep.sh
#
unset GNUSTEP_SYSTEM_HEADERS
unset GNUSTEP_SYSTEM_LIBRARIES
unset GNUSTEP_NETWORK_HEADERS
unset GNUSTEP_NETWORK_LIBRARIES
unset GNUSTEP_LOCAL_HEADERS
unset GNUSTEP_LOCAL_LIBRARIES
GNUSTEP_MAKEFILES="$CURRENT_GNUSTEP_MAKEFILES"
GNUSTEP_SH_EXPORT_ALL_VARIABLES=yes
. "$CURRENT_GNUSTEP_MAKEFILES/GNUstep.sh"
unset GNUSTEP_SH_EXPORT_ALL_VARIABLES

# For backwards compatibility, define GNUSTEP_SYSTEM_HEADERS from
# GNUSTEP_SYSTEM_ROOT if not set yet.
if test x"$GNUSTEP_SYSTEM_HEADERS" = x""; then
  GNUSTEP_SYSTEM_HEADERS="$GNUSTEP_SYSTEM_ROOT/Library/Headers"
fi

if test x"$GNUSTEP_SYSTEM_LIBRARIES" = x""; then
  GNUSTEP_SYSTEM_LIBRARIES="$GNUSTEP_SYSTEM_ROOT/Library/Libraries"
fi

if test x"$GNUSTEP_NETWORK_HEADERS" = x""; then
  GNUSTEP_NETWORK_HEADERS="$GNUSTEP_NETWORK_ROOT/Library/Headers"
fi

if test x"$GNUSTEP_NETWORK_LIBRARIES" = x""; then
  GNUSTEP_NETWORK_LIBRARIES="$GNUSTEP_NETWORK_ROOT/Library/Libraries"
fi

if test x"$GNUSTEP_LOCAL_HEADERS" = x""; then
  GNUSTEP_LOCAL_HEADERS="$GNUSTEP_LOCAL_ROOT/Library/Headers"
fi

if test x"$GNUSTEP_LOCAL_LIBRARIES" = x""; then
  GNUSTEP_LOCAL_LIBRARIES="$GNUSTEP_LOCAL_ROOT/Library/Libraries"
fi

#
# Add standard library and header directories for configure to use to locate
# plain C developer headers/libraries which have been installed in the
# GNUstep hierarchy. These take precedence
#
if test x"$GNUSTEP_IS_FLATTENED" = x"yes"; then
  CPPFLAGS="$CPPFLAGS -I$GNUSTEP_LOCAL_HEADERS"
  CPPFLAGS="$CPPFLAGS -I$GNUSTEP_NETWORK_HEADERS"
  CPPFLAGS="$CPPFLAGS -I$GNUSTEP_SYSTEM_HEADERS"
  LDFLAGS="$LDFLAGS -L$GNUSTEP_LOCAL_LIBRARIES"
  LDFLAGS="$LDFLAGS -L$GNUSTEP_NETWORK_LIBRARIES"
  LDFLAGS="$LDFLAGS -L$GNUSTEP_SYSTEM_LIBRARIES"
else
  # FIXME: Cross-compiling should read the target from the configure
  # options and use it.   GNUSTEP_TARGET_* variables do not exist in
  # this context (so "$GNUSTEP_TARGET_CPU" will always be "")
  if test x"$GNUSTEP_TARGET_CPU" = x""; then
    ARCH="$GNUSTEP_HOST_CPU-$GNUSTEP_HOST_OS"
  else
    ARCH=$GNUSTEP_TARGET_CPU-$GNUSTEP_TARGET_OS
  fi
  ALIB="$ARCH/$LIBRARY_COMBO"
  CPPFLAGS="$CPPFLAGS -I$GNUSTEP_LOCAL_HEADERS/$ALIB"
  CPPFLAGS="$CPPFLAGS -I$GNUSTEP_LOCAL_HEADERS/$ARCH"
  CPPFLAGS="$CPPFLAGS -I$GNUSTEP_LOCAL_HEADERS"
  CPPFLAGS="$CPPFLAGS -I$GNUSTEP_NETWORK_HEADERS/$ALIB"
  CPPFLAGS="$CPPFLAGS -I$GNUSTEP_NETWORK_HEADERS/$ARCH"
  CPPFLAGS="$CPPFLAGS -I$GNUSTEP_NETWORK_HEADERS"
  CPPFLAGS="$CPPFLAGS -I$GNUSTEP_SYSTEM_HEADERS/$ALIB"
  CPPFLAGS="$CPPFLAGS -I$GNUSTEP_SYSTEM_HEADERS/$ARCH"
  CPPFLAGS="$CPPFLAGS -I$GNUSTEP_SYSTEM_HEADERS"
  LDFLAGS="$LDFLAGS -L$GNUSTEP_LOCAL_LIBRARIES/$ALIB"
  LDFLAGS="$LDFLAGS -L$GNUSTEP_LOCAL_LIBRARIES/$ARCH"
  LDFLAGS="$LDFLAGS -L$GNUSTEP_LOCAL_LIBRARIES"
  LDFLAGS="$LDFLAGS -L$GNUSTEP_NETWORK_LIBRARIES/$ALIB"
  LDFLAGS="$LDFLAGS -L$GNUSTEP_NETWORK_LIBRARIES/$ARCH"
  LDFLAGS="$LDFLAGS -L$GNUSTEP_NETWORK_LIBRARIES"
  LDFLAGS="$LDFLAGS -L$GNUSTEP_SYSTEM_LIBRARIES/$ALIB"
  LDFLAGS="$LDFLAGS -L$GNUSTEP_SYSTEM_LIBRARIES/$ARCH"
  LDFLAGS="$LDFLAGS -L$GNUSTEP_SYSTEM_LIBRARIES"
fi


#--------------------------------------------------------------------
# Find the compiler
#--------------------------------------------------------------------
MAKECC=`gnustep-config --variable=CC`
MAKECPP=`gnustep-config --variable=CPP`
MAKECXX=`gnustep-config --variable=CXX`
if test "$CC" = ""; then
  CC=$MAKECC
  export CC
else
  if test "$CC" != "$MAKECC"; then
    AC_MSG_WARN([You are running configure with the compiler ($CC) set to a different value from that used by gnustep-make ($MAKECC).  To avoid conflicts/problems, reconfigure/reinstall gnustep-make to use CC=$CC or run the gnustep-base configure again with your CC environment variable set to $MAKECC])
  fi
fi
if test "$CPP" = ""; then
  CPP=$MAKECPP
  export CPP
else
  if test "$CPP" != "$MAKECPP"; then
    AC_MSG_WARN([You are running configure with the preprocessor ($CPP) set to a different value from that used by gnustep-make ($MAKECPP).  To avoid conflicts/problems, reconfigure/reinstall gnustep-make to use CPP=$CPP or run the gnustep-base configure again with your CPP environment variable set to $MAKECPP])
  fi
fi
if test "$CXX" = ""; then
  CXX=$MAKECXX
  export CXX
else
  if test "$CXX" != "$MAKECXX"; then
    AC_MSG_WARN([You are running configure with the compiler ($CXX) set to a different value from that used by gnustep-make ($MAKECXX).  To avoid conflicts/problems, reconfigure/reinstall gnustep-make to use CXX=$CXX or run the gnustep-base configure again with your CXX environment variable set to $MAKECXX])
  fi
fi
AC_PROG_CC
AC_PROG_CPP
AC_USE_SYSTEM_EXTENSIONS

AC_MSG_CHECKING(whether the compiler is clang)
if test ! x"${GCC}" = x"yes" ; then
  CLANG_CC=no
  AC_MSG_RESULT(no)
else
  if "${CC}" -v 2>&1 | grep -q 'clang version'; then
    CLANG_CC=yes
    AC_MSG_RESULT(yes)
  else
    CLANG_CC=no
    AC_MSG_RESULT(no)
  fi
fi
AC_SUBST(CLANG_CC)


# Large file support needed by NSData/NSFileHandle.
# These macros must be called after AC_USE_SYSTEM_EXTENSIONS because
# the `fseeko' declaration may be hidden by default on some systems.
AC_FUNC_FSEEKO
AH_BOTTOM([
/* Define `fseeko' to `fseek' if the former is missing.
   Likewise for `ftello'.  */
#if !HAVE_FSEEKO
# define fseeko fseek
# define ftello ftell
#endif
])
AC_SYS_LARGEFILE
AC_TYPE_OFF_T

#--------------------------------------------------------------------
# Check how to enable builtins for atomic operations
#--------------------------------------------------------------------

AC_LANG_PUSH(C)
AC_MSG_CHECKING([whether the compiler supports atomic operations]);
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[typedef int atomic;]],
    [[atomic x; atomic y; __sync_bool_compare_and_swap(&x, y, y + 1);]])],
  have_atomic=yes,
  have_atomic=no);
  if test "$have_atomic" = "yes"; then
    AC_MSG_RESULT([yes]);
    AC_DEFINE(USE_ATOMIC_BUILTINS,1,
      [Define if the compiler provides builtins for atomic operations])
  else
    AC_MSG_RESULT([no]);
    if test "$CC" = "gcc"; then
      saved_CFLAGS="$CFLAGS";
      ATOMIC_CFLAGS="";

      # FIXME: Forcing -march=i568 for any i568 or later CPU is a
      # stop gap measure to make the compiler emit native assembly
      # for atomic operations on i586 or latter processors (GCC by
      # default emits code compatible with the original i386 and
      # requires library functions to emulate atomic operations).
      # When gnustep-make takes care of this kind of target setting,
      # the check can safely be removed.
      case "$target_cpu" in
        i586*|i686*|i786*)
          ATOMIC_CFLAGS="-march=i586"
          CFLAGS="$saved_CFLAGS $ATOMIC_CFLAGS"
          ;;
        x86_64)
          ATOMIC_CFLAGS="-march=x86-64"
          CFLAGS="$saved_CFLAGS $ATOMIC_CFLAGS"
          ;;
      esac
      AC_MSG_CHECKING([whether CC supports atomic operations with -march]);
        AC_LINK_IFELSE([AC_LANG_PROGRAM([[typedef int atomic;]],
          [[atomic x; atomic y; __sync_bool_compare_and_swap(&x, y, y+1);]])],
        need_march=yes,
      need_march=no);

      if test "$need_march" = "yes"; then
        AC_MSG_RESULT([yes]);
	OBJCFLAGS="$OBJCFLAGS $ATOMIC_CFLAGS";
        AC_DEFINE(USE_ATOMIC_BUILTINS,1,
          [Define if the compiler provides builtins for atomic operations])
      else
        AC_MSG_RESULT([no]);
        saved_LDFLAGS="$LDFLAGS";
        LDFLAGS="$saved_LDFLAGS -lgcc";
        AC_MSG_CHECKING([whether CC supports atomic operations using libgcc]);
          AC_LINK_IFELSE([AC_LANG_PROGRAM([[typedef int atomic;]],
            [[atomic x; atomic y; __sync_bool_compare_and_swap(&x, y, y+1);]])],
          atomic_in_libgcc=yes,
        atomic_in_libgcc=no);
        if test "$atomic_in_libgcc" = "yes"; then
          AC_MSG_RESULT([yes]);
          LIBS="$LIBS -lgcc";
          AC_DEFINE(USE_ATOMIC_BUILTINS,1,
            [Define if the compiler provides builtins for atomic operations])
        else
          LDFLAGS="$saved_LDFLAGS";
          AC_MSG_RESULT([no]);
        fi
      fi
    fi
  fi
AC_LANG_POP(C)


AC_PATH_PROG(WHOAMI, whoami, echo, $PATH:/usr/ucb)
PKG_PROG_PKG_CONFIG
# If the modern mechanism failswe may havew an older pkg-config
# so try looking it up the old fashioned way.
if test -z "$PKG_CONFIG"; then
  AC_PATH_PROG(PKG_CONFIG,pkg-config,,)
fi


#--------------------------------------------------------------------
# specific target_os options
#--------------------------------------------------------------------
INCLUDE_FLAGS="$CPPFLAGS"
LDIR_FLAGS="$LDFLAGS"
# This is just for configuring. Later, in config.make, INCLUDE_FLAGS
# goes in CONFIG_SYSTEM_INCL and LIBS goes in CONFIG_SYSTEM_LIBS
case "$target_os" in
  freebsd* | openbsd* )
	        CPPFLAGS="$CPPFLAGS -I/usr/local/include"
		LDFLAGS="$LDFLAGS -L/usr/local/lib";;
  netbsd*)	CPPFLAGS="$CPPFLAGS -I/usr/pkg/include"
		LDFLAGS="$LDFLAGS -Wl,-R/usr/pkg/lib -L/usr/pkg/lib";;
  linux-android* )
		# link against libandroid for native application APIs
		LIBS="$LIBS -landroid";;
esac

#--------------------------------------------------------------------
# Set Apple/Darwin/OSX/NeXT information for other tests
#--------------------------------------------------------------------
AC_MSG_CHECKING(the Objective-C runtime)
if test "$OBJC_RUNTIME_LIB" = "nx" -o "$OBJC_RUNTIME_LIB" = "apple"; then
  AC_MSG_RESULT(NeXT)
  OBJCFLAGS="$OBJCFLAGS -fnext-runtime -DNeXT_RUNTIME"
elif test "$OBJC_RUNTIME_LIB" = "ng"; then
  AC_MSG_RESULT(Next Gen)
else
  AC_MSG_RESULT(GNU)
  if test "$CLANG_CC" = "yes"; then
    OBJCFLAGS="$OBJCFLAGS -fobjc-runtime=gcc"
  fi
fi
LIBOBJC=`gnustep-config --objc-libs`
if test "$LIBOBJC" = ""; then
  AC_MSG_ERROR([The command 'gnustep-config --objc-libs' (provided by the gnustep-make package) returned no Objective-C library.  Unable to continue configuring without Objective-C support.])
  exit 1
fi

#--------------------------------------------------------------------
# Miscellaneous flags
#--------------------------------------------------------------------

# Check to see if the libobjc library is in our GNUSTEP_SYSTEM_LIBRARIES.
# If so, there are probably other libraries that we want there also, so
# leave the proper includes in CPPFLAGS and LDFLAGS
AC_MSG_CHECKING(for custom shared objc library)
AC_CACHE_VAL(gs_cv_objc_libdir,
[dnl
gs_cv_objc_libdir=NONE
gs_cv_objc_incdir=NONE
# Try GNUSTEP_SYSTEM_LIBRARIES first
if test "$GNUSTEP_IS_FLATTENED" = yes; then
  GNUSTEP_LDIR="$GNUSTEP_SYSTEM_LIBRARIES"
  GNUSTEP_HDIR="$GNUSTEP_SYSTEM_HEADERS"
else
  GNUSTEP_LDIR="$GNUSTEP_SYSTEM_LIBRARIES/$obj_dir"
  GNUSTEP_HDIR="$GNUSTEP_SYSTEM_HEADERS/$LIBRARY_COMBO"
fi

if test -f "$GNUSTEP_HDIR/objc/objc.h"; then
  if test -f "$GNUSTEP_LDIR/libobjc.a" -o -f "$GNUSTEP_LDIR/libobjc.so" -o -f "$GNUSTEP_LDIR/libobjc.dll.a" -o -f "$GNUSTEP_LDIR/libobjc-gnu.dylib" -o -f "$GNUSTEP_LDIR/objc.lib"; then
    gs_cv_objc_libdir="$GNUSTEP_LDIR"
    gs_cv_objc_incdir="$GNUSTEP_HDIR"
  fi
fi

# Try GNUSTEP_NETWORK_LIBRARIES second (override GNUSTEP_SYSTEM if
# found)
if test "$GNUSTEP_IS_FLATTENED" = yes; then
  GNUSTEP_LDIR="$GNUSTEP_NETWORK_LIBRARIES"
  GNUSTEP_HDIR="$GNUSTEP_NETWORK_HEADERS"
else
  GNUSTEP_LDIR="$GNUSTEP_NETWORK_LIBRARIES/$obj_dir"
  GNUSTEP_HDIR="$GNUSTEP_NETWORK_HEADERS/$LIBRARY_COMBO"
fi

if test -f "$GNUSTEP_HDIR/objc/objc.h"; then
  if test -f "$GNUSTEP_LDIR/libobjc.a" -o -f "$GNUSTEP_LDIR/libobjc.so" -o -f "$GNUSTEP_LDIR/libobjc.dll.a" -o -f "$GNUSTEP_LDIR/libobjc-gnu.dylib" -o -f "$GNUSTEP_LDIR/objc.lib"; then
    gs_cv_objc_libdir="$GNUSTEP_LDIR"
    gs_cv_objc_incdir="$GNUSTEP_HDIR"
  fi
fi

# Try GNUSTEP_LOCAL_LIBRARIES third (override GNUSTEP_SYSTEM and
# GNUSTEP_NETWORK if found)
if test "$GNUSTEP_IS_FLATTENED" = yes; then
  GNUSTEP_LDIR="$GNUSTEP_LOCAL_LIBRARIES"
  GNUSTEP_HDIR="$GNUSTEP_LOCAL_HEADERS"
else
  GNUSTEP_LDIR="$GNUSTEP_LOCAL_LIBRARIES/$obj_dir"
  GNUSTEP_HDIR="$GNUSTEP_LOCAL_HEADERS/$LIBRARY_COMBO"
fi

if test -f "$GNUSTEP_HDIR/objc/objc.h"; then
  if test -f "$GNUSTEP_LDIR/libobjc.a" -o -f "$GNUSTEP_LDIR/libobjc.so" -o -f "$GNUSTEP_LDIR/libobjc.dll.a" -o -f "$GNUSTEP_LDIR/libobjc-gnu.dylib" -o -f "$GNUSTEP_LDIR/objc.lib"; then
    gs_cv_objc_libdir="$GNUSTEP_LDIR"
    gs_cv_objc_incdir="$GNUSTEP_HDIR"
  fi
fi
])
AC_MSG_RESULT($gs_cv_objc_libdir)

if test "$gs_cv_objc_libdir" != "NONE"; then
  #
  # The following one is so that headers of custom libraries into
  # $GNUSTEP_HDIR are used before the standard ones
  #
  INCLUDE_FLAGS="$INCLUDE_FLAGS -I$gs_cv_objc_incdir"
  LDIR_FLAGS="$LDIR_FLAGS -L$gs_cv_objc_libdir/$LIBRARY_COMBO -L$gs_cv_objc_libdir"
  CPPFLAGS="$CPPFLAGS -I$gs_cv_objc_incdir"
  LDFLAGS="$LDFLAGS -L$gs_cv_objc_libdir"

  # add to path on Windows for config checks to find DLL at runtime
  case $host_os in
    mingw*|windows*) PATH=$PATH:$gs_cv_objc_libdir;;
  esac
fi

#--------------------------------------------------------------------
# Check if Objective-C is installed
#--------------------------------------------------------------------
AC_CHECK_HEADERS(objc/runtime.h)
AC_CHECK_HEADERS(objc/objc.h)
if test $ac_cv_header_objc_objc_h = no; then
  echo "Check to make sure you have the Objective-C runtime library"
  echo "and its headers installed."
  AC_MSG_ERROR(Could not find Objective-C headers)
fi

#--------------------------------------------------------------------
# Check for strange network stuff used by gdomap
#--------------------------------------------------------------------
AC_MSG_NOTICE(for gdomap network details)
AC_MSG_CHECKING(for variable length socket addresses)
AC_TRY_COMPILE([
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <net/if.h>
  ],
  [struct ifreq s; s.ifr_addr.sa_len = 0;],
  sa_len=1, sa_len=0)
if test $sa_len = 1; then
  AC_MSG_RESULT([found])
  AC_DEFINE(HAVE_SA_LEN,1,
            [Define if your system has variable length network addresses])
else
  AC_MSG_RESULT([not found])
fi

AC_MSG_CHECKING(for compiler visibility attribute support)
saved_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Werror"
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
  #include <stdio.h>
  int foo() __attribute__ ((visibility("internal")));
  int foo(){ return 1; }
  int main(){ return foo(); }])],
  AC_MSG_RESULT([found])
  gs_visibility=1,
  AC_MSG_RESULT([not present])
  gs_visibility=0
)
AC_DEFINE_UNQUOTED(HAVE_VISIBILITY_ATTRIBUTE,$gs_visibility,
  [Says whether the visibility attribute works])
CFLAGS="$saved_CFLAGS"


#--------------------------------------------------------------------
# Check if system has buggy SO_REUSEADDR
#--------------------------------------------------------------------
AC_MSG_CHECKING(whether SO_REUSEADDR is broken)
AC_TRY_RUN([#include "$srcdir/config/config.reuseaddr.c"],
	reuseaddr_ok=1,
	reuseaddr_ok=0,
	reuseaddr_ok="$cross_reuseaddr_ok")
if test $reuseaddr_ok = 0; then
  AC_DEFINE(BROKEN_SO_REUSEADDR,1,
            [Define if SO_REUSEADDR is broken on this system])
  echo
  echo "The SO_REUSEADDR socket option for controlling re-use of network"
  echo "sockets immediately after shutdown appears to be broken on this"
  echo "machine.  Networking code will be built without using this"
  echo "feature."
  echo "The effect of this lack is that when a network service is shut"
  echo "down, it cannot be re-started on the same network port until"
  echo "an operating-system timeout has expired."
  echo "For servers other than gdomap, GNUstep does not normally need"
  echo "a particular network port, so the problem is unlikely to arise."
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

#--------------------------------------------------------------------
# Check for thread flags for libobjc.
#--------------------------------------------------------------------
#
AC_MSG_CHECKING(for objc threading flags)
#
# Get them from gnustep-make which contains the real code to get them
#
objc_threaded=`grep objc_threaded: $CURRENT_GNUSTEP_MAKEFILES/$lobj_dir/config.make | sed -e 's/objc_threaded:=//'`
#
AC_MSG_RESULT($objc_threaded)

#--------------------------------------------------------------------
# Byte order information needed for foundation headers.
#--------------------------------------------------------------------
AC_C_BIGENDIAN
if test $ac_cv_c_bigendian = yes; then
  GS_WORDS_BIGENDIAN=1
else
  GS_WORDS_BIGENDIAN=0
fi
AC_SUBST(GS_WORDS_BIGENDIAN)

#--------------------------------------------------------------------
# Type size information needed for foundation headers.
#--------------------------------------------------------------------
AC_CHECK_SIZEOF(void*)

GS_SINT8="signed char"
GS_UINT8="unsigned char"
AC_SUBST(GS_SINT8)
AC_SUBST(GS_UINT8)

AC_CHECK_SIZEOF(short)
AC_SUBST(ac_cv_sizeof_short)

AC_CHECK_SIZEOF(int)
AC_SUBST(ac_cv_sizeof_int)

AC_CHECK_SIZEOF(long)
AC_SUBST(ac_cv_sizeof_long)

AC_CHECK_SIZEOF(long long)
AC_SUBST(ac_cv_sizeof_long_long)

AC_CHECK_SIZEOF(float)
AC_SUBST(ac_cv_sizeof_float)

AC_CHECK_SIZEOF(double)
AC_SUBST(ac_cv_sizeof_double)

AC_SUBST(ac_cv_sizeof_voidp)
if test $ac_cv_sizeof_voidp = $ac_cv_sizeof_int; then
  GS_SADDR="int"
  GS_UADDR="unsigned int"
else
  if test $ac_cv_sizeof_voidp = $ac_cv_sizeof_long; then
    GS_SADDR="long"
    GS_UADDR="unsigned long"
  else
    if test $ac_cv_sizeof_voidp = $ac_cv_sizeof_long_long; then
      GS_SADDR="long long"
      GS_UADDR="unsigned long long"
    else
      AC_MSG_ERROR([Unable to find integer of same size as void*])
    fi
  fi
fi
AC_SUBST(GS_SADDR)
AC_SUBST(GS_UADDR)

if test $ac_cv_sizeof_short = 2; then
  GS_SINT16="signed short"
  GS_UINT16="unsigned short"
else
  if test $ac_cv_sizeof_int = 2; then
    GS_SINT16="signed int"
    GS_UINT16="unsigned int"
  else
    AC_MSG_ERROR([Unable to determine type for 16-bit integer])
  fi
fi
AC_SUBST(GS_SINT16)
AC_SUBST(GS_UINT16)

if test $ac_cv_sizeof_int = 4; then
  GS_SINT32="signed int"
  GS_UINT32="unsigned int"
else
  if test $ac_cv_sizeof_long = 4; then
    GS_SINT32="signed long"
    GS_UINT32="unsigned long"
  else
    if test $ac_cv_sizeof_short = 4; then
      GS_SINT32="signed short"
      GS_UINT32="unsigned short"
    else
      AC_MSG_ERROR([Unable to determine type for 32-bit integer])
    fi
  fi
fi
AC_SUBST(GS_SINT32)
AC_SUBST(GS_UINT32)

GS_HAVE_I64=1
if test $ac_cv_sizeof_int = 8; then
  GS_SINT64="signed int"
  GS_UINT64="unsigned int"
else
  if test $ac_cv_sizeof_long = 8; then
    GS_SINT64="signed long"
    GS_UINT64="unsigned long"
  else
    if test $ac_cv_sizeof_long_long = 8; then
      GS_SINT64="signed long long"
      GS_UINT64="unsigned long long"
    else
#     64-bit ints not supported - but we need a dummy type for byte-swapping
#     of 64-bit values arriving from another system.
      GS_SINT64="struct { gsu8 a[8]; }"
      GS_UINT64="struct { gsu8 a[8]; }"
      GS_HAVE_I64=0
    fi
  fi
fi
AC_SUBST(GS_SINT64)
AC_SUBST(GS_UINT64)
AC_SUBST(GS_HAVE_I64)

GS_HAVE_I128=1
if test $ac_cv_sizeof_long = 16; then
  GS_SINT128="signed long"
  GS_UINT128="unsigned long"
else
  if test $ac_cv_sizeof_long_long = 16; then
    GS_SINT128="signed long long"
    GS_UINT128="unsigned long long"
  else
#     128-bit ints not supported - but we need a dummy type for byte-swapping
#     of 128-bit values arriving from another system.
    GS_SINT128="struct { gsu8 a[[16]]; }"
    GS_UINT128="struct { gsu8 a[[16]]; }"
    GS_HAVE_I128=0
  fi
fi
AC_SUBST(GS_SINT128)
AC_SUBST(GS_UINT128)
AC_SUBST(GS_HAVE_I128)

if test $ac_cv_sizeof_float = 4; then
  GS_FLT32="float"
else
  AC_MSG_ERROR([Unable to determine type for 32-bit float])
fi
AC_SUBST(GS_FLT32)

if test $ac_cv_sizeof_double = 8; then
  GS_FLT64="double"
else
  AC_MSG_ERROR([Unable to determine type for 64-bit float])
fi
AC_SUBST(GS_FLT64)

#--------------------------------------------------------------------
# Type-size information for encoding into archives using NSArchiver etc.
#--------------------------------------------------------------------
if test $ac_cv_sizeof_short = 2; then
  _GSC_S_SHT=_GSC_I16
else
  _GSC_S_SHT=_GSC_I32
fi
AC_SUBST(_GSC_S_SHT)

if test $ac_cv_sizeof_int = 2; then
  _GSC_S_INT=_GSC_I16
else
  if test $ac_cv_sizeof_int = 4; then
    _GSC_S_INT=_GSC_I32
  else
    if test $ac_cv_sizeof_int = 8; then
      _GSC_S_INT=_GSC_I64
    else
      if test $ac_cv_sizeof_int = 16; then
	_GSC_S_INT=_GSC_I128
      fi
    fi
  fi
fi
AC_SUBST(_GSC_S_INT)

if test $ac_cv_sizeof_long = 4; then
  _GSC_S_LNG=_GSC_I32
else
  if test $ac_cv_sizeof_long = 8; then
    _GSC_S_LNG=_GSC_I64
  else
    if test $ac_cv_sizeof_long = 16; then
      _GSC_S_LNG=_GSC_I128
    fi
  fi
fi
AC_SUBST(_GSC_S_LNG)

if test $ac_cv_sizeof_long_long = 4; then
  _GSC_S_LNG_LNG=_GSC_I32
else
  if test $ac_cv_sizeof_long_long = 8; then
    _GSC_S_LNG_LNG=_GSC_I64
  else
    if test $ac_cv_sizeof_long_long = 16; then
      _GSC_S_LNG_LNG=_GSC_I128
    fi
  fi
fi
AC_SUBST(_GSC_S_LNG_LNG)


#--------------------------------------------------------------------
# Limit information needed for foundation headers.
#--------------------------------------------------------------------
AC_LANG_PUSH(C)
AC_MSG_CHECKING([for working INTPTR_MAX, INTPTR_MIN, UINTPTR_MAX]);
AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([[#include <inttypes.h>]],
    [[int imax = INTPTR_MAX; int imin = INTPTR_MIN; unsigned umax = UINTPTR_MAX;]])],
  have_valid_ptr_limits=yes,
  have_valid_ptr_limits=no);
  if test "$have_valid_ptr_limits" = "yes"; then
    AC_MSG_RESULT([yes]);
    BUGGY_PTR_LIMITS=0
  else
    AC_MSG_RESULT([no]);
    BUGGY_PTR_LIMITS=1
    case "$target_os" in
      solaris*)
        case "$target_cpu" in
          sparc64*|x86_64*)
            GS_INTPTR_MAX="INT64_MAX"
            GS_INTPTR_MIN="INT64_MIN"
            GS_UINTPTR_MAX="UINT64_MAX"
            ;;
          sparc*|i386*|i586*|i686*)
            GS_INTPTR_MAX="INT32_MAX"
            GS_INTPTR_MIN="INT32_MIN"
            GS_UINTPTR_MAX="UINT32_MAX"
            ;;
          *) ;;
        esac ;;
      # support for other OS's with broken macros to be added here (HP-UX, IRIX being known)
      *)
      ;;
    esac
    AC_SUBST(GS_INTPTR_MAX)
    AC_SUBST(GS_INTPTR_MIN)
    AC_SUBST(GS_UINTPTR_MAX)
  fi
AC_SUBST(BUGGY_PTR_LIMITS)
AC_LANG_POP(C)

#--------------------------------------------------------------------
# Setup dynamic linking
#--------------------------------------------------------------------
OBJC_SYS_DYNAMIC_LINKER()
# NOTE: libdl should be in LIBS now if it's available.
AC_CHECK_FUNCS(dladdr)

AC_CHECK_FUNCS(gethostbyname)
if test "$ac_cv_func_gethostbyname" = "no"; then
  # QNX has gethostbyname and friends in libsocket
  AC_CHECK_LIB(socket, gethostbyname)
fi

AC_CHECK_FUNCS(getaddrinfo)

#--------------------------------------------------------------------
# Check for pthread.h
#--------------------------------------------------------------------
AC_CHECK_HEADERS([pthread.h])
AC_CHECK_HEADERS([pthread_np.h],[],[],[AC_INCLUDES_DEFAULT
		  #ifdef HAVE_PTHREAD_H
		  #include <pthread.h>
		  #endif
		  ])
if test $ac_cv_header_pthread_h = yes ; then
  AC_CHECK_SIZEOF(pthread_mutex_t,,[AC_INCLUDES_DEFAULT
#include <pthread.h>])
  GS_SIZEOF_MUTEX_T=$ac_cv_sizeof_pthread_mutex_t
  if test $ac_cv_sizeof_pthread_mutex_t = 0 ; then
    AC_MSG_ERROR([Unable to find size of pthread_mutex_t (required).])
  fi
  AC_SUBST(GS_SIZEOF_MUTEX_T)
# pthread_mutex_t.__data.__owner is non-standard since pthread_mutex_t is
# nominally an opaque type.  We must not rely on this for anything other
# than debug output!
  AC_CHECK_MEMBER([pthread_mutex_t.__data.__owner],,,[AC_INCLUDES_DEFAULT
#include <pthread.h>])
  if test $ac_cv_member_pthread_mutex_t___data___owner = yes ; then
    AC_DEFINE(HAVE_PTHREAD_MUTEX_OWNER, 1,
      [Define if you have pthread_mutex_t.__data.__owner])
  fi
  AC_CHECK_SIZEOF(pthread_cond_t,,[AC_INCLUDES_DEFAULT
#include <pthread.h>])
  if test $ac_cv_sizeof_pthread_cond_t = 0 ; then
    AC_MSG_ERROR([Unable to find size of pthread_cond_t (required).])
  fi
  GS_SIZEOF_COND_T=$ac_cv_sizeof_pthread_cond_t
  AC_SUBST(GS_SIZEOF_COND_T)
  AC_CHECK_ALIGNOF(pthread_mutex_t,[AC_INCLUDES_DEFAULT
#include <pthread.h>])
  GS_ALIGNOF_MUTEX_T=$ac_cv_alignof_pthread_mutex_t
  if test $ac_cv_alignof_pthread_mutex_t = 0 ; then
    AC_MSG_ERROR([Unable to find align of pthread_mutex_t (required).])
  fi
  AC_SUBST(GS_ALIGNOF_MUTEX_T)
  AC_CHECK_ALIGNOF(pthread_cond_t,[AC_INCLUDES_DEFAULT
#include <pthread.h>])
  if test $ac_cv_alignof_pthread_cond_t = 0 ; then
    AC_MSG_ERROR([Unable to find align of pthread_cond_t (required).])
  fi
  GS_ALIGNOF_COND_T=$ac_cv_alignof_pthread_cond_t
  AC_SUBST(GS_ALIGNOF_COND_T)
else
  AC_MSG_ERROR([Unable to find pthread.h (needed for thread support).])
fi
AC_CHECK_LIB(pthread, pthread_join, pthread_ok=yes, pthread_ok=no)
ismingw=no
checkinlibc=no
case "$target_os" in
  mingw*)	ismingw=yes;;
  nto*)		checkinlibc=yes;;
  qnx*)		checkinlibc=yes;;
  *android*)   checkinlibc=yes;;
esac
if test $pthread_ok = yes ; then
  LIBS="$LIBS -lpthread"
else
  if test $ismingw = yes ; then
    AC_CHECK_LIB(pthreadGC2, pthread_join, pthread_ok=yes, pthread_ok=no)
    if test $pthread_ok = yes ; then
      LIBS="$LIBS -lpthreadGC2"
    fi
  fi
  # Android and QNX have pthread in libc instead of libpthread
  if test $checkinlibc = yes ; then
    AC_CHECK_LIB(c, pthread_join, pthread_ok=yes, pthread_ok=no)
    if test $pthread_ok = yes ; then
       LIBS="$LIBS -lc"
    fi
  fi
fi
if test $pthread_ok = no ; then
  AC_MSG_ERROR([Unable to find pthread library (needed for thread support).])
fi

# Check threading extensions
AC_CHECK_FUNCS(pthread_getthreadid_np pthread_main_np)

# Typically need librt on Solaris for sched_yield
AC_CHECK_LIB(rt, sched_yield)


#--------------------------------------------------------------------
# One of these function needed by NSThread.m and objc initialize test
#--------------------------------------------------------------------
AC_CHECK_FUNCS(nanosleep usleep Sleep)

AC_MSG_CHECKING(for objc_root_class attribute support)
saved_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Werror $OBJCFLAGS -x objective-c"
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
  __attribute__((objc_root_class)) @interface RootObject
  @end
  @implementation RootObject
  @end])],
  AC_MSG_RESULT([found])
  gs_objc_root_class_attr=1,
  AC_MSG_RESULT([not present])
  gs_objc_root_class_attr=0
)

GS_HAVE_OBJC_ROOT_CLASS_ATTR=$gs_objc_root_class_attr
AC_SUBST(GS_HAVE_OBJC_ROOT_CLASS_ATTR)
AC_DEFINE_UNQUOTED(HAVE_OBJC_ROOT_CLASS_ATTRIBUTE,$gs_objc_root_class_attr,
  [Says whether the objc_root_class attribute works])
CFLAGS=$saved_CFLAGS


#--------------------------------------------------------------------
# Check if we can name pthreads
#--------------------------------------------------------------------

AC_CACHE_CHECK([for pthread_setname_np()], gs_cv_pthread_setname_np,
[AC_LINK_IFELSE(
  [AC_LANG_PROGRAM([#include <pthread.h>],
    [pthread_setname_np("name");])],
  [gs_cv_pthread_setname_np=darwin],
  [AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([#include <pthread.h>],
      [pthread_setname_np(pthread_self(), "name");])],
    [gs_cv_pthread_setname_np=glibc],
    [AC_LINK_IFELSE(
      [AC_LANG_PROGRAM([#include <pthread.h>],
        [pthread_setname_np(pthread_self(), "%s", "name");])],
      [gs_cv_pthread_setname_np=netbsd],
      [gs_cv_pthread_setname_np=none])])])])
case $gs_cv_pthread_setname_np in
  darwin)
    AC_DEFINE(PTHREAD_SETNAME(a), pthread_setname_np(a),
      [Description: Define set name function for pthread with one arg])
    ;;
  glibc)
    AC_DEFINE(PTHREAD_SETNAME(a), pthread_setname_np(pthread_self(),a),
      [Description: Define setname function for pthread with two args])
    ;;
  netbsd)
    AC_DEFINE(PTHREAD_SETNAME(a), pthread_setname_np(pthread_self(),"%s",a),
      [Description: Define setname function for pthread with three args])
    ;;
esac

#--------------------------------------------------------------------
# Check if we have spinlock support
#--------------------------------------------------------------------
AC_CHECK_FUNCS(pthread_spin_lock)

#--------------------------------------------------------------------
# Check whether we can get the system thread ID
#--------------------------------------------------------------------
AC_CACHE_CHECK([for gettid()], ac_cv_gettid,
[AC_TRY_RUN(#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
int main(int argc, char **argv) {
pid_t t = syscall(SYS_gettid); return t == -1 ? 1 : 0; },
[ac_cv_gettid=yes], [ac_cv_gettid=no], [ac_cv_gettid=no])])
if test "$ac_cv_gettid" = "yes"; then
  AC_DEFINE(HAVE_GETTID, 1, [Define if you have gettid()])
fi

#--------------------------------------------------------------------
# Check whether Objective-C /really/ works
#--------------------------------------------------------------------
AC_MSG_CHECKING(whether objc really works)
saved_LIBS="$LIBS"
saved_CPPFLAGS="$CPPFLAGS"
LIBS="$LIBS $LIBOBJC"
CPPFLAGS="$CPPFLAGS $OBJCFLAGS -x objective-c"
if test x"$objc_threaded" != x""; then
  LIBS="$LIBS $objc_threaded"
fi
LIBS="$LIBS $extra_LIBS"
AC_CACHE_VAL(gs_cv_objc_works,
  AC_TRY_RUN([#include "$srcdir/config/config.objc.m"],
  gs_cv_objc_works=yes,
  gs_cv_objc_works=no,
  gs_cv_objc_works="$cross_gs_cv_objc_works")
)
if test $gs_cv_objc_works = yes; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
  echo "I don't seem to be able to use your Objective-C compiler to produce"
  echo "working binaries!  Please check your Objective-C compiler installation."
  echo "If you are using gcc-3.x make sure that your compiler's libgcc_s and libobjc"
  echo "can be found by the dynamic linker - usually that requires you to play"
  echo "with LD_LIBRARY_PATH or /etc/ld.so.conf."
  echo "Please refer to your compiler installation instructions for more help."
  AC_MSG_ERROR(The Objective-C compiler does not work or is not installed properly.)
fi

# Don't revert any Objective-C flags as they are used in the next test

#---------------------------------------------------------------------
# See if we are using a compiler which allows us to change the class
# to be used for constant strings by using the -fconstant-string-class
# option.  If that is the case, we change it to NSConstantString.
# Unless we are building for the apple runtime (ie only building base
# additions library and not implementing a constant string class).
#---------------------------------------------------------------------
if test "$OBJC_RUNTIME_LIB" = "nx" -o "$OBJC_RUNTIME_LIB" = "apple"; then
  NX_CONST_STRING_OBJCFLAGS=""
  NX_CONST_STRING_CLASS=NXConstantString
else
  strclass_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS -fconstant-string-class=FooConstantString"
  AC_MSG_CHECKING(if the compiler supports -fconstant-string-class)
  AC_CACHE_VAL(gs_cv_objc_compiler_supports_constant_string_class,
    AC_TRY_RUN([#include "$srcdir/config/config.constant-string-class.m"],
    gs_cv_objc_compiler_supports_constant_string_class=yes,
    gs_cv_objc_compiler_supports_constant_string_class=no,
    gs_cv_objc_compiler_supports_constant_string_class="$cross_gs_cv_objc_compiler_supports_constant_string_class")
  )
  if test $gs_cv_objc_compiler_supports_constant_string_class = yes; then
    NX_CONST_STRING_OBJCFLAGS="-fconstant-string-class=NSConstantString"
    NX_CONST_STRING_CLASS=NSConstantString
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    AC_ARG_ENABLE(nxconstantstring,
        [  --enable-nxconstantstring
	    Enables the use of the NXConstantString class for old compilers.],,
        enable_nxconstantstring=no)
    if test $enable_nxconstantstring = yes; then
      NX_CONST_STRING_OBJCFLAGS=""
      NX_CONST_STRING_CLASS=NXConstantString
      AC_MSG_WARN([You have enabled the use of NXConstantString as the string class in gnustep-base.  The objective-c runtime library typically implements its own class of the same name, so you must either enforce a link order which ensures that the gnustep-base implementation is used, or (better) you must remove the class from the runtime library!])
    else
      AC_MSG_ERROR([Your compiler does not appear to implement the -fconstant-string-class option needed for support of strings.  Please check for a more recent version or consider using --enable-nxconstantstring])
    fi
  fi
  CPPFLAGS="$strclass_CPPFLAGS"
fi

AC_SUBST(NX_CONST_STRING_OBJCFLAGS)
AC_SUBST(NX_CONST_STRING_CLASS)

# Don't revert any Objective-C flags as they are used in the next test

#---------------------------------------------------------------------
# Guess if we are using a compiler which has the (GNU extension) +load
# method which is executed before main.
# Defines HAVE_LOAD_METHOD if +load methods are called before main.
# Needed by NSProcessInfo.m
#---------------------------------------------------------------------
AC_MSG_CHECKING(if +load method is executed before main)
AC_CACHE_VAL(gs_cv_objc_load_method_worked,
  AC_TRY_RUN([#include "$srcdir/config/config.loadtest.m"],
	gs_cv_objc_load_method_worked=yes,
	gs_cv_objc_load_method_worked=no,
	gs_cv_objc_load_method_worked="$cross_gs_cv_objc_load_method_worked")
)
if test $gs_cv_objc_load_method_worked = yes; then
  AC_DEFINE(HAVE_LOAD_METHOD,1,
            [Define if your Obj-C compiler calls +load methods before main])
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

# Don't revert any Objective-C flags as they are used in the next test

#--------------------------------------------------------------------
# Check for thread synchronisation support in runtime
#--------------------------------------------------------------------
if test "$host_os" = windows -a "$OBJC_RUNTIME_LIB" = "ng"; then
  # On Windows MSVC, AC_CHECK_FUNCS() will throw linker errors ("relocation
  # against symbol in discarded section") as the file won't contain any actual
  # ObjC code, so we just assume the function is there.
  AC_MSG_NOTICE([assuming objc_sync_enter() on Windows MSVC])
  OBJCSYNC=1
else
  AC_CHECK_FUNCS(objc_sync_enter)
  if test $ac_cv_func_objc_sync_enter = yes ; then
    OBJCSYNC=1
  else
    OBJCSYNC=0
    if test "$OBJC_RUNTIME_LIB" = "ng"; then
      AC_MSG_ERROR([The objc runtime library does not appear to have synchronisation support.  Try re-configuring gnustep-make with a CPPFLAGS variable containing a -L point to specify the directory containing the correct libobjc, or using the --with-objc-lib-flag=... option.])
    fi
  fi
fi
AC_SUBST(OBJCSYNC)

# Don't revert any Objective-C flags as they are used in the next test

#--------------------------------------------------------------------
# Check for ObjC2 support in runtime
#--------------------------------------------------------------------
if test "$cross_compiling" = "yes"; then
  AC_MSG_NOTICE(["Cross compiling! Using predefined OBJC2RUNTIME variable"])
  OBJC2RUNTIME="$cross_objc2_runtime"
elif test "$host_os" = windows -a "$OBJC_RUNTIME_LIB" = "ng"; then
  # On Windows MSVC, AC_CHECK_FUNCS() will throw linker errors ("relocation
  # against symbol in discarded section") as the file won't contain any actual
  # ObjC code, so we just assume the function is there.
  AC_MSG_NOTICE([assuming objc_setProperty() on Windows MSVC])
  OBJC2RUNTIME=1
else
  AC_CHECK_FUNCS(objc_setProperty)
  if test $ac_cv_func_objc_setProperty = yes ; then
    OBJC2RUNTIME=1
  else
    OBJC2RUNTIME=0
    if test "$OBJC_RUNTIME_LIB" = "ng"; then
      AC_MSG_ERROR([The objc runtime library does not appear to have property support.  Try re-configuring gnustep-make with a CPPFLAGS variable containing a -L point to specify the directory containing the correct libobjc, or using the --with-objc-lib-flag=... option.])
    fi
  fi
fi
AC_SUBST(OBJC2RUNTIME)

#--------------------------------------------------------------------
# Check for blocks support in runtime
#--------------------------------------------------------------------
if test "$host_os" = windows -a "$OBJC_RUNTIME_LIB" = "ng"; then
  # On Windows MSVC, AC_CHECK_FUNCS() will throw linker errors ("relocation
  # against symbol in discarded section") as the file won't contain any actual
  # ObjC code, so we just assume the function is there.
  AC_MSG_NOTICE([assuming _Block_copy() on Windows MSVC])
  HAVE_BLOCKS=1
else
  AC_CHECK_FUNCS(_Block_copy)
  if test $ac_cv_func__Block_copy = yes ; then
    HAVE_BLOCKS=1
  else
    HAVE_BLOCKS=0
    if test "$OBJC_RUNTIME_LIB" = "ng"; then
      AC_MSG_ERROR([The objc runtime library does not appear to have blocks support.  Try re-configuring gnustep-make with a CPPFLAGS variable containing a -L point to specify the directory containing the correct libobjc, or using the --with-objc-lib-flag=... option.])
    fi
  fi
fi
AC_SUBST(HAVE_BLOCKS)

# Don't revert any Objective-C flags as they are used in the next test

GS_NONFRAGILE=0
if test "$nonfragile" = "yes"; then
  if test "$host_os" = windows -a "$OBJC_RUNTIME_LIB" = "ng"; then
    # On Windows MSVC, AC_CHECK_FUNCS() will throw linker errors ("relocation
    # against symbol in discarded section") as the file won't contain any actual
    # ObjC code, so we just assume the function is there.
    AC_MSG_NOTICE([assuming non-fragile-abi support on Windows MSVC])
    GS_NONFRAGILE=1
  else
    saved_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $OBJCFLAGS -x objective-c"
    AC_MSG_CHECKING(for non-fragile-abi support)
    AC_RUN_IFELSE([AC_LANG_SOURCE(
      [[#include "$srcdir/config/config.non-fragile-ivars.m"]])],
      non_fragile=yes, non_fragile=no,
      non_fragile="$cross_non_fragile")
    CPPFLAGS="$saved_CPPFLAGS"
    if test $non_fragile = yes; then
      GS_NONFRAGILE=1
    fi
    AC_MSG_RESULT($non_fragile)
  fi
fi
AC_SUBST(GS_NONFRAGILE)


# Don't revert any Objective-C flags as they are used in the next test

#--------------------------------------------------------------------
# get_uninstalled_dtable used by behavior.m and objc-load.m
#--------------------------------------------------------------------
AC_EGREP_HEADER(objc_get_uninstalled_dtable, objc/objc-api.h,
	AC_DEFINE(HAVE_OBJC_GET_UNINSTALLED_DTABLE,1,
                  [ Define if objc-api.h defines this function]),)

# Don't revert any Objective-C flags as they are used in the next test


#--------------------------------------------------------------------
# Native Objective-C exceptions
#--------------------------------------------------------------------

if test "$OBJC_RUNTIME_LIB" = "ng"; then
  exceptions=yes
else
  # Determine if native Objective-C exceptions are enabled
  # in gnustep-make.  They are enabled if the compiler supports it; they
  # are disabled if it doesn't.  And, of course, the user may have
  # forced one behaviour or the other.  Note that we go and look at the
  # actual config.make file to be able to know what was configured in
  # gnustep-make regardless of any gnustep-base that is currently
  # installed.  We can't use `gnustep-config --objc-flags` because that
  # may report native exceptions as disabled because the currently
  # installed gnustep-base has them disabled.
  if grep USE_OBJC_EXCEPTIONS $CURRENT_GNUSTEP_MAKEFILES/$lobj_dir/config.make | grep yes >&5 2>&5; then
    exceptions=yes
  else
    exceptions=no
  fi
fi

# At this point, if exceptions=no, then native exceptions are disabled
# and won't be used.  If exceptions=yes, we need to check if they work
# and if the runtime has proper support for them.  If it does, we'll
# happily use them; if it doesn't, we'll automatically disable them
# because they don't work. ;-)

#--------------------------------------------------------------------
# One of these is needed by for handling uncaught exceptions
#--------------------------------------------------------------------

# TODO: These checks are not really needed if native exceptions are
# disabled.  So maybe we should not run them at all in that case.  On
# the other hand, that case is going to become more and more unusual.

AC_MSG_CHECKING(for objc_setUncaughtExceptionHandler() in runtime)
AC_LINK_IFELSE([AC_LANG_SOURCE(
  [[#include "$srcdir/config/config.setUncaughtExceptionHandler.m"]])],
	have_set_uncaught_exception_handler=yes, have_set_uncaught_exception_handler=no)
if test $have_set_uncaught_exception_handler = yes; then
  AC_DEFINE(HAVE_SET_UNCAUGHT_EXCEPTION_HANDLER,1,
	    [Define if libobjc has the objc_setUncaughtExceptionHandler() function])
fi
AC_MSG_RESULT($have_set_uncaught_exception_handler)

# Don't revert any Objective-C flags as they are used in the next test

AC_MSG_CHECKING(for objc_set_unexpected() in runtime)
AC_LINK_IFELSE(
  [AC_LANG_SOURCE([[#include "$srcdir/config/config.set_unexpected.m"]])],
	have_set_unexpected=yes, have_set_unexpected=no)
if test $have_set_unexpected = yes; then
  AC_DEFINE(HAVE_SET_UNEXPECTED,1,
	    [Define if libobjc has the objc_set_unexpected() function])
fi
AC_MSG_RESULT($have_set_unexpected)

# Don't revert any Objective-C flags as they are used in the next test

AC_MSG_CHECKING(for _objc_unexpected_exception in runtime)
if test "$host_os" = windows -a "$OBJC_RUNTIME_LIB" = "ng"; then
  # On Windows MSVC, AC_CHECK_FUNCS() will throw linker errors ("relocation
  # against symbol in discarded section") as the file won't contain any actual
  # ObjC code, so we just assume the function is there.
  AC_MSG_NOTICE([assuming _objc_unexpected_exception() on Windows MSVC])
  have_unexpected=yes
else
  AC_RUN_IFELSE(
    [AC_LANG_SOURCE([[#include "$srcdir/config/config.unexpected.m"]])],
      have_unexpected=yes,
      have_unexpected=no,
      have_unexpected="$cross_have_unexpected")
fi
if test $have_unexpected = yes; then
  AC_DEFINE(HAVE_UNEXPECTED,1,
	    [Define if libobjc has the _objc_unexpected_exception callback])
fi
AC_MSG_RESULT($have_unexpected)

# TODO: It would also be nice to actually test that native Objective-C
# exceptions work.

if test "$exceptions" = "yes"; then
  if test x"$have_set_uncaught_exception_handler" = x"no"; then
    if test x"$have_set_unexpected" = x"no"; then
      if test x"$have_unexpected" = x"no"; then
        AC_MSG_NOTICE([Disabling native Objective-C exceptions because the ObjC runtime])
        AC_MSG_NOTICE([has no way to set an uncaught exception handler.  Please install])
        AC_MSG_NOTICE([a recent Objective-C runtime if you want to use them.])
        exceptions="no";
      fi
    fi
  fi
fi

AC_MSG_CHECKING(whether to enable native Objective-C exceptions)
if test "$exceptions" = "yes"; then
  AC_MSG_RESULT(yes)
  BASE_NATIVE_OBJC_EXCEPTIONS=1
else
  AC_MSG_RESULT(no)
  BASE_NATIVE_OBJC_EXCEPTIONS=0
fi
AC_SUBST(BASE_NATIVE_OBJC_EXCEPTIONS)

# Don't revert any Objective-C flags as they are used in the next test

#--------------------------------------------------------------------
# Function needed by @synchronize directive
#--------------------------------------------------------------------
HAVE_OBJC_SYNC_ENTER=no
if test "$host_os" = windows -a "$OBJC_RUNTIME_LIB" = "ng"; then
  # On Windows MSVC, AC_CHECK_FUNCS() will throw linker errors ("relocation
  # against symbol in discarded section") as the file won't contain any actual
  # ObjC code, so we just assume the function is there.
  AC_MSG_NOTICE([assuming objc_sync_enter() on Windows MSVC])
  HAVE_OBJC_SYNC_ENTER=yes
else
  AC_CHECK_FUNCS(objc_sync_enter)
  if test $ac_cv_func_objc_sync_enter = yes ; then
    HAVE_OBJC_SYNC_ENTER=yes
  fi
fi
AC_SUBST(HAVE_OBJC_SYNC_ENTER)

# Don't revert any Objective-C flags as they are used in the next test

AC_MSG_CHECKING(for thread-safe +initialize in runtime)
AC_RUN_IFELSE(
  [AC_LANG_SOURCE([[#include "$srcdir/config/config.initialize.m"]])],
	safe_initialize=yes, safe_initialize=no, safe_initialize="$cross_safe_initialize")
if test $safe_initialize = yes; then
  AC_DEFINE(HAVE_INITIALIZE,1,
	    [Define if libobjc has thread-safe +initialize support])
else
  AC_MSG_WARN([Your ObjectiveC runtime does not support thread-safe class initialisation.  Please use a different runtime if you intend to use threads.])
fi
AC_MSG_RESULT($safe_initialize)

LIBS="$saved_LIBS"
CPPFLAGS="$saved_CPPFLAGS"

AC_SUBST(OBJCFLAGS)

#--------------------------------------------------------------------
# Generic settings needed by NSZone.m
#--------------------------------------------------------------------
AC_TYPE_SIZE_T
AC_C_INLINE

#--------------------------------------------------------------------
# Following header checks needed for bzero in Storage.m and other places
#--------------------------------------------------------------------
AC_HEADER_STDC
AC_CHECK_HEADERS(string.h memory.h alloca.h)

#--------------------------------------------------------------------
# Following header check needed NSConnection.h
#--------------------------------------------------------------------
AC_CHECK_HEADERS(float.h)

#--------------------------------------------------------------------
# Header files and functions for files and filesystems
#--------------------------------------------------------------------
AC_CHECK_HEADERS(sys/stat.h sys/vfs.h sys/statfs.h sys/statvfs.h pwd.h grp.h)
AC_CHECK_HEADERS(sys/cdefs.h sys/syslimits.h sys/param.h)
# Twisted header checks for some BSDs with stupid interdependencies
AC_CHECK_HEADERS([sys/syslimits.h], [], [],
  [#if HAVE_SYS_CDEFS_H
   #include <sys/cdefs.h>
   #endif
   #include <sys/syslimits.h>
  ])
AC_CHECK_HEADERS([sys/param.h], [], [],
  [#if HAVE_SYS_CDEFS_H
   #include <sys/cdefs.h>
   #endif
   #if HAVE_SYS_SYSLIMITS_H
   #include <sys/syslimits.h>
   #endif
  ])
AC_CHECK_HEADERS([sys/mount.h], [], [],
  [#if HAVE_SYS_CDEFS_H
   #include <sys/cdefs.h>
   #endif
   #if HAVE_SYS_SYSLIMITS_H
   #include <sys/syslimits.h>
   #endif
   #if HAVE_SYS_PARAM_H
   #include <sys/param.h>
   #endif
  ])
AC_CHECK_HEADERS(sys/types.h windows.h locale.h langinfo.h)
saved_LIBS="$LIBS"
AC_CHECK_LIB(m, main)
AC_CHECK_FUNCS(utimensat statvfs link symlink readlink geteuid getlogin getpwnam getpwnam_r getpwuid getpwuid_r getgrgid getgrgid_r getgrnam getgrnam_r rint getopt malloc_usable_size)
LIBS="$saved_LIBS"

AC_CACHE_CHECK([for pw_gecos field in struct passwd],
		ac_cv_have_pw_gecos_in_struct_passwd, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[ #include <pwd.h> ]],
	[[ struct passwd p; p.pw_gecos = 0; ]])],
	[ ac_cv_have_pw_gecos_in_struct_passwd="yes" ],
	[ ac_cv_have_pw_gecos_in_struct_passwd="no"
	])
])
if test "x$ac_cv_have_pw_gecos_in_struct_passwd" = "xyes" ; then
	AC_DEFINE([HAVE_PW_GECOS_IN_PASSWD], [1],
		[Define if you have pw_gecos field in struct passwd])
fi

AC_CACHE_CHECK([for currency_symbol field in struct lconv],
		ac_cv_have_currency_symbol_in_struct_lconv, [
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[ #include <locale.h> ]],
	[[ struct lconv l; l.currency_symbol = NULL; ]])],
	[ ac_cv_have_currency_symbol_in_struct_lconv="yes" ],
	[ ac_cv_have_currency_symbol_in_struct_lconv="no"
	])
])
if test "x$ac_cv_have_currency_symbol_in_struct_lconv" = "xyes" ; then
	AC_DEFINE([HAVE_CURRENCY_SYMBOL_IN_LCONV], [1],
		[Define if you have currency_symbol field in struct lconv])
fi

#--------------------------------------------------------------------
# These two headers (functions) needed by Time.m
#--------------------------------------------------------------------
dnl AC_REPLACE_FUNCS(getrusage gettimeofday)

AC_CHECK_HEADERS(time.h sys/time.h tzfile.h sys/rusage.h ucbinclude/sys/resource.h)
AC_CHECK_FUNCS(time ctime tzset)

# Check if tzfile contains the proper definitions
if test $ac_cv_header_tzfile_h = yes; then
  AC_EGREP_HEADER(tzhead, tzfile.h,
	AC_DEFINE(HAVE_TZHEAD,1,
                  [ Define if tzfile.h includes the proper definitions]),)
fi

#--------------------------------------------------------------------
# These headers/functions needed by GSTcpPort.m
#--------------------------------------------------------------------
AC_CHECK_HEADERS(sys/socket.h netinet/in.h)
AC_CHECK_TYPES([socklen_t])
dnl AC_REPLACE_FUNCS(recvfrom)

#--------------------------------------------------------------------
# These headers/functions needed for stacktrace in NSException.m
#--------------------------------------------------------------------
AC_ARG_ENABLE(bfd,
  [  --enable-bfd
	Enables the use of libbfd to provide symbolic stack traces.
	Enabling this option provides support for symbolic stack traces
	on platforms where the backtrace_symbols() function is not
	available or does not work properly.
	Enabling this option also has the effect of changing the license
	of gnustep-base from LGPL to GPL since libbfd uses the GPL license],,
  enable_bfd=no)
if test $enable_bfd = yes ; then
  AC_DEFINE(USE_BFD,1, [Define to use bfd library for stack traces])
  AC_MSG_WARN([You enabled bfd, which causes gnustep-base to link with libbfd. This makes the license GPL rather than the normal LGPL.])
fi

AC_CHECK_HEADERS(bfd.h)
AC_CHECK_LIB(intl, libintl_fprintf)
AC_CHECK_LIB(iberty, dyn_string_append)
AC_CHECK_LIB(bfd, bfd_openr)
if test $ismingw = yes ; then
  AC_CHECK_HEADERS(dbghelp.h)
else
  AC_CHECK_HEADERS(execinfo.h)
  AC_CHECK_FUNCS(backtrace)
fi

AC_CHECK_FUNCS(__builtin_extract_return_address)

# Enable unwind if found by default
AC_ARG_WITH(unwind,
  AS_HELP_STRING([--without-unwind], [Ignore unwind if found and disable it]))

AS_IF([test "x$with_unwind" != "xno"],
  [AC_CHECK_HEADERS(unwind.h)] [AC_CHECK_FUNCS(_Unwind_GetIP)],
  [have_unwind=no])

AS_IF([test "x$ac_cv_func__Unwind_GetIP" = "xyes"],
  [have_unwind=yes],
  [have_unwind=no])

AS_IF([test "x$have_unwind" = "xyes"],
  [AC_DEFINE([WITH_UNWIND], [1], [Have and use Unwind library])],
  [AS_IF([test "x$with_unwind" = "xyes"],
         [AC_MSG_ERROR([unwind requested but not found])
  ])
])

#--------------------------------------------------------------------
# These headers/functions needed by NSLog.m
#--------------------------------------------------------------------
AC_CHECK_HEADERS(syslog.h sys/slog.h sys/slogcodes.h)
AC_CHECK_FUNCS(syslog)
if test "$ac_cv_header_sys_slog_h" = "yes"; then
  oldLibs="$LIBS";
  LIBS="$LIBS -l:libc.a";
  AC_CHECK_FUNCS(slogf)
  if test "$ac_cv_func_slogf" = "no"; then
    LIBS="$oldLibs"
  fi
fi
#--------------------------------------------------------------------
# These headers/functions needed by NSRunLoop.m
#--------------------------------------------------------------------
AC_CHECK_HEADERS(poll.h)
AC_CHECK_FUNCS(poll)
have_poll=no
if test $ac_cv_header_poll_h = yes; then
  have_poll=yes
  AC_MSG_CHECKING(for poll emulation)
  AC_EGREP_CPP(emulating_poll, [#include "config/config.poll.c"],
  	have_poll=no,)
  if test $have_poll = yes; then
    AC_MSG_RESULT(no)
    AC_MSG_CHECKING([if poll supports devices])
    AC_TRY_RUN([#include "$srcdir/config/config.poll-dev.c"i], ,have_poll=no, have_poll="$cross_have_poll" )
    if test $have_poll = yes; then
      AC_MSG_RESULT(yes)
      AC_DEFINE(HAVE_POLL_F,1, [ Define if poll is NOT emulated via select])
    else
      AC_MSG_RESULT(no)
    fi
  else
    AC_MSG_RESULT(yes)
  fi
fi

#--------------------------------------------------------------------
# This function needed by StdioStream.m
#--------------------------------------------------------------------
AC_CHECK_FUNCS(vsprintf vasprintf snprintf vsnprintf)
if test $ac_cv_func_vsprintf = yes ; then
    AC_TRY_RUN([#include "$srcdir/config/config.vsprintf.c"],
		VSPRINTF_RETURNS_LENGTH=1,
		VSPRINTF_RETURNS_LENGTH=0,
		VSPRINTF_RETURNS_LENGTH="$cross_VSPRINTF_RETURNS_LENGTH")
    AC_DEFINE_UNQUOTED(VSPRINTF_RETURNS_LENGTH, $VSPRINTF_RETURNS_LENGTH,
                       [Define if vsprintf returns the length printed])
fi
if test $ac_cv_func_vasprintf = yes ; then
    AC_TRY_RUN([#include "$srcdir/config/config.vasprintf.c"],
		VASPRINTF_RETURNS_LENGTH=1,
		VASPRINTF_RETURNS_LENGTH=0,
		VASPRINTF_RETURNS_LENGTH="$cross_VASPRINTF_RETURNS_LENGTH")
    AC_DEFINE_UNQUOTED(VASPRINTF_RETURNS_LENGTH, $VASPRINTF_RETURNS_LENGTH,
                       [Define if vasprintf returns the length printed])
fi

#--------------------------------------------------------------------
# This function needed by NSFileManager.m
#--------------------------------------------------------------------
AC_CHECK_FUNCS(getcwd)
AC_HEADER_DIRENT

# Look for file creation and modification date on NetBSD, FreeBSD, Darwin
AC_CHECK_MEMBERS([struct stat.st_mtim, struct stat.st_birthtime, struct stat.st_birthtimespec, struct stat.st_birthtim, struct stat64.st_birthtimespec],
		 ,,[#include <sys/stat.h>])

#--------------------------------------------------------------------
# This function needed by gdomap.c
#--------------------------------------------------------------------
AC_CHECK_HEADERS(getopt.h)

#--------------------------------------------------------------------
# This function needed by NSPage.m
#--------------------------------------------------------------------
AC_CHECK_FUNCS(posix_memalign valloc)

#--------------------------------------------------------------------
# This function needed by Time.m
#--------------------------------------------------------------------
AC_CHECK_FUNCS(times)

#--------------------------------------------------------------------
# These functions needed by NSData.m and GSFFIInvocation.m
#--------------------------------------------------------------------
AC_CHECK_FUNCS(mkstemp)
AC_CHECK_FUNCS(shmctl)
AC_CHECK_FUNCS(mmap)
AC_CHECK_FUNCS(mprotect)
AC_CHECK_HEADERS(sys/mman.h)

#--------------------------------------------------------------------
# These functions needed by NSTask.m
#--------------------------------------------------------------------
AC_CHECK_FUNCS(killpg setpgrp setpgid setsid grantpt)
if test "x$ac_cv_func_setpgrp" = xyes; then
  AC_FUNC_SETPGRP
fi

HAVE_PTS_STREAM_MODULES=0
case "${target}" in
  *-sysv-*)
    HAVE_PTS_STREAM_MODULES=1
    ;;
esac
AC_DEFINE_UNQUOTED(HAVE_PTS_STREAM_MODULES, $HAVE_PTS_STREAM_MODULES,
                   [Define this if you work on sysv])
AC_SUBST(HAVE_PTS_STREAM_MODULES)

AC_CHECK_HEADERS(dnl
	fcntl.h dnl
	inttypes.h dnl
	libc.h dnl
	limits.h dnl
	malloc.h dnl
	memory.h dnl
	signal.h dnl
	stdint.h dnl
	string.h dnl
	sys/fcntl.h dnl
	sys/file.h dnl
	sys/filio.h dnl
	sys/inttypes.h dnl
	sys/ioctl.h dnl
	sys/signal.h dnl
	sys/stropts.h dnl
	sys/wait.h dnl
	unistd.h dnl
	utime.h dnl
        stdlib.h dnl
	)

if test $ac_cv_header_inttypes_h = yes; then
INCLUDE_INTTYPES="#include <inttypes.h>"
elif test $ac_cv_header_sys_inttypes_h = yes; then
INCLUDE_INTTYPES="#include <sys/inttypes.h>"
elif test $ac_cv_header_stdint_h = yes; then
INCLUDE_INTTYPES="#include <stdint.h>"
else
INCLUDE_INTTYPES="no"
fi

if test "$INCLUDE_INTTYPES" = "no"; then
INCLUDE_INTTYPES=""
DEFINE_INT8_T="#define int8_t gss8"
DEFINE_UINT8_T="#define uint8_t gsu8"
DEFINE_INT16_T="#define int16_t gss16"
DEFINE_UINT16_T="#define uint16_t gsu16"
DEFINE_INT32_T="#define int32_t gss32"
DEFINE_UINT32_T="#define uint32_t gsu32"
DEFINE_INT64_T="#define int64_t gss64"
DEFINE_UINT64_T="#define uint64_t gsu64"
DEFINE_INTPTR_T="#define intptr_t gssaddr"
DEFINE_UINTPTR_T="#define uintptr_t gsuaddr"
else
DEFINE_INT8_T=""
DEFINE_UINT8_T=""
DEFINE_INT16_T=""
DEFINE_UINT16_T=""
DEFINE_INT32_T=""
DEFINE_UINT32_T=""
DEFINE_INT64_T=""
DEFINE_UINT64_T=""
DEFINE_INTPTR_T=""
DEFINE_UINTPTR_T=""
fi

AC_SUBST(INCLUDE_INTTYPES)
AC_SUBST(DEFINE_INT8_T)
AC_SUBST(DEFINE_UINT8_T)
AC_SUBST(DEFINE_INT16_T)
AC_SUBST(DEFINE_UINT16_T)
AC_SUBST(DEFINE_INT32_T)
AC_SUBST(DEFINE_UINT32_T)
AC_SUBST(DEFINE_INT64_T)
AC_SUBST(DEFINE_UINT64_T)
AC_SUBST(DEFINE_INTPTR_T)
AC_SUBST(DEFINE_UINTPTR_T)

#--------------------------------------------------------------------
# These used by GSFileHandle.m and distributed objects
# On some systems we need -lnsl ... so check for that case.
#--------------------------------------------------------------------
AC_CHECK_HEADERS(ws2tcpip.h)
AC_SEARCH_LIBS([inet_ntop],[nsl Ws2_32])
AC_CHECK_FUNCS(gethostbyaddr_r inet_aton inet_pton inet_ntop sigaction)

USE_ZLIB=0
AC_CHECK_HEADERS(zlib.h)
if test $ac_cv_header_zlib_h = yes; then
  AC_CHECK_LIB(z, gzseek, zlib_ok=yes, zlib_ok=no)
  if test "$zlib_ok" = yes; then
    LIBS="$LIBS -lz"
    USE_ZLIB=1
  fi
fi
AC_SUBST(USE_ZLIB)

HAVE_INET_PTON=no
if test $ac_cv_func_inet_pton = yes ; then
  HAVE_INET_PTON=yes
fi
AC_SUBST(HAVE_INET_PTON)

HAVE_INET_NTOP=no
if test $ac_cv_func_inet_ntop = yes ; then
  HAVE_INET_NTOP=yes
fi
AC_SUBST(HAVE_INET_NTOP)

#--------------------------------------------------------------------
# For setting thread stack size
#--------------------------------------------------------------------
AC_CHECK_HEADERS(sys/resource.h)
AC_CHECK_FUNCS(setrlimit)

#--------------------------------------------------------------------
# One of these functions needed by NSDebug.m and NSProcessInfo.m
#--------------------------------------------------------------------
AC_CHECK_FUNCS(strerror)
AC_FUNC_STRERROR_R

#--------------------------------------------------------------------
# Needed by NSDebug.m
#--------------------------------------------------------------------
AC_CHECK_FUNCS(sigsetjmp)

#--------------------------------------------------------------------
# This type needed by GSFormat
#--------------------------------------------------------------------
AC_CHECK_TYPES([uintmax_t])
AC_CHECK_HEADERS(wchar.h)

#--------------------------------------------------------------------
# Check if short and int values need to be word aligned
#--------------------------------------------------------------------
saved_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -O0"
AC_MSG_CHECKING(short/int needs to be word aligned)
AC_TRY_RUN([#include "$srcdir/config/config.align.c"],
	NEED_WORD_ALIGNMENT=0,
	NEED_WORD_ALIGNMENT=1,
	NEED_WORD_ALIGNMENT="$cross_NEED_WORD_ALIGNMENT")
CFLAGS="$saved_CFLAGS"
AC_DEFINE_UNQUOTED(NEED_WORD_ALIGNMENT, $NEED_WORD_ALIGNMENT,
                   [Define if your system needs to have short/int word aligned])
if test $NEED_WORD_ALIGNMENT = 1; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

#--------------------------------------------------------------------
# This needed by NSString for handling of %@ printf directive.
#--------------------------------------------------------------------
AC_CHECK_FUNCS(register_printf_specifier)
AC_CHECK_FUNC(register_printf_function, register_printf=1,
register_printf=0)
if test $register_printf = 1; then
  AC_TRY_RUN([#include "$srcdir/config/config.printf.c"],
	       working_register_printf=1, working_register_printf=0,
               working_register_printf="$cross_working_register_printf")
  if test $working_register_printf = 1; then
    AC_DEFINE(HAVE_REGISTER_PRINTF_FUNCTION,1,
              [Define if you have the register_printf_function function])
    AC_TRY_RUN([#include "$srcdir/config/config.wprintf.c"],
	       wide_register_printf=1, wide_register_printf=0,
               wide_register_printf="$cross_wide_register_printf")
    if test $wide_register_printf = 1; then
      AC_DEFINE(HAVE_WIDE_PRINTF_FUNCTION,1,
              [Define if register_printf_function supports wide characters])
    fi
  fi
fi


#--------------------------------------------------------------------
# This function needed by NSString.
#--------------------------------------------------------------------
AC_CHECK_FUNCS(realpath)

#--------------------------------------------------------------------
# Check if the C Library defines extern char *program_invocation_name
# Used in critical cases by NSProcessInfo.m
#--------------------------------------------------------------------
AC_MSG_CHECKING(program_invocation_name in C Library)
AC_CACHE_VAL(gs_cv_program_invocation_name_worked,
[AC_TRY_RUN([
#include <string.h>
int
main (int argc, char *argv[])
{
  extern char *program_invocation_name;
  return (strcmp (program_invocation_name, argv[0]));
}
], gs_cv_program_invocation_name_worked=yes, gs_cv_program_invocation_name_worked=no,
 gs_cv_program_invocation_name_worked="$cross_gs_cv_program_invocation_name_worked")])
if test $gs_cv_program_invocation_name_worked = yes; then
  AC_DEFINE(HAVE_PROGRAM_INVOCATION_NAME,1,
            [Define if your Lib C defines program_invocation_name])
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

#--------------------------------------------------------------------
# Check for uname header used by NSProcessInfo.m
#--------------------------------------------------------------------
AC_CHECK_HEADERS(sys/utsname.h)

#--------------------------------------------------------------------
# Check for sysctlbyname used by NSProcessInfo.m
#--------------------------------------------------------------------
AC_CHECK_HEADERS(sys/sysctl.h)
AC_CHECK_FUNCS(sysctlbyname)

#--------------------------------------------------------------------
# Defines HAVE_PROCFS if the kernel supports the /proc filesystem.
# Needed by NSProcessInfo.m
#--------------------------------------------------------------------
AC_CHECK_HEADERS(procfs.h)
AC_SYS_PROCFS
AC_SYS_PROCFS_PSINFO
AC_SYS_PROCFS_EXE_LINK

#--------------------------------------------------------------------
# Check if /proc/$$/cmdline terminates the last argument with a nul
#--------------------------------------------------------------------
AC_MSG_CHECKING(/proc/$$/cmdline terminated by nul)
AC_TRY_RUN([#include "$srcdir/config/config.proccmd.c"],
	CMDLINE_TERMINATED=1,
	CMDLINE_TERMINATED=0,
	CMDLINE_TERMINATED="$cross_CMDLINE_TERMINATED")
AC_DEFINE_UNQUOTED(CMDLINE_TERMINATED, $CMDLINE_TERMINATED,
		   [Define if your system terminates the final argument in /proc/$$/cmdline])
if test $CMDLINE_TERMINATED = 1; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

#--------------------------------------------------------------------
# If /proc doesn't work, try kvm (on FreeBSD, for instance)
#--------------------------------------------------------------------
have_kvm_env=0
save_LIBS="$LIBS"
AC_CHECK_LIB(kvm, kvm_getenvv)
if test "$ac_cv_lib_kvm_kvm_getenvv" = yes; then
  AC_MSG_CHECKING(if we can access kernel memory)
  AC_TRY_RUN([#include "$srcdir/config/config.kvmopen.c"],
	have_kvm_env=1,
	have_kvm_env=0,
	have_kvm_env="$cross_have_kvm_env")
  if test $have_kvm_env = 1; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(HAVE_KVM_ENV, 1,
    			    [Define if you can access the kernel via kvm_open])
  else
    AC_MSG_RESULT(no)
    # Reset LIBS since we don't need kvm
    LIBS="$save_LIBS"
  fi
fi

#--------------------------------------------------------------------
# Include redefinition of main () only if needed.
# On some systems - force redefinition to be used as the /proc stuff
# doesn't work. Allow NSProcessInfo initialization method also.
#--------------------------------------------------------------------
PASS_ARG=no
case "$target_os" in
  cygwin*)	PASS_ARG=yes;;
esac

AC_MSG_CHECKING(use of pass-through arguments)
AC_ARG_ENABLE(pass-arguments,
  [  --enable-pass-arguments	Force user main call to NSProcessInfo initialize],,
  enable_pass_arguments=$PASS_ARG)

if test "$enable_pass_arguments" = "yes"; then
  GS_PASS_ARGUMENTS=1
else
  GS_PASS_ARGUMENTS=0
fi
AC_SUBST(GS_PASS_ARGUMENTS)
AC_MSG_RESULT($enable_pass_arguments)

AC_MSG_CHECKING(use of fake-main definition)
AC_ARG_ENABLE(fake-main,
  [  --enable-fake-main  		Force redefine of user main function],,
  enable_fake_main=no)

if test "$enable_pass_arguments" = "no"; then
  case "$target_os" in
    freebsd2*)	enable_fake_main=yes;;
    freebsd*)	;;
    kfreebsd*)	enable_fake_main=yes;;
    netbsd*)	enable_fake_main=yes;;
    openbsd*)	enable_fake_main=yes;;
    *sysv*)	enable_fake_main=yes;;
  esac
fi

GS_FAKE_MAIN=0
if test "$enable_fake_main" = "yes"; then
  GS_FAKE_MAIN=1
elif test "$enable_pass_arguments" = "no"; then
  if test "$gs_cv_objc_load_method_worked" = yes -a \( "$ac_cv_sys_procfs" = yes -o "$have_kvm_env" = 1 -o "$ac_cv_sys_procfs_psinfo" = yes \); then
    GS_FAKE_MAIN=0
    if test "$have_kvm_env" = "1"; then
      AC_MSG_WARN([Using libkvm which is known to be buggy on some systems consider configuring with --enable-fake-main instead.])
    fi
  else
    case "$target_os" in
      darwin* ) ;;
      * )
       GS_FAKE_MAIN=1
       enable_fake_main=yes
    esac
  fi
fi
case "$target_os" in
    mingw*|windows*)	enable_fake_main=no; GS_FAKE_MAIN=0;;
esac
AC_SUBST(GS_FAKE_MAIN)
AC_MSG_RESULT($enable_fake_main)

#--------------------------------------------------------------------
# Simple way to add a bunch of paths to the flags
#--------------------------------------------------------------------
AC_ARG_WITH(include-flags,
    [  --with-include-flags=FLAGS    Specify all include flags at once],
    include_flags="$withval", include_flags="no")
if test ${include_flags} != "no"; then
    CPPFLAGS="$CPPFLAGS ${include_flags}"
    INCLUDE_FLAGS="$INCLUDE_FLAGS ${include_flags}"
fi

AC_ARG_WITH(library-flags,
    [  --with-library-flags=FLAGS    Specify all library flags at once],
    library_flags="$withval", library_flags="no")
if test ${library_flags} != "no"; then
    LDFLAGS="$LDFLAGS ${library_flags}"
    LDIR_FLAGS="$LDIR_FLAGS ${library_flags}"
fi

#--------------------------------------------------------------------
# Check for FFI interface libraries for invocations
# We enable ffi by default now, as it's the only well supported
# library nowadays.
#--------------------------------------------------------------------
do_broken_libffi=no
do_broken_libffcall=no
do_enable_libffi=yes
do_enable_libffcall=no
case "$target_cpu" in
  sparc64*)
    case "$target_os" in
      solaris*)
        do_broken_libffcall=yes;
        do_enable_libffi=yes;
        do_enable_libffcall=no;;
      *) ;;
    esac ;;
  *) ;;
esac

if test "$exceptions" = "yes"; then
  # ffcall will mess up native exceptions, so we must disable it.
  do_broken_libffcall=yes
  do_enable_libffcall=no
fi

AC_ARG_ENABLE(libffi,
  [  --disable-libffi  		Disable use of libffi library],,
  enable_libffi=$do_enable_libffi)

AC_ARG_ENABLE(ffcall,
  [  --enable-ffcall  		Enable use of the deprecated ffcall library],,
  enable_ffcall=$do_enable_libffcall)

if test $enable_ffcall = yes; then
  AC_MSG_WARN([ffcall has been enabled ... this is deprecated ... please install and use a recent libffi if possible])
fi

AC_ARG_ENABLE(invocations,
  [  --disable-invocations		Build even if invocation-dependencies are not met],,
  enable_invocations=yes)

# DO isn't used on apple-apple-apple
if test $LIBRARY_COMBO = apple-apple-apple; then
  enable_invocations=no
fi

AC_ARG_WITH(ffi-include,
[  --with-ffi-include=PATH       Include path for ffi headers],
    ffi_incdir="$withval", ffi_incdir="no")
if test ${ffi_incdir} != "no"; then
    CPPFLAGS="$CPPFLAGS -I${ffi_incdir}"
    INCLUDE_FLAGS="$INCLUDE_FLAGS -I${ffi_incdir}"
fi

AC_ARG_WITH(ffi-library,
[  --with-ffi-library=PATH       Library path for ffi libs],
    ffi_libdir="$withval", ffi_libdir="no")
if test ${ffi_libdir} != "no"; then
  GS_ADD_LIBRARY_PATH([${ffi_libdir}])
fi

if test "$do_broken_libffi" = "no"; then
  if test -n "$PKG_CONFIG"; then
    if $PKG_CONFIG --exists libffi; then
      pkg_config_libffi=yes
      ffi_CFLAGS=`$PKG_CONFIG --cflags libffi`;
      CPPFLAGS="$CPPFLAGS $ffi_CFLAGS"
      INCLUDE_FLAGS="$INCLUDE_FLAGS $ffi_CFLAGS"
    fi
  fi
  AC_CHECK_HEADER(ffi.h, have_libffi=yes, have_libffi=no)
else
  have_libffi=no
fi
if test "$do_broken_libffcall" = "no"; then
  AC_CHECK_HEADERS(callback.h, have_ffcall=yes, have_ffcall=no)
else
  have_ffcall=no
fi
if test $have_ffcall = no; then
  enable_ffcall=no
  # If we don't have ffcall but do have libffi, use libffi
  if test $have_libffi = yes; then
    enable_libffi=yes
  fi
fi
if test $have_libffi = no; then
  enable_libffi=no
  # If we don't have libffi but do have ffcall, use ffcall
  if test $have_ffcall = yes; then
    if test $enable_ffcall = no; then
      AC_MSG_WARN([ffi support seems to be missing on this system ... please install a recent libffi])
    fi
  fi
fi

have_forward_hook=yes
saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $OBJCFLAGS -x objective-c"
AC_MSG_CHECKING(for forwarding callback in runtime)
AC_COMPILE_IFELSE(
  [AC_LANG_SOURCE([[#include "$srcdir/config/config.forward2.m"]])],
	have_forward_hook=yes, have_forward_hook=no)
if test $have_forward_hook = yes; then
  AC_DEFINE(HAVE_FORWARD2,1,
            [Define if libobjc has the __objc_msg_forward2 function])
else
  AC_COMPILE_IFELSE(
    [AC_LANG_SOURCE([[#include "$srcdir/config/config.forward.m"]])],
	have_forward_hook=yes, have_forward_hook=no)
fi
AC_MSG_RESULT($have_forward_hook)
if test $have_forward_hook = no; then
  enable_libffi=no
  enable_ffcall=no
fi
CPPFLAGS="$saved_CPPFLAGS"

AC_MSG_CHECKING(FFI library usage)
WITH_FFI=none
if test $enable_libffi = yes; then
  AC_DEFINE(USE_LIBFFI,1,
            [Define if using the libffi library for invocations])
  WITH_FFI=libffi
  if test "$pkg_config_libffi" = "yes"; then
    ffi_LIBS=`pkg-config --libs libffi`
  else
    ffi_LIBS=-lffi
  fi
  LIBS="$ffi_LIBS $LIBS"
  AC_TRY_RUN([#include "$srcdir/config/config.ffi.c"],
             ffi_ok="yes",ffi_ok="no",ffi_ok="$cross_ffi_ok")
  if test $ffi_ok = yes; then
    AC_MSG_RESULT(libffi)
    if test $do_broken_libffi = yes; then
      AC_MSG_WARN([ffi may be broken on this system ... try enabling ffcall])
    fi
    AC_CHECK_FUNCS(ffi_prep_closure_loc)
  else
    AC_MSG_ERROR([The ffi library (libffi) does not appear to be working.  Perhaps it's missing or you need a more recent version.  Version 3.0.9 or later should work, and you can find a link to it n the list of packages for download at http://www.gnustep.org/resources/sources.html])
  fi
elif test $enable_ffcall = yes; then
  AC_DEFINE(USE_FFCALL,1,
            [Define if using the ffcall library for invocations])
  WITH_FFI=ffcall
  LIBS="-lcallback -lavcall $LIBS"
  AC_TRY_LINK([#include <callback.h>], , ffi_ok="yes", ffi_ok="no")
  if test $ffi_ok = yes; then
    AC_MSG_RESULT(ffcall)
  fi
  AC_MSG_WARN([ffcall is broken on some systems and is deprecated ... try enabling ffi])
else
  ffi_ok=no
fi

if test $enable_ffcall = yes -a $ffi_ok = yes; then
  AC_MSG_CHECKING(if ffcall trampolines work)
  AC_RUN_IFELSE(
    [AC_LANG_SOURCE([[#include "$srcdir/config/config.trampoline.c"]])],
	  have_working_trampoline=yes, have_working_trampoline=no,
	  have_working_trampoline=yes)
  AC_MSG_RESULT($have_working_trampoline)
fi

if test $ffi_ok = no; then
  AC_MSG_RESULT(none)
  echo
  if test $have_forward_hook = no; then
    echo "You do not have an up-to-date libobjc library installed"
  elif test "$have_working_trampoline" = no; then
    echo "You have ffcall, but it does not work properly. Most likely because"
    echo "your system's security policy is blocking some parts of ffcall"
    echo "we recommend installing libffi instead."
  else
    echo "You do not have either ffcall or libffi installed/enabled, or configure needs"
    echo "--with-ffi-include and/or --with-ffi-library flags so GNUstep can find them,"
    echo "or you have ffcall but gnustep-make is configured to use native exceptions"
    echo "(native exceptions are not compatible with ffcall)."
  fi
  echo "GNUstep requires libffi (or ffcall) and proper libobjc hooks to do"
  echo "invocations and DO."
  echo "(This does not apply on apple-apple-apple systems where DO is"
  echo "not compatible with other GNUstep systems.)"
  if test $enable_invocations = yes; then
    echo
    echo "You most likely do not want to build base without invocation support."
    echo "Many things (including Distributed Objects and undo/redo), won't work"
    echo "at all without invocations."
    echo "If you really want to build -base without invocation support,"
    echo "add --disable-invocations to the configure arguments."
    echo "For more information, read the GNUstep build guide, ffcall section:"
    echo "http://gnustep.made-it.com/BuildGuide/index.html"
    AC_MSG_ERROR([Incomplete support for ffi functionality.])
  fi
  AC_MSG_WARN([Incomplete support for ffi funtionality.])
fi
AC_SUBST(WITH_FFI)


#--------------------------------------------------------------------
# Check for iconv support (for Unicode conversion).
#--------------------------------------------------------------------
# Do this before checking for xml2,  as xml2 may require iconv.
#
# We need to find an iconv library that matches the installed iconv.h header
# (if any). It is important to check header/library compatibility. It's
# fairly common to have iconv support both in libc and from libiconv. In that
# case, a naive check that iconv() is in libc will succeed, but if we use
# libiconv's iconv.h, it will redefine iconv() to functions that exist
# only in libiconv, and we'll get link errors.
#
# Some versions of iconv don't support the '//TRANSLIT' option, which is
# needed for a lossy conversion (where we pick the closest equivalent for
# any character present in the source string which does not exist in the
# destination characterset), so we check for support of that first.

# First, check if there's a working iconv in libc (ie. if the test program
# runs without any extra flags).
AC_ARG_ENABLE(iconv,
  [  --disable-iconv		Build even if iconv is not available],,
  enable_iconv=yes)

if test $enable_iconv = yes; then
AC_MSG_CHECKING(iconv support)
AC_RUN_IFELSE([AC_LANG_SOURCE([#include <iconv.h>
int main(int argc,char **argv)
{ return iconv_open("UTF-8//TRANSLIT","ASCII") == -1 ? 1 : 0; }])]
,
  # libc has a working iconv.
  AC_DEFINE(HAVE_ICONV,1, [Define if you have this function])
  AC_MSG_RESULT([[yes, in libc]])
  found_iconv=yes
,
  found_iconv=no
,
  found_iconv="$cross_found_iconv_libc";
  if test "$found_iconv" = "yes"; then
    AC_DEFINE(HAVE_ICONV,1, [Define if you have this function])
    AC_MSG_RESULT([[yes, in libc (via cross.config)]])
  fi
)

if test $found_iconv = no ; then
  # libc doesn't have a working iconv with translit.
  # Try adding -liconv and any user supplied directory.
  AC_ARG_WITH(libiconv-library,
    [  --with-libiconv-library=PATH  Library path for libiconv libraries],
    libiconv_libdir="$withval", libiconv_libdir="no")

  if test "$libiconv_libdir" != "no"; then
    GS_ADD_LIBRARY_PATH([${libiconv_libdir}])
  fi

  old_LIBS="$LIBS"
  LIBS="-liconv $LIBS"
  AC_RUN_IFELSE([AC_LANG_SOURCE([#include <iconv.h>
  int main(int argc,char **argv)
  { return iconv_open("UTF-8//TRANSLIT","ASCII") == -1 ? 1 : 0; }])]
  ,
    # -liconv works.
    AC_DEFINE(HAVE_ICONV,1, [Define if you have this function])
    AC_MSG_RESULT([[yes, -liconv]])
    found_iconv=yes
  ,
    found_iconv=no
    LIBS="$old_LIBS"
  ,
    found_iconv="$cross_found_iconv_liconv";
    if test "$found_iconv" = "yes"; then
      AC_DEFINE(HAVE_ICONV,1, [Define if you have this function])
      AC_MSG_RESULT([[yes, -liconv (via cross.config)]])
    else
      LIBS="$old_LIBS"
    fi
  )
fi

if test $found_iconv = no ; then
  # -liconv with translit didn't work. Try giconv.h and -lgiconv.
  # BSDs install this lib as libgiconv.
  old_LIBS="$LIBS"
  LIBS="-lgiconv $LIBS"
  AC_RUN_IFELSE([AC_LANG_SOURCE([#include <giconv.h>
  int main(int argc,char **argv)
  { return iconv_open("UTF-8//TRANSLIT","ASCII") == -1 ? 1 : 0; }])]
  ,
    AC_DEFINE(HAVE_ICONV,1, [Define if you have this function])
    AC_DEFINE(HAVE_GICONV,1, [Define if you have this function])
    AC_MSG_RESULT([[yes, -lgiconv]])
    found_iconv=yes
  ,
    found_iconv=no
    LIBS="$old_LIBS"
  ,
    found_iconv="$cross_found_iconv_lgiconv";
    if test "$found_iconv" = "yes"; then
      AC_DEFINE(HAVE_ICONV,1, [Define if you have this function])
      AC_DEFINE(HAVE_GICONV,1, [Define if you have this function])
      AC_MSG_RESULT([[yes, -lgiconv (via cross.config)]])
    else
      LIBS="$old_LIBS"
    fi
  )
fi

AC_ARG_ENABLE(stricticonv,
  [  --enable-stricticonv		Build even if iconv is strict],,
  enable_stricticonv=no)
if test $enable_stricticonv = yes; then
AC_MSG_CHECKING(non-lossy iconv support)
AC_RUN_IFELSE([AC_LANG_SOURCE([#include <iconv.h>
int main(int argc,char **argv)
{ return iconv_open("UTF-8","ASCII") == -1 ? 1 : 0; }])]
,
  # libc has a working iconv.
  AC_DEFINE(HAVE_ICONV,1, [Define if you have this function])
  AC_MSG_RESULT([[yes, in libc]])
  found_iconv=yes
,
  found_iconv=no
)

if test $found_iconv = no ; then
  # libc doesn't have a working iconv. Try adding -liconv and any user
  # supplied directory.
  AC_ARG_WITH(libiconv-library,
    [  --with-libiconv-library=PATH  Library path for libiconv libraries],
    libiconv_libdir="$withval", libiconv_libdir="no")

  if test "$libiconv_libdir" != "no"; then
    GS_ADD_LIBRARY_PATH([${libiconv_libdir}])
  fi

  old_LIBS="$LIBS"
  LIBS="-liconv $LIBS"
  AC_RUN_IFELSE([AC_LANG_SOURCE([#include <iconv.h>
  int main(int argc,char **argv)
  { return iconv_open("UTF-8","ASCII") == -1 ? 1 : 0; }])]
  ,
    # -liconv works.
    AC_DEFINE(HAVE_ICONV,1, [Define if you have this function])
    AC_MSG_RESULT([[yes, -liconv]])
    found_iconv=yes
  ,
    found_iconv=no
    LIBS="$old_LIBS"
  )
fi

if test $found_iconv = no ; then
  # -liconv didn't work. Try giconv.h and -lgiconv.
  # BSDs install this lib as libgiconv.
  old_LIBS="$LIBS"
  LIBS="-lgiconv $LIBS"
  AC_RUN_IFELSE([AC_LANG_SOURCE([#include <giconv.h>
  int main(int argc,char **argv)
  { return iconv_open("UTF-8","ASCII") == -1 ? 1 : 0; }])]
  ,
    AC_DEFINE(HAVE_ICONV,1, [Define if you have this function])
    AC_DEFINE(HAVE_GICONV,1, [Define if you have this function])
    AC_MSG_RESULT([[yes, -lgiconv]])
    found_iconv=yes
  ,
    found_iconv=no
    LIBS="$old_LIBS"
  )
fi
fi

if test $found_iconv = no ; then
  AC_MSG_RESULT([[no]])
  echo
  echo "You do not appear to have usable iconv header/library."
  echo "Building without them will disable much characterset support."
  echo "If you really want to build gnustep-base without character conversion"
  echo " support, please add --disable-iconv to the configure arguments."
  AC_MSG_ERROR([Missing support for character conversion functionality.])
fi
fi

#--------------------------------------------------------------------
# Check recent libxml2 for GSXML and NSXMLNode
# See DEPENDENCIES POLICY at the start of this file.
#--------------------------------------------------------------------
AC_ARG_ENABLE(xml,
  [  --disable-xml			Build even if XML-dependencies are not met],,
  enable_xml=yes)

HAVE_LIBXML=0

if test $enable_xml = yes; then
  PKG_CHECK_MODULES([XML], [libxml-2.0 >= 2.3.0], [enable_libxml=yes], [
    AM_PATH_XML(2.3.0, enable_libxml=yes, enable_libxml=no)
  ])

  if test $enable_libxml = yes; then
    CPPFLAGS="$CPPFLAGS $XML_CFLAGS"
    INCLUDE_FLAGS="$INCLUDE_FLAGS $XML_CFLAGS"
    LIBS="$XML_LIBS $LIBS"
    HAVE_LIBXML=1
    AC_DEFINE(HAVE_LIBXML,1,[Define if libxml available])
    AC_CHECK_HEADERS(libxml/SAX2.h)

    #--------------------------------------------------------------------
    # Check for (optional) libxslt
    #--------------------------------------------------------------------
    AC_ARG_ENABLE(xslt,
      [  --disable-xslt		Build even if XSLT-dependency is not met],,
      enable_xslt=yes)

    if test $enable_xslt = yes; then
      AC_CHECK_LIB(xslt, xsltApplyStylesheet, xslt_ok=yes, xslt_ok=no)
      if test "$xslt_ok" = "yes"; then
	AC_CHECK_HEADER(libxslt/xslt.h, xslthdr_ok=yes, xslthdr_ok=no)
	if test "$xslthdr_ok" = "no"; then
	  xslt_ok=no
	fi
      fi
      if test "$xslt_ok" = "yes"; then
	HAVE_LIBXSLT=1
	AC_DEFINE(HAVE_LIBXSLT,1,[Define if libxslt available])
	LIBS="-lxslt $LIBS"
      else
	echo
	echo "You do not appear to have usable libxslt headers/library."
	echo "Building without them will disable the XSLT extensions."
	echo "If you really want to build gnustep-base without XSLT support,"
	echo "add --disable-xslt to the configure arguments to avoid warning."
	AC_MSG_WARN([Missing support for XSLT functionality.])
      fi
    else
      HAVE_LIBXSLT=0
      AC_MSG_WARN([Disabled support for XSLT funtionality.])
    fi
    AC_SUBST(HAVE_LIBXSLT)
  else
    echo
    echo "You do not appear to have usable libxml2 headers/library."
    echo "Building without them will disable the GSXML and NSXMLNode."
    echo "If you really want to build gnustep-base without XML DOM support"
    echo " (though NSXMLParser is unaffected),  please"
    echo "add --disable-xml to the configure arguments."
    AC_MSG_ERROR([Missing support for XML DOM functionality.])
  fi
else
  AC_MSG_WARN([XML functionality disabled.])
fi
AC_SUBST(HAVE_LIBXML)

#--------------------------------------------------------------------
# Check recent libgnutls for SSL streams.
# See DEPENDENCIES POLICY at the start of this file.
#--------------------------------------------------------------------
AC_ARG_ENABLE(tls,
  [  --disable-tls			Disable use of GNUTLS],,
  enable_tls=yes)

if test $enable_tls = yes; then
  HAVE_GNUTLS=0
  # Save CFLAGS and LIBS as AM_PATH_TLS clobbers these variables regardless
  # of the success of the macro.
  saved_LIBS="$LIBS"
  saved_CFLAGS="$CFLAGS"

  if test -n "$PKG_CONFIG"; then
    if pkg-config --exists gnutls; then
      AC_MSG_CHECKING(gnutls support)
      HAVE_GNUTLS=1
      TLS_CFLAGS=`$PKG_CONFIG --cflags gnutls`
      TLS_LIBS=`$PKG_CONFIG --libs gnutls`
    fi
  fi

  if test $HAVE_GNUTLS = 0; then
    #  AM_PATH_TLS(2.0.1, enable_libgnutls=yes, enable_libgnutls=no)
    AM_PATH_TLS(1.4.0, enable_libgnutls=yes, enable_libgnutls=no)
    if test $enable_libgnutls = yes; then
      HAVE_GNUTLS=1
    else
      HAVE_GNUTLS=0
    fi
  fi
  if test $HAVE_GNUTLS = 1; then
    if ! pkg-config --atleast-version=2.12 gnutls; then
      AC_CHECK_LIB(gcrypt, gcry_control, have_gcrypt=yes, have_gcrypt=no)
      if test "$have_gcrypt" = "no"; then
        HAVE_GNUTLS=0
      else
        TLS_LIBS="$TLS_LIBS -lgcrypt"
      fi
    fi
  fi
  if test $HAVE_GNUTLS = 0; then
    # Restore the CFLAGS and LIBS because AM_PATH_TLS messes them
    LIBS="$saved_LIBS"
    CFLAGS="$saved_CFLAGS"
    AC_MSG_RESULT(no)
    echo
    echo "You do not appear to have usable libgnutls headers/library."
    echo "Building without them will disable SSL/TLS/HTTPS in NSStream,"
    echo "NSFileHandle, NSURLHandle and NSURLConnection."
    echo "If you really want to build gnustep-base without TLS support,"
    echo "add --disable-tls to the configure arguments."
    AC_MSG_ERROR([Missing support for TLS functionality.])
  else
    AC_MSG_RESULT(yes)
    CPPFLAGS="$CPPFLAGS $TLS_CFLAGS"
    INCLUDE_FLAGS="$INCLUDE_FLAGS $TLS_CFLAGS"
    LIBS="$TLS_LIBS $LIBS"
    AC_CHECK_FUNCS(gnutls_transport_set_errno)
    if test "$ac_cv_func_gnutls_transport_set_errno" = "no"; then
      AC_MSG_WARN([Missing support for thread-safe error handling in GNUTLS.  Please check that you have the most recent version installed (2.0 or later chould be fine).])
    fi
    AC_CHECK_FUNCS(gnutls_x509_privkey_import2)
  fi
else
  AC_MSG_WARN([Disabled support for TLS funtionality.])
  HAVE_GNUTLS=0
fi
if test $HAVE_GNUTLS = 1; then
  AC_DEFINE_UNQUOTED(HAVE_GNUTLS,$HAVE_GNUTLS,[Define if libgnutls available])
fi
AC_SUBST(HAVE_GNUTLS)

#--------------------------------------------------------------------
# Check for NSNetServices
# See DEPENDENCIES POLICY at the start of this file.
#--------------------------------------------------------------------
HAVE_MDNS=0
HAVE_AVAHI=0
AC_ARG_ENABLE(zeroconf,
  [  --disable-zeroconf		Disable NSNetServices support],,
  enable_zeroconf=yes)
AC_ARG_WITH(zeroconf-api,
  [  --with-zeroconf-api=API  force use of a specific zeroconf API (mdns or avahi)],
  zeroconf_api="$withval", zeroconf_api="any")
if test $enable_zeroconf = yes; then
  if test "$zeroconf_api" = "any" || test "$zeroconf_api" = "mdns"; then
    AC_CHECK_HEADERS(dns_sd.h, have_mdns=yes, have_mdns=no)
    if test "$have_mdns" = "yes"; then
      AC_CHECK_LIB(dns_sd, DNSServiceBrowse, have_mdns=yes, have_mdns=no)
      if test "$have_mdns" = "yes"; then
        MDNS_LIBS="-ldns_sd"
        HAVE_MDNS=1
      fi
    fi
  fi
  if test "$zeroconf_api" = "any" || test "$zeroconf_api" = "avahi"; then
    AC_CHECK_HEADERS(avahi-client/client.h, have_avahi=yes, have_avahi=no)
    if test "$have_avahi" = "yes"; then
      AC_CHECK_LIB(avahi-client, avahi_client_new, have_avahi=yes, have_avahi=no)
      if test "$have_avahi" = "yes"; then
        AVAHI_LIBS="-lavahi-common -lavahi-client"
        HAVE_AVAHI=1
      fi
    fi
  fi
  # If we have both APIs, prefer Avahi, because the mDNS API
  # is most certainly the compatability one
  if test "$have_avahi" = "yes" && test "$have_mdns" = "yes"; then
    LIBS="$AVAHI_LIBS $LIBS"
    HAVE_MDNS=0
  else
    # One of those will be empty.
    LIBS="$AVAHI_LIBS $MDNS_LIBS $LIBS"
  fi
fi
AC_SUBST(HAVE_MDNS)
AC_SUBST(HAVE_AVAHI)
#--------------------------------------------------------------------
# Check for International Components for Unicode
# See DEPENDENCIES POLICY at the start of this file.
#--------------------------------------------------------------------
HAVE_ICU=0
AC_ARG_ENABLE(icu,
  [  --disable-icu			Disable International Components for Unicode],,
  enable_icu=yes)

if test "$enable_icu" = "yes"; then
  PKG_CHECK_MODULES([ICU], [icu-i18n > 49.0])
  AC_CHECK_HEADERS(unicode/uloc.h unicode/ulocdata.h unicode/ucol.h unicode/ucurr.h unicode/uregex.h unicode/ucal.h unicode/unorm2.h unicode/unum.h unicode/udat.h unicode/udatpg.h unicode/ustring.h unicode/usearch.h unicode/ucnv.h)
  LIBS="$LIBS $ICU_LIBS"
  HAVE_ICU=1
fi
AC_SUBST(HAVE_ICU)

#--------------------------------------------------------------------
# Check for libdispatch
# See DEPENDENCIES POLICY at the start of this file.
#--------------------------------------------------------------------

HAVE_LIBDISPATCH=0
AC_ARG_ENABLE(libdispatch,
  [  --disable-libdispatch		Disable dispatching blocks via libdispatch],
  enable_libdispatch=$enableval,
  enable_libdispatch=yes)

if test $enable_libdispatch = yes; then
  AC_ARG_WITH(dispatch-include,
  [  --with-dispatch-include=PATH       Include path for dispatch header],
      dispatch_incdir="$withval", dispatch_incdir="no")
  if test ${dispatch_incdir} != "no"; then
      CPPFLAGS="$CPPFLAGS -I${dispatch_incdir}"
      INCLUDE_FLAGS="$INCLUDE_FLAGS -I${dispatch_incdir}"
  fi

  AC_ARG_WITH(dispatch-library,
  [  --with-dispatch-library=PATH       Library path for dispatch lib],
      dispatch_libdir="$withval", dispatch_libdir="no")
  if test ${dispatch_libdir} != "no"; then
    GS_ADD_LIBRARY_PATH([${dispatch_libdir}])
  fi

  AC_CHECK_HEADERS(dispatch/dispatch.h, have_dispatch=yes, have_dispatch=no)
  if test "$have_dispatch" = "yes"; then
    # check for private header which includes runloop integration functions in
    # the Swift corelibs libdispatch release
    AC_CHECK_HEADERS(dispatch/private.h)
  else
    AC_CHECK_HEADERS(dispatch.h, have_dispatch=yes, have_dispatch=no)
  fi
  if test "$have_dispatch" = "yes"; then
    AC_CHECK_LIB(dispatch, dispatch_queue_create, have_dispatch=yes, have_dispatch=no)
    if test "$have_dispatch" = "yes"; then
      saveLIBS="$LIBS"
      LIBS="-lobjc -ldispatch";
      # This check is needed because libdispatch might be linked against a
      # version of libBlocksRuntime that defines symbols conflicting with libobjc
      AC_MSG_CHECKING(whether we can link libdispatch and libobjc at the same time)
      AC_TRY_LINK(,,have_dispatch=yes, have_dispatch=no)
      if test "$have_dispatch" = "yes"; then
	LIBS="$saveLIBS -ldispatch";
	AC_MSG_RESULT(yes);
	HAVE_LIBDISPATCH=1;
      else
	LIBS="$saveLIBS";
	AC_MSG_RESULT(no);
      fi
    fi
  else
    HAVE_LIBDISPATCH=0;
    # just ignore libdispatch if it's not there
  fi
fi
AC_SUBST(HAVE_LIBDISPATCH)

HAVE_LIBDISPATCH_RUNLOOP=0
if test $HAVE_LIBDISPATCH = 1; then
  # We check whether we have a variant of libdispatch that allows runloop
  # integration
  AC_CHECK_FUNCS(dispatch_main_queue_drain_np dispatch_get_main_queue_handle_np)
  if test "$ac_cv_func_dispatch_main_queue_drain_np" = "yes" && test "$ac_cv_func_dispatch_get_main_queue_handle_np" = "yes"; then
    HAVE_LIBDISPATCH_RUNLOOP=1
  fi
  # Check for "_4CF" variants of runloop integration functions provided by the
  # Swift corelibs libdispatch release
  AC_CHECK_FUNCS(_dispatch_main_queue_callback_4CF _dispatch_get_main_queue_handle_4CF)
  if test "$ac_cv_func__dispatch_main_queue_callback_4CF" = "yes" && test "$ac_cv_func__dispatch_get_main_queue_handle_4CF" = "yes"; then
    HAVE_LIBDISPATCH_RUNLOOP=1
  fi
fi
AC_SUBST(HAVE_LIBDISPATCH_RUNLOOP)


#--------------------------------------------------------------------
# Check for libcurl
# See DEPENDENCIES POLICY at the start of this file.
#--------------------------------------------------------------------
CURL_CONFIG="curl-config"
AC_ARG_WITH(curl, AS_HELP_STRING([--with-curl=<DIR>],
  [use curl installed in directory <DIR>]))
if test "$with_curl" != ""; then
  CURL_CONFIG="$with_curl/bin/curl-config"
fi

HAVE_LIBCURL=0
curl_all=no
AC_MSG_CHECKING([for libcurl])
if eval $CURL_CONFIG --version 2>/dev/null >/dev/null; then
  curl_ver=`$CURL_CONFIG --version | sed -e "s/libcurl //g"`
  curl_maj=`echo $curl_ver | sed -e "s/^\(.*\)\.\(.*\)\.\(.*\)$/\1/"`
  curl_min=`echo $curl_ver | sed -e "s/^\(.*\)\.\(.*\)\.\(.*\)$/\2/"`
  if test $curl_maj -lt 7 -o \( $curl_maj -eq 7 -a $curl_min -lt 49 \); then
    AC_MSG_RESULT([FAILED (version too old to use])
  else
    AC_MSG_RESULT(yes ... version $curl_ver)
    AC_CHECK_HEADERS(curl/curl.h, curl_ok=yes, curl_ok=no)
    if test "$curl_ok" = yes; then
      HAVE_LIBCURL=1
      CURLCFLAGS=`$CURL_CONFIG --cflags`
      CURLLIBS=`$CURL_CONFIG --libs`
      CFLAGS="$CFLAGS $CURLCFLAGS"
      LIBS="$LIBS $CURLLIBS"
      curl_all=yes
      if test \( $curl_maj -eq 7 -a $curl_min -lt 56 \); then
        AC_MSG_WARN([LOSS OF FUNCTIONALITY (version too old for SSL backend control)])
        curl_all=no
      elif test \( $curl_maj -eq 7 -a $curl_min -lt 65 \); then
        AC_MSG_WARN([LOSS OF FUNCTIONALITY (version too old for connection lifetime control)])
        curl_all=no
      fi
    fi
  fi
else
  AC_MSG_RESULT([FAILED (curl-config not found)])
fi

if test $HAVE_LIBCURL = 1; then
  AC_MSG_CHECKING([for libcurl TLS support with gnutls])
  if $CURL_CONFIG --configure | grep -q with-gnutls; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    curl_all=no
    AC_MSG_WARN([LOSS OF FUNCTIONALITY (curl lacks gnutls support)])
  fi
fi

if test $HAVE_LIBCURL = 1; then
  AC_CHECK_LIB(curl, curl_multi_perform, curl_ok=yes, curl_ok=no)
fi
if test $HAVE_LIBCURL = 1; then
  AC_CHECK_DECL(CURLOPT_CONNECT_TO, curl_ok=yes, curl_ok=no,
    [#include <curl/curl.h>])
fi
if test $HAVE_LIBCURL = 1; then
  # If curl supports runtime selection of the TLS module we will need
  # to use curl_global_sslset to pick GNUTLS
  AC_CHECK_FUNCS(curl_global_sslset)
fi
AC_SUBST(HAVE_LIBCURL)

#--------------------------------------------------------------------
# Check dependencies of NSURLSession API
#--------------------------------------------------------------------
nsurlsessiondefault=no
if test "$OBJC_RUNTIME_LIB" = "ng" -a $HAVE_BLOCKS = 1 -a $HAVE_LIBDISPATCH = 1 -a $HAVE_LIBCURL = 1; then
  nsurlsessiondefault=yes
fi

GS_HAVE_NSURLSESSION=0
AC_ARG_ENABLE(nsurlsession,
  [  --disable-nsurlsession  		Disable support for NSURLSession],,
  enable_nsurlsession=$nsurlsessiondefault)
if test $enable_nsurlsession = yes; then
  if test "$OBJC_RUNTIME_LIB" != "ng"; then
    AC_MSG_ERROR([Missing ng runtime (needed for NSURLSession). To build without NSURLSession support, please run configure again with the --disable-nsurlsession option.])
  fi
  if test $HAVE_BLOCKS = 0; then
    AC_MSG_ERROR([Missing blocks support (needed for NSURLSession).  To build without NSURLSession support, please run configure again with the --disable-nsurlsession option.])
  fi
  if test $HAVE_LIBDISPATCH = 0; then
    AC_MSG_ERROR([Missing libdispatch (needed for NSURLSession).  To build without NSURLSession support, please run configure again with the --disable-nsurlsession option.])
   fi
  # Check for dispatch_queue_create_with_target needed for NSURLSession
  AC_CHECK_FUNCS(dispatch_queue_create_with_target)
  if test $HAVE_LIBCURL = 0; then
    AC_MSG_ERROR([Missing libcurl (needed for NSURLSession). To build without NSURLSession support, please run configure again with the --disable-nsurlsession option.])
  fi
  GS_HAVE_NSURLSESSION=1
fi
AC_SUBST(GS_HAVE_NSURLSESSION)

#--------------------------------------------------------------------
# Check GMP for NSDecimal
#--------------------------------------------------------------------
AC_ARG_WITH(gmp-include,
  [  --with-gmp-include=PATH  include path for gmp headers],
  gmp_incdir="$withval", gmp_incdir="no")

AC_ARG_WITH(gmp-library,
  [  --with-gmp-library=PATH  library path for gmp libraries],
  gmp_libdir="$withval", gmp_libdir="no")

libs_temp=$LIBS

if test "$gmp_incdir" != "no"; then
  CPPFLAGS="$CPPFLAGS -I$gmp_incdir"
  INCLUDE_FLAGS="$INCLUDE_FLAGS -I$gmp_incdir"
fi
if test "$gmp_libdir" != "no"; then
  GS_ADD_LIBRARY_PATH([${gmp_libdir}])
fi

USE_GMP=0
AC_CHECK_HEADERS(gmp.h)
if test $ac_cv_header_gmp_h = yes; then
  AC_CHECK_LIB(gmp, mpf_abs, gmp_ok=yes, gmp_ok=no)
  if test "$gmp_ok" = no; then
    AC_CHECK_LIB(gmp, __gmpf_abs, gmp_ok=yes, gmp_ok=no)
  fi
  if test "$gmp_ok" = yes; then
    LIBS="-lgmp $LIBS"
    USE_GMP=1
  fi
fi

AC_SUBST(USE_GMP)


#--------------------------------------------------------------------
# Check whether nl_langinfo(CODESET) is supported, needed by Unicode.m.
#--------------------------------------------------------------------
AM_LANGINFO_CODESET


AC_SUBST(INCLUDE_FLAGS)
AC_SUBST(LDIR_FLAGS)

#--------------------------------------------------------------------
# Check for -Wdeclaration-after-statement
#--------------------------------------------------------------------
AC_MSG_CHECKING(whether the compiler supports -Wdeclaration-after-statement)

saved_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Wdeclaration-after-statement"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],HAS_W_DECL_AFTER_STATEMENT=yes,HAS_W_DECL_AFTER_STATEMENT=no)
CFLAGS="$saved_CFLAGS"

AC_MSG_RESULT($HAS_W_DECL_AFTER_STATEMENT)

if test x"$HAS_W_DECL_AFTER_STATEMENT" = x"yes"; then
  WARN_FLAGS="-Wall -Wdeclaration-after-statement"
else
  WARN_FLAGS=-Wall
fi

AC_SUBST(WARN_FLAGS)

#--------------------------------------------------------------------
# Check if we should use an alternative gdomap port
#--------------------------------------------------------------------
AC_ARG_WITH(gdomap-port,
  [  --with-gdomap-port=PORT  alternative port for gdomap],
  gdomap_port="$withval", gdomap_port="no")

if test "$gdomap_port" = "no"; then
  GNUSTEP_GDOMAP_PORT_OVERRIDE="$gdomap_port"
else
  GNUSTEP_GDOMAP_PORT_OVERRIDE="$gdomap_port"
fi
AC_SUBST(GNUSTEP_GDOMAP_PORT_OVERRIDE)

#--------------------------------------------------------------------
# Check if we should install gdomap as setuid
#--------------------------------------------------------------------
AC_MSG_CHECKING([if we should install gdomap as setuid])
AC_ARG_ENABLE(setuid-gdomap,[
  --enable-setuid-gdomap        Enable installing gdomap as a setuid
                                executable.  By default, it is installed
                                as a normal program intended to be started
				by root at system boot time, but it can
                                also be started up automatically
                                by any user at any time.  Use this
                                option if you are happy having the program
                                started automatically on demand.
],
  ac_cv_setuid_gdomap=$enableval,
  ac_cv_setuid_gdomap="no")

if test "$ac_cv_setuid_gdomap" = "yes"; then
  AC_MSG_RESULT(yes);
  GNUSTEP_INSTALL_GDOMAP_AS_SETUID="yes"
else
  AC_MSG_RESULT(no);
  GNUSTEP_INSTALL_GDOMAP_AS_SETUID="no"
fi

AC_SUBST(GNUSTEP_INSTALL_GDOMAP_AS_SETUID)

#--------------------------------------------------------------------
# Record the version
#--------------------------------------------------------------------
AC_MSG_CHECKING(for the version of gnustep-base we are compiling)
if test -f "Version"; then
  . ./Version
fi
AC_MSG_RESULT($VERSION)
AC_SUBST(VERSION)
AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(SUBMINOR_VERSION)
AC_SUBST(GCC_VERSION)

#--------------------------------------------------------------------
# Write the Makefiles
#--------------------------------------------------------------------
AC_CONFIG_FILES([config.mak base.make Headers/GNUstepBase/GSConfig.h])
AC_OUTPUT
