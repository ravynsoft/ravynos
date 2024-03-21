#ifndef TURBO_FUNCVIEW_H
#define TURBO_FUNCVIEW_H

#include <type_traits>

namespace detail {

template<class A, class B>
using enable_if_not_same_t =
    typename std::enable_if<
        !std::is_same<
            typename std::remove_reference<A>::type,
            typename std::remove_reference<B>::type
        >::value
    >::type;

} // namespace detail

template<class Signature>
class TFuncView;

template<class R, class... Args>
class TFuncView<R(Args...)>
{

    // This class implements a function view.
    //
    // TFuncView can be initialized with a function reference or with a
    // callable object (such as a lambda). If you have a function pointer
    // instead, you'll have to ensure it is not null and dereference it:
    //
    //  void (*func_ptr)() = &some_function;
    //  TFuncView<void()> func_view {*func_ptr};
    //
    // TFuncView cannot be empty; unlike std::function, it always contains
    // a target. That's one less case you have to check. Also unlike std::function,
    // 'operator()' is 'noexcept'; this means TFuncView cannot propagate exceptions.
    //
    // Like string views, function views do not control the lifetime of
    // whatever they reference. Watch out.

    union Context
    {
        void *voidPtr;
        R (*funcPtr)(Args...);
        constexpr Context(void *voidPtr) : voidPtr(voidPtr) {}
        constexpr Context(R (*funcPtr)(Args...)) : funcPtr(funcPtr) {}
    } ctx;
    R (* invk)(Context, Args...) noexcept;

    constexpr static R invkStatic(Context ctx, Args... args) noexcept
    {
        return (*ctx.funcPtr)(args...);
    }

    template <class Functor_>
    static R invkFunctor(Context ctx, Args... args) noexcept
    {
        using Functor = typename std::remove_reference<Functor_>::type;
        return (*static_cast<Functor *>(ctx.voidPtr))(static_cast<Args>(args)...);
    }

public:

    constexpr TFuncView(R (&func)(Args...)) noexcept :
        ctx(func),
        invk(&invkStatic)
    {
    }

    template <class Functor,
              class = detail::enable_if_not_same_t<TFuncView, Functor>, // Avoid infinite recursion.
              class = detail::enable_if_not_same_t<R (*)(Args...), Functor>> // Avoid function pointers.
    TFuncView(Functor &&func) noexcept :
        ctx((void *) &func),
        invk(&invkFunctor<Functor>)
    // Pre: the lifetime of 'func' exceeds the lifetime of 'this'.
    {
    }

    constexpr R operator()(Args... args) const noexcept
    {
        return (*invk)(ctx, static_cast<Args>(args)...);
    }

};

#endif // TURBO_FUNCVIEW_H
