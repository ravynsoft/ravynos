#include "rusticl_system_bindings.h"

FILE *
stdout_ptr(void)
{
    return stdout;
}

FILE *
stderr_ptr(void)
{
    return stderr;
}
