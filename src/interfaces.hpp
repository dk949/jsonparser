#ifndef INTERFACES_HPP
#define INTERFACES_HPP
#include "utils.hpp"

#include <functional>
#include "parser.hpp"


template<typename C, typename F>
void fmap(C, F) {
    static_assert(sizeof(C) < 0, "No");
}

template<template<typename> class T, typename Enclosed, auto c>
concept Functor = requires(T<Enclosed> t) {
    fmap(c, t);
};



#endif  // INTERFACES_HPP
