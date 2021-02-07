
#ifdef __clang__
#define FOR_IN(type, var, collection) \
  for (type var in collection)\
  {
#define END_FOR_IN(collection) }
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
void objc_enumerationMutation(id);
#pragma GCC diagnostic pop
#define FOR_IN(type, var, c) \
do\
{\
  type var;\
  NSFastEnumerationState gs_##c##_enumState = { 0 };\
  id gs_##c##_items[16];\
  unsigned long gs_##c##_limit = \
    [c countByEnumeratingWithState: &gs_##c##_enumState \
                           objects: gs_##c##_items \
                             count: 16];\
  if (gs_##c##_limit)\
  {\
    unsigned long gs_startMutations = *gs_##c##_enumState.mutationsPtr;\
    do {\
      unsigned long gs_##c##counter = 0;\
      do {\
        if (gs_startMutations != *gs_##c##_enumState.mutationsPtr)\
        {\
          objc_enumerationMutation(c);\
        }\
        var = gs_##c##_enumState.itemsPtr[gs_##c##counter++];\

#define END_FOR_IN(c) \
      } while (gs_##c##counter < gs_##c##_limit);\
    } while ((gs_##c##_limit \
      = [c countByEnumeratingWithState: &gs_##c##_enumState\
			       objects: gs_##c##_items\
				 count: 16]));\
  }\
} while(0);
#endif
