#ifdef __clang__
#define COREDATA_DLLEXPORT
#define COREDATA_DLLIMPORT
#else
#define COREDATA_DLLEXPORT __declspec(dllexport)
#define COREDATA_DLLIMPORT __declspec(dllimport)
#endif

#ifdef __cplusplus

#if defined(__WIN32__)
#if defined(COREDATA_INSIDE_BUILD)
#define COREDATA_EXPORT extern "C" COREDATA_DLLEXPORT
#else
#define COREDATA_EXPORT extern "C" COREDATA_DLLIMPORT
#endif
#else
#define COREDATA_EXPORT extern "C"
#endif

#else

#if defined(__WIN32__)
#if defined(COREDATA_INSIDE_BUILD)
#define COREDATA_EXPORT COREDATA_DLLEXPORT extern
#else
#define COREDATA_EXPORT COREDATA_DLLIMPORT extern
#endif
#else
#define COREDATA_EXPORT extern
#endif

#endif
