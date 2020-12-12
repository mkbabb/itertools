#ifndef ITERTOOLS_H
#define ITERTOOLS_H

#include "itertools/range_container.hpp"
#include "itertools/tupletools.hpp"
#include "itertools/types.hpp"
#include "itertools/views/all.hpp"

#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <vector>

#pragma once

namespace itertools {

template<class Iterable>
constexpr Iterable
roll(Iterable&& iter, int axis = -1)
{
    if (axis == 0) {
        return iter;
    } else if (axis == -1) {
        axis = iter.size() - 1;
    }
    int i = 0;
    while (i < axis) {
        std::swap(iter[axis], iter[i]);
        i += 1;
    };
    return iter;
}

template<class Func>
struct y_combinator
{
    Func func;
    y_combinator(Func func)
      : func(std::move(func))
    {}
    template<class... Args>
    auto operator()(Args&&... args) const
    {
        return func(std::ref(*this), std::forward<Args>(args)...);
    }
};

namespace detail {

struct get_ndim_impl
{
    size_t _ndim = 0;

    template<
        class Iterable,
        std::enable_if_t<tupletools::is_iterable_v<Iterable>, int> = 0>
    constexpr void recurse(Iterable&& iter)
    {
        _ndim = 0;
        auto it = std::begin(iter);
        ++it;
        recurse(std::forward<decltype(*it)>(*it));
        _ndim++;
    }

    template<class Tup, std::enable_if_t<tupletools::is_tupleoid_v<Tup>, int> = 0>
    constexpr void recurse(Tup&& tup)
    {
        tupletools::for_each(tup, [&](auto&& n, auto&& v) {
            recurse(std::forward<decltype(v)>(v));
        });
    }

    template<
        class Iterable,
        std::enable_if_t<
            !tupletools::is_iterable_v<Iterable> &&
                !tupletools::is_tupleoid_v<Iterable>,
            int> = 0>
    constexpr void recurse(Iterable&& iter)
    {}

    template<class Iterable>
    constexpr size_t operator()(Iterable&& iter)
    {
        recurse(std::forward<Iterable>(iter));
        return _ndim + 1;
    }
};
}; // namespace detail

template<class Iterable>
constexpr size_t
get_ndim(Iterable&& iter)
{
    return detail::get_ndim_impl{}(std::forward<Iterable>(iter));
}

template<typename Iterable>
std::string
join(Iterable&& iter, std::string&& sep)
{
    std::ostringstream result;
    for (auto&& [n, i] : views::enumerate(iter)) {
        result << (n > 0 ? sep : "") << i;
    }
    return result.str();
}

std::string
ltrim(std::string s)
{
    auto end =
        std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); });
    s.erase(s.begin(), end);
    return s;
}

std::string
rtrim(std::string s)
{
    auto begin = std::find_if(s.rbegin(), s.rend(), [](int ch) {
                     return !std::isspace(ch);
                 }).base();
    s.erase(begin, s.end());
    return s;
}

std::string
trim(std::string s)
{
    ltrim(s);
    rtrim(s);
    return s;
}

};     // namespace itertools
#endif // ITERTOOLS_H
