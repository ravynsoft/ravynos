#ifndef TURBO_UTIL_H
#define TURBO_UTIL_H

namespace turbo {

template <class Func>
inline void forEachNotNull(Func &&func)
{
}

template <class Func, class T, class... Args>
inline void forEachNotNull(Func &&func, T *arg, Args&&... args)
// When building with optimizations, this results in an unrolled version of:
//  for (auto *arg : args)
//      if (arg)
//          func(*arg);
{
    if (arg) func(*arg);
    forEachNotNull(static_cast<Func &&>(func), static_cast<Args &&>(args)...);
}

} // namespace turbo

#endif
