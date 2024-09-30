/*
 * A target program for fuzzing the XKB keymap text format.
 *
 * Currently, just parses an input file, and hopefully doesn't crash or hang.
 */
#include "config.h"

#include <assert.h>

#include "xkbcommon/xkbcommon.h"

int
main(int argc, char *argv[])
{
    struct xkb_context *ctx;
    FILE *file;
    struct xkb_keymap *keymap;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <file>\n", argv[0]);
        return 1;
    }

    ctx = xkb_context_new(XKB_CONTEXT_NO_DEFAULT_INCLUDES | XKB_CONTEXT_NO_ENVIRONMENT_NAMES);
    assert(ctx);

#ifdef __AFL_HAVE_MANUAL_CONTROL
  __AFL_INIT();

    while (__AFL_LOOP(1000))
#endif
    {
        file = fopen(argv[1], "rb");
        assert(file);
        keymap = xkb_keymap_new_from_file(ctx, file,
                                          XKB_KEYMAP_FORMAT_TEXT_V1,
                                          XKB_KEYMAP_COMPILE_NO_FLAGS);
        xkb_keymap_unref(keymap);
        fclose(file);
    }

    puts(keymap ? "OK" : "FAIL");
    xkb_context_unref(ctx);
}
