AC_DEFUN([CHECK_LLVM],
[
    AC_ARG_ENABLE([lto-support],
    AS_HELP_STRING([--enable-lto-support],
                   [enable link time optimization support]),
    [], [enable_lto_support=yes])

    AC_ARG_WITH([llvm-config],
    AS_HELP_STRING([--with-llvm-config],
                   [llvm config tool]),
    [LLVM_CONFIG=$with_llvm_config], [LLVM_CONFIG=no])

    if test "x$enable_lto_support" = "xyes"; then
      if test "x$LLVM_CONFIG" = "xno"; then
          AC_PATH_PROGS(LLVM_CONFIG,
              [llvm-config                                              \
               llvm-config-11.0 llvm-config-11                          \
               llvm-config-10.0 llvm-config-10                          \
               llvm-config-9.0 llvm-config-9                            \
               llvm-config-8.0 llvm-config-8                            \
               llvm-config-7.0 llvm-config-7                            \
               llvm-config-6.0 llvm-config-5.0 llvm-config-4.0          \
               llvm-config-3.9 llvm-config-3.8 llvm-config-3.7          \
               llvm-config-3.6 llvm-config-3.5 llvm-config-3.4          \
               llvm-config-3.3 llvm-config-3.2 llvm-config-3.1          \
               llvm-config110 llvm-config100                            \
               llvm-config90 llvm-config80                              \
               llvm-config70 llvm-config60                              \
               llvm-config50 llvm-config40                              \
               llvm-config39 llvm-config38 llvm-config37 llvm-config36  \
               llvm-config35 llvm-config34 llvm-config33 llvm-config32  \
               llvm-config31],
          no)
      fi

      if test "x$LLVM_CONFIG" != "xno"; then
        LLVM_INCLUDE_DIR="`${LLVM_CONFIG} --includedir`"
        LLVM_LIB_DIR="`${LLVM_CONFIG} --libdir`"

        ORIGLDFLAGS=$LDFLAGS
        LDFLAGS="$LDFLAGS -L${LLVM_LIB_DIR}"

        AC_CHECK_LIB([LTO],[lto_get_version], [

          # DO NOT include the LLVM include dir directly,
          # it may cause the build to fail.

          if test -e $LLVM_INCLUDE_DIR/llvm-c/lto.h; then
            cp -f $LLVM_INCLUDE_DIR/llvm-c/lto.h `dirname ${0}`/include/llvm-c

            if test -e $LLVM_INCLUDE_DIR/llvm-c/ExternC.h; then
              cp -f $LLVM_INCLUDE_DIR/llvm-c/ExternC.h `dirname ${0}`/include/llvm-c
            fi

            LTO_DEF=-DLTO_SUPPORT
            LTO_LIB="-L${LLVM_LIB_DIR} -lLTO"

            if test "x$rpathlink" = "xyes"; then
              LTO_RPATH="-Wl,-rpath,$LLVM_LIB_DIR,--enable-new-dtags"
            fi

            if test "x$isdarwin" = "xyes"; then
              LTO_RPATH="-Wl,-rpath,$LLVM_LIB_DIR"
            fi
          else
            AC_MSG_WARN([<llvm-c/lto.h> header file not found, disabling LTO support])
          fi

          AC_SUBST([LTO_DEF])
          AC_SUBST([LTO_RPATH])
          AC_SUBST([LTO_LIB])

        ])

        LDFLAGS=$ORIGLDFLAGS
      else
        AC_MSG_WARN([llvm-config not found, disabling LTO support])
      fi
    fi

    AC_SUBST(LLVM_CONFIG)
    AC_SUBST(LLVM_INCLUDE_DIR)
    AC_SUBST(LLVM_LIB_DIR)
])
