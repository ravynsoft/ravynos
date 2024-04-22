/*
 * A target program for fuzzing the Compose text format.
 *
 * Currently, just parses an input file, and hopefully doesn't crash or hang.
 */
#include "config.h"

#include <assert.h>

#include "xkbcommon/xkbcommon.h"
#include "xkbcommon/xkbcommon-compose.h"

int
main(int argc, char *argv[])
{
    struct xkb_context *ctx;
    FILE *file;
    struct xkb_compose_table *table;

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
        table = xkb_compose_table_new_from_file(ctx, file,
                                                "en_US.UTF-8",
                                                XKB_COMPOSE_FORMAT_TEXT_V1,
                                                XKB_COMPOSE_COMPILE_NO_FLAGS);
        xkb_compose_table_unref(table);
        fclose(file);
    }

    puts(table ? "OK" : "FAIL");
    xkb_context_unref(ctx);
}
