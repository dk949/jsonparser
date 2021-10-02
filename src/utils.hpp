#ifndef UTILS_HPP
#define UTILS_HPP

#include <optional>

template<typename T>
struct Maybe : std::optional<T> {
    using std::optional<T>::optional;

    static Maybe<T> nothing() {
        return std::nullopt;
    }
};


template<typename T>
struct function_traits : public function_traits<decltype(&T::operator())> {};

template<typename ReturnType, typename... Args>
struct function_traits<ReturnType (*)(Args...)> {
    using result_type = ReturnType;
    using arg_tuple = std::tuple<Args...>;
    static constexpr auto arity = sizeof...(Args);
};


template<typename T>
concept IsLambda = requires() {
    &T::operator();
};

template<typename Sign>
struct DecomposeSign;

template<IsLambda Sign>
struct DecomposeSign<Sign> : DecomposeSign<std::decay_t<decltype(&Sign::operator())>> {};

template<typename Ret, typename Arg>

struct DecomposeSign<Ret(Arg)> {
    using ret = Ret;
    using arg = Arg;
};

template<typename Ret, typename Arg>
struct DecomposeSign<Ret (*)(Arg)> : DecomposeSign<Ret(Arg)> {};


template<typename T, typename Sign>
concept Callable = requires(T t, typename DecomposeSign<Sign>::arg a) {
    { t(a) } -> std::same_as<typename DecomposeSign<Sign>::ret>;
};



template<typename T>
struct respecialise {};

template<template<typename> class T, typename U>
struct respecialise<T<U>> {
    template<typename TT>
    using type = T<TT>;
};

template<typename From, typename To>
using respecialise_t = typename respecialise<From>::template type<To>;


#endif  // UTILS_HPP
