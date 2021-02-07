# SYNOPSIS
#
#   GS_LIBRARY_COMBO([default_library_combo],[no-cache])
#
# DESCRIPTION
#
#   This macro installs the library combo configuration by setting  the following:
#
#     * The makefile variable `ac_cv_library_combo'
#     * The variables `OBJC_RUNTIME_LIB' and `ac_cv_library_combo'
#
AC_DEFUN([GS_LIBRARY_COMBO],dnl
  [AC_REQUIRE([AC_CANONICAL_TARGET])
  AC_REQUIRE([AC_PROG_AWK])
  gs_cv_library_combo_from_existing_install=no
  m4_ifblank([$1], [
    case "$host_os" in
          darwin*[)]   default_library_combo=apple-apple-apple ;;
          nextstep4[)] default_library_combo=nx-nx-nx          ;;
          openstep4[)] default_library_combo=nx-nx-nx          ;;
          *[)]         default_library_combo=gnu-gnu-gnu       ;;
    esac
    if test x"$HAVE_GNUSTEP_CONFIG" = x"yes"; then
      maybe_library_combo=$(gnustep-config --variable=LIBRARY_COMBO||echo "")
      if test ! x"$maybe_library_combo" = x""; then
        default_library_combo=$maybe_library_combo
        gs_cv_library_combo_from_existing_install=yes
      fi
    fi
  ], [
    default_library_combo="$1"
    ])

  m4_if([$2], [yes], [
    AS_UNSET([_gs_cv_libray_combo])
    AS_UNSET([with_library_combo])
  ])

    AC_ARG_WITH([library-combo],
        [AS_HELP_STRING([--with-libray-combo], [
            Define the default "library combo".  The library combo is a string
            of the form aaa-bbb-ccc where 'aaa' is the Objective-C runtime
            library to use (examples are 'gnu' and 'apple'),
            'bbb' is the Foundation library to use (examples are 'gnu' for
            gnustep-base, and 'apple' for Apple Cocoa FoundationKit),
            and 'ccc' is the ApplicationKit to use (examples are 'gnu'
            for gnustep-gui and 'apple' for Apple Cocoa AppKit).  Use this
            option if you want to force a different default library combo than
            the one that would be used by default.  For example, on Darwin GNUstep
            will automatically use the Apple Objective-C frameworks by
            default (library-combo=apple-apple-apple); if you are planning
            on installing and using gnustep-base on there, you would need
            to use --with-library-combo=gnu-gnu-gnu instead.  Please notice
            that if --disable-flattened is used, gnustep-make can have fat
            binaries that support multiple library combos.  In that case,
            this flag will only configure the default one, but you can still
            use other ones at run-time.
            Please use 'ng-gnu-gnu' to build with 'next generation' cutting edge
            runtime and compile time features (requires a recent version of clang).
        ])],[
          gs_cv_library_combo_implicit=no
        ],[dnl
          with_library_combo=${default_library_combo}
          if test x"$gs_cv_library_combo_from_existing_install" = x"yes"; then
            gs_cv_library_combo_implicit=no
            AC_MSG_NOTICE([Using library combo $default_library_combo from your existing installation of gnustep-make. To prevent this, pass --with-library-combo explicitly.])
          else
            gs_cv_library_combo_implicit=yes
          fi
        ])
  AC_CACHE_CHECK([for library combo],[_gs_cv_libray_combo], [
    case "$with_library_combo" in
        apple) with_library_combo=apple-apple-apple ;;
        gnu)   with_library_combo=gnu-gnu-gnu ;;
        ng)    with_library_combo=ng-gnu-gnu ;;
        nx)    with_library_combo=nx-nx-nx ;;
    esac
    _gs_cv_libray_combo=${with_library_combo}
  ])
  AS_VAR_SET([ac_cv_library_combo], [${_gs_cv_libray_combo}])
  AS_VAR_SET([OBJC_RUNTIME_LIB], [$(echo ${_gs_cv_libray_combo} | $AWK -F- '{ print $[1] }')])
  AC_SUBST([ac_cv_library_combo], [${_gs_cv_libray_combo}])
])