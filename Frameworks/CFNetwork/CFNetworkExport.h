
#ifdef __clang__
#define CFNETWORK_DLLEXPORT
#define CFNETWORK_DLLIMPORT
#else
#define CFNETWORK_DLLEXPORT __declspec(dllexport)
#define CFNETWORK_DLLIMPORT __declspec(dllimport)
#endif

#ifdef __cplusplus

#if defined(__WIN32__)
#if defined(CFNETWORK_INSIDE_BUILD)
#define CFNETWORK_EXPORT extern "C" CFNETWORK_DLLEXPORT
#else
#define CFNETWORK_EXPORT extern "C" CFNETWORK_DLLIMPORT
#endif
#else
#define CFNETWORK_EXPORT extern "C"
#endif

#else

#if defined(__WIN32__)
#if defined(CFNETWORK_INSIDE_BUILD)
#define CFNETWORK_EXPORT CFNETWORK_DLLEXPORT extern
#else
#define CFNETWORK_EXPORT CFNETWORK_DLLIMPORT extern
#endif
#else
#define CFNETWORK_EXPORT extern
#endif

#endif // __cplusplus
