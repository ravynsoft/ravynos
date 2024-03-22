#include <cutils/properties.h>
#include <cutils/trace.h>

extern "C" {

int
property_get(const char *key, char *value, const char *default_value)
{
   return 0;
}

void
atrace_begin_body(const char * /*name*/)
{
}

void
atrace_end_body()
{
}

void
atrace_init()
{
}

uint64_t
atrace_get_enabled_tags()
{
   return ATRACE_TAG_NOT_READY;
}
}
