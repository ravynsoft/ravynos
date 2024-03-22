# shellcheck disable=SC1091
# shellcheck disable=SC2086 # we want word splitting
if command -V ccache >/dev/null 2>/dev/null; then
  CCACHE=ccache
else
  CCACHE=
fi

if echo "$@" | grep -E 'meson-private/tmp[^ /]*/testfile.c' >/dev/null; then
    # Invoked for meson feature check
    exec $CCACHE $_COMPILER "$@"
fi

if [ "$(eval printf "'%s'" "\"\${$(($#-1))}\"")" = "-c" ]; then
    # Not invoked for linking
    exec $CCACHE $_COMPILER "$@"
fi

# Compiler invoked by ninja for linking. Add -Werror to turn compiler warnings into errors
# with LTO. (meson's werror should arguably do this, but meanwhile we need to)
exec $CCACHE $_COMPILER "$@" -Werror
