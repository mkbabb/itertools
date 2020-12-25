#include <type_traits>

#include "itertools/range_iterator.hpp"
#include "itertools/tupletools.hpp"

#pragma once

namespace itertools {

constexpr auto forward_inserter = [](auto& container, auto&& y) {
    container.push_back(y);
};

constexpr auto copy_inserter = [](auto& container, auto y) { container.push_back(y); };

template<template<typename... Ts> class Container, class Func = decltype(copy_inserter)>
decltype(auto)
to(Func&& inserter = {})
{
    return [inserter = std::forward<Func>(inserter)]<tupletools::Rangeable Range>(
             Range&& range) {
        using Value = std::remove_cvref_t<tupletools::range_value_t<Range>>;
        auto container = Container<Value>{};

        for (auto&& x : range) {
            using T = decltype(x);
            inserter(container, std::forward<T>(x));
        }

        return container;
    };
}

template<class T>
requires std::is_same_v<std::string, T> decltype(auto)
to()
{
    return []<tupletools::Rangeable Range>(Range&& range) {
        std::string s;
        for (auto&& x : range) {
            s += x;
        }
        return s;
    };
}

}