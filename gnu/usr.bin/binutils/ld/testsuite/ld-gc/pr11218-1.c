extern void unresolved_detected_at_runtime_not_at_linktime(void);
void foo_in_so(void)
{
   unresolved_detected_at_runtime_not_at_linktime();
}
