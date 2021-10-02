#ifndef FMAP_HPP
#define FMAP_HPP

#include "interfaces.hpp"
#include "utils.hpp"

template<template<typename> class F, class C>
requires Functor<F, typename DecomposeSign<C>::arg> && Callable < C,
    typename DecomposeSign<C>::arg(typename DecomposeSign<C>::ret)
> F<typename DecomposeSign<C>::ret> fmap(C a, F<typename DecomposeSign<C>::arg> f) {
    if (bool(f)) {
        return F<typename DecomposeSign<C>::ret>(a(f.value()));
    }
    return F<typename DecomposeSign<C>::ret>::nothing();
}

#endif  // FMAP_HPP
